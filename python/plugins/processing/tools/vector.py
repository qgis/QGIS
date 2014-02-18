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

from PyQt4.QtCore import *
from qgis.core import *
from processing.core.ProcessingConfig import ProcessingConfig


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
        pt = inGeom.asMultiPoint()
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
    fieldsA = layerA.dataProvider().fields()
    fields.extend(fieldsA)
    namesA = [unicode(f.name()).lower() for f in fieldsA]
    fieldsB = layerB.dataProvider().fields()
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
