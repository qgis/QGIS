# FindO2.cmake - Find the O2 library using pkg-config
#
# This module defines:
# - `O2_FOUND` - Set to TRUE if the library is found.
# - `O2_INCLUDE_DIRS` - Include directories for the library.
# - `O2_LIBRARIES` - Libraries to link against.
# - The alias target `o2::o2` if the library is found.

find_package(PkgConfig REQUIRED)

pkg_search_module(PC_O2 IMPORTED_TARGET o2)

if (PC_O2_FOUND)
    set(O2_FOUND TRUE)
    set(O2_INCLUDE_DIRS ${PC_O2_INCLUDE_DIRS})
    set(O2_LIBRARIES ${PC_O2_LIBRARIES})

    add_library(o2::o2 ALIAS PkgConfig::PC_O2)
else()
    set(O2_FOUND FALSE)
    if (NOT O2_FIND_QUIETLY)
        message(WARNING "O2 library not found via pkg-config.")
    endif()
    if (O2_FIND_REQUIRED)
        message(FATAL_ERROR "O2 library is required but was not found.")
    endif()
endif()

# Mark results for cache
mark_as_advanced(O2_INCLUDE_DIRS O2_LIBRARIES)
