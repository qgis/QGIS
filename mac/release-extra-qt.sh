#!/bin/sh
# Copy Qt frameworks to qgis bundle
# and make search paths for them relative to bundle

PREFIX=qgis0.10.0.app/Contents/MacOS

# Edit version when any library is upgraded
LIBJPEG=libjpeg.62.0.0.dylib
LNKJPEG=libjpeg.62.dylib
LIBPNG=libpng.3.24.0.dylib
LNKPNG=libpng.3.dylib

QTPREFIX=/usr/local/Qt4.3
QTFRAMEWORKS="QtCore QtGui QtNetwork QtSql QtSvg QtXml Qt3Support"

# Copy supporting frameworks to application bundle
cd $PREFIX/lib

for FRAMEWORK in $QTFRAMEWORKS
do
	LIBFRAMEWORK=$FRAMEWORK.framework/Versions/4/$FRAMEWORK
	if test ! -f $LIBFRAMEWORK; then
		mkdir $FRAMEWORK.framework
		mkdir $FRAMEWORK.framework/Versions
		mkdir $FRAMEWORK.framework/Versions/4
		cp $QTPREFIX/lib/$LIBFRAMEWORK $LIBFRAMEWORK
		install_name_tool -id @executable_path/lib/$LIBFRAMEWORK $LIBFRAMEWORK
	fi
done

# Update path to supporting frameworks
for FRAMEWORK in QtGui QtNetwork QtSql QtSvg QtXml Qt3Support
do
	install_name_tool -change QtCore.framework/Versions/4/QtCore \
		@executable_path/lib/QtCore.framework/Versions/4/QtCore \
		$FRAMEWORK.framework/Versions/4/$FRAMEWORK
done
for FRAMEWORK in QtSvg Qt3Support
do
	install_name_tool -change QtGui.framework/Versions/4/QtGui \
		@executable_path/lib/QtGui.framework/Versions/4/QtGui \
		$FRAMEWORK.framework/Versions/4/$FRAMEWORK
	install_name_tool -change QtXml.framework/Versions/4/QtXml \
		@executable_path/lib/QtXml.framework/Versions/4/QtXml \
		$FRAMEWORK.framework/Versions/4/$FRAMEWORK
done
for FRAMEWORK in Qt3Support
do
	install_name_tool -change QtNetwork.framework/Versions/4/QtNetwork \
		@executable_path/lib/QtNetwork.framework/Versions/4/QtNetwork \
		$FRAMEWORK.framework/Versions/4/$FRAMEWORK
	install_name_tool -change QtSql.framework/Versions/4/QtSql \
		@executable_path/lib/QtSql.framework/Versions/4/QtSql \
		$FRAMEWORK.framework/Versions/4/$FRAMEWORK
done
for FRAMEWORK in QtGui QtSvg Qt3Support
do
	install_name_tool -change /usr/local/lib/$LNKPNG @executable_path/lib/$LNKPNG \
		$FRAMEWORK.framework/Versions/4/$FRAMEWORK
done
cd ../../
LIBQJPEG=plugins/imageformats/libqjpeg.dylib
if test ! -f $LIBQJPEG; then
	mkdir plugins
	mkdir plugins/imageformats
	cp $QTPREFIX/$LIBQJPEG $LIBQJPEG
	install_name_tool -id @executable_path/../$LIBQJPEG $LIBQJPEG
	# Update path to supporting libraries
	install_name_tool -change QtCore.framework/Versions/4/QtCore \
		@executable_path/lib/QtCore.framework/Versions/4/QtCore \
		$LIBQJPEG
	install_name_tool -change QtGui.framework/Versions/4/QtGui \
		@executable_path/lib/QtGui.framework/Versions/4/QtGui \
		$LIBQJPEG
	install_name_tool -change /usr/local/lib/$LNKJPEG @executable_path/lib/$LNKJPEG $LIBQJPEG
	install_name_tool -change /usr/local/lib/$LNKPNG @executable_path/lib/$LNKPNG $LIBQJPEG
	# Empty qt.conf indicates plugins are in default location within this bundle
	cp /dev/null Resources/qt.conf
fi
cd ../../

for FILE in \
	qgis \
	bin/qgis_help.app/Contents/MacOS/qgis_help \
	lib/libqgis_core.dylib \
	lib/libqgis_gui.dylib \
	lib/libqgisgrass.dylib \
	lib/qgis/libcopyrightlabelplugin.so \
	lib/qgis/libdelimitedtextplugin.so \
	lib/qgis/libdelimitedtextprovider.so \
	lib/qgis/libgeorefplugin.so \
	lib/qgis/libgpsimporterplugin.so \
	lib/qgis/libgpxprovider.so \
	lib/qgis/libgrassplugin.so \
	lib/qgis/libgrassprovider.so \
	lib/qgis/libgridmakerplugin.so \
	lib/qgis/libwfsprovider.so \
	lib/qgis/libnortharrowplugin.so \
	lib/qgis/libogrprovider.so \
	lib/qgis/libpggeoprocessingplugin.so \
	lib/qgis/libpostgresprovider.so \
	lib/qgis/libquickprintplugin.so \
	lib/qgis/libscalebarplugin.so \
	lib/qgis/libspitplugin.so \
	lib/qgis/libwfsplugin.so \
	lib/qgis/libwmsprovider.so
do
	for FRAMEWORK in QtCore QtGui QtNetwork QtSql QtSvg QtXml Qt3Support
	do
		install_name_tool -change $FRAMEWORK.framework/Versions/4/$FRAMEWORK \
			@executable_path/lib/$FRAMEWORK.framework/Versions/4/$FRAMEWORK \
			$PREFIX/$FILE
	done
done
