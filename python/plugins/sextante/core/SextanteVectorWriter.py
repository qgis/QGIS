# -*- coding: utf-8 -*-

"""
***************************************************************************
    SextanteVectorWriter.py
    ---------------------
    Date                 : September 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Victor Olaya'
__date__ = 'September 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

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

        if encoding is None:
            settings = QSettings()
            encoding = settings.value("/SextanteQGIS/encoding", "System")

        if self.fileName.startswith(self.MEMORY_LAYER_PREFIX):
            self.isMemory = True

            uri = self.TYPE_MAP[geometryType]
            if crs.isValid():
                uri += "?crs=" + crs.authid() + "&"
            fieldsdesc = ["field=" + str(f.name())  for f in fields]
            #+ ":" + str(f.typeName())
            fieldsstring = "&".join(fieldsdesc)
            uri += fieldsstring
            self.memLayer = QgsVectorLayer(uri, self.fileName, "memory")
            self.writer = self.memLayer.dataProvider()
        else:
            formats = QgsVectorFileWriter.supportedFiltersAndFormats()
            OGRCodes = {}
            for key, value in formats.items():
                extension = unicode(key)
                extension = extension[extension.find('*.') + 2:]
                extension = extension[:extension.find(" ")]
                OGRCodes[extension] = value

            extension = self.fileName[self.fileName.rfind(".") + 1:]
            if extension not in OGRCodes:
                extension = "shp"
                self.filename = self.filename + "shp"

            qgsfields = QgsFields()
            for field in fields:
                qgsfields.append(field)

            self.writer = QgsVectorFileWriter(self.fileName, encoding, qgsfields, geometryType, crs, OGRCodes[extension])

    def addFeature(self, feature):
        if self.isMemory:
            self.writer.addFeatures([feature])
        else:
            self.writer.addFeature(feature)
