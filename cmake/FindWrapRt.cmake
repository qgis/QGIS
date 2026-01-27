# FindWrapRt.cmake - Custom version for Emscripten cross-compilation
#
# WrapRt wraps librt (POSIX real-time library) for clock_gettime/shm_open.
# On Emscripten, these functions are provided by the runtime without linking librt.
# The Qt6 version of this file does compile checks that fail during cross-compilation,
# so we provide this override that correctly handles the Emscripten case.

# Avoid duplicate target creation
if(TARGET WrapRt::WrapRt)
    set(WrapRt_FOUND ON)
    return()
endif()

if(EMSCRIPTEN)
    # Emscripten provides clock_gettime natively in its runtime
    # No need for librt - just create the target and mark as found
    set(WrapRt_FOUND ON)
    add_library(WrapRt::WrapRt INTERFACE IMPORTED)
else()
    # For non-Emscripten builds, use the standard Qt6 FindWrapRt.cmake logic
    set(WrapRt_FOUND OFF)

    include(CheckCXXSourceCompiles)
    include(CMakePushCheckState)

    find_library(LIBRT rt)

    cmake_push_check_state()
    if(LIBRT)
        list(APPEND CMAKE_REQUIRED_LIBRARIES "${LIBRT}")
    endif()

    check_cxx_source_compiles("
    #include <time.h>
    #include <unistd.h>

    int main(int, char **) {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        return 0;
    }
    " HAVE_GETTIME)

    check_cxx_source_compiles("
    #include <sys/types.h>
    #include <sys/mman.h>
    #include <fcntl.h>

    int main(int, char **) {
        shm_open(\"test\", O_RDWR | O_CREAT | O_EXCL, 0666);
        shm_unlink(\"test\");
        return 0;
    }
    " HAVE_SHM_OPEN_SHM_UNLINK)

    cmake_pop_check_state()

    if(HAVE_GETTIME OR HAVE_SHM_OPEN_SHM_UNLINK)
        set(WrapRt_FOUND ON)
        add_library(WrapRt::WrapRt INTERFACE IMPORTED)
        if(LIBRT)
            target_link_libraries(WrapRt::WrapRt INTERFACE "${LIBRT}")
        endif()
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(WrapRt DEFAULT_MSG WrapRt_FOUND)
