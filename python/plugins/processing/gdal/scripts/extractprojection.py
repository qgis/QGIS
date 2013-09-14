# -*- coding: utf-8 -*-

"""
***************************************************************************
    extractprojection.py
    ---------------------
    Date                 : September 2013
    Copyright            : (C) 2013 by Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Alexander Bruy'
__date__ = 'September 2013'
__copyright__ = '(C) 2013, Alexander Bruy'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

##Input_file=raster
##Create_prj_file=boolean False
##[GDAL] Projections=group

import os

from osgeo import gdal, osr

from processing.gdal.GdalUtils import GdalUtils


raster = gdal.Open(unicode(Input_file))

crs = raster.GetProjection()
geotransform = raster.GetGeoTransform()

raster = None

outFileName = os.path.splitext(unicode(Input_file))[0]

# create prj file requested and if projection available
if crs != "" and Create_prj_file:
  # convert CRS into ESRI format
  tmp = osr.SpatialReference()
  tmp.ImportFromWkt( crs )
  tmp.MorphToESRI()
  crs = tmp.ExportToWkt()
  tmp = None

  prj = open(outFileName + '.prj', 'wt')
  prj.write( crs )
  prj.close()

# create wld file
wld = open(outFileName + '.wld', 'wt')
wld.write("%0.8f\n" % geotransform[1])
wld.write("%0.8f\n" % geotransform[4])
wld.write("%0.8f\n" % geotransform[2])
wld.write("%0.8f\n" % geotransform[5])
wld.write("%0.8f\n" % (geotransform[0] + 0.5 * geotransform[1] + 0.5 * geotransform[2]))
wld.write("%0.8f\n" % (geotransform[3] + 0.5 * geotransform[4] + 0.5 * geotransform[5]))
wld.close()
