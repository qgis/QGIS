#!/bin/sh
# create_qm_files.sh,v 1.4 2004/07/14 18:16:00 gsherman Exp
# Create the .qm files from the translation files
# 1. create a clean Qt .pro file for the project
# 2. run lrelease using the .pro file from step 1
# 3. remove the .pro
# Note the .pro file must NOT be named qgis.pro as this
# name is reserved for the Windows qmake project file
# create_qm_files.sh,v 1.4 2004/07/14 18:16:00 gsherman Exp
echo Creating qmake project file
# force the output name to be qgis_qm.pro
/usr/bin/qmake -project -o qgis_qm.pro
echo Creating qm files
lrelease -verbose qgis_qm.pro
echo Removing qmake project file
rm qgis_qm.pro
