# -*- coding: utf-8 -*-

"""
***************************************************************************
    FToolsUtils.py
    ---------------------
    Date                 : September 2012
    Copyright            : (C) 2012 by Carson Farmer, Victor Olaya
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
from sextante.core.QGisLayers import QGisLayers

__author__ = 'Carson, Farmer, Victor Olaya'
__date__ = 'September 2012'
__copyright__ = '(C) 2012, Carson Farmer, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from PyQt4.QtCore import *

from qgis.core import *

def createSpatialIndex(layer):
    provider = layer.provider()
    idx = QgsSpatialIndex()
    provider.rewind()
    provider.select()
    features = QGisLayers.features(layer)
    for ft in features:
        idx.insertFeature(ft)
    return idx

def createUniqueFieldName(fieldName, fieldList):
    shortName = fieldName[:10]

    if len(fieldList) == 0:
        return shortName

    if shortName not in fieldList:
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
        idx = len(fieldList)
        if idx == max(fieldList.keys()):
            idx += 1
        fn = createUniqueFieldName(fieldName, fieldList)
        field =  QgsField(fn, QVariant.Double, "", fieldLen, fieldPrec)
        fieldList[idx] = field
    return idx, fieldList

def extractPoints( geom ):
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

def getUniqueValuesCount(layer, fieldIndex):
    count = 0
    values = []
    layer.select([fieldIndex], QgsRectangle(), False)

    features = QGisLayers.features(layer)
    for feat in features:
        if feat.attributeMap()[fieldIndex].toString() not in values:
            values.append(feat.attributeMap()[fieldIndex].toString())
            count += 1
    return count
