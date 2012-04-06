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
        if obj == None:
            if self.optional:
                self.value = None
                return True
            else:
                return False
        if isinstance(obj, QgsVectorLayer):
            source = str(obj.source())
            if source.endswith("dbf") or source.endswith("csv"):
                self.value = source
                return True
            else:
                return False
        else:
            layers = QGisLayers.getVectorLayers()
            for layer in layers:
                if layer.name() == self.value:
                    source = str(layer.source())
                    if source.endswith("dbf") or source.endswith("csv"):
                        self.value = source
                        return True
            val = str(obj)
            if val.endswith("dbf") or val.endswith("csv"):
                self.value = val
                return True
            else:
                return False

    def serialize(self):
        return self.__module__.split(".")[-1] + "|" + self.name + "|" + self.description +\
                        "|" + str(self.optional)

    def deserialize(self, s):
        tokens = s.split("|")
        return ParameterTable(tokens[0], tokens[1], str(True) == tokens[2])

    def getAsScriptCode(self):
        return "##" + self.name + "=table"