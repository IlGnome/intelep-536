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

*****************************************************************************/


#include "hamcore.h"
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <asm/system.h>
#include <asm/irq.h>
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,5,0) 
#include <linux/tqueue.h>
#else
#include <linux/workqueue.h>
#endif


#include "rts.h"
#include "lock_lin.h"
#include <linux/interrupt.h>

#define MAX_TASK_RUN_TIME   10

volatile ubyte rs_task_enabled = FALSE;
volatile ubyte rs_task_running = FALSE;
volatile ubyte rs_timer_running = FALSE;

volatile ubyte rs_task_yielded = FALSE;
DECLARE_WAIT_QUEUE_HEAD(yield_wq);
DECLARE_WAIT_QUEUE_HEAD(terminate_wq);

volatile ulong rts_ticks;
volatile ulong rts_timeout;
volatile ulong rts_timer;
volatile ulong global_ham_timer;
extern spinlock_t Lock_timer_task;

struct timer_list rs_timer;

void RTS_Task(void *ptr);

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,0) 
struct tq_struct  rs_task =
{
   routine: (void (*) (void*)) RTS_Task,
   data: NULL
};
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,20)
//DECLARE_TASKLET(rs_tasklet, RTS_Task, 0);
static DECLARE_WORK(rs_wq, (work_func_t)RTS_Task);
#else
static DECLARE_WORK(rs_wq, RTS_Task, 0);
#endif

void RTS_Timer(unsigned long ptr);
void RTS_Task_Schedule(void);
void RTS_Task_Terminate(void);

extern asmlinkage void acu_timer(void);

/****************************************************************************
/ Function:  RTS_Enable
/
/ Remarks:   Enables Real-Time Services
/
/ Inputs;    none
/
/ Outputs:   none
*****************************************************************************/
void RTS_Enable(void)
{
   if (!rs_timer_running)
   {
      rs_timer_running = TRUE;
      rs_task_enabled = TRUE;

      rts_ticks = 0;
      rts_timeout = RTS_ASYNC_TIMER;
      rts_timer = rts_timeout;

      /* initialize RTS task */
      rs_task_running = FALSE;
      rs_task_yielded = FALSE;

      /* initialize RTS timer */
      init_timer(&rs_timer);

      /* schedule RTS timer to run */
      rs_timer.function = RTS_Timer;
      rs_timer.data = 0;
      rs_timer.expires = jiffies + 1;
      add_timer(&rs_timer);
   }
}

/****************************************************************************
/ Function:  RTS_Disable
/
/ Remarks:   Disables Real-Time Services
/
/ Inputs;    none
/
/ Outputs:   none
*****************************************************************************/
void RTS_Disable(void)
{
   if (rs_timer_running)
   {
      rs_timer_running = FALSE;
      /* stop RTS timer */
      MT_AcquireLock_timer_task();
      del_timer_sync(&rs_timer);
      MT_ReleaseLock_timer_task();
   }
}

/****************************************************************************
/ Function:  RTS_Get_Ticks
/
/ Remarks:   Gets RTS tick counter
/
/ Inputs;    none
/
/ Outputs:   ticks - current system tick counter
*****************************************************************************/
ulong RTS_Get_Ticks(void)
{
   return (rts_ticks);
}

/****************************************************************************
/ Function:  RTS_Set_Timeout
/
/ Remarks:   This routine allows the RTS timer frequency to be adjusted.
/
/ Inputs;    timeout - timeout frequency (in msec)
/
/ Outputs:   none
*****************************************************************************/
asmlinkage void RTS_Set_Timeout(ulong timeout)
{
   linux_AcquireLock_rts1();

   rts_timeout = timeout;

   linux_ReleaseLock_rts1();
}

/****************************************************************************
/ Function:  RTS_Timer
/
/ Remarks:   RTS timer handler
/
/ Inputs;    none
/
/ Outputs:   none
*****************************************************************************/
extern asmlinkage void linux_sys_isr_timer_ticker(void);
#define NANO_TIME_QUANTUM 1000000000/HZ 
void RTS_Timer(unsigned long ptr)
{
    static unsigned long time = 0;
    static unsigned long past_jiffies = 0;

    MT_AcquireLock_timer_task();
    if(past_jiffies==0) past_jiffies=jiffies-1;

    if (rs_timer_running)
    {
        time = time + (NANO_TIME_QUANTUM * (jiffies - past_jiffies));
        while(time >= 10000000)
        {
            time = time - 10000000;

            linux_sys_isr_timer_ticker(); 
            global_ham_timer--;
            rts_ticks += 10;
            acu_timer();
            if (rts_timer >= 10)
                rts_timer -= 10;
            else
            rts_timer = 0;

            if (rts_timer == 0)
            {
                RTS_Task_Schedule();
            }
        }
        past_jiffies = jiffies;
        /* reschedule RTS timer to run again */
        /* 2.6.x timer is faster, let's slow our code down a bit */
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,0) 
//        rs_timer.expires = jiffies + 1;
        mod_timer(&rs_timer,jiffies+1);
#else
//        rs_timer.expires = jiffies + 10;
//#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,19)
//        mod_timer(&rs_timer,jiffies+CONFIG_HZ/100);
//#else
        mod_timer(&rs_timer,jiffies+1);
//#endif
#endif
//        add_timer(&rs_timer);
    }
    MT_ReleaseLock_timer_task();
}

/****************************************************************************
/ Function:  RTS_Task
/
/ Remarks:   RTS Task (bottom half)
/
/ Inputs;    ptr
/
/ Outputs:   none
*****************************************************************************/
void RTS_Task(void *ptr)
{
   if(rs_task_enabled && rs_task_running)

   {
      clm_bg();
   }
   rs_task_running = FALSE;
   wake_up_interruptible(&terminate_wq);
   return;
}

/****************************************************************************
/ Function:  RTS_Task_Schedule
/
/ Remarks:   This routine schedules RTS_Task to run.  If RTS_Task is not
/            currently running, it will be scheduled to run.  If RTS_Task
/            is already running but has yielded, then it will be awakened.
/
/ Inputs;    none
/
/ Outputs:   none
*****************************************************************************/
void RTS_Task_Schedule(void)
{
   if (rs_task_enabled)
   {
      /* if RTS_Task() is not currently running, start it */
      if (!rs_task_running)
      {
         rts_timer = rts_timeout;
         rs_task_running = TRUE;
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,5,0) 
      schedule_task(&rs_task);
#else
//      tasklet_schedule(&rs_tasklet);
      schedule_work(&rs_wq);
#endif
      }
      /* if RTS_Task() is running but is currently suspended, resume it */
      else if (rs_task_yielded)
      {
         rts_timer = rts_timeout;
         rs_task_yielded = FALSE;
         wake_up_interruptible(&yield_wq);
      }
   }
}

/****************************************************************************
/ Function:  RTS_Task_Terminate
/
/ Remarks:   This routine attempts to terminate RTS_Task gracefully.
/
/ Inputs;    none
/
/ Outputs:   none
*****************************************************************************/
void RTS_Task_Terminate(void)
{
   int timeout = HZ*4;
   /* don't allow task rescheduling until RTS_Enable() is called again */
   rs_task_enabled = FALSE;

   while(rs_task_running && timeout > 0)
   {
#if defined(DEBUG_LINUX)
         printk(KERN_INFO"ham: task terminate T%d\n", timeout);
#endif
//      timeout = interruptible_sleep_on_timeout(&terminate_wq, timeout);
      timeout = wait_event_interruptible_timeout(terminate_wq, rs_task_running == FALSE, timeout);
   }
#if defined(DEBUG_LINUX)
      printk(KERN_INFO"ham task terminated, to=%d\n", timeout);
#endif
}

