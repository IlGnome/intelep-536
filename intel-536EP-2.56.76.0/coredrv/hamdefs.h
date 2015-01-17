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
#ifdef LINUX
#ifndef HAMDEFS_H
#define HAMDEFS_H


#ifndef __KERNEL__
   #define __KERNEL__
#endif

#ifndef MODULE
   #define MODULE
#endif

#ifdef __GENKSYMS__
   #undef __GENKSYMS__
#endif

#if defined(DRIVERCODE)
   #define EXPORT_SYMTAB
   #include<linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
   #include<linux/autoconf.h>
#else
   #include<linux/config.h>
#endif
   #include<linux/kernel.h>
   #define _LOOSE_KERNEL_NAMES
   #include<linux/types.h>
#endif


#if !defined(DRIVERCODE) || defined(__KERNEL_STRICT_NAMES)
   #if !defined(_LINUX_TYPES_H)
      #if !defined(_SYS_TYPES_H)
         typedef unsigned long ulong;
         typedef unsigned short ushort;
      #endif
   #endif
#endif

#if defined(DRIVERCODE) 
   #include<asm/bitops.h>
#endif

typedef unsigned char ubyte;
typedef char byte;
typedef unsigned short uword;
typedef short word;
typedef unsigned long BOOLEAN;
typedef unsigned long BOOL;
typedef unsigned char* PUCHAR;
typedef unsigned long DWORD;

#define FALSE 0
#define TRUE 1

#ifndef NULL
#define NULL 0
#endif

#define OFF 0
#define ON 1

#define IN
#define OUT

#endif //HAMDEFS_H
#endif //LINUX

