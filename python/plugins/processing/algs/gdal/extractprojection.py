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

import os

from qgis.PyQt.QtGui import QIcon

from osgeo import gdal, osr

from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from qgis.core import QgsProcessingException
from qgis.core import (QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterBoolean,
                       QgsProcessingOutputFile)

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class ExtractProjection(GdalAlgorithm):
    INPUT = 'INPUT'
    PRJ_FILE_CREATE = 'PRJ_FILE_CREATE'
    WORLD_FILE = 'WORLD_FILE'
    PRJ_FILE = 'PRJ_FILE'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterRasterLayer(
            self.INPUT,
            self.tr('Input file')))
        self.addParameter(QgsProcessingParameterBoolean(
            self.PRJ_FILE_CREATE,
            self.tr('Create also .prj file'), False))
        self.addOutput(QgsProcessingOutputFile(
            self.WORLD_FILE,
            self.tr('World file')))
        self.addOutput(QgsProcessingOutputFile(
            self.PRJ_FILE,
            self.tr('ESRI Shapefile prj file')))

    def name(self):
        return 'extractprojection'

    def displayName(self):
        return self.tr('Extract projection')

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'gdaltools',
                                  'projection-export.png'))

    def group(self):
        return self.tr('Raster projections')

    def groupId(self):
        return 'rasterprojections'

    def commandName(self):
        return 'extractprojection'

    def getConsoleCommands(self, parameters, context, feedback,
                           executing=True):
        return [self.commandName()]

    def processAlgorithm(self, parameters, context, feedback):
        createPrj = self.parameterAsBoolean(parameters,
                                            self.PRJ_FILE_CREATE,
                                            context)
        raster = self.parameterAsRasterLayer(parameters, self.INPUT,
                                             context)
        if raster.dataProvider().name() != 'gdal':
            raise QgsProcessingException(self.tr('This algorithm can '
                                                 'only be used with '
                                                 'GDAL raster layers'))
        rasterPath = raster.source()
        rasterDS = gdal.Open(rasterPath, gdal.GA_ReadOnly)
        geotransform = rasterDS.GetGeoTransform()
        inputcrs = raster.crs()
        crs = inputcrs.toWkt()
        raster = None
        rasterDS = None

        outFileName = os.path.splitext(str(rasterPath))[0]

        results = {}
        if crs != '' and createPrj:
            tmp = osr.SpatialReference()
            tmp.ImportFromWkt(crs)
            tmp.MorphToESRI()
            crs = tmp.ExportToWkt()
            tmp = None

            with open(outFileName + '.prj', 'wt', encoding='utf-8') as prj:
                prj.write(crs)
            results[self.PRJ_FILE] = outFileName + '.prj'
        else:
            results[self.PRJ_FILE] = None

        with open(outFileName + '.wld', 'wt') as wld:
            wld.write('%0.8f\n' % geotransform[1])
            wld.write('%0.8f\n' % geotransform[4])
            wld.write('%0.8f\n' % geotransform[2])
            wld.write('%0.8f\n' % geotransform[5])
            wld.write('%0.8f\n' % (geotransform[0]
                                   + 0.5 * geotransform[1]
                                   + 0.5 * geotransform[2]))
            wld.write('%0.8f\n' % (geotransform[3]
                                   + 0.5 * geotransform[4]
                                   + 0.5 * geotransform[5]))
        results[self.WORLD_FILE] = outFileName + '.wld'

        return results
