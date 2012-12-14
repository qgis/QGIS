ADD_DEFINITIONS(-DGRASS_BASE=\\\"${GRASS_PREFIX}\\\")

########################################################
# Build

INCLUDE_DIRECTORIES(
  ../../core
  ../../core/raster
  ${GRASS_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
  ${PROJ_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

#
# GRASS library
#

QT4_WRAP_CPP(GRASS_PROVIDER_MOC_SRCS qgsgrassprovider.h)
ADD_LIBRARY(qgisgrass SHARED qgsgrass.cpp qgsgrassprovider.cpp ${GRASS_PROVIDER_MOC_SRCS})

SET_TARGET_PROPERTIES(qgisgrass PROPERTIES
  CLEAN_DIRECT_OUTPUT 1
  FRAMEWORK 1
  FRAMEWORK_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}"
  MACOSX_FRAMEWORK_INFO_PLIST "${CMAKE_SOURCE_DIR}/mac/framework.info.plist.in"
  MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${COMPLETE_VERSION}
  MACOSX_FRAMEWORK_IDENTIFIER org.qgis.qgisgrass
  COMPILE_FLAGS "\"-DGRASS_LIB_EXPORT=${DLLEXPORT}\" \"-DGRASS_EXPORT=${DLLIMPORT}\"" )

#generate unversioned libs for android
IF (NOT ANDROID)
  SET_TARGET_PROPERTIES(qgisgrass PROPERTIES
    VERSION ${COMPLETE_VERSION}
    SOVERSION ${COMPLETE_VERSION}
    )
ENDIF (NOT ANDROID)

TARGET_LINK_LIBRARIES(qgisgrass
  qgis_core
  ${GRASS_LIBRARY_gis}
  ${GRASS_LIBRARY_vect}
  ${GRASS_LIBRARY_dbmibase}
  ${GRASS_LIBRARY_dbmiclient}
  ${GRASS_LIBRARY_gproj}
)

IF (APPLE)
  SET_TARGET_PROPERTIES(qgisgrass PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
ENDIF (APPLE)

#
# Fake GRASS gis library
#

# Generate function mapping for functions used in original version
# Create list of functions to be mapped
SET ( FUNCTIONS
  "G_add_color_rule"
  "G_add_c_raster_color_rule"
  "G_add_d_raster_color_rule"
  "G_add_f_raster_color_rule"
  "G_add_raster_color_rule"
  "G_adjust_Cell_head"
  "G_adjust_easting"
  "G_adjust_east_longitude"
  "G_align_window"
  "G__allocate_null_bits"
  "G_allocate_cell_buf"
  "G_allocate_c_raster_buf"
  "G_allocate_d_raster_buf"
  "G_allocate_f_raster_buf"
  "G_allocate_null_buf"
  "G_allocate_raster_buf"
  "G__calloc"
  "G_chop"
  "G_chrcat"
  "G_chrcpy"
  "G_chrmov"
  "G_colors_count"
  "G_col_to_easting"
  "G_convert_dirseps_to_host"
  "G_create_key_value"
  "G_database_projection_name"
  "G_date"
  "G_define_flag"
  "G_define_module"
  "G_define_option"
  "G_define_standard_option"
  "G_easting_to_col"
  "G_find_key_value"
  "G_find_cell_stat"
  "G_fopen_new"
  "G_format_easting"
  "G_format_northing"
  "G_format_resolution"
  "G_free"
  "G_free_cats"
  "G_free_cell_stats"
  "G_free_key_value"
  "G_free_raster_cats"
  "G_fseek"
  "G_ftell"
  "G_get_cat"
  "G_get_c_raster_cat"
  "G_get_d_raster_cat"
  "G_get_fp_range_min_max"
  "G_get_f_raster_cat"
  "G_getl"
  "G_getl2"
  "G_get_range_min_max"
  "G_get_raster_cat"
  "G_get_raster_row_colors"
  "G_get_set_window"
  "G_gettext"
  "G_get_window"
  "G_incr_void_ptr"
  "G_init_colors"
  "G_init_fp_range"
  "G__init_null_patterns"
  "G_init_range"
  "G_init_raster_cats"
  "G_is_c_null_value"
  "G_is_d_null_value"
  "G_is_f_null_value"
  "G__ls"
  "G_ls"
  "G_ls_format"
  "G__malloc"
  "G_northing_to_row"
  "G__projection_name"
  "G_percent"
  "G_program_name"
  "G_projection"
  "G_quant_add_rule"
  "G_quant_free"
  "G_quant_get_limits"
  "G_quant_init"
  "G_raster_size"
  "G_read_key_value_file"
  "G__realloc"
  "G_row_to_northing"
  "G_set_c_null_value"
  "G_set_d_null_value"
  "G_set_d_raster_cat"
  "G_setenv"
  "G_set_f_null_value"
  "G_set_gisrc_mode"
  "G_set_key_value"
  "G_set_null_value"
  "G_set_raster_cat"
  "G_set_raster_cats_title"
  "G_set_raster_value_d"
  "G_set_window"
  "G_store"
  "G_strcasecmp"
  "G_strcat"
  "G_strchg"
  "G_strcpy"
  "G_strdup"
  "G_strend"
  "G_strip"
  "G_strmov"
  "G_strncpy"
  "G_str_replace"
  "G_strstr"
  "G_str_to_lower"
  "G_str_to_sql"
  "G_str_to_upper"
  "G_suppress_masking"
  "G_trim_decimal"
  "G_update_fp_range"
  "G_update_key_value_file"
  "G_update_range"
  "G_window_cols"
  "G_window_rows"
  "G_write_key_value_file"
  "G_zero"
)

# If a function, say G_1 called in true GRASS gis lib (loaded by QLibrary) 
# calls another function G_2 which does not have to be reimplemented in gis fake lib:
# - on Linux: G_2 is resolved in original GRASS true gis lib and it is not necessary
#             to add it to functions mapped in fake lib
# - on Windows: it seems that dynamic linker is searching for G_2 in the first loaded 
#               module of the same name, i.e. in our fake library, not in the original 
#               true GRASS gis lib, so we have to add all functions called in true lib
#               also to mapped functions in fake lib  

IF(MSVC)
SET ( FUNCTIONS
  ${FUNCTIONS}
  "G__getenv"
  "G_get_gisrc_mode"
)
ENDIF(MSVC)

# List of functions which are implemented in qgsgrassgislib.cpp and 
# thus we only need prototype
SET ( FUNCTION_PROTOTYPES
  "G_area_of_cell_at_row"
  "G_asprintf"
  "G_begin_cell_area_calculations"
  "G_begin_distance_calculations"
  "G_begin_geodesic_distance"
  "G_check_input_output_name"
  "G_check_overwrite"
  "G_close_cell"
  "G_command_history"
  "G_database_units_to_meters_factor"
  "G_debug"
  "G_distance"
  "G_done_msg"
  "G_fatal_error"
  "G__file_name"
  "G__file_name_misc"
  "G_find_cell"
  "G_find_cell2"
  "G_find_file"
  "G_find_file2"
  "G_find_file_misc"
  "G_find_file2_misc"
  "G_fully_qualified_name"
  "G_get_cellhd"
  "G_get_c_raster_row"
  "G_get_c_raster_row_nomask"
  "G_get_d_raster_row"
  "G_get_d_raster_row_nomask"
  "G_get_f_raster_row"
  "G_get_f_raster_row_nomask"
  "G_get_map_row"
  "G_get_map_row_nomask"
  "G_get_null_value_row"
  "G_get_raster_map_type"
  "G_get_raster_row"
  "G_get_raster_row_nomask"
  "G__gisinit"
  "G_legal_filename"
  "G_location"
  "G_make_aspect_fp_colors"
  "G_mapset"
  "G_maskfd"
  "G_message"
  "G_open_cell_new"
  "G_open_cell_old"
  "G_open_raster_new"
  "G_parser"
  "G_put_raster_row"
  "G_quantize_fp_map_range"
  "G_raster_map_is_fp"
  "G_read_colors"
  "G_read_fp_range"
  "G_read_history"
  "G_read_range"
  "G_read_raster_cats"
  "G_set_cats_title"
  "G_set_error_routine"
  "G_set_quant_rules"
  "G_short_history"
  "G__temp_element"
  "G_tempfile"
  "G_vasprintf"
  "G_verbose_message"
  "G_warning"
  "G_write_colors"
  "G_write_history"
  "G_write_raster_cats"
)

SET ( FUNCTIONS_ALL
  ${FUNCTIONS}
  ${FUNCTION_PROTOTYPES}
)

# Read GRASS header file and create functions mapping
FILE(READ "${GRASS_INCLUDE_DIR}/grass/gisdefs.h" HEADER_FILE)

# Function definitions in gisdefs.h may spread over more lines -> remove comments
# and split by ';'
# Remove comments and directives (some macros are lost)
STRING(REGEX REPLACE "(/\\*([^*]|[\r\n]|(\\*+([^*/]|[\r\n])))*\\*+/)" "" HEADER_FILE "${HEADER_FILE}")
STRING(REGEX REPLACE "#[^\r\n]*" "" HEADER_FILE "${HEADER_FILE}")

# Add G_gettext defined in glocale.h
SET ( HEADER_FILE 
  ${HEADER_FILE}
  "char *G_gettext(const char *, const char *);"
)

SET ( PROTOTYPES "// Auto generated by cmake, do not edit\n" )
LIST ( APPEND PROTOTYPES "#define GRASS_GISDEFS_H\n" )
LIST ( APPEND PROTOTYPES "extern \"C\"\n" )
LIST ( APPEND PROTOTYPES "{\n" )
LIST ( APPEND PROTOTYPES "#include <grass/gis.h>\n" )
#LIST ( APPEND PROTOTYPES "int GRASS_LIB_EXPORT G_set_error_routine(int (*)(const char *, int))\;\n" )

SET ( FUNCTIONS_MAP "// Auto generated by cmake, do not edit\n" )
LIST ( APPEND FUNCTIONS_MAP "#include \"qgsgrassgislibfunctions.h\"\n" )
LIST ( APPEND FUNCTIONS_MAP "#include \"qgsgrassgislib.h\"\n" )
LIST ( APPEND FUNCTIONS_MAP "#include \"qgslogger.h\"\n" )
LIST ( APPEND FUNCTIONS_MAP "#include \"qgis.h\"\n" )

FOREACH( ROW ${HEADER_FILE} )
  STRING(REGEX REPLACE "\n" " " ROW "${ROW}")
  STRING(REGEX REPLACE "__attribute__.*" "" ROW "${ROW}")
  #MESSAGE (STATUS, "ROW: ${ROW}")
  # Parse function declaration
  STRING( REGEX REPLACE ".*(G_[^\\(]*)\\(.*" "\\1" FUNCTION_NAME "${ROW}" )
  FOREACH( FN ${FUNCTIONS_ALL})
    IF ( "${FN}" STREQUAL "${FUNCTION_NAME}" )
      # \\*? and \\** patterns do not work, why?
      STRING( REGEX REPLACE "^[ \t]*(.*)G_.*" "\\1" FUNCTION_TYPE "${ROW}" )
      STRING( REGEX REPLACE "\\*" "" FUNCTION_TYPE "${FUNCTION_TYPE}" )
      STRING( REGEX REPLACE ".*(\\*+) *G_.*" "\\1" POINTER "${ROW}" )
      IF ( NOT "${POINTER}" STREQUAL "*" )
        SET ( POINTER "" )
      ENDIF ( NOT "${POINTER}" STREQUAL "*" )
      STRING( REGEX REPLACE ".*G_[^\\(]*\\((.*)\\).*" "\\1" PARAM_TYPES "${ROW}" )
      SET ( PARAM_NAMES "" )
      SET ( PARAMS "" )
      IF ( NOT "${PARAM_TYPES}" STREQUAL "void" )
        STRING ( REGEX MATCHALL "[^,]+" PARAM_TYPE_LIST ${PARAM_TYPES} )
        SET ( I 0 )
        FOREACH( PARAM_TYPE ${PARAM_TYPE_LIST} )
          LIST ( APPEND PARAM_NAMES "p${I}" )
          LIST ( APPEND PARAMS "${PARAM_TYPE} p${I}" )
          MATH(EXPR  I "${I} + 1")
        ENDFOREACH ( PARAM_TYPE )
      ENDIF ( NOT "${PARAM_TYPES}" STREQUAL "void" )

      STRING( REPLACE ";" ", " PARAM_NAMES "${PARAM_NAMES}" )
      STRING( REPLACE ";" ", " PARAMS "${PARAMS}" )

      # Declare all
      LIST ( APPEND PROTOTYPES "${FUNCTION_TYPE} GRASS_LIB_EXPORT ${POINTER} ${FUNCTION_NAME} ( ${PARAM_TYPES} )\;\n" )

      # Define only those not implemented in qgsgrassgislib.cpp 
      LIST (FIND FUNCTIONS "${FUNCTION_NAME}" FUNCTION_IDX)
      IF( ${FUNCTION_IDX} GREATER -1 ) 
        LIST ( APPEND FUNCTIONS_MAP "// ${ROW}\n" )
        # Declare function type
        LIST ( APPEND FUNCTIONS_MAP "typedef ${FUNCTION_TYPE} ${POINTER} ${FUNCTION_NAME}_type(${PARAM_TYPES})\;\n\n" )
        LIST ( APPEND FUNCTIONS_MAP "${FUNCTION_TYPE} GRASS_LIB_EXPORT ${POINTER} ${FUNCTION_NAME} ( ${PARAMS} ) {\n" )
        #LIST ( APPEND FUNCTIONS_MAP "  QgsDebugMsg( \"Entered\" )\;\n" )
        LIST ( APPEND FUNCTIONS_MAP "  ${FUNCTION_NAME}_type* fn = (${FUNCTION_NAME}_type*) cast_to_fptr (QgsGrassGisLib::instance()->resolve( \"${FUNCTION_NAME}\" ))\;\n" )
        LIST ( APPEND FUNCTIONS_MAP "  return fn( ${PARAM_NAMES} )\;\n")
        LIST ( APPEND FUNCTIONS_MAP "}\n\n" )
      ENDIF ( ${FUNCTION_IDX} GREATER -1 )
    ENDIF ( "${FN}" STREQUAL "${FUNCTION_NAME}" )
  ENDFOREACH ( FN )
ENDFOREACH( ROW )

LIST ( APPEND PROTOTYPES "}\n" )

FILE(WRITE "${CMAKE_CURRENT_BINARY_DIR}/qgsgrassgislibfunctions.cpp" ${FUNCTIONS_MAP})
FILE(WRITE "${CMAKE_CURRENT_BINARY_DIR}/qgsgrassgislibfunctions.h" ${PROTOTYPES})

# Build fake library
IF(MSVC)
  SET (FAKE_LIB_GRASS_GIS "libgrass_gis.${GRASS_VERSION}")
ELSE(MSVC)
  SET (FAKE_LIB_GRASS_GIS "grass_gis.${GRASS_VERSION}")
ENDIF(MSVC)
ADD_LIBRARY( ${FAKE_LIB_GRASS_GIS} MODULE qgsgrassgislib.cpp qgsgrassgislibfunctions.cpp )

SET_TARGET_PROPERTIES(${FAKE_LIB_GRASS_GIS} PROPERTIES
  CLEAN_DIRECT_OUTPUT 1
  COMPILE_FLAGS "\"-DGRASS_LIB_EXPORT=${DLLEXPORT}\" \"-DGRASS_EXPORT=${DLLIMPORT}\" -DGRASS_LIBRARY_GIS=\\\"${GRASS_LIBRARY_gis}\\\" \"-I${CMAKE_CURRENT_SOURCE_DIR}\" ")

IF (NOT APPLE)
  SET_TARGET_PROPERTIES(${FAKE_LIB_GRASS_GIS} PROPERTIES
    VERSION ${COMPLETE_VERSION}
    SOVERSION ${COMPLETE_VERSION}
    )
ENDIF (NOT APPLE)

TARGET_LINK_LIBRARIES(${FAKE_LIB_GRASS_GIS}
  qgis_core
)

#
# GRASS vector provider
#

ADD_LIBRARY(grassprovider MODULE qgsgrassprovidermodule.cpp)
SET_TARGET_PROPERTIES(grassprovider PROPERTIES COMPILE_FLAGS "\"-DGRASS_EXPORT=${DLLEXPORT}\" \"-DGRASS_LIB_EXPORT=${DLLIMPORT}\"" )
TARGET_LINK_LIBRARIES(grassprovider qgisgrass)

#
# grass raster provider
#

QT4_WRAP_CPP(GRASS_RASTERPROVIDER_MOC_SRCS qgsgrassrasterprovider.h)
ADD_LIBRARY(grassrasterprovider MODULE qgsgrassrasterprovider.cpp ${GRASS_RASTERPROVIDER_MOC_SRCS})
SET_TARGET_PROPERTIES(grassrasterprovider PROPERTIES COMPILE_FLAGS "\"-DGRASS_EXPORT=${DLLEXPORT}\" \"-DGRASS_LIB_EXPORT=${DLLIMPORT}\"" )
TARGET_LINK_LIBRARIES(grassrasterprovider qgisgrass qgis_core)

# override default path where built files are put to allow running qgis without installing
# the modules go under libexec subdir
SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${QGIS_OUTPUT_DIRECTORY}/${QGIS_LIBEXEC_SUBDIR}/grass/modules)

#
# grass raster display module
#

ADD_EXECUTABLE(qgis.d.rast qgis.d.rast.c)
TARGET_LINK_LIBRARIES(qgis.d.rast
  ${GRASS_LIBRARY_gis}
  ${GRASS_LIBRARY_datetime}
  ${GDAL_LIBRARY}
)

#
# grass info module
#

ADD_EXECUTABLE(qgis.g.info qgis.g.info.c)
TARGET_LINK_LIBRARIES(qgis.g.info
  ${GRASS_LIBRARY_gis}
  ${GRASS_LIBRARY_datetime}
  ${GRASS_LIBRARY_gproj}
  ${GDAL_LIBRARY}
)
IF (UNIX)
  TARGET_LINK_LIBRARIES(qgis.g.info m)
ENDIF (UNIX)

########################################################
# Install

INSTALL(TARGETS qgisgrass
  RUNTIME DESTINATION ${QGIS_BIN_DIR}
  LIBRARY DESTINATION ${QGIS_LIB_DIR}
  FRAMEWORK DESTINATION ${QGIS_FW_SUBDIR})

INSTALL(TARGETS ${FAKE_LIB_GRASS_GIS}
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

INSTALL(TARGETS grassprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

INSTALL(TARGETS grassrasterprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

INSTALL(TARGETS qgis.d.rast qgis.g.info
  RUNTIME DESTINATION ${QGIS_LIBEXEC_DIR}/grass/modules
  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)

