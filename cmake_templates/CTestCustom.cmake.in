#
# Note that the ITK/CMakeLists.txt file configures this file
#
#               CMake/CTestCustom.cmake.in
#
# to this file
#
#       ${ITK_BINARY_DIR}/CTestCustom.cmake
#
#----------------------------------------------------------------------
#
# For further details regarding this file,
# see http://www.cmake.org/Wiki/CMake_Testing_With_CTest#Customizing_CTest
#
# and
# http://www.kitware.com/blog/home/post/27
#
#----------------------------------------------------------------------

SET(CTEST_CUSTOM_MAXIMUM_PASSED_TEST_OUTPUT_SIZE 1000000)
SET(CTEST_CUSTOM_MAXIMUM_NUMBER_OF_WARNINGS 300)
SET(CTEST_CUSTOM_MAXIMUM_NUMBER_OF_ERRORS 50)


SET(CTEST_CUSTOM_COVERAGE_EXCLUDE
 ${CTEST_CUSTOM_COVERAGE_EXCLUDE}

 # Exclude try_compile sources from coverage results:
 "/CMakeFiles/CMakeTmp/"

 # Exclude files from the Examples directories
 #".*/Examples/.*"

 # Exclude files from the ThirdParty Utilities directories
 ".*/Testing/Utilities/.*"
 ".*/Utilities/.*"
 
 # Exclude SWIG wrappers files
 ".*/Code/Wrappers/SWIG/otbApplicationPYTHON_wrap.*"
 ".*/Code/Wrappers/SWIG/otbApplicationJAVA_wrap.*"
 )

# warning to ignore via regex expressions
SET(CTEST_CUSTOM_WARNING_EXCEPTION
  ${CTEST_CUSTOM_WARNING_EXCEPTION}
  "vcl_deprecated_header"
  "backward_warning"
  "Utilities"
  "warning LNK4221"
  "ranlib:.*file:.*has no symbols"
  "ranlib: file: .+ has no symbols"
  "ranlib:.*warning for library:.*the table of contents is empty.*no object file members in the library define global symbols.*"
  "libtool:.*file:.*has no symbols"
  "Fl_Image.H:.*warning:.*dereferencing type-punned pointer will break strict-aliasing rules.*"
  "warning -.: directory name .* does not exist"
  "ld.*warning.*duplicate dylib.*"
  "pyconfig.h:.*warning:.*XOPEN.*SOURCE.*redefined"
  )
