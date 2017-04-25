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

from qgis.core import (QgsApplication)
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterRaster
from processing.core.parameters import ParameterNumber
from processing.core.outputs import OutputRaster
from processing.tools.raster import RasterWriter
from processing.tools import dataobjects


class CreateConstantRaster(GeoAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    NUMBER = 'NUMBER'

    def icon(self):
        return QgsApplication.getThemeIcon("/providerQgis.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("providerQgis.svg")

    def group(self):
        return self.tr('Raster tools')

    def name(self):
        return 'createconstantrasterlayer'

    def displayName(self):
        return self.tr('Create constant raster layer')

    def defineCharacteristics(self):
        self.addParameter(ParameterRaster(self.INPUT,
                                          self.tr('Reference layer')))
        self.addParameter(ParameterNumber(self.NUMBER,
                                          self.tr('Constant value'),
                                          default=1.0))

        self.addOutput(OutputRaster(self.OUTPUT,
                                    self.tr('Constant')))

    def processAlgorithm(self, context, feedback):
        layer = dataobjects.getLayerFromString(
            self.getParameterValue(self.INPUT))
        value = self.getParameterValue(self.NUMBER)

        output = self.getOutputFromName(self.OUTPUT)

        raster = gdal.Open(layer.source(), gdal.GA_ReadOnly)
        geoTransform = raster.GetGeoTransform()

        cellsize = (layer.extent().xMaximum() - layer.extent().xMinimum()) \
            / layer.width()

        w = RasterWriter(output.getCompatibleFileName(self),
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
