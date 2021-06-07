########################################################
# Files

#[[
set(VIRTUAL_RASTER_PROVIDER_HDRS
 qgsvirtualrasterprovider.h
)
#]]
set(VRP_SRCS
 qgsvirtualrasterprovider.cpp
)

# static library
add_library(virtualrasterprovider_a STATIC
 ${VRP_SRCS}
)

# require c++17
target_compile_features(virtualrasterprovider_a PRIVATE cxx_std_17)

target_link_libraries(virtualrasterprovider_a
 qgis_core
 ${Qt5Xml_LIBRARIES}
 ${Qt5Core_LIBRARIES}
 ${Qt5Svg_LIBRARIES}
 ${Qt5Network_LIBRARIES}
 ${Qt5Concurrent_LIBRARIES}

)

target_compile_definitions(virtualrasterprovider_a PRIVATE "-DQT_NO_FOREACH")

if (FORCE_STATIC_PROVIDERS)
 # for (external) mobile apps to be able to pick up provider for linking
 install (TARGETS virtualrasterprovider_a ARCHIVE DESTINATION ${QGIS_PLUGIN_DIR})
 #[[
 if (WITH_GUI)
   install (TARGETS wmsprovider_gui_a ARCHIVE DESTINATION ${QGIS_PLUGIN_DIR})
 endif()
 ]]
else()
 # dynamically loaded module
 #add_library(virtualrasterprovider MODULE ${VIRTUAL_RASTER_PROVIDER_SRCS} ${VIRTUAL_RASTER_PROVIDER_HDRS})
 add_library(virtualrasterprovider MODULE ${VRP_SRCS})

 # require c++17
 target_compile_features(virtualrasterprovider PRIVATE cxx_std_17)

 target_link_libraries(virtualrasterprovider
   qgis_core
   #${GDAL_LIBRARY}  # for OGR_G_CreateGeometryFromJson()
 )


 target_compile_definitions(virtualrasterprovider PRIVATE "-DQT_NO_FOREACH")

 install (TARGETS virtualrasterprovider
   RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
   LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

endif()
