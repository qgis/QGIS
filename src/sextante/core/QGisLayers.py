from qgis.core import *
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4 import QtCore, QtGui
from os import path

class QGisLayers:

    ALL_TYPES = -1
    iface = None;

    @staticmethod
    def getRasterLayers():
        layers = QGisLayers.iface.legendInterface().layers()
        raster = list()

        for layer in layers:
            if layer.type() == layer.RasterLayer :
                raster.append(layer)
        return raster

    @staticmethod
    def getVectorLayers(shapetype=-1):
        layers = QGisLayers.iface.legendInterface().layers()
        vector = list()
        for layer in layers:
            if layer.type() == layer.VectorLayer :
                if shapetype == QGisLayers.ALL_TYPES or layer.geometryType() == shapetype:
                    vector.append(layer)
        return vector

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
        if layer == None:
            return
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
        except Exception:
            QtGui.QMessageBox(None, "Error", "Could not load layer: " + str(layer))
        finally:
            if prjSetting:
                settings.setValue("/Projections/defaultBehaviour", prjSetting)


    @staticmethod
    def loadFromDict(layersdict, crs):
        for name in layersdict.keys():
            QGisLayers.load(layersdict[name], name, crs)


    @staticmethod
    def getObjectFromUri(uri):
        layers = QGisLayers.getRasterLayers()
        for layer in layers:
            if layer.source() == uri:
                return layer
        layers = QGisLayers.getVectorLayers()
        for layer in layers:
            if layer.source() == uri:
                return layer
        #if is not opened, we open it
        layer = QgsVectorLayer(uri, uri , 'ogr')
        if layer.isValid():
            return layer
        layer = QgsRasterLayer(uri, uri)
        if layer.isValid():
            return layer









