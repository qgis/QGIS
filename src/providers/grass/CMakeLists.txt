#add_definitions(-DGRASS_BASE="${GRASS_PREFIX}")
########################################################
# Build

include_directories(

  ${CMAKE_BINARY_DIR}/src/providers/grass
  ${CMAKE_CURRENT_BINARY_DIR}
)
include_directories (SYSTEM
  ${GDAL_INCLUDE_DIR}
  ${PROJ_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  ${POSTGRES_INCLUDE_DIR}
)

#
# GRASS library
#
macro(ADD_GRASSLIB GRASS_BUILD_VERSION)
    set(GRASS_PREFIX ${GRASS_PREFIX${GRASS_BUILD_VERSION}})
    set(GRASS_MAJOR_VERSION ${GRASS_MAJOR_VERSION${GRASS_BUILD_VERSION}})
    set(GRASS_MINOR_VERSION ${GRASS_MINOR_VERSION${GRASS_BUILD_VERSION}})
    set(GRASS_OFF_T_SIZE ${GRASS_OFF_T_SIZE${GRASS_BUILD_VERSION}})

    set (GRASS_LIBRARY_SRCS
      ../qgsgrass.cpp
      ../qgsgrasswin.cpp
      ../qgsgrassdatafile.cpp
      ../qgsgrassfeatureiterator.cpp
      ../qgsgrassimport.cpp
      ../qgsgrassprovider.cpp
      ../qgsgrassrasterprovider.cpp
      ../qgsgrassundocommand.cpp
      ../qgsgrassvector.cpp
      ../qgsgrassvectormap.cpp
      ../qgsgrassvectormaplayer.cpp
    )

    if (WITH_GUI)
      set(GRASS_LIBRARY_SRCS ${GRASS_LIBRARY_SRCS}
        ../qgsgrassoptions.cpp
      )
    endif()

    if(MSVC)
      # 4611 setjmp
      set_source_files_properties(
        ../qgsgrassimport.cpp
        ../qgsgrassprovider.cpp
        ../qgsgrassvector.cpp
        ../qgsgrassvectormap.cpp
        ../qgsgrassvectormaplayer.cpp
        PROPERTIES COMPILE_FLAGS "-wd4611"
      )

      # 4611 setjmp
      # 4702 unreachable code
      set_source_files_properties(
        ../qgsgrass.cpp
        PROPERTIES COMPILE_FLAGS "-wd4611 -wd4702"
      )
    endif()

    if (WITH_QT6)
      QT6_WRAP_UI (GRASS_LIBRARY_UIS_H
	../qgsgrassoptionsbase.ui
      )
    else()
      QT5_WRAP_UI (GRASS_LIBRARY_UIS_H
	../qgsgrassoptionsbase.ui
      )
    endif()


    add_library(qgisgrass${GRASS_BUILD_VERSION} SHARED
      ${GRASS_LIBRARY_SRCS}
      ${GRASS_LIBRARY_UIS_H}
    )

    # require c++17
    target_compile_features(qgisgrass${GRASS_BUILD_VERSION} PRIVATE cxx_std_17)

    if(GRASS_OFF_T_SIZE)
      set(GRASS_OFF_T_SIZE_DEF "\"-DGRASS_OFF_T_SIZE=${GRASS_OFF_T_SIZE}\"")
    else()
      set(GRASS_OFF_T_SIZE_DEF "")
    endif()

    if(QGIS_MACAPP_FRAMEWORK)
      set_target_properties(qgisgrass${GRASS_BUILD_VERSION} PROPERTIES
        CLEAN_DIRECT_OUTPUT 1
        FRAMEWORK 1
        FRAMEWORK_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}"
        MACOSX_FRAMEWORK_INFO_PLIST "${CMAKE_SOURCE_DIR}/mac/framework.info.plist.in"
        MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${COMPLETE_VERSION}
        MACOSX_FRAMEWORK_IDENTIFIER org.qgis.qgis3_grass
        COMPILE_FLAGS "-DGRASS_BASE=\\\"${GRASS_PREFIX}\\\" ${GRASS_OFF_T_SIZE_DEF}"
      )
    else()
        set_target_properties(qgisgrass${GRASS_BUILD_VERSION} PROPERTIES
        CLEAN_DIRECT_OUTPUT 1
        FRAMEWORK 0
        COMPILE_FLAGS "-DGRASS_BASE=\\\"${GRASS_PREFIX}\\\" ${GRASS_OFF_T_SIZE_DEF}"
      )
    endif()

    if (APPLE)
      set_target_properties(qgisgrass${GRASS_BUILD_VERSION} PROPERTIES
        LINK_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}"
        )
    endif()

    #generate unversioned libs for android
    if (NOT ANDROID)
      set_target_properties(qgisgrass${GRASS_BUILD_VERSION} PROPERTIES
        VERSION ${COMPLETE_VERSION}
        SOVERSION ${COMPLETE_VERSION}
      )
    endif()

    if (GRASS_MAJOR_VERSION LESS 7 )
      set(GRASS_TARGET_LINK_LIBRARIES${GRASS_BUILD_VERSION}
          qgis_core
          ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_gis}
          ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_vect}
          ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_I}
          ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_dbmibase}
          ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_dbmiclient}
          ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_gproj}
      )
    else()
      set(GRASS_TARGET_LINK_LIBRARIES${GRASS_BUILD_VERSION}
        qgis_core
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_gis}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_vector}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_raster}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_imagery}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_dbmibase}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_dbmiclient}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_gproj}
      )
    endif()

    target_link_libraries(qgisgrass${GRASS_BUILD_VERSION} ${GRASS_TARGET_LINK_LIBRARIES${GRASS_BUILD_VERSION}})

    if (WITH_GUI)
      target_link_libraries(qgisgrass${GRASS_BUILD_VERSION}
        qgis_gui
      )
    endif()

    if (APPLE)
      set_target_properties(qgisgrass${GRASS_BUILD_VERSION} PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
    endif()

    GENERATE_EXPORT_HEADER(
       qgisgrass${GRASS_BUILD_VERSION}
       BASE_NAME GRASS_LIB
       EXPORT_FILE_NAME qgis_grass_lib.h
    )

    #
    # GRASS vector provider
    #
    set (GRASS_VECTOR_PROVIDER_SRCS ../qgsgrassprovidermodule.cpp)
    add_library(provider_grass${GRASS_BUILD_VERSION} MODULE ${GRASS_VECTOR_PROVIDER_SRCS})

    # require c++17
    target_compile_features(provider_grass${GRASS_BUILD_VERSION} PRIVATE cxx_std_17)

    set_target_properties(provider_grass${GRASS_BUILD_VERSION} PROPERTIES
      COMPILE_FLAGS "-DGRASS_BASE=\\\"${GRASS_PREFIX}\\\""
    )
    target_link_libraries(provider_grass${GRASS_BUILD_VERSION}
      qgisgrass${GRASS_BUILD_VERSION}
    )

    if (WITH_GUI)
      target_link_libraries(provider_grass${GRASS_BUILD_VERSION}
        qgis_gui
      )
      add_dependencies(provider_grass${GRASS_BUILD_VERSION} ui)
    endif()

    #
    # grass raster provider
    #
    set (GRASS_RASTER_PROVIDER_SRCS
      ../qgsgrassrasterprovidermodule.cpp
    )
    add_library(provider_grassraster${GRASS_BUILD_VERSION} MODULE ${GRASS_RASTER_PROVIDER_SRCS})

    # require c++17
    target_compile_features(provider_grassraster${GRASS_BUILD_VERSION} PRIVATE cxx_std_17)

    set_target_properties(provider_grassraster${GRASS_BUILD_VERSION} PROPERTIES
      COMPILE_FLAGS "-DGRASS_BASE=\\\"${GRASS_PREFIX}\\\""
    )
    target_link_libraries(provider_grassraster${GRASS_BUILD_VERSION}
      qgisgrass${GRASS_BUILD_VERSION}
      qgis_core
    )

    # override default path where built files are put to allow running qgis without installing
    # the modules go under libexec subdir
    set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${QGIS_OUTPUT_DIRECTORY}/${QGIS_LIBEXEC_SUBDIR}/grass/modules)

    #
    # grass raster display module
    #
    add_executable(qgis.d.rast${GRASS_BUILD_VERSION} ../qgis.d.rast.c)
    if (GRASS_MAJOR_VERSION LESS 7 )
      target_link_libraries(qgis.d.rast${GRASS_BUILD_VERSION}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_gis}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_datetime}
        ${GDAL_LIBRARY}
      )
    else()
      target_link_libraries(qgis.d.rast${GRASS_BUILD_VERSION}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_gis}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_datetime}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_raster}
        ${GDAL_LIBRARY}
      )
    endif()

    #
    # grass info module
    #
    add_executable(qgis.g.info${GRASS_BUILD_VERSION} ../qgis.g.info.c)
    if (GRASS_MAJOR_VERSION LESS 7 )
      target_link_libraries(qgis.g.info${GRASS_BUILD_VERSION}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_gis}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_datetime}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_gproj}
        ${GDAL_LIBRARY}
      )
    else()
      target_link_libraries(qgis.g.info${GRASS_BUILD_VERSION}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_gis}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_datetime}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_gproj}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_raster}
        ${GDAL_LIBRARY}
      )
    endif()
    if (UNIX)
      target_link_libraries(qgis.g.info${GRASS_BUILD_VERSION} m)
    endif()

    #
    # grass raster import module
    #
    add_executable(qgis.r.in${GRASS_BUILD_VERSION} ../qgis.r.in.cpp)
    set_target_properties(qgis.r.in${GRASS_BUILD_VERSION} PROPERTIES
      COMPILE_FLAGS "-DGRASS_BASE=\\\"${GRASS_PREFIX}\\\""
    )
    if (GRASS_MAJOR_VERSION LESS 7 )
      target_link_libraries(qgis.r.in${GRASS_BUILD_VERSION}
        qgisgrass${GRASS_BUILD_VERSION}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_gis}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_datetime}
        ${GDAL_LIBRARY}
        qgis_core
      )
    else()
      target_link_libraries(qgis.r.in${GRASS_BUILD_VERSION}
        qgisgrass${GRASS_BUILD_VERSION}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_gis}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_datetime}
        ${GRASS_LIBRARY${GRASS_BUILD_VERSION}_raster}
        ${GDAL_LIBRARY}
        qgis_core
      )
    endif()

    #
    # grass vector import module
    #
    add_executable(qgis.v.in${GRASS_BUILD_VERSION} ../qgis.v.in.cpp)
    set_target_properties(qgis.v.in${GRASS_BUILD_VERSION} PROPERTIES
      COMPILE_FLAGS "-DGRASS_BASE=\\\"${GRASS_PREFIX}\\\""
    )
    target_link_libraries(qgis.v.in${GRASS_BUILD_VERSION}
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

    install(TARGETS qgisgrass${GRASS_BUILD_VERSION}
      RUNTIME DESTINATION ${QGIS_BIN_DIR}
      LIBRARY DESTINATION ${QGIS_LIB_DIR}
      FRAMEWORK DESTINATION ${QGIS_FW_SUBDIR})

    install(TARGETS provider_grass${GRASS_BUILD_VERSION}
      RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
      LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

    install(TARGETS provider_grassraster${GRASS_BUILD_VERSION}
      RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
      LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

    install(TARGETS
      qgis.d.rast${GRASS_BUILD_VERSION}
      qgis.g.info${GRASS_BUILD_VERSION}
      qgis.r.in${GRASS_BUILD_VERSION}
      qgis.v.in${GRASS_BUILD_VERSION}
      RUNTIME DESTINATION ${QGIS_LIBEXEC_DIR}/grass/modules
      PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    )
endmacro(ADD_GRASSLIB)

foreach(GRASS_BUILD_VERSION 6 7 8)
  if(GRASS_FOUND${GRASS_BUILD_VERSION})
    add_subdirectory(${GRASS_BUILD_VERSION})
  endif()
endforeach(GRASS_BUILD_VERSION)

#
# Fake GRASS gis library
#
set(WITH_GRASS_DIRECT FALSE) # currently disabled

if(WITH_GRASS_DIRECT)
  # only process functions if qgsgrassgislibfunctions.cpp/h not present
  # or gisdefs.h is newer

  if("${GRASS_INCLUDE_DIR}/grass/gisdefs.h" IS_NEWER_THAN "${CMAKE_CURRENT_BINARY_DIR}/qgsgrassgislibfunctions.cpp" OR "${GRASS_INCLUDE_DIR}/grass/gisdefs.h" IS_NEWER_THAN "${CMAKE_CURRENT_BINARY_DIR}/qgsgrassgislibfunctions.h")

  # Generate function mapping for functions used in original version
  # Create list of functions to be mapped
  set ( FUNCTIONS
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
  set ( FUNCTION_PROTOTYPES
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

  set ( FUNCTIONS_ALL
    ${FUNCTIONS}
    ${FUNCTION_PROTOTYPES}
  )

  # Read GRASS header file and create functions mapping
  file(READ "${GRASS_INCLUDE_DIR}/grass/gisdefs.h" HEADER_FILE)

  # Function definitions in gisdefs.h may spread over more lines -> remove comments
  # and split by ';'
  # Remove comments and directives (some macros are lost)
  string(REGEX REPLACE "(/\\*([^*]|[\r\n]|(\\*+([^*/]|[\r\n])))*\\*+/)" "" HEADER_FILE "${HEADER_FILE}")
  string(REGEX REPLACE "#[^\r\n]*" "" HEADER_FILE "${HEADER_FILE}")

  # Add functions defined in glocale.h, colors.h and spawn.h
  set ( HEADER_FILE
    ${HEADER_FILE}
    "char *G_gettext(const char *, const char *);"
    "int G_num_standard_colors(void);"
    "int G_spawn(const char *, ...);"
    "int G_spawn_ex(const char *, ...);"
    "int G_wait(int);"
  )

  set ( PROTOTYPES "// Auto generated by cmake, do not edit\n" )
  list ( APPEND PROTOTYPES "#define GRASS_GISDEFS_H\n" )
  list ( APPEND PROTOTYPES "extern \"C\"\n" )
  list ( APPEND PROTOTYPES "{\n" )
  list ( APPEND PROTOTYPES "#include <sys/types.h>\n" )
  list ( APPEND PROTOTYPES "#include <grass/gis.h>\n" )
  #list ( APPEND PROTOTYPES "int GRASS_LIB_EXPORT G_set_error_routine(int (*)(const char *, int))\;\n" )

  set ( FUNCTIONS_MAP "// Auto generated by cmake, do not edit\n" )
  list ( APPEND FUNCTIONS_MAP "#include \"qgsgrassgislibfunctions.h\"\n" )
  list ( APPEND FUNCTIONS_MAP "#include \"qgsgrassgislib.h\"\n" )
  list ( APPEND FUNCTIONS_MAP "#include \"qgslogger.h\"\n" )
  list ( APPEND FUNCTIONS_MAP "#include \"qgis.h\"\n" )

  foreach( ROW ${HEADER_FILE} )
    string(REGEX REPLACE "\n" " " ROW "${ROW}")
    string(REGEX REPLACE "__attribute__.*" "" ROW "${ROW}")
    #message (STATUS, "ROW: ${ROW}")
    # Parse function declaration
    string( REGEX REPLACE ".*(G_[^\\(]*)\\(.*" "\\1" FUNCTION_NAME "${ROW}" )
    foreach( FN ${FUNCTIONS_ALL})
      if ( "${FN}" STREQUAL "${FUNCTION_NAME}" )
        # \\*? and \\** patterns do not work, why?
        string( REGEX REPLACE "^[ \t]*(.*)G_.*" "\\1" FUNCTION_TYPE "${ROW}" )
        string( REGEX REPLACE "\\*" "" FUNCTION_TYPE "${FUNCTION_TYPE}" )
        string( REGEX REPLACE "[^*]*(\\*+) *G_.*" "\\1" POINTER "${ROW}" )
        if ( (NOT "${POINTER}" STREQUAL "*") AND (NOT "${POINTER}" STREQUAL "**") )
          set ( POINTER "" )
        endif()
        string( REGEX REPLACE ".*G_[^\\(]*\\((.*)\\).*" "\\1" PARAM_TYPES "${ROW}" )
        set ( PARAM_NAMES "" )
        set ( PARAMS "" )
        if ( NOT "${PARAM_TYPES}" STREQUAL "void" )
          string ( REGEX MATCHALL "[^,]+" PARAM_TYPE_LIST ${PARAM_TYPES} )
          set ( I 0 )
          foreach( PARAM_TYPE ${PARAM_TYPE_LIST} )
            list ( APPEND PARAM_NAMES "p${I}" )
            list ( APPEND PARAMS "${PARAM_TYPE} p${I}" )
            math(EXPR  I "${I} + 1")
          endforeach ( PARAM_TYPE )
        endif()

        string( REPLACE ";" ", " PARAM_NAMES "${PARAM_NAMES}" )
        string( REPLACE ";" ", " PARAMS "${PARAMS}" )

        # Declare all
        list ( APPEND PROTOTYPES "${FUNCTION_TYPE} GRASS_LIB_EXPORT ${POINTER} ${FUNCTION_NAME} ( ${PARAM_TYPES} )\;\n" )

        # Define only those not implemented in qgsgrassgislib.cpp
        list (FIND FUNCTIONS "${FUNCTION_NAME}" FUNCTION_IDX)
        if( ${FUNCTION_IDX} GREATER -1 )
          list ( APPEND FUNCTIONS_MAP "// ${ROW}\n" )
          # Declare function type
          list ( APPEND FUNCTIONS_MAP "typedef ${FUNCTION_TYPE} ${POINTER} ${FUNCTION_NAME}_type(${PARAM_TYPES})\;\n\n" )
          list ( APPEND FUNCTIONS_MAP "${FUNCTION_TYPE} GRASS_LIB_EXPORT ${POINTER} ${FUNCTION_NAME} ( ${PARAMS} ) {\n" )
	  #list ( APPEND FUNCTIONS_MAP "  QgsDebugMsgLevel( \"Entered\", 4 )\;\n" )
          list ( APPEND FUNCTIONS_MAP "  ${FUNCTION_NAME}_type* fn = (${FUNCTION_NAME}_type*) cast_to_fptr (QgsGrassGisLib::instance()->resolve( \"${FUNCTION_NAME}\" ))\;\n" )
          list ( APPEND FUNCTIONS_MAP "  return fn( ${PARAM_NAMES} )\;\n")
          list ( APPEND FUNCTIONS_MAP "}\n\n" )
        endif()
      endif()
    endforeach ( FN )
  endforeach( ROW )

  list ( APPEND PROTOTYPES "}\n" )

  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/qgsgrassgislibfunctions.cpp" ${FUNCTIONS_MAP})
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/qgsgrassgislibfunctions.h" ${PROTOTYPES})

  endif() # process functions

  # Build fake library
  if(MSVC)
    set (FAKE_LIB_GRASS_GIS "libgrass_gis.${GRASS_VERSION}")
  else()
    set (FAKE_LIB_GRASS_GIS "grass_gis.${GRASS_VERSION}")
  endif()
  add_library(${FAKE_LIB_GRASS_GIS} MODULE qgsgrassgislib.cpp qgsgrassgislibfunctions.cpp )

  # require c++17
  target_compile_features(${FAKE_LIB_GRASS_GIS} PRIVATE cxx_std_17)

  # GRASS_LIBRARY_gis is path to the GRASS library used for compilation, it is the same
  # on runtime on Linux and Mac but on Windows with OSGEO4W the GRASS may be installed
  # in different directory
  set_target_properties(${FAKE_LIB_GRASS_GIS} PROPERTIES
    CLEAN_DIRECT_OUTPUT 1
    COMPILE_FLAGS "-DGRASS_VERSION=\\\"${GRASS_VERSION}\\\" -DGRASS_LIBRARY_GIS=\\\"${GRASS_LIBRARY_gis}\\\" \"-I${CMAKE_CURRENT_SOURCE_DIR}\" ")

  if (NOT APPLE)
    set_target_properties(${FAKE_LIB_GRASS_GIS} PROPERTIES
      VERSION ${COMPLETE_VERSION}
      SOVERSION ${COMPLETE_VERSION}
      )
  endif()

  target_link_libraries(${FAKE_LIB_GRASS_GIS}
    qgis_core
  )


  install(TARGETS ${FAKE_LIB_GRASS_GIS}
    RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
    LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
endif()
