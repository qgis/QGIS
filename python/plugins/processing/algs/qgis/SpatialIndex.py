# -*- coding: utf-8 -*-

"""
***************************************************************************
    SpatialIndex.py
    ---------------------
    Date                 : January 2016
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
__date__ = 'January 2016'
__copyright__ = '(C) 2016, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.core import (QgsApplication,
                       QgsVectorDataProvider)

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.outputs import OutputVector

from processing.tools import dataobjects

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class SpatialIndex(GeoAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QgsApplication.getThemeIcon("/providerQgis.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("providerQgis.svg")

    def group(self):
        return self.tr('Vector general tools')

    def name(self):
        return 'createspatialindex'

    def displayName(self):
        return self.tr('Create spatial index')

    def defineCharacteristics(self):
        self.addParameter(ParameterVector(self.INPUT,
                                          self.tr('Input Layer')))
        self.addOutput(OutputVector(self.OUTPUT,
                                    self.tr('Indexed layer'), True))

    def processAlgorithm(self, context, feedback):
        fileName = self.getParameterValue(self.INPUT)
        layer = dataobjects.getLayerFromString(fileName)
        provider = layer.dataProvider()

        if provider.capabilities() & QgsVectorDataProvider.CreateSpatialIndex:
            if not provider.createSpatialIndex():
                feedback.pushInfo(self.tr('Could not create spatial index'))
        else:
            feedback.pushInfo(self.tr("Layer's data provider does not support "
                                      "spatial indexes"))

        self.setOutputValue(self.OUTPUT, fileName)
