SET (QGIS_PYQT_DIR ${PYTHON_SITE_PACKAGES_DIR}/qgis/PyQt)
SET (PYTHON_OUTPUT_DIRECTORY ${QGIS_OUTPUT_DIRECTORY}/python/qgis/PyQt)

SET(PYQT_COMPAT_FILES
  __init__.py
  QtWidgets.py
  QtCore.py
  QtGui.py
  QtPrintSupport.py
  QtWebKit.py
  QtWebKitWidgets.py
  QtNetwork.py
  QtXml.py
  QtSql.py
  QtTest.py
  QtSvg.py
  Qsci.py
  uic/__init__.py
  uic/pyuic.py
  uic/properties.py
)

ADD_CUSTOM_TARGET(pyqtcompat ALL)

IF(ENABLE_QT5)
  SET(PYQT_PREFIX PyQt5)
ELSE(ENABLE_QT5)
  SET(PYQT_PREFIX PyQt4)
ENDIF(ENABLE_QT5)

FOREACH(pyfile ${PYQT_COMPAT_FILES})
  GET_FILENAME_COMPONENT(_dir ${pyfile} PATH)
  SET(pyfile ${PYQT_PREFIX}/${pyfile})
  ADD_CUSTOM_COMMAND(TARGET pyqtcompat
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PYTHON_OUTPUT_DIRECTORY}/${_dir}
    COMMAND ${CMAKE_COMMAND} -E copy ${pyfile} ${PYTHON_OUTPUT_DIRECTORY}/${_dir}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${pyfile}
  )
  INSTALL(FILES ${pyfile} DESTINATION "${QGIS_PYQT_DIR}/${_dir}")
ENDFOREACH(pyfile)

PY_COMPILE(pyqtcompat "${PYTHON_OUTPUT_DIRECTORY}")
