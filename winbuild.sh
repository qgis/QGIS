#!/bin/sh
export PATH=/home/user/win/qt4.1_win/bin:$PATH
export LD_LIBRARY_PATH=/home/user/win/qt4.1_win/lib:$LD_LIBRARY_PATH
export QTDIR=$WIN/qt-4.1_win

# added by gsherman to enable ccache
#export CC='ccache gcc'
#export CXX='ccache g++'

export QT_CFLAGS="-DQT_SHARED -I/home/user/win/qt4.1_win/include -I/home/user/win/qt4.1_win/include/QtCore -I/home/user/win/qt4.1_win/include/QtGui -I/home/user/win/qt4.1_win/include/Qt3Support -I/home/user/win/qt4.1_win/include/QtNetwork -I/home/user/win/qt4.1_win/include/QtXml -I/home/user/win/qt4.1_win/include/QtSvg -I/home/user/win/qt4.1_win/include/QtSql -I/home/user/win/qt4.1_win/include/QtTest"
export QT_LIBS="-L/home/user/win/qt4.1_win/lib -lQtNetwork -lQtSvg -lQtXml -lQtGui -lQtSql  -lQt3Support -lQtCore -lQtTest"

#export QT_LIBS="-L/home/user/win/qt4.1_win/lib"
#export QT_LDADD="-L/home/user/win/qt4.1_win/lib -lQtNetwork -lQtSvg -lQtXml -lQtGui -lQtSql -lQt3Support -lpthread -lQtTest -lQtCore"

#checking for QT_CFLAGS... -DQT_SHARED -I/usr/include/qt4 -I/usr/include/qt4/QtCore -I/usr/include/qt4/QtGui -I/usr/include/qt4/Qt3Support -I/usr/include/qt4/QtNetwork -I/usr/include/qt4/QtXml -I/usr/include/qt4/QtSvg
#checking for QT_LIBS... -L/home/holl/software/qt4-x11-4.1.0/lib -L/usr/X11R6/lib -lQt3Support -lQtSql -lQtNetwork -lQtSvg -lQtXml -lQtGui -laudio -lXt -lpng -lSM -lICE -lXi -lXrender -lXrandr -lXcursor -lXinerama -lfreetype -lXext -lX11 -lQtTest -lQtCore -lfontconfig -lz -lm -ldl -lpthread
#checking for QT_TEST_LIBS... -L/home/holl/software/qt4-x11-4.1.0/lib -lQtTest -lQtCore -lfontconfig -lz -lm -ldl -lpthread
#checking QT_CXXFLAGS... -DQT3_SUPPORT -DQT_SHARED -I/usr/include/qt4 -I/usr/include/qt4/QtCore -I/usr/include/qt4/QtGui -I/usr/include/qt4/Qt3Support -I/usr/include/qt4/QtNetwork -I/usr/include/qt4/QtXml -I/usr/include/qt4/QtSvg   -I/usr/include/qt4/QtTest
#checking QT_LDADD... -L/home/holl/software/qt4-x11-4.1.0/lib -L/usr/X11R6/lib -lQt3Support -lQtSql -lQtNetwork -lQtSvg -lQtXml -lQtGui -laudio -lXt -lpng -lSM -lICE -lXi -lXrender -lXrandr -lXcursor -lXinerama -lfreetype -lXext -lX11 -lQtTest -lQtCore -lfontconfig -lz -lm -ldl -lpthread


./autogen.sh \
--with-qt-pkg-config=no \
--prefix=$WIN/i586-mingw32msvc/release \
--target=$TARGET \
--host=$TARGET \
--build=i386-linux \
--with-qtdir=$WIN/qt-4.1_win \
--with-projdir=$WIN/i586-mingw32msvc \
--with-gdal=$WIN/i586-mingw32msvc/bin/gdal-config \
--with-geos=$WIN/i586-mingw32msvc/bin/geos-config \
--with-sqlite3dir=$WIN/i586-mingw32msvc \
--with-gsl=$WIN/i586-mingw32msvc/bin/gsl-config \
--with-grass=$WIN/i586-mingw32msvc/grass-6.1.cvs \
--with-postgresql

#./configure \
#--with-qt-pkg-config=no \
#--prefix=$WIN/i586-mingw32msvc \
#--target=$TARGET \
#--host=$TARGET \
#--build=i386-linux \
#--with-qtdir=$WIN/qt-4.1_win \
#--with-projdir=$WIN/i586-mingw32msvc \
#--with-gdal=$WIN/i586-mingw32msvc/bin/gdal-config \
#--with-geos=$WIN/i586-mingw32msvc/bin/geos-config \
#--with-sqlite3dir=$WIN/i586-mingw32msvc \
#--with-gsl=$WIN/i586-mingw32msvc/bin/gsl-config \
#--with-grass=$WIN/i586-mingw32msvc/grass-6.1.cvs \
#--with-postgresql


# build the beast
#~/bin/winmake -f Makefile.win

