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

#if !defined(SOFTCORE_H)
#define SOFTCORE_H


struct softcore_struct
{
   struct pci_struct
   {
      struct pci_dev* pdev;
      unsigned short  ven_id;
      unsigned short  dev_id;
      unsigned char   function_id;
   } pci;
#if defined (TARGET_CATAWBA)
   int afe_type;
   int sound_enabled;
   unsigned long io_base;
   unsigned long io_range;
   unsigned long mem_base;
   unsigned long mem_range;
#endif

   unsigned long  tx_size;
   unsigned char* tx_fifo;

   struct tty_struct* ptty;
 
   int  (*open)           (void);
   void  (*stop)           (void);
   void  (*close)          (void);
   void  (*start)          (void);
   int   (*write)          (int);
   void  (*dtr_on)         (void);
   void  (*rts_on)         (void);
   void  (*hangup)         (void);
   void  (*putchar)        (unsigned char);
   void  (*dtr_off)        (void);
   void  (*rts_off)        (void);
   void  (*msr_wait)       (void);
   void  (*send_xchar)     (unsigned char);
   int   (*write_free)     (void);
   void  (*write_flush)    (void);
   int   (*write_pending)  (void);
   int   (*get_line_status)(unsigned char);
   unsigned int(*set_char) (int, char, char, char);
   void (*get_char) (int *, char *, char *, char *);
};



#endif
