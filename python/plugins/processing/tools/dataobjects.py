"""
***************************************************************************
    dataobject.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Victor Olaya"
__date__ = "August 2012"
__copyright__ = "(C) 2012, Victor Olaya"

import os
import re

from qgis.core import (
    QgsDataProvider,
    QgsExpressionContext,
    QgsExpressionContextScope,
    QgsExpressionContextUtils,
    QgsFeatureRequest,
    QgsProcessingContext,
    QgsProcessingUtils,
    QgsProject,
    QgsRasterLayer,
    QgsSettings,
    QgsVectorLayer,
    QgsWkbTypes,
)
from qgis.gui import QgsSublayersDialog
from qgis.PyQt.QtCore import QCoreApplication
from qgis.utils import iface

from processing.core.ProcessingConfig import ProcessingConfig

ALL_TYPES = [-1]

TYPE_VECTOR_ANY = -1
TYPE_VECTOR_POINT = 0
TYPE_VECTOR_LINE = 1
TYPE_VECTOR_POLYGON = 2
TYPE_RASTER = 3
TYPE_FILE = 4
TYPE_TABLE = 5


# changing this signature? make sure you update the signature in
# python/processing/__init__.py too!
# Docstring for this function is in python/processing/__init__.py
def createContext(feedback=None):
    context = QgsProcessingContext()
    context.setProject(QgsProject.instance())
    context.setFeedback(feedback)

    invalid_features_method = ProcessingConfig.getSetting(
        ProcessingConfig.FILTER_INVALID_GEOMETRIES
    )
    if invalid_features_method is None:
        invalid_features_method = (
            QgsFeatureRequest.InvalidGeometryCheck.GeometryAbortOnInvalid
        )
    else:
        invalid_features_method = QgsFeatureRequest.InvalidGeometryCheck(
            int(invalid_features_method)
        )
    context.setInvalidGeometryCheck(invalid_features_method)

    settings = QgsSettings()
    context.setDefaultEncoding(
        QgsProcessingUtils.resolveDefaultEncoding(
            settings.value("/Processing/encoding")
        )
    )

    context.setExpressionContext(createExpressionContext())

    if iface and iface.mapCanvas() and iface.mapCanvas().mapSettings().isTemporal():
        context.setCurrentTimeRange(iface.mapCanvas().mapSettings().temporalRange())

    return context


def createExpressionContext():
    context = QgsExpressionContext()
    context.appendScope(QgsExpressionContextUtils.globalScope())
    context.appendScope(QgsExpressionContextUtils.projectScope(QgsProject.instance()))

    if iface and iface.mapCanvas():
        context.appendScope(
            QgsExpressionContextUtils.mapSettingsScope(iface.mapCanvas().mapSettings())
        )

    processingScope = QgsExpressionContextScope()

    if iface and iface.mapCanvas():
        extent = iface.mapCanvas().fullExtent()
        processingScope.setVariable("fullextent_minx", extent.xMinimum())
        processingScope.setVariable("fullextent_miny", extent.yMinimum())
        processingScope.setVariable("fullextent_maxx", extent.xMaximum())
        processingScope.setVariable("fullextent_maxy", extent.yMaximum())

    context.appendScope(processingScope)
    return context


def getRasterSublayer(path, param):
    layer = QgsRasterLayer(path)

    try:
        # If the layer is a raster layer and has multiple sublayers, let the user chose one.
        # Based on QgisApp::askUserForGDALSublayers
        if (
            layer
            and param.showSublayersDialog
            and layer.dataProvider().name() == "gdal"
            and len(layer.subLayers()) > 1
        ):
            layers = []
            subLayerNum = 0
            # simplify raster sublayer name
            for subLayer in layer.subLayers():
                # if netcdf/hdf use all text after filename
                if bool(re.match("netcdf", subLayer, re.I)) or bool(
                    re.match("hdf", subLayer, re.I)
                ):
                    subLayer = subLayer.split(path)[1]
                    subLayer = subLayer[1:]
                else:
                    # remove driver name and file name
                    subLayer.replace(
                        subLayer.split(QgsDataProvider.SUBLAYER_SEPARATOR)[0], ""
                    )
                    subLayer.replace(path, "")
                # remove any : or " left over
                if subLayer.startswith(":"):
                    subLayer = subLayer[1:]
                if subLayer.startswith('"'):
                    subLayer = subLayer[1:]
                if subLayer.endswith(":"):
                    subLayer = subLayer[:-1]
                if subLayer.endswith('"'):
                    subLayer = subLayer[:-1]

                ld = QgsSublayersDialog.LayerDefinition()
                ld.layerId = subLayerNum
                ld.layerName = subLayer
                layers.append(ld)
                subLayerNum = subLayerNum + 1

            # Use QgsSublayersDialog
            # Would be good if QgsSublayersDialog had an option to allow only one sublayer to be selected
            chooseSublayersDialog = QgsSublayersDialog(
                QgsSublayersDialog.ProviderType.Gdal, "gdal"
            )
            chooseSublayersDialog.populateLayerTable(layers)

            if chooseSublayersDialog.exec():
                return layer.subLayers()[chooseSublayersDialog.selectionIndexes()[0]]
            else:
                # If user pressed cancel then just return the input path
                return path
        else:
            # If the sublayers selection dialog is not to be shown then just return the input path
            return path
    except:
        # If the layer is not a raster layer, then just return the input path
        return path
