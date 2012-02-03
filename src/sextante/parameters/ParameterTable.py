from sextante.parameters.ParameterDataObject import ParameterDataObject
from sextante.core.QGisLayers import QGisLayers
from qgis.core import *

class ParameterTable(ParameterDataObject):

    def setValue(self, obj):
        if isinstance(obj, QgsVectorLayer):
            self.value = str(obj.dataProvider().dataSourceUri())
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
                    self.value = str(layer.dataProvider().dataSourceUri())
                    if self.value.endswith("shp"):
                        self.value = self.value[:-3] + "dbf"
                        return True
                    else:
                        return False
                return False


