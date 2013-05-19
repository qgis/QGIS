# -*- coding: utf-8 -*-

"""
***************************************************************************
    raster.py
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

from osgeo import gdal
from osgeo.gdalconst import *
import struct

def scanraster(layer, progress):
    filename = unicode(layer.source())
    dataset = gdal.Open(filename, GA_ReadOnly)
    band = dataset.GetRasterBand(1)
    nodata = band.GetNoDataValue()
    for y in xrange(band.YSize):
        progress.setPercentage(y / float(band.YSize) * 100)
        scanline = band.ReadRaster(0, y, band.XSize, 1,band.XSize, 1, band.DataType)
        values = struct.unpack('f' * band.XSize, scanline)
        for value in values:
            if value == nodata:
                value = None
            yield value
