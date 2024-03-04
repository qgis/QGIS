if (NATIVE_CRSSYNC_BIN)
  add_custom_target(synccrsdb
    COMMENT "Running native crssync"
    COMMAND ${NATIVE_CRSSYNC_BIN}
  )

else ()
  add_executable(crssync main.cpp)
  target_compile_features(crssync PRIVATE cxx_std_17)

  target_link_libraries(crssync
    qgis_core
  )

  target_compile_definitions(crssync PRIVATE "CMAKE_INSTALL_PREFIX=\"${CMAKE_INSTALL_PREFIX}\"")

  if(MSVC AND NOT USING_NMAKE)
    add_custom_target(synccrsdb
      COMMENT "Running crssync"
      COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/crssync.exe
      DEPENDS crssync
    )
  else()
    add_custom_target(synccrsdb
      COMMENT "Running crssync"
      COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/crssync
      DEPENDS crssync
    )
  endif()

  install(CODE "message(\"Installing crssync ...\")")
  install(TARGETS crssync RUNTIME DESTINATION ${QGIS_LIBEXEC_DIR})

endif()
