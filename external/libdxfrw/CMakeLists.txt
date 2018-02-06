INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}/src/core
        ${CMAKE_SOURCE_DIR}/src/core/geometry

        ${CMAKE_BINARY_DIR}/src/core
        ${CMAKE_BINARY_DIR}/src/core/geometry

	${Qt5Core_INCLUDE_DIRS}
)

ADD_LIBRARY(libdxfrw STATIC
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
SET_PROPERTY(TARGET libdxfrw PROPERTY POSITION_INDEPENDENT_CODE ON)

IF(FALSE)
IF(DOXYGEN_FOUND)
  ADD_CUSTOM_TARGET(doxygen-dxfrw ALL
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
ENDIF(DOXYGEN_FOUND)
ENDIF(FALSE)
