#!/bin/bash
#
# A simple script to build QGIS
#       Tim Sutton 2005
#
if [ ! $1 ]
then
echo "Usage: ${0} install_prefix"
echo "e.g."
echo "${0} \$HOME/apps/"
 exit 1
fi 
export QTDIR=/usr/local/Trolltech/Qt-4.1.0/
export PATH=$QTDIR/bin:$PATH
export LD_LIBRARY_PATH=$QTDIR/lib
./autogen.sh --enable-debug --prefix=${1} --with-qtdir=$QTDIR --with-grass=/usr/lib/grass && make && make install
