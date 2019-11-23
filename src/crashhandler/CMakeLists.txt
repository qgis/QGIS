INCLUDE_DIRECTORIES(SYSTEM
  ${CMAKE_CURRENT_BINARY_DIR}
)

QT5_WRAP_UI(CRASH_UIS_H qgscrashdialog.ui)
QT5_WRAP_CPP(CRASH_HDR_MOC qgscrashdialog.h)

SET(IMAGE_RCCS ../../images/images.qrc)
QT5_ADD_RESOURCES(IMAGE_RCC_SRCS ${IMAGE_RCCS})

# -wd4091 Avoid 'typedef' ignored on left of '' when no variable is declared warning in dbghelp.h
SET_SOURCE_FILES_PROPERTIES(qgsstacktrace.cpp PROPERTIES COMPILE_FLAGS -wd4091)

ADD_EXECUTABLE(qgiscrashhandler WIN32
  main.cpp
  ${CRASH_UIS_H}
  ${CRASH_HDR_MOC}
  ${IMAGE_RCC_SRCS}
  qgscrashdialog.cpp
  qgsstacktrace.cpp
  qgscrashreport.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../app/qgis_win32.rc
)

TARGET_LINK_LIBRARIES(qgiscrashhandler
  ${Qt5Core_LIBRARIES}
  ${Qt5Gui_LIBRARIES}
  ${Qt5Widgets_LIBRARIES}
  dbghelp
)

INSTALL(CODE "MESSAGE(\"Installing crashhandler ...\")")
INSTALL(TARGETS qgiscrashhandler RUNTIME DESTINATION ${QGIS_LIBEXEC_DIR})
