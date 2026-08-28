vcpkg_download_distfile(
    ARCHIVE_PATH
    URLS "https://archive.apache.org/dist/arrow/arrow-${VERSION}/apache-arrow-${VERSION}.tar.gz"
    FILENAME apache-arrow-${VERSION}.tar.gz
    SHA512 c687e50dfcdbf7e0e39710224360d35d9aa734452b3a47adc8c101f3019b6b4116310c05b9f3cd0a5ed4ad9b7bd8fb88edb70e79b3cbd413a57e5e35e4554a6c
)
vcpkg_extract_source_archive(
    SOURCE_PATH
    ARCHIVE ${ARCHIVE_PATH}
    PATCHES
        0001-msvc-static-name.patch
        0003-android-musl.patch
        0004-android-datetime.patch
        0005-cmake-msvcruntime.patch
        0007-use-vcpkg-mimalloc.patch
        0008-cython-3.3-const-move.patch
)

# Check cpp/cmake_modules/DefineOptions.cmake for option dependencies -
# they must be modeled as feature dependencies in vcpkg.json.
vcpkg_check_features(OUT_FEATURE_OPTIONS FEATURE_OPTIONS
    FEATURES
        acero       ARROW_ACERO
        compute     ARROW_COMPUTE
        csv         ARROW_CSV
        cuda        ARROW_CUDA
        dataset     ARROW_DATASET
        filesystem  ARROW_FILESYSTEM
        flight      ARROW_FLIGHT
        flightsql   ARROW_FLIGHT_SQL
        gcs         ARROW_GCS
        jemalloc    ARROW_JEMALLOC
        json        ARROW_JSON
        mimalloc    ARROW_MIMALLOC
        orc         ARROW_ORC
        parquet     ARROW_PARQUET
        parquet     PARQUET_REQUIRE_ENCRYPTION
        s3          ARROW_S3
)

if(VCPKG_TARGET_IS_WINDOWS AND NOT VCPKG_TARGET_IS_MINGW)
    list(APPEND FEATURE_OPTIONS "-DARROW_USE_NATIVE_INT128=OFF")
endif()

if(VCPKG_TARGET_IS_WINDOWS AND VCPKG_TARGET_ARCHITECTURE STREQUAL "arm64")
    list(APPEND FEATURE_OPTIONS "-DARROW_SIMD_LEVEL=NONE")
endif()

string(COMPARE EQUAL ${VCPKG_LIBRARY_LINKAGE} "dynamic" ARROW_BUILD_SHARED)
string(COMPARE EQUAL ${VCPKG_LIBRARY_LINKAGE} "static" ARROW_BUILD_STATIC)
string(COMPARE EQUAL ${VCPKG_LIBRARY_LINKAGE} "dynamic" ARROW_DEPENDENCY_USE_SHARED)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}/cpp"
    OPTIONS
        ${FEATURE_OPTIONS}
        -DARROW_BUILD_SHARED=${ARROW_BUILD_SHARED}
        -DARROW_BUILD_STATIC=${ARROW_BUILD_STATIC}
        -DARROW_BUILD_TESTS=OFF
        -DARROW_DEPENDENCY_SOURCE=SYSTEM
        -DARROW_DEPENDENCY_USE_SHARED=${ARROW_DEPENDENCY_USE_SHARED}
        -DARROW_PACKAGE_KIND=vcpkg
        -DARROW_WITH_BROTLI=ON
        -DARROW_WITH_BZ2=ON
        -DARROW_WITH_LZ4=ON
        -DARROW_WITH_SNAPPY=ON
        -DARROW_WITH_ZLIB=ON
        -DARROW_WITH_ZSTD=ON
        -DBUILD_WARNING_LEVEL=PRODUCTION
        -DZSTD_MSVC_LIB_PREFIX=
    MAYBE_UNUSED_VARIABLES
        ZSTD_MSVC_LIB_PREFIX
)

vcpkg_cmake_install()
vcpkg_copy_pdbs()

vcpkg_fixup_pkgconfig()

if(EXISTS "${CURRENT_PACKAGES_DIR}/lib/arrow_static.lib")
    message(FATAL_ERROR "Installed lib file should be named 'arrow.lib' via patching the upstream build.")
endif()

if("dataset" IN_LIST FEATURES)
    vcpkg_cmake_config_fixup(
        PACKAGE_NAME arrowdataset
        CONFIG_PATH lib/cmake/ArrowDataset
        DO_NOT_DELETE_PARENT_CONFIG_PATH
    )
endif()

if("acero" IN_LIST FEATURES)
    vcpkg_cmake_config_fixup(
        PACKAGE_NAME arrowacero
        CONFIG_PATH lib/cmake/ArrowAcero
        DO_NOT_DELETE_PARENT_CONFIG_PATH
    )
endif()

if("compute" IN_LIST FEATURES)
    vcpkg_cmake_config_fixup(
        PACKAGE_NAME arrowcompute
        CONFIG_PATH lib/cmake/ArrowCompute
        DO_NOT_DELETE_PARENT_CONFIG_PATH
    )
endif()

if("flight" IN_LIST FEATURES)
    vcpkg_cmake_config_fixup(
        PACKAGE_NAME arrowflight
        CONFIG_PATH lib/cmake/ArrowFlight
        DO_NOT_DELETE_PARENT_CONFIG_PATH
    )
endif()

if("flightsql" IN_LIST FEATURES)
    vcpkg_cmake_config_fixup(
        PACKAGE_NAME arrowflightsql
        CONFIG_PATH lib/cmake/ArrowFlightSql
        DO_NOT_DELETE_PARENT_CONFIG_PATH
    )
endif()

if("parquet" IN_LIST FEATURES)
    vcpkg_cmake_config_fixup(
        PACKAGE_NAME parquet
        CONFIG_PATH lib/cmake/Parquet
        DO_NOT_DELETE_PARENT_CONFIG_PATH
    )
endif()

file(GLOB main_configs "${CURRENT_PACKAGES_DIR}/lib/cmake/Arrow/*onfig.cmake")
file(GLOB extra_configs "${CURRENT_PACKAGES_DIR}/lib/cmake/*/*onfig.cmake")
list(REMOVE_ITEM extra_configs ${main_configs})
if(NOT "${extra_configs}" STREQUAL "")
    message("${Z_VCPKG_BACKCOMPAT_MESSAGE_LEVEL}"
        "Unhandled CMake config: ${extra_configs}\n"
        "This might be caused by insufficient feature dependencies in ports/arrow/vcpkg.json."
    )
endif()
vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/Arrow)

file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
foreach(feature IN ITEMS parquet dataset acero compute flight flightsql)
    if(feature IN_LIST FEATURES)
        file(READ "${CMAKE_CURRENT_LIST_DIR}/usage-${feature}" feature_usage)
        file(APPEND "${CURRENT_PACKAGES_DIR}/share/${PORT}/usage" "${feature_usage}")
    endif()
endforeach()

if("example" IN_LIST FEATURES)
    file(INSTALL "${SOURCE_PATH}/cpp/examples/minimal_build/" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}/example")
endif()

if("python" IN_LIST FEATURES)
    # use the vcpkg-installed python so we create the wheel for the correct version
    message(STATUS "Building pyarrow")

    # only build release config for python
    set(VCPKG_POLICY_MISMATCHED_NUMBER_OF_BINARIES enabled)

    # pyarrow puts dlls in site-packages, which is where they should be
    set(VCPKG_POLICY_ALLOW_DLLS_IN_LIB enabled)
    
    if (VCPKG_TARGET_IS_WINDOWS)
        set(PYTHON3 "${CURRENT_HOST_INSTALLED_DIR}/tools/python3/python${VCPKG_HOST_EXECUTABLE_SUFFIX}")
    else()
        set(PYTHON3 "${CURRENT_HOST_INSTALLED_DIR}/tools/python3/python3${VCPKG_HOST_EXECUTABLE_SUFFIX}")
    endif()
    
    if(NOT EXISTS "${PYTHON3}")
        vcpkg_find_acquire_program(PYTHON3)
    endif()
    x_vcpkg_get_python_packages(
        PYTHON_VERSION 3
        PYTHON_EXECUTABLE "${PYTHON3}"
        REQUIREMENTS_FILE "${SOURCE_PATH}/python/requirements-build.txt"
        OUT_PYTHON_VAR PYTHON3_VENV
    )

    message(STATUS "Building and installing extension")
    set(ENV{Arrow_DIR} "${CURRENT_PACKAGES_DIR}/share/arrow")
    set(ENV{ArrowCompute_DIR} "${CURRENT_PACKAGES_DIR}/share/arrowcompute")
    if("dataset" IN_LIST FEATURES)
        set(ENV{ArrowDataset_DIR} "${CURRENT_PACKAGES_DIR}/share/arrowdataset")
    endif()
    if("parquet" IN_LIST FEATURES)
        set(ENV{Parquet_DIR} "${CURRENT_PACKAGES_DIR}/share/parquet")
    endif()
    if("acero" IN_LIST FEATURES)
        set(ENV{ArrowAcero_DIR} "${CURRENT_PACKAGES_DIR}/share/arrowacero")
    endif()
    if("flight" IN_LIST FEATURES)
        set(ENV{ArrowFlight_DIR} "${CURRENT_PACKAGES_DIR}/share/arrowflight")
    endif()
    if ("flightsql" IN_LIST FEATURES)
         set(ENV{ArrowFlightSql_DIR} "${CURRENT_PACKAGES_DIR}/share/arrowflightsql")
    endif()
    
    set(ENV{SETUPTOOLS_SCM_PRETEND_VERSION} "${VERSION}")
    set(ENV{PDM_BUILD_SCM_VERSION} "${VERSION}")

    if (NOT "${VCPKG_BUILD_TYPE}" STREQUAL "")
        set(build_opts "--build-type=${VCPKG_BUILD_TYPE}")
    else()
        set(build_opts "--build-type=release")
    endif()

    vcpkg_execute_required_process(
        COMMAND "${PYTHON3_VENV}" "setup.py"  
        "build_ext" ${build_opts} "--cmake-generator" "Ninja" "--rpath" "@loader_path/../../../"
        "install"  "--prefix" "${CURRENT_PACKAGES_DIR}" 
        LOGNAME "python-build-${TARGET_TRIPLET}"
        WORKING_DIRECTORY "${SOURCE_PATH}/python"
    )
endif()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/share/doc")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE.txt")
