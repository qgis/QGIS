
if (WITH_DESKTOP)
  file (GLOB ICONS *.icns)
  install (FILES ${ICONS}
           DESTINATION ${CMAKE_INSTALL_PREFIX}/../Resources)
endif()
