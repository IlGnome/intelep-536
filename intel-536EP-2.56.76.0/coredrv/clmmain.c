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

****************************************************************************/


//#define DEBUG_PORT_LOAD
//#define DEBUG_PORT_UNLOAD

#include "hamcore.h"
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/proc_fs.h>
#include <linux/workqueue.h>
#include <asm/atomic.h>

#include "uart.h"
#include "tasker.h"
#include "rts.h"
#include "lock_lin.h"

/* Constants */
#define UNKNOWN_STATE       99

/* Local defines */
#define DATA_CLASS_MODE     0
#define FAX_CLASS_MODE      1
#define VOICE_CLASS_MODE    8
#define VVIEW_CLASS_MODE    80
#define WAKEUP_CHARS        256

/* global variables */
ubyte last_fax1_state = UNKNOWN_STATE;
ubyte last_acu_state = UNKNOWN_STATE;
ubyte last_parse_state = UNKNOWN_STATE;
ubyte ext_intr_pending = FALSE;
int acu_init_ok = FALSE;


/* Function prototypes */
void UpdateMinRunTime(void);
void UpdateDataRunTime(void);
void UpdateFaxRunTime(void);

extern asmlinkage void Init_WWH(int);  //t35 code in hex
extern asmlinkage int Read_ProductStrings(void);
extern asmlinkage int Read_EEPROM(void);
extern asmlinkage int Read_CurrentCountry(void);
extern asmlinkage int Read_CountryList(void);
void clm_rx_int(void *ptr);

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,0)
struct tq_struct  clm_rx_task =
{
   routine: (void (*) (void*)) clm_rx_int,
   data: NULL
};
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,20)
static DECLARE_WORK(clm_rx_task, (work_func_t)clm_rx_int);
#else
static DECLARE_WORK(clm_rx_task, clm_rx_int, 0);
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
   struct semaphore clm_bg_sem = MUTEX;
   struct semaphore clm_rx_int_sem = MUTEX;
#else
//   DECLARE_MUTEX(clm_bg_sem);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,17)
   DECLARE_MUTEX(clm_rx_int_sem);
#else
   static atomic_t clm_rx_int_sem;
#endif
#endif

/****************************************************************************
/ Function:  clm_bg
/
/ Remarks:   Calls all necessary background routines
/
/ Inputs;    none
/
/ Outputs:   none
*****************************************************************************/
extern void send_data_to_user(void) ;
void clm_bg(void)
{
   MT_AcquireLock_clm_bg();
   // If there has been some kind of fatal modem error, hard_reset()
   // may have been called, which would clear the acu_loop global
   // variable.  If the flag has been cleared, reinitialize the modem.
   if (!acu_loop)
   {
#if defined(DEBUG_LINUX)
printk(KERN_INFO"ham:clm_bg: !aculoop...running master init\n");
#endif
      /* Need extra time during critical portion */
      RSATaskSetMinRunTime(INIT_RUN_TIME, TASKER_INIT_MODEM);
      master_init();

      /* Until an AT command is being parsed, there is no need
         to consume cycles. */
      RSATaskSetMinRunTime(RUN_ONE_TIME_ONLY, TASKER_IDLE_MODE);
#if defined(DEBUG_LINUX)
      printk(KERN_INFO"ham:clm_bg: !aculoop done\n");
#endif
   }
   send_data_to_user();
   ACU_Main();
#if 0
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,5,0)
      schedule_task(&clm_rx_task);
#else
//      tasklet_schedule(&rs_tasklet);
      schedule_work(&clm_rx_task);
#endif
#else
   clm_rx_int(NULL);
#endif
   clm_tx_int();
   /* See if minimum runtimes need to be modified based on
      current fax class and acu state */
   UpdateMinRunTime();
   MT_ReleaseLock_clm_bg();
}

/****************************************************************************
/ Function:  clm_configure
/
/ Remarks:   Configures load-time modem parameters
/
/ Inputs;    none
/
/ Outputs:   none
*****************************************************************************/
void clm_configure(void)
{
#if defined(DEBUG_LINUX)
   printk(KERN_INFO"clm_configure\n");
#endif
   init_eeprom_data();
   strcpy(mdm_mfg_id, MDM_MFG_ID);
   strcpy(mdm_model_name, MDM_MODEL_NAME);
}

/****************************************************************************
/ Function:  clm_initialize
/
/ Remarks:   Initializes modem operation
/
/ Inputs;    none
/
/ Outputs:   none
*****************************************************************************/
void clm_initialize(void)
{
#if defined(DEBUG_LINUX)
   printk(KERN_DEBUG"clm_initialize:\n");
#endif
   Init_WWH(0xB5); //USA
   Read_ProductStrings();
   Read_EEPROM();
   Read_CountryList();
   Read_CurrentCountry();

   RTS_Set_Timeout(10);

   AFE_type = 0;
   mt_v90_configured = TRUE;

   acu_loop = FALSE;
   at_z_command = FALSE;

   RTS_Enable();
#if defined(DEBUG_LINUX)
   printk(KERN_INFO"RTS timer started\n");
#endif

//   UART_init(115200,8,0,1);

   dte_to_clear();
   dte_from_clear();
   acu_to_clear();
   acu_from_clear();

   dte_to_modem_xon_char = XON_CHAR;
   dte_to_modem_xoff_char = XOFF_CHAR;

   //LINUX - enable DCE flow control by default
   slG = 1;

   //LINUX - disable DTE flow control X-0N/X-OFF
   slQ = 1;


   acu_io_enable();
   UART_dtr_on();
   UART_rts_on();
   acu_init_ok = TRUE;  // Allows RTS to call ACU

}

/****************************************************************************
/ Function:  clm_terminate
/
/ Remarks:   Terminates modem operation
/
/ Inputs;    none
/
/ Outputs:   none
*****************************************************************************/
void clm_terminate(void)
{
#if defined(DEBUG_LINUX)
   printk(KERN_DEBUG"ham:clm_terminate:\n");
#endif

   /* ensure that RTS_Task has completed before stopping RTS */
   RTS_Task_Terminate();
   RTS_Disable();

   UART_dtr_off();
   UART_rts_off();

   UART_dce_flush();
   UART_dte_flush();

   dte_to_clear();
   dte_from_clear();
   acu_to_clear();
   acu_from_clear();
   acu_io_disable();
   acu_loop = FALSE;
   acu_init_ok = FALSE;

#if defined(TARGET_SELAH)
        IM_DSP_OFF();
#else
        /* Reset the DSP but don't download patches */
        ResetDspInternal();
#endif


   dspdrv_clear_dsp_interrupt();
   dspdrv_SetCramISRCallBack(NULL);

#if defined(DEBUG_LINUX)
   printk("ham:clm_terminate: completed\n");
#endif
}


/****************************************************************************
/ Function:  clm_tx_int
/
/ Remarks:   Tx interrupt emulation
/
/ Inputs;    none
/
/ Outputs:   none
*****************************************************************************/
void clm_tx_int(void)
{
   if (acu_enabled)
      acu_tx_int();
   else
      dte_tx_int();
}

/****************************************************************************
/ Function:  clm_rx_int
/
/ Remarks:   Rx interrupt emulation
/
/ Inputs;    none
/
/ Outputs:   none
*****************************************************************************/
extern void schedule_background_event(void);

void clm_rx_int(void *ptr)
{
   extern asmlinkage void dte_check_rx_resume(void *);
   ubyte  ch;
   uword  free;
   ubyte  wrote_something = FALSE;

   /* NOTE: clm_rx_int() is not reentrant and can be called recursively in */
   /* certain situations.  Simply, exit clm_rx_int() if already running    */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,17)
   if(down_trylock(&clm_rx_int_sem)) return; //non-blockingly attemtp to down the sem.
#else
   if (!atomic_add_unless(&clm_rx_int_sem,1,1)) return;
#endif
   if (acu_loop)
   {
      // if DTE flowed off UART layer and there is now room, flow on.  This
      // ensures that we don't miss sending data if there is room for it.
      dte_check_rx_resume("clm_rx_int");

      /* move all data possible from UART to L2 */
      while (UART_dce_to_num() > 0)
      {
         if (acu_enabled)
            free = acu_from_free();
         else
            free = dte_from_free();

         /* if buffers are full, exit */
         if (free == 0)
         {
            break;
         }

         ch = UART_dce_from_char();
         wrote_something = TRUE;

         if (acu_enabled)
         {
            acu_rx_char(ch);
         }
         else if (fax_dte_enabled)
         {
            fax_rx_char(ch);
         }
         else if (voice_dte_enabled)
         {
            voice_dte_rx_char(ch);
         }
         else
         {
            dte_rx_char(ch);
         }
      }
      if (wrote_something && (UART_dce_to_num() < WAKEUP_CHARS))
          schedule_background_event();
   }

   if (ext_intr_pending)
   {
      ext_intr_pending = FALSE;

      if (acu_enabled)
         acu_external_int();
      else
         dte_external_int();
   }
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,17)
   up(&clm_rx_int_sem);
#else
   atomic_set(&clm_rx_int_sem,0);
#endif
}

/****************************************************************************
/ Function:  clm_external_int
/
/ Remarks:   External interrupt emulation
/
/ Inputs;    none
/
/ Outputs:   none
*****************************************************************************/
void clm_external_int(void)
{
   ext_intr_pending = TRUE;
}

/****************************************************************************
/ Function:  UpdateMinRunTime
/
/ Remarks:
/
/ Inputs;    none
/
/ Outputs:   none
*****************************************************************************/
void UpdateMinRunTime(void)
{
   switch (fax_class)
   {
   case DATA_CLASS_MODE:
      UpdateDataRunTime();
      break;

   case FAX_CLASS_MODE:
      UpdateFaxRunTime();
      break;

   case VOICE_CLASS_MODE:
   case VVIEW_CLASS_MODE:
      RTS_Set_Timeout(RTS_VOICE_TIMEOUT);
      RSATaskSetMinRunTime(RUN_ONE_TIME_ONLY, TASKER_VOICE_MODE);
      break;

   default:
      break;
   }
}

/****************************************************************************
/ Function:  UpdateDataRunTime
/
/ Remarks:
/
/ Inputs;    none
/
/ Outputs:   none
*****************************************************************************/
void UpdateDataRunTime(void)
{
   if (last_acu_state != acu_state)
   {
      last_acu_state = acu_state;

#ifdef DEBUG_LINUX
      printk(KERN_DEBUG"ham:UpdateDataRunTime: ACU State=%d\n", acu_state);
#endif

      switch (acu_state)
      {
      /* AT Command parse mode needs a big runtime boost */
      case S_PARSE:
         RTS_Set_Timeout(RTS_ACU_PARSE_TIMEOUT);
         RSATaskSetMinRunTime(AT_PARSE_RUN_TIME, TASKER_AT_PARSING);

#ifdef DEBUG_LINUX
         printk(KERN_DEBUG"ham:UpdateDataRunTime:(Parsing)\n");
#endif
         break;

      /* Online data mode */
      case S_ONLINE:
         RTS_Set_Timeout(RTS_ACU_ONLINE_TIMEOUT);
         RSATaskSetMinRunTime(RUN_ONE_TIME_ONLY, TASKER_ONLINE_MODE);

#ifdef DEBUG_LINUX
            printk(KERN_DEBUG"ham:UpdateDataRunTime:(ONLINE)\n");
#endif
         break;

      /* IDLE time should use minimal CPU time. */
      case S_IDLE:
      case S_ORIG:
      case S_ANSW:
      case S_HANGUP:
      case S_TEST:
      case S_RETRAIN:
      default:
         RTS_Set_Timeout(RTS_DEFAULT_TIMEOUT);
         RSATaskSetMinRunTime(RUN_ONE_TIME_ONLY, TASKER_IDLE_MODE);

#ifdef DEBUG_LINUX
         printk(KERN_DEBUG"ham:UpdateDataRunTime:(Idle or Default)\n");
#endif
         break;
      }
   }
}

/****************************************************************************
/ Function:  UpdateFaxRunTime
/
/ Remarks:
/
/ Inputs;    none
/
/ Outputs:   none
*****************************************************************************/
void UpdateFaxRunTime(void)
{
   /* One shot this code so we don't repeat ourselves. */
   if (last_fax1_state != fax1_state)
   {
      /* Stash it */
      last_fax1_state = fax1_state;

      switch (fax1_state)
      {
      case IDLE:
         /*****************************************************************/
         /* We have just transitioned out ofCOMPLETE mode or the ACU has  */
         /* justenabled FAX Class 1 mode. Once we're complete then        */
         /* focus should shift to the APP thus we don't need tobe run as  */
         /* often until the next AT cmd.                                  */
         /*****************************************************************/
         RTS_Set_Timeout(RTS_FAX_IDLE_TIMEOUT);

         /*****************************************************************/
         /* We need a few extra cycles here to handlesuch things as       */
         /* building a "OK" msg event which is handled in following       */
         /* thread executions. We want to make sure that those            */
         /* executions are done ASAP so that the APP gets its info ASAP.  */
         /*****************************************************************/
         RSATaskSetMinRunTime(IDLE_RUN_TIME, TASKER_FAX_IDLE);
         break;

      case COMPLETE_FRM:
      case COMPLETE_FTH:
      case COMPLETE_FRH:
      case COMPLETE_FTM:
         /*****************************************************************/
         /*We have just transitioned out of EXECUTE mode.  Basically      */
         /* we're here waiting for TX data to empty from the DSP or are   */
         /* waiting for the Rxdata to be flushed upto the APP.            */
         /* Additionally, in Rx HDLC mode, we're waiting for CXR to go    */
         /* off thus sequencing the RX -> TX transitionswith respect to   */
         /* the remote end finishing itsTX.                               */
         /*****************************************************************/
         RTS_Set_Timeout(RTS_FAX_COMPLETE_TIMEOUT);

         /*****************************************************************/
         /*We really only need to check if all the workhas completed so   */
         /* just a quick poll will do. Thepremise here is that its OK to  */
         /* YIELD when enteringthe COMPLETE mode.                         */
         /*****************************************************************/
         RSATaskSetMinRunTime(RUN_ONE_TIME_ONLY, TASKER_FAX_COMPLETE);
         break;

      case EXECUTE_FRH:
      case EXECUTE_FTM:
      case EXECUTE_FRM:
      case EXECUTE_FTH:
         /*****************************************************************/
         /* We have just transitioned out of a CONFIGUREmode.             */
         /* The execute modes are insensitive to the frequencyof RTS      */
         /* callbacks because, on each execution ofclm_bg(), all          */
         /* collected data (or as much as possible)is delivered to the    */
         /* host or the DSP.                                              */
         /*****************************************************************/
         RTS_Set_Timeout(RTS_FAX_EXECUTE_TIMEOUT);

         /*****************************************************************/
         /* The first thing the EXECUTE modes do is checks for data.      */
         /* Therefore, before the firstexecution of an EXECUTE mode, we   */
         /* can affordto yield because no data has built up.              */
         /*****************************************************************/
         RSATaskSetMinRunTime(RUN_ONE_TIME_ONLY, TASKER_FAX_EXECUTE);
         break;

      case CONFIGURE_FRH:
      case CONFIGURE_FTH:
      case CONFIGURE_FTM:
      case CONFIGURE_FRM:
         /*****************************************************************/
         /* In CONFIGURE states we are relatively sensitive to getting    */
         /* cycles, however, the minimum run time should prevent us from  */
         /* yielding, making the timeout value less important.            */
         /*****************************************************************/
         RTS_Set_Timeout(RTS_FAX_CONFIGURE_TIMEOUT);

         /*****************************************************************/
         /* We have just transitioned into a CONFIGURE state and must     */
         /* configure the line expeditiously.  Therefore, we don't want   */
         /* to yield until we have made it through all of the CONFIGURE   */
         /* substates.  We don't have to worry about setting the minimum  */
         /* run time too high because, after all CONFIGURE substates have */
         /* executed, we will end up back in this case statement.         */
         /* Fax_state will have a different value andthe minimum run      */
         /* time will be set appropriately.                               */
         /*****************************************************************/
         RSATaskSetMinRunTime(FAX_PRIORITY_RUN_TIME, TASKER_FAX_CONFIGURE);
         break;
      }
   }
}
