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

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST

# Checks for library functions.

AC_SUBST(BOOST_PATH)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
