#!/bin/sh
# Make all libs and plugins in qgis bundle relative to bundle
# This allows the bundle to be moved to another folder and still work

# Edit INSTALLPREFIX to match the value of cmake INSTALL_PREFIX
INSTALLPREFIX=$PWD

VER=1.0
BUNDLE=qgis$VER.0.app/Contents/MacOS
BUILDPREFIX=$INSTALLPREFIX/$BUNDLE

QLIBNAMES="core gui"

# Declare libqgis_* relative to bundle and update qgis and qgis_help app clients
for NAME in $QLIBNAMES
do
	install_name_tool -id @executable_path/lib/libqgis_$NAME.$VER.dylib \
		$BUILDPREFIX/lib/libqgis_$NAME.$VER.dylib

	install_name_tool -change $BUILDPREFIX/lib/libqgis_$NAME.$VER.dylib \
		@executable_path/lib/libqgis_$NAME.$VER.dylib \
		$BUILDPREFIX/qgis

	install_name_tool -change $INSTALLPREFIX/src/$NAME/libqgis_$NAME.$VER.dylib \
		@executable_path/lib/libqgis_$NAME.$VER.dylib \
		$BUILDPREFIX/bin/qgis_help.app/Contents/MacOS/qgis_help
done

# Update libqgis_gui client of libqgis_core
install_name_tool -change $BUILDPREFIX/lib/libqgis_core.$VER.dylib \
	@executable_path/lib/libqgis_core.$VER.dylib \
	$BUILDPREFIX/lib/libqgis_gui.$VER.dylib

# Update plugin and lib clients of libqgis_*
for PLUGIN in \
	qgis/libcoordinatecaptureplugin.so \
	qgis/libcopyrightlabelplugin.so \
	qgis/libdelimitedtextplugin.so \
	qgis/libdelimitedtextprovider.so \
	qgis/libdxf2shpconverterplugin.so \
	qgis/libgeorefplugin.so \
	qgis/libgpsimporterplugin.so \
	qgis/libgpxprovider.so \
	qgis/libgridmakerplugin.so \
	qgis/libinterpolationplugin.so \
	qgis/libmemoryprovider.so \
	qgis/libnortharrowplugin.so \
	qgis/libogrconverterplugin.so \
	qgis/libogrprovider.so \
	qgis/libpostgresprovider.so \
	qgis/libscalebarplugin.so \
	qgis/libspitplugin.so \
	qgis/libwfsplugin.so \
	qgis/libwfsprovider.so \
	qgis/libwmsprovider.so
do
	for NAME in $QLIBNAMES
	do
		install_name_tool -change $BUILDPREFIX/lib/libqgis_$NAME.$VER.dylib \
			@executable_path/lib/libqgis_$NAME.$VER.dylib \
			$BUILDPREFIX/lib/$PLUGIN
	done
done

if test -f $BUILDPREFIX/lib/libqgisgrass.$VER.dylib; then

	# Declare libqgisgrass relative to bundle
	install_name_tool -id @executable_path/lib/libqgisgrass.$VER.dylib \
		$BUILDPREFIX/lib/libqgisgrass.$VER.dylib

	# Update plugin and lib clients of libqgis_*
	for PLUGIN in \
		qgis/libgrassplugin.so \
		qgis/libgrassprovider.so \
		libqgisgrass.$VER.dylib
	do
		for NAME in $QLIBNAMES
		do
			install_name_tool -change $BUILDPREFIX/lib/libqgis_$NAME.$VER.dylib \
				@executable_path/lib/libqgis_$NAME.$VER.dylib \
				$BUILDPREFIX/lib/$PLUGIN
		done
	done

	# Update plugin clients of libqgisgrass
	for PLUGIN in qgis/libgrassplugin.so qgis/libgrassprovider.so
	do
		install_name_tool -change $BUILDPREFIX/lib/libqgisgrass.$VER.dylib \
			@executable_path/lib/libqgisgrass.$VER.dylib \
			$BUILDPREFIX/lib/$PLUGIN
	done

fi

if test -f $BUILDPREFIX/lib/libqgispython.$VER.dylib; then

	# Declare libqgispython relative to bundle
	install_name_tool -id @executable_path/lib/libqgispython.$VER.dylib \
		$BUILDPREFIX/lib/libqgispython.$VER.dylib

	# Update python lib paths to libqgis_*
	for NAME in $QLIBNAMES
	do
		install_name_tool -change $BUILDPREFIX/lib/libqgis_$NAME.$VER.dylib \
			@executable_path/lib/libqgis_$NAME.$VER.dylib \
			$BUILDPREFIX/lib/libqgispython.$VER.dylib
	done

	# Update python plugin paths libqgis_*
	for PLUGIN in core.so gui.so
	do
		for NAME in $QLIBNAMES
		do
			install_name_tool -change $INSTALLPREFIX/src/$NAME/libqgis_$NAME.$VER.dylib \
				@executable_path/lib/libqgis_$NAME.$VER.dylib \
				$BUILDPREFIX/share/qgis/python/qgis/$PLUGIN
		done
	done

fi
