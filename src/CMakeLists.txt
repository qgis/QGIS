add_subdirectory(core)

if (WITH_AUTH)
  add_subdirectory(auth)
endif()

if (WITH_ANALYSIS)
  add_subdirectory(analysis)
endif()

if (WITH_GUI)
  add_subdirectory(ui)
  add_subdirectory(gui)
  add_subdirectory(native)
endif()

add_subdirectory(providers)

add_subdirectory(crssync)

if (WITH_CRASH_HANDLER)
  add_subdirectory(crashhandler)
endif()

add_subdirectory(test)

if (WITH_DESKTOP)
  add_subdirectory(app)
  add_subdirectory(plugins)
endif()

if (WITH_3D)
  add_subdirectory(3d)
endif()

if (WITH_BINDINGS)
  add_subdirectory(python)
endif()

if (WITH_SERVER)
  add_subdirectory(server)
endif()

if (WITH_CUSTOM_WIDGETS)
  add_subdirectory(customwidgets)
endif()

if (WITH_QUICK)
  add_subdirectory(quickgui)
endif()

if (WITH_QGIS_PROCESS)
  add_subdirectory(process)
endif()
