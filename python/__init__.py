# -*- coding: utf-8 -*-

"""
***************************************************************************
    __init__.py
    ---------------------
    Date                 : January 2007
    Copyright            : (C) 2007 by Martin Dobias
    Email                : wonder dot sk at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Martin Dobias'
__date__ = 'January 2007'
__copyright__ = '(C) 2007, Martin Dobias'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import sip

try:
    apis = ["QDate", "QDateTime", "QString", "QTextStream", "QTime", "QUrl", "QVariant"]
    for api in apis:
        sip.setapi(api, 2)
except ValueError:
    # API has already been set so we can't set it again.
    pass

from qgis.core import QgsFeature, QgsGeometry


def mapping_feature(feature):
    geom = feature.geometry()
    properties = {}
    fields = [field.name() for field in feature.fields()]
    properties = dict(zip(fields, feature.attributes()))
    return {'type': 'Feature',
            'properties': properties,
            'geometry': geom.__geo_interface__}


def mapping_geometry(geometry):
    geo = geometry.exportToGeoJSON()
    # We have to use eval because exportToGeoJSON() gives us
    # back a string that looks like a dictionary.
    return eval(geo)

QgsFeature.__geo_interface__ = property(mapping_feature)
QgsGeometry.__geo_interface__ = property(mapping_geometry)
