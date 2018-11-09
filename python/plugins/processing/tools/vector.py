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

from qgis.core import (NULL,
                       QgsFeatureRequest)


def resolveFieldIndex(source, attr):
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
        index = source.fields().lookupField(attr)
        if index == -1:
            raise ValueError('Wrong field name')
        return index


def values(source, *attributes):
    """Returns the values in the attributes table of a feature source,
    for the passed fields.

    Field can be passed as field names or as zero-based field indices.
    Returns a dict of lists, with the passed field identifiers as keys.
    It considers the existing selection.

    It assummes fields are numeric or contain values that can be parsed
    to a number.
    """
    ret = {}
    indices = []
    attr_keys = {}
    for attr in attributes:
        index = resolveFieldIndex(source, attr)
        indices.append(index)
        attr_keys[index] = attr

    # use an optimised feature request
    request = QgsFeatureRequest().setSubsetOfAttributes(indices).setFlags(QgsFeatureRequest.NoGeometry)

    for feature in source.getFeatures(request):
        for i in indices:

            # convert attribute value to number
            try:
                v = float(feature[i])
            except:
                v = None

            k = attr_keys[i]
            if k in ret:
                ret[k].append(v)
            else:
                ret[k] = [v]
    return ret


def convert_nulls(values, replacement=None):
    """
    Converts NULL items in a list of values to a replacement value (usually None)
    :param values: list of values
    :param replacement: value to use in place of NULL
    :return: converted list
    """
    return [i if i != NULL else replacement for i in values]


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
