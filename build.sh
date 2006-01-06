#!/bin/bash
#
# A simple script to build QGIS
#       Tim Sutton 2005-2006
#       and Martin Dobias 2006
#
if [ ! $1 ]
then
  echo "Usage: ${0} install_prefix"
  echo "e.g."
  echo "${0} \$HOME/apps/"
   exit 1
 fi
  
 AUTOGEN_FLAGS=
  
 # for debug build:
 # - disable default optimisations to improve debugging (omitting default -O2 flags)
 # - show warnings
 if [ x$2 = xdebug ]
 then
   AUTOGEN_FLAGS="--enable-debug"
   export CFLAGS="-g -Wall"
   export CXXFLAGS="-g -Wall"
 fi
  
 export QTDIR=/usr/local/Trolltech/Qt-4.1.0/
 export PATH=$QTDIR/bin:$PATH
 export LD_LIBRARY_PATH=$QTDIR/lib
 ./autogen.sh $AUTOGEN_FLAGS --prefix=${1} --with-qtdir=$QTDIR --with-grass=/usr/lib/grass && make && make install

