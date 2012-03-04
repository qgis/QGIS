from sextante.parameters.ParameterDataObject import ParameterDataObject
from sextante.core.QGisLayers import QGisLayers
from qgis.core import *

class ParameterTable(ParameterDataObject):

    def __init__(self, name="", description="", optional=False):
        self.name = name
        self.description = description
        self.optional = optional
        self.value = None

    def setValue(self, obj):
        if isinstance(obj, QgsVectorLayer):
            self.value = str(obj.source())
            if self.value.endswith("shp"):
                self.value = self.value[:-3] + "dbf"
                return True
            else:
                return False
        else:
            self.value = str(obj)
            layers = QGisLayers.getVectorLayers()
            for layer in layers:
                if layer.name() == self.value:
                    self.value = str(layer.source())
                    if self.value.endswith("shp"):
                        self.value = self.value[:-3] + "dbf"
                        return True
                    else:
                        return False
                return False

    def serialize(self):
        return self.__module__.split(".")[-1] + "|" + self.name + "|" + self.description +\
                        "|" + str(self.optional)

    def deserialize(self, s):
        tokens = s.split("|")
        return ParameterTable(tokens[0], tokens[1], str(True) == tokens[2])

    def getAsScriptCode(self):
        return "##" + self.name + "=table"