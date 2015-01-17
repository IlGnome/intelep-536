ReadMe file for the
Intel(R) 537 V9x DF modem Linux driver

contents:
1.  License
2.  Release Notes
3.  Installation/Uninstallation
4.  File Descriptions
5.  International Users
6.  Beta Tester appreciation
7.  Security issues
8.  Compilation issues
    a. Instructions for Debian Users
    b. Kernel Source
9.  What is the Hamregistry?
10. what's v92 and v44?
11. The Hamregistry tool (for persistance)
12. The usrsound tool (for softbuzzer)
13. Known Bugs/Issues
14. Comments, ideas, problems, fixes

-------------------------------------------------------------------------------
1. LICENSE

IMPORTANT - read the file "LICENSE.txt" for the INTEL SOFTWARE LICENSE 
AGREEMENT BEFORE COPYING, INSTALLING OR USING.

-------------------------------------------------------------------------------
2. Release Notes 

      This release supports 2.4.x and 2.6.x kernels. It is not compatible to 2.2.x kernels.

      The softmodem binary was compiled with gcc version 3.2

      v92 support added:  modem on hold AT command set,
      PCM upstream, v44, and quick connect are implemented.			 
      Linux Compatability tests are performed on the latest or previous 
      versions of the following distributions: Mandrake, RedHat, and SuSE

-------------------------------------------------------------------------------
3.  INSTALLATION

Prerequisites:
   1. root access
   2. bash shell to run install scripts
   3. an Intel soft modem (537, 537EP, 537SP, 537AA or 537EA)
   4. KERNEL SOURCE HEADERS FOR THE KERNEL YOU ARE RUNNING
      (found on your distribution's CD)

6 steps to install
   1. login as ROOT 
   2. extract the archive into a directory with "tar -zxvf <archivename>.tgz"
   3. cd into the directory it created.
   4. Type: make clean
   5. Type: make 537
   6. Type: make install
This will create a /dev/modem device file. This file is used as an interface to 
modem by all applications: minicom, kpppd, efax, etc. Please configure the applications
to use /dev/modem if neccessary.

The installation script has been designed for the following distributions 
release versions

   mandrake-release
   SuSE-release
   redhat-release
   debian_version (including Corel)
   slackware-version
   conectiva-version
   bluepoint-release
   redflag-release
   Unknown distributions install modules and utilities but
   will not install boot scripts!.

Please examine the 537_inst and 537_boot scripts if you have a different distribution.

The driver registers itself as character device 
   major number 240, minor number 1.
The driver takes one argument right now, which is used to enable softbuzzer. It is disabled by default.

ATTENTION:  if the driver compiles but the script just wont work for you.
   Here are the bare minimum steps to get your modem to work.

   0.  log in as root.
   1.  insmod -f Intel537.o    (Intel537.ko for kernel 2.6)
   2. you can start "hamregistry &" at this point if you wish.
   3.  rm /dev/537
   4.  mknod /dev/537 c 240 1   (note "240" is the default, if it does not 
       work see what /proc/devices says 537's major number is)
   5.  ln -s /dev/537 /dev/modem
   6.  start a comm application like minicom and use the modem.
   7.  see section 3 (International Users) for info on setting the correct 
       country settings.

Uninstallation.
Linux modem driver is started by the boot script 537_boot. The script location 
is distribution specific, usually /etc/init.d. So to uninstall the modem, delete
this file, and stop the modules using command: rmmod Intel537
make uninstall does this for redhat.

-------------------------------------------------------------------------------
4.  FILE DESCRIPTIONS

537_inst installation script to install 537 modules and supporting files


files copied to /lib/modules/(kernel-version)/misc
   Intel537.o (Intel537.ko)	soft modem driver and pseudo serial driver

files copied to  /etc/rc.d/...  (path differes per distribution)
   537_boot		boot scrip to start and stop driver module

files copied to /usr/sbin
   hamregistry	hamregistry is the "registry" like tool that the modem uses to 
   get and store persistant data such as county info and profile strings.
   usrsound The usermode program implementing a software buzzer. When
   software buzzer is on, modem connections sound can be heard with soundcard
   installed. OSS soundcards are supported. ALSA was not tested.

files copied to /etc
   hamregistry.bin	file that stores the initial persistant data for modem.

-------------------------------------------------------------------------------
5.  INTERNATIONAL USERS

hamregistry will store the last country setting you
set in the modem.

in minicom (or equivalent comm application)
the commmand to change country setting is "AT+GCI="
the command takes a t.35 country code in hexadecimal.
below is a list of currently supported t.35 country codes.
you can also put this "AT" command in the init string of
the comm application you are using.

if you are a CTR-21 country I think you should be able to 
choose a CTR-21 country on the list and be ok.  but 
that's no guarantee.
The same goes for countries that are "USA" compatable.
(this table also exist in the source file wwh_dflt.c that
ships with the Intel537 driver)

country  code   t.35 code
---------------------------
USA      1      B5 
KOR      82     61 
ECU      593    35 
BOL      591    14 
CHL      56     15 
COL      57     27 
PAN      507    85 
PER      51     88 
SAU      966    98 
THA      66     A9 
VNM      84     BC 
SWE      46     A5 
DNK      45     31 
FIN      358    3C 
NOR      47     82 
ISL      354    52 
IRL      353    57 
ISR      972    58 
LIE      423    68 
ESP      34     A0
TUR      90     AE
DEU      49     42
AUT      43     0A
CHE      41     A6
CYP      357    2D
GRC      30     46
ITA      39     59
LUX      352    69
NLD      31     7B 
GBR      44     B4 
BEL      32     0F
FRA      33     3D
PRT      351    8B
PAK      92     84
JPN      81     00
RUS      7      B8
AUS      61     09
MYS      60     6C
CHN      86     26
HKG      852    50
SGP      65     9C
NZL      64     7E
ARG      54     07
BRA      55     16
MEX      52     73
TWN      886    E3
IND      91     53
PHL      63     89
IDN      62     54
BHS      103    0B 
BRB      104    0E 
BMU      105    12 
GTM      502    49 
HTI      509    4E 
HND      504    4F 
JAM      1      5B 
NIC      505    7F 
PRY      595    87 
PRI      121    8C 
SUR      597    A3 
TTO      117    AC 
URY      598    B7 
VEN      58     BB 
ZWE      263    C4
GUY      592    4D
EST      372    E0
HUN      36     51
SVN      386    E2
ARE      971    B3 
SVK      421    2E
CAN      107    14 
CRI      506    1B 
DOM      110    33 
SLV      503    37 
GMB      220    41 
GIB      350    45 
POL      48     8A
EGY      20     36
CZE      420    2E
ZAF      27     9F
GUF      594    E1


-------------------------------------------------------------------------------
6.  Thanks to the following beta testers for their valuable input and 
    suggestions during the HaM 333 beta test between January 2 - 26, 2001
    and those who submitted bug reports for Intel 537 driver

Dorian S. Araneda
Sean Walbran
Rob Clark
Marvin Stodolsky
Dominique Duval
Roman Krais 
Ulrich Guenther
Marcelino Viana Pinheiro
Thomas S. Iversen
Jospeh Teichman
Michel Bartolone (MED)
Ramon Gonzalez Montoiro
Ryoji Kawagishi
Torsten Vogel
"jandro"
Ian Carr-de Avelon
Helga Weindl
Ed Casas
Bernhard Hoelcker
Alexander "Sasha" Voytov
Albert Woo
Andrey Vitsenko
Peter Hirschmann
Tom Lane

and all of the helpful Linux HaM and 537 modem users
around the world and at www.linmodems.org 

-------------------------------------------------------------------------------
7. Security issues

the 537_inst and 537_boot file install the files and device nodes as 
root for the owner and group.  
this will cause problems for those who want to user the modem to dialout
using an account other than root.

In SuSE, "dialout" is the group used to install the files and device node.
This way, anyone belonging to the "dialout" group can use the modem to dialout.
(take a look at /etc/group)

I did not want the script to allow full access of the modem to everyone without
"root" knowing.

Edit the 537_boot and 537_inst scripts to fit your needs.

-------------------------------------------------------------------------------
8. Compile issues
   a. this driver will now compile with the this path:
   /lib/modules/<kernel version>/build/include
   the 2.4.4+ kernels says to copy the /boot/vmlinuz.version.h
   over to the kernel build path.  I have the makefile do this
   if this file exists.  You must install the kernel source
   code anyways.  It should be on your distribution's CD.


-------------------------------------------------------------------------------
9.  What is the Hamregistry?
   The hamregistery is an application that stores data for the ham driver onto
   the disk.  hamregistry stores information from the driver that needs to 
   persist from reboot to reboot such as you current country setting.
      The 537_inst install script and the 537_boot script start this utility 
      automatically for you.
   If this tool is not present when the driver gets used your profile, 
   quickconnect, and current country setting will not be saved but the driver
   should still work fine.	The only step that would need to be done is to
   make sure that the driver is set to the correct country with 
   at+gci= (see section 5)


-------------------------------------------------------------------------------
10. What's v92 and v44?  

   a. modem on hold: (ISP and your ISP dialer must also support this)
      This will allow you to pause your ppp connection to answer an incoming
      call. You will need call waiting, dialer, and ISP support for this to 
      work.  When you are done with the call you can resume your ppp connection
      without having to reconnect.  The AT command set for this feature exist
      in the driver.

   b, pcm upstream: 
      (ISP must also support this, as of version 4.32 I
       dont know any ISP's that do)
      This will allow faster upload speeds.
      to enable: at+pig=0
      to disable: at+pig=1

   c. quickconnect:
      Once you make a call to a v92 modem, your phoneline characteristics are
      stored.  Whenever you make a new v92 connection it will use this data 
      to make the call negotiation  quicker (approx 10 seconds).
      to enable: at+pqc=0  at+pss=0
      to disable: at+pss=2

   d. v44: (ISP must also support this)
      A better compression protocol than v42 which can give you better transfer
      speeds.

-------------------------------------------------------------------------------
11. The Hamregistry tool
   
   The hamregistry tool is used to provide persistance of settings across
   reboots.  The 537_inst and 537_boot scripts automatically setup and start
   the hamregistry background task for the modem to use.
   The hamregistry tool has command line arguments for those who wish to 
   customize persistant settings.  To use these command lines
   you must first stop the driver with "bash 537_boot stop".
   Once the driver has been stopped you may run hamregistry with one of these
   arguments to store into the /etc/hamregistry.bin persistance file:
   (supply value for items in < >)
      -mfg <Modem manufactures name>
      -mod <Modem model name>
      -hookflash  <0,1,2>
        hookflash method:  0=(default)without tone  1=with tone  2=reserved
      -v92rptopt  <0,1>
        control v92 reporting:   0=PCM upsteam only       1=(default) all v92
      -gpio_lpohd <0,1>
        Handset Hook detection:  0=not supported          1=(default)supported
      -current_country <t.35 code>

   This info is written to the /etc/hamregistry.bin file.
   If hamregistry.bin exists along with the installation files, 537_inst will
   copy it to /etc/hamregistry.bin when installing the modem.
-------------------------------------------------------------------------------
12. The usrsound tool
   
   The usrsound tool is used to output modem negotiation sounds to sound device.
   The 537_inst and 537_boot scripts automatically setup usrsound. However, usrsound is not
   started by default now. To start usrsound, please make sure the Intel537.o driver is
   loaded with sound_enabled=1 parameter and run usrsound daemon task.

For example, on typical install, edit the script /etc/init.d/537_boot
Change string
      if ! ( /sbin/insmod -f $core 1>/dev/null 2>/dev/null ); then
to
      if ! ( /sbin/insmod -f $core sound_enabled=1 1>/dev/null 2>/dev/null ); then
and add string
usrsound
after      
ln -sf /dev/537 /dev/modem 1> /dev/null 2> /dev/null

-------------------------------------------------------------------------------
13. Known Bugs/Issues

   a. Be aware that the build replaces your 
      /lib/module/<kernver>//build/include/linux/version.h file with 
      /boot/vmlinuz.version.h
      (this is what Linus T. told me to do with a compiler error)
   b. There may be an incompatibility with DevFS. The 537 device may be located
      in /dev/tts/537
      instead of /dev/537.  Be aware of this and link /dev/modem to the 537 
      device that corresponds to your setup.
   c. Currently there is a problem with driver and SuSE 7.2 ppp 
      connections.   I have been able to get ppp connection with bellsouth 
      by calling this bash script and wvdial config section:

   ------my script----------------------
   #! /bin/sh
   /usr/sbin/pppd -detach lock asyncmap 00000000 \
      defaultroute debug /dev/modem 115200 \
      ipparam ppp0 linkname ppp0 \
      noauth \
      connect "/usr/bin/wvdial --chat bellsouth"

   ------my /etc/wvdial.conf section ---
   [Dialer bellsouth]
   Modem = /dev/modem
   Baud = 115200
   Init1 = ATZ
   Inti2 = ATQ0 V1 E1 S0=0 &C1 &D2
   Dial Command = ATDT
   Phone = 9777888
   Username = myloginname
   Password = mysecretpassword
   #Ask Password = 1
   Stupid Mode = 0
   ------------------

wvdial.conf can be created automatically:
   wvdialconf wvtest.txt
Found a modem on /dev/537, using link /dev/modem in config.
Modem configuration written to wvtest.txt.
--------
Edit wvtest.txt  as indicated.  If your country is not the Unites States,
the Init lines should be
   Init1 = ATZ
   Init2 = AT+GCI=< your hexadecimnmal country code >
   Init3 = ATQ0 V1 E1 S0=0 &C1 &D2 +FCLASS=0
Then:
   cp wvtest.txt  /etc/wvdial.conf, which will later direct the dialout.

First check COMM status with:
  # ifconfig
eth0      Link encap:Ethernet  HWaddr 00:D0:59:36:60:A2
          UP BROADCAST MULTICAST  MTU:1500  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:0 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000
          RX bytes:0 (0.0 b)  TX bytes:0 (0.0 b)
          Interrupt:11 Base address:0x2000

lo        Link encap:Local Loopback
          inet addr:127.0.0.1  Mask:255.0.0.0
          UP LOOPBACK RUNNING  MTU:16436  Metric:1
          RX packets:212 errors:0 dropped:0 overruns:0 frame:0
          TX packets:212 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:0
          RX bytes:11316 (11.0 KiB)  TX bytes:11316 (11.0 KiB)

The loopback "lo" is standard.  But any extra COMM mode such as eth0
should be shut down if it will interfere with domain name services (DNS)
   ifconfig eth0 down
will suffice more most systems. Check with:
   ifconfig
lo        Link encap:Local Loopback
          inet addr:127.0.0.1  Mask:255.0.0.0
          UP LOOPBACK RUNNING  MTU:16436  Metric:1
          RX packets:212 errors:0 dropped:0 overruns:0 frame:0
          TX packets:212 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:0
          RX bytes:11316 (11.0 KiB)  TX bytes:11316 (11.0 KiB)

Then dialout with
   wvdial &
# wvdial
--> WvDial: Internet dialer version 1.54
--> Initializing modem.
--> Sending: ATZ
ATZ
OK
--> Sending: ATQ0 V1 E1 S0=0 &C1 &D2 +FCLASS=0
ATQ0 V1 E1 S0=0 &C1 &D2 +FCLASS=0
OK
--> Modem initialized.
--> Sending: ATDT9777888
--> Waiting for carrier.
ATDT9777888
CONNECT 53333 V44
--> Carrier detected.  Waiting for prompt.
** dialup12.rol.ru Terminal Server **
Login: 
--> Looks like a login prompt.
--> Sending: internet
internet
Password: 
--> Looks like a password prompt.
--> Sending: (password)
    Entering PPP Session.
    IP address is 66.44.1.90
    MTU is 1006.
--> Looks like a welcome message.
--> Starting pppd at Sun Apr  4 23:11:49 2004
--> pid of pppd: 4879
--> Using interface ppp0
   ROM checksum self-test: passed (0xdbd8681d).
 CSLIP: code copyright 1989 Regents of the University of California
 PPP generic driver version 2.4.2
 pppd 2.4.2 started by root, uid 0
 Using interface ppp0
 Connect: ppp0 <--> /dev/ttyLT0
 kernel does not support PPP filtering
--> local  IP address 68.14.26.80
--> remote IP address 207.116.5.132
--> primary   DNS address 207.116.4.8
--> secondary DNS address 207.116.4.9
 PPP BSD Compression module registered
 PPP Deflate Compression module registered
 local  IP address 68.14.26.80
 remote IP address 207.116.5.132
 primary   DNS address 207.116.4.8

 The DNS address gets copied to /etc/resolv.conf
wherefrom it consulted to find NAME SERVERS which translate Named addresses into
the Numerical addresses used by the Internet system. Display with

  cat /etc/resolv.conf
search
nameserver 207.116.4.8
nameserver 207.116.4.9

Test navigation capability with a known Internet address.
For example the numeric address of novell.com is 130.57.4.70

# ping 130.57.4.70
PING 130.57.4.70 (130.57.4.70): 56 data bytes
64 bytes from 130.57.4.70: icmp_seq=0 ttl=50 time=208.0 ms
64 bytes from 130.57.4.70: icmp_seq=1 ttl=50 time=209.9 ms
64 bytes from 130.57.4.70: icmp_seq=2 ttl=50 time=210.0 ms
shows that your System is attached to the internet, while using the named address

   ping novell.com
PING novell.com (130.57.4.70): 56 data bytes
64 bytes from 130.57.4.70: icmp_seq=0 ttl=50 time=204.4 ms
64 bytes from 130.57.4.70: icmp_seq=1 ttl=50 time=210.0 ms
64 bytes from 130.57.4.70: icmp_seq=2 ttl=50 time=210.0 ms
64 bytes from 130.57.4.70: icmp_seq=3 ttl=50 time=210.0 ms
shows that DNS services are OK. ALWAYS quickly abort a "ping" test with
  Ctrl -C

This COMM session was started with
    wvdial &
where the "&" puts wvdial in the background and allows command prompt recovery
To stop this session I will fore ground (fg)
    fg wvdial and then stop wvdial with
    Ctrl-C

-------------------------------------------------------------------------------
14. Comments, ideas, problems, fixes? please contact:

Linux Voice Band Modems (VBM) of Intel Residential Access Division (RAD)

vbm.linux@intel.com
http://developer.intel.com/design/modems/

To restrict email volume, please email only development related issues that are
needed to fix a bug or improve the driver. General questions on how to use the 
Linux OS may not be responed to.

Other resources and information on Linux controllerless modems can be found on 
these useful sites

http://www.linmodems.org 
	and
http://linmodems.technion.ac.il

If you have problems please read Jacques' PostInstall.html FIRST at
   http://linmodems.technion.ac.il/
before sending a message to DISCUSS@LINMODES.ORG
