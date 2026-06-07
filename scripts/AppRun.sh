#!/bin/bash
# AppRun launcher embedded into Strata AppImage.
# Sets runtime paths so QGIS finds GDAL/PROJ data, Qt plugins and Python modules
# bundled inside the AppDir.

set -e

HERE="$(dirname "$(readlink -f "${0}")")"

export GDAL_DATA="${HERE}/usr/share/gdal"
export PROJ_LIB="${HERE}/usr/share/proj"
export QT_PLUGIN_PATH="${HERE}/usr/lib/qt6/plugins:${HERE}/usr/lib/x86_64-linux-gnu/qt6/plugins:${QT_PLUGIN_PATH}"
export QML2_IMPORT_PATH="${HERE}/usr/lib/qt6/qml:${HERE}/usr/lib/x86_64-linux-gnu/qt6/qml:${QML2_IMPORT_PATH}"
export PYTHONPATH="${HERE}/usr/share/qgis/python:${HERE}/usr/share/qgis/python/plugins:${PYTHONPATH}"
export QGIS_PREFIX_PATH="${HERE}/usr"
export LD_LIBRARY_PATH="${HERE}/usr/lib:${HERE}/usr/lib/x86_64-linux-gnu:${LD_LIBRARY_PATH}"

exec "${HERE}/usr/bin/Strata" "$@"
