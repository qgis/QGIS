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

    get_filename_component(PYTHON_EXECUTABLE "${Python_EXECUTABLE}" NAME)
    install(PROGRAMS ${Python_EXECUTABLE}
      DESTINATION "${QGIS_BIN_SUBDIR}")
    configure_file("${CMAKE_SOURCE_DIR}/platform/macos/python.in" "${CMAKE_BINARY_DIR}/platform/macos/python" @ONLY)
    install(PROGRAMS  "${CMAKE_BINARY_DIR}/platform/macos/python"
      DESTINATION "${QGIS_BIN_SUBDIR}")
  endif()

  install(DIRECTORY "${_SOURCE_PYTHON_DIR}"
    DESTINATION "${_TARGET_PYTHON_DIR}"
    PATTERN "*.sip" EXCLUDE)
endif()

function(fixup_shebang INPUT_FILE OUTPUT_VARIABLE)
  get_filename_component(_FILE ${INPUT_FILE} NAME)
  file(READ ${INPUT_FILE} CONTENTS)
  string(REGEX MATCH "^#!" SHEBANG_PRESENT "${CONTENTS}")
  if (NOT SHEBANG_PRESENT)
    message(FATAL_ERROR "File ${INPUT_FILE} does not start with a shebang (#!).")
  endif()

  # Replace the first line
  string(REGEX REPLACE "^#![^\n]*" "#!/bin/sh\n\"exec\" \"\`dirname \$0\`/python\" \"\$0\" \"\$@\"" TRANSFORMED_CONTENTS "${CONTENTS}")
  
  # Write the transformed contents to the output file
  set(OUTPUT_FILE "${CMAKE_BINARY_DIR}/bundled_program/${_FILE}")
  file(WRITE "${OUTPUT_FILE}" "${TRANSFORMED_CONTENTS}")
  set(${OUTPUT_VARIABLE} ${OUTPUT_FILE} PARENT_SCOPE)
endfunction()

if(NOT MSVC)
  set(BUNDLED_PROGRAMS
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
  set(PYTHON_SCRIPTS
    "bin/gdal2tiles"
    "bin/gdal2tiles.py"
    "bin/gdal2xyz"
    "bin/gdal2xyz.py"
    "bin/gdal_calc"
    "bin/gdal_calc.py"
    "bin/gdal_edit"
    "bin/gdal_edit.py"
    "bin/gdal_fillnodata"
    "bin/gdal_fillnodata.py"
    "bin/gdal_merge"
    "bin/gdal_merge.py"
    "bin/gdal_pansharpen"
    "bin/gdal_pansharpen.py"
    "bin/gdal_polygonize"
    "bin/gdal_polygonize.py"
    "bin/gdal_proximity"
    "bin/gdal_proximity.py"
    "bin/gdal_retile"
    "bin/gdal_retile.py"
    "bin/gdal_sieve"
    "bin/gdal_sieve.py"
    "bin/gdalattachpct"
    "bin/gdalattachpct.py"
    "bin/gdalcompare"
    "bin/gdalcompare.py"
    "bin/gdalmove"
    "bin/gdalmove.py"
    "bin/ogr_layer_algebra"
    "bin/ogr_layer_algebra.py"
    "bin/ogrmerge"
    "bin/ogrmerge.py"
    "bin/pct2rgb"
    "bin/pct2rgb.py"
    "bin/rgb2pct"
    "bin/rgb2pct.py"
  )
  list(TRANSFORM BUNDLED_PROGRAMS PREPEND "${VCPKG_BASE_DIR}/")
  list(TRANSFORM PYTHON_SCRIPTS PREPEND "${VCPKG_BASE_DIR}/")
  foreach(FILE ${PYTHON_SCRIPTS})
    fixup_shebang("${FILE}" OUTPUT_FILE)
    list(APPEND BUNDLED_PROGRAMS "${OUTPUT_FILE}")
  endforeach()
  install(PROGRAMS ${BUNDLED_PROGRAMS}
    DESTINATION "${QGIS_BIN_SUBDIR}")
endif()

