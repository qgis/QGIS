# -*- coding: utf-8 -*-

"""
***************************************************************************
    RasterLayerCrsAlgorithm.py
    ---------------------
    Date                 : August 2016
    Copyright            : (C) 2016 by Alexander Bruy
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
__date__ = 'August 2016'
__copyright__ = '(C) 2016, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterRaster
from processing.core.outputs import OutputCrs
from processing.tools import dataobjects


class RasterLayerCrsAlgorithm(GeoAlgorithm):

    LAYER = 'LAYER'
    CRS = 'CRS'

    def defineCharacteristics(self):
        self.showInModeler = True
        self.showInToolbox = False

        self.name = self.tr('Raster layer CRS', 'RasterLayerCrsAlgorithm')
        self.group = self.tr('Modeler-only tools', 'RasterLayerCrsAlgorithm')

        self.addParameter(ParameterRaster(self.LAYER, self.tr('Layer', 'RasterLayerCrsAlgorithm')))
        self.addOutput(OutputCrs(self.CRS, self.tr('CRS', 'RasterLayerCrsAlgorithm')))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(self.getParameterValue(self.LAYER))
        self.setOutputValue(self.CRS, layer.crs().authid())
