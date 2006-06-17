#!/bin/bash
set -x
if [ -d `pwd`/src ]
then
  #src exists so we are prolly in the right dir
  echo "good we are in  qgis checkout dir!"
else
  echo "You must run this from the top level qgis checkout dir!"
  exit 1
fi

if [ -d `pwd`/debian ]
then
  cd debian
  svn update
  cd ..
else
  svn co https://svn.qgis.org/repos/qgis/trunk/debian
fi

export DEBFULLNAME="Tim Sutton"
export DEBEMAIL=tim@linfiniti.com
dch -v 0.7.9+svn`date +%Y%m%d`
fakeroot dpkg-buildpackage
