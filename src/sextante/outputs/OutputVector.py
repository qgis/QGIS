from sextante.outputs.Output import Output
from qgis.core import *
from PyQt4.QtCore import *
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException

class OutputVector(Output):

    MEMORY_LAYER_PREFIX = "memory:"

    def getFileFilter(self,alg):
        exts = alg.provider.getSupportedOutputVectorLayerExtensions()
        for i in range(len(exts)):
            exts[i] = exts[i].upper() + " files(*." + exts[i].lower() + ")"
        return ";;".join(exts)

    def getDefaultFileExtension(self, alg):
        return alg.provider.getSupportedOutputVectorLayerExtensions()[0]

    def getVectorWriter(self, fields, geomType, crs, options=None):
        '''Returns a suitable writer to which features can be added as a result of the algorithm.
        Use this to transparently handle output values instead of creating your own method.
        Parameters:
        -field: an array with the fields of the attributes table or dict of int-QgsField
        -geomType: A suitable geometry type, as it would be passed to a QgsVectorFileWriter constructor
        -crs: the crs of the layer to create.
        Executing this method might modify the object, adding additional information to it, so the writer
        can be later accessed and processed within QGIS.
        It should be called just once, since a new call might result in previous data being replaced,
        thus rendering a previously obtained writer useless'''

        if self.value.startswith(self.MEMORY_LAYER_PREFIX):
            if isinstance(fields, dict):
                fields = fields.values()
            types = { QGis.WKBPoint : "Point", QGis.WKBLineString : "Point", QGis.WKBPolygon : "Polygon",
                     QGis.WKBMultiPoint : "MultiPoint", QGis.WKBMultiLineString : "MultiLineString", QGis.WKBMultiPolygon : "MultiPolygon",}
            v = QgsVectorLayer(types[geomType], self.description, "memory")
            pr = v.dataProvider()
            pr.addAttributes(fields)
            v.startEditing()
            self.memoryLayer = v #keep a reference to the writer
            return v
        else: #outputChannel is a file path
            #TODO: Add support for encodings
            check = QFile(self.value)
            if check.exists():
                if not QgsVectorFileWriter.deleteShapeFile(self.value):
                    raise GeoAlgorithmExecutionException("Could not delete existing output file")
            formats = QgsVectorFileWriter.supportedFiltersAndFormats()
            OGRCodes = {}
            for key,value in formats.items():
                extension = str(key)
                extension = extension[extension.find('*.') + 2:]
                extension = extension[:extension.find(" ")]
                OGRCodes[extension] = value
            if isinstance(fields, dict):
                fieldsDict = fields
            else:
                fieldsDict = {}
                i = 0
                for field in fields:
                    fieldsDict[i] = field
                    i += 1
            settings = QSettings()
            systemEncoding = settings.value( "/UI/encoding", "System" ).toString()
            extension = self.value[self.value.find(".")+1:]
            return QgsVectorFileWriter(self.value, systemEncoding,  fieldsDict, geomType, crs, OGRCodes[extension] )
