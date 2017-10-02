FILE(GLOB PY_FILES *.py)
PLUGIN_INSTALL(ResourceSharing . ${PY_FILES} metadata.txt LICENSE.txt)

FOREACH(dir
gui
gui/ui
repository_handler
resource_handler
resources
resources/js
resources/bootstrap
resources/bootstrap/js
resources/bootstrap/fonts
resources/bootstrap/css
resources/template)
  FILE(GLOB _items ${dir}/*)
  SET(_files)
  FOREACH(_item ${_items})
    IF(NOT IS_DIRECTORY ${_item})
      LIST(APPEND _files ${_item})
    ENDIF(NOT IS_DIRECTORY ${_item})
  ENDFOREACH(_item)
  PLUGIN_INSTALL(ResourceSharing ${dir} ${_files})
ENDFOREACH(dir)

ADD_SUBDIRECTORY(test)
