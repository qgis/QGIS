from sextante.parameters.ParameterDataObject import ParameterDataObject
from sextante.core.QGisLayers import QGisLayers
from qgis.core import *


class ParameterMultipleInput(ParameterDataObject):

    TYPE_VECTOR_POINT = 0
    TYPE_VECTOR_LINE = 1
    TYPE_VECTOR_POLYGON = 2
    TYPE_VECTOR_ANY = 3
    TYPE_RASTER = 4
    TYPE_TABLE = 5

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
        else:
            self.value = str(obj)


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
        elif self.datatype == ParameterMultipleInput.TYPE_VECTOR_ANY:
            if isinstance(value, QgsVectorLayer):
                return str(value.dataProvider().dataSourceUri())
            else:
                s = str(value)
                layers = QGisLayers.getVectorLayers()
                for layer in layers:
                    if layer.name() == s:
                        return str(layer.dataProvider().dataSourceUri())
                return s


