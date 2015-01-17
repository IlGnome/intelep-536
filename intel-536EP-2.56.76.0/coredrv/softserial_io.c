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
//#include <asm-i386/pgtable.h>
#include"softcore.h"
#include"hamcore.h"

#include"softserial.h"

extern struct global G;

//=============================================================================
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
int
#else
void
#endif
softserial_put_char(struct tty_struct* ptty,
                        unsigned char ch)
{
#if defined (DEBUG_LINUX)
   printk("softserial:softserial_put_char()\n");
#endif
   G.softcore.putchar(ch);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
   return 1;
#endif
}

//=============================================================================
void softserial_flush_chars(struct tty_struct* ptty)
{
#if defined (DEBUG_LINUX)
   printk("softserial:softserial_flush_chars()\n");
#endif
   //does nothing
}

//=============================================================================
static DECLARE_MUTEX(softserial_write_sem);
int softserial_write(struct tty_struct* ptty,
                      //int from_user_space,
                      const unsigned char* input_buffer,
                      int write_count_asked)  //why is it a signed int?
{
   int written_count  = 0;
   int softcore_space = 0;
   int copy_size = 0;
   const unsigned char* input_buffer_ptr = input_buffer;
   
   
   if(write_count_asked <=0) return (-EFAULT);
   while(write_count_asked - written_count > 0)
    {
         softcore_space = G.softcore.write_free();
         if(softcore_space == 0) break;  //try again?  potential infinite loop?
         if(write_count_asked > softcore_space)
         {
            copy_size = softcore_space;
         }
         else
         {
            copy_size = write_count_asked;
         }
         memcpy(G.softcore.tx_fifo, input_buffer_ptr, copy_size);
         G.softcore.write(copy_size);

         input_buffer_ptr += copy_size;
         written_count += copy_size;
   }

   return(written_count);
}

//=============================================================================
int softserial_write_room(struct tty_struct* ptty)
{
#if defined (DEBUG_LINUX)
   printk("softserial:softserial_write_room()\n");
#endif
   return(G.softcore.write_free());
}

//=============================================================================
int softserial_chars_in_buffer(struct tty_struct* ptty)
{
#if defined (DEBUG_LINUX)
   printk("softserial:softserial_chars_in_buffer()%d\n",
		   G.softcore.write_pending());
#endif
   return(G.softcore.write_pending());
}

//=============================================================================
void softserial_flush_buffer(struct tty_struct* ptty)
{
#if defined (DEBUG_LINUX)
   printk("softserial:softserial_flush_buffer()\n");
#endif
   G.softcore.write_flush();
}

//=============================================================================
void softserial_wait_until_sent(struct tty_struct* ptty,
                                int jiffies_waittime)
{
   unsigned long start = jiffies;
#if defined (DEBUG_LINUX)
   printk("softserial:softserial_wait_until_sent()\n");
#endif
   while(1)
   {
      if(G.softcore.write_pending() == 0) return;
      current->state = TASK_INTERRUPTIBLE;
      schedule_timeout(1);
      if(signal_pending(current)) break;
      if(jiffies_waittime && (jiffies - start >= jiffies_waittime)) break;
   }
   current->state = TASK_RUNNING;
}

//=============================================================================
void softserial_throttle(struct tty_struct* ptty)
{
#if defined (DEBUG_LINUX)
   printk("softserial:softserial_throttle()\n");
#endif
   if(I_IXOFF(ptty)) G.softcore.send_xchar(STOP_CHAR(ptty));
   G.softcore.rts_off();
}


//=============================================================================
void softserial_unthrottle(struct tty_struct* ptty)
{
#if defined (DEBUG_LINUX)
   printk("softserial:softserial_unthrottle()\n");
#endif
   G.softcore.rts_off();
   G.softcore.send_xchar(START_CHAR(ptty));
}

//=============================================================================
void softserial_send_xchar(struct tty_struct* ptty,
                           char softchar)
{
#if defined (DEBUG_LINUX)
   printk("softserial:softserial_send_xchar()\n");
#endif
   G.softcore.send_xchar(softchar);
}
//=============================================================================
void softserial_set_ldisc(struct tty_struct *ptty)
{
   int baud; 
   char parity;
   char stop_bits;
   char size;
   G.softcore.get_char(&baud,&size,&parity,&stop_bits);
   ptty->termios->c_cflag &= ~ CBAUD;
   switch(baud){
   case 0 : ptty->termios->c_cflag |= B0;
            break;
   case 50 : ptty->termios->c_cflag |= B50;
            break;
   case 75 : ptty->termios->c_cflag |= B75;
            break;
   case 110 : ptty->termios->c_cflag |= B110;
            break;
   case 134 : ptty->termios->c_cflag |= B134;
            break;
   case 150 : ptty->termios->c_cflag |= B150;
            break;
   case 200 : ptty->termios->c_cflag |= B200;
            break;
   case 300 : ptty->termios->c_cflag |= B300;
            break;
   case 600 : ptty->termios->c_cflag |= B600;
            break;
   case 1200 : ptty->termios->c_cflag |= B1200;
            break;
   case 1800 : ptty->termios->c_cflag |= B1800;
            break;
   case 2400 : ptty->termios->c_cflag |= B2400;
            break;
   case 4800 : ptty->termios->c_cflag |= B4800;
            break;
   case 9600 : ptty->termios->c_cflag |= B9600;
            break;
   case 19200 : ptty->termios->c_cflag |= B19200;
            break;
   case 38400 : ptty->termios->c_cflag |= B38400;
            break;
   case 57600 : ptty->termios->c_cflag |= B57600;
            break;
   case 115200 :
   default:     ptty->termios->c_cflag |= B115200;
   }
   ptty->termios->c_cflag &= ~CSIZE;
   switch(size){
   case 5: ptty->termios->c_cflag |= CS5;
           break;
   case 6: ptty->termios->c_cflag |= CS6;
           break;
   case 7: ptty->termios->c_cflag |= CS7;
           break;
   case 8: ptty->termios->c_cflag |= CS8;
   }
   ptty->termios->c_cflag &= ~(PARENB | PARODD | CMSPAR);
   switch (parity){
   case 0 : ptty->termios->c_cflag &= ~PARENB;
            break;
   case 1: ptty->termios->c_cflag |= PARENB | PARODD;
            break;
   case 2: ptty->termios->c_cflag |= PARENB;
            break;
   case 3: ptty->termios->c_cflag |= PARENB | CMSPAR;
            break;
   }
   ptty->termios->c_cflag &= ~CSTOPB;
   if (stop_bits==2)
       ptty->termios->c_cflag |= CSTOPB;
   
}   
//=============================================================================
void softserial_set_termios(struct tty_struct* ptty,
                            struct ktermios* previous_termios)
{
struct async_struct* ptr_async_struct;
#if 1
int baud = 38400;
char parity = 8;
char stop_bits = 1;
char size=8;
int mask = CBAUD | CSIZE | PARENB | PARODD | CMSPAR | CSTOPB;
#endif

#if defined (DEBUG_LINUX)
   printk("softserial:softserial_set_termios()\n");
#endif
   if(!ptty)
   {
	   printk("softserial_set_termios:set termios  null ptty\n");
	   return;
   }
   if(!(ptty->driver_data))
   {
	   printk("softserial_set_termios:set termios null driver data\n");
	   return;
   }
#if 1   
ptr_async_struct = (struct async_struct*)(ptty->driver_data);
   if(ptty->termios->c_cflag & CRTSCTS)
	   ptr_async_struct->flags |=  ASYNC_CTS_FLOW;
   else    
	   ptr_async_struct->flags &= ~ASYNC_CTS_FLOW;

   if(ptty->termios->c_cflag & CLOCAL)  
	   ptr_async_struct->flags &= ~ASYNC_CHECK_CD;
   else    
	   ptr_async_struct->flags |=  ASYNC_CHECK_CD;
#endif
#if 1
   if ((previous_termios) && 
       ((previous_termios->c_cflag & mask) == (ptty->termios->c_cflag & mask)))
        return;
   baud=tty_get_baud_rate(ptty);
   switch(ptty->termios->c_cflag & CSIZE){
   case CS5: size=5;
             break;
   case CS6: size=6;
             break;
   case CS7: size=7;
             break;
   case CS8: size=8;
   }
   if (!(ptty->termios->c_cflag & PARENB))
        parity = 0;
   if (ptty->termios->c_cflag & (PARENB & PARODD))
       parity = 1;
   if (ptty->termios->c_cflag & (PARENB & ~PARODD))
       parity = 2;
   if (ptty->termios->c_cflag & (PARENB & CMSPAR))
       parity=3;
   if (ptty->termios->c_cflag & CSTOPB)
       stop_bits=2;
//   G.softcore.rts_off();
   G.softcore.set_char(baud,size,parity,stop_bits);
//   G.softcore.rts_on();
#endif
}

//=============================================================================
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,26)
int
#else
void 
#endif
softserial_break(struct tty_struct* ptty,
                      int state)
{
#if defined (DEBUG_LINUX)
   printk("softserial:softserial_break()\n");
#endif
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,26)
 if (!state)
     return 0;
 else
     return 1;
#endif
}
