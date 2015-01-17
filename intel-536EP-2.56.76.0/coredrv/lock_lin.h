// LINUX file
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

#if !defined(_LOCK_LIN_H_)
#define _LOCK_LIN_H_

#define CLMPORT_MaskIRQ()
#define CLMPORT_UnMaskIRQ()

#define RSAResourceFree                   1
#define RSAResourceInUse                  0
asmlinkage extern void linux_RSAGetExclusive(int Resource, unsigned char *Indicator);
asmlinkage extern void linux_RSAFreeExclusive(int Resource);

enum MT_semaphores
{
   B0InUse,               //corecode
   B1InUse,               //corecode
   B2InUse,               //corecode
   B3InUse,               //corecode
   acu_tx_running,        //corecode
   DceRxDataState,        //corecode
   DceTxDataState,        //corecode
   clm_rx_int_running,    //corecode/windows
   ICD_InUse,             //corecode    !linux
   semiphoreDspCommRam,   //linux
   clm_bg_running,        //windows/linux
   async_handle_access,   //windows
   async_callback_running //windows
 };

#if(1)
   #define RSAGetExclusive(Resource,Indicator) linux_RSAGetExclusive(Resource,&Indicator);
   #define RSAFreeExclusive(Resource)          linux_RSAFreeExclusive(Resource);
#else
   #define RSAGetExclusive(Resource,Indicator) \
     Debug_Printf("RSAGetExclusive(%d,%d) %s(%d)\n",Resource, Indicator,__FILE__,__LINE__); \
     linux_RSAGetExclusive(Resource,&Indicator);
   #define RSAFreeExclusive(Resource) \
     Debug_Printf("RSAFreeExclusive(%d) %s(%d)\n",Resource,__FILE__,__LINE__); \
     linux_RSAFreeExclusive(Resource);
#endif

#define MT_LocalsLock_LAPMtx_que
#define MT_LocalsLock_LAPMrx_que
#define MT_LocalsLock_LAPMemptied_que
#define MT_LocalsLock_PortQIn
#define MT_LocalsLock_PortQOut
#define MT_LocalsLock_dte_rx_buffer
#define MT_LocalsLock_dte_tx_buffer
#define MT_LocalsLock_acu_rx_buffer
#define MT_LocalsLock_acu_tx_buffer
#define MT_LocalsLock_dce_rx_buffer
#define MT_LocalsLock_dce_tx_buffer
#define MT_LocalsLock_received_que
#define MT_LocalsLock_emptied_que
#define MT_LocalsLock_transmit_que
#define LINE_INTERRUPT_IN_USE

#ifndef LOCK_DEFINED 
extern void linux_MT_AcquireLock_LAPMrx_que(void);
extern void linux_MT_ReleaseLock_LAPMrx_que(void);
extern void linux_MT_AcquireLock_LAPMtx_que(void);
extern void linux_MT_ReleaseLock_LAPMtx_que(void);
extern void linux_MT_AcquireLock_LAPMemptied_que(void);
extern void linux_MT_ReleaseLock_LAPMemptied_que(void);
extern void linux_MT_AcquireLock_PortQIn(void);
extern void linux_MT_ReleaseLock_PortQIn(void);
extern void linux_MT_AcquireLock_PortQOut(void);
extern void linux_MT_ReleaseLock_PortQOut(void);
extern void linux_MT_AcquireLock_dte_rx_buffer(void);
extern void linux_MT_ReleaseLock_dte_rx_buffer(void);
extern void linux_MT_AcquireLock_dte_tx_buffer(void);
extern void linux_MT_ReleaseLock_dte_tx_buffer(void);
extern void linux_MT_AcquireLock_acu_rx_buffer(void);
extern void linux_MT_ReleaseLock_acu_rx_buffer(void);
extern void linux_MT_AcquireLock_acu_tx_buffer(void);
extern void linux_MT_ReleaseLock_acu_tx_buffer(void);
extern void linux_MT_AcquireLock_dce_rx_buffer(void);
extern void linux_MT_ReleaseLock_dce_rx_buffer(void);
extern void linux_MT_AcquireLock_dce_tx_buffer(void);
extern void linux_MT_ReleaseLock_dce_tx_buffer(void);
extern void linux_MT_AcquireLock_received_que(void);
extern void linux_MT_ReleaseLock_received_que(void);
extern void linux_MT_AcquireLock_emptied_que(void);
extern void linux_MT_ReleaseLock_emptied_que(void);
extern void linux_MT_AcquireLock_transmit_que(void);
extern void linux_MT_ReleaseLock_transmit_que(void);
extern void linux_MT_AcquireLock_clm_bg(void);
extern void linux_MT_ReleaseLock_clm_bg(void);
extern void linux_MT_AcquireLock_timer_task(void);
extern void linux_MT_ReleaseLock_timer_task(void);
extern void linux_MT_AcquireLock_mt_dsp(void);
extern void linux_MT_ReleaseLock_mt_dsp(void);
extern void linux_AcquireLock_data_to_user(void);
extern void linux_ReleaseLock_data_to_user(void);
extern void linux_AcquireLock_rts1(void);
extern void linux_ReleaseLock_rts1(void);
#endif

#if(1)
#define  MT_AcquireLock_mt_dsp()          linux_MT_AcquireLock_mt_dsp();
#define  MT_ReleaseLock_mt_dsp()          linux_MT_ReleaseLock_mt_dsp();
#define  MT_AcquireLock_clm_bg()          linux_MT_AcquireLock_clm_bg();
#define  MT_ReleaseLock_clm_bg()          linux_MT_ReleaseLock_clm_bg();
#define  MT_AcquireLock_timer_task()      linux_MT_AcquireLock_timer_task();
#define  MT_ReleaseLock_timer_task()      linux_MT_ReleaseLock_timer_task();
#define  MT_AcquireLock_LAPMrx_que()      linux_MT_AcquireLock_LAPMrx_que();
#define  MT_ReleaseLock_LAPMrx_que()      linux_MT_ReleaseLock_LAPMrx_que();
#define  MT_AcquireLock_LAPMtx_que()      linux_MT_AcquireLock_LAPMtx_que();
#define  MT_ReleaseLock_LAPMtx_que()      linux_MT_ReleaseLock_LAPMtx_que();
#define  MT_AcquireLock_LAPMemptied_que() linux_MT_AcquireLock_LAPMemptied_que();
#define  MT_ReleaseLock_LAPMemptied_que() linux_MT_ReleaseLock_LAPMemptied_que();
#define  MT_AcquireLock_PortQIn()         linux_MT_AcquireLock_PortQIn();
#define  MT_ReleaseLock_PortQIn()         linux_MT_ReleaseLock_PortQIn();
#define  MT_AcquireLock_PortQOut()        linux_MT_AcquireLock_PortQOut();
#define  MT_ReleaseLock_PortQOut()        linux_MT_ReleaseLock_PortQOut();
#define  MT_AcquireLock_dte_rx_buffer()   linux_MT_AcquireLock_dte_rx_buffer();
#define  MT_ReleaseLock_dte_rx_buffer()   linux_MT_ReleaseLock_dte_rx_buffer();
#define  MT_AcquireLock_dte_tx_buffer()   linux_MT_AcquireLock_dte_tx_buffer();
#define  MT_ReleaseLock_dte_tx_buffer()   linux_MT_ReleaseLock_dte_tx_buffer();
#define  MT_AcquireLock_acu_rx_buffer()   linux_MT_AcquireLock_acu_rx_buffer();
#define  MT_ReleaseLock_acu_rx_buffer()   linux_MT_ReleaseLock_acu_rx_buffer();
#define  MT_AcquireLock_acu_tx_buffer()   linux_MT_AcquireLock_acu_tx_buffer();
#define  MT_ReleaseLock_acu_tx_buffer()   linux_MT_ReleaseLock_acu_tx_buffer();
#define  MT_AcquireLock_dce_rx_buffer()   linux_MT_AcquireLock_dce_rx_buffer();
#define  MT_ReleaseLock_dce_rx_buffer()   linux_MT_ReleaseLock_dce_rx_buffer();
#define  MT_AcquireLock_dce_tx_buffer()   linux_MT_AcquireLock_dce_tx_buffer();
#define  MT_ReleaseLock_dce_tx_buffer()   linux_MT_ReleaseLock_dce_tx_buffer();
#define  MT_AcquireLock_received_que()    linux_MT_AcquireLock_received_que();
#define  MT_ReleaseLock_received_que()    linux_MT_ReleaseLock_received_que();
#define  MT_AcquireLock_emptied_que()     linux_MT_AcquireLock_emptied_que();
#define  MT_ReleaseLock_emptied_que()     linux_MT_ReleaseLock_emptied_que();
#define  MT_AcquireLock_transmit_que()    linux_MT_AcquireLock_transmit_que
#define  MT_ReleaseLock_transmit_que()    linux_MT_ReleaseLock_transmit_que();

#else
#define  MT_AcquireLock_clm_bg()         linux_MT_AcquireLock_clm_bg();
#define  MT_ReleaseLock_clm_bg()         linux_MT_ReleaseLock_clm_bg();
#define  MT_AcquireLock_timer_task()     linux_MT_AcquireLock_timer_task();
#define  MT_ReleaseLock_timer_task()     linux_MT_ReleaseLock_timer_task();

#define  MT_AcquireLock_LAPMrx_que() \
   Debug_Printf("MT_AcquireLock_LAPMrx_que %s(%d)\n",__FILE__,__LINE__); \
   linux_MT_AcquireLock_LAPMrx_que();

#define  MT_ReleaseLock_LAPMrx_que() \
   Debug_Printf("MT_ReleaseLock_LAPMrx_que %s(%d)\n",__FILE__,__LINE__); \
   linux_MT_ReleaseLock_LAPMrx_que();

#define  MT_AcquireLock_LAPMtx_que() \
   Debug_Printf("MT_AcquireLock_LAPMtx_que %s(%d)\n",__FILE__,__LINE__); \
   linux_MT_AcquireLock_LAPMtx_que();

#define  MT_ReleaseLock_LAPMtx_que() \
   Debug_Printf("MT_ReleaseLock_LAPMtx_que %s(%d)\n",__FILE__,__LINE__); \
   linux_MT_ReleaseLock_LAPMtx_que();

#define  MT_AcquireLock_LAPMemptied_que() \
   Debug_Printf("MT_AcquireLock_LAPMemptied_que %s(%d)\n",__FILE__,__LINE__); \
   linux_MT_AcquireLock_LAPMemptied_que();

#define  MT_ReleaseLock_LAPMemptied_que() \
   Debug_Printf("MT_ReleaseLock_LAPMemptied_que %s(%d)\n",__FILE__,__LINE__); \
   linux_MT_ReleaseLock_LAPMemptied_que();

#define  MT_AcquireLock_PortQIn() \
   Debug_Printf("MT_AcquireLock_PortQIn %s(%d)\n",__FILE__,__LINE__); \
   linux_MT_AcquireLock_PortQIn();

#define  MT_ReleaseLock_PortQIn() \
   Debug_Printf("MT_ReleaseLock_PortQIn %s(%d)\n",__FILE__,__LINE__); \
   linux_MT_ReleaseLock_PortQIn();

#define  MT_AcquireLock_PortQOut() \
   Debug_Printf("MT_AcquireLock_PortQOut %s(%d)\n",__FILE__,__LINE__); \
   linux_MT_AcquireLock_PortQOut();

#define  MT_ReleaseLock_PortQOut() \
   Debug_Printf("MT_ReleaseLock_PortQOut %s(%d)\n",__FILE__,__LINE__); \
   linux_MT_ReleaseLock_PortQOut();

#define  MT_AcquireLock_dte_rx_buffer() \
   Debug_Printf("MT_AcquireLock_dte_rx_buffer %s(%d)\n",__FILE__,__LINE__); \
   linux_MT_AcquireLock_dte_rx_buffer();

#define  MT_ReleaseLock_dte_rx_buffer() \
   Debug_Printf("MT_ReleaseLock_dte_rx_buffer %s(%d)\n",__FILE__,__LINE__); \
   linux_MT_ReleaseLock_dte_rx_buffer();

#define  MT_AcquireLock_dte_tx_buffer() \
   Debug_Printf("MT_AcquireLock_dte_tx_buffer %s(%d)\n",__FILE__,__LINE__); \
   linux_MT_AcquireLock_dte_tx_buffer();

#define  MT_ReleaseLock_dte_tx_buffer() \
   Debug_Printf("MT_ReleaseLock_dte_tx_buffer %s(%d)\n",__FILE__,__LINE__); \
   linux_MT_ReleaseLock_dte_tx_buffer();


#define  MT_AcquireLock_acu_rx_buffer() linux_MT_AcquireLock_acu_rx_buffer();
#define  MT_ReleaseLock_acu_rx_buffer() linux_MT_ReleaseLock_acu_rx_buffer();
#define  MT_AcquireLock_acu_tx_buffer() linux_MT_AcquireLock_acu_tx_buffer();
#define  MT_ReleaseLock_acu_tx_buffer() linux_MT_ReleaseLock_acu_tx_buffer();
/*
#define  MT_AcquireLock_acu_rx_buffer() \
   Debug_Printf("MT_AcquireLock_acu_rx_buffer %s(%d)\n",__FILE__,__LINE__); \
   linux_MT_AcquireLock_acu_rx_buffer();

#define  MT_ReleaseLock_acu_rx_buffer() \
   Debug_Printf("MT_ReleaseLock_acu_rx_buffer %s(%d)\n",__FILE__,__LINE__); \
   linux_MT_ReleaseLock_acu_rx_buffer();

#define  MT_AcquireLock_acu_tx_buffer() \
   Debug_Printf("MT_AcquireLock_acu_tx_buffer %s(%d)\n",__FILE__,__LINE__); \
   linux_MT_AcquireLock_acu_tx_buffer();

#define  MT_ReleaseLock_acu_tx_buffer() \
   Debug_Printf("MT_ReleaseLock_acu_tx_buffer %s(%d)\n",__FILE__,__LINE__); \
   linux_MT_ReleaseLock_acu_tx_buffer();
*/

#define  MT_AcquireLock_dce_rx_buffer() \
   Debug_Printf("MT_AcquireLock_dce_rx_buffer %s(%d)\n",__FILE__,__LINE__); \
   linux_MT_AcquireLock_dce_rx_buffer();

#define  MT_ReleaseLock_dce_rx_buffer() \
   Debug_Printf("MT_ReleaseLock_dce_rx_buffer %s(%d)\n",__FILE__,__LINE__); \
   linux_MT_ReleaseLock_dce_rx_buffer();

#define  MT_AcquireLock_dce_tx_buffer() \
   Debug_Printf("MT_AcquireLock_dce_tx_buffer %s(%d)\n",__FILE__,__LINE__); \
   linux_MT_AcquireLock_dce_tx_buffer();

#define  MT_ReleaseLock_dce_tx_buffer() \
   Debug_Printf("MT_ReleaseLock_dce_tx_buffer %s(%d)\n",__FILE__,__LINE__); \
   linux_MT_ReleaseLock_dce_tx_buffer();

#define  MT_AcquireLock_received_que() \
   Debug_Printf("MT_AcquireLock_received_que %s(%d)\n",__FILE__,__LINE__); \
   linux_MT_AcquireLock_received_que();

#define  MT_ReleaseLock_received_que() \
   Debug_Printf("MT_ReleaseLock_received_que %s(%d)\n",__FILE__,__LINE__); \
   linux_MT_ReleaseLock_received_que();

#define  MT_AcquireLock_emptied_que() \
   Debug_Printf("MT_AcquireLock_emptied_que %s(%d)\n",__FILE__,__LINE__); \
   linux_MT_AcquireLock_emptied_que();

#define  MT_ReleaseLock_emptied_que() \
   Debug_Printf("MT_ReleaseLock_emptied_que %s(%d)\n",__FILE__,__LINE__); \
   linux_MT_ReleaseLock_emptied_que();

#define  MT_AcquireLock_transmit_que() \
   Debug_Printf("MT_AcquireLock_transmit_que %s(%d)\n",__FILE__,__LINE__); \
   linux_MT_AcquireLock_transmit_que

#define  MT_ReleaseLock_transmit_que() \
   Debug_Printf("MT_ReleaseLock_transmit_que %s(%d)\n",__FILE__,__LINE__); \
   linux_MT_ReleaseLock_transmit_que();

#endif




#define IM_LOCK(x)
#define IM_UNLOCK(x)
#define IM_LOCK_DSP_LOCAL(x)
#define IM_LOCK_DSP(x)
#define IM_UNLOCK_DSP(x)

#endif      // defined _LINLOCK_H_
