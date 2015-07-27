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

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Create constant raster layer')
        self.group, self.i18n_group = self.trAlgorithm('Raster tools')

        self.addParameter(ParameterRaster(self.INPUT,
            self.tr('Reference layer')))
        self.addParameter(ParameterNumber(self.NUMBER,
            self.tr('Constant value'), default=1.0))

        self.addOutput(OutputRaster(self.OUTPUT,
            self.tr('Constant')))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT))
        value = self.getOutputValue(self.NUMBER)

        output = self.getOutputFromName(self.OUTPUT)

        cellsize = (layer.extent().xMaximum() - layer.extent().xMinimum()) \
            / layer.width()

        w = RasterWriter(output.getCompatibleFileName(self),
                         layer.extent().xMinimum(),
                         layer.extent().yMinimum(),
                         layer.extent().xMaximum(),
                         layer.extent().yMaximum(),
                         cellsize,
                         1,
                         self.crs,
                         )
        w.matrix[:] = value
        w.close()
