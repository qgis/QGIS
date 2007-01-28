
SUBDIRS(core ui gui providers plugins helpviewer)

IF (APPLE)
  SUBDIRS(mac)
ENDIF(APPLE)
