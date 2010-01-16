
SUBDIRS(core analysis ui gui app providers plugins helpviewer)

IF (WITH_BINDINGS)
  SUBDIRS(python)
ENDIF (WITH_BINDINGS)

IF (APPLE)
  SUBDIRS(mac)
ENDIF(APPLE)
