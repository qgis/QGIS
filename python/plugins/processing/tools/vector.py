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

from processing.core.QGisLayers import QGisLayers
from qgis.core import *
from PyQt4.QtCore import *

def uniquevalues(layer, attribute):
    '''Returns a list of unique values for a given attribute.
    Attribute can be defined using a field names or a zero-based field index.
    It considers the existing selection'''
    values = []
    fieldIndex = resolveFieldIndex(layer, attribute)
    features = QGisLayers.features(layer)
    for feat in features:
        if feat.attributes()[fieldIndex] not in values:
            values.append(feat.attributes()[fieldIndex])
    return values

def resolveFieldIndex(layer, attr):
    '''This method takes an object and returns the index field it refers to in a layer.
    If the passed object is an integer, it returns the same integer value.
    If the passed value is not an integer, it returns the field whose name is the string
    representation of the passed object.
    Ir raises an exception if the int value is larger than the number of fields, or if
    the passed object does not correspond to any field'''
    if isinstance(attr, int):
        return attr
    else:
        index = layer.fieldNameIndex(unicode(attr))
        if index == -1:
            raise ValueError('Wrong field name')
        return index


def values(layer, *attributes):
    '''Returns the values in the attributes table of a vector layer, for the passed fields.
    Field can be passed as field names or as zero-based field indices.
    Returns a dict of lists, with the passed field identifiers as keys.
    It considers the existing selection.
    It assummes fields are numeric or contain values that can be parsed to a number'''
    ret = {}
    for attr in attributes:
        index = resolveFieldIndex(layer, attr)
        values = []
        features = QGisLayers.features(layer)
        for feature in features:
            try:
                v = float(feature.attributes()[index])
                values.append(v)
            except:
                values.append(None)
        ret[attr] = values;
    return ret

def spatialindex(layer):
    '''Creates a spatial index for the passed vector layer'''
    idx = QgsSpatialIndex()
    features = QGisLayers.features(layer)
    for ft in features:
        idx.insertFeature(ft)
    return idx

def getfeatures(layer):
    '''returns an iterator over the features of a vector layer, considering the existing selection'''
    return QGisLayers.features(layer)


def createUniqueFieldName(fieldName, fieldList):
    shortName = fieldName[:10]

    if len(fieldList) == 0:
        return shortName

    fieldNames = [f.name() for f in fieldList]

    if shortName not in fieldNames:
        return shortName

    shortName = fieldName[:8] + "_1"
    changed = True
    while changed:
        changed = False
        for n in fieldList:
            if n == shortName:
                # create unique field name
                num = int(shortName[-1:])
                if num < 9:
                    shortName = shortName[:8] + "_" + str(num + 1)
                else:
                    shortName = shortName[:7] + "_" + str(num + 1)

                changed = True

    return shortName

def findOrCreateField(layer, fieldList, fieldName, fieldLen=24, fieldPrec=15):
    idx = layer.fieldNameIndex(fieldName)
    if idx == -1:
        fn = createUniqueFieldName(fieldName, fieldList)
        field = QgsField(fn, QVariant.Double, "", fieldLen, fieldPrec)
        idx = len(fieldList)
        fieldList.append(field)

    return idx, fieldList

def extractPoints(geom):
    points = []
    if geom.type() ==  QGis.Point:
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
    # method defines calculation type:
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
    features = QGisLayers.features(layer)
    for feat in features:
        if feat.attributes()[fieldIndex] not in values:
            values.append(feat.attributes()[fieldIndex])
    return values

def getUniqueValuesCount(layer, fieldIndex):
    return len(getUniqueValues(layer, fieldIndex))

# From two input field maps, create single field map
def combineVectorFields(layerA, layerB):
    fields = []
    fieldsA = layerA.dataProvider().fields()
    fields.extend(fieldsA)
    namesA = [unicode(f.name()).lower() for f in fieldsA]
    fieldsB = layerB.dataProvider().fields()
    for field in fieldsB:
        name = unicode(field.name()).lower()
        if name in namesA:
            idx=2
            newName = name + "_" + unicode(idx)
            while newName in namesA:
                idx += 1
                newName = name + "_" + unicode(idx)
            field = QgsField(newName, field.type(), field.typeName())
        fields.append(field)

    return fields

