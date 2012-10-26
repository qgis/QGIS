
# for included scripts that set policies
INSTALL (CODE "cmake_policy(SET CMP0011 NEW)")

CONFIGURE_FILE("PostInstall.cmake.in" "PostInstall.cmake" @ONLY)
INSTALL(SCRIPT "${CMAKE_BINARY_DIR}/postinstall/PostInstall.cmake")
