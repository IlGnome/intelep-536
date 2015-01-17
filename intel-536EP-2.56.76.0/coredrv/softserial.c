/*****************************************************************************
 Copyright (c) 2004, Intel Corporation 

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



#include"softcore.h"
#include"hamcore.h"

#include"softserial.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,10) && \
    LINUX_VERSION_CODE < KERNEL_VERSION(2,6,15)
   MODULE_LICENSE("Proprietary");
#else
   MODULE_LICENSE("GPL");
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,21)
#define pci_find_device pci_get_device
#endif

struct global G;

extern int softcore_init_struct(struct softcore_struct*);

//=============================================================================
extern void core_init_module(void);
static int __init init_536(void)
{
   int rc = 0;
#if defined (DEBUG_LINUX)
   printk("softserial:init_module()\n");
#endif
   core_init_module();

   rc = softcore_init_struct(&G.softcore);  //get the softcore structure
		   
   if(rc) return(-1);
#if defined (DEBUG_LINUX)
   printk("softserial: venid %x, dev %x\n",G.softcore.pci.ven_id,
		              G.softcore.pci.dev_id);
#endif
   G.refcount = 0;

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,5,0) 
   if(pci_present())
#endif
   {

      G.softcore.pci.pdev = NULL;
      G.softcore.pci.pdev = pci_find_device(G.softcore.pci.ven_id,
                                            G.softcore.pci.dev_id,
                                            G.softcore.pci.pdev);
      if(!G.softcore.pci.pdev)
      {
         return(-ENODEV);
      }
   }

   rc = softserial_register_tty();

   if(rc) return(rc);
#if defined (DEBUG_LINUX)
   printk("softserial: init done\n");
#endif
return (0);
}

//=============================================================================
struct ktermios* softserial_termios[SOFTSERIAL_NUMBER_OF_PORTS];
struct ktermios* softserial_termios_locked[SOFTSERIAL_NUMBER_OF_PORTS];
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,5,0) 
static struct tty_struct* softserial_ptty_table[SOFTSERIAL_NUMBER_OF_PORTS];
static int             softserial_refcount;
#endif

int softserial_register_tty(void)
{
   int rc;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
   static struct tty_operations ops;

   memset(&ops,0,sizeof(struct tty_operations));
#endif
   memset(&G.softserial_tty_driver, 0, sizeof(struct tty_driver));

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,28)
   kref_init(&G.softserial_tty_driver.kref);
#endif
   G.softserial_tty_driver.driver_name     = SOFTSERIAL_DRIVER_NAME;
   G.softserial_tty_driver.name            = SOFTSERIAL_DEVICE_NAME;
   G.softserial_tty_driver.major           = SOFTSERIAL_MAJOR_NUMBER;
   G.softserial_tty_driver.minor_start     = SOFTSERIAL_MINOR_NUMBER_START;
   G.softserial_tty_driver.num             = SOFTSERIAL_NUMBER_OF_PORTS;
   G.softserial_tty_driver.type            = TTY_DRIVER_TYPE_SERIAL;
   G.softserial_tty_driver.subtype         = SERIAL_TYPE_NORMAL;
   G.softserial_tty_driver.flags           = TTY_DRIVER_REAL_RAW;
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,5,0) 
   G.softserial_tty_driver.refcount        = &softserial_refcount;
   G.softserial_tty_driver.table           = softserial_ptty_table;
#endif
   G.softserial_tty_driver.termios         = softserial_termios;
   G.softserial_tty_driver.termios_locked  = softserial_termios_locked;
   G.softserial_tty_driver.init_termios    = tty_std_termios;
   //TODO verify unknown flags
   //all already set except clocal, dont know what clocal stands for
   G.softserial_tty_driver.init_termios.c_cflag = B115200 |\
                                                  CS8    |\
                                                  CREAD  |\
                                                  HUPCL  |\
                                                  CLOCAL;
   G.softserial_tty_driver.magic           = TTY_DRIVER_MAGIC;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26)
#define ops G.softserial_tty_driver
#endif
   ops.open            = softserial_open;
   ops.close           = softserial_close;
   ops.write           = softserial_write;
   ops.put_char        = softserial_put_char;
   ops.flush_chars     = softserial_flush_chars;
   ops.write_room      = softserial_write_room;
   ops.chars_in_buffer = softserial_chars_in_buffer;
   ops.flush_buffer    = softserial_flush_buffer;
   ops.ioctl           = softserial_ioctl;
   ops.throttle        = softserial_throttle;
   ops.unthrottle      = softserial_unthrottle;
   ops.send_xchar      = softserial_send_xchar;
   ops.set_termios     = softserial_set_termios;
   ops.stop            = softserial_stop;
   ops.start           = softserial_start;
   ops.hangup          = softserial_hangup;
   ops.break_ctl       = softserial_break;
   ops.wait_until_sent = softserial_wait_until_sent;
   ops.set_ldisc       = softserial_set_ldisc;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
   G.softserial_tty_driver.ops=&ops;
#endif
   rc = tty_register_driver(&(G.softserial_tty_driver));
   if(rc)
   {
      printk("softserial_register_tty() cant register tty,rc=%x\n",rc);
      return(rc);//TODO error code correct?);
   }
#if defined (DEBUG_LINUX)
   printk("softserial_register_tty(): tty registered\n");
#endif
   return(0);
}


//=============================================================================
extern void core_cleanup_module(void);
static void __exit cleanup_536(void)
{
#if defined (DEBUG_LINUX)
   printk("softserial:exit_module()\n");
#endif
   tty_unregister_driver(&G.softserial_tty_driver);
   core_cleanup_module();
}


//=============================================================================
static struct async_struct softserial_async;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,5,0) && \
LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20) 
DECLARE_WORK(softserial_bg_event_work,softserial_background_event_handler,
             &softserial_async);
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,20) 
DECLARE_WORK(softserial_bg_event_work,
             (work_func_t)softserial_background_event_handler);
#endif
static struct serial_state state = {0,BASE_BAUD,0,0,ASYNC_SPD_VHI | ASYNC_SKIP_TEST};
int softserial_open(struct tty_struct* ptty, struct file* pfile)
{
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,5,0) 
int line;
#endif
#if defined (DEBUG_LINUX)
   printk("softserial:softserial_open()\n");
   printk("softserial_open:caller %s[%d]\n", current->comm, current->pid);
#endif
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,5,0) 
   if(MOD_IN_USE)
   {
      MOD_INC_USE_COUNT;
#if defined (DEBUG_LINUX)
      printk("softserial_open:device already opened, count now %d\n", MOD_IN_USE);
#endif
      return(0);
   }
   MOD_INC_USE_COUNT;
   line = MINOR(ptty->device) - ptty->driver.minor_start;

   if(line != 0)  //only one softmodem device supported
   {
      printk("softserial_open: bad minor number %d, no such device, count is %d\n",line,MOD_IN_USE);
      return(-ENODEV);
   }
   state.normal_termios = tty_std_termios;
   state.normal_termios.c_cflag = B115200 |\
                                  CS8    |\
                                  CREAD  |\
                                  HUPCL  |\
                                  CLOCAL;

#else // > KERNEL_VERSION(2,5,0)
   if(G.refcount++) return 0;

#endif
   state.xmit_fifo_size = PAGE_SIZE; //TODO GFP_KERNEL==4096?; //xmit fifo==write buffer? i hope
   
   softserial_async.magic = SOFTSERIAL_MAJIK;
   softserial_async.line = 0;
   softserial_async.tty= ptty;
   softserial_async.state = &state;
   softserial_async.port = state.port;
   softserial_async.flags = state.flags;
   softserial_async.xmit_fifo_size = state.xmit_fifo_size;
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,5,0) 
   softserial_async.tqueue.routine = softserial_background_event_handler;
   softserial_async.tqueue.data = &softserial_async;
   softserial_async.session = current->session;
   softserial_async.pgrp = current->pgrp;
#else // > KERNEL_VERSION(2,5,0)  
   //todo: figure how to use the work member of the async struct.
   //the macro for workqueues did not seem to like the ".work" argument  
   softserial_async.work = softserial_bg_event_work;
#endif

   ptty->driver_data = &softserial_async;
   ptty->low_latency = 0;
   *(ptty->termios) = tty_std_termios;
   ptty->termios->c_cflag = B115200 |\
                            CS8    |\
                            CREAD  |\
                            HUPCL  |\
                            CLOCAL;

   G.softcore.tx_size=PAGE_SIZE;
   G.softcore.tx_fifo=(unsigned char*) get_zeroed_page(GFP_KERNEL);

   G.softcore.ptty = ptty;

   if(G.softcore.open() != 0)
   {
      //error occured
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,5,0) 
      MOD_DEC_USE_COUNT;
#else
      G.refcount--;
#endif
      return(-EFAULT);
   };

    return(0);   
}

//=============================================================================
void softserial_close(struct tty_struct* ptty, struct file* pfile)
{

  struct async_struct* s = ptty->driver_data;


#if defined (DEBUG_LINUX)
   printk("softserial:softserial_close()\n");
   printk("softserial_close:caller %s[%d]\n", current->comm, current->pid);
#endif
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,5,0) 
   if(MOD_IN_USE > 1)
   {
      MOD_DEC_USE_COUNT;
#if defined (DEBUG_LINUX)
      printk("softserial_close:count now %d\n",MOD_IN_USE);
#endif
      return;
   }
   else if(MOD_IN_USE == 0)
   {
#if defined (DEBUG_LINUX)
      printk("softserial_close: already closed\n");
#endif
      return;
   }
   MOD_DEC_USE_COUNT;  //if count was 1
#if defined (DEBUG_LINUX)
   printk("softserial_close: count now %d\n", MOD_IN_USE);
#endif
#else // > KERNEL_VERSION(2,5,0)
    if(G.refcount) G.refcount--; //if was opened, decrement
    else return;                 //if not just return

    if(G.refcount) return;  //if not last one open, just return.
#endif

   G.softcore.close();

   if(G.softcore.tx_fifo)
   {
      free_page((unsigned long)G.softcore.tx_fifo);
      G.softcore.tx_fifo = NULL;
   }

   s->tty = NULL; //removed the softserial_async reference
}

//=============================================================================
void softserial_background_event_handler(void* pvoid)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,20)
   struct async_struct* p_async_struct = &softserial_async;
#else
   struct async_struct* p_async_struct = (struct async_struct*) pvoid;
#endif
   struct tty_struct* ptty;

#if defined (DEBUG_LINUX)
   printk("softserial:softserial_backgound_event_handler()\n");
#endif

   if(p_async_struct->tty == NULL) return;

   ptty = p_async_struct->tty;

   if(test_and_clear_bit(RS_EVENT_WRITE_WAKEUP, &p_async_struct->event))
   {
      if( (ptty->flags & (1 << TTY_DO_WRITE_WAKEUP)) &&
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,26)
          (ptty->ldisc.ops->write_wakeup != NULL))
#else
          (ptty->ldisc.write_wakeup != NULL))
#endif
      {
#if defined (DEBUG_LINUX)
	 printk("ss_bg_eh:ldisc wakeup\n");
#endif
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,26)
         (ptty->ldisc.ops->write_wakeup)(ptty);
#else
         (ptty->ldisc.write_wakeup)(ptty);
#endif
      }
#if defined (DEBUG_LINUX)
      printk("ss_bg_eh: wakeup write_wait\n");
#endif
      wake_up_interruptible(&ptty->write_wait);
   }
}

//=============================================================================
void softserial_interrupt(int irq,
                          void* device_id,
                          struct pt_regs* pt_registers_ptr)
{
#if defined (DEBUG_LINUX)
   printk("softserial:softserial_interrupt()\n");
#endif
}


//=============================================================================
void softserial_stop(struct tty_struct* ptty)
{
#if defined (DEBUG_LINUX)
   printk("softserial:softserial_stop()\n");
#endif
   G.softcore.stop();
}

//=============================================================================
void softserial_start(struct tty_struct* ptty)
{
#if defined (DEBUG_LINUX)
   printk("softserial:softserial_start()\n");
#endif
   G.softcore.start();
}


//=============================================================================
void softserial_hangup(struct tty_struct* ptty)
{
#if defined (DEBUG_LINUX)
   printk("softserial:softserial_hangup()\n");
#endif
   G.softcore.hangup(); //ath the modem
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
module_init(init_536);
module_exit(cleanup_536);
#endif

