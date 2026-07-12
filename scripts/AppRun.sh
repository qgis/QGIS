#!/bin/bash
# AppRun launcher embedded into Strata AppImage.
# Sets runtime paths so QGIS finds GDAL/PROJ data, Qt plugins and Python modules
# bundled inside the AppDir.

set -e

HERE="$(dirname "$(readlink -f "${0}")")"
PYTHON_LIB_DIR="$(find "${HERE}/usr/lib" -maxdepth 1 -type d -name 'python3.*' | sort -V | tail -n 1)"
PYTHON_SITE_PACKAGES=""
if [ -n "${PYTHON_LIB_DIR}" ]; then
  PYTHON_SITE_PACKAGES="${PYTHON_LIB_DIR}/site-packages"
  export PYTHONHOME="${HERE}/usr"
  export PYTHONNOUSERSITE=1
fi

export GDAL_DATA="${HERE}/usr/share/gdal"
export PROJ_LIB="${HERE}/usr/share/proj"
export QT_PLUGIN_PATH="${HERE}/usr/lib/qt6/plugins:${HERE}/usr/lib/x86_64-linux-gnu/qt6/plugins:${QT_PLUGIN_PATH}"
export QML2_IMPORT_PATH="${HERE}/usr/lib/qt6/qml:${HERE}/usr/lib/x86_64-linux-gnu/qt6/qml:${QML2_IMPORT_PATH}"
export PYTHONPATH="${HERE}/usr/share/qgis/python:${HERE}/usr/share/qgis/python/plugins:${PYTHON_SITE_PACKAGES}:${PYTHONPATH}"
export QGIS_PREFIX_PATH="${HERE}/usr"
export LD_LIBRARY_PATH="${HERE}/usr/lib:${HERE}/usr/lib/qgis:${HERE}/usr/lib/x86_64-linux-gnu:${LD_LIBRARY_PATH}"

exec "${HERE}/usr/bin/Strata" "$@"
