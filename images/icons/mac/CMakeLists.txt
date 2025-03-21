
if (WITH_DESKTOP)
  file (GLOB ICONS *.icns)
  install (FILES ${ICONS}
           DESTINATION ${APP_RESOURCES_DIR})
endif()
