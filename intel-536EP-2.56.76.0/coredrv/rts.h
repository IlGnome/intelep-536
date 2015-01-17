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

#include "hamdefs.h"

/* Function prototypes for exported routines */
extern void  RTS_Enable(void);
asmlinkage extern void  RTS_Set_Timeout(ulong NewTimeout);
extern ulong RTS_Get_Ticks(void);
extern void  RTS_Disable(void);
extern ulong RTS_System_Time(void);
extern void  RTS_Task_Terminate(void);
extern ubyte RTS_ISR_Schedule_Callback (void);


/* RTS Callback Frequency Constants */
#define RTS_FREQ_HIGHEST_THROUGHPUT 10
#define RTS_FREQ_HIGH_THROUGHPUT    20
#define RTS_FREQ_MEDIUM_THROUGHPUT  30
#define RTS_THROUGHPUT_TIMEOUT      RTS_FREQ_HIGHEST_THROUGHPUT

#define RTS_DEFAULT_TIMEOUT         10
#define RTS_ACU_TIMEOUT             10
#define RTS_FAX_TIMEOUT             10

#define RTS_ACU_IDLE_TIMEOUT        50      /* ACU States */
#define RTS_ACU_PARSE_TIMEOUT       10
#define RTS_ACU_ORIG_TIMEOUT        30
#define RTS_ACU_ANSW_TIMEOUT        30
#define RTS_ACU_ONLINE_TIMEOUT      RTS_THROUGHPUT_TIMEOUT
#define RTS_ACU_HANGUP_TIMEOUT      30
#define RTS_ACU_TEST_TIMEOUT        30
#define RTS_ACU_RETRAIN_TIMEOUT     30

#define RTS_FAX_IDLE_TIMEOUT        10      /* Fax states */
#define RTS_FAX_COMPLETE_TIMEOUT    10
#define RTS_FAX_EXECUTE_TIMEOUT     10
#define RTS_FAX_CONFIGURE_TIMEOUT   10

#define RTS_DISABLE_TIMEOUT         0 
#define RTS_VOICE_TIMEOUT           15


/* The async timer is the frequency that Windows will call
   our Async Timer callback. */
#if defined(LINUX) || defined(TARGET_NT)
   #define RTS_ASYNC_TIMER   10  /* 10 ms */
#else
   #define RTS_ASYNC_TIMER    5  /* 5 ms */
#endif

/* This is the interval that RSA's ACU code's acu_timer() routine
   expects to be called. */
#define ACU_EXPECTED_UPDATE_INTERVAL    10      /* 10 ms */

/* Value for RTS functions that allow a pointer to reference data */
#define RTS_NO_REFERENCE_DATA   0

/* Index constants for timers */
#define RTS_ASYNC_TIMER_INDEX 0
#define RTS_BG_EVENT_INDEX    1
#define RTS_MAX_EVENT         2


