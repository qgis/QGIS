set(VCPKG_POLICY_EMPTY_INCLUDE_FOLDER enabled) # Numpy includes are stored in the module itself
set(VCPKG_POLICY_MISMATCHED_NUMBER_OF_BINARIES enabled)
set(VCPKG_BUILD_TYPE release) # No debug builds required for pure python modules since vcpkg does not install a debug python executable. 

#TODO: Fix E:\vcpkg_folders\numpy\installed\x64-windows-release\tools\python3\Lib\site-packages\numpy\testing\_private\extbuild.py

set(VCPKG_PYTHON3_BASEDIR "${CURRENT_HOST_INSTALLED_DIR}/tools/python3")
find_program(VCPKG_PYTHON3 NAMES python${PYTHON3_VERSION_MAJOR}.${PYTHON3_VERSION_MINOR} python${PYTHON3_VERSION_MAJOR} python PATHS "${VCPKG_PYTHON3_BASEDIR}" NO_DEFAULT_PATH)
find_program(VCPKG_CYTHON NAMES cython PATHS "${VCPKG_PYTHON3_BASEDIR}" "${VCPKG_PYTHON3_BASEDIR}/Scripts" NO_DEFAULT_PATH)

set(ENV{PYTHON3} "${VCPKG_PYTHON3}")
set(PYTHON3 "${VCPKG_PYTHON3}")
set(z_vcpkg_python_func_python ${VCPKG_PYTHON3})

vcpkg_add_to_path(PREPEND "${VCPKG_PYTHON3_BASEDIR}")
if(VCPKG_TARGET_IS_WINDOWS)
  vcpkg_add_to_path(PREPEND "${VCPKG_PYTHON3_BASEDIR}/Scripts")
endif()

cmake_path(GET SCRIPT_MESON PARENT_PATH MESON_DIR)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO numpy/numpy
    REF v${VERSION}
    SHA512 0c3fa8fc09e929d1fdc723e9974e07057c9651fe1dac2704b984f482cfb25dbf8c43c13d43bf003677095e2a9c9914bec05336e82bddd0fc123e3bcb9e7471a2
    HEAD_REF main
)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH_SIMD
    REPO intel/x86-simd-sort
    REF 5adb33411f3cea8bdbafa9d91bd75bc4bf19c7dd
    SHA512 cdc08361b2f7f8480d5b8eb8f3cffbe1c86ef898f5780633bf53a60eb22cddd1571d1358490026b076767b776f53485c90730e354fcaf562c38e9fcc76ed2471
    HEAD_REF main
)

file(COPY "${SOURCE_PATH_SIMD}/" DESTINATION "${SOURCE_PATH}/numpy/_core/src/npysort/x86-simd-sort")

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH_MESON_NUMPY
    REPO numpy/meson
    REF 5d5a3d478da115c812be77afa651db2492d52171
    SHA512 7045d09b123fac0d305071283357e2ee66c6cd2b0459f62b7a27194c68bfc734bf2675ba49ca48fcc738e160dfea9b648e70bd9361afe42a8722c3dfd2f4fd3d
    HEAD_REF main
)

file(COPY "${SOURCE_PATH_MESON_NUMPY}/mesonbuild/modules/features" DESTINATION "${MESON_DIR}/mesonbuild/modules")

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH_SVML
    REPO numpy/SVML
    REF 3a713b13018325451c1b939d3914ceff5ec68e19
    SHA512 aa2d1f83a7fdc1c5b31f51c4d8d3ffd2604be68011584ec30e1e18522f9b36c39d613e9e9e4e1b100548b5db42f3cb60d95d042f3d523802103de90f617a8b66
    HEAD_REF main
)

file(COPY "${SOURCE_PATH_SVML}/" DESTINATION "${SOURCE_PATH}/numpy/_core/src/umath/svml")

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH_HIGHWAY
    REPO google/highway
    REF ee36c837129310be19c17c9108c6dc3f6ae06942
    SHA512 8c2a34a329e9b4c239ded17f906756e79cfc6afd47711ce17eaf7ffab74ae8c7f60bd64b81cfa5eaa2338779998373e1a2c5cb4c97c7a2e8ca7b0514622e8bdb
    HEAD_REF main
)

file(COPY "${SOURCE_PATH_HIGHWAY}/" DESTINATION "${SOURCE_PATH}/numpy/_core/src/highway")

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH_POCKETFFT
    REPO mreineck/pocketfft
    REF 33ae5dc94c9cdc7f1c78346504a85de87cadaa12
    SHA512 2acd1b2c4419a2a817e5fdc7770e8f9dae991a7b45c115651eb4df489f28b7ae8d088806bc100434bb9a6c77c02018c3ee14315c3c02c0dc433f18d8fbf064ad
    HEAD_REF main
)

file(COPY "${SOURCE_PATH_POCKETFFT}/" DESTINATION "${SOURCE_PATH}/numpy/fft/pocketfft")

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH_PYTHONCAPI
    REPO python/pythoncapi-compat
    REF 8636bccf29adfa23463f810b3c2830f7cff1e933
    SHA512 7ee775682ddaa6a312a2bd9a723c9aabfd522ba29a438d80c7478262f8d9a31c2850b6d62a6c143d715e611155a879030f83cf8602c3c710dfa89e0a0de45f51
    HEAD_REF main
)

file(COPY "${SOURCE_PATH_PYTHONCAPI}/" DESTINATION "${SOURCE_PATH}/numpy/_core/src/common/pythoncapi-compat")

vcpkg_replace_string("${SOURCE_PATH}/meson.build" "py.dependency()" "dependency('python-3.${PYTHON3_VERSION_MINOR}', method : 'pkg-config')")

if(VCPKG_TARGET_IS_WINDOWS AND VCPKG_CROSSCOMPILING AND VCPKG_TARGET_ARCHITECTURE MATCHES "arm")
  set(opts 
      ADDITIONAL_PROPERTIES
      "longdouble_format = 'IEEE_DOUBLE_LE'"
  )
endif()

vcpkg_mesonpy_prepare_build_options(
    OUTPUT meson_opts
    ${opts}
)

z_vcpkg_setup_pkgconfig_path(CONFIG "RELEASE")

list(APPEND meson_opts
  "--python.platlibdir" 
  "${CURRENT_INSTALLED_DIR}/${PYTHON3_SITE}"
)

# needed so pythran can be found
vcpkg_add_to_path("${CURRENT_HOST_INSTALLED_DIR}/bin")


if (VCPKG_TARGET_IS_OSX)
  list(APPEND meson_opts
    "-Dblas=accelerate"
    "-Dlapack=accelerate"
  )
else()
  list(APPEND meson_opts
    "-Dblas=blas"
    "-Dlapack=lapack"
  )
endif()

list(JOIN meson_opts "\",\""  meson_opts)

vcpkg_python_build_and_install_wheel(
  SOURCE_PATH "${SOURCE_PATH}"
  OPTIONS 
    --config-json "{\"setup-args\" : [\"${meson_opts}\" ] }"
)

# Move pkgconfig from deep numpy install tree to standard location
file(MAKE_DIRECTORY "${CURRENT_PACKAGES_DIR}/lib/pkgconfig")
file(GLOB _numpy_pc_files "${CURRENT_PACKAGES_DIR}/${PYTHON3_SITE}/numpy/_core/lib/pkgconfig/*.pc")
if(NOT _numpy_pc_files)
  file(GLOB _numpy_pc_files "${CURRENT_PACKAGES_DIR}/lib/site-packages/numpy/_core/lib/pkgconfig/*.pc")
endif()
foreach(_pc_file IN LISTS _numpy_pc_files)
  get_filename_component(_pc_name "${_pc_file}" NAME)
  file(RENAME "${_pc_file}" "${CURRENT_PACKAGES_DIR}/lib/pkgconfig/${_pc_name}")
endforeach()
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/${PYTHON3_SITE}/numpy/_core/lib/pkgconfig")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/lib/site-packages/numpy/_core/lib/pkgconfig")
vcpkg_fixup_pkgconfig()

set(subdir "${CURRENT_PACKAGES_DIR}/${PYTHON3_SITE}/")
set(pyfile "${subdir}/numpy/__config__.py")
file(READ "${pyfile}" contents)
if (VCPKG_TARGET_IS_WINDOWS)
  string(REPLACE "/" "\\" python_executable ${VCPKG_PYTHON3})
else()
  set(python_executable ${VCPKG_PYTHON3})
endif()
string(REPLACE "from enum import Enum" "from enum import Enum\nimport sys" contents "${contents}")
string(REPLACE "r\"${python_executable}\"" "sys.executable" contents "${contents}")
string(REPLACE "${CURRENT_INSTALLED_DIR}" "$(prefix)" contents "${contents}")
string(REGEX REPLACE "r\"(\.\./)+([^\\/]+/)+site-packages/pythran" "r\"../pythran" contents "${contents}")
string(REGEX REPLACE "\"commands\": r\"[A-Za-z0-9_ .:\\/-]+[/\\]([A-Za-z0-9_-]+)${VCPKG_HOST_EXECUTABLE_SUFFIX}\"" "\"commands\": r\"\\1${VCPKG_HOST_EXECUTABLE_SUFFIX}\"" contents "${contents}")
file(WRITE "${pyfile}" "${contents}")

file(REMOVE_RECURSE
    "${CURRENT_PACKAGES_DIR}/debug/include"
    "${CURRENT_PACKAGES_DIR}/debug/share"
)

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE.txt")

# Add required Metadata for some python build plugins
file(WRITE "${CURRENT_PACKAGES_DIR}/${PYTHON3_SITE}/numpy-${VERSION}.dist-info/METADATA"
"Metadata-Version: 2.1\n\
Name: numpy\n\
Version: ${VERSION}"
)

# vcpkg_python_test_import(MODULE "numpy")
