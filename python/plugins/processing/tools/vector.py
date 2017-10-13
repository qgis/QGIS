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

from qgis.core import (QgsWkbTypes,
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
                v = float(feature.attributes()[i])
            except:
                v = None

            k = attr_keys[i]
            if k in ret:
                ret[k].append(v)
            else:
                ret[k] = [v]
    return ret


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


NOGEOMETRY_EXTENSIONS = [
    u'csv',
    u'dbf',
    u'ods',
    u'xlsx',
]


class TableWriter(object):

    def __init__(self, fileName, encoding, fields):
        self.fileName = fileName
        if not self.fileName.lower().endswith('csv'):
            self.fileName += '.csv'

        self.encoding = encoding
        if self.encoding is None or encoding == 'System':
            self.encoding = 'utf-8'

        with open(self.fileName, 'w', newline='', encoding=self.encoding) as f:
            self.writer = csv.writer(f)
            if len(fields) != 0:
                self.writer.writerow(fields)

    def addRecord(self, values):
        with open(self.fileName, 'a', newline='', encoding=self.encoding) as f:
            self.writer = csv.writer(f)
            self.writer.writerow(values)

    def addRecords(self, records):
        with open(self.fileName, 'a', newline='', encoding=self.encoding) as f:
            self.writer = csv.writer(f)
            self.writer.writerows(records)
