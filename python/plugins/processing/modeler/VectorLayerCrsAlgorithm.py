# -*- coding: utf-8 -*-

"""
***************************************************************************
    VectorLayerCrsAlgorithm.py
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
from processing.core.parameters import ParameterVector
from processing.core.outputs import OutputCrs
from processing.tools import dataobjects


class VectorLayerCrsAlgorithm(GeoAlgorithm):

    LAYER = 'LAYER'
    CRS = 'CRS'

    def defineCharacteristics(self):
        self.showInModeler = True
        self.showInToolbox = False

        self.name = self.tr('Vector layer CRS', 'VectorLayerCrsAlgorithm')
        self.group = self.tr('Modeler-only tools', 'VectorLayerCrsAlgorithm')

        self.addParameter(ParameterVector(self.LAYER, self.tr('Layer', 'VectorLayerCrsAlgorithm'), ))
        self.addOutput(OutputCrs(self.CRS, self.tr('CRS', 'VectorLayerCrsAlgorithm')))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(self.getParameterValue(self.LAYER))
        self.setOutputValue(self.CRS, layer.crs().authid())
