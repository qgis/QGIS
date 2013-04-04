#!/bin/bash

# This script is used to register QGIS translatable resources with Transifex
# http://transifex.com
#
# Note that this script updates or creates entries in .tx/config file
#
# Tim Sutton, March 2013


LOCALES=`ls i18n/qgis_*.ts| grep -o "qgis_[a-z\_A-Z]*" | sed 's/qgis_//g' | sort | uniq`

TSFILE='i18n/qgis_en.ts'
RESOURCE='i18n/qgis_<lang>.ts'


#qgis-application because no _ allowed in resource name
set -x
tx set -t QT --auto-local -r QGIS.qgis-application \
  $RESOURCE \
  --source-lang en \
  --source $TSFILE \
  --execute

for LOCALE in $LOCALES
do
  LOCALEFILE=`echo $TSFILE | sed "s/\_en/\_$LOCALE/g"`
  tx set -r QGIS.qgis-application -l $LOCALE  "$LOCALEFILE" 
done 

#Print out a listing of all registered resources
tx status

# Push all the resources to the tx server
tx push -s -t --skip
