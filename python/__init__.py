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

from builtins import zip
import os


def setupenv():
    """
    Set the environment for Windows based on the .vars files from the
    OSGeo4W package format.
    """
    # If the prefix path is already set the we don't do any more path setup.
    if os.getenv('QGIS_PREFIX_PATH'):
        return

    # Setup the paths based on the .vars file.
    from pathlib import PurePath

    path_split = PurePath(os.path.dirname(os.path.realpath(__file__))).parts

    try:
        appname = os.environ['QGIS_ENVNAME']
    except KeyError:
        appname = path_split[-3]

    envfile = list(path_split[:-4])
    envfile.append("bin")
    envfile.append("{0}-bin.env".format(appname))
    envfile = os.path.join(*envfile)

    if not os.path.exists(envfile):
        return

    with open(envfile) as f:
        for line in f:
            linedata = line.split("=")
            name = linedata[0]
            data = linedata[1]
            os.environ[name] = data


if os.name == 'nt':
    # On windows we need to setup the paths before we can import
    # any of the QGIS modules or else it will error.
    setupenv()


from qgis.PyQt import QtCore
from qgis.core import QgsFeature, QgsGeometry


def mapping_feature(feature):
    geom = feature.geometry()
    properties = {}
    fields = [field.name() for field in feature.fields()]
    properties = dict(list(zip(fields, feature.attributes())))
    return {'type': 'Feature',
            'properties': properties,
            'geometry': geom.__geo_interface__}


def mapping_geometry(geometry):
    geo = geometry.asJson()
    # We have to use eval because exportToGeoJSON() gives us
    # back a string that looks like a dictionary.
    return eval(geo)


QgsFeature.__geo_interface__ = property(mapping_feature)
QgsGeometry.__geo_interface__ = property(mapping_geometry)
