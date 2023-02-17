set(MDAL_SRCS
  qgsmdalprovider.cpp
)

if (WITH_GUI)
  set(MDAL_SRCS ${MDAL_SRCS}
    qgsmdalprovidergui.cpp
    qgsmdalsourceselect.cpp
  )
endif()

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/external/mdal/cmake ${CMAKE_MODULE_PATH})
if(MSVC)
  # HDF5 with enabled thread safety (HDF5_ENABLE_THREADSAFE=ON) on Windows needs Threads::Threads
  find_package(Threads)
endif()
find_package(HDF5)
find_package(NetCDF)
find_package(LibXml2)

########################################################
# Compile internal MDAL
if (WITH_INTERNAL_MDAL)

  add_definitions(-DMDAL_STATIC)

  include_directories(
    ${CMAKE_SOURCE_DIR}/external/mdal
    ${CMAKE_SOURCE_DIR}/external/mdal/api
    ${CMAKE_SOURCE_DIR}/external/mdal/3rdparty
  )

  set(MDAL_LIB_SRCS
    ${CMAKE_SOURCE_DIR}/external/mdal/mdal.cpp
    ${CMAKE_SOURCE_DIR}/external/mdal/mdal_utils.cpp
    ${CMAKE_SOURCE_DIR}/external/mdal/mdal_data_model.cpp
    ${CMAKE_SOURCE_DIR}/external/mdal/mdal_datetime.cpp
    ${CMAKE_SOURCE_DIR}/external/mdal/mdal_memory_data_model.cpp
    ${CMAKE_SOURCE_DIR}/external/mdal/mdal_driver_manager.cpp
    ${CMAKE_SOURCE_DIR}/external/mdal/mdal_logger.cpp
    ${CMAKE_SOURCE_DIR}/external/mdal/mdal_logger.hpp
    ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_driver.cpp
    ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_dynamic_driver.cpp
    ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_2dm.cpp
    ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_ascii_dat.cpp
    ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_binary_dat.cpp
    ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_selafin.cpp
    ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_esri_tin.cpp
    ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_xms_tin.cpp
    ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_ply.cpp
    ${CMAKE_SOURCE_DIR}/external/mdal/3rdparty/libplyxx/libplyxx.cpp
  )

  set(MDAL_LIB_HDRS
    ${CMAKE_SOURCE_DIR}/external/mdal/api/mdal.h
    ${CMAKE_SOURCE_DIR}/external/mdal/mdal_utils.hpp
    ${CMAKE_SOURCE_DIR}/external/mdal/mdal_data_model.hpp
    ${CMAKE_SOURCE_DIR}/external/mdal/mdal_datetime.hpp
    ${CMAKE_SOURCE_DIR}/external/mdal/mdal_memory_data_model.hpp
    ${CMAKE_SOURCE_DIR}/external/mdal/mdal_driver_manager.hpp
    ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_driver.hpp
    ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_dynamic_driver.hpp
    ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_2dm.hpp
    ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_ascii_dat.hpp
    ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_binary_dat.hpp
    ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_selafin.hpp
    ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_esri_tin.hpp
    ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_xms_tin.hpp
    ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_ply.hpp
    ${CMAKE_SOURCE_DIR}/external/mdal/3rdparty/libplyxx.h
    ${CMAKE_SOURCE_DIR}/external/mdal/3rdparty/libplyxx/libplyxx_internal.h
    ${CMAKE_SOURCE_DIR}/external/mdal/3rdparty/textio.h
    ${CMAKE_SOURCE_DIR}/external/nlohmann/json.hpp
  )

  if(HDF5_FOUND)
    set(MDAL_LIB_SRCS ${MDAL_LIB_SRCS}
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_hdf5.cpp
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_xmdf.cpp
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_hec2d.cpp
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_flo2d.cpp
    )
    set(MDAL_LIB_HDRS ${MDAL_LIB_HDRS}
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_hdf5.hpp
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_xmdf.hpp
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_hec2d.hpp
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_flo2d.hpp
    )
    set (HAVE_HDF5 TRUE)
  endif()

  if(SQLITE3_FOUND)
     set(HAVE_SQLITE3 TRUE)
  endif()

  if(SQLITE3_FOUND AND NETCDF_FOUND)
     set(MDAL_LIB_SRCS ${MDAL_LIB_SRCS}
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_sqlite3.cpp
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_3di.cpp
    )
    set(MDAL_LIB_HDRS ${MDAL_LIB_HDRS}
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_sqlite3.hpp
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_3di.hpp
    )
  endif()

  if(GDAL_FOUND)
    set(MDAL_LIB_SRCS ${MDAL_LIB_SRCS}
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_gdal.cpp
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_gdal_grib.cpp
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_h2i.cpp
    )
    set(MDAL_LIB_HDRS ${MDAL_LIB_HDRS}
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_gdal.hpp
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_gdal_grib.hpp
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_h2i.hpp
    )
    set (HAVE_GDAL TRUE)
  endif()

  if(NETCDF_FOUND)
    set(MDAL_LIB_SRCS ${MDAL_LIB_SRCS}
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_cf.cpp
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_netcdf.cpp
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_sww.cpp
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_ugrid.cpp
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_tuflowfv.cpp
    )
    set(MDAL_LIB_HDRS ${MDAL_LIB_HDRS}
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_cf.hpp
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_netcdf.hpp
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_sww.hpp
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_ugrid.hpp
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_tuflowfv.hpp
    )
    set (HAVE_NETCDF TRUE)
  endif()

  if(GDAL_FOUND AND NETCDF_FOUND)
    set(MDAL_LIB_SRCS ${MDAL_LIB_SRCS}
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_gdal_netcdf.cpp
    )
    set(MDAL_LIB_HDRS ${MDAL_LIB_HDRS}
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_gdal_netcdf.hpp
    )
  endif()

  if(HDF5_FOUND AND LIBXML2_FOUND)
    set(MDAL_LIB_SRCS ${MDAL_LIB_SRCS}
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_xml.cpp
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_xdmf.cpp
    )
    set(MDAL_LIB_HDRS ${MDAL_LIB_HDRS}
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_xml.hpp
      ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_xdmf.hpp
    )
    set (HAVE_XML TRUE)
  endif()


  # create mdal_config.h
  set (BUILD_PLY TRUE)
  configure_file(${CMAKE_SOURCE_DIR}/external/mdal/cmake_templates/mdal_config.hpp.in ${CMAKE_BINARY_DIR}/mdal_config.hpp)
  include_directories(${CMAKE_BINARY_DIR})

  unset(MDAL_LIBRARY)
  unset(MDAL_INCLUDE_DIR)

else()

  include_directories (SYSTEM
    ${MDAL_INCLUDE_DIR}
  )

endif()

########################################################

include_directories (

  ${CMAKE_BINARY_DIR}/src/ui
)

add_library (provider_mdal MODULE ${MDAL_SRCS} ${MDAL_LIB_SRCS} ${MDAL_LIB_HDRS})

# require c++17
target_compile_features(provider_mdal PRIVATE cxx_std_17)

target_link_libraries (provider_mdal
  qgis_core
  ${MDAL_LIBRARY}
)

if (WITH_INTERNAL_MDAL)
  target_link_libraries(provider_mdal ${CMAKE_DL_LIBS})
endif()

if (WITH_GUI)
  target_link_libraries (provider_mdal
    qgis_gui
  )
  add_dependencies(provider_mdal ui)
endif()

if (HDF5_FOUND)
  target_include_directories(provider_mdal PRIVATE ${HDF5_INCLUDE_DIRS})
  target_link_libraries(provider_mdal ${HDF5_C_LIBRARIES} )
  target_compile_definitions(provider_mdal PRIVATE ${HDF5_DEFINITIONS})
endif()

if (GDAL_FOUND)
  target_include_directories(provider_mdal PRIVATE ${GDAL_INCLUDE_DIR})
  target_link_libraries(provider_mdal ${GDAL_LIBRARY} )
endif()

if (NETCDF_FOUND)
  target_include_directories(provider_mdal PRIVATE ${NETCDF_INCLUDE_DIR})
  target_link_libraries(provider_mdal ${NETCDF_LIBRARY} )
endif()

if (LIBXML2_FOUND)
  target_include_directories(provider_mdal PRIVATE ${LIBXML2_INCLUDE_DIR})
  target_link_libraries(provider_mdal ${LIBXML2_LIBRARIES} )
  target_compile_definitions(provider_mdal PRIVATE ${LIBXML2_DEFINITIONS})
endif()

# clang-tidy
if(CLANG_TIDY_EXE)
  set_target_properties(
    provider_mdal PROPERTIES
    CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
  )
endif()

install(TARGETS provider_mdal
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

