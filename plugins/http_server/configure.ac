#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(plugin, 0.1, tim@linfiniti.com)

AC_CONFIG_SRCDIR([plugingui.h])

AM_INIT_AUTOMAKE(plugin, 0.1)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL

AC_LANG([C++])


# Checks for libraries.

dnl ---------------------------------------------------------------------------
dnl Qt check
dnl ---------------------------------------------------------------------------
gw_CHECK_QT


dnl
dnl Look for qgis headers
dnl
dnl XXX -DQGISDEBUG should be added iff --enable-debug is true
dnl
AC_CHECK_PROG( QGIS_CONFIG, qgis-config, yes,
               AC_MSG_ERROR([No Qgis C++ library -- get from http://qgis.sf.net/]) )

QGIS_CFLAGS="`qgis-config --cflags`"
QGIS_LDFLAGS="`qgis-config --libs`"

dnl must add Qt header search path as qgis headers, in turn, include Qt stuff
saved_CPPFLAGS=$CPPFLAGS
CPPFLAGS="$CPPFLAGS $QT_CXXFLAGS $QGIS_CFLAGS"

AC_CHECK_HEADER([qgis/qgsfeature.h],,
                 AC_MSG_ERROR([No Qgis C++ library -- get from http://qgis.sf.net/]))

CPPFLAGS=$saved_CPPFLAGS
dnl XXX add check for successful link against library


dnl
dnl Look for Frank Warmerdam's GDAL library
dnl

AC_CHECK_PROG( GDAL_CONFIG, gdal-config, yes,
               AC_MSG_ERROR([No GDAL library.  Get from http://remotesensing.org/gdal/]) )


if test $GDAL_CONFIG = "yes"; then
  GDAL_CFLAGS=`gdal-config --cflags`
  GDAL_LIBS=`gdal-config --libs`
fi

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST

# Checks for library functions.

AC_SUBST(QGIS_CFLAGS)
AC_SUBST(QGIS_LDFLAGS)
AC_SUBST(BOOST_PATH)
AC_SUBST(GDAL_CFLAGS)
AC_SUBST(GDAL_LIBS)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
