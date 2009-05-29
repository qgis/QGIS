#!/bin/bash

#
# A bash script to create a dmg image file of the 
#            final application bundle
#              (c) Tim Sutton 2007

DMGNAME=QGISAlaskaUncompressed.dmg
COMPRESSEDDMGNAME=QGISAlaska.dmg
set -x

echo "Removing old dmg if it exists"
rm ~/Desktop/${DMGNAME}
rm ~/Desktop/${COMPRESSEDDMGNAME}
hdiutil create -size 300m -fs HFS+ -volname "QGISAlaska" ~/Desktop/${DMGNAME}
 
# Mount the disk image
hdiutil attach ~/Desktop/${DMGNAME}

# Obtain device information
DEVS=$(hdiutil attach ~/Desktop/${DMGNAME} | cut -f 1)
DEV=$(echo $DEVS | cut -f 1 -d ' ')
VOLUME=$(mount |grep ${DEV} | cut -f 3 -d ' ') 
 
# copy in the application bundle
cp -Rp /Applications/QGISAlaska.app ${VOLUME}/QGISAlaska.app

# copy in background image and folder settings for icon sizes etc
tar xvfz alaska_extra_dmg_files.tar.gz -C ${VOLUME} 
cp ../LICENSE ${VOLUME}/LICENSE.txt

# Unmount the disk image
hdiutil detach $DEV
 
# Convert the disk image to read-only
hdiutil convert ~/Desktop/${DMGNAME} \
  -format UDZO -o ~/Desktop/${COMPRESSEDDMGNAME}

