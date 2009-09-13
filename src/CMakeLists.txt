
SUBDIRS(core analysis ui gui app providers plugins helpviewer)

IF (HAVE_PYTHON AND WITH_BINDINGS)
  SUBDIRS(python)
ENDIF (HAVE_PYTHON AND WITH_BINDINGS)

IF (APPLE)
  SUBDIRS(mac)
ENDIF(APPLE)
