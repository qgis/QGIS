# (c) 2014 Copyright ownCloud GmbH
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING* file.

# - Try to find QtKeychain
# Once done this will define
#  QTKEYCHAIN_FOUND - System has QtKeychain
#  QTKEYCHAIN_INCLUDE_DIRS - The QtKeychain include directories
#  QTKEYCHAIN_LIBRARIES - The libraries needed to use QtKeychain
#  QTKEYCHAIN_DEFINITIONS - Compiler switches required for using LibXml2

find_path(QTKEYCHAIN_INCLUDE_DIR
        NAMES
        keychain.h
        PATH_SUFFIXES
        qtkeychain
        )


find_library(QTKEYCHAIN_LIBRARY
        NAMES
        qtkeychain
        libqtkeychain
        PATHS
        /usr/lib
        /usr/lib/${CMAKE_ARCH_TRIPLET}
        /usr/local/lib
        /opt/local/lib
        ${CMAKE_LIBRARY_PATH}
        ${CMAKE_INSTALL_PREFIX}/lib
        )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set QTKEYCHAIN_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(QtKeychain  DEFAULT_MSG
        QTKEYCHAIN_LIBRARY QTKEYCHAIN_INCLUDE_DIR)

mark_as_advanced(QTKEYCHAIN_INCLUDE_DIR QTKEYCHAIN_LIBRARY)
