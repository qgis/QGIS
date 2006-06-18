#!/bin/sh
# Make all libs and plugins in qgis bundle relative to bundle
# This allows the bundle to be moved to another folder and still work

# Edit CONFIGPREFIX to match the value of ./configure --prefix
CONFIGPREFIX=$PWD

BUNDLE=qgis.app/Contents/MacOS
BUILDPREFIX=$CONFIGPREFIX/$BUNDLE

QLIBNAMES="composer core gui legend"

# Declare libqgis_* relative to bundle and update qgis app client
for NAME in $QLIBNAMES
do
	install_name_tool -id @executable_path/lib/libqgis_$NAME.0.0.1.dylib \
		$BUILDPREFIX/lib/libqgis_$NAME.0.0.1.dylib

	install_name_tool -change $BUILDPREFIX/lib/libqgis_$NAME.0.dylib \
		@executable_path/lib/libqgis_$NAME.0.dylib \
		$BUILDPREFIX/qgis
done

# Declare libqgis_libqgis_raster relative to bundle and update qgis app client
# (libqgis_raster has different version than libqgis_*)
install_name_tool -id @executable_path/lib/libqgis_raster.0.0.0.dylib \
	$BUILDPREFIX/lib/libqgis_raster.0.0.0.dylib

install_name_tool -change $BUILDPREFIX/lib/libqgis_raster.0.dylib \
	@executable_path/lib/libqgis_raster.0.dylib \
	$BUILDPREFIX/qgis

# Declare libqgsprojectionselector relative to bundle and update qgis app client
install_name_tool -id @executable_path/lib/qgis/libqgsprojectionselector.dylib \
	$BUILDPREFIX/lib/qgis/libqgsprojectionselector.dylib

install_name_tool -change $BUILDPREFIX/lib/qgis/libqgsprojectionselector.dylib \
	@executable_path/lib/qgis/libqgsprojectionselector.dylib \
	$BUILDPREFIX/qgis

# Update libqgis_gui client of libqgis_*
for LIB in libqgis_composer.0.dylib libqgis_core.0.dylib libqgis_legend.0.dylib \
	libqgis_raster.0.dylib qgis/libqgsprojectionselector.dylib
do
	install_name_tool -change $BUILDPREFIX/lib/$LIB \
		@executable_path/lib/$LIB \
		$BUILDPREFIX/lib/libqgis_gui.0.0.1.dylib
done

# Update plugin and lib clients of libqgis_* and libqgsprojectionselector
for PLUGIN in \
	qgis/copyrightlabelplugin.so \
	qgis/delimitedtextplugin.so \
	qgis/delimitedtextprovider.so \
	qgis/georefplugin.so \
	qgis/gpsimporterplugin.so \
	qgis/gpxprovider.so \
	qgis/grassplugin.so \
	qgis/grassprovider.so \
	qgis/gridmakerplugin.so \
	qgis/libScaleBarplugin.so \
	qgis/northarrowplugin.so \
	qgis/ogrprovider.so \
	qgis/pggeoprocessingplugin.so \
	qgis/postgresprovider.so \
	qgis/spitplugin.so \
	qgis/wmsprovider.so \
	qgis/libqgsprojectionselector.dylib \
	libqgisgrass.0.0.1.dylib
do
	for NAME in $QLIBNAMES raster
	do
		install_name_tool -change $BUILDPREFIX/lib/libqgis_$NAME.0.dylib \
			@executable_path/lib/libqgis_$NAME.0.dylib \
			$BUILDPREFIX/lib/$PLUGIN
	done
	install_name_tool -change $BUILDPREFIX/lib/qgis/libqgsprojectionselector.dylib \
		@executable_path/lib/qgis/libqgsprojectionselector.dylib \
		$BUILDPREFIX/lib/$PLUGIN
done

# Declare libqgisgrass relative to bundle
install_name_tool -id @executable_path/lib/libqgisgrass.0.0.1.dylib \
	$BUILDPREFIX/lib/libqgisgrass.0.0.1.dylib

# Update plugin clients of libqgisgrass
for PLUGIN in qgis/grassplugin.so qgis/grassprovider.so
do
	install_name_tool -change $BUILDPREFIX/lib/libqgisgrass.0.dylib \
		@executable_path/lib/libqgisgrass.0.dylib \
		$BUILDPREFIX/lib/$PLUGIN
done

# Declare libmsexport relative to bundle
install_name_tool -id @executable_path/lib/libmsexport.0.0.0.dylib \
	$BUILDPREFIX/lib/libmsexport.0.0.0.dylib
