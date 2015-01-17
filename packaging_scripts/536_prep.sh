#!/bin/bash
# Version 0.1.2 Public Domain
# Stephen Pinker

# ./536_prep.sh
# Edit this script as necessary. Copy the new driver to the same dir as this script, then run this script.

export ubuntureleaseold="lucid" # Ex "hardy". Use lowercase. (If you're only making a single package, don't bother to change this.)
export ubunturelease="lucid" # Ex "jaunty". Use lowercase. (Here should be the name of the release this package is made for.)
export kernelrelease="2.6.32-21" # Ex "2.6.27-11", leave off "generic".
export packagerelease="1" # Ex "2". This package release number.
# Ex "Feb2009". Release date of Philippe Vouters tar ball. No spaces.
export voutersrelease="Jan2010"

export driver="536" # Don't edit this.

./prep_release.sh
