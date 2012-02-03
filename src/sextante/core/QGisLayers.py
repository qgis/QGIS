from qgis.core import *
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4 import QtCore, QtGui
from os import path

class QGisLayers:

    iface = None;

    @staticmethod
    def getRasterLayers():
        layers = QGisLayers.iface.legendInterface().layers()
        layerNames = list()

        for layer in layers:
            if layer.type() == layer.RasterLayer :
                layerNames.append(layer)
        return layerNames

    @staticmethod
    def getVectorLayers():
        layers = QGisLayers.iface.legendInterface().layers()
        layerNames = list()
        for layer in layers:
            if layer.type() == layer.VectorLayer :
                layerNames.append(layer)
        return layerNames

    @staticmethod
    def getTables():
        layers = QGisLayers.iface.legendInterface().layers()
        tables = list()
        for layer in layers:
            if layer.type() == layer.VectorLayer :
                uri = str(layer.dataProvider().dataSourceUri())
                if (uri.endswith("shp")):
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
    def load(layer, name = None, crs = None):
        prjSetting = None
        settings = QSettings()
        try:
            if crs != None:
                prjSetting = settings.value("/Projections/defaultBehaviour")
                settings.setValue("/Projections/defaultBehaviour", QVariant(""))
            if name == None:
                name = path.split(layer)[1]
            if layer.endswith("shp"):
                qgslayer = QgsVectorLayer(layer, name, 'ogr')
                if crs != None:
                    qgslayer.setCrs(crs, False)
                QgsMapLayerRegistry.instance().addMapLayer(qgslayer)
            else:
                qgslayer = QgsRasterLayer(layer, name)
                if crs != None:
                    qgslayer.setCrs(crs,False)
                QgsMapLayerRegistry.instance().addMapLayer(qgslayer)
        finally:
            if prjSetting:
                settings.setValue("/Projections/defaultBehaviour", prjSetting)


    @staticmethod
    def loadMap(layersmap, crs):
        for name in layersmap.keys():
            QGisLayers.load(layersmap[name], name, crs)


    @staticmethod
    def loadFromAlg(alg):
        QGisLayers.loadMap(alg.getOuputsChannelsAsMap(), alg.crs)


    @staticmethod
    def getObjectFromUri(uri):
        layers = QGisLayers.getRasterLayers()
        for layer in layers:
            if layer.dataProvider().dataSourceUri() == uri:
                return layer
        layers = QGisLayers.getVectorLayers()
        for layer in layers:
            if layer.dataProvider().dataSourceUri() == uri:
                return layer









