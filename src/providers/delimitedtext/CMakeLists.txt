set (DTEXT_SRCS
  qgsdelimitedtextfeatureiterator.cpp
  qgsdelimitedtextprovider.cpp
  qgsdelimitedtextfile.cpp
)

if (WITH_GUI)
  set(DTEXT_GUI_SRCS
    qgsdelimitedtextprovidergui.cpp
    qgsdelimitedtextsourceselect.cpp
  )

  set(DTEXT_UIS ${CMAKE_SOURCE_DIR}/src/ui/qgsdelimitedtextsourceselectbase.ui)
endif()

# static library
add_library(provider_delimitedtext_a STATIC ${DTEXT_SRCS})

target_include_directories(provider_delimitedtext_a PUBLIC
  ${CMAKE_SOURCE_DIR}/src/providers/delimitedtext
)

target_link_libraries(provider_delimitedtext_a
  qgis_core
)

# require c++17
target_compile_features(provider_delimitedtext_a PRIVATE cxx_std_17)

target_compile_definitions(provider_delimitedtext_a PRIVATE "-DQT_NO_FOREACH")

if (WITH_GUI)
  if (WITH_QT6)
    QT6_WRAP_UI(DTEXT_UIS_H ${DTEXT_UIS})
  else()
    QT5_WRAP_UI(DTEXT_UIS_H ${DTEXT_UIS})
  endif()

  add_library(provider_delimitedtext_gui_a STATIC ${DTEXT_GUI_SRCS} ${DTEXT_UIS_H})

  target_include_directories(provider_delimitedtext_gui_a PUBLIC
    ${CMAKE_BINARY_DIR}/src/providers/delimitedtext
  )

  target_link_libraries(provider_delimitedtext_gui_a
    qgis_gui
  )

  # require c++17
  target_compile_features(provider_delimitedtext_gui_a PRIVATE cxx_std_17)

  target_compile_definitions(provider_delimitedtext_gui_a PRIVATE "-DQT_NO_FOREACH")

  add_dependencies(provider_delimitedtext_gui_a ui)
endif()

if (FORCE_STATIC_LIBS)
  # for (external) mobile apps to be able to pick up provider for linking
  install (TARGETS provider_delimitedtext_a ARCHIVE DESTINATION ${QGIS_PLUGIN_DIR})
  if (WITH_GUI)
    install (TARGETS provider_delimitedtext_a ARCHIVE DESTINATION ${QGIS_PLUGIN_DIR})
  endif()
else()
  add_library(provider_delimitedtext MODULE ${DTEXT_SRCS} ${DTEXT_GUI_SRCS})
  
  # require c++17
  target_compile_features(provider_delimitedtext PRIVATE cxx_std_17)
  
  target_link_libraries(provider_delimitedtext
    qgis_core
  )

  if (WITH_GUI)
    target_link_libraries (provider_delimitedtext
      qgis_gui
    )
    add_dependencies(provider_delimitedtext ui)
  endif()

  # clang-tidy
  if(CLANG_TIDY_EXE)
    set_target_properties(
      provider_delimitedtext_a PROPERTIES
      CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
    )
    set_target_properties(
      provider_delimitedtext PROPERTIES
      CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
    )
  endif()

  install (TARGETS provider_delimitedtext
    RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
    LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

  target_compile_definitions(provider_delimitedtext PRIVATE "-DQT_NO_FOREACH")
endif()
