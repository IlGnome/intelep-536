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

June 2002- August 2002 Modified for use with Linux kernel 2.6
                         by Philippe Vouters (Philippe.Vouters@laposte.net)
****************************************************************************/
#include "hamcore.h"
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/pci.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial.h>
#include <linux/serialP.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <asm/system.h>
#include <asm/irq.h>
#include <asm/uaccess.h>

#undef CONFIG_PM

#ifdef CONFIG_PM
#include <linux/pm.h>
#endif

#include "uart.h"
#include "rts.h"
#include "lock_lin.h"

/////////////////////////////////////////////////////////////////////
//////////////////////////// softcore changes ///////////////////////
/////////////////////////////////////////////////////////////////////
asmlinkage int (*Debug_Printf)(const char*, ...)
         __attribute__ ((format (printf, 1, 2)));

#if !defined (TARGET_CATAWBA)
extern PUCHAR       pBaseAddress;
#endif
extern asmlinkage void (*CRAM_CallBack)(void);
unsigned int*      dsp_irq;


#include"softcore.h"
int softcore_init_struct(struct softcore_struct*);
//#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,5,0)
EXPORT_SYMBOL(softcore_init_struct);
//#endif
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,21)
#define pci_find_device pci_get_device
#endif
int open(void);
void close(void);
void stop(void);
void start(void);
int  write(int);
void dtr_on(void);
void rts_on(void);
void hangup(void);
void put_char(unsigned char);
void dtr_off(void);
void rts_off(void);
void msr_wait(void);
void send_xchar(unsigned char);
int  write_free(void);
void write_flush(void);
int  write_pending(void);
int  get_line_status(unsigned char);
unsigned int set_char(int, char, char, char);
void get_char(int *, char *, char *, char *);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)) && (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)) 
 int interrupt_handler(int irq, void *dev_id, struct pt_regs *regs);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,19)
 irqreturn_t interrupt_handler(int irq, void *dev_id);
#else
 void interrupt_handler(int irq, void *dev_id, struct pt_regs *regs);
#endif

void send_data_to_user(void);//rs_recieve
extern volatile ubyte dce_tx_enabled;

static struct global
{
   struct softcore_struct* softcore;
   int irq_share;
   unsigned int irq;

}G;

#if defined (TARGET_CATAWBA)
int get_pci_info_537(void *data)
{
    // We assume 1 or 0 catawba devices installed. This is stated in requirements.
    struct pci_dev *pdev = NULL;
    struct softcore_struct* driver = (struct softcore_struct*)data;

    // This should be first!!!
    pdev = pci_find_device (TJ320_VENDOR_ID, TJ320_DEVICE_ID, pdev);
    if (pdev)
    {
        printk(KERN_INFO"Intel 537 card found\n");
        if (strstr(VER_PRODUCTNAME_STR, "537 "))
        {
            driver->afe_type = TJ320_VENDOR_ID;
            driver->pci.ven_id = TJ320_VENDOR_ID;
            driver->pci.dev_id = TJ320_DEVICE_ID;
            driver->pci.function_id = 0;
            return 0;
        }
        else
        {
            printk(KERN_ERR"This %s driver won't install on Intel 537 card, please download the right driver\n",
                    VER_PRODUCTNAME_STR);
            return -1;
        }
     }

    pdev = pci_find_device (SI3052_VENDOR_ID, SI3052_DEVICE_ID, pdev);
    if (pdev)
    {
        printk(KERN_INFO"Intel 537EP card found\n");
        if (strstr(VER_PRODUCTNAME_STR, "537EP"))
        {
            driver->afe_type = SI3052_VENDOR_ID + 2;
            driver->pci.ven_id = SI3052_VENDOR_ID;
            driver->pci.dev_id = SI3052_DEVICE_ID;
            driver->pci.function_id = 0;
            return 0;
        }
        else
        {
            printk(KERN_ERR"This %s driver won't install on Intel 537EP card, please download the right driver\n",
                    VER_PRODUCTNAME_STR);
            return -1;
        }
    }
    pdev = pci_find_device (NEW_SI3052_VENDOR_ID, NEW_SI3052_DEVICE_ID, pdev);
    if (pdev)
    {
        printk(KERN_INFO"Intel 537EP card found\n");
        if (strstr(VER_PRODUCTNAME_STR, "537EP"))
        {
            driver->afe_type = TJ320_VENDOR_ID + 1;
            driver->pci.ven_id = NEW_SI3052_VENDOR_ID;
            driver->pci.dev_id = NEW_SI3052_DEVICE_ID;
            driver->pci.function_id = 0;
            return 0;
        }
        else
        {
            printk(KERN_ERR"This %s driver won't install on Intel 537EP card, please download the right driver\n",
                    VER_PRODUCTNAME_STR);
            return -1;
        }
    }
    pdev = pci_find_device (SELAH_PCI_VENDOR_ID, SELAH_PCI_DEVICE_ID, pdev);
    if (pdev)
    {
        printk(KERN_INFO"Intel 537SP card found\n");
        if (strstr(VER_PRODUCTNAME_STR, "537SP"))
        {
            driver->afe_type = SELAH_PCI_VENDOR_ID + 1; // Dirty hack
            driver->pci.ven_id = SELAH_PCI_VENDOR_ID;
            driver->pci.dev_id = SELAH_PCI_DEVICE_ID;
            driver->pci.function_id = 0;
            return 0;
        }
        else
        {
            printk(KERN_ERR"This %s driver won't install on Intel 537SP card, please download the right driver\n",
                    VER_PRODUCTNAME_STR);
            return -1;
        }
    }

    if (!strstr(VER_PRODUCTNAME_STR, "537EA") && !strstr(VER_PRODUCTNAME_STR, "537AA"))
    {
        printk(KERN_ERR"This %s driver won't install on AC97 modem card, please download intel537EA "
                       "driver to check if it supports your modem\n",
                VER_PRODUCTNAME_STR);
        return -1;
    }

    pdev = pci_find_device (ALI_VENDOR_ID, ALI_DEVICE_ID, pdev);
    if (pdev)
    {
        driver->afe_type = ALI_VENDOR_ID;
        driver->pci.ven_id = ALI_VENDOR_ID;
        driver->pci.dev_id = ALI_DEVICE_ID;
        driver->pci.function_id = 0;
        return 0;
    }

    pdev = pci_find_device (ATI_VENDOR_ID, ATI_DEVICE_ID, pdev);
    if (pdev)
    {
        driver->afe_type = ATI_VENDOR_ID;
        driver->pci.ven_id = ATI_VENDOR_ID;
        driver->pci.dev_id = ATI_DEVICE_ID;
        driver->pci.function_id = 0;
        return 0;
    }

    pdev = pci_find_device (INTEL_VENDOR_ID, INTEL_DEVICE_ID, pdev);
    if (pdev)
    {
        driver->afe_type = INTEL_VENDOR_ID;
        driver->pci.ven_id = INTEL_VENDOR_ID;
        driver->pci.dev_id = INTEL_DEVICE_ID;
        driver->pci.function_id = 0;
        return 0;
    }
    pdev = pci_find_device (INTEL_VENDOR_ID, INTEL_ICH4_DEVICE_ID, pdev);
    if (pdev)
    {
        driver->afe_type = INTEL_VENDOR_ID;
        driver->pci.ven_id = INTEL_VENDOR_ID;
        driver->pci.dev_id = INTEL_ICH4_DEVICE_ID;
        driver->pci.function_id = 0;
        return 0;
    }
    pdev = pci_find_device (INTEL_VENDOR_ID, INTEL_ICH5_DEVICE_ID, pdev);
    if (pdev)
    {
        driver->afe_type = INTEL_VENDOR_ID;
        driver->pci.ven_id = INTEL_VENDOR_ID;
        driver->pci.dev_id = INTEL_ICH5_DEVICE_ID;
        driver->pci.function_id = 0;
        return 0;
    }

    pdev = pci_find_device (NVIDIA_VENDOR_ID, NVIDIA_DEVICE_ID, pdev);
    if (pdev)
    {
        driver->afe_type = NVIDIA_VENDOR_ID;
        driver->pci.ven_id = NVIDIA_VENDOR_ID;
        driver->pci.dev_id = NVIDIA_DEVICE_ID;
        driver->pci.function_id = 0;
        return 0;
    }

    pdev = pci_find_device (SIS_VENDOR_ID, SIS_DEVICE_ID, pdev);
    if (pdev)
    {
        driver->afe_type = SIS_VENDOR_ID;
        driver->pci.ven_id = SIS_VENDOR_ID;
        driver->pci.dev_id = SIS_DEVICE_ID;
        driver->pci.function_id = 0;
        return 0;
    }

    pdev = pci_find_device (VIA_VENDOR_ID, VIA_DEVICE_ID, pdev);
    if (pdev)
    {
        driver->afe_type = VIA_VENDOR_ID;
        driver->pci.ven_id = VIA_VENDOR_ID;
        driver->pci.dev_id = VIA_DEVICE_ID;
        driver->pci.function_id = 0;
        return 0;
    }

    printk(KERN_ERR"Cannot find modem h/w supported by this driver\n");
    return -1;
}
#endif
#ifdef CONFIG_PM
static    struct pm_dev *power_dev;
static int power_callback(struct pm_dev *dev, pm_request_t rqst, void *data)
{
    switch(rqst)
    {
        case PM_SUSPEND:
			close();
            break;
        case PM_RESUME:
			open();
            break;
//        case PM_SAVE_STATE:
//            printk(KERN_WARNING"Saving power state is not implemented\n");
//            break;
        default:
            printk(KERN_WARNING"Unknown power mode request to 537 modem driver\n");
    }
    return 0;
}
#endif


int softcore_init_struct(struct softcore_struct* s)
{
  #if defined (TARGET_SELAH)
   s->pci.ven_id = 0x8086;
   s->pci.dev_id = 0x1040;
   s->pci.function_id = 0;
  #elif defined(TARGET_CATAWBA)
   if (get_pci_info_537(s))
       return -1;
  #endif

   s->open              = open;
   s->stop              = stop;
   s->close             = close;
   s->start             = start;
   s->write             = write;
   s->dtr_on            = dtr_on;
   s->rts_on            = rts_on;
   s->hangup            = hangup;
   s->putchar           = put_char;
   s->dtr_off           = dtr_off;
   s->rts_off           = rts_off;
   s->msr_wait          = msr_wait;
   s->send_xchar        = send_xchar;
   s->write_free        = write_free;
   s->write_flush       = write_flush;
   s->write_pending     = write_pending;
   s->get_line_status   = get_line_status;
   s->set_char          = set_char;
   s->get_char          = get_char;
   G.softcore = s;
  #if defined (TARGET_SELAH)
   Debug_Printf = printk;
  #endif

//  printk("softcore: init struct done\n");
  return 0;
}


int open(void)
{
   int retval = 0;
   if(pci_enable_device(G.softcore->pci.pdev))  
   {
      return(-ENODEV);
   }
                   
   G.irq = G.softcore->pci.pdev->irq;
   dsp_irq = &G.irq;
#if !defined (TARGET_CATAWBA)
   {
	   unsigned long   phy_mem_start;
	   unsigned long   phy_mem_len;
	   phy_mem_start = pci_resource_start(G.softcore->pci.pdev,
	                                      G.softcore->pci.function_id);
	   phy_mem_len   = pci_resource_len(G.softcore->pci.pdev,
	                                    G.softcore->pci.function_id);
	   pBaseAddress = (PUCHAR) ioremap(phy_mem_start,
	                                   phy_mem_len);
   }
#else
   G.softcore->mem_base  = pci_resource_start(G.softcore->pci.pdev, 0);
   G.softcore->mem_range = pci_resource_len(G.softcore->pci.pdev, 0);
   G.softcore->io_base  = pci_resource_start(G.softcore->pci.pdev, 1);
   G.softcore->io_range = pci_resource_len(G.softcore->pci.pdev, 1);
   if (0 != modem_init(G.softcore))
       return -1;
#endif

   clm_configure();

   dspdrv_clear_dsp_interrupt();
   dspdrv_SetCramISRCallBack(line_int);

   G.irq_share = 1;
   retval = request_irq(G.irq,
               &interrupt_handler,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,19)
	       IRQF_SHARED,
#else
               SA_SHIRQ,
#endif
               MDM_DEVICE_NAME,
               &G.irq_share);
   
//   printk("softcore:open: irq status %d, irq %d\n",retval,G.irq);
   UART_init(115200,8,0,1);
//   dspdrv_SetCramISRCallBack(line_int);
   
   clm_initialize();
   #if defined(TARGET_CATAWBA)
     ModemCardStart();    
#ifdef CONFIG_PM
{
    power_dev = pm_register(PM_PCI_DEV, PM_PCI_ID(G.softcore->pci.pdev),
                            power_callback);
}
#endif

   #endif
   return(retval);
}

void close(void)
{
   clm_terminate();
  #if defined(TARGET_CATAWBA)
   ModemCardStop();    
  #endif

   free_irq(G.irq, &(G.irq_share));

#if !defined (TARGET_CATAWBA)
    vfree(pBaseAddress);
#endif
#ifdef CONFIG_PM
    pm_unregister(power_dev);
#endif
}


void stop(void)
{
   return;
}

void start(void)
{
   return;
}

int  write(int size)
{
   int i = 0;

   for(i = 0; i < size; i++)
   {
      if(!UART_dce_to_free())
      {
         //full
         return(i);
      }
      UART_dce_to_char(G.softcore->tx_fifo[i]);
   }
   return(size);
}

void dtr_on(void)
{
   UART_dtr_on();
}

void rts_on(void)
{
   UART_rts_on();
}

void dtr_off(void)
{
   UART_dtr_off();
}

void hangup(void)
{
   return;
}

void put_char(unsigned char ch)
{
   UART_dce_to_char(ch);
}

void rts_off(void)
{
   UART_rts_off();
}

void msr_wait(void)
{
   UART_msr_wait();
}

void send_xchar(unsigned char ch)
{
      UART_dce_to_char(ch);
}

int  write_free(void)
{
   return(UART_dce_to_free());
}

void write_flush(void)
{
   UART_dce_flush();
}

int  write_pending(void)
{
   return(UART_dce_to_num());
}

int  get_line_status(unsigned char parameter)
{

   int status = 0;
   switch(parameter)
   {
   default:
   case 0:
      if(UART_line_status(RTS))     status |= TIOCM_RTS;
      if(UART_line_status(DTR))     status |= TIOCM_DTR;
      if(UART_line_status(RINGING)) status |= TIOCM_RNG;
      if(UART_line_status(DSR))     status |= TIOCM_DSR;
      if(UART_line_status(CTS))     status |= TIOCM_CTS;
      if(UART_line_status(RLSD))    status |= TIOCM_CAR;
      return(status);

   case 1:
      return(UART_line_status(RINGING));

   case 2:
      return(UART_line_status(DSR));

   case 3:
      return(UART_line_status(RLSD));

   case 4:
      return(UART_line_status(CTS));
   }
}
unsigned int set_char(int baud_rate,char data_bits, char parity, char stop_bits)
{
   UART_set_baud(baud_rate);
   UART_data_bits(data_bits);
   UART_parity(parity);
   UART_stop_bits(stop_bits);
   dce_stop_bits = stop_bits;
   dte_init();
   return 1;
}
void get_char(int *baud_rate,char *data_bits, char *parity, char *stop_bits)
{
   *baud_rate = UART_get_baud();
   *data_bits = UART_get_data_bits();
   *parity = UART_get_parity();
   *stop_bits = dce_stop_bits;
}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19) 
  int interrupt_handler(int irq, void *dev_id, struct pt_regs *regs)
  {
  #if defined (TARGET_CATAWBA)
    if (AfeIRQ()) return IRQ_NONE;
  #else
    if ((IM_Peek_Reg(CMHISR) & bHIS_MP) == 0) return IRQ_NONE;//not our interrupt
  #endif
    dspdrv_CommRamISR();
    return IRQ_HANDLED;
  }
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,19)
  irqreturn_t interrupt_handler(int irq, void *dev_id)
  {
  #if defined (TARGET_CATAWBA)
    if (AfeIRQ()) return IRQ_NONE;
  #else
    if ((IM_Peek_Reg(CMHISR) & bHIS_MP) == 0) return IRQ_NONE;//not our interrupt
  #endif
    dspdrv_CommRamISR();
    return IRQ_HANDLED;
  }
#else //before 2.5 kernel or after 2.6.19 kernel
  void interrupt_handler(int irq, void *dev_id, struct pt_regs *regs)
  {
  #if defined (TARGET_CATAWBA)
    if (AfeIRQ()) return;
  #else
    if ((IM_Peek_Reg(CMHISR) & bHIS_MP) == 0) return;//not our interrupt
  #endif
    dspdrv_CommRamISR();
  }
#endif
#define TTY_FLIP_BUFFER_SIZE 1000

void send_data_to_user(void)
{
   unsigned char ch;
   char flip_flag=0;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,15)
   unsigned long flags;
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,15)
   int imax=0;
   int i=0;
#endif


   while ((UART_dte_to_num() > 0) 
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,15)
         && (G.softcore->ptty->flip.count < TTY_FLIPBUF_SIZE)
#else
         && (i < TTY_FLIP_BUFFER_SIZE)
#endif
	)
   {
      ch = UART_dte_from_char();
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,15)
      if(!imax)
         imax=tty_buffer_request_room(G.softcore->ptty,TTY_FLIP_BUFFER_SIZE);
      i+=tty_insert_flip_char(G.softcore->ptty, ch, flip_flag);
#else
      spin_lock_irqsave(&(G.softcore->ptty->read_lock), flags);
      *(G.softcore->ptty->flip.char_buf_ptr) = ch;
      *(G.softcore->ptty->flip.flag_buf_ptr) = flip_flag;

      G.softcore->ptty->flip.flag_buf_ptr++;
      G.softcore->ptty->flip.char_buf_ptr++;
      G.softcore->ptty->flip.count++;
      tty_flip_buffer_push(G.softcore->ptty);
      spin_unlock_irqrestore(&(G.softcore->ptty->read_lock), flags);
#endif
   }
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,15)
     if (i){
         tty_flip_buffer_push(G.softcore->ptty);
     }
#endif
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,5,0) 
extern struct work_struct softserial_bg_event_work;
#endif

void schedule_background_event(void)
{
   struct async_struct* async_struct_ptr  = G.softcore->ptty->driver_data;  
   async_struct_ptr->event |= 1 << RS_EVENT_WRITE_WAKEUP;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0) 
   queue_task(&async_struct_ptr->tqueue, &tq_immediate);
   mark_bh(IMMEDIATE_BH);
#else
   schedule_work(&softserial_bg_event_work);
#endif
}

/////////////////////////////////////////////////////////////////////
//////////////////////END /// softcore changes ///////////////////////
/////////////////////////////////////////////////////////////////////


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) 

#if defined(TARGET_CATAWBA)
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,5,0) 
   EXPORT_SYMBOL_NOVERS(ModemCardStart);
   EXPORT_SYMBOL_NOVERS(ModemCardStop);
   EXPORT_SYMBOL_NOVERS(AfeIRQ);
#endif //kernel 2.4
#endif //catawba

#endif //2.4.0

//globals
unsigned int*      io_base;
unsigned int*      io_range;
unsigned int*      mm_range;

//#if defined(TARGET_SELAH)
 // extern PUCHAR       pBaseAddress;
//#else
//  extern PUCHAR       pMappedComram;
//#endif

asmlinkage void   kdelay(short);
asmlinkage void kdisable_irq(unsigned int);
asmlinkage void kenable_irq(unsigned int);
#ifdef KGDB
extern void do_schedule(void);
#endif

   DECLARE_MUTEX(exec_reg_sem);
   DECLARE_WAIT_QUEUE_HEAD(persistWriteQ);
   DECLARE_WAIT_QUEUE_HEAD(persistReadQ);
   DECLARE_WAIT_QUEUE_HEAD(persistShutdownQ);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,10) && \
    LINUX_VERSION_CODE < KERNEL_VERSION(2,6,15)
   MODULE_LICENSE("Proprietary");
#else
   MODULE_LICENSE("GPL");
#endif

extern asmlinkage void ham_proc_shutdown(void);

//extern int ham_proc_function(char*, char**, off_t, int, int);
//extern int ham_proc_recieve(struct file*, const char*, unsigned long, void*); 
//extern read_proc_t hamproc_read;
//extern write_proc_t hamproc_write;
struct proc_dir_entry* hamproc;
extern asmlinkage int persist_hamproc_write(const char*);
extern asmlinkage int persist_hamproc_read(char*);


int hamproc_write(struct file* file, 
                     const char *buffer,
                     unsigned long count,
                     void *data)
{
#if 0
   static char temp[4096];

   if(copy_from_user(temp, buffer, (count > 4096) ? 4096 : count))
      return -EFAULT;
   return(persist_hamproc_write(temp));
#else
#define SIZE 4096
   char *temp;
   char *cp = (char *)buffer;
   unsigned long len = count;
   int rc = 0;

   if ((temp = kmalloc(SIZE,GFP_KERNEL)) == NULL)
        return -ENOMEM;
   while (len > 0){
       if (copy_from_user(temp, cp, (len > SIZE) ? SIZE : len)){
           kfree(temp);
           return -EFAULT;
       }
       cp += (len > SIZE) ? SIZE : len;
       len -= (len > SIZE) ? SIZE : len;
       rc = persist_hamproc_write(temp);
       if (rc < 0){
           kfree(temp);
           return rc;
       }
   }
   kfree(temp);
   return count;
#endif
}
//used to send data to app reading the ham proc
int hamproc_read(char* page,
                  char** start,
                  off_t offset,
                  int len, int* eof, void* data)
{
#if 0
   *eof = 1;
   return(persist_hamproc_read(page));
#else
   if (offset > 0){
       *eof = 1;
       return 0;
   }
   return(persist_hamproc_read(page));
#endif
}
const char *proc_file_name = "ham";

int create_hamproc(void)
{
   hamproc = create_proc_entry(proc_file_name, S_IFREG | S_IRUGO, NULL);
   if(hamproc == NULL) return -ENOMEM;
   hamproc->read_proc = hamproc_read;
   hamproc->write_proc = hamproc_write;
   hamproc->owner = THIS_MODULE;
   return 0;
}
void detach_hamproc(void)
{
   remove_proc_entry(proc_file_name, NULL);
}


//************************************************************************
int core_init_module(void)
{
   int rc;
   rc = create_hamproc();
//   printk(KERN_INFO"hamcore module init\n");
   #if defined (TARGET_CATAWBA)
   sound_init();
   #endif
   return rc;
}

void core_cleanup_module(void)
{
//   printk(KERN_INFO"hamcore module cleanup\n");
   ham_proc_shutdown();
   detach_hamproc();
   #if defined (TARGET_CATAWBA)
      modem_kill();
   #endif
}

asmlinkage void kdelay(short x)
{
//   printk("hamdelay %d\n",x);
   mdelay(x);
}

asmlinkage void kudelay(unsigned long x)
{
   udelay(x);
}

asmlinkage void up_exec_reg_sem(void)
{
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,5,0) 
   up(&exec_reg_sem);
#else
   ;
#endif
}

asmlinkage void down_exec_reg_sem(void)
{
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,5,0) 
   down(&exec_reg_sem);
#else
   ;
#endif
}

static int writeQSem = 0, readQSem = 0;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0) 
void WakeTask(void *foo)
{
   readQSem = 0;
   wake_up_interruptible(&persistReadQ);
}
static int *pkt_to_app1;
static int timeout1;
void WakeTask2(void *foo)
{
   while((*pkt_to_app1 == TRUE) && timeout1 > 0) //while pkt hasnt been read yet.
   {
      timeout1 = wait_event_interruptible_timeout(persistReadQ, 0, timeout1);
   }
}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,20)
DECLARE_WORK(wait_wq2, (work_func_t)WakeTask2);
DECLARE_WORK(wait_wq, (work_func_t)WakeTask);
#else
DECLARE_WORK(wait_wq2, WakeTask2, 0);
DECLARE_WORK(wait_wq, WakeTask, 0);
#endif
#endif

asmlinkage void wake_up_interruptible_persistWriteQ(void)
{
   writeQSem = 0;
   wake_up_interruptible(&persistWriteQ);
}

asmlinkage void wake_up_interruptible_persistReadQ(void)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0) 
   if (in_atomic()||in_softirq()||in_interrupt())
   {
      schedule_work(&wait_wq);
      return;
   }
#endif
   readQSem = 0;
   wake_up_interruptible(&persistReadQ);
}

asmlinkage void interruptible_sleep_on_persistReadQ(void)
{
   readQSem = 1;
   wait_event_interruptible(persistReadQ, readQSem == 0);
}

asmlinkage void interruptible_sleep_on_timeout_persistWriteQ(int* pkt_from_app, unsigned long timeout)
{
   while((*pkt_from_app == FALSE) && timeout > 0) //while we have not gotten ack pkt yet.
   {
      writeQSem = 1;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0) 
      timeout = interruptible_sleep_on_timeout(&persistWriteQ, timeout);
#else
      timeout = wait_event_interruptible_timeout(persistWriteQ, writeQSem == 0, timeout);
#endif
   }
}

asmlinkage void interruptible_sleep_on_timeout_persistReadQ(int* pkt_to_app, unsigned long timeout)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0) 
   if (in_atomic()||in_softirq()||in_interrupt())
   {
      pkt_to_app1 = pkt_to_app;
      timeout1 = timeout;
      schedule_work(&wait_wq2);
      return;
   }
#endif

   while((*pkt_to_app == TRUE) && timeout > 0) //while pkt hasnt been read yet.
   {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0) 
      timeout = interruptible_sleep_on_timeout(&persistReadQ, timeout);
#else
      timeout = wait_event_interruptible_timeout(persistReadQ, 0, timeout);
#endif
   }
}

asmlinkage void ham_proc_shutdown_wait(void)
{
   int timeout = HZ*2;
   while(timeout > 0)
   {
      current->state = TASK_INTERRUPTIBLE;
      timeout = schedule_timeout(timeout);
   }
}

asmlinkage void kdisable_irq(unsigned int irq)
{
#if defined (TARGET_CATAWBA)
#else
   disable_irq(irq);
#endif
}

asmlinkage void kenable_irq(unsigned int irq)
{
#if defined (TARGET_CATAWBA)
#else
   enable_irq(irq);
#endif

}

asmlinkage char* kCurrentComm(void)
{
   return(current->comm);
}

void dpc_function(unsigned long x)
{
   dce_tx_enabled = TRUE; 
   if(CRAM_CallBack != NULL) CRAM_CallBack();
}

DECLARE_TASKLET(dpc_tasklet,dpc_function,0);

asmlinkage void kScheduleDPC(void)
{
#ifdef CONFIG_PM
    pm_access(power_dev);
#endif
   #if defined(TARGET_CATAWBA)
      dpc_function( 0 );
   #else
      tasklet_schedule(&dpc_tasklet);
   #endif
}

//Purpose: 
//DSP interrupt service routine. This ISR clears the DSP Interrupt
//and calls the CommRam interface handler.
extern asmlinkage void dspdrv_CommRamHandler(void);

asmlinkage void dspdrv_CommRamISR(void)
{
//    asmlinkage void kScheduleDPC();
    // clear DSP interrupt
//    #if !defined (TARGET_SELAH)
    dspdrv_clear_dsp_interrupt();
//    #endif
	
    dce_tx_enabled = TRUE;    // Enable TX
    dspdrv_CommRamHandler();
    kScheduleDPC();
}

int IM_atomic_dec(void *t)
{
return 0;
}
int IM_atomic_inc(void *t)
{
return 0;
}
int IM_atomic_set(int *address, int value)
{
return 0;
}
int IM_atomic_get(void *t)
{
return 0;
}


#ifdef CONFIG_PM
/* Power management code goes here*/


#endif

