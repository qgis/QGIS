#!/bin/sh
# Make all libs and plugins in qgis bundle relative to bundle
# This allows the bundle to be moved to another folder and still work

# Edit INSTALLPREFIX to match the value of cmake INSTALL_PREFIX
INSTALLPREFIX=$PWD

BUNDLE=qgis0.9.1.app/Contents/MacOS
BUILDPREFIX=$INSTALLPREFIX/$BUNDLE

QLIBNAMES="core gui"

# Declare libqgis_* relative to bundle and update qgis app client
for NAME in $QLIBNAMES
do
	install_name_tool -id @executable_path/lib/libqgis_$NAME.dylib \
		$BUILDPREFIX/lib/libqgis_$NAME.dylib

	install_name_tool -change $BUILDPREFIX/lib/libqgis_$NAME.dylib \
		@executable_path/lib/libqgis_$NAME.dylib \
		$BUILDPREFIX/qgis

	install_name_tool -change $INSTALLPREFIX/src/$NAME/libqgis_$NAME.dylib \
		@executable_path/lib/libqgis_$NAME.dylib \
		$BUILDPREFIX/bin/msexport.app/Contents/MacOS/msexport

	install_name_tool -change $INSTALLPREFIX/src/$NAME/libqgis_$NAME.dylib \
		@executable_path/lib/libqgis_$NAME.dylib \
		$BUILDPREFIX/bin/qgis_help.app/Contents/MacOS/qgis_help
done

# Update libqgis_gui client of libqgis_*
install_name_tool -change $BUILDPREFIX/lib/libqgis_core.dylib \
	@executable_path/lib/libqgis_core.dylib \
	$BUILDPREFIX/lib/libqgis_gui.dylib

# Update plugin and lib clients of libqgis_* and libqgsprojectionselector
for PLUGIN in \
	qgis/libcopyrightlabelplugin.so \
	qgis/libdelimitedtextplugin.so \
	qgis/libdelimitedtextprovider.so \
	qgis/libgeorefplugin.so \
	qgis/libgpsimporterplugin.so \
	qgis/libgpxprovider.so \
	qgis/libgrassplugin.so \
	qgis/libgrassprovider.so \
	qgis/libgridmakerplugin.so \
	qgis/libwfsprovider.so \
	qgis/libnortharrowplugin.so \
	qgis/libogrprovider.so \
	qgis/libpggeoprocessingplugin.so \
	qgis/libpostgresprovider.so \
	qgis/libscalebarplugin.so \
	qgis/libspitplugin.so \
	qgis/libwfsplugin.so \
	qgis/libwmsprovider.so \
	libqgisgrass.dylib
do
	for NAME in $QLIBNAMES
	do
		install_name_tool -change $BUILDPREFIX/lib/libqgis_$NAME.dylib \
			@executable_path/lib/libqgis_$NAME.dylib \
			$BUILDPREFIX/lib/$PLUGIN
	done
done

# Declare libqgisgrass relative to bundle
install_name_tool -id @executable_path/lib/libqgisgrass.dylib \
	$BUILDPREFIX/lib/libqgisgrass.dylib

# Update plugin clients of libqgisgrass
for PLUGIN in qgis/libgrassplugin.so qgis/libgrassprovider.so
do
	install_name_tool -change $BUILDPREFIX/lib/libqgisgrass.dylib \
		@executable_path/lib/libqgisgrass.dylib \
		$BUILDPREFIX/lib/$PLUGIN
done
