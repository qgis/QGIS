# -*- coding: utf-8 -*-

"""
***************************************************************************
    QGisLayers.py
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

from qgis.core import *
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from os import path
from sextante.core.SextanteConfig import SextanteConfig
from sextante.gdal.GdalUtils import GdalUtils


class QGisLayers:
    '''This class contains method to communicate SEXTANTE with the QGIS interface,
    mostly for retrieving layers and adding new ones to the QGIS canvas'''

    ALL_TYPES = [-1]
    iface = None

    @staticmethod
    def getSupportedOutputVectorLayerExtensions():
        formats = QgsVectorFileWriter.supportedFiltersAndFormats()
        exts = ["shp"]#shp is the default, should be the first
        for extension in formats.keys():
            extension = unicode(extension)
            extension = extension[extension.find('*.') + 2:]
            extension = extension[:extension.find(" ")]
            if extension.lower() != "shp":
                exts.append(extension)
        return exts

    @staticmethod
    def getSupportedOutputRasterLayerExtensions():
        allexts = ["tif"]
        for exts in GdalUtils.getSupportedRasters().values():
            for ext in exts:
                if ext not in allexts:
                    allexts.append(ext)
        return allexts

    @staticmethod
    def getSupportedOutputTableExtensions():
        exts = ["csv"]
        return exts

    @staticmethod
    def getRasterLayers():
        layers = QGisLayers.iface.legendInterface().layers()
        raster = list()

        for layer in layers:
            if layer.type() == layer.RasterLayer:
                if layer.providerType() == 'gdal':#only gdal file-based layers
                    raster.append(layer)
        return raster

    @staticmethod
    def getVectorLayers(shapetype=[-1]):
        layers = QGisLayers.iface.legendInterface().layers()
        vector = []
        for layer in layers:
            if layer.type() == layer.VectorLayer:
                if shapetype == QGisLayers.ALL_TYPES or layer.geometryType() in shapetype:
                    uri = unicode(layer.source())
                    if not uri.lower().endswith("csv") and not uri.lower().endswith("dbf"):
                        vector.append(layer)
        return vector

    @staticmethod
    def getAllLayers():
        layers = []
        layers += QGisLayers.getRasterLayers();
        layers += QGisLayers.getVectorLayers();
        return layers

    @staticmethod
    def getTables():
        layers = QGisLayers.iface.legendInterface().layers()
        tables = list()
        for layer in layers:
            if layer.type() == layer.VectorLayer :
                tables.append(layer)
        return tables

    @staticmethod
    def setInterface(iface):
        QGisLayers.iface = iface


    @staticmethod
    def loadList(layers):
        for layer in layers:
            QGisLayers.load(layer)

    @staticmethod
    def load(fileName, name = None, crs = None, style  = None):
        if fileName == None:
            return
        prjSetting = None
        settings = QSettings()
        if crs != None:
            prjSetting = settings.value("/Projections/defaultBehaviour")
            settings.setValue("/Projections/defaultBehaviour", "")
        if name == None:
            name = path.split(fileName)[1]
        qgslayer = QgsVectorLayer(fileName, name , 'ogr')
        if qgslayer.isValid():
            if crs is not None and qgslayer.crs() is None:
                qgslayer.setCrs(crs, False)
            if style == None:
                if qgslayer.geometryType == 0:
                    style = SextanteConfig.getSetting(SextanteConfig.VECTOR_POINT_STYLE)
                elif qgslayer.geometryType == 1:
                    style = SextanteConfig.getSetting(SextanteConfig.VECTOR_LINE_STYLE)
                else:
                    style = SextanteConfig.getSetting(SextanteConfig.VECTOR_POLYGON_STYLE)
            qgslayer.loadNamedStyle(style)
            QgsMapLayerRegistry.instance().addMapLayers([qgslayer])
        else:
            qgslayer = QgsRasterLayer(fileName, name)
            if qgslayer.isValid():
                if crs != None:
                    qgslayer.setCrs(crs,False)
                if style == None:
                    style = SextanteConfig.getSetting(SextanteConfig.RASTER_STYLE)
                qgslayer.loadNamedStyle(style)
                QgsMapLayerRegistry.instance().addMapLayers([qgslayer])
                QGisLayers.iface.legendInterface().refreshLayerSymbology(qgslayer)
            else:
                if prjSetting:
                    settings.setValue("/Projections/defaultBehaviour", prjSetting)
                raise RuntimeError("Could not load layer: " + unicode(fileName)
                                       +"\nCheck the SEXTANTE log to look for errors")
        if prjSetting:
            settings.setValue("/Projections/defaultBehaviour", prjSetting)

        return qgslayer

    #===========================================================================
    # @staticmethod
    # def loadFromDict(layersdict, crs = None):
    #    for name in layersdict.keys():
    #        QGisLayers.load(layersdict[name], name, crs)
    #===========================================================================


    @staticmethod
    def getObjectFromUri(uri, forceLoad = True):
        if uri is None:
            return None
        layers = QGisLayers.getRasterLayers()
        for layer in layers:
            if layer.source() == uri:
                return layer
        layers = QGisLayers.getVectorLayers()
        for layer in layers:
            if layer.source() == uri:
                return layer
        tables = QGisLayers.getTables()
        for table in tables:
            if table.source() == uri:
                return table
        if forceLoad:
            settings = QSettings()
            prjSetting = settings.value("/Projections/defaultBehaviour")
            settings.setValue("/Projections/defaultBehaviour", "")
            #if is not opened, we open it
            layer = QgsVectorLayer(uri, uri , 'ogr')
            if layer.isValid():
                if prjSetting:
                    settings.setValue("/Projections/defaultBehaviour", prjSetting)
                return layer
            layer = QgsRasterLayer(uri, uri)
            if layer.isValid():
                if prjSetting:
                    settings.setValue("/Projections/defaultBehaviour", prjSetting)
                return layer
            if prjSetting:
                settings.setValue("/Projections/defaultBehaviour", prjSetting)
        else:
            return None

    @staticmethod
    def features(layer):
        '''this returns an iterator over features in a vector layer, considering the
        selection that might exist in the layer, and the SEXTANTE configuration that
        indicates whether to use only selected feature or all of them.
        This should be used by algorithms instead of calling the QGis API directly,
        to ensure a consistent behaviour across algorithms'''
        return Features(layer)


class Features():

    def __init__(self, layer):
        self.layer = layer
        self.iter = layer.getFeatures()
        self.selection = False;
        ##self.layer.dataProvider().rewind()
        if SextanteConfig.getSetting(SextanteConfig.USE_SELECTED):
            self.selected = layer.selectedFeatures()
            if len(self.selected) > 0:
                self.selection = True
                self.idx = 0;

    def __iter__(self):
        return self

    def next(self):
        if self.selection:
            if self.idx < len(self.selected):
                feature = self.selected[self.idx]
                self.idx += 1
                return feature
            else:
                raise StopIteration()
        else:
            if self.iter.isClosed():
                raise StopIteration()
            f = QgsFeature()
            if self.iter.nextFeature(f):
                return f
            else:
                self.iter.close()
                raise StopIteration()

    def __len__(self):
        if self.selection:
            return int(self.layer.selectedFeatureCount())
        else:
            return int(self.layer.featureCount())











