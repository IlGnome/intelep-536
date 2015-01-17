/*****************************************************************************
 Copyright (c) 1999-2004, Intel Corporation 

 All rights reserved.

 Redistribution and use in source and binary forms, with or without 
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, 
 this list of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation 
 and/or other materials provided with the distribution.

 3. Neither the name of Intel Corporation nor the names of its contributors 
 may be used to endorse or promote products derived from this software 
 without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
 LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 

June 2005- August 2005 Modified for use with Linux kernel 2.6
                         by Philippe Vouters (Philippe.Vouters@laposte.net)


****************************************************************************/


#include "hamcore.h"
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/ptrace.h>
#include <linux/version.h>
#include <linux/interrupt.h>
#include "rts.h"
#include "tasker.h"

/* Local functions */
static void  RestartTickers(void);
static ubyte TimeSliceExpired(void);
static ubyte MinLoopsExpired(void);

typedef struct tasker
{
   ulong active;              /* Denotes whether a task has been scheduled */
   ulong time_slice;          /* Number of clock ticks we're allowed to run */
   ulong time_slice_start;    /* Clock tick in msecs when we started */
   ulong time_slice_end;      /* Clock tick in msecs when we expect to end */

   void  (*Task)(void);       /* task to schedule */

   uword min_loops;           /* Minimum number of times to run RSACore() */
   uword cur_loops;           /* Current tasker loop count for min_loops */

} TASKER;

/* Local data */
TASKER clm_task = {0};      /* Instance of modem task */

/****************************************************************************
/ Function:  TaskerInitStack
/
/ Remarks:   Initializes the RSA task stack
/
/ Inputs;    none
/
/ Outputs:   none
*****************************************************************************/
void TaskerInitStack(void (*Task)(void))
{
   clm_task.active = 0;
   clm_task.Task = Task;

   /*
    * The default Tasker minimum run time should be set to zero,
    * which is non-intrusive.  This will cause the RSA core code
    * to run only once per call to RSATaskSchedule(), just as
    * it would if Tasker were stubbed out.  If a platform (such as CLM)
    * needs to set a minumum runtime, it can update the
    * platform-specific value with a call to RSATaskSetMinRunTime().
    */
   RSATaskSetMinRunTime(RUN_ONE_TIME_ONLY, TASKER_INITIALIZE);
   RSATaskSetMinLoops(TASKER_NO_LOOPS, TASKER_INITIALIZE);
}

/****************************************************************************
/ Function:  RSATaskSchedule
/
/ Remarks:   Schedules the task identified in TaskerInitStack()
/
/ Inputs;    none
/
/ Outputs:   none
*****************************************************************************/
void RSATaskSchedule(void)
{
   clm_task.active = 1;
}

/****************************************************************************
/ Function:  RSATaskYield
/
/ Remarks:   Yields to the native operating system if task time expired
/
/ Inputs;    none
/
/ Outputs:   none
*****************************************************************************/
asmlinkage void RSATaskYield(void)
{
   extern volatile ubyte rs_task_yielded;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
   extern struct wait_queue *yield_wq;
#else
   extern wait_queue_head_t yield_wq;
#endif



   clm_task.cur_loops++;

   if (!MinLoopsExpired())
   {
      return;
   }

   if (!TimeSliceExpired())
   {
      return;
   }

   /* indicate that RSATask has yielded so RTS will resume it */
   rs_task_yielded = TRUE;
   if (in_softirq())
   {
	   printk(KERN_ERR"in interrupt!!!\n");
   } else
       wait_event_interruptible(yield_wq,rs_task_yielded == FALSE);
   RestartTickers();
}

/****************************************************************************
/ Function:  RSATaskSetMinRunTime
/
/ Remarks:
/
/ Inputs;    none
/
/ Outputs:   none
*****************************************************************************/
asmlinkage ulong RSATaskSetMinRunTime(ubyte msecs, ubyte reason)
{
   ulong time_slice = clm_task.time_slice;

   clm_task.time_slice = msecs;

   /**********************************************************************/
   /* If the task is currently active, then the caller is requesting the */
   /* additional time it is specifying.                                  */
   /**********************************************************************/
   if (clm_task.active)
   {
      clm_task.time_slice_end = RTS_Get_Ticks() + clm_task.time_slice;
   }

   return time_slice;
}

/****************************************************************************
/ Function:  RSATaskSetMinLoops
/
/ Remarks:
/
/ Inputs;    none
/
/ Outputs:   none
*****************************************************************************/
uword RSATaskSetMinLoops(uword loops, ubyte reason)
{
   uword min_loops = clm_task.min_loops;

   clm_task.min_loops = loops;

   return (min_loops);
}

/****************************************************************************
/ Function:  RestartTickers
/
/ Remarks:   Resets the current time slice ticker.
/
/ Inputs;    none
/
/ Outputs:   none
*****************************************************************************/
static void RestartTickers(void)
{
   /* Reset time slice start time */
   clm_task.time_slice_start = RTS_Get_Ticks();
   clm_task.time_slice_end = clm_task.time_slice_start + clm_task.time_slice;

   /* Reset count of minimum iterations */
   clm_task.cur_loops = 0;
}

/****************************************************************************
/ Function:  TimeSliceExpired
/
/ Remarks:   Checks if time slice has expired and return situation.
/
/ Inputs;    none
/
/ Outputs:   none
*****************************************************************************/
static ubyte TimeSliceExpired(void)
{
   ubyte rc;

   if (RTS_Get_Ticks() >= clm_task.time_slice_end)
      rc = TRUE;
   else
      rc = FALSE;

   return (rc);
}

/****************************************************************************
/ Function:  MinLoopsExpired
/
/ Remarks:
/
/ Inputs;    none
/
/ Outputs:   none
*****************************************************************************/
static ubyte MinLoopsExpired(void)
{
   ubyte rc;

   /* See if we've looped the minimum number of iterations */
   if (clm_task.cur_loops >= clm_task.min_loops)
   {
      rc = TRUE;
   }
   else
   {
      rc = FALSE;
   }

   return(rc);
}

