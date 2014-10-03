# -*- coding: utf-8 -*-

"""
***************************************************************************
    VectorLayerBoundsAlgorithm.py
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
from processing.core.parameters import ParameterVector
from processing.core.outputs import OutputNumber
from processing.core.outputs import OutputExtent
from processing.tools import dataobjects


class VectorLayerBoundsAlgorithm(GeoAlgorithm):

    LAYER = 'LAYER'
    XMIN = 'XMIN'
    XMAX = 'XMAX'
    YMIN = 'YMIN'
    YMAX = 'YMAX'
    EXTENT = 'EXTENT'

    def defineCharacteristics(self):
        self.showInModeler = True
        self.showInToolbox = False
        self.name = self.tr('Vector layer bounds', 'VectorLayerBoundsAlgorithm')
        self.group = self.tr('Modeler-only tools', 'VectorLayerBoundsAlgorithm')
        self.addParameter(ParameterVector(self.LAYER, self.tr('Layer', 'VectorLayerBoundsAlgorithm')))
        self.addOutput(OutputNumber(self.XMIN, self.tr('min X', 'VectorLayerBoundsAlgorithm')))
        self.addOutput(OutputNumber(self.XMAX, self.tr('max X', 'VectorLayerBoundsAlgorithm')))
        self.addOutput(OutputNumber(self.YMIN, self.tr('min Y', 'VectorLayerBoundsAlgorithm')))
        self.addOutput(OutputNumber(self.YMAX, self.tr('max Y', 'VectorLayerBoundsAlgorithm')))
        self.addOutput(OutputExtent(self.EXTENT, self.tr('Extent', 'VectorLayerBoundsAlgorithm')))

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
