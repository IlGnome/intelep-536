#***************************************************************************
# Makefile to build Driver executable
#***************************************************************************

PWD	:= $(shell pwd)
INCLUDES = -I$(KERNEL_SOURCE_PATH)/include -isystem $(PWD)/../inc
 #if no kernel source path was specified, correct INCLUDES to reflect that
ifeq ($(KERNEL_SOURCE_PATH),)
     INCLUDES = -I/lib/modules/`uname -r`/build/include -isystem ../inc
endif
EXTRA_CFLAGS = $(PSTN_DEF) -Wall -O3 $(INCLUDES) -fno-common -DCONFIG_MODULES -DMODULE -DTARGET_SELAH 

DRIVER_OBJS = coredrv.o clmmain.o rts.o task.o uart.o wwh_dflt.o locks.o \
		softserial_io.o softserial_ioctl.o softserial.o

clean:
	rm -f *.ko .*.o.cmd *.mod.c .*.ko.cmd *.o *~ core Module.* modules.*
	rm -rf .tmp_versions

536core: $(DRIVER_OBJS)
	strip --strip-debug 536core.lib
	$(LD) -r $(DRIVER_OBJS) 536core.lib -o Intel536.o

ifeq ($(KERNEL_SOURCE_PATH),)
KDIR	:= /lib/modules/$(shell uname -r)/build
else
KDIR=$(KERNEL_SOURCE_PATH)
endif

obj-m := Intel536.o

Intel536-objs := $(DRIVER_OBJS) 536core.lib

536core_26:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

# DO NOT DELETE
