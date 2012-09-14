from PyQt4.QtCore import *

from qgis.core import *

class SextanteVectorWriter:

    MEMORY_LAYER_PREFIX = "memory:"

    TYPE_MAP = {QGis.WKBPoint : "Point",
                QGis.WKBLineString : "LineString",
                QGis.WKBPolygon : "Polygon",
                QGis.WKBMultiPoint : "MultiPoint",
                QGis.WKBMultiLineString : "MultiLineString",
                QGis.WKBMultiPolygon : "MultiPolygon"
               }

    def __init__(self, fileName, encoding, fields, geometryType, crs, options=None):
        self.fileName = fileName
        self.isMemory = False
        self.memLayer = None
        self.writer = None

        if self.fileName.startswith(self.MEMORY_LAYER_PREFIX):
            self.isMemory = True

            uri = self.TYPE_MAP[geometryType]
            if crs.isValid():
              uri += "?crs=" + crs.authid()
            self.memLayer = QgsVectorLayer(uri, self.fileName, "memory")
            self.writer = self.memLayer.dataProvider()
            self.writer.addAttributes(fields.values())
            self.memLayer.updateFieldMap()
        else:
            formats = QgsVectorFileWriter.supportedFiltersAndFormats()
            OGRCodes = {}
            for key, value in formats.items():
                extension = str(key)
                extension = extension[extension.find('*.') + 2:]
                extension = extension[:extension.find(" ")]
                OGRCodes[extension] = value

            extension = self.fileName[self.fileName.find(".") + 1:]
            self.writer = QgsVectorFileWriter(self.fileName, encoding, fields, geometryType, crs, OGRCodes[extension])

    def addFeature(self, feature):
        if self.isMemory:
            self.writer.addFeatures([feature])
        else:
            self.writer.addFeature(feature)
