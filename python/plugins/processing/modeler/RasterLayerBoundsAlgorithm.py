# -*- coding: utf-8 -*-

"""
***************************************************************************
    RasterLayerBoundsAlgorithm.py
    ---------------------
    Date                 : January 2013
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
__date__ = 'January 2013'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterRaster
from processing.core.outputs import OutputNumber
from processing.tools import dataobjects
from processing.core.outputs import OutputExtent

class RasterLayerBoundsAlgorithm(GeoAlgorithm):

    LAYER = 'LAYER'
    XMIN = 'XMIN'
    XMAX = 'XMAX'
    YMIN = 'YMIN'
    YMAX = 'YMAX'
    EXTENT = 'EXTENT'

    def defineCharacteristics(self):
        self.showInModeler = True
        self.showInToolbox = False
        self.name = 'Raster layer bounds'
        self.group = 'Modeler-only tools'
        self.addParameter(ParameterRaster(self.LAYER, 'Layer'))
        self.addOutput(OutputNumber(self.XMIN, 'min X'))
        self.addOutput(OutputNumber(self.XMAX, 'max X'))
        self.addOutput(OutputNumber(self.YMIN, 'min Y'))
        self.addOutput(OutputNumber(self.YMAX, 'max Y'))
        self.addOutput(OutputExtent(self.EXTENT, 'Extent'))

    def processAlgorithm(self, progress):
        uri = self.getParameterValue(self.LAYER)
        layer = dataobjects.getObjectFromUri(uri)
        self.setOutputValue(self.XMIN, layer.extent().xMinimum())
        self.setOutputValue(self.XMAX, layer.extent().xMaximum())
        self.setOutputValue(self.YMIN, layer.extent().yMinimum())
        self.setOutputValue(self.YMAX, layer.extent().yMaximum())
        self.setOutputValue(self.EXTENT, (layer.extent().xMinimum(),
                            layer.extent().xMaximum(),
                            layer.extent().yMinimum(),
                            layer.extent().yMaximum()))

