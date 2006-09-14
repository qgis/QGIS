#!/bin/sh
# Copy Qt frameworks and libraries to qgis bundle
# and make search paths for them relative to bundle

PREFIX=qgis.app/Contents/MacOS

# Edit version when any library is upgraded
LIBJPEG=libjpeg.62.0.0.dylib
LNKJPEG=libjpeg.62.dylib
LIBPNG=libpng.3.1.2.8.dylib
LNKPNG=libpng.3.dylib

QTVER=4.1.4
QTPREFIX=/usr/local/Trolltech/Qt-$QTVER
QTFRAMEWORKS="QtCore QtGui QtNetwork QtSql QtSvg QtXml Qt3Support"
QTLIBRARIES="QtDesigner QtTest"

# Copy supporting frameworks to application bundle
cd $PREFIX/lib

for FRAMEWORK in $QTFRAMEWORKS
do
	LIBFRAMEWORK=$FRAMEWORK.framework/Versions/4.0/$FRAMEWORK
	if test ! -f $LIBFRAMEWORK; then
		mkdir $FRAMEWORK.framework
		mkdir $FRAMEWORK.framework/Versions
		mkdir $FRAMEWORK.framework/Versions/4.0
		cp $QTPREFIX/lib/$LIBFRAMEWORK $LIBFRAMEWORK
		install_name_tool -id @executable_path/libs/$LIBFRAMEWORK $LIBFRAMEWORK
	fi
done
for LIBRARY in $QTLIBRARIES
do
	LIB=lib$LIBRARY.$QTVER.dylib
	if test ! -f $LIB; then
		cp $QTPREFIX/lib/$LIB $LIB
		ln -s $LIB lib$LIBRARY.4.dylib
		install_name_tool -id @executable_path/libs/$LIB $LIB
	fi
done

# Update path to supporting frameworks
for FRAMEWORK in QtGui QtNetwork QtSql QtSvg QtXml Qt3Support
do
	install_name_tool -change $QTPREFIX/lib/QtCore.framework/Versions/4.0/QtCore \
		@executable_path/lib/QtCore.framework/Versions/4.0/QtCore \
		$FRAMEWORK.framework/Versions/4.0/$FRAMEWORK
done
for FRAMEWORK in QtSvg Qt3Support
do
	install_name_tool -change $QTPREFIX/lib/QtGui.framework/Versions/4.0/QtGui \
		@executable_path/lib/QtGui.framework/Versions/4.0/QtGui \
		$FRAMEWORK.framework/Versions/4.0/$FRAMEWORK
	install_name_tool -change $QTPREFIX/lib/QtXml.framework/Versions/4.0/QtXml \
		@executable_path/lib/QtXml.framework/Versions/4.0/QtXml \
		$FRAMEWORK.framework/Versions/4.0/$FRAMEWORK
done
for FRAMEWORK in Qt3Support
do
	install_name_tool -change $QTPREFIX/lib/QtNetwork.framework/Versions/4.0/QtNetwork \
		@executable_path/lib/QtNetwork.framework/Versions/4.0/QtNetwork \
		$FRAMEWORK.framework/Versions/4.0/$FRAMEWORK
	install_name_tool -change $QTPREFIX/lib/QtSql.framework/Versions/4.0/QtSql \
		@executable_path/lib/QtSql.framework/Versions/4.0/QtSql \
		$FRAMEWORK.framework/Versions/4.0/$FRAMEWORK
done
for FRAMEWORK in QtGui QtSvg Qt3Support
do
	install_name_tool -change /usr/local/lib/$LNKPNG @executable_path/lib/$LNKPNG \
		$FRAMEWORK.framework/Versions/4.0/$FRAMEWORK
done
for FRAMEWORK in QtCore QtGui QtXml
do
	install_name_tool -change $QTPREFIX/lib/$FRAMEWORK.framework/Versions/4.0/$FRAMEWORK \
		@executable_path/lib/$FRAMEWORK.framework/Versions/4.0/$FRAMEWORK \
		libQtDesigner.$QTVER.dylib
done
install_name_tool -change /usr/local/lib/$LNKPNG \
	@executable_path/lib/$LNKPNG \
	libQtDesigner.$QTVER.dylib
install_name_tool -change $QTPREFIX/lib/QtCore.framework/Versions/4.0/QtCore \
	@executable_path/lib/QtCore.framework/Versions/4.0/QtCore \
	libQtTest.$QTVER.dylib
cd ../
LIBQJPEG=imageformats/libqjpeg.dylib
if test ! -f $LIBQJPEG; then
	mkdir imageformats
	cp $QTPREFIX/plugins/$LIBQJPEG $LIBQJPEG
	# Update path to supporting libraries
	install_name_tool -change $QTPREFIX/lib/QtCore.framework/Versions/4.0/QtCore \
		@executable_path/lib/QtCore.framework/Versions/4.0/QtCore \
		$LIBQJPEG
	install_name_tool -change $QTPREFIX/lib/QtGui.framework/Versions/4.0/QtGui \
		@executable_path/lib/QtGui.framework/Versions/4.0/QtGui \
		$LIBQJPEG
	install_name_tool -change /usr/local/lib/$LNKJPEG @executable_path/lib/$LNKJPEG $LIBQJPEG
	install_name_tool -change /usr/local/lib/$LNKPNG @executable_path/lib/$LNKPNG $LIBQJPEG
fi
cd ../../../

for FILE in \
	qgis \
	bin/qgis_help.app/Contents/MacOS/qgis_help \
	bin/gridmaker \
	bin/msexport.app/Contents/MacOS/msexport \
	bin/spit \
	lib/libmsexport.0.0.0.dylib \
	lib/libqgis_core.0.0.1.dylib \
	lib/libqgis_gui.0.0.1.dylib \
	lib/libqgis_raster.0.0.0.dylib \
	lib/libqgisgrass.0.0.1.dylib \
	lib/qgis/copyrightlabelplugin.so \
	lib/qgis/delimitedtextplugin.so \
	lib/qgis/delimitedtextprovider.so \
	lib/qgis/georefplugin.so \
	lib/qgis/gpsimporterplugin.so \
	lib/qgis/gpxprovider.so \
	lib/qgis/grassplugin.so \
	lib/qgis/grassprovider.so \
	lib/qgis/gridmakerplugin.so \
	lib/qgis/libScaleBarplugin.so \
	lib/qgis/northarrowplugin.so \
	lib/qgis/ogrprovider.so \
	lib/qgis/pggeoprocessingplugin.so \
	lib/qgis/postgresprovider.so \
	lib/qgis/spitplugin.so \
	lib/qgis/wmsprovider.so \
	lib/qgis/libqgsprojectionselector.dylib
	#bin/omgui \
	#lib/qgis/libopenmodeller.so
do
	for FRAMEWORK in QtCore QtGui QtNetwork QtSvg QtXml Qt3Support
	do
		install_name_tool -change $QTPREFIX/lib/$FRAMEWORK.framework/Versions/4.0/$FRAMEWORK \
			@executable_path/lib/$FRAMEWORK.framework/Versions/4.0/$FRAMEWORK \
			$PREFIX/$FILE
	done
	for LIB in QtDesigner QtTest
	do
		install_name_tool -change $QTPREFIX/lib/lib$LIB.4.dylib \
			@executable_path/lib/lib$LIB.4.dylib \
			$PREFIX/$FILE
	done
done
