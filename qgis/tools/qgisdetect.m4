dnl ------------------------------------------------------------------------
dnl Detect QGIS
dnl
dnl use AQ_CHECK_QGIS to detect QGIS
dnl it sets:
dnl   QGIS_CXXFLAGS
dnl   QGIS_LDADD
dnl ------------------------------------------------------------------------

# Check for QGIS compiler and linker flags
# Jens Oberender <j.obi@troja.net> 2004

AC_DEFUN([AQ_CHECK_QGIS],
[
dnl 
dnl Get the cflags and libraries from qgis-config
dnl
AC_ARG_WITH([qgis],
AC_HELP_STRING([--with-qgis=path],
  [Full path to 'qgis-config', e.g. --with-qgis=/usr/local/bin/qgis-config]),
  [ac_qgis_config_path=$withval])

if test x"$ac_qgis_config_path" = x ; then
  ac_qgis_config_path=`which qgis-config`
fi

ac_qgis_config_path=`dirname $ac_qgis_config_path 2> /dev/null`
AC_PATH_PROG(QGIS_CONFIG, qgis-config, no, $ac_qgis_config_path)

if test x"$QGIS_CONFIG" = xno ; then
  if test x"$ac_qgis_config_path" = x ; then
    AC_MSG_ERROR([qgis-config not found in $ac_qgis_config_path! Supply a path with --with-qgis=PATH])
  else
    AC_MSG_ERROR([qgis-config not found! Supply a path with --with-qgis=PATH])
  fi
else
  AC_MSG_CHECKING([QGIS_CXXFLAGS])
  QGIS_CXXFLAGS=`$QGIS_CONFIG --cflags`
  AC_MSG_RESULT($QGIS_CXXFLAGS)

  AC_MSG_CHECKING([QGIS_LDADD])
  QGIS_LDADD=`$QGIS_CONFIG --libs`
  AC_MSG_RESULT($QGIS_LDADD)

  AC_MSG_CHECKING([QGIS_PREFIX])
  QGIS_PREFIX=`$QGIS_CONFIG --prefix`
  AC_MSG_RESULT($QGIS_PREFIX)

  AC_MSG_CHECKING([QGIS_PLUGINPATH])
  QGIS_PLUGINPATH=`$QGIS_CONFIG --plugindir`
  AC_MSG_RESULT($QGIS_PLUGINPATH)

  AC_MSG_CHECKING([QGIS_MAJOR_VERSION])
  QGIS_MAJOR_VERSION=`$QGIS_CONFIG --major_version`
  AC_MSG_RESULT($QGIS_MAJOR_VERSION)

  AC_MSG_CHECKING([QGIS_MINOR_VERSION])
  QGIS_MINOR_VERSION=`$QGIS_CONFIG --minor_version`
  AC_MSG_RESULT($QGIS_MINOR_VERSION)

  AC_MSG_CHECKING([QGIS_MICRO_VERSION])
  QGIS_MICRO_VERSION=`$QGIS_CONFIG --micro_version`
  AC_MSG_RESULT($QGIS_MICRO_VERSION)

  ac_save_CXXFLAGS="$CXXFLAGS"
  ac_save_LDFLAGS="$LDFLAGS"
  CXXFLAGS="$CXXFLAGS $QGIS_CXXFLAGS $QT_CXXFLAGS"
  LDFLAGS="$LDFLAGS $QGIS_LDADD $QT_LDADD $GDAL_LDADD"

  case "${host}" in
    *darwin*)
      LDFLAGS="$LDFALGS -flat_namespace -undefined suppress"
      ;;
  esac

  AC_LINK_IFELSE([
  #include <qgsfeature.h>
  int main(int argc, char *argv[]) {
    QgsFeature * myQgsFeature = new QgsFeature(1);
    return 0;
  }
  ], [ac_qgis_linked='yes'], [ac_qgis_linked='no'])
  CXXFLAGS="$ac_save_CXXFLAGS"
  LDFLAGS="$ac_save_LDFLAGS"

  AC_MSG_CHECKING([if linking with QGIS works])
  if test x$ac_qgis_linked = xyes ; then
    AC_MSG_RESULT([yes])
  else
    AC_MSG_RESULT([no])
  fi
fi

AC_SUBST(QGIS_CXXFLAGS)
AC_SUBST(QGIS_LDADD)
AC_SUBST(QGIS_PREFIX)
AC_SUBST(QGIS_PLUGINPATH)
AC_SUBST(QGIS_MAJOR_VERSION)
AC_SUBST(QGIS_MINOR_VERSION)
AC_SUBST(QGIS_MICRO_VERSION)
])
