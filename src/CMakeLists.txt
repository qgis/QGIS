SUBDIRS(core analysis ui gui app providers plugins helpviewer crssync browser)

IF (WITH_BINDINGS)
  SUBDIRS(python)
ENDIF (WITH_BINDINGS)

IF (WITH_MAPSERVER)
  SUBDIRS(mapserver)
ENDIF (WITH_MAPSERVER)

IF (WITH_ASTYLE)
  SUBDIRS(astyle)
ENDIF(WITH_ASTYLE)

IF (APPLE)
  SUBDIRS(mac)
ENDIF(APPLE)

