#!/bin/sh
# Update the translation files with strings used in QGIS
# 1. create a clean Qt .pro file for the project
# 2. run lupdate using the .pro file from step 1
# 3. remove the .pro
echo Creating qmake project file
qmake -project
echo Updating translation files
lupdate -verbose qgis.pro
echo Removing qmake project file
rm qgis.pro
