include_directories(SYSTEM
  ${CMAKE_CURRENT_BINARY_DIR}
)

if (WITH_QT6)
  QT6_WRAP_UI(CRASH_UIS_H qgscrashdialog.ui)
  QT6_WRAP_CPP(CRASH_HDR_MOC qgscrashdialog.h)
else()
  QT5_WRAP_UI(CRASH_UIS_H qgscrashdialog.ui)
  QT5_WRAP_CPP(CRASH_HDR_MOC qgscrashdialog.h)
endif()

set(IMAGE_RCCS ../../images/images.qrc)


if(WIN32 AND NOT MINGW)
  # -wd4091 Avoid 'typedef' ignored on left of '' when no variable is declared warning in dbghelp.h
  set_source_files_properties(qgsstacktrace.cpp PROPERTIES COMPILE_FLAGS -wd4091)
endif()

add_executable(qgiscrashhandler WIN32
  main.cpp
  ${CRASH_UIS_H}
  ${CRASH_HDR_MOC}
  ${IMAGE_RCCS}
  qgscrashdialog.cpp
  qgsstacktrace.cpp
  qgscrashreport.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../app/qgis_win32.rc
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
