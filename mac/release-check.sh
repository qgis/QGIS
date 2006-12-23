#!/bin/sh
# Display all paths to supporting libraries
# Output should be visually inspected for paths which haven't been made relative (such as /usr/local)

PREFIX=qgis.app/Contents/MacOS

otool -L $PREFIX/qgis
otool -L $PREFIX/bin/qgis-config
otool -L $PREFIX/bin/qgis_help.app/Contents/MacOS/qgis_help
otool -L $PREFIX/bin/msexport.app/Contents/MacOS/msexport
#otool -L $PREFIX/bin/gridmaker
#otool -L $PREFIX/bin/spit
#otool -L $PREFIX/bin/omgui

otool -L $PREFIX/lib/libmsexport.1.0.0.dylib
otool -L $PREFIX/lib/libqgis_composer.1.0.0.dylib
otool -L $PREFIX/lib/libqgis_core.1.0.0.dylib
otool -L $PREFIX/lib/libqgis_gui.1.0.0.dylib
otool -L $PREFIX/lib/libqgis_legend.1.0.0.dylib
otool -L $PREFIX/lib/libqgis_raster.1.0.0.dylib
otool -L $PREFIX/lib/libqgisgrass.1.0.0.dylib
otool -L $PREFIX/lib/qgis/copyrightlabelplugin.so 
otool -L $PREFIX/lib/qgis/delimitedtextplugin.so
otool -L $PREFIX/lib/qgis/delimitedtextprovider.so
otool -L $PREFIX/lib/qgis/georefplugin.so 
otool -L $PREFIX/lib/qgis/gpsimporterplugin.so
otool -L $PREFIX/lib/qgis/gpxprovider.so
otool -L $PREFIX/lib/qgis/grassplugin.so 
otool -L $PREFIX/lib/qgis/grassprovider.so 
otool -L $PREFIX/lib/qgis/gridmakerplugin.so 
otool -L $PREFIX/lib/qgis/libwfsprovider.so
otool -L $PREFIX/lib/qgis/northarrowplugin.so
otool -L $PREFIX/lib/qgis/ogrprovider.so
otool -L $PREFIX/lib/qgis/pggeoprocessingplugin.so
otool -L $PREFIX/lib/qgis/postgresprovider.so
otool -L $PREFIX/lib/qgis/scalebarplugin.so
otool -L $PREFIX/lib/qgis/spitplugin.so
otool -L $PREFIX/lib/qgis/wfsplugin.so
otool -L $PREFIX/lib/qgis/wmsprovider.so
otool -L $PREFIX/lib/qgis/libqgsprojectionselector.dylib
#otool -L $PREFIX/lib/qgis/libopenmodellerplugin.so 

otool -L $PREFIX/lib/Qt3Support.framework/Versions/4/Qt3Support
otool -L $PREFIX/lib/QtCore.framework/Versions/4/QtCore
otool -L $PREFIX/lib/QtGui.framework/Versions/4/QtGui
otool -L $PREFIX/lib/QtNetwork.framework/Versions/4/QtNetwork
otool -L $PREFIX/lib/QtSql.framework/Versions/4/QtSql
otool -L $PREFIX/lib/QtSvg.framework/Versions/4/QtSvg
otool -L $PREFIX/lib/QtXml.framework/Versions/4/QtXml
otool -L $PREFIX/lib/QtDesigner.framework/Versions/4/QtDesigner
otool -L $PREFIX/lib/QtTest.framework/Versions/4/QtTest
otool -L $PREFIX/imageformats/libqjpeg.dylib

otool -L $PREFIX/lib/libgdal.1.10.0.dylib
otool -L $PREFIX/lib/gdalplugins/gdal_GRASS.so
otool -L $PREFIX/lib/gdalplugins/ogr_GRASS.so
otool -L $PREFIX/lib/libgeos.2.2.3.dylib
otool -L $PREFIX/lib/libproj.0.5.0.dylib
otool -L $PREFIX/lib/libsqlite3.0.8.6.dylib
otool -L $PREFIX/lib/libxerces-c.27.0.dylib
otool -L $PREFIX/lib/libgif.4.1.4.dylib
otool -L $PREFIX/lib/libjpeg.62.0.0.dylib
otool -L $PREFIX/lib/libpng.3.1.2.8.dylib
otool -L $PREFIX/lib/libtiff.3.dylib
otool -L $PREFIX/lib/libgeotiff.1.2.3.dylib
otool -L $PREFIX/lib/libjasper-1.701.1.0.0.dylib
otool -L $PREFIX/lib/libexpat.1.5.0.dylib
otool -L $PREFIX/lib/libgsl.0.9.0.dylib
otool -L $PREFIX/lib/libgslcblas.0.0.0.dylib
#otool -L $PREFIX/lib/libopenmodeller.0.0.0.dylib
#otool -L $PREFIX/lib/openmodeller/libombioclim.0.0.0.dylib
#otool -L $PREFIX/lib/openmodeller/libombioclim_distance.0.0.0.dylib
#otool -L $PREFIX/lib/openmodeller/libomcsmbs.0.0.0.dylib
#otool -L $PREFIX/lib/openmodeller/libomdg_bs.0.0.0.dylib
#otool -L $PREFIX/lib/openmodeller/libomdistance_to_average.0.0.0.dylib
#otool -L $PREFIX/lib/openmodeller/libomminimum_distance.0.0.0.dylib
#otool -L $PREFIX/lib/openmodeller/libomoldgarp.0.0.0.dylib
otool -L $PREFIX/lib/libpq.5.0.dylib

for LIBGRASS in datetime dbmibase dbmiclient dgl dig2 form gis gmath gproj I linkm rtree shape vask vect
do
	otool -L $PREFIX/lib/grass/libgrass_$LIBGRASS.6.2.1.dylib
done
