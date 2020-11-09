# -*- coding: utf-8 -*-

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

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

import os
import re

from qgis.core import (QgsDataProvider,
                       QgsRasterLayer,
                       QgsWkbTypes,
                       QgsVectorLayer,
                       QgsProject,
                       QgsSettings,
                       QgsProcessingContext,
                       QgsFeatureRequest,
                       QgsExpressionContext,
                       QgsExpressionContextUtils,
                       QgsExpressionContextScope)
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


def createContext(feedback=None):
    """
    Creates a default processing context

    :param feedback: Optional existing QgsProcessingFeedback object, or None to use a default feedback object
    :type feedback: Optional[QgsProcessingFeedback]

    :returns: New QgsProcessingContext object
    :rtype: QgsProcessingContext
    """
    context = QgsProcessingContext()
    context.setProject(QgsProject.instance())
    context.setFeedback(feedback)

    invalid_features_method = ProcessingConfig.getSetting(ProcessingConfig.FILTER_INVALID_GEOMETRIES)
    if invalid_features_method is None:
        invalid_features_method = QgsFeatureRequest.GeometryAbortOnInvalid
    context.setInvalidGeometryCheck(invalid_features_method)

    settings = QgsSettings()
    context.setDefaultEncoding(settings.value("/Processing/encoding", "System"))

    context.setExpressionContext(createExpressionContext())

    if iface and iface.mapCanvas() and iface.mapCanvas().mapSettings().isTemporal():
        context.setCurrentTimeRange(iface.mapCanvas().mapSettings().temporalRange())

    return context


def createExpressionContext():
    context = QgsExpressionContext()
    context.appendScope(QgsExpressionContextUtils.globalScope())
    context.appendScope(QgsExpressionContextUtils.projectScope(QgsProject.instance()))

    if iface and iface.mapCanvas():
        context.appendScope(QgsExpressionContextUtils.mapSettingsScope(iface.mapCanvas().mapSettings()))

    processingScope = QgsExpressionContextScope()

    if iface and iface.mapCanvas():
        extent = iface.mapCanvas().fullExtent()
        processingScope.setVariable('fullextent_minx', extent.xMinimum())
        processingScope.setVariable('fullextent_miny', extent.yMinimum())
        processingScope.setVariable('fullextent_maxx', extent.xMaximum())
        processingScope.setVariable('fullextent_maxy', extent.yMaximum())

    context.appendScope(processingScope)
    return context


def load(fileName, name=None, crs=None, style=None, isRaster=False):
    """
    Loads a layer/table into the current project, given its file.

    .. deprecated:: 3.0
    Do not use, will be removed in QGIS 4.0
    """

    from warnings import warn
    warn("processing.load is deprecated and will be removed in QGIS 4.0", DeprecationWarning)

    if fileName is None:
        return

    if name is None:
        name = os.path.split(fileName)[1]

    if isRaster:
        options = QgsRasterLayer.LayerOptions()
        options.skipCrsValidation = True
        qgslayer = QgsRasterLayer(fileName, name, 'gdal', options)
        if qgslayer.isValid():
            if crs is not None and qgslayer.crs() is None:
                qgslayer.setCrs(crs, False)
            if style is None:
                style = ProcessingConfig.getSetting(ProcessingConfig.RASTER_STYLE)
            qgslayer.loadNamedStyle(style)
            QgsProject.instance().addMapLayers([qgslayer])
        else:
            raise RuntimeError(QCoreApplication.translate('dataobject',
                                                          'Could not load layer: {0}\nCheck the processing framework log to look for errors.').format(
                fileName))
    else:
        options = QgsVectorLayer.LayerOptions()
        options.skipCrsValidation = True
        qgslayer = QgsVectorLayer(fileName, name, 'ogr', options)
        if qgslayer.isValid():
            if crs is not None and qgslayer.crs() is None:
                qgslayer.setCrs(crs, False)
            if style is None:
                if qgslayer.geometryType() == QgsWkbTypes.PointGeometry:
                    style = ProcessingConfig.getSetting(ProcessingConfig.VECTOR_POINT_STYLE)
                elif qgslayer.geometryType() == QgsWkbTypes.LineGeometry:
                    style = ProcessingConfig.getSetting(ProcessingConfig.VECTOR_LINE_STYLE)
                else:
                    style = ProcessingConfig.getSetting(ProcessingConfig.VECTOR_POLYGON_STYLE)
            qgslayer.loadNamedStyle(style)
            QgsProject.instance().addMapLayers([qgslayer])

    return qgslayer


def getRasterSublayer(path, param):
    layer = QgsRasterLayer(path)

    try:
        # If the layer is a raster layer and has multiple sublayers, let the user chose one.
        # Based on QgisApp::askUserForGDALSublayers
        if layer and param.showSublayersDialog and layer.dataProvider().name() == "gdal" and len(layer.subLayers()) > 1:
            layers = []
            subLayerNum = 0
            # simplify raster sublayer name
            for subLayer in layer.subLayers():
                # if netcdf/hdf use all text after filename
                if bool(re.match('netcdf', subLayer, re.I)) or bool(re.match('hdf', subLayer, re.I)):
                    subLayer = subLayer.split(path)[1]
                    subLayer = subLayer[1:]
                else:
                    # remove driver name and file name
                    subLayer.replace(subLayer.split(QgsDataProvider.SUBLAYER_SEPARATOR)[0], "")
                    subLayer.replace(path, "")
                # remove any : or " left over
                if subLayer.startswith(":"):
                    subLayer = subLayer[1:]
                if subLayer.startswith("\""):
                    subLayer = subLayer[1:]
                if subLayer.endswith(":"):
                    subLayer = subLayer[:-1]
                if subLayer.endswith("\""):
                    subLayer = subLayer[:-1]

                ld = QgsSublayersDialog.LayerDefinition()
                ld.layerId = subLayerNum
                ld.layerName = subLayer
                layers.append(ld)
                subLayerNum = subLayerNum + 1

            # Use QgsSublayersDialog
            # Would be good if QgsSublayersDialog had an option to allow only one sublayer to be selected
            chooseSublayersDialog = QgsSublayersDialog(QgsSublayersDialog.Gdal, "gdal")
            chooseSublayersDialog.populateLayerTable(layers)

            if chooseSublayersDialog.exec_():
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
