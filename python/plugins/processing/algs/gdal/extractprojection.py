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

import os

from osgeo import gdal, osr

from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.core.parameters import ParameterRaster
from processing.core.parameters import ParameterBoolean


class ExtractProjection(GdalAlgorithm):

    INPUT = 'INPUT'
    PRJ_FILE = 'PRJ_FILE'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Extract projection')
        self.group, self.i18n_group = self.trAlgorithm('[GDAL] Projections')
        self.addParameter(ParameterRaster(self.INPUT, self.tr('Input file')))
        self.addParameter(ParameterBoolean(self.PRJ_FILE,
            self.tr('Create also .prj file'), False))

    def getConsoleCommands(self):
        return ""

    def processAlgorithm(self, progress):
        rasterPath = self.getParameterValue(self.INPUT)
        createPrj = self.getParameterValue(self.PRJ_FILE)

        raster = gdal.Open(unicode(rasterPath))
        crs = raster.GetProjection()
        geotransform = raster.GetGeoTransform()
        raster = None

        outFileName = os.path.splitext(unicode(rasterPath))[0]

        if crs != '' and createPrj:
            tmp = osr.SpatialReference()
            tmp.ImportFromWkt(crs)
            tmp.MorphToESRI()
            crs = tmp.ExportToWkt()
            tmp = None

            prj = open(outFileName + '.prj', 'wt')
            prj.write(crs)
            prj.close()

        wld = open(outFileName + '.wld', 'wt')
        wld.write('%0.8f\n' % geotransform[1])
        wld.write('%0.8f\n' % geotransform[4])
        wld.write('%0.8f\n' % geotransform[2])
        wld.write('%0.8f\n' % geotransform[5])
        wld.write('%0.8f\n' % (geotransform[0] + 0.5 * geotransform[1] + 0.5
                  * geotransform[2]))
        wld.write('%0.8f\n' % (geotransform[3] + 0.5 * geotransform[4] + 0.5
                  * geotransform[5]))
        wld.close()
