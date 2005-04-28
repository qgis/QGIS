#!/bin/bash

##
## A simple bash script to perform a checkinstall
##

# Accept default answers to all questions
# Set name
# Set version
# Set software group
# The package maintainer (.deb)

checkinstall --exclude ~/.ccache/stats --default --pkgname=qgis-plugin-georeferencing -epkgversion=0.1.pre --pkggroup=GIS --maintainer=tim@linfiniti.com        

