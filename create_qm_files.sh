#!/bin/sh
# create_qm_files.sh,v 1.2 2004/04/22 05:25:10 gsherman Exp
# Create the .qm files from the translation files
# 1. create a clean Qt .pro file for the project
# 2. run lrelease using the .pro file from step 1
# 3. remove the .pro
echo Creating qmake project file
# force the output name to be qgis_ts.pro
qmake -project -o qgis_ts.pro
echo Creating qm files
lrelease -verbose qgis_ts.pro
echo Removing qmake project file
rm qgis_ts.pro
