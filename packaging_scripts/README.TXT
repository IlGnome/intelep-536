Instructions for Creating a Debian Package from the Modem Driver.

If you do not have the package "fakeroot" installed, you will need to install
it before beginning. You can install it using Synaptic package manager, or with
the shell commandline by entering: sudo apt-get install fakeroot



1. Compile the Modem Driver.
	-Download the latest tarball from http://vouters.dyndns.org/Intel/
	-Compile the 536 running: make 536
	-Compile the 537 by running: export MODEM_TYPE=537EP make 537
	-For more detailed instructions, see:
		- https://help.ubuntu.com/community/DialupModemHowto/Intel536EP/
		- https://help.ubuntu.com/community/DialupModemHowto/Intel537EP/
	
2. Copy the Compiled Driver to the same directory as this README.TXT file.
	-Run: cp Intel53*.ko /path/to/directory/
	
3. Edit the proper script as necessary.
	-Modify the variables in 536_prep.sh for packaging the 536ep driver.
	-Modify the variables in 537_prep.sh for packaging the 537ep driver.
	
4. Execute the script to create the deb package.
	-Run: ./536_prep.sh to create 536ep package.
	-Run: ./537_prep.sh to create 537ep package.


