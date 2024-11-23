if(NOT WITH_VCPKG)
  return()
endif()

set(VCPKG_BASE_DIR "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}")

if(MSVC)
  # At least python3.dll, qgis_analysis.dll and gsl.dll are missing
  # Copy everything
  file(GLOB ALL_LIBS
    "${VCPKG_BASE_DIR}/bin/*.dll"
  )
  install(FILES ${ALL_LIBS} DESTINATION "bin")
endif()

set(PROJ_DATA_PATH "${VCPKG_BASE_DIR}/share/proj")

if(NOT EXISTS "${PROJ_DATA_PATH}/proj.db")
    message(FATAL_ERROR "proj.db not found at ${PROJ_DATA_PATH}/proj.db")
endif()

install(DIRECTORY "${PROJ_DATA_PATH}/" DESTINATION "${CMAKE_INSTALL_DATADIR}/proj")
install(DIRECTORY "${VCPKG_BASE_DIR}/share/gdal/" DESTINATION "${CMAKE_INSTALL_DATADIR}/gdal")
install(DIRECTORY "${VCPKG_BASE_DIR}/bin/Qca/" DESTINATION "bin/Qca") # QCA plugins
install(DIRECTORY "${VCPKG_BASE_DIR}/Qt6/" DESTINATION "bin/Qt6") # qt plugins (qml and others)
if(WITH_BINDINGS)
  install(DIRECTORY "${VCPKG_BASE_DIR}/tools/python3/"
    DESTINATION "bin"
    PATTERN "*.sip" EXCLUDE)
endif()
