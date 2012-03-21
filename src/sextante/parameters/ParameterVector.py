from sextante.parameters.ParameterDataObject import ParameterDataObject
from sextante.core.QGisLayers import QGisLayers
from qgis.core import *

class ParameterVector(ParameterDataObject):

    VECTOR_TYPE_POINT = 0
    VECTOR_TYPE_LINE = 1
    VECTOR_TYPE_POLYGON = 2
    VECTOR_TYPE_ANY = -1

    def __init__(self, name="", description="", shapetype=-1, optional=False):
        self.name = name
        self.description = description
        self.optional = optional
        self.shapetype = shapetype
        self.value = None

    def setValue(self, obj):
        if obj == None:
            if self.optional:
                self.value = None
                return True
            else:
                return False
        if isinstance(obj, QgsVectorLayer):
            self.value = str(obj.source())
            return True
        else:
            self.value = str(obj)
            layers = QGisLayers.getVectorLayers(self.shapetype)
            for layer in layers:
                if layer.name() == self.value:
                    self.value = str(layer.source())
                    return True
            return True

    def serialize(self):
        return self.__module__.split(".")[-1] + "|" + self.name + "|" + self.description +\
                        "|" + str(self.shapetype) + "|" + str(self.optional)

    def deserialize(self, s):
        tokens = s.split("|")
        return ParameterVector(tokens[0], tokens[1], int(tokens[2]), str(True) == tokens[3])

    def getAsScriptCode(self):
        return "##" + self.name + "=vector"