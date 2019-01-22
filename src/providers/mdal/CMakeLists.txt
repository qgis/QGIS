SET(MDAL_SRCS
  qgsmdalprovider.cpp
  qgsmdaldataitems.cpp
)
SET(MDAL_MOC_HDRS
  qgsmdalprovider.h
  qgsmdaldataitems.h
)

IF (WITH_GUI)
  SET(MDAL_SRCS ${MDAL_SRCS}
    qgsmdalsourceselect.cpp
  )
  SET(MDAL_MOC_HDRS ${MDAL_MOC_HDRS}
    qgsmdalsourceselect.h
  )
ENDIF ()

SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/external/mdal/cmake ${CMAKE_MODULE_PATH})
FIND_PACKAGE(HDF5)
FIND_PACKAGE(NetCDF)

########################################################
# Compile internal MDAL
IF (WITH_INTERNAL_MDAL)

  ADD_DEFINITIONS(-DMDAL_STATIC)

  INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/external/mdal
    ${CMAKE_SOURCE_DIR}/external/mdal/api
  )

  SET(MDAL_LIB_SRCS
    ${CMAKE_SOURCE_DIR}/external/mdal/mdal.cpp
    ${CMAKE_SOURCE_DIR}/external/mdal/mdal_utils.cpp
    ${CMAKE_SOURCE_DIR}/external/mdal/mdal_data_model.cpp
    ${CMAKE_SOURCE_DIR}/external/mdal/mdal_memory_data_model.cpp
    ${CMAKE_SOURCE_DIR}/external/mdal/mdal_driver_manager.cpp
    ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_driver.cpp
    ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_2dm.cpp
    ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_ascii_dat.cpp
    ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_binary_dat.cpp
  )

  SET(MDAL_LIB_HDRS
    ${CMAKE_SOURCE_DIR}/external/mdal/api/mdal.h
    ${CMAKE_SOURCE_DIR}/external/mdal/mdal_utils.hpp
    ${CMAKE_SOURCE_DIR}/external/mdal/mdal_data_model.hpp
    ${CMAKE_SOURCE_DIR}/external/mdal/mdal_memory_data_model.hpp
    ${CMAKE_SOURCE_DIR}/external/mdal/mdal_driver_manager.hpp
    ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_driver.hpp
    ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_2dm.hpp
    ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_ascii_dat.hpp
    ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_binary_dat.hpp
  )

  IF(HDF5_FOUND)
    SET(MDAL_LIB_SRCS ${MDAL_LIB_SRCS}
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_hdf5.cpp
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_xmdf.cpp
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_hec2d.cpp
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_flo2d.cpp
    )
    SET(MDAL_LIB_HDRS ${MDAL_LIB_HDRS}
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_hdf5.hpp
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_xmdf.hpp
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_hec2d.hpp
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_flo2d.hpp
    )
    SET(MDAL_DEPS ${MDAL_DEPS} )
    SET (HAVE_HDF5 TRUE)
  ENDIF(HDF5_FOUND)

  IF(GDAL_FOUND)
    SET(MDAL_LIB_SRCS ${MDAL_LIB_SRCS}
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_gdal.cpp
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_gdal_grib.cpp
    )
    SET(MDAL_LIB_HDRS ${MDAL_LIB_HDRS}
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_gdal.hpp
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_gdal_grib.hpp
    )
    SET (HAVE_GDAL TRUE)
  ENDIF(GDAL_FOUND)

  IF(NETCDF_FOUND)
    SET(MDAL_LIB_SRCS ${MDAL_LIB_SRCS}
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_cf.cpp
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_3di.cpp
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_netcdf.cpp
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_sww.cpp
    )
    SET(MDAL_LIB_HDRS ${MDAL_LIB_HDRS}
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_cf.hpp
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_3di.hpp
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_netcdf.hpp
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_sww.hpp
    )
    SET (HAVE_NETCDF TRUE)
  ENDIF(NETCDF_FOUND)

  IF(GDAL_FOUND AND NETCDF_FOUND)
    SET(MDAL_LIB_SRCS ${MDAL_LIB_SRCS}
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_gdal_netcdf.cpp
    )
    SET(MDAL_LIB_HDRS ${MDAL_LIB_HDRS}
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_gdal_netcdf.hpp
    )
  ENDIF(GDAL_FOUND AND NETCDF_FOUND)

  # create mdal_config.h
  CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/external/mdal/cmake_templates/mdal_config.hpp.in ${CMAKE_BINARY_DIR}/mdal_config.hpp)
  INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})

  UNSET(MDAL_LIBRARY)
  UNSET(MDAL_INCLUDE_DIR)

ELSE (WITH_INTERNAL_MDAL)

  INCLUDE_DIRECTORIES (SYSTEM
    ${MDAL_INCLUDE_DIR}
  )

ENDIF (WITH_INTERNAL_MDAL)

########################################################

INCLUDE_DIRECTORIES (
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/mesh
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/gui
  ${CMAKE_SOURCE_DIR}/src/gui/auth

  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
  ${CMAKE_BINARY_DIR}/src/ui

)

QT5_WRAP_CPP(MDAL_MOC_SRCS ${MDAL_MOC_HDRS})
ADD_LIBRARY (mdalprovider MODULE ${MDAL_SRCS} ${MDAL_MOC_HDRS} ${MDAL_MOC_SRCS} ${MDAL_LIB_SRCS} ${MDAL_LIB_HDRS})

TARGET_LINK_LIBRARIES (mdalprovider
  qgis_core
  ${MDAL_LIBRARY}
)

IF (WITH_GUI)
  TARGET_LINK_LIBRARIES (mdalprovider
    qgis_gui
  )
ENDIF (WITH_GUI)

IF (HDF5_FOUND)
  TARGET_INCLUDE_DIRECTORIES(mdalprovider PRIVATE ${HDF5_INCLUDE_DIRS})
  TARGET_LINK_LIBRARIES(mdalprovider ${HDF5_C_LIBRARIES} )
ENDIF (HDF5_FOUND)

IF (GDAL_FOUND)
  TARGET_INCLUDE_DIRECTORIES(mdalprovider PRIVATE ${GDAL_INCLUDE_DIR})
  TARGET_LINK_LIBRARIES(mdalprovider ${GDAL_LIBRARY} )
ENDIF (GDAL_FOUND)

IF (NETCDF_FOUND)
  TARGET_INCLUDE_DIRECTORIES(mdalprovider PRIVATE ${NETCDF_INCLUDE_DIR})
  TARGET_LINK_LIBRARIES(mdalprovider ${NETCDF_LIBRARY} )
ENDIF (NETCDF_FOUND)

# clang-tidy
IF(CLANG_TIDY_EXE)
  SET_TARGET_PROPERTIES(
    mdalprovider PROPERTIES
    CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
  )
ENDIF(CLANG_TIDY_EXE)

INSTALL(TARGETS mdalprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

