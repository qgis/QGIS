from sextante.parameters.ParameterDataObject import ParameterDataObject
from sextante.core.QGisLayers import QGisLayers
from qgis.core import *

class ParameterRaster(ParameterDataObject):

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
