set(AUTH_PKIPATHS_SRCS
  qgsauthpkipathsmethod.cpp
)
if (WITH_GUI)
  set(AUTH_PKIPATHS_SRCS ${AUTH_PKIPATHS_SRCS}
    qgsauthpkipathsmethodgui.cpp
    qgsauthpkipathsedit.cpp
  )
endif()

set(AUTH_PKIPATHS_HDRS
  qgsauthpkipathsmethod.h
)
if (WITH_GUI)
  set(AUTH_PKIPATHS_HDRS ${AUTH_PKIPATHS_HDRS}
    qgsauthpkipathsedit.h
  )
endif()

include_directories (SYSTEM
  ${QCA_INCLUDE_DIR}
  ${QTKEYCHAIN_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

if (WITH_GUI)
  set(AUTH_PKIPATHS_UIS qgsauthpkipathsedit.ui)
  QT5_WRAP_UI (AUTH_PKIPATHS_UIS_H ${AUTH_PKIPATHS_UIS})
  add_library (pkipathsauthmethod MODULE ${AUTH_PKIPATHS_SRCS} ${AUTH_PKIPATHS_HDRS} ${AUTH_PKIPATHS_UIS_H})
else()
  add_library (pkipathsauthmethod MODULE ${AUTH_PKIPATHS_SRCS} ${AUTH_PKIPATHS_HDRS})
endif()

# require c++17
target_compile_features(pkipathsauthmethod PRIVATE cxx_std_17)

target_link_libraries (pkipathsauthmethod
  qgis_core
)
if (WITH_GUI)
  target_link_libraries (pkipathsauthmethod
    qgis_gui
  )
endif()

target_compile_definitions(pkipathsauthmethod PRIVATE "-DQT_NO_FOREACH")

install(TARGETS pkipathsauthmethod
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
