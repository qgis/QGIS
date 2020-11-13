
# for included scripts that set policies
install (CODE "cmake_policy(SET CMP0011 NEW)")

if(MSVC AND NOT USING_NMAKE)
  set(CRSSYNC "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/crssync.exe")
else()
  set(CRSSYNC "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/crssync")
endif()

configure_file("PostInstall.cmake.in" "PostInstall.cmake" @ONLY)
install(SCRIPT "${CMAKE_BINARY_DIR}/postinstall/PostInstall.cmake")
