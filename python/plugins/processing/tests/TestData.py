# -*- coding: utf-8 -*-

"""
***************************************************************************
    TestData.py
    ---------------------
    Date                 : March 2013
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
__date__ = 'March 2013'
__copyright__ = '(C) 2013, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os.path
from processing.tools import dataobjects

dataFolder = os.path.join(os.path.dirname(__file__), 'data')


def table():
    return os.path.join(dataFolder, "table.dbf")

def points():
    return os.path.join(dataFolder, "points.shp")

def points2():
    return os.path.join(dataFolder, "points2.shp")

def raster():
    return os.path.join(dataFolder, "raster.tif")

def lines():
    return os.path.join(dataFolder, "lines.shp")

def polygons():
    return os.path.join(dataFolder, "polygons.shp")

def polygons2():
    return os.path.join(dataFolder, "polygons2.shp")

def polygonsGeoJson():
    return os.path.join(dataFolder, "polygons.geojson")

def union():
    return os.path.join(dataFolder, "union.shp")

def loadTestData():
    dataobjects.load(points(), "points");
    dataobjects.load(points2(), "points2");
    dataobjects.load(polygons(), "polygons");
    dataobjects.load(polygons2(), "polygons2");
    dataobjects.load(polygonsGeoJson(), "polygonsGeoJson");
    dataobjects.load(lines(), "lines");
    dataobjects.load(raster(), "raster");
    dataobjects.load(table(), "table");
    dataobjects.load(union(), "union");