//LINUX file
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
#include"hamdefs.h"

/* Constants */
#define DELTA_CTS    0x01
#define DELTA_DSR    0x02
#define DELTA_RING   0x04
#define DELTA_RLSD   0x08

#define UART_DTE_BUFFER_SIZE    4096   /* 4K default buffer */
#define UART_DTE_NEAR_EMPTY     (UART_DTE_BUFFER_SIZE * 1/8)
#define UART_DTE_NEAR_FULL      (UART_DTE_BUFFER_SIZE * 7/8)

#define FAX_DTE_NEAR_EMPTY      DTE_NEAR_EMPTY
#define FAX_DTE_NEAR_FULL       DTE_NEAR_FULL

/*** UART DTE buffer variables ***/

/* data from terminal */
extern unsigned char UART_dte_rx_buffer[UART_DTE_BUFFER_SIZE];
extern unsigned int UART_dte_rx_remove;   /* buffer remove pointer */
extern unsigned int UART_dte_rx_insert;   /* buffer insert pointer */
extern unsigned int UART_dte_rx_count;    /* # of chars in buffer */

/* data to terminal */ 
extern unsigned char UART_dte_tx_buffer[UART_DTE_BUFFER_SIZE];
extern unsigned int UART_dte_tx_remove;    /* buffer remove pointer */
extern unsigned int UART_dte_tx_insert;    /* buffer insert pointer */
extern unsigned int UART_dte_tx_count;     /* # of chars in buffer */

extern ubyte acu_enabled;
extern ubyte fax_dte_enabled;
extern ubyte voice_dte_enabled;

#define CLEARED         0
#define ASSERTED        1
#define RTS_ASSERTED    1
#define RTS_CLEARED     0
#define DTR_ASSERTED    1
#define DTR_CLEARED     0

/* Constants used in UART_line_status() */
#define DSR                 0
#define DTR                 1
#define RTS                 2
#define CTS                 3
#define RLSD                4
#define RINGING             5
#define PC_TO_MODEM_BREAK   6
#define MODEM_TO_PC_BREAK   7

/* Structure to keep track of hardware lines and settings for the UART */
typedef struct uart_struct
{
  ubyte data_bits;
  ubyte stop_bits;
  ulong baud_rate;
  ubyte parity;

  ubyte dtr;
  ubyte dsr;
  ubyte rts;
  ubyte cts;
  ubyte rlsd;
  ubyte ring;

  ubyte pc_to_modem_break;
  ubyte modem_to_pc_break;

} UART;


/* UART setup functions */
extern void  UART_init(int, char, char, char );
asmlinkage extern unsigned int UART_set_baud(unsigned long);
asmlinkage extern unsigned long UART_get_baud(void);
asmlinkage extern unsigned int UART_data_bits(char);
asmlinkage extern unsigned int UART_get_data_bits(void);
asmlinkage extern void  UART_stop_bits(char);
asmlinkage extern void  UART_parity(char);
asmlinkage extern ubyte UART_get_parity(void);

/* PC hardware lines */
extern void  UART_dtr_on(void);
extern void  UART_dtr_off(void);
extern void  UART_rts_on(void);
extern void  UART_rts_off(void);

/* Modem hardware lines */
asmlinkage extern void  UART_cts_on(void);
asmlinkage extern void  UART_cts_off(void);
asmlinkage extern void  UART_dsr_on(void);
asmlinkage extern void  UART_dsr_off(void);
asmlinkage extern void  UART_ring_on(void);
asmlinkage extern void  UART_ring_off(void);
asmlinkage extern void  UART_rlsd_on(void);
asmlinkage extern void  UART_rlsd_off(void);
asmlinkage extern ubyte UART_line_status (ubyte);

/* Break functions */
asmlinkage extern void  UART_start_dce_break(void);
asmlinkage extern void  UART_stop_dce_break(void);
asmlinkage extern void  UART_start_dte_break(void);
asmlinkage extern void  UART_stop_dte_break(void);

/* Data transmit / receive functions */
extern ubyte UART_queue_transmit (ubyte ch);
extern int   UART_dce_to_char(unsigned char);
asmlinkage extern int   UART_dte_to_char(unsigned char);
extern unsigned char UART_dte_from_char(void);
extern unsigned char UART_dce_from_char(void);

asmlinkage extern unsigned int UART_dce_to_num(void);
extern unsigned int UART_dte_to_num(void);
asmlinkage extern unsigned int UART_dce_from_num(void);
asmlinkage extern unsigned int UART_dte_from_num(void);
extern unsigned int UART_dce_to_free(void);
asmlinkage extern unsigned int UART_dte_to_free(void);
asmlinkage extern void  UART_dce_flush(void);
asmlinkage extern void  UART_dte_flush(void);

asmlinkage extern ubyte UART_dte_rx_resume (void);
asmlinkage extern ubyte UART_dte_rx_suspend (void);
extern unsigned char UART_dce_peek_char(void);

/* Other functions */
asmlinkage extern void UART_timer(uword);
extern void UART_msr_wait(void);

/* Character transfer / 'interrupt' functions */
asmlinkage extern ubyte acu_rx_char(ubyte rx_char);
asmlinkage extern ubyte dte_rx_char(ubyte rx_char);
asmlinkage extern ubyte fax_rx_char(ubyte rx_char);
asmlinkage extern ubyte voice_dte_rx_char(ubyte rx_char);

