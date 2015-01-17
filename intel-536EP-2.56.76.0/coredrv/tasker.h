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

/* Define minimum run times for various modes of the RSA code in msecs */
#define RUN_ONE_TIME_ONLY       0

#define INIT_RUN_TIME           2
#define FAX_PRIORITY_RUN_TIME   10
#define FAX_RUN_TIME            4
#define DATA_RUN_TIME           3
#define IDLE_RUN_TIME           2
#define AT_PARSE_RUN_TIME       2  /* Guarantee min time for AT cmd parsing */


/* Define minimum number of RSACore() iterations */
#if defined(TARGET_VXD)
   #define TASKER_MIN_LOOPS        10
   #define TASKER_NO_LOOPS         TASKER_MIN_LOOPS
#else
   #define TASKER_NO_LOOPS         0
#endif

/* Reason codes for more information when RSATaskSetMinRunTime() is called.
   These codes are to aid Controlerless Modem debugging. */
#define TASKER_INIT_MODEM       0
#define TASKER_AT_PARSING       1
#define TASKER_IDLE_MODE        2
#define TASKER_ONLINE_MODE      3
#define TASKER_ON_HOOK_IDLE     5
#define TASKER_OFF_HOOK_DATA    6
#define TASKER_OFF_HOOK_FAX     7
#define TASKER_FAX_IDLE         8
#define TASKER_FAX_COMPLETE     9
#define TASKER_FAX_EXECUTE      10
#define TASKER_FAX_CONFIGURE    11
#define TASKER_RUN_DSP          12
#define TASKER_HALT_DSP         13
#define TASKER_INITIALIZE       14
#define TASKER_RESTORE          15
#define TASKER_VOICE_MODE       16
#define TASKER_WIN32_UPDATE     17

/* Tasker prototypes */
#ifdef TARGET_NT
void  RSATaskSwitch(void);
#endif

asmlinkage void  RSATaskYield(void);
void  RSATaskSchedule(void);
asmlinkage unsigned long RSATaskSetMinRunTime(unsigned char, unsigned char);
unsigned short RSATaskSetMinLoops  (unsigned short, unsigned char);
unsigned short RSATaskSetTempLoops (unsigned short);
void  TaskerInitStack(void (*)(void));


