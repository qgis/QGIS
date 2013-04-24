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

from sextante.core.QGisLayers import QGisLayers
from qgis.core import *

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
    It considers the existing selection'''
    ret = {}
    for attr in attributes:
        index = resolveFieldIndex(layer, attr)
        values = []
        features = QGisLayers.features(layer)
        for feature in features:
            try:
                v = float(feature.attributes()[index].toString())
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