#!/bin/sh
# Copy PyQt supporting libraries to qgis bundle
# and make search paths for them relative to bundle

BUNDLE=qgis1.0.0.app/Contents/MacOS

# Edit version when any library is upgraded
LNKGDAL=libgdal.1.dylib
LNKGEOSC=libgeos_c.1.dylib

QTPREFIX=/usr/local/Trolltech/Qt-4.4.3
QTFRAMEWORKS="QtAssistant QtHelp QtOpenGL QtScript QtTest QtWebKit QtXmlPatterns phonon"
LIBQTCL=libQtCLucene.4.4.3.dylib
LNKQTCL=libQtCLucene.4.dylib

SITEPKG=/Library/Python/2.5/site-packages

# Copy additional Qt frameworks needed by PyQt to application bundle
cd $BUNDLE/lib

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
if test ! -f $LIBQTCL; then
	cp $QTPREFIX/lib/$LIBQTCL $LIBQTCL
	ln -s $LIBQTCL $LNKQTCL
	install_name_tool -id @executable_path/lib/$LNKQTCL $LNKQTCL
fi

# Update paths to supporting Qt frameworks
for FRAMEWORK in QtAssistant QtHelp QtOpenGL QtScript QtTest QtWebKit QtXmlPatterns phonon
do
	install_name_tool -change $QTPREFIX/lib/QtCore.framework/Versions/4/QtCore \
		@executable_path/lib/QtCore.framework/Versions/4/QtCore \
		$FRAMEWORK.framework/Versions/4/$FRAMEWORK
done
install_name_tool -change $QTPREFIX/lib/QtCore.framework/Versions/4/QtCore \
	@executable_path/lib/QtCore.framework/Versions/4/QtCore \
	$LIBQTCL
for FRAMEWORK in QtAssistant QtHelp QtOpenGL QtWebKit phonon
do
	install_name_tool -change $QTPREFIX/lib/QtGui.framework/Versions/4/QtGui \
		@executable_path/lib/QtGui.framework/Versions/4/QtGui \
		$FRAMEWORK.framework/Versions/4/$FRAMEWORK
done
for FRAMEWORK in QtAssistant QtHelp QtWebKit QtXmlPatterns
do
	install_name_tool -change $QTPREFIX/lib/QtNetwork.framework/Versions/4/QtNetwork \
		@executable_path/lib/QtNetwork.framework/Versions/4/QtNetwork \
		$FRAMEWORK.framework/Versions/4/$FRAMEWORK
done
install_name_tool -change $QTPREFIX/lib/QtSql.framework/Versions/4/QtSql \
	@executable_path/lib/QtSql.framework/Versions/4/QtSql \
	QtHelp.framework/Versions/4/QtHelp
install_name_tool -change $QTPREFIX/lib/QtXml.framework/Versions/4/QtXml \
	@executable_path/lib/QtXml.framework/Versions/4/QtXml \
	QtHelp.framework/Versions/4/QtHelp
install_name_tool -change $QTPREFIX/lib/$LNKQTCL \
	@executable_path/lib/$LNKQTCL \
	QtHelp.framework/Versions/4/QtHelp

cd ../../../../

# Copy sip and PyQt libraries to application bundle
cd $BUNDLE/share/qgis/python

if test ! -f sip.so; then
	cp $SITEPKG/sip.so sip.so
	cp $SITEPKG/sipconfig.py sipconfig.py
fi

if test ! -d PyQt4; then
	cp -R $SITEPKG/PyQt4 .
	for LIBPYQT4 in Qt QtCore QtGui QtNetwork QtSql QtSvg QtXml QtAssistant QtHelp QtOpenGL QtScript QtTest QtWebKit QtXmlPatterns phonon
	do
		cp $SITEPKG/PyQt4/$LIBPYQT4.so PyQt4/$LIBPYQT4.so
		# Update paths to supporting Qt frameworks
		install_name_tool -change $QTPREFIX/lib/$LIBPYQT4.framework/Versions/4/$LIBPYQT4 \
			@executable_path/lib/$LIBPYQT4.framework/Versions/4/$LIBPYQT4 \
			PyQt4/$LIBPYQT4.so
		install_name_tool -change $QTPREFIX/lib/QtCore.framework/Versions/4/QtCore \
			@executable_path/lib/QtCore.framework/Versions/4/QtCore \
			PyQt4/$LIBPYQT4.so
		install_name_tool -change $QTPREFIX/lib/QtGui.framework/Versions/4/QtGui \
			@executable_path/lib/QtGui.framework/Versions/4/QtGui \
			PyQt4/$LIBPYQT4.so
	done
	# Update paths to supporting Qt frameworks
	for LIBPYQT4 in QtAssistant QtWebKit QtXmlPatterns
	do
		install_name_tool -change $QTPREFIX/lib/QtNetwork.framework/Versions/4/QtNetwork \
			@executable_path/lib/QtNetwork.framework/Versions/4/QtNetwork \
			PyQt4/$LIBPYQT4.so
	done
	install_name_tool -change $QTPREFIX/lib/QtSql.framework/Versions/4/QtSql \
		@executable_path/lib/QtSql.framework/Versions/4/QtSql \
		PyQt4/QtHelp.so
	for LIBPYQT4 in QtSvg QtHelp
	do
		install_name_tool -change $QTPREFIX/lib/QtXml.framework/Versions/4/QtXml \
			@executable_path/lib/QtXml.framework/Versions/4/QtXml \
			PyQt4/$LIBPYQT4.so
	done
	install_name_tool -change $QTPREFIX/lib/libQtCLucene.4.dylib \
		@executable_path/lib/libQtCLucene.4.dylib \
		PyQt4/QtHelp.so
fi

# Update qgis python plugin paths to supporting libraries
for LIBQGIS in core gui
do
	install_name_tool -change /usr/local/lib/$LNKGDAL \
		@executable_path/lib/$LNKGDAL \
		qgis/$LIBQGIS.so
	install_name_tool -change /usr/local/lib/$LNKGEOSC \
		@executable_path/lib/$LNKGEOSC \
		qgis/$LIBQGIS.so
	for FRAMEWORK in QtCore QtGui QtNetwork QtSql QtSvg QtXml Qt3Support
	do
		install_name_tool -change $QTPREFIX/lib/$FRAMEWORK.framework/Versions/4/$FRAMEWORK \
			@executable_path/lib/$FRAMEWORK.framework/Versions/4/$FRAMEWORK \
			qgis/$LIBQGIS.so
	done
done

cd ../../../../../../
