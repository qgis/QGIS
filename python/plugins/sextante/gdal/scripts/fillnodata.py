# -*- coding: utf-8 -*-

"""
***************************************************************************
    fillnodata.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
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
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

#******************************************************************************
#  Adapted from gdal_fillnodata.py, Copyright (c) 2008, Frank Warmerdam
#******************************************************************************
##src_filename=raster
##dst_filename=output raster
##[GDAL] Analysis=group
from sextante.gdal.GdalUtils import GdalUtils

from osgeo import gdal, ogr

def CopyBand( srcband, dstband ):
    for line in range(srcband.YSize):
        line_data = srcband.ReadRaster( 0, line, srcband.XSize, 1 )
        dstband.WriteRaster( 0, line, srcband.XSize, 1, line_data,
                             buf_type = srcband.DataType )

max_distance = 100
smoothing_iterations = 0
options = []
src_band = 1

gdal.AllRegister()
src_ds = gdal.Open(src_filename, gdal.GA_ReadOnly)
srcband = src_ds.GetRasterBand(src_band)
maskband = srcband.GetMaskBand()
drv = gdal.GetDriverByName(GdalUtils.getFormatShortNameFromFilename(dst_filename))
dst_ds = drv.Create(dst_filename,src_ds.RasterXSize, src_ds.RasterYSize,1,
                         srcband.DataType)
wkt = src_ds.GetProjection()
if wkt != '':
    dst_ds.SetProjection( wkt )
dst_ds.SetGeoTransform( src_ds.GetGeoTransform() )

dstband = dst_ds.GetRasterBand(1)
CopyBand( srcband, dstband )

prog_func = gdal.TermProgress

result = gdal.FillNodata( dstband, maskband ,
                          max_distance, smoothing_iterations, options,
                          callback = prog_func )


src_ds = None
dst_ds = None
mask_ds = None
