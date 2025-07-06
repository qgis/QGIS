include_directories(SYSTEM
  ${CMAKE_CURRENT_BINARY_DIR}
)

qt_wrap_ui(CRASH_UIS_H qgscrashdialog.ui)

set(IMAGE_RCCS ../../images/images.qrc)


if(WIN32 AND NOT MINGW)
  # -wd4091 Avoid 'typedef' ignored on left of '' when no variable is declared warning in dbghelp.h
  set_source_files_properties(qgsstacktrace.cpp PROPERTIES COMPILE_FLAGS -wd4091)
endif()

set (SRCS main.cpp)
win32_icon(SRCS)
win32_version_info("QGIS Crash Handler" "qgiscrashhandler" SRCS)
add_executable(qgiscrashhandler WIN32
  ${SRCS}
  ${CRASH_UIS_H}
  ${CRASH_HDR_MOC}
  ${IMAGE_RCCS}
  qgscrashdialog.cpp
  qgsstacktrace.cpp
  qgscrashreport.cpp
)

target_compile_features(qgiscrashhandler PRIVATE cxx_std_17)

target_link_libraries(qgiscrashhandler
  ${QT_VERSION_BASE}::Core
  ${QT_VERSION_BASE}::Gui
  ${QT_VERSION_BASE}::Widgets
)

if(WIN32 AND NOT MINGW)
  target_link_libraries(qgiscrashhandler
    dbghelp
  )
endif()

install(CODE "message(\"Installing crashhandler ...\")")
install(TARGETS qgiscrashhandler RUNTIME DESTINATION ${QGIS_LIBEXEC_DIR})
