#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(plugin.cpp)

AC_CONFIG_SRCDIR([plugingui.h])

AM_INIT_AUTOMAKE(qgis-exif-importer, 0.1)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL

AC_LANG([C++])


# Checks for libraries.

dnl ---------------------------------------------------------------------------
dnl GDAL check
dnl ---------------------------------------------------------------------------
AQ_CHECK_GDAL

dnl ---------------------------------------------------------------------------
dnl Qt check
dnl ---------------------------------------------------------------------------
AQ_CHECK_QT

dnl ---------------------------------------------------------------------------
dnl QGIS check
dnl ---------------------------------------------------------------------------
AQ_CHECK_QGIS

dnl ---------------------------------------------------------------------------
dnl EXIF check
dnl ---------------------------------------------------------------------------
EXIF_CFLAGS=`pkg-config --cflags libexif`
EXIF_LDFLAGS=`pkg-config --libs libexif`

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST

# Checks for library functions.

AC_SUBST(QGIS_CFLAGS)
AC_SUBST(QGIS_LDFLAGS)
AC_SUBST(EXIF_CFLAGS)
AC_SUBST(EXIF_LDFLAGS)
AC_SUBST(BOOST_PATH)
AC_SUBST(GDAL_CFLAGS)
AC_SUBST(GDAL_LIBS)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
