#!/bin/sh
# Copy supporting libraries (except Qt) to qgis bundle
# and make search paths for them relative to bundle

PREFIX=qgis0.9.2.app/Contents/MacOS

HELPPREFIX=$PREFIX/bin/qgis_help.app/Contents/MacOS
PREFIXBACKTRACK=../../../..

# Edit version when any library is upgraded
LIBGDAL=libgdal.1.dylib
LNKGDAL=libgdal.1.dylib
LIBGEOS=libgeos.3.0.0.dylib
LNKGEOS=libgeos.3.dylib
LIBGEOSC=libgeos_c.1.4.1.dylib
LNKGEOSC=libgeos_c.1.dylib
LIBPROJ=libproj.0.5.4.dylib
LNKPROJ=libproj.0.dylib
LIBSQLITE3=libsqlite3.0.8.6.dylib
LNKSQLITE3=libsqlite3.0.dylib
LIBXERCESC=libxerces-c.28.0.dylib
LNKXERCESC=libxerces-c.28.dylib
LIBGIF=libgif.4.1.6.dylib
LNKGIF=libgif.4.dylib
LIBJPEG=libjpeg.62.0.0.dylib
LNKJPEG=libjpeg.62.dylib
LIBPNG=libpng.3.24.0.dylib
LNKPNG=libpng.3.dylib
LIBTIFF=libtiff.3.dylib
LNKTIFF=libtiff.3.dylib
LIBGEOTIFF=libgeotiff.1.2.4.dylib
LNKGEOTIFF=libgeotiff.1.dylib
LIBJASPER=libjasper-1.701.1.0.0.dylib
LNKJASPER=libjasper-1.701.1.dylib
LIBGSL=libgsl.0.9.0.dylib
LNKGSL=libgsl.0.dylib
LIBGSLCBLAS=libgslcblas.0.0.0.dylib
LNKGSLCBLAS=libgslcblas.0.dylib
LIBEXPAT=libexpat.1.5.2.dylib
LNKEXPAT=libexpat.1.dylib
LIBPQ=libpq.5.0.dylib
LNKPQ=libpq.5.dylib
GRASSLIB=/usr/local/grass-6.3.0RC4/lib

# Copy supporting libraries to application bundle
cd $PREFIX/lib
if test ! -f $LIBGEOS; then
	cp /usr/local/lib/$LIBGEOS $LIBGEOS
	ln -s $LIBGEOS $LNKGEOS
	install_name_tool -id @executable_path/lib/$LNKGEOS $LIBGEOS
fi
if test ! -f $LIBGEOSC; then
	cp /usr/local/lib/$LIBGEOSC $LIBGEOSC
	ln -s $LIBGEOSC $LNKGEOSC
	install_name_tool -id @executable_path/lib/$LNKGEOSC $LIBGEOSC
	# Update path to supporting libraries
	install_name_tool -change /usr/local/lib/$LNKGEOS @executable_path/lib/$LNKGEOS $LIBGEOSC
fi
if test ! -f $LIBGDAL; then
	cp /usr/local/lib/$LIBGDAL $LIBGDAL
	#ln -s $LIBGDAL $LNKGDAL
	install_name_tool -id @executable_path/lib/$LNKGDAL $LIBGDAL
	# Update path to supporting libraries
	install_name_tool -change /usr/local/lib/$LNKGEOS @executable_path/lib/$LNKGEOS $LIBGDAL
	install_name_tool -change /usr/local/lib/$LNKGEOSC @executable_path/lib/$LNKGEOSC $LIBGDAL
	install_name_tool -change /usr/local/lib/$LNKSQLITE3 @executable_path/lib/$LNKSQLITE3 $LIBGDAL
	install_name_tool -change /usr/local/lib/$LNKXERCESC @executable_path/lib/$LNKXERCESC $LIBGDAL
	install_name_tool -change /usr/local/lib/$LNKGIF @executable_path/lib/$LNKGIF $LIBGDAL
	install_name_tool -change /usr/local/lib/$LNKJPEG @executable_path/lib/$LNKJPEG $LIBGDAL
	install_name_tool -change /usr/local/lib/$LNKPNG @executable_path/lib/$LNKPNG $LIBGDAL
	install_name_tool -change /usr/local/lib/$LNKTIFF @executable_path/lib/$LNKTIFF $LIBGDAL
	install_name_tool -change /usr/local/lib/$LNKGEOTIFF @executable_path/lib/$LNKGEOTIFF $LIBGDAL
	install_name_tool -change /usr/local/lib/$LNKJASPER @executable_path/lib/$LNKJASPER $LIBGDAL
	install_name_tool -change /usr/local/lib/$LNKEXPAT @executable_path/lib/$LNKEXPAT $LIBGDAL
	install_name_tool -change /usr/local/pgsql/lib/$LNKPQ @executable_path/lib/$LNKPQ $LIBGDAL
	# Copy plugins
	mkdir gdalplugins
	for PLUGIN in gdal_GRASS.so ogr_GRASS.so
	do
		cp /usr/local/lib/gdalplugins/$PLUGIN gdalplugins/$PLUGIN
		install_name_tool -id @executable_path/lib/gdalplugins/$PLUGIN gdalplugins/$PLUGIN
		# Update path to supporting libraries
		install_name_tool -change /usr/local/lib/$LNKGDAL @executable_path/lib/$LNKGDAL gdalplugins/$PLUGIN
	done
	# Copy supporting data files
	cp -R /usr/local/share/gdal ../share
fi
if test ! -f $LIBPROJ; then
	cp /usr/local/lib/$LIBPROJ $LIBPROJ
	ln -s $LIBPROJ $LNKPROJ
	install_name_tool -id @executable_path/lib/$LNKPROJ $LIBPROJ
	# Copy supporting data files
	cp -R /usr/local/share/proj ../share
fi
if test ! -f $LIBSQLITE3; then
	cp /usr/local/lib/$LIBSQLITE3 $LIBSQLITE3
	ln -s $LIBSQLITE3 $LNKSQLITE3
	install_name_tool -id @executable_path/lib/$LNKSQLITE3 $LIBSQLITE3
fi
if test ! -f $LIBXERCESC; then
	cp /usr/local/lib/$LIBXERCESC $LIBXERCESC
	ln -s $LIBXERCESC $LNKXERCESC
	install_name_tool -id @executable_path/lib/$LNKXERCESC $LIBXERCESC
fi
if test ! -f $LIBGIF; then
	cp /usr/local/lib/$LIBGIF $LIBGIF
	ln -s $LIBGIF $LNKGIF
	install_name_tool -id @executable_path/lib/$LNKGIF $LIBGIF
fi
if test ! -f $LIBPNG; then
	cp /usr/local/lib/$LIBPNG $LIBPNG
	ln -s $LIBPNG $LNKPNG
	install_name_tool -id @executable_path/lib/$LNKPNG $LIBPNG
fi
if test ! -f $LIBJPEG; then
	cp /usr/local/lib/$LIBJPEG $LIBJPEG
	ln -s $LIBJPEG $LNKJPEG
	install_name_tool -id @executable_path/lib/$LNKJPEG $LIBJPEG
fi
if test ! -f $LIBTIFF; then
	cp /usr/local/lib/$LIBTIFF $LIBTIFF
	#ln -s $LIBTIFF $LNKTIFF
	install_name_tool -id @executable_path/lib/$LNKTIFF $LIBTIFF
	# Update path to supporting libraries
	install_name_tool -change /usr/local/lib/$LNKJPEG @executable_path/lib/$LNKJPEG $LIBTIFF
fi
if test ! -f $LIBGEOTIFF; then
	cp /usr/local/lib/$LIBGEOTIFF $LIBGEOTIFF
	ln -s $LIBGEOTIFF $LNKGEOTIFF
	install_name_tool -id @executable_path/lib/$LNKGEOTIFF $LIBGEOTIFF
	# Update path to supporting libraries
	install_name_tool -change /usr/local/lib/$LNKJPEG @executable_path/lib/$LNKJPEG $LIBGEOTIFF
	install_name_tool -change /usr/local/lib/$LNKTIFF @executable_path/lib/$LNKTIFF $LIBGEOTIFF
	install_name_tool -change /usr/local/lib/$LNKPROJ @executable_path/lib/$LNKPROJ $LIBGEOTIFF
fi
if test ! -f $LIBJASPER; then
	cp /usr/local/lib/$LIBJASPER $LIBJASPER
	ln -s $LIBJASPER $LNKJASPER
	install_name_tool -id @executable_path/lib/$LNKJASPER $LIBJASPER
	# Update path to supporting libraries
	install_name_tool -change /usr/local/lib/$LNKJPEG @executable_path/lib/$LNKJPEG $LIBJASPER
fi
if test ! -f $LIBGSL; then
	cp /usr/local/lib/$LIBGSL $LIBGSL
	ln -s $LIBGSL $LNKGSL
	install_name_tool -id @executable_path/lib/$LNKGSL $LIBGSL
fi
if test ! -f $LIBGSLCBLAS; then
	cp /usr/local/lib/$LIBGSLCBLAS $LIBGSLCBLAS
	ln -s $LIBGSLCBLAS $LNKGSLCBLAS
	install_name_tool -id @executable_path/lib/$LNKGSLCBLAS $LIBGSLCBLAS
fi
if test ! -f $LIBEXPAT; then
	cp /usr/local/lib/$LIBEXPAT $LIBEXPAT
	ln -s $LIBEXPAT $LNKEXPAT
	install_name_tool -id @executable_path/lib/$LNKEXPAT $LIBEXPAT
fi
if test ! -f $LIBPQ; then
	cp /usr/local/pgsql/lib/$LIBPQ $LIBPQ
	ln -s $LIBPQ $LNKPQ
	install_name_tool -id @executable_path/lib/$LNKPQ $LIBPQ
fi
cd ../../../../

# Update application paths to supporting libraries
install_name_tool -change /usr/local/lib/$LNKGDAL @executable_path/lib/$LNKGDAL $PREFIX/qgis
install_name_tool -change /usr/local/lib/$LNKGEOS @executable_path/lib/$LNKGEOS $PREFIX/qgis
install_name_tool -change /usr/local/lib/$LNKPROJ @executable_path/lib/$LNKPROJ $PREFIX/qgis
install_name_tool -change /usr/local/lib/$LNKSQLITE3 @executable_path/lib/$LNKSQLITE3 $PREFIX/qgis
install_name_tool -change /usr/local/pgsql/lib/$LNKPQ @executable_path/lib/$LNKPQ $PREFIX/qgis

# Update library paths to supporting libraries
for LIB in _core _gui grass
do
	install_name_tool -change /usr/local/lib/$LNKGDAL @executable_path/lib/$LNKGDAL $PREFIX/lib/libqgis$LIB.dylib
	install_name_tool -change /usr/local/lib/$LNKGEOS @executable_path/lib/$LNKGEOS $PREFIX/lib/libqgis$LIB.dylib
	install_name_tool -change /usr/local/lib/$LNKPROJ @executable_path/lib/$LNKPROJ $PREFIX/lib/libqgis$LIB.dylib
	install_name_tool -change /usr/local/lib/$LNKSQLITE3 @executable_path/lib/$LNKSQLITE3 $PREFIX/lib/libqgis$LIB.dylib
done

# Update plugin paths to supporting libraries
for PLUGIN in \
	libcopyrightlabelplugin.so \
	libdelimitedtextplugin.so \
	libdelimitedtextprovider.so \
	libgeorefplugin.so \
	libgpsimporterplugin.so \
	libgpxprovider.so \
	libgrassplugin.so \
	libgrassprovider.so \
	libgridmakerplugin.so \
	libwfsprovider.so \
	libnortharrowplugin.so \
	libogrprovider.so \
	libpggeoprocessingplugin.so \
	libpostgresprovider.so \
	libquickprintplugin.so \
	libscalebarplugin.so \
	libspitplugin.so \
	libwfsplugin.so \
	libwmsprovider.so
 	#libopenmodellerplugin.so
do
	install_name_tool -change /usr/local/lib/$LNKGDAL @executable_path/lib/$LNKGDAL $PREFIX/lib/qgis/$PLUGIN
	install_name_tool -change /usr/local/lib/$LNKGEOS @executable_path/lib/$LNKGEOS $PREFIX/lib/qgis/$PLUGIN
	install_name_tool -change /usr/local/lib/$LNKPROJ @executable_path/lib/$LNKPROJ $PREFIX/lib/qgis/$PLUGIN
	install_name_tool -change /usr/local/lib/$LNKSQLITE3 @executable_path/lib/$LNKSQLITE3 $PREFIX/lib/qgis/$PLUGIN
done

for PLUGIN in \
	libgpxprovider.so \
	libwfsprovider.so
do
	install_name_tool -change /usr/local/lib/$LNKEXPAT @executable_path/lib/$LNKEXPAT $PREFIX/lib/qgis/$PLUGIN
done

install_name_tool -change /usr/local/lib/$LNKGSL @executable_path/lib/$LNKGSL $PREFIX/lib/qgis/libgeorefplugin.so
install_name_tool -change /usr/local/lib/$LNKGSLCBLAS @executable_path/lib/$LNKGSLCBLAS $PREFIX/lib/qgis/libgeorefplugin.so

for PLUGIN in \
	libpggeoprocessingplugin.so \
	libpostgresprovider.so \
	libspitplugin.so
do
	install_name_tool -change /usr/local/pgsql/lib/$LNKPQ @executable_path/lib/$LNKPQ $PREFIX/lib/qgis/$PLUGIN
done

# Update GRASS plugins paths to GRASS libraries
for PLUGIN in \
	libqgisgrass.dylib \
	qgis/libgrassplugin.so \
	qgis/libgrassprovider.so
do
	for LIB in datetime dbmibase dbmiclient dgl dig2 form gis gproj linkm rtree shape vect
	do
		install_name_tool -change $GRASSLIB/libgrass_$LIB.dylib \
			@executable_path/lib/grass/libgrass_$LIB.dylib \
			$PREFIX/lib/$PLUGIN
	done
done
for PLUGIN in \
	gdalplugins/gdal_GRASS.so \
	gdalplugins/ogr_GRASS.so
do
	for LIB in datetime dbmibase dbmiclient dgl dig2 gis gmath gproj I linkm rtree vask vect
	do
		install_name_tool -change $GRASSLIB/libgrass_$LIB.dylib \
			@executable_path/lib/grass/libgrass_$LIB.dylib \
			$PREFIX/lib/$PLUGIN
	done
done

# Update qgis_help application paths to supporting libraries
install_name_tool -change /usr/local/lib/$LNKGDAL @executable_path/lib/$LNKGDAL $HELPPREFIX/qgis_help
install_name_tool -change /usr/local/lib/$LNKGEOS @executable_path/lib/$LNKGEOS $HELPPREFIX/qgis_help
install_name_tool -change /usr/local/lib/$LNKPROJ @executable_path/lib/$LNKPROJ $HELPPREFIX/qgis_help
install_name_tool -change /usr/local/lib/$LNKSQLITE3 @executable_path/lib/$LNKSQLITE3 $HELPPREFIX/qgis_help
ln -sf $PREFIXBACKTRACK/lib $HELPPREFIX/lib
