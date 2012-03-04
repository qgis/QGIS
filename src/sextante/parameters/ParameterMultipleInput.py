from sextante.parameters.ParameterDataObject import ParameterDataObject
from sextante.core.QGisLayers import QGisLayers
from qgis.core import *


class ParameterMultipleInput(ParameterDataObject):

    TYPE_VECTOR_ANY = -1
    TYPE_VECTOR_POINT = 0
    TYPE_VECTOR_LINE = 1
    TYPE_VECTOR_POLYGON = 2
    TYPE_RASTER = 3
    TYPE_TABLE = 4

    def __init__(self, name="", description="", datatype=-1, optional = False):
        self.name = name
        self.description = description
        self.datatype = datatype
        self.optional = optional
        self.value = None

    def setValue(self, obj):
        if obj == None:
            if self.optional:
                self.value = None
                return True
            else:
                return False

        if isinstance(obj, list):
            if len(obj) == 0:
                if self.optional:
                    return True
                else:
                    return False
            s = ""
            idx = 0
            for layer in obj:
                s += self.getAsString(layer)
                if idx < len(obj) - 1:
                    s+=";"
                    idx=idx+1;
            self.value = s;
            return True
        else:
            self.value = str(obj)
            return True



    def getAsString(self,value):
        if self.datatype == ParameterMultipleInput.TYPE_RASTER:
            if isinstance(value, QgsRasterLayer):
                return str(value.dataProvider().dataSourceUri())
            else:
                s = str(value)
                layers = QGisLayers.getRasterLayers()
                for layer in layers:
                    if layer.name() == s:
                        return str(layer.dataProvider().dataSourceUri())
                return s
        elif self.datatype == ParameterMultipleInput.TYPE_TABLE:
            pass
        else:
            if isinstance(value, QgsVectorLayer):
                return str(value.source())
            else:
                s = str(value)
                layers = QGisLayers.getVectorLayers(self.datatype)
                for layer in layers:
                    if layer.name() == s:
                        return str(layer.source())
                return s



    def serialize(self):
        return self.__module__.split(".")[-1] + "|" + self.name + "|" + self.description +\
                        "|" + str(self.datatype) + "|" + str(self.optional)

    def deserialize(self, s):
        tokens = s.split("|")
        return ParameterMultipleInput(tokens[0], tokens[1], float(tokens[2]), tokens[3] == str(True))

    def getAsScriptCode(self):
        if self.datatype == ParameterMultipleInput.TYPE_RASTER:
            return "##" + self.name + "=multiple raster"
        else:
            return "##" + self.name + "=multiple vector"