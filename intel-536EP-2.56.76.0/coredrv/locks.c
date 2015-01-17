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

#include"hamcore.h"

#include<linux/kernel.h>
#include<linux/sched.h> 
#include<asm/current.h>
#include<linux/smp.h>
#include<linux/interrupt.h>
#include<linux/irq.h>
#include<linux/spinlock.h>
#if  LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
#include <linux/semaphore.h>
#else
#include <asm/semaphore.h>
#endif
#define LOCK_DEFINED
#include"lock_lin.h"

#if !defined(CONFIG_SMP)
  #define xspinlock_t         spinlock_t
  #define xSPIN_LOCK_UNLOCKED SPIN_LOCK_UNLOCKED
  #define xspin_lock(x)       spin_lock(x)
  #define xspin_unlock(x)     spin_unlock(x)
  #define xspin_lock_bh(x)    spin_lock_bh(x)
  #define xspin_unlock_bh(x)  spin_unlock_bh(x)
  #define xspin_trylock(x)    spin_trylock(x)
#else
  #define xspinlock_t          spinlock_t
  #define xSPIN_LOCK_UNLOCKED  SPIN_LOCK_UNLOCKED
  #define xspin_lock(x)        spin_lock(x)
  #define xspin_unlock(x)      spin_unlock(x)
  #define xspin_lock_bh(x)     spin_lock_bh(x)
  #define xspin_unlock_bh(x)   spin_unlock_bh(x)
  #define xspin_trylock(x)     spin_trylock(x)
#endif

 xspinlock_t Lock_clm_bg             = xSPIN_LOCK_UNLOCKED;
 xspinlock_t Lock_timer_task         = xSPIN_LOCK_UNLOCKED;
 xspinlock_t Lock_LAPMtx_que         = xSPIN_LOCK_UNLOCKED;
 xspinlock_t Lock_LAPMrx_que         = xSPIN_LOCK_UNLOCKED;
 xspinlock_t Lock_LAPMemptied_que    = xSPIN_LOCK_UNLOCKED; 
 xspinlock_t Lock_PortQIn            = xSPIN_LOCK_UNLOCKED;
 xspinlock_t Lock_PortQOut           = xSPIN_LOCK_UNLOCKED;
 xspinlock_t Lock_dte_rx_buffer      = xSPIN_LOCK_UNLOCKED;
 xspinlock_t Lock_dte_tx_buffer      = xSPIN_LOCK_UNLOCKED;
 xspinlock_t Lock_acu_rx_buffer      = xSPIN_LOCK_UNLOCKED;
 xspinlock_t Lock_acu_tx_buffer      = xSPIN_LOCK_UNLOCKED;
 xspinlock_t Lock_dce_rx_buffer      = xSPIN_LOCK_UNLOCKED;
 xspinlock_t Lock_dce_tx_buffer      = xSPIN_LOCK_UNLOCKED;
 xspinlock_t Lock_received_que       = xSPIN_LOCK_UNLOCKED;
 xspinlock_t Lock_emptied_que        = xSPIN_LOCK_UNLOCKED;
 xspinlock_t Lock_transmit_que       = xSPIN_LOCK_UNLOCKED;
 xspinlock_t Lock_DiagnosticCallback = xSPIN_LOCK_UNLOCKED;
 xspinlock_t Lock_mt_dsp             = xSPIN_LOCK_UNLOCKED;
 xspinlock_t Lock_data_to_user       = xSPIN_LOCK_UNLOCKED;
 xspinlock_t Lock_rts1               = xSPIN_LOCK_UNLOCKED;

//linux code spinlocks
asmlinkage void linux_MT_AcquireLock_timer_task(void) {xspin_lock_bh(&Lock_timer_task);}
asmlinkage void linux_MT_ReleaseLock_timer_task(void) {xspin_unlock_bh(&Lock_timer_task);}
asmlinkage void linux_MT_AcquireLock_PortQIn(void){xspin_lock(&Lock_PortQIn);}   
asmlinkage void linux_MT_ReleaseLock_PortQIn(void){xspin_unlock(&Lock_PortQIn);} 
asmlinkage void linux_MT_AcquireLock_PortQOut(void){xspin_lock(&Lock_PortQOut);}
asmlinkage void linux_MT_ReleaseLock_PortQOut(void){xspin_unlock(&Lock_PortQOut);}

//dpc disabling corecode spinlocks
asmlinkage void linux_MT_AcquireLock_LAPMrx_que(void)     { xspin_lock_bh  (&Lock_LAPMrx_que);     }
asmlinkage void linux_MT_ReleaseLock_LAPMrx_que(void)     { xspin_unlock_bh(&Lock_LAPMrx_que);     } 
asmlinkage void linux_MT_AcquireLock_LAPMtx_que(void)     { xspin_lock_bh  (&Lock_LAPMtx_que);     }
asmlinkage void linux_MT_ReleaseLock_LAPMtx_que(void)     { xspin_unlock_bh(&Lock_LAPMtx_que);     }         
asmlinkage void linux_MT_AcquireLock_LAPMemptied_que(void)
   {xspin_lock_bh(&Lock_LAPMemptied_que);}
asmlinkage void linux_MT_ReleaseLock_LAPMemptied_que(void)
   {xspin_unlock_bh(&Lock_LAPMemptied_que);}
asmlinkage void linux_MT_AcquireLock_dce_rx_buffer(void){xspin_lock_bh(&Lock_dce_rx_buffer);}
asmlinkage void linux_MT_ReleaseLock_dce_rx_buffer(void){xspin_unlock_bh(&Lock_dce_rx_buffer);}
asmlinkage void linux_MT_ReleaseLock_dce_tx_buffer(void){xspin_unlock_bh(&Lock_dce_tx_buffer);}
asmlinkage void linux_MT_AcquireLock_dce_tx_buffer(void){xspin_lock_bh(&Lock_dce_tx_buffer);}

//537
asmlinkage void linux_MT_AcquireLock_mt_dsp(void)  {xspin_lock_bh(&Lock_mt_dsp);}
asmlinkage void linux_MT_ReleaseLock_mt_dsp(void)  {xspin_unlock_bh(&Lock_mt_dsp);}
asmlinkage void linux_AcquireLock_rts1(void)  {xspin_lock_bh(&Lock_rts1);}
asmlinkage void linux_ReleaseLock_rts1(void)  {xspin_unlock_bh(&Lock_rts1);}
asmlinkage void linux_AcquireLock_data_to_user(void)  {xspin_lock_bh(&Lock_data_to_user);}
asmlinkage void linux_ReleaseLock_data_to_user(void)  {xspin_unlock_bh(&Lock_data_to_user);}

//corecode spinlocks
asmlinkage void linux_MT_AcquireLock_dte_rx_buffer(void)  { xspin_lock  (&Lock_dte_rx_buffer);  }
asmlinkage void linux_MT_ReleaseLock_dte_rx_buffer(void)  { xspin_unlock(&Lock_dte_rx_buffer);  }
asmlinkage void linux_MT_AcquireLock_dte_tx_buffer(void)  { xspin_lock  (&Lock_dte_tx_buffer);  }
asmlinkage void linux_MT_ReleaseLock_dte_tx_buffer(void)  { xspin_unlock(&Lock_dte_tx_buffer);  }
asmlinkage void linux_MT_AcquireLock_acu_rx_buffer(void)  { xspin_lock  (&Lock_acu_rx_buffer);  }
asmlinkage void linux_MT_ReleaseLock_acu_rx_buffer(void)  { xspin_unlock(&Lock_acu_rx_buffer);  }
asmlinkage void linux_MT_AcquireLock_acu_tx_buffer(void)  { xspin_lock  (&Lock_acu_tx_buffer);  }
asmlinkage void linux_MT_ReleaseLock_acu_tx_buffer(void)  { xspin_unlock(&Lock_acu_tx_buffer);  }
asmlinkage void linux_MT_AcquireLock_received_que(void)   { xspin_lock  (&Lock_received_que);   }
asmlinkage void linux_MT_ReleaseLock_received_que(void)   { xspin_unlock(&Lock_received_que);   }
asmlinkage void linux_MT_AcquireLock_emptied_que(void)    { xspin_lock  (&Lock_emptied_que);    }
asmlinkage void linux_MT_ReleaseLock_emptied_que(void)    { xspin_unlock(&Lock_emptied_que);    }
asmlinkage void linux_MT_AcquireLock_clm_bg(void)         { xspin_lock (&Lock_clm_bg); }
asmlinkage void linux_MT_ReleaseLock_clm_bg(void)         { xspin_unlock (&Lock_clm_bg); }

asmlinkage void linux_MT_AcquireLock_transmit_que(void)   {
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,5,0) 
	xspin_lock  (&Lock_transmit_que);
#else
	;
#endif
}
asmlinkage void linux_MT_ReleaseLock_transmit_que(void)   { 
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,5,0) 
	xspin_unlock(&Lock_transmit_que);   
#else
	;
#endif
} 

DECLARE_MUTEX(sem0);
DECLARE_MUTEX(sem1);
DECLARE_MUTEX(sem2);
DECLARE_MUTEX(sem3);
DECLARE_MUTEX(sem4);
DECLARE_MUTEX(sem5);
DECLARE_MUTEX(sem6);
DECLARE_MUTEX(sem7);
DECLARE_MUTEX(sem8);

asmlinkage void linux_RSAGetExclusive(int Resource, unsigned char *Indicator) 
{
   *Indicator = 0;
   switch(Resource)
   {
   case B0InUse:                //corecode
      if(down_trylock(&sem0)) return;
      *Indicator = 1;
      return;
   case B1InUse:                //corecode
      if(down_trylock(&sem1)) return;
      *Indicator = 1;
      return;
   case B2InUse:                //corecode
      if(down_trylock(&sem2)) return;
      *Indicator = 1;
      return;
   case B3InUse:                //corecode
      if(down_trylock(&sem3)) return;
      *Indicator = 1;
      return;
   case acu_tx_running:         //corecode
      if(down_trylock(&sem4)) return;
      *Indicator = 1;
      return;
   case DceRxDataState:         //corecode
      if(down_trylock(&sem5)) return;
      *Indicator = 1;
      return;
   case DceTxDataState:         //corecode
      if(down_trylock(&sem6)) return;
      *Indicator = 1;
      return;
   case semiphoreDspCommRam:    //linux
      if(down_trylock(&sem7)) return;
      *Indicator = 1;
      return;
   case ICD_InUse:              //corecode
      if(down_trylock(&sem8)) return;
      *Indicator = 1;
      return;
   case clm_rx_int_running:     //corecode/windows
   case clm_bg_running:         //windows/linux
   case async_handle_access:    //windows
   case async_callback_running: //windows
   default:
        printk("linux_RSAGetExclusive error %d\n", Resource);
   }
   return;
}

asmlinkage void linux_RSAFreeExclusive(int Resource)
{
   switch(Resource)
   {
   case B0InUse:                //corecode
      up(&sem0);
      return;
   case B1InUse:                //corecode
      up(&sem1);
      return;
   case B2InUse:                //corecode
      up(&sem2);
       return;
   case B3InUse:                //corecode
      up(&sem3);
      return;
   case acu_tx_running:         //corecode
      up(&sem4);
      return;
   case DceRxDataState:         //corecode
      up(&sem5);
      return;
   case DceTxDataState:         //corecode
      up(&sem6);
      return;
   case semiphoreDspCommRam:    //linux
      up(&sem7);
      return;
   case ICD_InUse:              //corecode
      up(&sem8);
      return;
   case clm_rx_int_running:     //corecode/windows
   case clm_bg_running:         //windows/linux
   case async_handle_access:    //windows
   case async_callback_running: //windows
   default:
        printk("linux_RSAFreeExclusive error %d\n", Resource);
   }
   return;
}
