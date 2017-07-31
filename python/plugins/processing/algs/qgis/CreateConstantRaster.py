# -*- coding: utf-8 -*-

"""
***************************************************************************
    CreateConstantRaster.py
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

from osgeo import gdal

from qgis.core import (QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterRasterDestination)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.tools.raster import RasterWriter


class CreateConstantRaster(QgisAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    NUMBER = 'NUMBER'

    def group(self):
        return self.tr('Raster tools')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterRasterLayer(self.INPUT,
                                                            self.tr('Reference layer')))
        self.addParameter(QgsProcessingParameterNumber(self.NUMBER,
                                                       self.tr('Constant value'), QgsProcessingParameterNumber.Double,
                                                       defaultValue=1))
        self.addParameter(QgsProcessingParameterRasterDestination(self.OUTPUT, self.tr('Constant')))

    def name(self):
        return 'createconstantrasterlayer'

    def displayName(self):
        return self.tr('Create constant raster layer')

    def processAlgorithm(self, parameters, context, feedback):
        layer = self.parameterAsRasterLayer(parameters, self.INPUT, context)
        value = self.parameterAsDouble(parameters, self.NUMBER, context)

        outputFile = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)

        raster = gdal.Open(layer.source(), gdal.GA_ReadOnly)
        geoTransform = raster.GetGeoTransform()

        cellsize = (layer.extent().xMaximum() - layer.extent().xMinimum()) \
            / layer.width()

        w = RasterWriter(outputFile,
                         layer.extent().xMinimum(),
                         layer.extent().yMinimum(),
                         layer.extent().xMaximum(),
                         layer.extent().yMaximum(),
                         cellsize,
                         1,
                         layer.crs(),
                         geoTransform
                         )
        w.matrix.fill(value)
        w.close()

        return {self.OUTPUT: outputFile}
