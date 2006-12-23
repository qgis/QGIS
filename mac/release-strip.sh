#!/bin/sh
# Strip all non-global symbols

PREFIX=qgis.app/Contents/MacOS

strip -x $PREFIX/qgis
strip -x $PREFIX/bin/qgis-config
strip -x $PREFIX/bin/qgis_help.app/Contents/MacOS/qgis_help
strip -x $PREFIX/bin/msexport.app/Contents/MacOS/msexport
#strip -x $PREFIX/bin/gridmaker
#strip -x $PREFIX/bin/spit
#strip -x $PREFIX/bin/omgui

strip -x $PREFIX/lib/libmsexport.1.0.0.dylib
strip -x $PREFIX/lib/libqgis_composer.1.0.0.dylib
strip -x $PREFIX/lib/libqgis_core.1.0.0.dylib
strip -x $PREFIX/lib/libqgis_gui.1.0.0.dylib
strip -x $PREFIX/lib/libqgis_legend.1.0.0.dylib
strip -x $PREFIX/lib/libqgis_raster.1.0.0.dylib
strip -x $PREFIX/lib/libqgisgrass.1.0.0.dylib
strip -x $PREFIX/lib/qgis/copyrightlabelplugin.so 
strip -x $PREFIX/lib/qgis/delimitedtextplugin.so
strip -x $PREFIX/lib/qgis/delimitedtextprovider.so
strip -x $PREFIX/lib/qgis/georefplugin.so 
strip -x $PREFIX/lib/qgis/gpsimporterplugin.so
strip -x $PREFIX/lib/qgis/gpxprovider.so
strip -x $PREFIX/lib/qgis/grassplugin.so 
strip -x $PREFIX/lib/qgis/grassprovider.so 
strip -x $PREFIX/lib/qgis/gridmakerplugin.so 
strip -x $PREFIX/lib/qgis/libwfsprovider.so
strip -x $PREFIX/lib/qgis/northarrowplugin.so
strip -x $PREFIX/lib/qgis/ogrprovider.so
strip -x $PREFIX/lib/qgis/pggeoprocessingplugin.so
strip -x $PREFIX/lib/qgis/postgresprovider.so
strip -x $PREFIX/lib/qgis/scalebarplugin.so
strip -x $PREFIX/lib/qgis/spitplugin.so
strip -x $PREFIX/lib/qgis/wfsplugin.so
strip -x $PREFIX/lib/qgis/wmsprovider.so
strip -x $PREFIX/lib/qgis/libqgsprojectionselector.dylib
#strip -x $PREFIX/lib/qgis/libopenmodellerplugin.so 

strip -x $PREFIX/lib/Qt3Support.framework/Versions/4/Qt3Support
strip -x $PREFIX/lib/QtCore.framework/Versions/4/QtCore
strip -x $PREFIX/lib/QtGui.framework/Versions/4/QtGui
strip -x $PREFIX/lib/QtNetwork.framework/Versions/4/QtNetwork
strip -x $PREFIX/lib/QtSql.framework/Versions/4/QtSql
strip -x $PREFIX/lib/QtSvg.framework/Versions/4/QtSvg
strip -x $PREFIX/lib/QtXml.framework/Versions/4/QtXml
strip -x $PREFIX/lib/QtDesigner.framework/Versions/4/QtDesigner
strip -x $PREFIX/lib/QtTest.framework/Versions/4/QtTest
strip -x $PREFIX/imageformats/libqjpeg.dylib

strip -x $PREFIX/lib/libgdal.1.10.0.dylib
strip -x $PREFIX/lib/gdalplugins/gdal_GRASS.so
strip -x $PREFIX/lib/gdalplugins/ogr_GRASS.so
strip -x $PREFIX/lib/libgeos.2.2.3.dylib
strip -x $PREFIX/lib/libproj.0.5.0.dylib
strip -x $PREFIX/lib/libsqlite3.0.8.6.dylib
strip -x $PREFIX/lib/libxerces-c.27.0.dylib
strip -x $PREFIX/lib/libgif.4.1.4.dylib
strip -x $PREFIX/lib/libjpeg.62.0.0.dylib
strip -x $PREFIX/lib/libpng.3.1.2.8.dylib
strip -x $PREFIX/lib/libtiff.3.dylib
strip -x $PREFIX/lib/libgeotiff.1.2.3.dylib
strip -x $PREFIX/lib/libjasper-1.701.1.0.0.dylib
strip -x $PREFIX/lib/libexpat.1.5.0.dylib
strip -x $PREFIX/lib/libgsl.0.9.0.dylib
strip -x $PREFIX/lib/libgslcblas.0.0.0.dylib
#strip -x $PREFIX/lib/libopenmodeller.0.0.0.dylib
#strip -x $PREFIX/lib/openmodeller/libombioclim.0.0.0.dylib
#strip -x $PREFIX/lib/openmodeller/libombioclim_distance.0.0.0.dylib
#strip -x $PREFIX/lib/openmodeller/libomcsmbs.0.0.0.dylib
#strip -x $PREFIX/lib/openmodeller/libomdg_bs.0.0.0.dylib
#strip -x $PREFIX/lib/openmodeller/libomdistance_to_average.0.0.0.dylib
#strip -x $PREFIX/lib/openmodeller/libomminimum_distance.0.0.0.dylib
#strip -x $PREFIX/lib/openmodeller/libomoldgarp.0.0.0.dylib
strip -x $PREFIX/lib/libpq.5.0.dylib

for LIBGRASS in datetime dbmibase dbmiclient dgl dig2 form gis gmath gproj I linkm rtree shape vask vect
do
	strip -x $PREFIX/lib/grass/libgrass_$LIBGRASS.6.2.1.dylib
done

# Delete unneeded files in application bundle
rm $PREFIX/lib/libmsexport.a
rm $PREFIX/lib/libmsexport.dylib
rm $PREFIX/lib/libmsexport.la
rm $PREFIX/lib/libqgis_*.a
rm $PREFIX/lib/libqgis_composer.dylib
rm $PREFIX/lib/libqgis_core.dylib
rm $PREFIX/lib/libqgis_gui.dylib
rm $PREFIX/lib/libqgis_legend.dylib
rm $PREFIX/lib/libqgis_raster.dylib
rm $PREFIX/lib/libqgis_*.la
rm $PREFIX/lib/libqgisgrass.a
rm $PREFIX/lib/libqgisgrass.dylib
rm $PREFIX/lib/libqgisgrass.la
rm $PREFIX/lib/qgis/*.a
rm $PREFIX/lib/qgis/*.la
