
if (WITH_DESKTOP)
  file (GLOB ICONS *.icns)
  install (FILES ${ICONS}
           DESTINATION "${DEFAULT_DATA_SUBDIR}")
endif()
