if HAVE_SPIT
SPIT = spit
endif

if HAVE_POSTGRESDB
GEOPROCESSING = geoprocessing
endif

SUBDIRS = $(GEOPROCESSING) $(SPIT)

EXTRA_DIST = qgisplugin.h \
  plugin_builder.pl \
  plugin_template/icon.xpm \
  plugin_template/Makefile.am \
  plugin_template/plugin.cpp \
  plugin_template/pluginguibase.ui \
  plugin_template/pluginguibase.ui.h \
  plugin_template/plugingui.cpp \
  plugin_template/plugingui.h \
  plugin_template/plugin.h \
  plugin_template/README \
  maplayer/Makefile.am \
  maplayer/Makefile.in \
  maplayer/maplayertest.cpp \
  maplayer/maplayertest.h \
  maplayer/matrix1.xpm \
  example/Makefile.am \
  example/Makefile.in \
  example/exampleplugin.cpp \
  example/exampleplugin.h \
  example/matrix1.xpm
