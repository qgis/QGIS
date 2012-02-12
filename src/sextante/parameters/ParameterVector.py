from sextante.parameters.ParameterDataObject import ParameterDataObject
from sextante.core.QGisLayers import QGisLayers
from qgis.core import *

class ParameterVector(ParameterDataObject):

    VECTOR_TYPE_POINT = 0
    VECTOR_TYPE_LINE = 1
    VECTOR_TYPE_POLYGON = 2
    VECTOR_TYPE_ANY = 3

    def __init__(self, name, description, shapetype, optional=False):
        self.name = name
        self.description = description
        self.optional = optional
        self.shapetype = shapetype

    def setValue(self, obj):
        if isinstance(obj, QgsVectorLayer):
            self.value = str(obj.dataProvider().dataSourceUri())
            return True
        else:
            self.value = str(obj)
            layers = QGisLayers.getVectorLayers()
            for layer in layers:
                if layer.name() == self.value:
                    self.value = str(layer.dataProvider().dataSourceUri())
                    return True

