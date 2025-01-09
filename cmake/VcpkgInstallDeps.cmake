if(NOT WITH_VCPKG)
  return()
endif()

set(VCPKG_BASE_DIR "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}")

if(MSVC)
  file(GLOB ALL_LIBS
    "${VCPKG_BASE_DIR}/bin/*.dll"
  )
  install(FILES ${ALL_LIBS} DESTINATION "bin")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  file(GLOB ALL_LIBS
    "${VCPKG_BASE_DIR}/lib/*.dylib"
  )
  install(FILES ${ALL_LIBS} DESTINATION "${QGIS_LIB_SUBDIR}")
endif()

set(PROJ_DATA_PATH "${VCPKG_BASE_DIR}/share/proj")

if(NOT EXISTS "${PROJ_DATA_PATH}/proj.db")
    message(FATAL_ERROR "proj.db not found at ${PROJ_DATA_PATH}/proj.db")
endif()

install(DIRECTORY "${PROJ_DATA_PATH}/" DESTINATION "${QGIS_DATA_SUBDIR}/proj")
install(DIRECTORY "${VCPKG_BASE_DIR}/share/gdal/" DESTINATION "${QGIS_DATA_SUBDIR}/gdal")
if(MSVC)
  install(DIRECTORY "${VCPKG_BASE_DIR}/bin/Qca/crypto/" DESTINATION "${QGIS_LIB_SUBDIR}/Qt6/plugins/crypto") # QCA plugins
else()
  install(DIRECTORY "${VCPKG_BASE_DIR}/bin/Qca/crypto/" DESTINATION "${APP_PLUGINS_DIR}/crypto") # QCA plugins
endif()

if(MSVC)
  install(DIRECTORY "${VCPKG_BASE_DIR}/Qt6/" DESTINATION "${QGIS_LIB_SUBDIR}/Qt6") # qt plugins (qml and others)
else()
  install(DIRECTORY "${VCPKG_BASE_DIR}/Qt6/plugins/" DESTINATION "${APP_PLUGINS_DIR}/") # qt plugins (qml and others)
endif()

if(WITH_BINDINGS)
  if(MSVC)
    set(_SOURCE_PYTHON_DIR "${VCPKG_BASE_DIR}/tools/python3/")
    set(_TARGET_PYTHON_DIR "bin")
  elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    cmake_path(GET Python_SITEARCH PARENT_PATH _SOURCE_PYTHON_DIR)
    set(_TARGET_PYTHON_DIR "${APP_FRAMEWORKS_DIR}/lib")
  endif()

  install(DIRECTORY "${_SOURCE_PYTHON_DIR}"
    DESTINATION "${_TARGET_PYTHON_DIR}"
    PATTERN "*.sip" EXCLUDE)
endif()

set(BUNDLED_EXECUTABLES
  "tools/gdal/gdal_contour"
  "tools/gdal/gdal_create"
  "tools/gdal/gdal_footprint"
  "tools/gdal/gdal_grid"
  "tools/gdal/gdal_rasterize"
  "tools/gdal/gdal_translate"
  "tools/gdal/gdal_viewshed"
  "tools/gdal/gdaladdo"
  "tools/gdal/gdalbuildvrt"
  "tools/gdal/gdaldem"
  "tools/gdal/gdalenhance"
  "tools/gdal/gdalinfo"
  "tools/gdal/gdallocationinfo"
  "tools/gdal/gdalmanage"
  "tools/gdal/gdalmdiminfo"
  "tools/gdal/gdalmdimtranslate"
  "tools/gdal/gdalsrsinfo"
  "tools/gdal/gdaltindex"
  "tools/gdal/gdaltransform"
  "tools/gdal/gdalwarp"
  "tools/gdal/gnmanalyse"
  "tools/gdal/gnmmanage"
  "tools/gdal/nearblack"
  "tools/gdal/ogr2ogr"
  "tools/gdal/ogrinfo"
  "tools/gdal/ogrlineref"
  "tools/gdal/ogrtindex"
  "tools/gdal/sozip"
)
if(NOT MSVC)
  list(TRANSFORM BUNDLED_EXECUTABLES PREPEND "${VCPKG_BASE_DIR}/")
  install(PROGRAMS ${BUNDLED_EXECUTABLES}
    DESTINATION "${QGIS_BIN_SUBDIR}")
endif()

