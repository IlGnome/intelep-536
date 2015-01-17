#!/bin/bash
# Version 0.1.2 Public Domain
# Stephen Pinker

# DO NOT RUN this script directly. This script is meant to be run secondarily to the scripts ./536_prep.sh and ./537_prep.sh

# Modify the variables in ./536_prep.sh or ./537_prep.sh
# Then run which ever of them you choose.



# Setup all variables used.

packagerelease=$packagerelease-Philippe.Vouters-$voutersrelease

# Convert "abc" to "Abc".
ubuntureleasecap=${ubunturelease:0:1}
ubuntureleasecap=$(echo $ubuntureleasecap | tr a-z A-Z)
ubuntureleasecap=$ubuntureleasecap${ubunturelease:1}

ubuntureleasecapold=${ubuntureleaseold:0:1}
ubuntureleasecapold=$(echo $ubuntureleasecapold | tr a-z A-Z)
ubuntureleasecapold=$ubuntureleasecapold${ubuntureleaseold:1}

# Separate kernelrelease into 2 parts (front and back).
kreleasefront=$(echo $kernelrelease | sed "s/-.*//")
kreleaseback=$(echo $kernelrelease | rev | sed "s/-.*//" | rev)

# Go through and find any accidentally left backup files.
if [ -n "$(find $driver -iname '*~')" ]; then
	"Backup files(*~) found in package. Please move or delete them."
	exit 1
fi

# Copy new driver into package and delete old driver.
# Do Not delete old driver if new one does not exist.
if [ -r Intel${driver}.ko ]; then
	OLD_DRIVER=$(ls ${driver}/lib/modules/Intel${driver}*.ko) 2> /dev/null
	mv "$OLD_DRIVER" $(basename "$OLD_DRIVER").$(date +%F_%T).bak 2> /dev/null
	cp Intel${driver}.ko ${driver}/lib/modules/Intel${driver}DriverFor$ubuntureleasecap.ko
	#rm Intel${driver}.ko
else
	echo "No new driver detected: Intel${driver}.ko    continuing..."
fi



cd $driver

# Go inside each file and change occurences of old release name to the new release.
find . -type f -name "*" -print | while read i
do
	sed -i "s/$ubuntureleaseold/$ubunturelease/g" $i
	sed -i "s/$ubuntureleasecapold/$ubuntureleasecap/g" $i
	sleep 0
done
i=DEBIAN/postinst
sed -i "s/2\..*-generic/$kernelrelease-generic/g" $i


# If not yet moved, then move docs to release named location.
[ -a "usr/share/doc/intel${driver}ep-$ubunturelease" ] || \
	mv $(ls -d usr/share/doc/intel${driver}ep-*) "usr/share/doc/intel${driver}ep-$ubunturelease"

# If $driver is 536, and the udev rules file has not yet been moved to the new release name, then move it now.
if [ "$driver" == "536" ]; then
	[ -a "etc/udev/rules.d/10-intel536ep-${ubunturelease}.rules" ] || \
	mv $(ls etc/udev/rules.d/*.rules) etc/udev/rules.d/10-intel536${ubunturelease}.rules
fi

# Insert binaries into md5sum into md5sums file.
if [ "$driver" == "536" ]; then
	md5sum lib/modules/Intel${driver}DriverFor$ubuntureleasecap.ko > DEBIAN/md5sums
else # Else $driver == 537.
	md5sum lib/modules/Intel${driver}DriverFor$ubuntureleasecap.ko usr/sbin/hamregistry usr/sbin/usrsound > DEBIAN/md5sums
fi


# Delete old directories and links.
rm -R lib/modules/2.*

# Create directories and links to driver.
base="lib/modules/$kreleasefront-"
controlfilelist=""
for i in $(seq $kreleaseback $(($kreleaseback + 2)) ); do
	mkdir $base$i-generic
	mkdir $base$i-generic/kernel
	mkdir $base$i-generic/kernel/drivers
	mkdir $base$i-generic/kernel/drivers/char
	ln -s ../../../../Intel${driver}DriverFor$ubuntureleasecap.ko $base$i-generic/kernel/drivers/char/Intel${driver}.ko
	controlfilelist="$controlfilelist | linux-image-$kreleasefront-$i-generic"
done

controlfilelist=${controlfilelist:3} # Trim off front 3 characters.

# Set package release in control file.
i=DEBIAN/control
sed -i "s/^Version:.*/Version: $packagerelease/g" $i

# Substitute new directories into control file.
sed -i "s/^Depends:.*/Depends: grep, module-init-tools, $controlfilelist/g" $i

# Set correct package size in the control file.
packagesize=$(du -s . | sed 's/\t\.//g')
sed -i "s/^Installed-Size:.*/Installed-Size: $packagesize/g" $i

cd ..


# Build Package.
fakeroot dpkg -b $driver "intel${driver}ep-${ubunturelease}_${packagerelease:0:1}-Philippe.Vouters_i386.deb"

