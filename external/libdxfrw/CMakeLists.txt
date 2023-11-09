add_library(libdxfrw STATIC
    drw_base.cpp
    drw_classes.cpp
    drw_entities.cpp
    drw_header.cpp
    drw_objects.cpp
    libdwgr.cpp
    libdxfrw.cpp
    intern/drw_dbg.cpp
    intern/drw_textcodec.cpp
    intern/dwgbuffer.cpp
    intern/dwgreader.cpp
    intern/dwgreader15.cpp
    intern/dwgreader18.cpp
    intern/dwgreader21.cpp
    intern/dwgreader24.cpp
    intern/dwgreader27.cpp
    intern/dwgutil.cpp
    intern/dxfreader.cpp
    intern/dxfwriter.cpp
    intern/rscodec.cpp
)

target_include_directories(libdxfrw PUBLIC
  ${CMAKE_SOURCE_DIR}/external/libdxfrw
)

set_property(TARGET libdxfrw PROPERTY POSITION_INDEPENDENT_CODE ON)

# require c++11
target_compile_features(libdxfrw PRIVATE cxx_std_11)

if(FALSE)
if(DOXYGEN_FOUND)
  add_custom_target(doxygen-dxfrw ALL
    COMMAND ${DOXYGEN_EXECUTABLE} libdxfrw.dox
      drw_base.h
      drw_classes.cpp
      drw_classes.h
      drw_entities.cpp
      drw_entities.h
      drw_header.cpp
      drw_header.h
      drw_interface.h
      drw_objects.cpp
      drw_objects.h
      libdwgr.cpp
      libdwgr.h
      libdxfrw.cpp
      libdxfrw.h
      intern/drw_cptable932.h
      intern/drw_cptable936.h
      intern/drw_cptable949.h
      intern/drw_cptable950.h
      intern/drw_cptables.h
      intern/drw_dbg.cpp
      intern/drw_dbg.h
      intern/drw_textcodec.cpp
      intern/drw_textcodec.h
      intern/dwgbuffer.cpp
      intern/dwgbuffer.h
      intern/dwgreader.cpp
      intern/dwgreader.h
      intern/dwgreader15.cpp
      intern/dwgreader15.h
      intern/dwgreader18.cpp
      intern/dwgreader18.h
      intern/dwgreader21.cpp
      intern/dwgreader21.h
      intern/dwgreader24.cpp
      intern/dwgreader24.h
      intern/dwgreader27.cpp
      intern/dwgreader27.h
      intern/dwgutil.cpp
      intern/dwgutil.h
      intern/dxfreader.cpp
      intern/dxfreader.h
      intern/dxfwriter.cpp
      intern/dxfwriter.h
      intern/rscodec.cpp
      intern/rscodec.h
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS libdxfrw
    COMMENT "Generating libdxfrw documentation" VERBATIM)
endif()
endif()
