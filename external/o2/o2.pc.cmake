prefix=@CMAKE_INSTALL_PREFIX@
libdir=@CMAKE_INSTALL_PREFIX@/lib@o2_LIB_SUFFIX@
includedir=${prefix}/include/o2

Name: o2
Description: OAuth 2.0 for Qt
Version: @PROJECT_VERSION@

Cflags: -I${includedir} @CMAKE_INCLUDE_PATH@
Libs: -L${libdir} -lo2
