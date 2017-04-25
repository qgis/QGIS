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
from builtins import str


__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import re

from qgis.core import (QgsVectorFileWriter,
                       QgsMapLayer,
                       QgsRasterLayer,
                       QgsWkbTypes,
                       QgsVectorLayer,
                       QgsProject,
                       QgsCoordinateReferenceSystem,
                       QgsSettings,
                       QgsProcessingUtils,
                       QgsProcessingContext)
from qgis.gui import QgsSublayersDialog
from qgis.PyQt.QtCore import QCoreApplication

from processing.core.ProcessingConfig import ProcessingConfig
from processing.algs.gdal.GdalUtils import GdalUtils
from processing.tools.system import (getTempFilenameInTempFolder,
                                     getTempFilename,
                                     removeInvalidChars,
                                     isWindows)
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException

ALL_TYPES = [-1]

TYPE_VECTOR_ANY = -1
TYPE_VECTOR_POINT = 0
TYPE_VECTOR_LINE = 1
TYPE_VECTOR_POLYGON = 2
TYPE_RASTER = 3
TYPE_FILE = 4
TYPE_TABLE = 5


def createContext():
    """
    Creates a default processing context
    """
    context = QgsProcessingContext()
    context.setProject(QgsProject.instance())

    use_selection = ProcessingConfig.getSetting(ProcessingConfig.USE_SELECTED)
    if use_selection:
        context.setFlags(QgsProcessingContext.UseSelectionIfPresent)

    invalid_features_method = ProcessingConfig.getSetting(ProcessingConfig.FILTER_INVALID_GEOMETRIES)
    context.setInvalidGeometryCheck(invalid_features_method)

    def raise_error(f):
        raise GeoAlgorithmExecutionException(QCoreApplication.translate("FeatureIterator",
                                                                        'Features with invalid geometries found. Please fix these geometries or specify the "Ignore invalid input features" flag'))

    context.setInvalidGeometryCallback(raise_error)

    return context


def getSupportedOutputRasterLayerExtensions():
    allexts = []
    for exts in list(GdalUtils.getSupportedRasters().values()):
        for ext in exts:
            if ext != 'tif' and ext not in allexts:
                allexts.append(ext)
    allexts.sort()
    allexts.insert(0, 'tif')  # tif is the default, should be the first
    return allexts


def extent(layers):
    first = True
    for layer in layers:
        if not isinstance(layer, (QgsMapLayer.QgsRasterLayer, QgsMapLayer.QgsVectorLayer)):
            layer = getLayerFromString(layer)
            if layer is None:
                continue
        if first:
            xmin = layer.extent().xMinimum()
            xmax = layer.extent().xMaximum()
            ymin = layer.extent().yMinimum()
            ymax = layer.extent().yMaximum()
        else:
            xmin = min(xmin, layer.extent().xMinimum())
            xmax = max(xmax, layer.extent().xMaximum())
            ymin = min(ymin, layer.extent().yMinimum())
            ymax = max(ymax, layer.extent().yMaximum())
        first = False
    if first:
        return '0,0,0,0'
    else:
        return str(xmin) + ',' + str(xmax) + ',' + str(ymin) + ',' + str(ymax)


def loadList(layers):
    for layer in layers:
        load(layer)


def load(fileName, name=None, crs=None, style=None):
    """Loads a layer/table into the current project, given its file.
    """

    if fileName is None:
        return
    prjSetting = None
    settings = QgsSettings()
    if crs is not None:
        prjSetting = settings.value('/Projections/defaultBehavior')
        settings.setValue('/Projections/defaultBehavior', '')
    if name is None:
        name = os.path.split(fileName)[1]
    qgslayer = QgsVectorLayer(fileName, name, 'ogr')
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
    else:
        qgslayer = QgsRasterLayer(fileName, name)
        if qgslayer.isValid():
            if crs is not None and qgslayer.crs() is None:
                qgslayer.setCrs(crs, False)
            if style is None:
                style = ProcessingConfig.getSetting(ProcessingConfig.RASTER_STYLE)
            qgslayer.loadNamedStyle(style)
            QgsProject.instance().addMapLayers([qgslayer])
        else:
            if prjSetting:
                settings.setValue('/Projections/defaultBehavior', prjSetting)
            raise RuntimeError('Could not load layer: ' + str(fileName) +
                               '\nCheck the processing framework log to look for errors')
    if prjSetting:
        settings.setValue('/Projections/defaultBehavior', prjSetting)

    return qgslayer


def getLayerFromString(string, forceLoad=True):
    """Returns an object (layer/table) given a source definition.

    if forceLoad is true, it tries to load it if it is not currently open
    Otherwise, it will return the object only if it is loaded in QGIS.
    """

    if string is None:
        return None

    # prefer project layers
    layer = QgsProcessingUtils.mapLayerFromProject(string, QgsProject.instance())
    if layer:
        return layer

    if not forceLoad:
        return None

    return QgsProcessingUtils.mapLayerFromString(string)


def exportVectorLayer(layer, supported=None):
    """Takes a QgsVectorLayer and returns the filename to refer to it,
    which allows external apps which support only file-based layers to
    use it. It performs the necessary export in case the input layer
    is not in a standard format suitable for most applications, it is
    a remote one or db-based (non-file based) one, or if there is a
    selection and it should be used, exporting just the selected
    features.

    Currently, the output is restricted to shapefiles, so anything
    that is not in a shapefile will get exported. It also export to
    a new file if the original one contains non-ascii characters.
    """

    supported = supported or ["shp"]
    settings = QgsSettings()
    systemEncoding = settings.value('/UI/encoding', 'System')

    output = getTempFilename('shp')
    basename = removeInvalidChars(os.path.basename(layer.source()))
    if basename:
        if not basename.endswith("shp"):
            basename = os.path.splitext(basename)[0] + ".shp"
        output = getTempFilenameInTempFolder(basename)
    else:
        output = getTempFilename("shp")
    useSelection = ProcessingConfig.getSetting(ProcessingConfig.USE_SELECTED)
    if useSelection and layer.selectedFeatureCount() != 0:
        writer = QgsVectorFileWriter(output, systemEncoding,
                                     layer.fields(),
                                     layer.wkbType(), layer.crs())
        selection = layer.selectedFeatures()
        for feat in selection:
            writer.addFeature(feat)
        del writer
        return output
    else:
        if not os.path.splitext(layer.source())[1].lower() in supported:
            writer = QgsVectorFileWriter(
                output, systemEncoding,
                layer.fields(), layer.wkbType(),
                layer.crs()
            )
            for feat in layer.getFeatures():
                writer.addFeature(feat)
            del writer
            return output
        else:
            return layer.source()


def exportRasterLayer(layer):
    """Takes a QgsRasterLayer and returns the filename to refer to it,
    which allows external apps which support only file-based layers to
    use it. It performs the necessary export in case the input layer
    is not in a standard format suitable for most applications, it is
    a remote one or db-based (non-file based) one.

    Currently, the output is restricted to geotiff, but not all other
    formats are exported. Only those formats not supported by GDAL are
    exported, so it is assumed that the external app uses GDAL to read
    the layer.
    """

    # TODO: Do the conversion here
    return str(layer.source())


def exportTable(table):
    """Takes a QgsVectorLayer and returns the filename to refer to its
    attributes table, which allows external apps which support only
    file-based layers to use it.

    It performs the necessary export in case the input layer is not in
    a standard format suitable for most applications, it isa remote
    one or db-based (non-file based) one.

    Currently, the output is restricted to DBF. It also export to a new
    file if the original one contains non-ascii characters.
    """

    settings = QgsSettings()
    systemEncoding = settings.value('/UI/encoding', 'System')
    output = getTempFilename()
    isASCII = True
    try:
        str(table.source()).decode('ascii')
    except UnicodeEncodeError:
        isASCII = False
    isDbf = str(table.source()).endswith('dbf') \
        or str(table.source()).endswith('shp')
    if not isDbf or not isASCII:
        writer = QgsVectorFileWriter(output, systemEncoding,
                                     table.fields(), QgsWkbTypes.NullGeometry,
                                     QgsCoordinateReferenceSystem('4326'))
        for feat in table.getFeatures():
            writer.addFeature(feat)
        del writer
        return output + '.dbf'
    else:
        filename = str(table.source())
        if str(table.source()).endswith('shp'):
            return filename[:-3] + 'dbf'
        else:
            return filename


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
                    subLayer.replace(subLayer.split(":")[0], "")
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


def vectorDataType(obj):
    types = ''
    for t in obj.datatype:
        if t == dataobjects.TYPE_VECTOR_POINT:
            types += 'point, '
        elif t == dataobjects.TYPE_VECTOR_LINE:
            types += 'line, '
        elif t == dataobjects.TYPE_VECTOR_POLYGON:
            types += 'polygon, '
        else:
            types += 'any, '

    return types[:-2]
