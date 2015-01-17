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

extern struct global G;


//=============================================================================
int softserial_ioctl(struct tty_struct* ptty,
                      struct file* pfile,
                      unsigned int ioctl,
                      unsigned long ioctl_parameter)
{
  int error;
  unsigned int user_arg;

#if defined (DEBUG_LINUX)
   printk("softserial:softserial_ioctl()\n");
   printk(
     "ham:ioctl[%X]: dir[%d], type[%d], nr[%d], size[%d]\n",
             ioctl,
             _IOC_DIR(ioctl),
             _IOC_TYPE(ioctl),
             _IOC_NR(ioctl),
             _IOC_SIZE(ioctl));
#endif
   switch (ioctl)
   {
//CASES that (may) need implementing
   case TIOCSERCONFIG:
   case TIOCSERGETLSR:
   case TIOCSERGSTRUCT:
   case TIOCGICOUNT:
   case TIOCGSERIAL: //TODO: give user serial structure
   case TIOCSSERIAL: //TODO: update our serial structure
      return (0);

//CASES NOT IN SERIAL.C BUT TTY
#ifdef TIOCGETP
   case TIOCGETP:
   case TIOCSETP:
   case TIOCSETN:
#endif
#ifdef TIOCGETC
   case TIOCGETC:
   case TIOCSETC:
#endif
#ifdef TIOCGLTC
   case TIOCGLTC:
   case TIOCSLTC:
#endif
   case TCGETS:
   case TCSETSF:
   case TCSETSW:
   case TCSETS:
   case TCGETA:
   case TCSETAF:
   case TCSETAW:
   case TCSETA:
   case TCXONC:
   case TCFLSH:
   case TIOCOUTQ:
   case TIOCINQ:
   case TIOCGLCKTRMIOS:
   case TIOCSLCKTRMIOS:
   case TIOCPKT:
   case TIOCGSOFTCAR:
   case TIOCSSOFTCAR:
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,28)
    return(n_tty_ioctl(ptty,pfile,ioctl,ioctl_parameter));
#else
    return(n_tty_ioctl_helper(ptty,pfile,ioctl,ioctl_parameter));
#endif
//CASES FOR ONLY 2.1 KERNEL, IGNORE, RETURN AS DEFALUT
   case TCSBRK:
   case TCSBRKP:
      return (-ENOIOCTLCMD);

//OLD rejected DEBIAN call setserial -W     "setserial -W" is called in Debian boot
   case TIOCSERGWILD:
   case TIOCSERSWILD:
      return (0);

//CASES FOR MULTIPORT THAT DONT APPLY TO US
   case TIOCSERGETMULTI:
   case TIOCSERSETMULTI:
      return (-ENOIOCTLCMD);

//CASES implemented
   case TIOCMGET:
   {
      unsigned int status;
      status = 0;
#if defined (DEBUG_LINUX)
      printk("softserial: TIOCMGET\n");
#endif
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,5,0) 
      if(!MOD_IN_USE)
      {
         return -ENODEV;
      }
#else
      if(!G.refcount) return -ENODEV;
#endif
      return(put_user(G.softcore.get_line_status(0), (unsigned int*)ioctl_parameter));
   }

   case TIOCMBIS:
#if defined (DEBUG_LINUX)
      printk("softserial: TIOCMBIS\n");
#endif
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,5,0) 
      if(!MOD_IN_USE) return -ENODEV;
#else
      if(!G.refcount) return -ENODEV;
#endif
      error = get_user(user_arg, (unsigned int*)ioctl_parameter);
      if (error) return (error);
      if (user_arg & TIOCM_RTS) G.softcore.rts_on();
      if (user_arg & TIOCM_DTR) G.softcore.dtr_on();
      return(0);

  case TIOCMBIC:
#if defined (DEBUG_LINUX)
      printk("softserial: TIOCMBIS\n");
#endif
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,5,0) 
      if(!MOD_IN_USE) return -ENODEV;
#else
      if(!G.refcount) return -ENODEV;
#endif
      error = get_user(user_arg, (unsigned int*)ioctl_parameter);
      if (error) return (error);
      if (user_arg & TIOCM_RTS) G.softcore.rts_off();
      if (user_arg & TIOCM_DTR) G.softcore.dtr_off();
      return(0);


   case TIOCMSET:
#if defined (DEBUG_LINUX)
      printk("softserial: TIOCMSET\n");
#endif
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,5,0) 
      if(!MOD_IN_USE)return -ENODEV;
#else
      if(!G.refcount) return -ENODEV;
#endif
      error = get_user(user_arg, (unsigned int*)ioctl_parameter);
      if (error) return (error);

      if (user_arg & TIOCM_RTS) G.softcore.rts_on();
      else                      G.softcore.rts_off();

      if (user_arg & TIOCM_DTR) G.softcore.dtr_on();
      else                      G.softcore.dtr_off();

      return(0);

   case TIOCMIWAIT: 
   {
      unsigned long ring_count, new_ring_count;
      unsigned long  dsr_count, new_ndsr_count;
      unsigned long  dcd_count, new_ndcd_count;
      unsigned long  cts_count, new_ncts_count;
#if defined (DEBUG_LINUX)
      printk("softserial: TIOCMIWAIT\n");
#endif     
      ring_count= G.softcore.get_line_status(1);//RINGING);
      dsr_count = G.softcore.get_line_status(2);//DSR);
      dcd_count = G.softcore.get_line_status(3);//RLSD);
      cts_count = G.softcore.get_line_status(4);//CTS);
      while (1)
      {
         /// wait until line status changes 
         G.softcore.msr_wait();
         
         /// if no signal pending, exit
         if (signal_pending(current))
            return (-ERESTARTSYS);
         
         new_ring_count = G.softcore.get_line_status(1);// RINGING);
         new_ndsr_count = G.softcore.get_line_status(2);// DSR);
         new_ndcd_count = G.softcore.get_line_status(3);// RLSD);
         new_ncts_count = G.softcore.get_line_status(4);// CTS);
         
         //* if no signals changed, return error 
         if ((new_ring_count == ring_count) && 
             (new_ndsr_count == dsr_count)  && 
             (new_ndcd_count == dcd_count)  && 
             (new_ncts_count == cts_count))
         {
            return (-EIO);
         }
         
      if (((ioctl_parameter & TIOCM_RNG) && (new_ring_count != ring_count))   ||
          ((ioctl_parameter & TIOCM_DSR) && (new_ndsr_count != dsr_count ))   ||
          ((ioctl_parameter & TIOCM_CD)  && (new_ndcd_count != dcd_count ))   ||
          ((ioctl_parameter & TIOCM_CTS) && (new_ncts_count != cts_count )))
         {
            return (0);
         }
      }
      break;
   }

   default:
      if(_IOC_TYPE(ioctl) == 't')  //some ppp ioctl command that we dont support
         return(-ENOIOCTLCMD);
      
#if defined (DEBUG_LINUX)
      printk("softserial: DEFAULT IOCTL\n");
#endif
      return (-ENOIOCTLCMD);
   }
   return (0);
}
