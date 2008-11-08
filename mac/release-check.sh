#!/bin/sh
# Display all paths to supporting libraries
# Output should be visually inspected for paths which haven't been made relative (such as /usr/local)

PREFIX=qgis1.0.0.app/Contents/MacOS

otool -L $PREFIX/qgis
otool -L $PREFIX/bin/qgis_help.app/Contents/MacOS/qgis_help

otool -L $PREFIX/lib/libqgis_core.dylib
otool -L $PREFIX/lib/libqgis_gui.dylib
otool -L $PREFIX/lib/qgis/libcoordinatecaptureplugin.so
otool -L $PREFIX/lib/qgis/libcopyrightlabelplugin.so 
otool -L $PREFIX/lib/qgis/libdelimitedtextplugin.so
otool -L $PREFIX/lib/qgis/libdelimitedtextprovider.so
otool -L $PREFIX/lib/qgis/libdxf2shpconverterplugin.so
otool -L $PREFIX/lib/qgis/libgeorefplugin.so 
otool -L $PREFIX/lib/qgis/libgpsimporterplugin.so
otool -L $PREFIX/lib/qgis/libgpxprovider.so
otool -L $PREFIX/lib/qgis/libgridmakerplugin.so 
otool -L $PREFIX/lib/qgis/libinterpolationplugin.so
otool -L $PREFIX/lib/qgis/libmemoryprovider.so
otool -L $PREFIX/lib/qgis/libnortharrowplugin.so
otool -L $PREFIX/lib/qgis/libogrconverterplugin.so
otool -L $PREFIX/lib/qgis/libogrprovider.so
otool -L $PREFIX/lib/qgis/libpostgresprovider.so
otool -L $PREFIX/lib/qgis/libscalebarplugin.so
otool -L $PREFIX/lib/qgis/libspitplugin.so
otool -L $PREFIX/lib/qgis/libwfsplugin.so
otool -L $PREFIX/lib/qgis/libwfsprovider.so
otool -L $PREFIX/lib/qgis/libwmsprovider.so

otool -L $PREFIX/lib/Qt3Support.framework/Versions/4/Qt3Support
otool -L $PREFIX/lib/QtCore.framework/Versions/4/QtCore
otool -L $PREFIX/lib/QtGui.framework/Versions/4/QtGui
otool -L $PREFIX/lib/QtNetwork.framework/Versions/4/QtNetwork
otool -L $PREFIX/lib/QtSql.framework/Versions/4/QtSql
otool -L $PREFIX/lib/QtSvg.framework/Versions/4/QtSvg
otool -L $PREFIX/lib/QtXml.framework/Versions/4/QtXml
otool -L $PREFIX/../plugins/imageformats/libqjpeg.dylib

otool -L $PREFIX/lib/libgdal.1.dylib
otool -L $PREFIX/lib/libgeos.3.0.1.dylib
otool -L $PREFIX/lib/libgeos_c.1.4.2.dylib
otool -L $PREFIX/lib/libproj.0.5.5.dylib
otool -L $PREFIX/lib/libsqlite3.0.dylib
otool -L $PREFIX/lib/libxerces-c.28.0.dylib
otool -L $PREFIX/lib/libgif.4.1.6.dylib
otool -L $PREFIX/lib/libjpeg.62.0.0.dylib
otool -L $PREFIX/lib/libpng12.0.dylib
otool -L $PREFIX/lib/libtiff.3.dylib
otool -L $PREFIX/lib/libgeotiff.1.2.4.dylib
otool -L $PREFIX/lib/libjasper.1.0.0.dylib
otool -L $PREFIX/lib/libexpat.1.5.2.dylib
otool -L $PREFIX/lib/libfftw3.3.1.3.dylib
otool -L $PREFIX/lib/libgsl.0.dylib
otool -L $PREFIX/lib/libgslcblas.0.dylib
otool -L $PREFIX/lib/libpq.5.1.dylib

if test -f $PREFIX/lib/libqgisgrass.dylib; then
	otool -L $PREFIX/lib/libqgisgrass.dylib

	otool -L $PREFIX/lib/qgis/libgrassplugin.so 
	otool -L $PREFIX/lib/qgis/libgrassprovider.so 

	otool -L $PREFIX/lib/gdalplugins/gdal_GRASS.so
	otool -L $PREFIX/lib/gdalplugins/ogr_GRASS.so

	for LIBGRASS in datetime dbmibase dbmiclient dgl dig2 form gis gmath gproj I linkm rtree shape vask vect
	do
		otool -L $PREFIX/lib/grass/libgrass_$LIBGRASS.6.3.0.dylib
	done
fi

if test -f $PREFIX/lib/libqgispython.dylib; then
	otool -L $PREFIX/lib/libqgispython.dylib

	otool -L $PREFIX/share/qgis/python/qgis/core.so
	otool -L $PREFIX/share/qgis/python/qgis/gui.so

	for FRAMEWORK in QtAssistant QtHelp QtOpenGL QtScript QtTest QtWebKit QtXmlPatterns phonon
	do
		otool -L $PREFIX/lib/$FRAMEWORK.framework/Versions/4/$FRAMEWORK
	done
	otool -L $PREFIX/lib/libQtCLucene.4.4.3.dylib

	otool -L $PREFIX/share/qgis/python/sip.so
	for LIBPYQT4 in Qt QtCore QtGui QtNetwork QtSql QtSvg QtXml QtAssistant QtHelp QtOpenGL QtScript QtTest QtWebKit QtXmlPatterns phonon
	do
		otool -L $PREFIX/share/qgis/python/PyQt4/$LIBPYQT4.so
	done
fi

