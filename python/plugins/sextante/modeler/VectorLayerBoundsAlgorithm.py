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

from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.outputs.OutputNumber import OutputNumber
from sextante.core.QGisLayers import QGisLayers
from sextante.parameters.ParameterVector import ParameterVector

class VectorLayerBoundsAlgorithm(GeoAlgorithm):

    LAYER = "LAYER"
    XMIN = "XMIN"
    XMAX = "XMAX"
    YMIN = "YMIN"
    YMAX = "YMAX"

    def defineCharacteristics(self):
        self.showInModeler = True
        self.showInToolbox = False
        self.name = "Vector layer bounds"
        self.group = "Modeler-only tools"
        self.addParameter(ParameterVector(self.LAYER, "Layer"))
        self.addOutput(OutputNumber(self.XMIN, "min X"))
        self.addOutput(OutputNumber(self.XMAX, "max X"))
        self.addOutput(OutputNumber(self.YMIN, "min Y"))
        self.addOutput(OutputNumber(self.YMAX, "max Y"))

    def processAlgorithm(self, progress):
        uri = self.getParameterValue(self.LAYER)
        layer = QGisLayers.getObjectFromUri(uri);
        self.setOutputValue(self.XMIN, layer.extent().xMinimum())
        self.setOutputValue(self.XMAX, layer.extent().xMaximum())
        self.setOutputValue(self.YMIN, layer.extent().yMinimum())
        self.setOutputValue(self.YMAX, layer.extent().yMaximum())

