from qgis.core import *

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
    def setInterface(iface):
        QGisLayers.iface = iface


    @staticmethod
    def getLayersCount():
        count =  LayersCount()
        return count



class LayersCount:

    def __init__(self):
        self.raster = 0
        self.vector_point=0
        self.vector_line=0
        self.vector_polygon=0



