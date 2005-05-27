#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(openmodeller, 0.3, tim@linfiniti.com)

AC_CONFIG_SRCDIR([openmodellergui.h])

AM_INIT_AUTOMAKE(openmodeller, 0.3)

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

dnl ---------------------------------------------------------------------------
dnl Checking for openmodeller
dnl ---------------------------------------------------------------------------
AC_ARG_WITH([om],
  AC_HELP_STRING([--with-om=path],
    [Path to openmodeller includes and libraries, e.g. '--with-om=/usr/local']),
  [ac_om_config_path=$withval])
AC_MSG_CHECKING([for openmodeller])
for I in /usr /usr/local $ac_om_config_path; do
  if test -f ${I}/include/openmodeller/om.hh; then
    om_path=$I
  fi
done

if test "a$om_path" != a; then
  AC_MSG_RESULT([yes])
  OM_CXXFLAGS="-I$om_path/include -I$om_path/include/openmodeller"
  OM_LDADD="-lopenmodeller -L$om_path/lib"
  AC_SUBST(OM_CXXFLAGS)
  AC_SUBST(OM_LDADD)
else
  AC_MSG_ERROR([openmodeller includes not found, provide the path with --with-om])
fi


AC_CONFIG_FILES([Makefile])
AC_OUTPUT
