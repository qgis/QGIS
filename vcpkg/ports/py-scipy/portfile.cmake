set(VCPKG_PYTHON3_BASEDIR "${CURRENT_HOST_INSTALLED_DIR}/tools/python3")
find_program(VCPKG_PYTHON3 NAMES python${PYTHON3_VERSION_MAJOR}.${PYTHON3_VERSION_MINOR} python${PYTHON3_VERSION_MAJOR} python PATHS "${VCPKG_PYTHON3_BASEDIR}" NO_DEFAULT_PATH)
find_program(VCPKG_CYTHON NAMES cython PATHS "${VCPKG_PYTHON3_BASEDIR}" "${VCPKG_PYTHON3_BASEDIR}/Scripts" NO_DEFAULT_PATH)

set(ENV{PYTHON3} "${VCPKG_PYTHON3}")
set(PYTHON3 "${VCPKG_PYTHON3}")

vcpkg_add_to_path(PREPEND "${VCPKG_PYTHON3_BASEDIR}")
if(VCPKG_TARGET_IS_WINDOWS)
  vcpkg_add_to_path(PREPEND "${VCPKG_PYTHON3_BASEDIR}/Scripts")
endif()

cmake_path(GET VCPKG_CYTHON PARENT_PATH CYTHON_DIR)
vcpkg_add_to_path("${CYTHON_DIR}")

vcpkg_from_pythonhosted(
    OUT_SOURCE_PATH SOURCE_PATH
    PACKAGE_NAME    scipy
    VERSION         ${VERSION}
    SHA512          7386670d2be598f46425a5f2ac0194748c83ec006be0b0395a850a613bc12731669469aefe503c9db2521aba956325d44e7a75fa82e4c1e629202cafd1966aa5
)

vcpkg_mesonpy_prepare_build_options(OUTPUT meson_opts)

z_vcpkg_setup_pkgconfig_path(CONFIG "RELEASE")

list(APPEND meson_opts
  "--python.platlibdir" 
  "${CURRENT_INSTALLED_DIR}/${PYTHON3_SITE}"
)

# needed so pythran can be found
vcpkg_add_to_path("${CURRENT_HOST_INSTALLED_DIR}/bin")

# haven't been able to get this to build
# correctly on MacOS with lapack
if (VCPKG_TARGET_IS_OSX)
  list(APPEND meson_opts
    "-Dblas=accelerate"
  )
else()
  list(APPEND meson_opts
    "-Dblas=blas"
    "-Dlapack=lapack"
  )
endif()

# Scipy depends on NumPy's command-line f2py utility, but
# the open-vcpkg py-numpy port doesn't install it. Here, we
# create it from scratch based on the one pip/PyPi installs.
# At present, it is a unix-like only solution.

# TODO: figure out how pip creates/installs this
if(NOT VCPKG_HOST_IS_WINDOWS)
  file(WRITE "${CURRENT_HOST_INSTALLED_DIR}/tools/tmp/f2py"
  "\
#!/bin/sh\n\
'''exec' ${PYTHON3} \"$0\" \"$@\"\n\
' '''\n\
import sys\n\
from numpy.f2py.f2py2e import main\n\
if __name__ == '__main__':\n\
    if sys.argv[0].endswith('.exe'):\n\
        sys.argv[0] = sys.argv[0][:-4]\n\
    sys.exit(main())\n\
\n\
"
  )
endif()
file(COPY "${CURRENT_HOST_INSTALLED_DIR}/tools/tmp/f2py" DESTINATION ${CURRENT_HOST_INSTALLED_DIR}/tools/numpy/bin FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ)
file(REMOVE_RECURSE "${CURRENT_HOST_INSTALLED_DIR}/tools/tmp")
vcpkg_add_to_path("${CURRENT_HOST_INSTALLED_DIR}/tools/numpy/bin")

vcpkg_configure_meson(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS ${meson_opts}
    ADDITIONAL_BINARIES
      cython=['${VCPKG_CYTHON}']
      python3=['${VCPKG_PYTHON3}']
)

vcpkg_install_meson()
vcpkg_fixup_pkgconfig()

file(GLOB licenses "${SOURCE_PATH}/LICENSE*")
vcpkg_install_copyright(FILE_LIST ${licenses})

# vcpkg_python_test_import uses z_vckpkg_python_func_python instead of PYTHON3
# and somehow, that variable is getting set to the system python instead of the 
# vcpkg-built python
set(z_vcpkg_python_func_python ${PYTHON3})
vcpkg_python_test_import(MODULE "scipy")

# Add required Metadata for some python build plugins
file(WRITE "${CURRENT_PACKAGES_DIR}/${PYTHON3_SITE}/scipy-${VERSION}.dist-info/METADATA"
"Metadata-Version: 2.1\n\
Name: scipy\n\
Version: ${VERSION}"
)

set(VCPKG_POLICY_EMPTY_INCLUDE_FOLDER enabled)
