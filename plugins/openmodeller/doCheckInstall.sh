#!/bin/bash

##
## A simple bash script to perform a checkinstall
##

# Accept default answers to all questions
# Set name
# Set version
# Set software group
# The package maintainer (.deb)

checkinstall --default --pkgname=qgis-plugin-openmodeller --pkgversion=0.3.1 --pkggroup=GIS --maintainer=tim@linfiniti.com        

#
# Write rpm even if on debian (-R)
#
#checkinstall --default -R --pkgname=openmodeller-gui --pkgversion=0.2.1pre --pkggroup=GIS --maintainer=tim@linfiniti.com        

