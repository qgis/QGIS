# -*- coding: utf-8 -*-

"""
***************************************************************************
    vector.py
    ---------------------
    Date                 : February 2013
    Copyright            : (C) 2013 by Victor Olaya
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
__date__ = 'February 2013'
__copyright__ = '(C) 2013, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import csv
import uuid
import codecs
import cStringIO

from PyQt4.QtCore import QVariant, QSettings
from qgis.core import QGis, QgsFields, QgsField, QgsSpatialIndex, QgsMapLayerRegistry, QgsMapLayer, QgsVectorLayer, QgsVectorFileWriter, QgsDistanceArea
from processing.core.ProcessingConfig import ProcessingConfig


GEOM_TYPE_MAP = {
    QGis.WKBPoint: 'Point',
    QGis.WKBLineString: 'LineString',
    QGis.WKBPolygon: 'Polygon',
    QGis.WKBMultiPoint: 'MultiPoint',
    QGis.WKBMultiLineString: 'MultiLineString',
    QGis.WKBMultiPolygon: 'MultiPolygon',
}


TYPE_MAP = {
    str : QVariant.String,
    float: QVariant.Double,
    int: QVariant.Int,
    bool: QVariant.Bool
}


def features(layer):
    """This returns an iterator over features in a vector layer,
    considering the selection that might exist in the layer, and the
    configuration that indicates whether to use only selected feature
    or all of them.

    This should be used by algorithms instead of calling the QGis API
    directly, to ensure a consistent behaviour across algorithms.
    """
    class Features:

        def __init__(self, layer):
            self.layer = layer
            self.selection = False
            self.iter = layer.getFeatures()
            if ProcessingConfig.getSetting(ProcessingConfig.USE_SELECTED):
                selected = layer.selectedFeatures()
                if len(selected) > 0:
                    self.selection = True
                    self.iter = iter(selected)

        def __iter__(self):
            return self.iter

        def __len__(self):
            if self.selection:
                return int(self.layer.selectedFeatureCount())
            else:
                return int(self.layer.featureCount())

    return Features(layer)


def uniqueValues(layer, attribute):
    """Returns a list of unique values for a given attribute.

    Attribute can be defined using a field names or a zero-based
    field index. It considers the existing selection.
    """
    values = []
    fieldIndex = resolveFieldIndex(layer, attribute)
    feats = features(layer)
    for feat in feats:
        if feat.attributes()[fieldIndex] not in values:
            values.append(feat.attributes()[fieldIndex])
    return values


def resolveFieldIndex(layer, attr):
    """This method takes an object and returns the index field it
    refers to in a layer. If the passed object is an integer, it
    returns the same integer value. If the passed value is not an
    integer, it returns the field whose name is the string
    representation of the passed object.

    Ir raises an exception if the int value is larger than the number
    of fields, or if the passed object does not correspond to any
    field.
    """
    if isinstance(attr, int):
        return attr
    else:
        index = layer.fieldNameIndex(unicode(attr))
        if index == -1:
            raise ValueError('Wrong field name')
        return index


def values(layer, *attributes):
    """Returns the values in the attributes table of a vector layer,
    for the passed fields.

    Field can be passed as field names or as zero-based field indices.
    Returns a dict of lists, with the passed field identifiers as keys.
    It considers the existing selection.

    It assummes fields are numeric or contain values that can be parsed
    to a number.
    """
    ret = {}
    for attr in attributes:
        index = resolveFieldIndex(layer, attr)
        values = []
        feats = features(layer)
        for feature in feats:
            try:
                v = float(feature.attributes()[index])
                values.append(v)
            except:
                values.append(None)
        ret[attr] = values
    return ret


def testForUniqueness( fieldList1, fieldList2 ):
    '''Returns a modified version of fieldList2, removing naming
    collisions with fieldList1.'''
    changed = True
    while changed:
        changed = False
        for i in range(0,len(fieldList1)):
            for j in range(0,len(fieldList2)):
                if fieldList1[i].name() == fieldList2[j].name():
                    field = fieldList2[j]
                    name = createUniqueFieldName( field.name(), fieldList1 )
                    fieldList2[j] = QgsField(name, field.type(), len=field.length(), prec=field.precision(), comment=field.comment())
                    changed = True
    return fieldList2


def spatialindex(layer):
    """Creates a spatial index for the passed vector layer.
    """
    idx = QgsSpatialIndex()
    feats = features(layer)
    for ft in feats:
        idx.insertFeature(ft)
    return idx


def createUniqueFieldName(fieldName, fieldList):
    def nextname(name):
        num = 1
        while True:
            returnname ='{name}_{num}'.format(name=name[:8], num=num)
            yield returnname
            num += 1

    def found(name):
        return any(f.name() == name for f in fieldList)

    shortName = fieldName[:10]

    if not fieldList:
        return shortName

    if not found(shortName):
        return shortName

    for newname in nextname(shortName):
        if not found(newname):
            return newname


def findOrCreateField(layer, fieldList, fieldName, fieldLen=24, fieldPrec=15):
    idx = layer.fieldNameIndex(fieldName)
    if idx == -1:
        fn = createUniqueFieldName(fieldName, fieldList)
        field = QgsField(fn, QVariant.Double, '', fieldLen, fieldPrec)
        idx = len(fieldList)
        fieldList.append(field)

    return (idx, fieldList)


def extractPoints(geom):
    points = []
    if geom.type() == QGis.Point:
        if geom.isMultipart():
            points = geom.asMultiPoint()
        else:
            points.append(geom.asPoint())
    elif geom.type() == QGis.Line:
        if geom.isMultipart():
            lines = geom.asMultiPolyline()
            for line in lines:
                points.extend(line)
        else:
            points = geom.asPolyline()
    elif geom.type() == QGis.Polygon:
        if geom.isMultipart():
            polygons = geom.asMultiPolygon()
            for poly in polygons:
                for line in poly:
                    points.extend(line)
        else:
            polygon = geom.asPolygon()
            for line in polygon:
                points.extend(line)

    return points


def simpleMeasure(geom, method=0, ellips=None, crs=None):
    # Method defines calculation type:
    # 0 - layer CRS
    # 1 - project CRS
    # 2 - ellipsoidal

    if geom.wkbType() in [QGis.WKBPoint, QGis.WKBPoint25D]:
        pt = geom.asPoint()
        attr1 = pt.x()
        attr2 = pt.y()
    elif geom.wkbType() in [QGis.WKBMultiPoint, QGis.WKBMultiPoint25D]:
        pt = geom.asMultiPoint()
        attr1 = pt[0].x()
        attr2 = pt[0].y()
    else:
        measure = QgsDistanceArea()

        if method == 2:
            measure.setSourceCrs(crs)
            measure.setEllipsoid(ellips)
            measure.setEllipsoidalMode(True)

        attr1 = measure.measure(geom)
        if geom.type() == QGis.Polygon:
            attr2 = measure.measurePerimeter(geom)
        else:
            attr2 = None

    return (attr1, attr2)


def getUniqueValues(layer, fieldIndex):
    values = []
    feats = features(layer)
    for feat in feats:
        if feat.attributes()[fieldIndex] not in values:
            values.append(feat.attributes()[fieldIndex])
    return values


def getUniqueValuesCount(layer, fieldIndex):
    return len(getUniqueValues(layer, fieldIndex))


def combineVectorFields(layerA, layerB):
    """Create single field map from two input field maps.
    """
    fields = []
    fieldsA = layerA.pendingFields()
    fields.extend(fieldsA)
    namesA = [unicode(f.name()).lower() for f in fieldsA]
    fieldsB = layerB.pendingFields()
    for field in fieldsB:
        name = unicode(field.name()).lower()
        if name in namesA:
            idx = 2
            newName = name + '_' + unicode(idx)
            while newName in namesA:
                idx += 1
                newName = name + '_' + unicode(idx)
            field = QgsField(newName, field.type(), field.typeName())
        fields.append(field)

    return fields


def duplicateInMemory(layer, newName='', addToRegistry=False):
    """Return a memory copy of a layer

    layer: QgsVectorLayer that shall be copied to memory.
    new_name: The name of the copied layer.
    add_to_registry: if True, the new layer will be added to the QgsMapRegistry

    Returns an in-memory copy of a layer.
    """
    if newName is '':
        newName = layer.name() + ' (Memory)'

    if layer.type() == QgsMapLayer.VectorLayer:
        geomType = layer.geometryType()
        if geomType == QGis.Point:
            strType = 'Point'
        elif geomType == QGis.Line:
            strType = 'Line'
        elif geomType == QGis.Polygon:
            strType = 'Polygon'
        else:
            raise RuntimeError('Layer is whether Point nor Line nor Polygon')
    else:
        raise RuntimeError('Layer is not a VectorLayer')

    crs = layer.crs().authid().lower()
    myUuid = str(uuid.uuid4())
    uri = '%s?crs=%s&index=yes&uuid=%s' % (strType, crs, myUuid)
    memLayer = QgsVectorLayer(uri, newName, 'memory')
    memProvider = memLayer.dataProvider()

    provider = layer.dataProvider()
    fields = provider.fields().toList()
    memProvider.addAttributes(fields)
    memLayer.updateFields()

    for ft in provider.getFeatures():
        memProvider.addFeatures([ft])

    if addToRegistry:
        if memLayer.isValid():
            QgsMapLayerRegistry.instance().addMapLayer(memLayer)
        else:
            raise RuntimeError('Layer invalid')

    return memLayer


def checkMinDistance(point, index, distance, points):
    """Check if distance from given point to all other points is greater
    than given value.
    """
    if distance == 0:
        return True

    neighbors = index.nearestNeighbor(point, 1)
    if len(neighbors) == 0:
        return True

    if neighbors[0] in points:
        np = points[neighbors[0]]
        if np.sqrDist(point) < (distance * distance):
            return False

    return True


def _fieldName(f):
    if isinstance(f, basestring):
        return f
    return f.name()


def _toQgsField(f):
    if isinstance(f, QgsField):
        return f
    return QgsField(f[0], TYPE_MAP.get(f[1], QVariant.String))


class VectorWriter:

    MEMORY_LAYER_PREFIX = 'memory:'

    def __init__(self, fileName, encoding, fields, geometryType,
                 crs, options=None):
        self.fileName = fileName
        self.isMemory = False
        self.memLayer = None
        self.writer = None

        if encoding is None:
            settings = QSettings()
            encoding = settings.value('/Processing/encoding', 'System', type=str)

        if self.fileName.startswith(self.MEMORY_LAYER_PREFIX):
            self.isMemory = True

            uri = GEOM_TYPE_MAP[geometryType] + "?uuid=" + str(uuid.uuid4())
            if crs.isValid():
                uri += '&crs=' + crs.authid()

            fieldsdesc = ['field=' + _fieldName(f) for f in fields]
            if fieldsdesc:
              uri += '&' + '&'.join(fieldsdesc)

            self.memLayer = QgsVectorLayer(uri, self.fileName, 'memory')
            self.writer = self.memLayer.dataProvider()
        else:
            formats = QgsVectorFileWriter.supportedFiltersAndFormats()
            OGRCodes = {}
            for (key, value) in formats.items():
                extension = unicode(key)
                extension = extension[extension.find('*.') + 2:]
                extension = extension[:extension.find(' ')]
                OGRCodes[extension] = value

            extension = self.fileName[self.fileName.rfind('.') + 1:]
            if extension not in OGRCodes:
                extension = 'shp'
                self.filename = self.filename + 'shp'

            qgsfields = QgsFields()
            for field in fields:
                qgsfields.append(_toQgsField(field))

            self.writer = QgsVectorFileWriter(
                self.fileName, encoding,
                qgsfields, geometryType, crs, OGRCodes[extension])

    def addFeature(self, feature):
        if self.isMemory:
            self.writer.addFeatures([feature])
        else:
            self.writer.addFeature(feature)


class TableWriter:

    def __init__(self, fileName, encoding, fields):
        self.fileName = fileName
        if not self.fileName.lower().endswith('csv'):
            self.fileName += '.csv'

        self.encoding = encoding
        if self.encoding is None or encoding == 'System':
            self.encoding = 'utf-8'

        with open(self.fileName, 'wb') as csvFile:
            self.writer = UnicodeWriter(csvFile, encoding=self.encoding)
            if len(fields) != 0:
                self.writer.writerow(fields)

    def addRecord(self, values):
        with open(self.fileName, 'ab') as csvFile:
            self.writer = UnicodeWriter(csvFile, encoding=self.encoding)
            self.writer.writerow(values)

    def addRecords(self, records):
        with open(self.fileName, 'ab') as csvFile:
            self.writer = UnicodeWriter(csvFile, encoding=self.encoding)
            self.writer.writerows(records)


class UnicodeWriter:

    def __init__(self, f, dialect=csv.excel, encoding='utf-8', **kwds):
        self.queue = cStringIO.StringIO()
        self.writer = csv.writer(self.queue, dialect=dialect, **kwds)
        self.stream = f
        self.encoder = codecs.getincrementalencoder(encoding)()

    def writerow(self, row):
        row = map(unicode, row)
        try:
            self.writer.writerow([s.encode('utf-8') for s in row])
        except:
            self.writer.writerow(row)
        data = self.queue.getvalue()
        data = data.decode('utf-8')
        data = self.encoder.encode(data)
        self.stream.write(data)
        self.queue.truncate(0)

    def writerows(self, rows):
        for row in rows:
            self.writerow(row)
