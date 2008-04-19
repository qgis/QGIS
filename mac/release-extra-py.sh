#!/bin/sh
# Copy Py supporting libraries to qgis bundle
# and make search paths for them relative to bundle

BUNDLE=qgis0.10.0.app/Contents/MacOS
SITEPKG=/Library/Python/2.3/site-packages

LNKGDAL=libgdal.1.dylib

# Copy supporting libraries to application bundle
cd $BUNDLE/share/qgis/python
if test ! -f sip.so; then
	cp $SITEPKG/sip.so sip.so
	cp $SITEPKG/sipconfig.py sipconfig.py
fi
if test ! -d PyQt4; then
	cp -R $SITEPKG/PyQt4 .
	for LIBPYQT4 in Qt QtCore QtGui QtNetwork QtSql QtSvg QtXml QtAssistant QtDesigner QtOpenGL QtScript QtTest
	do
		cp $SITEPKG/PyQt4/$LIBPYQT4.so PyQt4/$LIBPYQT4.so
		# Update path to supporting libraries
		install_name_tool -change $LIBPYQT4.framework/Versions/4/$LIBPYQT4 \
			@executable_path/lib/$LIBPYQT4.framework/Versions/4/$LIBPYQT4 \
			PyQt4/$LIBPYQT4.so
		install_name_tool -change QtCore.framework/Versions/4/QtCore \
			@executable_path/lib/QtCore.framework/Versions/4/QtCore \
			PyQt4/$LIBPYQT4.so
		install_name_tool -change QtGui.framework/Versions/4/QtGui \
			@executable_path/lib/QtGui.framework/Versions/4/QtGui \
			PyQt4/$LIBPYQT4.so
	done
	install_name_tool -change QtXml.framework/Versions/4/QtXml \
		@executable_path/lib/QtXml.framework/Versions/4/QtXml \
		PyQt4/QtSvg.so
	install_name_tool -change QtNetwork.framework/Versions/4/QtNetwork \
		@executable_path/lib/QtNetwork.framework/Versions/4/QtNetwork \
		PyQt4/QtAssistant.so
	install_name_tool -change QtScript.framework/Versions/4/QtScript \
		@executable_path/lib/QtScript.framework/Versions/4/QtScript \
		PyQt4/QtDesigner.so
	install_name_tool -change QtXml.framework/Versions/4/QtXml \
		@executable_path/lib/QtXml.framework/Versions/4/QtXml \
		PyQt4/QtDesigner.so
fi

# Update path to supporting libraries
for LIBQGIS in core gui
do
	install_name_tool -change /usr/local/lib/$LNKGDAL \
		@executable_path/lib/$LNKGDAL \
		qgis/$LIBQGIS.so
	for FRAMEWORK in QtCore QtGui QtNetwork QtSql QtSvg QtXml Qt3Support
	do
		install_name_tool -change $FRAMEWORK.framework/Versions/4/$FRAMEWORK \
			@executable_path/lib/$FRAMEWORK.framework/Versions/4/$FRAMEWORK \
			qgis/$LIBQGIS.so
	done
done
cd ../../../../../../
