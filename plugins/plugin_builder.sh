#!/bin/bash

##################################################
# A script to automate creation of a new plugin
# using the plugin_template
##################################################


echo "Useage $0 [NewPluginDir] [PluginName] [\"Plugin description\"] [\"Menu Name\"] [\"Menu Item Name\"]"
echo "New dir : $1"
echo "New plugin name : $2"
echo "New plugin description : $3"
echo "New menu name : $4"
echo "New menu item : $5"
## 
## Copy the new plugin from the template dir
##
cp -r plugin_template $1

##
## Set the new plugin dir as the working dir
##

cd ${1}

echo `pwd`

##
## Sustitute and plugin specific vars
##
find *.cpp *.h *.am -type f | xargs  perl -pi -e 's/\[pluginname]/${2}/g'
find *.cpp *.h *.am -type f | xargs  perl -pi -e 's/\[plugindescription]/${3}/g'
find *.cpp *.h *.am -type f | xargs  perl -pi -e 's/\[menuname]/${4}/g'
find *.cpp *.h *.am -type f | xargs  perl -pi -e 's/\[menuitemname]/${5}/g'

##
## Add an entry to the qgis/plugins/Makefile.am
##

## DO ME!

##
## Add an entry to the qgis/configure.in 
##

## DO ME!
