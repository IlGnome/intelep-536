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




#if !defined(SOFTSERIAL_H)
#define SOFTSERIAL_H


#define SOFTSERIAL_MAJIK 0xdcba
#define SOFTSERIAL_NUMBER_OF_PORTS    1
#define SOFTSERIAL_DRIVER_NAME        "softserial"
#ifndef TARGET_CATAWBA
#define SOFTSERIAL_DEVICE_NAME        "536ep" 
#else
#define SOFTSERIAL_DEVICE_NAME        "Intel537" 
#endif
#define SOFTSERIAL_MAJOR_NUMBER       240
#define SOFTSERIAL_MINOR_NUMBER_START 1

#include<linux/version.h>
#include<linux/module.h>
#include<linux/tty.h>
#include<linux/serial.h>
#include<linux/serialP.h>
#include<asm/serial.h>
#include<linux/pci.h>
#include<linux/termios.h>
#include<linux/sched.h>
#include<asm/bitops.h>
#include<asm/uaccess.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
#define ktermios termios
#endif

struct global
{
    struct tty_driver  softserial_tty_driver;
    struct softcore_struct softcore;
    unsigned char refcount;
};

void softserial_background_event_handler(void*);
int  softserial_register_tty            (void);
void softserial_interrupt      (int, void*, struct pt_regs*);
void softserial_stop           (struct tty_struct*);
void softserial_start          (struct tty_struct*);
void softserial_hangup         (struct tty_struct*);
void softserial_flush_chars    (struct tty_struct*);
int  softserial_write_room     (struct tty_struct*);
int  softserial_chars_in_buffer(struct tty_struct*);
void softserial_flush_buffer   (struct tty_struct*);
void softserial_throttle       (struct tty_struct*);
void softserial_unthrottle     (struct tty_struct*);
void softserial_wait_until_sent(struct tty_struct*, int);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
int softserial_break          (struct tty_struct*, int);
#else
void softserial_break          (struct tty_struct*, int);
#endif
void softserial_send_xchar     (struct tty_struct*, char);
void softserial_close          (struct tty_struct*, struct file*);
int  softserial_open           (struct tty_struct*, struct file*);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
int softserial_put_char       (struct tty_struct*, unsigned char);
#else
void softserial_put_char       (struct tty_struct*, unsigned char);
#endif
void softserial_set_termios    (struct tty_struct*, struct ktermios*);
int  softserial_write	       (struct tty_struct * tty,
                      const unsigned char *buf, int count);
void softserial_set_ldisc     (struct tty_struct*);

//int  softserial_write          (struct tty_struct*, int, const unsigned char*, int);
int  softserial_ioctl          (struct tty_struct*, struct file*, unsigned int, unsigned long);




#endif //SOFTSERIAL_H
