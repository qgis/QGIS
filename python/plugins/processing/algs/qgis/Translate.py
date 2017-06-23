# -*- coding: utf-8 -*-

"""
***************************************************************************
    Translate.py
    --------------
    Date                 : August 2016
    Copyright            : (C) 2016 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Nyall Dawson'
__date__ = 'August 2016'
__copyright__ = '(C) 2016, Nyall Dawson'

# This will get replaced with a git SHA1 when you do a git archive323

__revision__ = '$Format:%H$'

import os

from qgis.core import (QgsApplication,
                       QgsProcessingUtils)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import ParameterVector, ParameterNumber
from processing.core.outputs import OutputVector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class Translate(QgisAlgorithm):

    INPUT_LAYER = 'INPUT_LAYER'
    OUTPUT_LAYER = 'OUTPUT_LAYER'
    DELTA_X = 'DELTA_X'
    DELTA_Y = 'DELTA_Y'

    def icon(self):
        return QgsApplication.getThemeIcon("/providerQgis.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("providerQgis.svg")

    def group(self):
        return self.tr('Vector geometry tools')

    def __init__(self):
        super().__init__()
        self.addParameter(ParameterVector(self.INPUT_LAYER,
                                          self.tr('Input layer')))
        self.addParameter(ParameterNumber(self.DELTA_X,
                                          self.tr('Offset distance (x-axis)'), default=1.0))
        self.addParameter(ParameterNumber(self.DELTA_Y,
                                          self.tr('Offset distance (y-axis)'), default=0.0))

        self.addOutput(OutputVector(self.OUTPUT_LAYER, self.tr('Translated')))

    def name(self):
        return 'translategeometry'

    def displayName(self):
        return self.tr('Translate geometry')

    def processAlgorithm(self, parameters, context, feedback):
        layer = QgsProcessingUtils.mapLayerFromString(self.getParameterValue(self.INPUT_LAYER), context)

        writer = self.getOutputFromName(
            self.OUTPUT_LAYER).getVectorWriter(layer.fields(), layer.wkbType(), layer.crs(), context)

        delta_x = self.getParameterValue(self.DELTA_X)
        delta_y = self.getParameterValue(self.DELTA_Y)

        features = QgsProcessingUtils.getFeatures(layer, context)
        total = 100.0 / layer.featureCount() if layer.featureCount() else 0

        for current, input_feature in enumerate(features):
            output_feature = input_feature
            input_geometry = input_feature.geometry()
            if input_geometry:
                output_geometry = input_geometry
                output_geometry.translate(delta_x, delta_y)
                if not output_geometry:
                    raise GeoAlgorithmExecutionException(
                        self.tr('Error translating geometry'))

                output_feature.setGeometry(output_geometry)

            writer.addFeature(output_feature)
            feedback.setProgress(int(current * total))

        del writer
