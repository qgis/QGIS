dnl ------------------------------------------------------------------------
dnl Detect GDAL/OGR
dnl
dnl use AQ_CHECK_GDAL to detect GDAL and OGR
dnl it sets:
dnl   GDAL_CFLAGS
dnl   GDAL_LDADD
dnl ------------------------------------------------------------------------

# Check for GDAL and OGR compiler and linker flags

AC_DEFUN([AQ_CHECK_GDAL],
[
AC_ARG_WITH([gdal],
  AC_HELP_STRING([--with-gdal=path],
    [Full path to 'gdal-config' script, e.g. '--with-gdal=/usr/local/bin/gdal-config']),
  [ac_gdal_config_path=$withval])

if test x"$ac_gdal_config_path" = x ; then
  ac_gdal_config_path=`which gdal-config`
fi

ac_gdal_config_path=`dirname $ac_gdal_config_path 2> /dev/null`
AC_PATH_PROG(GDAL_CONFIG, gdal-config, no, $ac_gdal_config_path)

if test x${GDAL_CONFIG} = xno ; then
  AC_MSG_ERROR([gdal-config not found! Supply it with --with-gdal=PATH])
else
  AC_MSG_CHECKING([for OGR in GDAL])
  if test x`$GDAL_CONFIG --ogr-enabled` = "xno" ; then
    AC_MSG_ERROR([GDAL must be compiled with OGR support and currently is not.])
  fi
  AC_MSG_RESULT(yes)
  AC_MSG_CHECKING([GDAL_CFLAGS])
  GDAL_CFLAGS=`$GDAL_CONFIG --cflags`
  AC_MSG_RESULT($GDAL_CFLAGS)

  AC_MSG_CHECKING([GDAL_LDADD])
  GDAL_LDADD=`$GDAL_CONFIG --libs`
  AC_MSG_RESULT($GDAL_LDADD)

  ac_gdalogr_version=`$GDAL_CONFIG --version`
  ac_gdalogr="yes"
fi

AC_SUBST(GDAL_CFLAGS)
AC_SUBST(GDAL_LDADD)
])

dnl ------------------------------------------------------------------------
dnl Detect GEOS
dnl
dnl use AQ_CHECK_GEOS to detect GEOS
dnl it sets:
dnl   GEOS_CFLAGS
dnl   GEOS_LDADD
dnl ------------------------------------------------------------------------

# Check for GEOS

AC_DEFUN([AQ_CHECK_GEOS],
[
AC_ARG_WITH([geos],
  AC_HELP_STRING([--with-geos=path],
    [Full path to 'geos-config' script, e.g. '--with-geos=/usr/local/bin/geos-config']),
  [ac_geos_config_path=$withval])

if test x"$ac_geos_config_path" = x ; then
  ac_geos_config_path=`which geos-config`
fi

ac_geos_config_path=`dirname $ac_geos_config_path 2> /dev/null`
AC_PATH_PROG(GEOS_CONFIG, geos-config, no, $ac_geos_config_path)

if test x${GEOS_CONFIG} = xno ; then
  AC_MSG_ERROR([geos-config not found! Supply it with --with-geos=PATH])
else
  ac_geos_version=`${GEOS_CONFIG} --version`
  if test `echo ${ac_geos_version} | sed -e 's#2\.0.*#OK#'` != OK ; then
    AC_MSG_ERROR([Geos Version 2.0.x is needed, but you have $ac_geos_version!])
  else
    AC_MSG_CHECKING([GEOS_CFLAGS])
    GEOS_CFLAGS=`$GEOS_CONFIG --cflags`
    AC_MSG_RESULT($GEOS_CFLAGS)

    AC_MSG_CHECKING([GEOS_LDADD])
    GEOS_LDADD=`$GEOS_CONFIG --libs`
    AC_MSG_RESULT($GEOS_LDADD)

    ac_geos="yes"
  fi
fi

AC_SUBST(GEOS_CFLAGS)
AC_SUBST(GEOS_LDADD)
])

dnl ------------------------------------------------------------------------                                                                             
dnl Detect QT3
dnl
dnl use AQ_CHECK_QT to detect QT3
dnl it sets:
dnl   QT_CXXFLAGS
dnl   QT_LDADD
dnl   QT_GUILINK
dnl   QASSISTANTCLIENT_LDADD
dnl ------------------------------------------------------------------------                                                                             

# Check for Qt compiler flags, linker flags, and binary packages

AC_DEFUN([AQ_CHECK_QT],
[
AC_REQUIRE([AC_PROG_CXX])
AC_REQUIRE([AC_PATH_X])

AC_MSG_CHECKING([QTDIR])
AC_ARG_WITH([qtdir], [  --with-qtdir=DIR        Qt installation directory [default=/usr/local]], QTDIR=$withval)
# Check that QTDIR is defined or that --with-qtdir given
if test x$QTDIR = x ; then
  QT_SEARCH="/usr/lib/qt31 /usr/local/qt31 /usr/lib/qt3 /usr/local/qt3 /usr/lib/qt2 /usr/local/qt2 /usr/lib/qt /usr/local/qt"
  for i in $QT_SEARCH; do
    if test x$QTDIR = x; then
      if test -f $i/include/qt/qglobal.h -o -f $i/include/qglobal.h; then
        QTDIR=$i
      fi
    fi
  done
fi
if test x$QTDIR = x ; then
  AC_MSG_ERROR([*** QTDIR must be defined, or --with-qtdir option given])
fi
AC_MSG_RESULT([$QTDIR])

# Change backslashes in QTDIR to forward slashes to prevent escaping
# problems later on in the build process, mainly for Cygwin build
# environment using MSVC as the compiler
# TODO: Use sed instead of perl
QTDIR=`echo $QTDIR | perl -p -e 's/\\\\/\\//g'`

# Check for QT includedir on Mac OSX
if test -f $QTDIR/include/qt/qglobal.h; then
  QTINC=$QTDIR/include/qt
else
  QTINC=$QTDIR/include
fi

# Figure out which version of Qt we are using
AC_MSG_CHECKING([Qt version])
QT_VER=`grep 'define.*QT_VERSION_STR\W' $QTINC/qglobal.h | perl -p -e 's/\D//g'`
case "${QT_VER}" in
  33*)
    QT_MAJOR="3"
    ;;
  32*)
    QT_MAJOR="3"
    ;;
  31*)
    QT_MAJOR="3"
    ;;
  *)
    AC_MSG_ERROR([*** Qt version 3.1.x or higher is required])
    ;;
esac
AC_MSG_RESULT([$QT_VER ($QT_MAJOR)])

# Check that moc is in path
AC_CHECK_PROG(MOC, moc, moc)
if test x$MOC = x ; then
  AC_MSG_ERROR([*** moc must be in path])
fi
# uic is the Qt user interface compiler
AC_CHECK_PROG(UIC, uic, uic)
if test x$UIC = x ; then
  AC_MSG_ERROR([*** uic must be in path])
fi
# qembed is the Qt data embedding utility.
# It is located in $QTDIR/tools/qembed, and must be compiled and installed
# manually, we'll let it slide if it isn't present
AC_CHECK_PROG(QEMBED, qembed, qembed)
# Calculate Qt include path
QT_CXXFLAGS="-I$QTINC"
QT_IS_EMBEDDED="no"
# On unix, figure out if we're doing a static or dynamic link

case "${host}" in
  *-cygwin)
    AC_DEFINE_UNQUOTED(WIN32, "", Defined if on Win32 platform)
    echo "$QTDIR/lib/qt-mt$QT_VER.lib"
    if test -f "$QTDIR/lib/qt-mt$QT_VER.lib" ; then
      QT_LIB="qt-mt$QT_VER.lib"
      QT_IS_STATIC="no"
      QT_IS_MT="yes"
         
    elif test -f "$QTDIR/lib/qt$QT_VER.lib" ; then
      QT_LIB="qt$QT_VER.lib"
      QT_IS_STATIC="no"
      QT_IS_MT="no"
    elif test -f "$QTDIR/lib/qt.lib" ; then
      QT_LIB="qt.lib"
      QT_IS_STATIC="yes"
      QT_IS_MT="no"
    elif test -f "$QTDIR/lib/qt-mt.lib" ; then
      QT_LIB="qt-mt.lib" 
      QT_IS_STATIC="yes"
      QT_IS_MT="yes"
    fi
    ;;
  *-darwin*)
    # determin static or dynamic -- prefer dynamic
    QT_IS_DYNAMIC=`ls $QTDIR/lib/libqt*.dylib 2> /dev/null`
    if test "x$QT_IS_DYNAMIC" = x;  then
      QT_IS_STATIC=`ls $QTDIR/lib/libqt*.a 2> /dev/null`
      if test "x$QT_IS_STATIC" = x; then
        QT_IS_STATIC="no"
        AC_MSG_ERROR([*** Couldn't find any Qt libraries])
      else
        QT_IS_STATIC="yes"
      fi
    else
      QT_IS_STATIC="no"
    fi
    # set link parameters based on shared/mt libs or static lib
    if test "x`ls $QTDIR/lib/libqt.a* 2> /dev/null`" != x ; then
      QT_LIB="-lqt"
      QT_IS_MT="no"
    elif test "x`ls $QTDIR/lib/libqt-mt.*.dylib 2> /dev/null`" != x ; then
      QT_LIB="-lqt-mt"
      QT_IS_MT="yes"
    elif test "x`ls $QTDIR/lib/libqt.*.dylib 2> /dev/null`" != x ; then
      QT_LIB="-lqt"
      QT_IS_MT="no"
    elif test "x`ls $QTDIR/lib/libqte.* 2> /dev/null`" != x ; then
      QT_LIB="-lqte"
      QT_IS_MT="no"
      QT_IS_EMBEDDED="yes"
    elif test "x`ls $QTDIR/lib/libqte-mt.* 2> /dev/null`" != x ; then
      QT_LIB="-lqte-mt"
      QT_IS_MT="yes"
      QT_IS_EMBEDDED="yes"
    fi
    ;;
  *)
    # determin static or dynamic -- prefer dynamic
    QT_IS_DYNAMIC=`ls $QTDIR/lib/libqt*.so 2> /dev/null`
    if test "x$QT_IS_DYNAMIC" = x;  then
      QT_IS_STATIC=`ls $QTDIR/lib/libqt*.a 2> /dev/null`
      if test "x$QT_IS_STATIC" = x; then
        QT_IS_STATIC="no"
        AC_MSG_ERROR([*** Couldn't find any Qt libraries])
      else
        QT_IS_STATIC="yes"
      fi
    else
      QT_IS_STATIC="no"
    fi
    # set link parameters based on shared/mt libs or static lib
    if test "x`ls $QTDIR/lib/libqt.a* 2> /dev/null`" != x ; then
      QT_LIB="-lqt"
      QT_IS_MT="no"
    elif test "x`ls $QTDIR/lib/libqt-mt.so* 2> /dev/null`" != x ; then
      QT_LIB="-lqt-mt"
      QT_IS_MT="yes"
    elif test "x`ls $QTDIR/lib/libqt.so* 2> /dev/null`" != x ; then
      QT_LIB="-lqt"
      QT_IS_MT="no"
    elif test "x`ls $QTDIR/lib/libqte.* 2> /dev/null`" != x ; then
      QT_LIB="-lqte"
      QT_IS_MT="no"
      QT_IS_EMBEDDED="yes"
    elif test "x`ls $QTDIR/lib/libqte-mt.* 2> /dev/null`" != x ; then
      QT_LIB="-lqte-mt"
      QT_IS_MT="yes"
      QT_IS_EMBEDDED="yes"
    fi
    ;;
esac

AC_MSG_CHECKING([if Qt is static])
AC_MSG_RESULT([$QT_IS_STATIC])
AC_MSG_CHECKING([if Qt is multithreaded])
AC_MSG_RESULT([$QT_IS_MT])
AC_MSG_CHECKING([if Qt is embedded])
AC_MSG_RESULT([$QT_IS_EMBEDDED])

QT_GUILINK=""
QASSISTANTCLIENT_LDADD="-lqassistantclient"
case "${host}" in
  *irix*)
    QT_LIBS="$QT_LIB"
    if test $QT_IS_STATIC = yes ; then
    QT_LIBS="$QT_LIBS -L$x_libraries -lXext -lX11 -lm -lSM -lICE"
    fi
    ;;

  *linux*)
    QT_LIBS="$QT_LIB"
    if test $QT_IS_STATIC = yes && test $QT_IS_EMBEDDED = no; then
      QT_LIBS="$QT_LIBS -L$x_libraries -lXext -lX11 -lm -lSM -lICE -ldl -ljpeg"
    fi
    ;;

  *darwin*)
    QT_LIBS="$QT_LIB"
    if test $QT_IS_STATIC = yes && test $QT_IS_EMBEDDED = no; then
      QT_LIBS="$QT_LIBS -L$x_libraries -lXext -lX11 -lm -lSM -lICE -ldl -ljpeg"
    fi
    ;;

  *osf*) 
    # Digital Unix (aka DGUX aka Tru64)
    QT_LIBS="$QT_LIB"
    if test $QT_IS_STATIC = yes ; then
      QT_LIBS="$QT_LIBS -L$x_libraries -lXext -lX11 -lm -lSM -lICE"
    fi
    ;;

  *solaris*)
    QT_LIBS="$QT_LIB"
    if test $QT_IS_STATIC = yes ; then
      QT_LIBS="$QT_LIBS -L$x_libraries -lXext -lX11 -lm -lSM -lICE -lresolv -lsocket -lnsl"
    fi
    ;;

  *win*)
    # linker flag to suppress console when linking a GUI app on Win32
    QT_GUILINK="/subsystem:windows"
    if test $QT_MAJOR = "3" ; then
      if test $QT_IS_MT = yes ; then
        QT_LIBS="/nodefaultlib:libcmt"
      else
        QT_LIBS="/nodefaultlib:libc"
      fi
    fi

    if test $QT_IS_STATIC = yes ; then
      QT_LIBS="$QT_LIBS $QT_LIB kernel32.lib user32.lib gdi32.lib comdlg32.lib ole32.lib shell32.lib imm32.lib advapi32.lib wsock32.lib winspool.lib winmm.lib netapi32.lib"
      if test $QT_MAJOR = "3" ; then
        QT_LIBS="$QT_LIBS qtmain.lib"
      fi
    else
      QT_LIBS="$QT_LIBS $QT_LIB"        
      if test $QT_MAJOR = "3" ; then
        QT_CXXFLAGS="$QT_CXXFLAGS -DQT_DLL"
        QT_LIBS="$QT_LIBS qtmain.lib qui.lib user32.lib netapi32.lib"
      fi
    fi
    QASSISTANTCLIENT_LDADD="qassistantclient.lib"
    ;;
esac

if test x"$QT_IS_EMBEDDED" = "xyes" ; then
  QT_CXXFLAGS="-DQWS $QT_CXXFLAGS"
fi

if test x"$QT_IS_MT" = "xyes" ; then
  QT_CXXFLAGS="$QT_CXXFLAGS -D_REENTRANT -DQT_THREAD_SUPPORT"
fi

QT_LDADD="-L$QTDIR/lib $QT_LIBS"

if test x$QT_IS_STATIC = xyes ; then
  OLDLIBS="$LIBS"
  LIBS="$QT_LDADD"
  AC_CHECK_LIB(Xft, XftFontOpen, QT_LDADD="$QT_LDADD -lXft")
  LIBS="$LIBS"
fi

AC_MSG_CHECKING([QT_CXXFLAGS])
AC_MSG_RESULT([$QT_CXXFLAGS])
AC_MSG_CHECKING([QT_LDADD])
AC_MSG_RESULT([$QT_LDADD])

AC_SUBST(QT_CXXFLAGS)
AC_SUBST(QT_LDADD)
AC_SUBST(QT_GUILINK)
AC_SUBST(QASSISTANTCLIENT_LDADD)
])

