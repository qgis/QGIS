#!/bin/sh
# Copy Qt frameworks to atlas bundle
# and make search paths for them relative to bundle

APP_PREFIX=/Applications/qgis0.11.0.app
MACOS_PREFIX=${APP_PREFIX}/Contents/MacOS
FRAMEWORKPREFIX=${APP_PREFIX}/Contents/Frameworks
mkdir -p $MACOS_PREFIX
mkdir -p $FRAMEWORKPREFIX
pushd $PWD
cd $FRAMEWORKPREFIX

# Edit version when any library is upgraded
UNIVERSAL_LIBS_PREFIX=/usr/local/qgis_universal_deps
QTPREFIX=${UNIVERSAL_LIBS_PREFIX}/lib
QTFRAMEWORKS="QtCore QtGui QtNetwork QtSql QtSvg QtXml QtDesigner"

#
# Copy supporting frameworks to application bundle
#
cd $FRAMEWORKPREFIX
for FRAMEWORK in $QTFRAMEWORKS
do
	LIBFRAMEWORK=$FRAMEWORK.framework/Versions/4/$FRAMEWORK
	if test ! -f $LIBFRAMEWORK; then
		mkdir -p $FRAMEWORK.framework/Versions/4
		cp $QTPREFIX/$LIBFRAMEWORK $LIBFRAMEWORK
		install_name_tool -id @executable_path/../Frameworks/$LIBFRAMEWORK $LIBFRAMEWORK
	fi
done

# Update path to supporting frameworks
for FRAMEWORK in QtGui QtNetwork QtSql QtSvg QtXml
do
	install_name_tool -change ${QTPREFIX}/QtCore.framework/Versions/4/QtCore \
		@executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore \
		$FRAMEWORK.framework/Versions/4/$FRAMEWORK
done

for FRAMEWORK in QtSvg
do
	install_name_tool -change ${QTPREFIX}/QtGui.framework/Versions/4/QtGui \
		@executable_path/../Frameworks/QtGui.framework/Versions/4/QtGui \
		$FRAMEWORK.framework/Versions/4/$FRAMEWORK
	install_name_tool -change ${QTPREFIX}/QtXml.framework/Versions/4/QtXml \
		@executable_path/../Frameworks/QtXml.framework/Versions/4/QtXml \
		$FRAMEWORK.framework/Versions/4/$FRAMEWORK
done

#
# Update qgis related libs and binaries
#

cd $MACOS_PREFIX
FILES="qgis
  lib/libqgis_core.dylib
  lib/libqgis_gui.dylib
  lib/qgis/libcatalogue.so
  lib/qgis/libcopyrightlabelplugin.so
  lib/qgis/libgpxprovider.so
  lib/qgis/libscalebarplugin.so
  lib/qgis/libdelimitedtextplugin.so
  lib/qgis/libgridmakerplugin.so
  lib/qgis/libwfsplugin.so
  lib/qgis/libdelimitedtextprovider.so
  lib/qgis/libnortharrowplugin.so
  lib/qgis/libwfsprovider.so
  lib/qgis/libgeorefplugin.so
  lib/qgis/libogrprovider.so
  lib/qgis/libwmsprovider.so
  lib/qgis/libgpsimporterplugin.so
  lib/qgis/libevis.so
  lib/qgis/libquickprintplugin.so
  share/qgis/python/qgis/core.so
  share/qgis/python/qgis/gui.so
  lib/libqgispython.dylib"
for FILE in ${FILES}
do
 for FRAMEWORK in QtCore QtGui QtNetwork QtSql QtSvg QtXml
	do
		install_name_tool -change ${QTPREFIX}/${FRAMEWORK}.framework/Versions/4/$FRAMEWORK \
			@executable_path/../Frameworks/$FRAMEWORK.framework/Versions/4/$FRAMEWORK \
			$MACOS_PREFIX/$FILE
	done
done



#
# Update qt imageformat plugin paths
#


cd ${MACOS_PREFIX}/../
mkdir -p plugins/imageformats
cd plugins/imageformats

LIBJPEG=libjpeg.dylib
LIBQJPEG=${UNIVERSAL_LIBS_PREFIX}/plugins/imageformats/libqjpeg.dylib
if test ! -f $LIBJPEG; then
	cp $LIBQJPEG $LIBJPEG
	# Update path to supporting libraries
	install_name_tool -change ${QTPREFIX}/QtCore.framework/Versions/4/QtCore \
		@executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore \
		$LIBJPEG
	install_name_tool -change ${QTPREFIX}/QtGui.framework/Versions/4/QtGui \
		@executable_path/../Frameworks/QtGui.framework/Versions/4/QtGui \
		$LIBJPEG
	install_name_tool -change $LIBQJPEG @executable_path/../plugins/imageformats/$LIBJPEG 
fi
LIBGIF=libgif.dylib
LIBQGIF=${UNIVERSAL_LIBS_PREFIX}/plugins/imageformats/libqgif.dylib
if test ! -f $LIBGIF; then
	cp $LIBQGIF $LIBGIF
	# Update path to supporting libraries
	install_name_tool -change ${QTPREFIX}/QtCore.framework/Versions/4/QtCore \
		@executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore \
		$LIBGIF
	install_name_tool -change ${QTPREFIX}/QtGui.framework/Versions/4/QtGui \
		@executable_path/../Frameworks/QtGui.framework/Versions/4/QtGui \
		$LIBGIF
	install_name_tool -change $LIBQGIF @executable_path/../plugins/imageformats/$LIBGIF 
fi

#
# QT Sql Drivers
#
cd ${MACOS_PREFIX}/../
mkdir -p plugins/sqldrivers
cd plugins/sqldrivers
LIBSQLITE=libqsqlite.dylib
LIBQSQLITE=${UNIVERSAL_LIBS_PREFIX}/plugins/sqldrivers/libqsqlite.dylib
if test ! -f $LIBSQLITE; then
	cp $LIBQSQLITE $LIBSQLITE
	# Update path to supporting libraries
	install_name_tool -change ${QTPREFIX}/QtCore.framework/Versions/4/QtCore \
		@executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore \
		$LIBSQLITE
	install_name_tool -change ${QTPREFIX}/QtSql.framework/Versions/4/QtSql \
		@executable_path/../Frameworks/QtSql.framework/Versions/4/QtSql \
		$LIBSQLITE
	install_name_tool -change $LIBSQLITE @executable_path/../plugins/sqldrivers/$LIBSQLITE 
fi

popd

#
# Strip the qt libs
#

strip -x ${FRAMEWORKPREFIX}/QtGui.framework/Versions/4/QtGui 
strip -x ${FRAMEWORKPREFIX}/QtCore.framework/Versions/4/QtCore 
strip -x ${FRAMEWORKPREFIX}/QtSql.framework/Versions/4/QtSql 
strip -x ${FRAMEWORKPREFIX}/QtSvg.framework/Versions/4/QtSvg 
strip -x ${FRAMEWORKPREFIX}/QtXml.framework/Versions/4/QtXml 
strip -x ${FRAMEWORKPREFIX}/QtNetwork.framework/Versions/4/QtNetwork 
strip -x ${FRAMEWORKPREFIX}/QtDesigner.framework/Versions/4/QtDesigner
