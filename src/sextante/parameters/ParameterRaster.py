from sextante.parameters.ParameterDataObject import ParameterDataObject
from sextante.core.QGisLayers import QGisLayers
from qgis.core import *

class ParameterRaster(ParameterDataObject):

    def __init__(self, name="", description="", optional=False):
        self.name = name
        self.description = description
        self.optional = optional
        self.value = None

    def setValue(self, obj):
        if isinstance(obj, QgsRasterLayer):
            self.value = str(obj.dataProvider().dataSourceUri())
            return True
        else:
            self.value = str(obj)
            layers = QGisLayers.getRasterLayers()
            for layer in layers:
                if layer.name() == self.value:
                    self.value = str(layer.dataProvider().dataSourceUri())
                    return True
        return True

    def serialize(self):
        return self.__module__.split(".")[-1] + "|" + self.name + "|" + self.description +\
                        "|" + str(self.optional)

    def deserialize(self, s):
        tokens = s.split("|")
        return ParameterRaster(tokens[0], tokens[1], "True" == tokens[2])

    def getAsScriptCode(self):
        return "##" + self.name + "=raster"