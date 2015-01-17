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

#include "hamcore.h"
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/version.h>
#include <asm/hardirq.h>
#include <asm/system.h>
#include <asm/atomic.h>
#include "uart.h"
#include "lock_lin.h"


// externals
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
   struct wait_queue* delta_msr_wait = NULL;
#else
   DECLARE_WAIT_QUEUE_HEAD(delta_msr_wait);
#endif

UART   uart;

#define TX_FIFO_SIZE   8192
#define RX_FIFO_SIZE   8192

static unsigned char uart_tx_fifo[TX_FIFO_SIZE];
static unsigned char uart_rx_fifo[RX_FIFO_SIZE];
static atomic_t uart_tx_put_index;
static atomic_t uart_tx_get_index;
static atomic_t uart_rx_put_index;
static atomic_t uart_rx_get_index;
static atomic_t uart_tx_count;
static atomic_t uart_rx_count;
//volatile ubyte dtr_dropped = FALSE;
extern spinlock_t Lock_PortQIn;
extern spinlock_t Lock_PortQOut;


/***************************************************************************
/ Function:  UART_init
/
/ Remarks:   This routine initializes the UART to the default settings.
/
/ Inputs:    none
/
/ Outputs:   none
***************************************************************************/
void UART_init(int baud_rate, char data_bits, char parity, char stop_bits)
{
   /* initialize the UART settings to the power-on defaults */
   uart.data_bits = data_bits;           /* 8 data bits */
   uart.stop_bits = stop_bits;           /* 0 stop bit  */
   uart.parity = parity;              /* No parity   */
   uart.baud_rate = baud_rate;      /* default baud rate */
   uart.pc_to_modem_break = 0;
   uart.modem_to_pc_break = 0;
   uart.ring = 0;
//   dtr_dropped = FALSE;

   UART_dce_flush();
   UART_dte_flush();

   UART_dtr_off();
   UART_rts_off();
   UART_dsr_off();
   UART_cts_off();
   UART_rlsd_off();
}

void UART_msr_wait(void)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
   DEFINE_WAIT(wait);
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
   interruptible_sleep_on(&delta_msr_wait);
#else
   prepare_to_wait(&delta_msr_wait, &wait, TASK_INTERRUPTIBLE);
   schedule();
   finish_wait(&delta_msr_wait, &wait);
//   wait_event_interruptible(delta_msr_wait,dtr_dropped==TRUE);
//   dtr_dropped = FALSE;
#endif
}


/***************************************************************************
/ Function:  UART_line_status
/
/ Remarks:   This routine gets the UART line status register contents.
/
/ Inputs:    line
/
/ Outputs:   none
***************************************************************************/
asmlinkage ubyte UART_line_status(ubyte line)
{
   /* Return status of requested line */
   switch (line)
   {
   case DTR:
      return uart.dtr;
   case DSR:
      return uart.dsr;
   case CTS:
      return uart.cts;
   case RTS:
      return uart.rts;
   case RLSD:
      return uart.rlsd;
   case RINGING:
      return uart.ring;
   case PC_TO_MODEM_BREAK:
      return uart.pc_to_modem_break;
   case MODEM_TO_PC_BREAK:
      return uart.modem_to_pc_break;
   default:
      return (0);
   }
}

/***************************************************************************
/ Function:  UART_dtr_on
/
/ Remarks:   This routine asserts the DTR signal.
/
/ Inputs:    none
/
/ Outputs:   none
***************************************************************************/
void UART_dtr_on(void)
{
   if (uart.dtr == 0)
   {
      #if defined(DEBUG_LINUX_UART)
         printk("Setting DTR\n");
      #endif
      uart.dtr = 1;
   }
}

/***************************************************************************
/ Function:  UART_dtr_off
/
/ Remarks:   This routine deasserts the DTR signal. 
/
/ Inputs:    none
/
/ Outputs:   none
***************************************************************************/
void UART_dtr_off(void)
{
   if (uart.dtr == 1)
   {
      #if defined(DEBUG_LINUX_UART)
        printk("Clearing DTR\n");
      #endif
      uart.dtr = 0;
//      dtr_dropped=TRUE;
      wake_up_interruptible(&delta_msr_wait);
      clm_external_int();
   }
}

/***************************************************************************
/ Function:  UART_rts_on
/
/ Remarks:   This routine asserts the RTS signal.
/
/ Inputs:    none
/
/ Outputs:   none
***************************************************************************/
void UART_rts_on(void)
{
   if (uart.rts == 0)
   {
      #if defined(DEBUG_LINUX_UART)
         printk("Setting RTS\n");
      #endif
      uart.rts = 1;

      /*******************************************************************/
      /* Call the DTE/FAX External Interrupt function to tell the DTE-   */
      /* FAX layer that the status of RTS has changed.                   */
      /*******************************************************************/
      clm_external_int();
   }
}

/***************************************************************************
/ Function:  UART_rts_off
/
/ Remarks:   This routine deasserts the RTS signal.
/
/ Inputs:    none
/
/ Outputs:   none
***************************************************************************/
void UART_rts_off(void)
{
   if (uart.rts == 1)
   {
   #if defined(DEBUG_LINUX_UART)
      printk("Clearing RTS\n");
   #endif
      uart.rts = 0;

      /*******************************************************************/
      /* Call the DTE/FAX External Interrupt function to tell the DTE-   */
      /* FAX layer that the status of RTS has changed.                   */
      /*******************************************************************/
      clm_external_int();
   }
}

/***************************************************************************
/ Function:  UART_dsr_on
/
/ Remarks:   This routine asserts the DTR signal.
/
/ Inputs:    none
/
/ Outputs:   none
***************************************************************************/
asmlinkage void UART_dsr_on(void)
{
   if (uart.dsr == 0)
   {
   #if defined(DEBUG_LINUX_UART)
      printk("Setting DSR\n");
   #endif
      uart.dsr = 1;
      wake_up_interruptible(&delta_msr_wait);
   }
}

/***************************************************************************
/ Function:  UART_dsr_off
/
/ Remarks:   This routine deasserts the DTR signal.
/
/ Inputs:    none
/
/ Outputs:   none
***************************************************************************/
asmlinkage void UART_dsr_off(void)
{
   if (uart.dsr == 1)
   {
      #if defined(DEBUG_LINUX_UART)
         printk("Clearing DSR\n");
      #endif
      uart.dsr = 0;
      wake_up_interruptible(&delta_msr_wait);
   }
}

/***************************************************************************
/ Function:  UART_cts_on
/
/ Remarks:   This routine asserts the CTS signal.
/
/ Inputs:    none
/
/ Outputs:   none
***************************************************************************/
extern void schedule_background_event(void);
asmlinkage void UART_cts_on(void)
{
   if (uart.cts == 0)
   {
      #if defined(DEBUG_LINUX_UART)
         printk("Setting CTS\n");
      #endif
      uart.cts = 1;
      wake_up_interruptible(&delta_msr_wait);
      schedule_background_event();
   }
}

/***************************************************************************
/ Function:  UART_cts_off
/
/ Remarks:   This routine deasserts the CTS signal.
/
/ Inputs:    none
/
/ Outputs:   none
***************************************************************************/
asmlinkage void UART_cts_off(void)
{
   if (uart.cts == 1)
   {
      #if defined(DEBUG_LINUX_UART)
         printk("Clearing CTS\n");
      #endif
      uart.cts = 0;
      wake_up_interruptible(&delta_msr_wait);
   }
}

/***************************************************************************
/ Function:  UART_rlsd_on
/
/ Remarks:   This routine asserts the RLSD signal.
/
/ Inputs:    none
/
/ Outputs:   none
***************************************************************************/
asmlinkage void UART_rlsd_on(void)
{
   if (uart.rlsd == 0)
   {
      #if defined(DEBUG_LINUX_UART)
         printk("RLSD on\n");
      #endif
      uart.rlsd = 1;
      wake_up_interruptible(&delta_msr_wait);
   }
}

/***************************************************************************
/ Function:  UART_rlsd_off
/
/ Remarks:   This routine deasserts the RLSD signal.
/
/ Inputs:    none
/
/ Outputs:   none
***************************************************************************/
asmlinkage void UART_rlsd_off(void)
{
   if (uart.rlsd == 1)
   {
      #if defined(DEBUG_LINUX_UART)
         printk("RLSD off\n");
      #endif
      uart.rlsd = 0;
      wake_up_interruptible(&delta_msr_wait);
   }
}

/***************************************************************************
/ Function:  UART_ring_on
/
/ Remarks:   This routine asserts the RI signal.
/
/ Inputs:    none
/
/ Outputs:   none
***************************************************************************/
asmlinkage void UART_ring_on(void)
{
   if (uart.ring == 0)
   {
      #if defined(DEBUG_LINUX_UART)
         printk("RING on\n");
      #endif
      uart.ring = 1;
      wake_up_interruptible(&delta_msr_wait);
   }
}

/***************************************************************************
/ Function:  UART_ring_off
/
/ Remarks:   This routine deasserts the RI signal.
/
/ Inputs:    none
/
/ Outputs:   none
***************************************************************************/
asmlinkage void UART_ring_off()
{
   if (uart.ring == 1)
   {
      #if defined(DEBUG_LINUX_UART)
         printk("RING off\n");
      #endif
      uart.ring = 0;
      wake_up_interruptible(&delta_msr_wait);
   }
}

/***************************************************************************
/ Function:  UART_start_dte_break
/
/ Remarks:   This routine starts sending a DTE break pattern.
/
/ Inputs:    none
/
/ Outputs:   none
***************************************************************************/
asmlinkage void UART_start_dte_break(void)
{
   #if defined(DEBUG_LINUX_UART)
      printk("uart start dte break\n");
   #endif
   uart.pc_to_modem_break = 1;
}

/***************************************************************************
/ Function:  UART_stop_dte_break
/
/ Remarks:   This routine stops sending a DTE break pattern.
/
/ Inputs:    none
/
/ Outputs:   none
***************************************************************************/
asmlinkage void UART_stop_dte_break(void)
{
   #if defined(DEBUG_LINUX_UART)
      printk("uart stop dte break\n");
   #endif
   uart.pc_to_modem_break = 0;
}

/***************************************************************************
/ Function:  UART_start_dce_break
/
/ Remarks:   This routine starts sending a DCE break pattern.
/
/ Inputs:    none
/
/ Outputs:   none
***************************************************************************/
asmlinkage void UART_start_dce_break(void)
{
   #if defined(DEBUG_LINUX_UART)
      printk("uart start dte break\n");
   #endif
   uart.modem_to_pc_break = 1;
}

/***************************************************************************
/ Function:  UART_stop_dce_break
/
/ Remarks:   This routine stops sending a DCE break pattern.
/
/ Inputs:    none
/
/ Outputs:   none
***************************************************************************/
asmlinkage void UART_stop_dce_break(void)
{
   #if defined(DEBUG_LINUX_UART)
      printk("uart stop dce break\n");
   #endif
   uart.modem_to_pc_break = 0;

   if (UART_dte_to_free() > 0)
   {
      UART_dte_to_char(0);
   }
}

/***************************************************************************
/ Function:  UART_set_baud
/
/ Remarks:   This routine sets the baud rate of the UART.
/
/ Inputs:    baud_rate -
/
/ Outputs:   none
****************************************************************************/
asmlinkage unsigned int UART_set_baud(unsigned long baud_rate)
{
   #if defined(DEBUG_LINUX_UART)
      printk("uart set baud\n");
   #endif
   uart.baud_rate = baud_rate;
   return (TRUE);
}

/***************************************************************************
/ Function:  UART_get_baud
/
/ Remarks:   This routine gets the baud rate of the UART.
/
/ Inputs:    none
/
/ Outputs:   baud_rate -
****************************************************************************/
asmlinkage unsigned long UART_get_baud(void)
{
   #if defined(DEBUG_LINUX_UART)
      printk("uart get buad\n");
   #endif
   return (uart.baud_rate);
}

/***************************************************************************
/ Function:  UART_data_bits
/
/ Remarks:   This routine sets the number of data bits to be used.
/
/ Inputs:    data_bits
/
/ Outputs:   status - TRUE
****************************************************************************/
asmlinkage unsigned int UART_data_bits(char data_bits)
{
   #if defined(DEBUG_LINUX_UART)
      printk("uart data bits\n");
   #endif
   uart.data_bits = data_bits;
   return (TRUE);
}

/***************************************************************************
/ Function:  UART_get_data_bits
/
/ Remarks:   This routine gets the number of data bits being used.
/
/ Inputs:    none
/
/ Outputs:   data_bits             
***************************************************************************/
asmlinkage unsigned int UART_get_data_bits(void)
{
   #if defined(DEBUG_LINUX_UART)
      printk("uart get data bits\n");
   #endif
   return (uart.data_bits);
}

/***************************************************************************
/ Function:  UART_stop_bits
/
/ Remarks:   This routine sets the number of stop bits to be used.
/
/ Inputs:    stop_bits - 0 - One Stop Bit Per Character
/                        1 - 1.5 Stop Bits Per Character
/                        2 - Two Stop Bits Per Character
/
/ Outputs:   none
*******************************************************************************/
asmlinkage void UART_stop_bits(char stop_bits)
{
   #if defined(DEBUG_LINUX_UART)
      printk("uart stop bits\n");
   #endif
   if ((stop_bits >= 0) && (stop_bits <= 2))
      uart.stop_bits = stop_bits;
}

/***************************************************************************
/ Function:  UART_parity
/
/ Remarks:   This routine sets the parity to be used in the UART.
/
/ Inputs:    parity - 0 - 0 - No Parity
/                         1 - Odd Parity
/                         2 - Even parity
/                         3 - Mark Parity
/                         4 - Space Parity
/
/ Outputs:   none
***************************************************************************/
asmlinkage void UART_parity(char parity)
{
   #if defined(DEBUG_LINUX_UART)
      printk("uart parity\n");
   #endif
   if ((parity >= 0) && (parity <= 4))
      uart.parity = parity;
}

/***************************************************************************
/ Function:  UART_get_parity
/
/ Remarks:   This routine gets the parity being used by the UART.
/
/ Inputs:    none
/
/ Outputs:   none
***************************************************************************/
asmlinkage ubyte UART_get_parity(void)
{
   #if defined(DEBUG_LINUX_UART)
      printk("uart get parity\n");
   #endif
   return (uart.parity);
}

/***************************************************************************
/ Function:  UART_dte_to_char
/
/ Remarks:   This routine receives one character from the DTE layer.
/            The character is examined to check for break characters,
/            event characters, and special characters to be optionally
/            filtered.
/
/ Inputs:    none
/
/ Outputs:   none
***************************************************************************/
asmlinkage int UART_dte_to_char(unsigned char ch)
{
   int   rc = 0;
//   unsigned long flags;
   #if defined(DEBUG_LINUX_UART)
      printk("uart dte to char\n");
   #endif
   /* If room in the buffer, receive the character */
   if (UART_dte_to_free() > 0)
   {
      MT_AcquireLock_PortQOut();
//      spin_lock_irqsave(&Lock_PortQOut,flags);
      uart_rx_fifo[atomic_read(&uart_rx_put_index)] = ch;
      atomic_inc(&uart_rx_put_index);
      atomic_set(&uart_rx_put_index,atomic_read(&uart_rx_put_index) % sizeof(uart_rx_fifo));
      atomic_inc(&uart_rx_count);
//      spin_unlock_irqrestore(&Lock_PortQOut,flags);
      MT_ReleaseLock_PortQOut();
      //data written to UART.  just hurry an pass it on the the user buffers
   }
   else
   {
      #if defined(DEBUG_LINUX_UART)
         printk("uart dte is FULL\n");
      #endif
      rc = 1;
   }

   return (rc);
}

/***************************************************************************
/ Function:  UART_dce_to_char
/
/ Remarks:   This routine sends a character to the DCE UART.
/
/ Inputs:    ch
/
/ Outputs:   status
***************************************************************************/
int UART_dce_to_char(ubyte ch)
{
   int   rc = 0;
//   unsigned long flags;
   #if defined(DEBUG_LINUX_UART)
      printk("uart dce to char\n");
   #endif
   /* If room in the buffer, transmit the character */
   if (UART_dce_to_free () > 0)
   {
      MT_AcquireLock_PortQIn();
//      spin_lock_irqsave(&Lock_PortQIn,flags);
      uart_tx_fifo[atomic_read(&uart_tx_put_index)] = ch;
      atomic_inc(&uart_tx_put_index);
      atomic_set(&uart_tx_put_index,atomic_read(&uart_tx_put_index) % sizeof(uart_tx_fifo));
      atomic_inc(&uart_tx_count);
//      spin_unlock_irqrestore(&Lock_PortQIn,flags);
      MT_ReleaseLock_PortQIn();
   }
   else
   {
      #if defined(DEBUG_LINUX_UART)
         printk("uart DCE is FULL\n");
      #endif
      rc = 1;
   }

   return (rc);
}

/***************************************************************************
/ Function:  UART_dce_from_char
/
/ Remarks:   This routine gets a character from the DCE UART.
/
/ Inputs:    none
/
/ Outputs:   ch - receive character
***************************************************************************/
unsigned char UART_dce_from_char(void)
{
   unsigned char ch;
//   unsigned long flags;
   #if defined(DEBUG_LINUX_UART)
      printk("uart dce from char\n");
   #endif

   if (atomic_read(&uart_tx_count) > 0)
   {
      MT_AcquireLock_PortQIn();
//      spin_lock_irqsave(&Lock_PortQIn, flags);
      ch = uart_tx_fifo[atomic_read(&uart_tx_get_index)];
      atomic_inc(&uart_tx_get_index);
      atomic_set(&uart_tx_get_index,atomic_read(&uart_tx_get_index) % sizeof(uart_tx_fifo));
      atomic_dec(&uart_tx_count);
//      spin_unlock_irqrestore(&Lock_PortQIn,flags);
      MT_ReleaseLock_PortQIn();
   }
   else
   {
   #if defined(DEBUG_LINUX_UART)
      printk("uart DCE is EMPTY\n");
   #endif
      ch = 0;
   }

#ifdef DEBUG_TX_CHAR
   if ((ch > ' ') && (ch < 0x7f))
      printk("Tx: %02x  %c\n", ch, ch);
   else
      printk("Tx: %02x\n", ch);
#endif

   return (ch);
}

/***************************************************************************
/ Function:  UART_dte_from_char
/
/ Remarks:   This routine gets a character from the DTE UART.
/
/ Inputs:    none
/
/ Outputs:   ch - transmit character
***************************************************************************/
unsigned char UART_dte_from_char(void)
{
   unsigned char ch;
//   unsigned long flags;
   #if defined(DEBUG_LINUX_UART)
      printk("uart DTE from char\n");
   #endif
   if (atomic_read(&uart_rx_count) > 0)
   {
      MT_AcquireLock_PortQOut();
//      spin_lock_irqsave(&Lock_PortQOut,flags);
      ch = uart_rx_fifo[atomic_read(&uart_rx_get_index)];
      atomic_inc(&uart_rx_get_index);
      atomic_set(&uart_rx_get_index,atomic_read(&uart_rx_get_index) % sizeof(uart_rx_fifo));
      atomic_dec(&uart_rx_count);
//      spin_unlock_irqrestore(&Lock_PortQOut,flags);
      MT_ReleaseLock_PortQOut();
   }
   else
   {
   #if defined(DEBUG_LINUX_UART)
      printk("uart DTE is EMPTY\n");
   #endif
      ch = 0;
   }

#ifdef DEBUG_RX_CHAR
   if ((ch > ' ') && (ch < 0x7f))
      printk("Rx: %02x  %c\n", ch, ch);
   else
      printk("Rx: %02x\n", ch);
#endif

   return (ch);
}

/***************************************************************************
/ Function:  UART_dce_to_num
/
/ Remarks:   This routine gets the number of bytes in the DCE UART.
/
/ Inputs:    none
/
/ Outputs:   none
***************************************************************************/
asmlinkage unsigned int UART_dce_to_num(void)
{
   #if defined(DEBUG_LINUX_UART)
      printk("uart dce 2 num: %d\n", atomic_read(&uart_tx_count));
   #endif  
   return (atomic_read(&uart_tx_count));
}

/***************************************************************************
/ Function:  UART_dce_to_free
/
/ Remarks:   This routine gets the number of free bytes in the DCE UART.
/
/ Inputs:    none
/
/ Outputs:   none
***************************************************************************/
unsigned int UART_dce_to_free(void)
{
   #if defined(DEBUG_LINUX_UART)
      printk("uart dce 2 free: %d\n", sizeof(uart_tx_fifo) - 
                                      atomic_read(&uart_tx_count));
   #endif 
   return (sizeof(uart_tx_fifo) - atomic_read(&uart_tx_count));
}

/***************************************************************************
/ Function:  UART_dce_from_num
/
/ Remarks:   This routine gets the number of bytes in the DCE UART.
/
/ Inputs:    none
/
/ Outputs:   none
***************************************************************************/
asmlinkage unsigned int UART_dce_from_num(void)
{
   #if defined(DEBUG_LINUX_UART)
      printk("uart dce fr num: %d\n", atomic_read(&uart_rx_count));
   #endif   
   return (atomic_read(&uart_rx_count));
}

/***************************************************************************
/ Function:  UART_dte_to_num
/
/ Remarks:   This routine gets the number of bytes in the DTE UART.
/
/ Inputs:    none
/
/ Outputs:   none
***************************************************************************/
unsigned int UART_dte_to_num(void)
{
   #if defined(DEBUG_LINUX_UART)
      printk("uart dte to num: %d\n", atomic_read(&uart_rx_count));
   #endif
   return (atomic_read(&uart_rx_count));
}

/***************************************************************************
/ Function:  UART_dte_to_free
/
/ Remarks:   This routine gets the number of free bytes in DTE UART.
/
/ Inputs:    none
/
/ Outputs:   none
***************************************************************************/
asmlinkage unsigned int UART_dte_to_free()
{
   //free up space before we report free space in the UART DTE Buffer

   #if defined(DEBUG_LINUX_UART)
      printk("uart_dte 2 free: %d\n", sizeof(uart_rx_fifo) - 
                                      atomic_read(&uart_rx_count));
   #endif
   return (sizeof(uart_rx_fifo) - atomic_read(&uart_rx_count));
}

/***************************************************************************
/ Function:  UART_dce_flush
/
/ Remarks:   This routine flushes any characters in the DCE UART.
/
/ Inputs:    none
/
/ Outputs:   none
***************************************************************************/
asmlinkage void UART_dce_flush(void)
{
//   unsigned long flags;
   #if defined(DEBUG_LINUX_UART)
      printk("uart dce flush\n");
   #endif
   MT_AcquireLock_PortQIn();
//   spin_lock_irqsave(&Lock_PortQIn,flags);
   atomic_set(&uart_tx_put_index,0);
   atomic_set(&uart_tx_get_index,0);
   atomic_set(&uart_tx_count,0);
//   spin_unlock_irqrestore(&Lock_PortQIn,flags);
   MT_ReleaseLock_PortQIn();
}

/***************************************************************************
/ Function:  UART_dte_flush
/
/ Remarks:   This routine flushes any characters in the DTE UART.
/
/ Inputs:    none
/
/ Outputs:   none
***************************************************************************/
asmlinkage void UART_dte_flush(void)
{
//   unsigned long flags;
   #if defined(DEBUG_LINUX_UART)
      printk("uart dte flush\n");
   #endif

   MT_AcquireLock_PortQOut();
//   spin_lock_irqsave(&Lock_PortQOut,flags);
   atomic_set(&uart_rx_put_index,0);
   atomic_set(&uart_rx_get_index,0);
   atomic_set(&uart_rx_count,0);
//   spin_unlock_irqrestore(&Lock_PortQOut,flags);
   MT_ReleaseLock_PortQOut();
}

asmlinkage void UART_timer(uword x) { }; //not used in linux

