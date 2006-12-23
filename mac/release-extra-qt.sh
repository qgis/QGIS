#!/bin/sh
# Copy Qt frameworks to qgis bundle
# and make search paths for them relative to bundle

PREFIX=qgis.app/Contents/MacOS

# Edit version when any library is upgraded
LIBJPEG=libjpeg.62.0.0.dylib
LNKJPEG=libjpeg.62.dylib
LIBPNG=libpng.3.1.2.8.dylib
LNKPNG=libpng.3.dylib

QTVER=4.2.2
QTPREFIX=/usr/local/Trolltech/Qt-$QTVER
QTFRAMEWORKS="QtCore QtGui QtNetwork QtSql QtSvg QtXml Qt3Support QtDesigner QtTest"

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
		install_name_tool -id @executable_path/libs/$LIBFRAMEWORK $LIBFRAMEWORK
	fi
done

# Update path to supporting frameworks
for FRAMEWORK in QtGui QtNetwork QtSql QtSvg QtXml Qt3Support QtDesigner QtTest
do
	install_name_tool -change $QTPREFIX/lib/QtCore.framework/Versions/4/QtCore \
		@executable_path/lib/QtCore.framework/Versions/4/QtCore \
		$FRAMEWORK.framework/Versions/4/$FRAMEWORK
done
for FRAMEWORK in QtSvg Qt3Support QtDesigner
do
	install_name_tool -change $QTPREFIX/lib/QtGui.framework/Versions/4/QtGui \
		@executable_path/lib/QtGui.framework/Versions/4/QtGui \
		$FRAMEWORK.framework/Versions/4/$FRAMEWORK
	install_name_tool -change $QTPREFIX/lib/QtXml.framework/Versions/4/QtXml \
		@executable_path/lib/QtXml.framework/Versions/4/QtXml \
		$FRAMEWORK.framework/Versions/4/$FRAMEWORK
done
for FRAMEWORK in Qt3Support
do
	install_name_tool -change $QTPREFIX/lib/QtNetwork.framework/Versions/4/QtNetwork \
		@executable_path/lib/QtNetwork.framework/Versions/4/QtNetwork \
		$FRAMEWORK.framework/Versions/4/$FRAMEWORK
	install_name_tool -change $QTPREFIX/lib/QtSql.framework/Versions/4/QtSql \
		@executable_path/lib/QtSql.framework/Versions/4/QtSql \
		$FRAMEWORK.framework/Versions/4/$FRAMEWORK
done
for FRAMEWORK in QtGui QtSvg Qt3Support QtDesigner
do
	install_name_tool -change /usr/local/lib/$LNKPNG @executable_path/lib/$LNKPNG \
		$FRAMEWORK.framework/Versions/4/$FRAMEWORK
done
cd ../
LIBQJPEG=imageformats/libqjpeg.dylib
if test ! -f $LIBQJPEG; then
	mkdir imageformats
	cp $QTPREFIX/plugins/$LIBQJPEG $LIBQJPEG
	# Update path to supporting libraries
	install_name_tool -change $QTPREFIX/lib/QtCore.framework/Versions/4/QtCore \
		@executable_path/lib/QtCore.framework/Versions/4/QtCore \
		$LIBQJPEG
	install_name_tool -change $QTPREFIX/lib/QtGui.framework/Versions/4/QtGui \
		@executable_path/lib/QtGui.framework/Versions/4/QtGui \
		$LIBQJPEG
	install_name_tool -change /usr/local/lib/$LNKJPEG @executable_path/lib/$LNKJPEG $LIBQJPEG
	install_name_tool -change /usr/local/lib/$LNKPNG @executable_path/lib/$LNKPNG $LIBQJPEG
fi
cd ../../../

for FILE in \
	qgis \
	bin/qgis_help.app/Contents/MacOS/qgis_help \
	bin/msexport.app/Contents/MacOS/msexport \
	bin/gridmaker \
	bin/spit \
	lib/libmsexport.1.0.0.dylib \
	lib/libqgis_core.1.0.0.dylib \
	lib/libqgis_gui.1.0.0.dylib \
	lib/libqgis_raster.1.0.0.dylib \
	lib/libqgisgrass.1.0.0.dylib \
	lib/qgis/copyrightlabelplugin.so \
	lib/qgis/delimitedtextplugin.so \
	lib/qgis/delimitedtextprovider.so \
	lib/qgis/georefplugin.so \
	lib/qgis/gpsimporterplugin.so \
	lib/qgis/gpxprovider.so \
	lib/qgis/grassplugin.so \
	lib/qgis/grassprovider.so \
	lib/qgis/gridmakerplugin.so \
	lib/qgis/libwfsprovider.so \
	lib/qgis/northarrowplugin.so \
	lib/qgis/ogrprovider.so \
	lib/qgis/pggeoprocessingplugin.so \
	lib/qgis/postgresprovider.so \
	lib/qgis/ScaleBarplugin.so \
	lib/qgis/spitplugin.so \
	lib/qgis/wfsplugin.so \
	lib/qgis/wmsprovider.so \
	lib/qgis/libqgsprojectionselector.dylib
	#bin/omgui \
	#lib/qgis/libopenmodeller.so
do
	for FRAMEWORK in QtCore QtGui QtNetwork QtSql QtSvg QtXml Qt3Support QtDesigner QtTest
	do
		install_name_tool -change $QTPREFIX/lib/$FRAMEWORK.framework/Versions/4/$FRAMEWORK \
			@executable_path/lib/$FRAMEWORK.framework/Versions/4/$FRAMEWORK \
			$PREFIX/$FILE
	done
done
