// LINUX FILE
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


#ifndef HAMCORE_H
#define HAMCORE_H
#define HAMCORE_VERSION 20020704//yyyymmdd
#define DRIVERCODE

#if defined (TARGET_CATAWBA) || defined (TARGET_SELAH)
   #define LINUX 1
   #define TARGET_LINUX 1
#endif

#include"hamdefs.h"
#include"sys_ver.h"

#define INTERNATIONAL_CODE 1 

#define MDM_MFG_ID "Intel Corporation"

#if INTERNATIONAL_CODE == 1
#define CODE_BASE "INT"
#else
#define CODE_BASE "FCC"
#endif

#if RELEASE_BUILD == 1
   #define PRE_PREFIX ""
#else
#define PRE_PREFIX "BETA-"
#endif

#if defined (TARGET_CATAWBA)
  #define VERSION_MSG "Intel(R) 537 Modem "PRE_PREFIX "Release " RELEASE_VERSION " " CODE_BASE" (kern:" UTS_RELEASE") - ""Jun 01 2004"
  #define MDM_MODEL_NAME 	"Intel(R) 537"
  #define MDM_DEVICE_NAME        "537"
  #define INTEL_VENDOR_ID      0x8086
  #define INTEL_DEVICE_ID      0x2446
  #define INTEL_ICH4_DEVICE_ID      0x24c6
  #define INTEL_ICH5_DEVICE_ID      0x24d6
  #define ALI_VENDOR_ID		   0x10b9
  #define ALI_DEVICE_ID	       0x5457
  #define NVIDIA_VENDOR_ID	   0x10de
  #define NVIDIA_DEVICE_ID     0x01c1
  #define SIS_VENDOR_ID		   0x1039
  #define SIS_DEVICE_ID	       0x7013
  #define ATI_VENDOR_ID		   0x1002
  #define ATI_DEVICE_ID	       0x434D
  #define VIA_VENDOR_ID		   0x1106
  #define VIA_DEVICE_ID	       0x3068
  #define TJ320_VENDOR_ID      0xe159
  #define TJ320_DEVICE_ID      0x0001
  #define NEW_SI3052_VENDOR_ID     0x1543
  #define NEW_SI3052_DEVICE_ID     0x3052
  #define SI3052_VENDOR_ID 0x8086
  #define SI3052_DEVICE_ID 0x1080
  #define SELAH_PCI_VENDOR_ID  0x8086
  #define SELAH_PCI_DEVICE_ID  0x1040
#elif defined(TARGET_SELAH)
   #define MDM_MODEL_NAME "Intel 536EP Data Fax"
   #define MDM_PCI_VENDOR_ID      0x8086
   #define MDM_PCI_DEVICE_ID      0x1040
   #define MDM_DEVICE_NAME        "536ep"
   #define VERSION_MSG "536EP "PRE_PREFIX "Release " RELEASE_VERSION " " CODE_BASE" (kern:" UTS_RELEASE") - ""Jun 01 2004"
#elif defined(TARGET_HAM)
   #define MDM_MODEL_NAME "Intel HaM Data Fax"
   #define MDM_PCI_VENDOR_ID      0x1813
   #define MDM_PCI_DEVICE_ID      0x4000
   #define MDM_DEVICE_NAME        "ham"
   #define VERSION_MSG "HaM "PRE_PREFIX "Release " RELEASE_VERSION " " CODE_BASE" (kern:" UTS_RELEASE") - ""Jun 01 2004"
#else
   #error hamcore.h  PROJECT UNDEFINED
#endif

#if defined(TARGET_HAMPLUS)
   #undef MDM_MODEL_NAME
   #undef MDM_PCI_VENDOR_ID  
   #undef MDM_PCI_DEVICE_ID
   #undef MDM_DEVICE_NAME
   #undef VERSION_MSG 

   #define MDM_MODEL_NAME "Intel HaM plus Data Fax"
   #define MDM_PCI_VENDOR_ID      0x1813
   #define MDM_PCI_DEVICE_ID      0x4100
   #define MDM_DEVICE_NAME        "ham"
   #define VERSION_MSG "HaM plus "PRE_PREFIX "Release " RELEASE_VERSION " " CODE_BASE" (kern:" UTS_RELEASE") - ""Jun 01 2004"
#endif


#define CLM_DRIVER_NAME        "serial"
#define CLM_MAJOR              240 //start of local-experimental use
#define CLM_MINOR_START        1
#define CLM_PORTS              1

struct hamcore_data_struct
{
   unsigned int version;   //==HAMCORE_VERSION
   unsigned int* dsp_irq;
   unsigned int* io_base;
   unsigned int* io_range;
   void* mm_base;
   unsigned int* mm_range;
#if defined (TARGET_CATAWBA)
   struct pci_dev *pdev;
   int afe_type;
#endif
};

typedef enum _DEVSTATE {
   OPENED,
   OPENING,
   CLOSED,
   CLOSING,
   STOPPED,
   REMOVED
} DEVSTATE, *PDEVSTATE;


/* PCI Configuration Register Information */
struct pci_info_str
{
   unsigned char  bus;
   unsigned char  funct;

   unsigned short vendor;
   unsigned short device;
   unsigned char  revision;

   unsigned int   class;
   unsigned char  cache_line;
   unsigned char  latency_timer;
   unsigned char  header_type;
   unsigned char  bist;

   unsigned int   address[6];
   unsigned int   mask[6];

   unsigned short subsystem_vendor;
   unsigned short subsystem_device;

   unsigned char  irq_line;
   unsigned char  irq_pin;
   unsigned char  min_gnt;
   unsigned char  max_lat;
};

int get_pci_info(struct pci_info_str *pci_info, unsigned short vendor,
   unsigned short device);



/*     Value for the variable modem_DTE_xon_char   */
#define XON_CHAR           0x11

/*     Value for the variable modem_DTE_xoff_char   */
#define XOFF_CHAR          0x13

/*     Values for the variable acu_state   */
#define S_IDLE                     0
#define S_PARSE                    1
#define S_ORIG                     2
#define S_ANSW                     3
#define S_ONLINE                   4
#define S_HANGUP                   5
#define S_TEST                     6
#define S_RETRAIN                  7
#define STATE_MAX                  8


/* fax1 state definitions */
#define IDLE                0
#define CONFIGURE_FTM       1
#define EXECUTE_FTM         2
#define COMPLETE_FTM        3
#define CONFIGURE_FRM       4
#define EXECUTE_FRM         5
#define COMPLETE_FRM        6
#define CONFIGURE_FTH       7
#define EXECUTE_FTH         8
#define COMPLETE_FTH        9
#define CONFIGURE_FRH       10
#define EXECUTE_FRH         11
#define COMPLETE_FRH        12


/* External data */
extern ubyte acu_loop;
extern ubyte acu_stop_bits;
extern ubyte dce_stop_bits;
extern ubyte acu_enabled;
extern char mdm_mfg_id[64];
extern char mdm_model_name[64];
extern ubyte at_z_command;
extern ubyte fax_class;
extern ubyte fax1_state;
extern ubyte acu_state;
extern ubyte slG, slQ;
extern ubyte AFE_type;
extern ubyte dte_to_modem_xon_char;
extern ubyte dte_to_modem_xoff_char;
extern ubyte mt_v90_configured;
extern ubyte fatal_error_occurred;

/* External function prototypes */
extern void  clm_bg(void);
extern void  clm_initialize(void);
extern void  clm_terminate(void);
extern void  clm_rx_int(void *);
extern void  clm_tx_int(void);
extern void  clm_external_int(void);
extern void  clm_configure(void);
asmlinkage extern void  init_eeprom_data(void);
extern asmlinkage void  master_init(void);
extern asmlinkage void  line_int(void);
extern asmlinkage long  ResetDspInternal(void);
extern asmlinkage void  dspdrv_clear_dsp_interrupt(void);
asmlinkage extern void  dspdrv_SetCramISRCallBack(asmlinkage void (*callback)(void));
asmlinkage extern void  dspdrv_CommRamISR(void);
asmlinkage extern word  Update_CurrentCountry(uword);
asmlinkage extern void  acu_from_clear(void);
asmlinkage extern uword acu_from_free(void);
extern asmlinkage void  ACU_Main(void);
asmlinkage extern void  acu_to_clear(void);
asmlinkage extern void  acu_external_int(void);
asmlinkage extern void  acu_tx_int(void);
asmlinkage extern void  acu_io_enable(void);
asmlinkage extern void  acu_io_disable(void);
asmlinkage extern void  dte_external_int(void);
asmlinkage extern void  dte_tx_int(void);
asmlinkage extern void  dte_to_clear(void);
asmlinkage extern void  dte_from_clear(void);
asmlinkage extern uword dte_from_free(void);
asmlinkage extern void  ModemCardStart(void);
asmlinkage extern void  ModemCardStop(void);
extern int   modem_init(void*);
extern int   sound_init(void);
extern int   modem_kill(void);
asmlinkage extern int AfeIRQ(void);
asmlinkage extern void  IM_DSP_ON(void);
asmlinkage extern void  IM_DSP_OFF(void);
asmlinkage extern void dte_init(void);
asmlinkage extern void line_enable_interrupts(void);
asmlinkage extern void line_disable_interrupts(void);

extern asmlinkage int (*Debug_Printf)(const char*, ...);

#define CPINT_STAT   0x01    /* control processor interrupt pending */
#define INTEV_REG    0xf7    /* interrupt event register */
#define CP_INTERRUPT 0xff    // control processor interrupt register

#if defined(TARGET_SELAH)
   #define REGISTER_OFFSET  0
   #define     CMHISR                      (REGISTER_OFFSET + 0xF008)
   #define     bHIS_MP                     0x02        // Generic MP int 
asmlinkage   extern unsigned char IM_Peek_Reg(unsigned short);
#endif //TARGET_SELAH


#endif //HAMCORE_H
