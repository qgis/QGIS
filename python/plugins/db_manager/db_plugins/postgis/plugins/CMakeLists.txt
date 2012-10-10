INSTALL(FILES __init__.py DESTINATION ${DB_MANAGER_POSTGIS_DIR}/plugins)

ADD_SUBDIRECTORY(qgis_topoview)
ADD_SUBDIRECTORY(versioning)

# Stage install

ADD_CUSTOM_TARGET(db_manager_postgis_plugins ALL)

FOREACH(file __init__.py)
  ADD_CUSTOM_COMMAND(TARGET db_manager_postgis_plugins
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${DB_MANAGER_POSTGIS_OUT_DIR}/plugins
    COMMAND ${CMAKE_COMMAND} -E copy ${file} ${DB_MANAGER_POSTGIS_OUT_DIR}/plugins
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )
ENDFOREACH(file)

