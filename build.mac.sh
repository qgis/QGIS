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
   echo "Building with debug support"
   AUTOGEN_FLAGS="--enable-debug"
   export CFLAGS="-g -Wall"
   export CXXFLAGS="-g -Wall"
   #for mac fink users
   #export CPPFLAGS=-I/sw/include
 else
   echo "Building without debug support"
   AUTOGEN_FLAGS="--disable-debug"
   export CFLAGS="-O2 -Wall"
   export CXXFLAGS="-O2 -Wall"
   #for mac fink users
   #export CPPFLAGS=-I/sw/include
 fi
 
 if [ x$2 = "static" ]
 then
   echo "Building with qgis statically linked to dependencies"
   #this is to tell linker to statically linke
   #against deps like gdal etc - useful for
   #trying to build a distributeable binary of qgis
   export LDFLAGS=-static
 fi
  
 #qt installed from source
 #export QTDIR=/usr/local/Trolltech/Qt-4.1.0
 #qt installed from debian apt
 #export QTDIR=/usr
 export QTDIR=/usr/local/Trolltech/Qt-4.1.4
 #export PATH=/usr/local/gislibs/bin:/usr/local/graphicslibs/bin/:$QTDIR/bin:$PATH
 export LD_LIBRARY_PATH=$QTDIR/lib

 # Note: --enable-static=no tells compiler 
 # 'dont build static versions of qgis .o files'
 # This only applies to qgis interal libs and speeds 
 # up the compilation process. See discussion on 
 # http://logs.qgis.org/slogs/%23qgis.2006-04-15.log at 17:06:10
 # for additional details
 ./autogen.sh $AUTOGEN_FLAGS --prefix=${1} \
                             --enable-static=no \
                             --with-qtdir=$QTDIR \
                             --with-python
                             #--with-grass=/usr/local/gislibs/grass
                             #--enable-unittests \
make && make install

cd tests/src
./runtests.sh
