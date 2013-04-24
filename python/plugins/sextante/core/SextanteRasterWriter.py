# -*- coding: utf-8 -*-

"""
***************************************************************************
    SextanteRasterWriter.py
    ---------------------
    Date                 : January 2013
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
from PyQt4 import QtGui

__author__ = 'Victor Olaya'
__date__ = 'September 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from qgis.core import *
import numpy
from PyQt4.QtCore import *
from osgeo import gdal
from osgeo import osr

class SextanteRasterWriter:

    NODATA = -99999.0

    def __init__(self, fileName, minx, miny, maxx, maxy, cellsize, nbands, crs):
        self.fileName = fileName
        self.nx = int((maxx - minx) / float(cellsize))
        self.ny = int((maxy - miny) / float(cellsize))
        self.nbands = nbands;
        self.matrix = numpy.ones(shape=(self.ny, self.nx), dtype=numpy.float32)
        self.matrix[:] = self.NODATA
        self.cellsize = cellsize
        self.crs = crs
        self.minx = minx
        self.maxy = maxy

    def setValue(self, value, x, y, band = 0):
        try:
            self.matrix[y, x] = value
        except IndexError:
            pass

    def getValue(self, x, y, band = 0):
        try:
            return self.matrix[y, x]
        except IndexError:
            return self.NODATA

    def close(self):
        format = "GTiff"
        driver = gdal.GetDriverByName( format )
        dst_ds = driver.Create(self.fileName, self.nx, self.ny, 1, gdal.GDT_Float32)
        dst_ds.SetProjection(str(self.crs.toWkt()))
        dst_ds.SetGeoTransform( [self.minx, self.cellsize, 0, self.maxy, self.cellsize, 0] )
        dst_ds.GetRasterBand(1).WriteArray(self.matrix)
        dst_ds = None
