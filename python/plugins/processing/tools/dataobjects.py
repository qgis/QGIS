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

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from os import path
from qgis.core import *
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.utils import iface
from processing.core.ProcessingConfig import ProcessingConfig
from processing.algs.gdal.GdalUtils import GdalUtils
from processing.tools.system import *

ALL_TYPES = [-1]

_loadedLayers = {}

def resetLoadedLayers():
    global _loadedLayers
    _loadedLayers = {}

def getSupportedOutputVectorLayerExtensions():
    formats = QgsVectorFileWriter.supportedFiltersAndFormats()
    exts = ['shp']  # shp is the default, should be the first
    for extension in formats.keys():
        extension = unicode(extension)
        extension = extension[extension.find('*.') + 2:]
        extension = extension[:extension.find(' ')]
        if extension.lower() != 'shp':
            exts.append(extension)
    return exts


def getSupportedOutputRasterLayerExtensions():
    allexts = ['tif']
    for exts in GdalUtils.getSupportedRasters().values():
        for ext in exts:
            if ext not in allexts:
                allexts.append(ext)
    return allexts


def getSupportedOutputTableExtensions():
    exts = ['csv']
    return exts


def getRasterLayers(sorting=True):
    layers = QgsProject.instance().layerTreeRoot().findLayers()
    raster = []

    for layer in layers:
        mapLayer = layer.layer()
        if mapLayer.type() == QgsMapLayer.RasterLayer:
            if mapLayer.providerType() == 'gdal':  # only gdal file-based layers
                raster.append(mapLayer)
    if sorting:
        return sorted(raster,  key=lambda layer: layer.name().lower())
    else:
        return raster


def getVectorLayers(shapetype=[-1], sorting=True):
    layers = QgsProject.instance().layerTreeRoot().findLayers()
    vector = []
    for layer in layers:
        mapLayer = layer.layer()
        if mapLayer.type() == QgsMapLayer.VectorLayer:
            if (mapLayer.hasGeometryType() and
                    (shapetype == ALL_TYPES or mapLayer.geometryType() in shapetype)):
                vector.append(mapLayer)
    if sorting:
        return sorted(vector,  key=lambda layer: layer.name().lower())
    else:
        return vector


def getAllLayers():
    layers = []
    layers += getRasterLayers()
    layers += getVectorLayers()
    return sorted(layers,  key=lambda layer: layer.name().lower())


def getTables(sorting=True):
    layers = QgsProject.instance().layerTreeRoot().findLayers()
    tables = []
    for layer in layers:
        mapLayer = layer.layer()
        if mapLayer.type() == QgsMapLayer.VectorLayer:
            tables.append(mapLayer)
    if sorting:
        return sorted(tables,  key=lambda table: table.name().lower())
    else:
        return tables


def extent(layers):
    first = True
    for layer in layers:
        if not isinstance(layer, (QgsMapLayer.QgsRasterLayer, QgsMapLayer.QgsVectorLayer)):
            layer = getObjectFromUri(layer)
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
    settings = QSettings()
    if crs is not None:
        prjSetting = settings.value('/Projections/defaultBehaviour')
        settings.setValue('/Projections/defaultBehaviour', '')
    if name is None:
        name = path.split(fileName)[1]
    qgslayer = QgsVectorLayer(fileName, name, 'ogr')
    if qgslayer.isValid():
        if crs is not None and qgslayer.crs() is None:
            qgslayer.setCrs(crs, False)
        if style is None:
            if qgslayer.geometryType == 0:
                style = ProcessingConfig.getSetting(
                        ProcessingConfig.VECTOR_POINT_STYLE)
            elif qgslayer.geometryType == 1:
                style = ProcessingConfig.getSetting(
                        ProcessingConfig.VECTOR_LINE_STYLE)
            else:
                style = ProcessingConfig.getSetting(
                        ProcessingConfig.VECTOR_POLYGON_STYLE)
        qgslayer.loadNamedStyle(style)
        QgsMapLayerRegistry.instance().addMapLayers([qgslayer])
    else:
        qgslayer = QgsRasterLayer(fileName, name)
        if qgslayer.isValid():
            if crs is not None and qgslayer.crs() is None:
                qgslayer.setCrs(crs, False)
            if style is None:
                style = ProcessingConfig.getSetting(
                        ProcessingConfig.RASTER_STYLE)
            qgslayer.loadNamedStyle(style)
            QgsMapLayerRegistry.instance().addMapLayers([qgslayer])
            iface.legendInterface().refreshLayerSymbology(qgslayer)
        else:
            if prjSetting:
                settings.setValue('/Projections/defaultBehaviour', prjSetting)
            raise RuntimeError(
                    'Could not load layer: ' + unicode(fileName)
                    + '\nCheck the procesing framework log to look for errors')
    if prjSetting:
        settings.setValue('/Projections/defaultBehaviour', prjSetting)

    return qgslayer


def getObjectFromName(name):
    layers = getAllLayers()
    for layer in layers:
        if layer.name() == name:
            return layer


def getObject(uriorname):
    ret = getObjectFromName(uriorname)
    if ret is None:
        ret = getObjectFromUri(uriorname)
    return ret


def getObjectFromUri(uri, forceLoad=True):
    """Returns an object (layer/table) given a file location.

    if forceLoad is true, it tries to load it if it is not currently open
    Otherwise, it will return the object only if it is loaded in QGIS.
    """

    if uri is None:
        return None
    if uri in _loadedLayers:
        return _loadedLayers[uri]
    layers = getRasterLayers()
    for layer in layers:
        if layer.source() == uri:
            return layer
    layers = getVectorLayers()
    for layer in layers:
        if layer.source() == uri:
            return layer
    tables = getTables()
    for table in tables:
        if table.source() == uri:
            return table
    if forceLoad:
        settings = QSettings()
        prjSetting = settings.value('/Projections/defaultBehaviour')
        settings.setValue('/Projections/defaultBehaviour', '')

        # If is not opened, we open it
        layer = QgsVectorLayer(uri, uri, 'ogr')
        if layer.isValid():
            if prjSetting:
                settings.setValue('/Projections/defaultBehaviour', prjSetting)
            _loadedLayers[layer.source()] = layer
            return layer
        layer = QgsRasterLayer(uri, uri)
        if layer.isValid():
            if prjSetting:
                settings.setValue('/Projections/defaultBehaviour', prjSetting)
            _loadedLayers[layer.source()] = layer
            return layer
        if prjSetting:
            settings.setValue('/Projections/defaultBehaviour', prjSetting)
    else:
        return None


def exportVectorLayer(layer):
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

    settings = QSettings()
    systemEncoding = settings.value('/UI/encoding', 'System')

    filename = os.path.basename(unicode(layer.source()))
    idx = filename.rfind('.')
    if idx != -1:
        filename = filename[:idx]

    filename = unicode(layer.name())
    validChars = \
        'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:'
    filename = ''.join(c for c in filename if c in validChars)
    if len(filename) == 0:
        filename = 'layer'
    output = getTempFilenameInTempFolder(filename + '.shp')
    provider = layer.dataProvider()
    useSelection = ProcessingConfig.getSetting(ProcessingConfig.USE_SELECTED)
    if useSelection and layer.selectedFeatureCount() != 0:
        writer = QgsVectorFileWriter(output, systemEncoding,
                                     layer.pendingFields(),
                                     provider.geometryType(), layer.crs())
        selection = layer.selectedFeatures()
        for feat in selection:
            writer.addFeature(feat)
        del writer
        return output
    else:
        isASCII = True
        try:
            unicode(layer.source()).decode('ascii')
        except UnicodeEncodeError:
            isASCII = False
        if not unicode(layer.source()).endswith('shp') or not isASCII:
            writer = QgsVectorFileWriter(output, systemEncoding,
                    layer.pendingFields(), provider.geometryType(),
                    layer.crs())
            for feat in layer.getFeatures():
                writer.addFeature(feat)
            del writer
            return output
        else:
            return unicode(layer.source())


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
    return unicode(layer.source())


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

    settings = QSettings()
    systemEncoding = settings.value('/UI/encoding', 'System')
    output = getTempFilename()
    provider = table.dataProvider()
    isASCII = True
    try:
        unicode(table.source()).decode('ascii')
    except UnicodeEncodeError:
        isASCII = False
    isDbf = unicode(table.source()).endswith('dbf') \
        or unicode(table.source()).endswith('shp')
    if not isDbf or not isASCII:
        writer = QgsVectorFileWriter(output, systemEncoding,
                                     provider.fields(), QGis.WKBNoGeometry,
                                     QgsCoordinateReferenceSystem('4326'))
        for feat in table.getFeatures():
            writer.addFeature(feat)
        del writer
        return output + '.dbf'
    else:
        filename = unicode(table.source())
        if unicode(table.source()).endswith('shp'):
            return filename[:-3] + 'dbf'
        else:
            return filename
