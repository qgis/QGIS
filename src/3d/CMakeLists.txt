#############################################################
# sources

SET(QGIS_3D_SRCS
  cameracontroller.cpp
  chunkboundsentity.cpp
  chunkedentity.cpp
  chunklist.cpp
  chunkloader.cpp
  chunknode.cpp
  demterraingenerator.cpp
  demterraintilegeometry.cpp
  flatterraingenerator.cpp
  lineentity.cpp
  map3d.cpp
  maptexturegenerator.cpp
  maptextureimage.cpp
  pointentity.cpp
  polygonentity.cpp
  polygongeometry.cpp
  #quantizedmeshgeometry.cpp
  #quantizedmeshterraingenerator.cpp
  scene.cpp
  terrain.cpp
  terrainchunkloader.cpp
  terraingenerator.cpp
  tessellator.cpp
  tilingscheme.cpp
  utils.cpp

  poly2tri/common/shapes.cc
  poly2tri/sweep/advancing_front.cc
  poly2tri/sweep/cdt.cc
  poly2tri/sweep/sweep_context.cc
  poly2tri/sweep/sweep.cc
)

SET(QGIS_3D_MOC_HDRS
  cameracontroller.h
  chunkedentity.h
  demterraingenerator.h
  demterraintilegeometry.h
  maptexturegenerator.h
  maptextureimage.h
  scene.h
  terrain.h
)

QT5_WRAP_CPP(QGIS_3D_MOC_SRCS ${QGIS_3D_MOC_HDRS})

QT5_ADD_RESOURCES(QGIS_3D_RCC_SRCS shaders.qrc)

# install headers

SET(QGIS_3D_HDRS
  aabb.h
  cameracontroller.h
  chunkboundsentity.h
  chunkedentity.h
  chunklist.h
  chunkloader.h
  chunknode.h
  demterraingenerator.h
  demterraintilegeometry.h
  flatterraingenerator.h
  lineentity.h
  map3d.h
  maptexturegenerator.h
  maptextureimage.h
  pointentity.h
  polygonentity.h
  polygongeometry.h
  #quantizedmeshgeometry.h
  #quantizedmeshterraingenerator.h
  scene.h
  terrain.h
  terrainchunkloader.h
  terraingenerator.h
  tessellator.h
  tilingscheme.h
  utils.h
)

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src/core/
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${CMAKE_SOURCE_DIR}/src/core/symbology-ng
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/3d
)
INCLUDE_DIRECTORIES(SYSTEM
  ${PROJ_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
  ${SQLITE3_INCLUDE_DIR}
)

#############################################################
# qgis_3d library

ADD_LIBRARY(qgis_3d SHARED ${QGIS_3D_SRCS} ${QGIS_3D_MOC_SRCS} ${QGIS_3D_HDRS} ${QGIS_3D_RCC_SRCS})

qt5_use_modules(qgis_3d 3DCore 3DRender 3DInput 3DLogic 3DExtras)

GENERATE_EXPORT_HEADER(
   qgis_3d
   BASE_NAME 3D
   EXPORT_FILE_NAME qgis_3d.h
)
IF(MSVC)
  SET_TARGET_PROPERTIES(qgis_3d PROPERTIES LINK_FLAGS "/FORCE:MULTIPLE")
ENDIF(MSVC)

SET(QGIS_3D_HDRS ${QGIS_3D_HDRS} ${CMAKE_CURRENT_BINARY_DIR}/qgis_3d.h)

IF(NOT APPLE)
  INSTALL(FILES ${QGIS_3D_HDRS} DESTINATION ${QGIS_INCLUDE_DIR})
ELSE(NOT APPLE)
  SET_TARGET_PROPERTIES(qgis_3d PROPERTIES
    # no moc headers, messes up PROPERTIES syntax
    CLEAN_DIRECT_OUTPUT 1
    FRAMEWORK 1
    FRAMEWORK_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}"
    MACOSX_FRAMEWORK_INFO_PLIST "${CMAKE_SOURCE_DIR}/mac/framework.info.plist.in"
    MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${COMPLETE_VERSION}
    MACOSX_FRAMEWORK_IDENTIFIER org.qgis.qgis3_3d
    BUILD_WITH_INSTALL_RPATH TRUE
    PUBLIC_HEADER "${QGIS_3D_HDRS}"
    LINK_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}"
  )
  # generated export header does not get copied with PUBLIC_HEADER files
  ADD_CUSTOM_COMMAND(TARGET qgis_3d
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy qgis_3d.h
            "${QGIS_OUTPUT_DIRECTORY}/${QGIS_LIB_SUBDIR}/qgis_3d.framework/Headers"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS qgis_3d.h
  )
ENDIF(NOT APPLE)

#generate unversioned libs for android
IF (NOT ANDROID)
  SET_TARGET_PROPERTIES(qgis_3d PROPERTIES
    VERSION ${COMPLETE_VERSION}
    SOVERSION ${COMPLETE_VERSION}
    )
ENDIF (NOT ANDROID)

TARGET_LINK_LIBRARIES(qgis_3d qgis_core)

# clang-tidy
IF(CLANG_TIDY_EXE)
  SET_TARGET_PROPERTIES(
    qgis_3d PROPERTIES
    CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
  )
ENDIF(CLANG_TIDY_EXE)

# install

INSTALL(TARGETS qgis_3d
  RUNTIME DESTINATION ${QGIS_BIN_DIR}
  LIBRARY DESTINATION ${QGIS_LIB_DIR}
  ARCHIVE DESTINATION ${QGIS_LIB_DIR}
  FRAMEWORK DESTINATION ${QGIS_FW_SUBDIR}
  PUBLIC_HEADER DESTINATION ${QGIS_INCLUDE_DIR})

# Mac dev frameworks

IF (APPLE AND QGIS_MACAPP_INSTALL_DEV)
  INSTALL(TARGETS qgis_3d FRAMEWORK DESTINATION ${QGIS_MACAPP_DEV_PREFIX})
  INSTALL(CODE "EXECUTE_PROCESS(COMMAND install_name_tool -id \"${QGIS_MACAPP_DEV_PREFIX}/qgis_3d.framework/Versions/${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}/qgis_3d\" \"$ENV{DESTDIR}${QGIS_MACAPP_DEV_PREFIX}/qgis_3d.framework/qgis_3d\")")
  INSTALL(CODE "EXECUTE_PROCESS(COMMAND install_name_tool -change \"${CMAKE_INSTALL_NAME_DIR}/qgis_core.framework/Versions/${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}/qgis_core\" \"${QGIS_MACAPP_DEV_PREFIX}/qgis_core.framework/Versions/${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}/qgis_core\" \"$ENV{DESTDIR}${QGIS_MACAPP_DEV_PREFIX}/qgis_3d.framework/qgis_3d\")")
ENDIF (APPLE AND QGIS_MACAPP_INSTALL_DEV)


#############################################################
# qgis 3d test executable (temporary)

add_executable(qgis3d
  testapp/main.cpp
  testapp/sidepanel.cpp
  testapp/window3d.cpp
)
target_link_libraries(qgis3d qgis_3d)
