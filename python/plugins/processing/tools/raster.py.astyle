# -*- coding: utf-8 -*-

"""
***************************************************************************
    raster.py
    ---------------------
    Date                 : February 2013
    Copyright            : (C) 2013 by Victor Olaya  and Alexander Bruy
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

__author__ = 'Victor Olaya  and Alexander Bruy'
__date__ = 'February 2013'
__copyright__ = '(C) 2013, Victor Olaya  and Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from builtins import str
from builtins import range
from builtins import object

import os
import struct

import numpy
from osgeo import gdal

from qgis.core import QgsProcessingException


def scanraster(layer, feedback, band_number=1):
    filename = str(layer.source())
    dataset = gdal.Open(filename, gdal.GA_ReadOnly)
    band = dataset.GetRasterBand(band_number)
    nodata = band.GetNoDataValue()
    bandtype = gdal.GetDataTypeName(band.DataType)
    for y in range(band.YSize):
        feedback.setProgress(y / float(band.YSize) * 100)
        scanline = band.ReadRaster(0, y, band.XSize, 1, band.XSize, 1,
                                   band.DataType)
        if bandtype == 'Byte':
            values = struct.unpack('B' * band.XSize, scanline)
        elif bandtype == 'Int16':
            values = struct.unpack('h' * band.XSize, scanline)
        elif bandtype == 'UInt16':
            values = struct.unpack('H' * band.XSize, scanline)
        elif bandtype == 'Int32':
            values = struct.unpack('i' * band.XSize, scanline)
        elif bandtype == 'UInt32':
            values = struct.unpack('I' * band.XSize, scanline)
        elif bandtype == 'Float32':
            values = struct.unpack('f' * band.XSize, scanline)
        elif bandtype == 'Float64':
            values = struct.unpack('d' * band.XSize, scanline)
        else:
            raise QgsProcessingException('Raster format not supported')
        for value in values:
            if value == nodata:
                value = None
            yield value


def mapToPixel(mX, mY, geoTransform):
    (pX, pY) = gdal.ApplyGeoTransform(
        gdal.InvGeoTransform(geoTransform), mX, mY)
    return (int(pX), int(pY))


def pixelToMap(pX, pY, geoTransform):
    return gdal.ApplyGeoTransform(geoTransform, pX + 0.5, pY + 0.5)
