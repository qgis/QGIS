# Authentication method plugins
# (similar to how data provider plugin libs are structured and loaded)
# see current auth method plugins for example implementation

# core/auth/QgsAuthMethod.h is the abstract base class for the method
# gui/auth/QgsAuthMethodEdit.h is the abstract base class for the method's edit widget

# override default path where built files are put to allow running qgis without installing
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${QGIS_OUTPUT_DIRECTORY}/${QGIS_PLUGIN_SUBDIR})
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${QGIS_OUTPUT_DIRECTORY}/${QGIS_PLUGIN_SUBDIR})

# NOTE: the plugin's file name must follow this pattern to be found during auth method registry loading:
#       (.*)authmethod\.(so|dll)

add_subdirectory(basic)
add_subdirectory(esritoken)
add_subdirectory(identcert)
add_subdirectory(pkipaths)
add_subdirectory(pkipkcs12)
add_subdirectory(apiheader)
add_subdirectory(maptiler_hmacsha256)

if (WITH_OAUTH2_PLUGIN)
  add_subdirectory(oauth2)
endif()
