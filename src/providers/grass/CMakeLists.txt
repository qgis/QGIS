#ADD_DEFINITIONS(-DGRASS_BASE=\\\"${GRASS_PREFIX}\\\")
########################################################
# Build

INCLUDE_DIRECTORIES(
  ../../core
  ../../core/geometry
  ../../core/raster
  ../../gui
  ${GDAL_INCLUDE_DIR}
  ${PROJ_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  ${POSTGRES_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

#
# GRASS library
#
MACRO(ADD_GRASSLIB GRASS_BUILD_VERSION)
    SET(GRASS_PREFIX ${GRASS_PREFIX${GRASS_BUILD_VERSION}})
    SET(GRASS_MAJOR_VERSION ${GRASS_MAJOR_VERSION${GRASS_BUILD_VERSION}})
    SET(GRASS_MINOR_VERSION ${GRASS_MINOR_VERSION${GRASS_BUILD_VERSION}})

    QT4_WRAP_CPP(GRASS_LIBRARY_MOC_SRCS
      ../qgsgrass.h
      ../qgsgrassprovider.h
      ../qgsgrassimport.h
      ../qgsgrassoptions.h
    )

    SET (GRASS_LIBRARY_SRCS
      ../qgsgrass.cpp
      ../qgsgrassdatafile.cpp
      ../qgsgrassfeatureiterator.cpp
      ../qgsgrassprovider.cpp
      ../qgsgrassimport.cpp
      ../qgsgrassoptions.cpp
    )

    QT4_WRAP_UI (GRASS_LIBRARY_UIS_H
      ../qgsgrassoptionsbase.ui
    )

    ADD_LIBRARY(qgisgrass${GRASS_BUILD_VERSION} SHARED
      ${GRASS_LIBRARY_SRCS}
      ${GRASS_LIBRARY_MOC_SRCS}
      ${GRASS_LIBRARY_UIS_H}
    )

    SET_TARGET_PROPERTIES(qgisgrass${GRASS_BUILD_VERSION} PROPERTIES
      CLEAN_DIRECT_OUTPUT 1
      FRAMEWORK 1
      FRAMEWORK_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}"
      MACOSX_FRAMEWORK_INFO_PLIST "${CMAKE_SOURCE_DIR}/mac/framework.info.plist.in"
      MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${COMPLETE_VERSION}
      MACOSX_FRAMEWORK_IDENTIFIER org.qgis.qgis2_grass
      COMPILE_FLAGS "-DGRASS_BASE=\\\"${GRASS_PREFIX}\\\" \"-DGRASS_LIB_EXPORT=${DLLEXPORT}\" \"-DGRASS_EXPORT=${DLLIMPORT}\""
    )

    IF (APPLE)
      SET_TARGET_PROPERTIES(qgisgrass${GRASS_BUILD_VERSION} PROPERTIES
        LINK_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}"
        )
    ENDIF (APPLE)

    #generate unversioned libs for android
    IF (NOT ANDROID)
      SET_TARGET_PROPERTIES(qgisgrass${GRASS_BUILD_VERSION} PROPERTIES
        VERSION ${COMPLETE_VERSION}
        SOVERSION ${COMPLETE_VERSION}
      )
    ENDIF (NOT ANDROID)

    IF (GRASS_MAJOR_VERSION LESS 7 )
      SET(GRASS_TARGET_LINK_LIBRARIES${GRASS_BUILD_VERSION}
          qgis_core
          qgis_gui
          ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_gis}
          ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_vect}
          ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_I}
          ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_dbmibase}
          ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_dbmiclient}
          ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_gproj}
      )
    ELSE (GRASS_MAJOR_VERSION LESS 7 )
      SET(GRASS_TARGET_LINK_LIBRARIES${GRASS_BUILD_VERSION}
        qgis_core
        qgis_gui
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_gis}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_vector}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_raster}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_imagery}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_dbmibase}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_dbmiclient}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_gproj}
      )
    ENDIF (GRASS_MAJOR_VERSION LESS 7 )

    TARGET_LINK_LIBRARIES(qgisgrass${GRASS_BUILD_VERSION} ${GRASS_TARGET_LINK_LIBRARIES${GRASS_BUILD_VERSION}})

    IF (APPLE)
      SET_TARGET_PROPERTIES(qgisgrass${GRASS_BUILD_VERSION} PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
    ENDIF (APPLE)

    #
    # GRASS vector provider
    #
    QT4_WRAP_CPP(GRASS_PROVIDER_MOC_SRCS ../qgsgrassprovidermodule.h)
    ADD_LIBRARY(grassprovider${GRASS_BUILD_VERSION} MODULE ../qgsgrassprovidermodule.cpp ${GRASS_PROVIDER_MOC_SRCS})
    SET_TARGET_PROPERTIES(grassprovider${GRASS_BUILD_VERSION} PROPERTIES
      COMPILE_FLAGS "-DGRASS_BASE=\\\"${GRASS_PREFIX}\\\" \"-DGRASS_EXPORT=${DLLEXPORT}\" \"-DGRASS_LIB_EXPORT=${DLLIMPORT}\""
    )
    TARGET_LINK_LIBRARIES(grassprovider${GRASS_BUILD_VERSION}
      qgisgrass${GRASS_BUILD_VERSION}
      qgis_gui
    )

    #
    # grass raster provider
    #
    QT4_WRAP_CPP(GRASS_RASTERPROVIDER_MOC_SRCS ../qgsgrassrasterprovider.h)
    ADD_LIBRARY(grassrasterprovider${GRASS_BUILD_VERSION} MODULE ../qgsgrassrasterprovider.cpp ${GRASS_RASTERPROVIDER_MOC_SRCS})
    SET_TARGET_PROPERTIES(grassrasterprovider${GRASS_BUILD_VERSION} PROPERTIES
      COMPILE_FLAGS "-DGRASS_BASE=\\\"${GRASS_PREFIX}\\\" \"-DGRASS_EXPORT=${DLLEXPORT}\" \"-DGRASS_LIB_EXPORT=${DLLIMPORT}\""
    )
    TARGET_LINK_LIBRARIES(grassrasterprovider${GRASS_BUILD_VERSION} qgisgrass${GRASS_BUILD_VERSION} qgis_core)

    # override default path where built files are put to allow running qgis without installing
    # the modules go under libexec subdir
    SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${QGIS_OUTPUT_DIRECTORY}/${QGIS_LIBEXEC_SUBDIR}/grass/modules)

    #
    # grass raster display module
    #
    ADD_EXECUTABLE(qgis.d.rast${GRASS_BUILD_VERSION} ../qgis.d.rast.c)
    IF (GRASS_MAJOR_VERSION LESS 7 )
      TARGET_LINK_LIBRARIES(qgis.d.rast${GRASS_BUILD_VERSION}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_gis}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_datetime}
        ${GDAL_LIBRARY}
      )
    ELSE (GRASS_MAJOR_VERSION LESS 7 )
      TARGET_LINK_LIBRARIES(qgis.d.rast${GRASS_BUILD_VERSION}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_gis}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_datetime}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_raster}
        ${GDAL_LIBRARY}
      )
    ENDIF (GRASS_MAJOR_VERSION LESS 7 )

    #
    # grass info module
    #
    ADD_EXECUTABLE(qgis.g.info${GRASS_BUILD_VERSION} ../qgis.g.info.c)
    IF (GRASS_MAJOR_VERSION LESS 7 )
      TARGET_LINK_LIBRARIES(qgis.g.info${GRASS_BUILD_VERSION}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_gis}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_datetime}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_gproj}
        ${GDAL_LIBRARY}
      )
    ELSE (GRASS_MAJOR_VERSION LESS 7 )
      TARGET_LINK_LIBRARIES(qgis.g.info${GRASS_BUILD_VERSION}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_gis}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_datetime}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_gproj}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_raster}
        ${GDAL_LIBRARY}
      )
    ENDIF (GRASS_MAJOR_VERSION LESS 7 )
    IF (UNIX)
      TARGET_LINK_LIBRARIES(qgis.g.info${GRASS_BUILD_VERSION} m)
    ENDIF (UNIX)

    #
    # grass raster import module
    #
    ADD_EXECUTABLE(qgis.r.in${GRASS_BUILD_VERSION} ../qgis.r.in.cpp)
    SET_TARGET_PROPERTIES(qgis.r.in${GRASS_BUILD_VERSION} PROPERTIES
      COMPILE_FLAGS "-DGRASS_BASE=\\\"${GRASS_PREFIX}\\\" \"-DGRASS_LIB_EXPORT=${DLLIMPORT}\" \"-DGRASS_EXPORT=${DLLIMPORT}\""
    )
    IF (GRASS_MAJOR_VERSION LESS 7 )
      TARGET_LINK_LIBRARIES(qgis.r.in${GRASS_BUILD_VERSION}
        qgisgrass${GRASS_BUILD_VERSION}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_gis}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_datetime}
        ${GDAL_LIBRARY}
        qgis_core
      )
    ELSE (GRASS_MAJOR_VERSION LESS 7 )
      TARGET_LINK_LIBRARIES(qgis.r.in${GRASS_BUILD_VERSION}
        qgisgrass${GRASS_BUILD_VERSION}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_gis}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_datetime}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_raster}
        ${GDAL_LIBRARY}
        qgis_core
      )
    ENDIF (GRASS_MAJOR_VERSION LESS 7 )

    #
    # grass vector import module
    #
    ADD_EXECUTABLE(qgis.v.in${GRASS_BUILD_VERSION} ../qgis.v.in.cpp)
    SET_TARGET_PROPERTIES(qgis.v.in${GRASS_BUILD_VERSION} PROPERTIES
      COMPILE_FLAGS "-DGRASS_BASE=\\\"${GRASS_PREFIX}\\\" \"-DGRASS_LIB_EXPORT=${DLLIMPORT}\" \"-DGRASS_EXPORT=${DLLIMPORT}\""
    )
    TARGET_LINK_LIBRARIES(qgis.v.in${GRASS_BUILD_VERSION}
      qgisgrass${GRASS_BUILD_VERSION}
      ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_gis}
      ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_datetime}
      ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_vect}
      ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_dbmibase}
      ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_dbmiclient}
      ${GDAL_LIBRARY}
      qgis_core
    )

    ########################################################
    # Install

    INSTALL(TARGETS qgisgrass${GRASS_BUILD_VERSION}
      RUNTIME DESTINATION ${QGIS_BIN_DIR}
      LIBRARY DESTINATION ${QGIS_LIB_DIR}
      FRAMEWORK DESTINATION ${QGIS_FW_SUBDIR})

    INSTALL(TARGETS grassprovider${GRASS_BUILD_VERSION}
      RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
      LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

    INSTALL(TARGETS grassrasterprovider${GRASS_BUILD_VERSION}
      RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
      LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

    INSTALL(TARGETS
      qgis.d.rast${GRASS_BUILD_VERSION}
      qgis.g.info${GRASS_BUILD_VERSION}
      qgis.r.in${GRASS_BUILD_VERSION}
      qgis.v.in${GRASS_BUILD_VERSION}
      RUNTIME DESTINATION ${QGIS_LIBEXEC_DIR}/grass/modules
      PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    )
ENDMACRO(ADD_GRASSLIB)

FOREACH(GRASS_BUILD_VERSION 6 7) # GRASS 6 and GRASS 7
  IF(GRASS_FOUND${GRASS_BUILD_VERSION})
    ADD_SUBDIRECTORY(${GRASS_BUILD_VERSION})
  ENDIF(GRASS_FOUND${GRASS_BUILD_VERSION})
ENDFOREACH(GRASS_BUILD_VERSION)

#
# Fake GRASS gis library
#
SET(WITH_GRASS_DIRECT FALSE) # currently disabled

IF(WITH_GRASS_DIRECT)
  # only process functions if qgsgrassgislibfunctions.cpp/h not present
  # or gisdefs.h is newer

  IF("${GRASS_INCLUDE_DIR}/grass/gisdefs.h" IS_NEWER_THAN "${CMAKE_CURRENT_BINARY_DIR}/qgsgrassgislibfunctions.cpp" OR "${GRASS_INCLUDE_DIR}/grass/gisdefs.h" IS_NEWER_THAN "${CMAKE_CURRENT_BINARY_DIR}/qgsgrassgislibfunctions.h")

  # Generate function mapping for functions used in original version
  # Create list of functions to be mapped
  SET ( FUNCTIONS
    "G_add_color_rule"
    "G_add_c_raster_color_rule"
    "G_add_d_raster_color_rule"
    "G_add_f_raster_color_rule"
    "G_add_modular_color_rule"
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
    "G_ask_any"
    "G_ask_in_mapset"
    "G_ask_new"
    "G_ask_old"
    "G__calloc"
    "G_chop"
    "G_chrcat"
    "G_chrcpy"
    "G_chrmov"
    "G_colors_count"
    "G_col_to_easting"
    "G_convert_dirseps_to_host"
    "G_copy"
    "G_copy_file"
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
    "G_format_timestamp"
    "G__write_timestamp"
    "G_free"
    "G_free_cats"
    "G_free_cell_stats"
    "G_free_colors"
    "G_free_histogram"
    "G_free_key_value"
    "G_free_raster_cats"
    "G_free_reclass"
    "G_free_tokens"
    "G_fseek"
    "G_ftell"
    "G_get_cat"
    "G_get_cats_title"
    "G_get_color"
    "G_get_color_range"
    "G_get_c_raster_cat"
    "G_get_c_raster_color"
    "G_get_d_raster_color"
    "G_get_f_raster_color"
    "G_get_d_color_range"
    "G_get_d_raster_cat"
    "G__getenv"
    "G__getenv2"
    "G_get_fp_range_min_max"
    "G_get_f_raster_cat"
    "G_get_gisrc_mode"
    "G_get_histogram_cat"
    "G_getl"
    "G_getl2"
    "G_get_next_marked_raster_cat"
    "G_get_range_min_max"
    "G_get_raster_cat"
    "G_get_raster_cats_title"
    "G_get_raster_color"
    "G_get_raster_row_colors"
    "G_get_raster_value_c"
    "G_get_raster_value_d"
    "G_get_raster_value_f"
    "G_gets"
    "G_get_set_window"
    "G_gettext"
    "G_get_window"
    "G_gisbase"
    "G_home"
    "G_incr_void_ptr"
    "G_index"
    "G_info_format"
    "G_init_cats"
    "G_init_cell_stats"
    "G_init_colors"
    "G_init_fp_range"
    "G__init_null_patterns"
    "G_init_range"
    "G_init_raster_cats"
    "G__insert_color_into_lookup"
    "G_interp_bicubic"
    "G_interp_bilinear"
    "G_interp_cubic"
    "G_interp_linear"
    "G_is_absolute_path"
    "G_is_c_null_value"
    "G_is_d_null_value"
    "G_is_f_null_value"
    "G_is_null_value"
    "G_lat_scan"
    "G_llres_scan"
    "G_lon_scan"
    "G_lookup_colors"
    "G_lookup_c_raster_colors"
    "G_lookup_d_raster_colors"
    "G_lookup_f_raster_colors"
    "G_lookup_raster_colors"
    "G__ls"
    "G_ls"
    "G_ls_format"
    "G__malloc"
    "G_make_fp_colors"
    "G_make_gyr_fp_colors"
    "G_make_random_colors"
    "G_mark_colors_as_fp"
    "G_mark_raster_cats"
    "G_mkdir"
    "G_next_cell_stat"
    "G_northing_to_row"
    "G_num_standard_colors"
    "G_number_of_tokens"
    "G__null_bitstream_size"
    "G__projection_name"
    "G_percent"
    "G_program_name"
    "G_projection"
    "G_quant_add_rule"
    "G_quant_free"
    "G_quant_get_limits"
    "G_quant_init"
    "G_quant_round"
    "G_quant_set_neg_infinite_rule"
    "G_putenv"
    "G_raster_cmp"
    "G_raster_cpy"
    "G_raster_size"
    "G_read_key_value_file"
    "G__realloc"
    "G_recreate_command"
    "G_rewind_cell_stats"
    "G_rewind_raster_cats"
    "G_rindex"
    "G_row_to_northing"
    "G_row_update_fp_range"
    "G_row_update_range"
    "G_scan_easting"
    "G_scan_northing"
    "G_scan_timestamp"
    "G_set_cat"
    "G_set_c_null_value"
    "G_set_d_null_value"
    "G_set_d_raster_cat"
    "G_set_color"
    "G_set_color_range"
    "G_setenv"
    "G_setenv2"
    "G_set_cats_fmt"
    "G_set_f_null_value"
    "G_set_gisrc_mode"
    "G_set_key_value"
    "G_set_null_value"
    "G_set_raster_cat"
    "G_set_raster_cats_title"
    "G_set_raster_value_d"
    "G_set_window"
    "G_sleep"
    "G_sleep_on_error"
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
    "G_suppress_warnings"
    "G_system"
    "G_tokenize"
    "G_trim_decimal"
    "G_unmark_raster_cats"
    "G_unset_error_routine"
    "G_update_cell_stats"
    "G_update_fp_range"
    "G_update_key_value_file"
    "G_update_range"
    "G_usage"
    "G_verbose"
    "G_verbose_min"
    "G_verbose_max"
    "G_wait"
    "G_whoami"
    "G_window_cols"
    "G_window_overlap"
    "G_window_rows"
    "G_write_key_value_file"
    "G_yes"
    "G_zero"
    "G_zero_cell_buf"
    "G_zero_raster_buf"
  )


  # List of functions which are implemented in qgsgrassgislib.cpp and
  # thus we only need prototype
  SET ( FUNCTION_PROTOTYPES
    "G_area_of_cell_at_row"
    "G_area_of_polygon"
    "G_ask_cell_new"
    "G_ask_cell_old"
    "G_asprintf"
    "G_begin_cell_area_calculations"
    "G_begin_distance_calculations"
    "G_begin_geodesic_distance"
    "G_begin_polygon_area_calculations"
    "G_check_input_output_name"
    "G_check_overwrite"
    "G_clear_screen"
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
    "G_find_vector"
    "G_find_vector2"
    "G_fopen_modify"
    "G_fopen_old"
    "G_fully_qualified_name"
    "G_geodesic_distance_lon_to_lon"
    "G_geodesic_distance"
    "G_get_cellhd"
    "G_get_cell_title"
    "G_get_c_raster_row"
    "G_get_c_raster_row_nomask"
    "G_get_d_raster_row"
    "G_get_d_raster_row_nomask"
    "G_get_ellipsoid_parameters"
    "G_get_f_raster_row"
    "G_get_f_raster_row_nomask"
    "G_get_map_row"
    "G_get_map_row_nomask"
    "G_get_null_value_row"
    "G_get_projinfo"
    "G_get_projunits"
    "G_get_raster_map_type"
    "G_get_raster_row"
    "G_get_raster_row_nomask"
    "G_get_reclass"
    "G__gisinit"
    "G_gisdbase"
    "G_important_message"
    "G_legal_filename"
    "G_location"
    "G_location_path"
    "G_lookup_key_value_from_file"
    "G_make_aspect_fp_colors"
    "G__make_mapset_element"
    "G_mapset"
    "G_maskfd"
    "G_mask_info"
    "G_message"
    "G__name_is_fully_qualified"
    "G_number_of_cats"
    "G_open_cell_new"
    "G_open_cell_old"
    "G_open_fp_cell_new"
    "G_open_new"
    "G_open_raster_new"
    "G_open_update"
    "G_parser"
    "G_put_cell_title"
    "G_put_c_raster_row"
    "G_put_f_raster_row"
    "G_put_d_raster_row"
    "G_put_raster_row"
    "G_quantize_fp_map_range"
    "G_raster_map_is_fp"
    "G_raster_map_type"
    "G_read_cats"
    "G_read_colors"
    "G_read_fp_range"
    "G_read_history"
    "G_read_quant"
    "G_read_range"
    "G_read_raster_cats"
    "G_remove"
    "G_rename"
    "G_round_fp_map"
    "G_set_cats_title"
    "G_set_error_routine"
    "G_set_geodesic_distance_lat1"
    "G_set_geodesic_distance_lat2"
    "G_set_quant_rules"
    "G_short_history"
    "G_snprintf"
    "G_spawn"
    "G_spawn_ex"
    "G__temp_element"
    "G_tempfile"
    "G_unopen_cell"
    "G_vasprintf"
    "G_verbose_message"
    "G_warning"
    "G_write_cats"
    "G__write_colors"
    "G_write_colors"
    "G_write_fp_range"
    "G_write_history"
    "G_write_range"
    "G_write_raster_cats"
    "G_write_raster_units"
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

  # Add functions defined in glocale.h, colors.h and spawn.h
  SET ( HEADER_FILE
    ${HEADER_FILE}
    "char *G_gettext(const char *, const char *);"
    "int G_num_standard_colors(void);"
    "int G_spawn(const char *, ...);"
    "int G_spawn_ex(const char *, ...);"
    "int G_wait(int);"
  )

  SET ( PROTOTYPES "// Auto generated by cmake, do not edit\n" )
  LIST ( APPEND PROTOTYPES "#define GRASS_GISDEFS_H\n" )
  LIST ( APPEND PROTOTYPES "extern \"C\"\n" )
  LIST ( APPEND PROTOTYPES "{\n" )
  LIST ( APPEND PROTOTYPES "#include <sys/types.h>\n" )
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
        STRING( REGEX REPLACE "[^*]*(\\*+) *G_.*" "\\1" POINTER "${ROW}" )
        IF ( (NOT "${POINTER}" STREQUAL "*") AND (NOT "${POINTER}" STREQUAL "**") )
          SET ( POINTER "" )
        ENDIF ( (NOT "${POINTER}" STREQUAL "*") AND (NOT "${POINTER}" STREQUAL "**") )
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

  ENDIF() # process functions

  # Build fake library
  IF(MSVC)
    SET (FAKE_LIB_GRASS_GIS "libgrass_gis.${GRASS_VERSION}")
  ELSE(MSVC)
    SET (FAKE_LIB_GRASS_GIS "grass_gis.${GRASS_VERSION}")
  ENDIF(MSVC)
  ADD_LIBRARY(${FAKE_LIB_GRASS_GIS} MODULE qgsgrassgislib.cpp qgsgrassgislibfunctions.cpp )

  # GRASS_LIBRARY_gis is path to the GRASS library used for compilation, it is the same
  # on runtime on Linux and Mac but on Windows with OSGEO4W the GRASS may be installed
  # in different directory
  SET_TARGET_PROPERTIES(${FAKE_LIB_GRASS_GIS} PROPERTIES
    CLEAN_DIRECT_OUTPUT 1
    COMPILE_FLAGS "\"-DGRASS_LIB_EXPORT=${DLLEXPORT}\" \"-DGRASS_EXPORT=${DLLIMPORT}\" -DGRASS_VERSION=\\\"${GRASS_VERSION}\\\" -DGRASS_LIBRARY_GIS=\\\"${GRASS_LIBRARY_gis}\\\" \"-I${CMAKE_CURRENT_SOURCE_DIR}\" ")

  IF (NOT APPLE)
    SET_TARGET_PROPERTIES(${FAKE_LIB_GRASS_GIS} PROPERTIES
      VERSION ${COMPLETE_VERSION}
      SOVERSION ${COMPLETE_VERSION}
      )
  ENDIF (NOT APPLE)

  TARGET_LINK_LIBRARIES(${FAKE_LIB_GRASS_GIS}
    qgis_core
  )

  INSTALL(TARGETS ${FAKE_LIB_GRASS_GIS}
    RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
    LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
ENDIF(WITH_GRASS_DIRECT)
