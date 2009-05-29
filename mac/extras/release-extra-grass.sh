#!/bin/sh
# Copy GRASS supporting libraries to qgis bundle
# and make search paths for them relative to bundle

PREFIX=qgis1.0.0.app/Contents/MacOS

# Edit version when any library is upgraded
LNKGDAL=libgdal.1.dylib
LNKPROJ=libproj.0.dylib
LIBFFTW=libfftw3.3.1.3.dylib
LNKFFTW=libfftw3.3.dylib
GRASSVER=6.3.0
GRASSLIB=/usr/local/grass-$GRASSVER/lib

cd $PREFIX/lib

# Copy supporting libraries to application bundle
if test ! -d grass; then
	mkdir grass
	for LIBGRASS in datetime dbmibase dbmiclient dgl dig2 form gis gmath gproj I linkm rtree shape vask vect
	do
		LIB=libgrass_$LIBGRASS.$GRASSVER.dylib
		LNK=libgrass_$LIBGRASS.dylib
		cp $GRASSLIB/$LIB grass/$LIB
		ln -s $LIB grass/$LNK
		install_name_tool -id @executable_path/lib/grass/$LNK grass/$LIB
	done
fi
if test ! -f $LIBFFTW; then
	cp /usr/local/lib/$LIBFFTW $LIBFFTW
	ln -s $LIBFFTW $LNKFFTW
	install_name_tool -id @executable_path/lib/$LNKFFTW $LIBFFTW
fi

# Update library paths to supporting libraries
install_name_tool -change $GRASSLIB/libgrass_datetime.dylib \
	@executable_path/lib/grass/libgrass_datetime.dylib \
	grass/libgrass_gis.$GRASSVER.dylib
for LIBGRASS in dbmibase dbmiclient dig2 form gmath gproj I vask
do
	install_name_tool -change $GRASSLIB/libgrass_datetime.dylib \
		@executable_path/lib/grass/libgrass_datetime.dylib \
		grass/libgrass_$LIBGRASS.$GRASSVER.dylib
	install_name_tool -change $GRASSLIB/libgrass_gis.dylib \
		@executable_path/lib/grass/libgrass_gis.dylib \
		grass/libgrass_$LIBGRASS.$GRASSVER.dylib
done
for LIBGRASS in dbmiclient form
do
	install_name_tool -change $GRASSLIB/libgrass_dbmibase.dylib \
		@executable_path/lib/grass/libgrass_dbmibase.dylib \
		grass/libgrass_$LIBGRASS.$GRASSVER.dylib
done
install_name_tool -change $GRASSLIB/libgrass_dbmiclient.dylib \
	@executable_path/lib/grass/libgrass_dbmiclient.dylib \
	grass/libgrass_form.$GRASSVER.dylib
install_name_tool -change $GRASSLIB/libgrass_gmath.dylib \
	@executable_path/lib/grass/libgrass_gmath.dylib \
	grass/libgrass_I.$GRASSVER.dylib
install_name_tool -change $GRASSLIB/libgrass_rtree.dylib \
	@executable_path/lib/grass/libgrass_rtree.dylib \
	grass/libgrass_dig2.$GRASSVER.dylib
install_name_tool -change $GRASSLIB/libgrass_vask.dylib \
	@executable_path/lib/grass/libgrass_vask.dylib \
	grass/libgrass_I.$GRASSVER.dylib
for LIBGRASS in datetime dbmibase dbmiclient gis dgl dig2 linkm rtree
do
	install_name_tool -change $GRASSLIB/libgrass_$LIBGRASS.dylib \
		@executable_path/lib/grass/libgrass_$LIBGRASS.dylib \
		grass/libgrass_vect.$GRASSVER.dylib
done

install_name_tool -change /usr/local/lib/$LNKFFTW @executable_path/lib/$LNKFFTW grass/libgrass_gmath.$GRASSVER.dylib
install_name_tool -change /usr/local/lib/$LNKGDAL @executable_path/lib/$LNKGDAL grass/libgrass_gproj.$GRASSVER.dylib
install_name_tool -change /usr/local/lib/$LNKPROJ @executable_path/lib/$LNKPROJ grass/libgrass_gproj.$GRASSVER.dylib
install_name_tool -change /usr/local/lib/$LNKGDAL @executable_path/lib/$LNKGDAL grass/libgrass_vect.$GRASSVER.dylib

cd ../../../../
