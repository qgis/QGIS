
SUBDIRS(core ui gui app providers plugins helpviewer)

IF (APPLE)
  SUBDIRS(mac)
ENDIF(APPLE)
