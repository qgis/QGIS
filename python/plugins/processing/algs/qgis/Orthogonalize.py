# -*- coding: utf-8 -*-

"""
***************************************************************************
    Orthogonalize.py
    ----------------
    Date                 : December 2016
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
__date__ = 'December 2016'
__copyright__ = '(C) 2016, Nyall Dawson'

# This will get replaced with a git SHA1 when you do a git archive323

__revision__ = '$Format:%H$'

from qgis.core import (QgsApplication,
                       QgsProcessingUtils)
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import ParameterVector, ParameterNumber
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector


class Orthogonalize(GeoAlgorithm):

    INPUT_LAYER = 'INPUT_LAYER'
    OUTPUT_LAYER = 'OUTPUT_LAYER'
    MAX_ITERATIONS = 'MAX_ITERATIONS'
    DISTANCE_THRESHOLD = 'DISTANCE_THRESHOLD'
    ANGLE_TOLERANCE = 'ANGLE_TOLERANCE'

    def icon(self):
        return QgsApplication.getThemeIcon("/providerQgis.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("providerQgis.svg")

    def tags(self):
        return self.tr('rectangle,perpendicular,right,angles,square,quadrilateralise').split(',')

    def group(self):
        return self.tr('Vector geometry tools')

    def name(self):
        return 'orthogonalize'

    def displayName(self):
        return self.tr('Orthogonalize')

    def defineCharacteristics(self):
        self.addParameter(ParameterVector(self.INPUT_LAYER,
                                          self.tr('Input layer'), [dataobjects.TYPE_VECTOR_LINE,
                                                                   dataobjects.TYPE_VECTOR_POLYGON]))
        self.addParameter(ParameterNumber(self.ANGLE_TOLERANCE,
                                          self.tr('Maximum angle tolerance (degrees)'),
                                          0.0, 45.0, 15.0))

        max_iterations = ParameterNumber(self.MAX_ITERATIONS,
                                         self.tr('Maximum algorithm iterations'),
                                         1, 10000, 1000)
        max_iterations.isAdvanced = True
        self.addParameter(max_iterations)

        self.addOutput(OutputVector(self.OUTPUT_LAYER, self.tr('Orthogonalized')))

    def processAlgorithm(self, context, feedback):
        layer = dataobjects.getLayerFromString(
            self.getParameterValue(self.INPUT_LAYER))
        max_iterations = self.getParameterValue(self.MAX_ITERATIONS)
        angle_tolerance = self.getParameterValue(self.ANGLE_TOLERANCE)
        writer = self.getOutputFromName(
            self.OUTPUT_LAYER).getVectorWriter(
                layer.fields(),
                layer.wkbType(),
                layer.crs())

        features = QgsProcessingUtils.getFeatures(layer, context)
        total = 100.0 / QgsProcessingUtils.featureCount(layer, context)

        for current, input_feature in enumerate(features):
            output_feature = input_feature
            input_geometry = input_feature.geometry()
            if input_geometry:
                output_geometry = input_geometry.orthogonalize(1.0e-8, max_iterations, angle_tolerance)
                if not output_geometry:
                    raise GeoAlgorithmExecutionException(
                        self.tr('Error orthogonalizing geometry'))

                output_feature.setGeometry(output_geometry)

            writer.addFeature(output_feature)
            feedback.setProgress(int(current * total))

        del writer
