set (QGIS_PYQT_DIR ${Python_SITEARCH}/qgis/PyQt)

set(PYQT_COMPAT_FILES
  __init__.py
  common.py
  QtWidgets.py
  QtCore.py
  QtGui.py
  QtSerialPort.py
  QtPrintSupport.py
  QtWebKit.py
  QtWebKitWidgets.py
  QtWebEngineCore.py
  QtWebEngineQuick.py
  QtWebEngineWidgets.py
  QtNetwork.py
  QtXml.py
  QtQuickWidgets.py
  QtSql.py
  QtTest.py
  QtSvg.py
  Qsci.py
  Qt.py
  sip.py
  uic/__init__.py
  uic/pyuic.py
  uic/properties.py
)

add_custom_target(pyqtcompat ALL)
add_dependencies(pyqtcompat pyutils)

set(PYQT_PREFIX PyQt)
foreach(pyfile ${PYQT_COMPAT_FILES})
  get_filename_component(_dir ${pyfile} PATH)
  configure_file(${PYQT_PREFIX}/${pyfile}.in ${QGIS_PYTHON_OUTPUT_DIRECTORY}/PyQt/${pyfile} @ONLY)
  install(FILES ${QGIS_PYTHON_OUTPUT_DIRECTORY}/PyQt/${pyfile} DESTINATION "${QGIS_PYQT_DIR}/${_dir}")
endforeach(pyfile)

PY_COMPILE(pyqtcompat "${PYTHON_OUTPUT_DIRECTORY}")
