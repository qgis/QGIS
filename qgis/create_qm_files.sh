#!/bin/sh
# Create the .qm files from the translation files
# 1. create a clean Qt .pro file for the project
# 2. run lrelease using the .pro file from step 1
# 3. remove the .pro
echo Creating qmake project file
qmake -project
echo Creating qm files
lrelease -verbose qgis.pro
echo Removing qmake project file
rm qgis.pro
