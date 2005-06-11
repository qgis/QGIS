#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(cdpwizard, 0.3, tim@linfiniti.com)

AC_CONFIG_SRCDIR([plugin.h])

AM_INIT_AUTOMAKE(cdp_wizard, 0.3)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL

AC_LANG([C++])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST

# Checks for libraries.

dnl ---------------------------------------------------------------------------
dnl GDAL check
dnl ---------------------------------------------------------------------------
AQ_CHECK_GDAL

dnl ---------------------------------------------------------------------------
dnl Qt check
dnl ---------------------------------------------------------------------------
AQ_CHECK_LIB64
AQ_CHECK_QT

dnl ---------------------------------------------------------------------------
dnl QGIS check
dnl ---------------------------------------------------------------------------
AQ_CHECK_QGIS
QGISDATAPATH=`$QGIS_CONFIG --prefix`/share/qgis
AC_SUBST(QGISDATAPATH)



AC_CONFIG_FILES([Makefile])
AC_OUTPUT
