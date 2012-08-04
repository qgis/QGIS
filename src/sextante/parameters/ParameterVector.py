from sextante.parameters.ParameterDataObject import ParameterDataObject
from sextante.core.QGisLayers import QGisLayers
from qgis.core import *
from sextante.core.LayerExporter import LayerExporter

class ParameterVector(ParameterDataObject):

    VECTOR_TYPE_POINT = 0
    VECTOR_TYPE_LINE = 1
    VECTOR_TYPE_POLYGON = 2
    VECTOR_TYPE_ANY = -1

    def __init__(self, name="", description="", shapetype=-1, optional=False):
        ParameterDataObject.__init__(self, name, description)
        self.optional = optional
        self.shapetype = shapetype
        self.value = None
        self.exported = None

    def setValue(self, obj):
        self.exported = None
        if obj == None:
            if self.optional:
                self.value = None
                return True
            else:
                return False
        if isinstance(obj, QgsVectorLayer):
            self.value = unicode(obj.source())
            return True
        else:
            self.value = unicode(obj)
            layers = QGisLayers.getVectorLayers(self.shapetype)
            for layer in layers:
                if layer.name() == self.value:
                    self.value = unicode(layer.source())
                    return True
            return True

    def getSafeExportedLayer(self):
        '''Returns not the value entered by the user, but a string with a filename which
        contains the data of this layer, but saved in a standard format (currently always
        a shapefile) so that it can be opened by most external applications.
        If there is a selection and SEXTANTE is configured to use just the selection, if exports
        the layer even if it is already in a suitable format.
        Works only if the layer represented by the parameter value is currently loaded in QGIS.
        Otherwise, it will not perform any export and return the current value string.
        If the current value represents a layer in a suitable format, it does not export at all
        and returns that value.
        The layer is exported just the first time the method is called. The method can be called
        several times and it will always return the same file, performing the export only the first time.'''
        if self.exported:
            return self.exported
        layer = QGisLayers.getObjectFromUri(self.value, False)
        if layer:
            self.exported = LayerExporter.exportVectorLayer(layer)
        else:
            self.exported = self.value
        return self.exported

    def serialize(self):
        return self.__module__.split(".")[-1] + "|" + self.name + "|" + self.description +\
                        "|" + str(self.shapetype) + "|" + str(self.optional)

    def deserialize(self, s):
        tokens = s.split("|")
        return ParameterVector(tokens[0], tokens[1], int(tokens[2]), str(True) == tokens[3])

    def getAsScriptCode(self):
        return "##" + self.name + "=vector"