# -*- coding: utf-8 -*-

"""
***************************************************************************
    Smooth.py
    ---------
    Date                 : November 2015
    Copyright            : (C) 2015 by Nyall Dawson
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
__date__ = 'November 2015'
__copyright__ = '(C) 2015, Nyall Dawson'

# This will get replaced with a git SHA1 when you do a git archive323

__revision__ = '$Format:%H$'

from qgis.core import QgsFeature
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import ParameterVector, ParameterNumber
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector


class Smooth(GeoAlgorithm):

    INPUT_LAYER = 'INPUT_LAYER'
    OUTPUT_LAYER = 'OUTPUT_LAYER'
    ITERATIONS = 'ITERATIONS'
    MAX_ANGLE = 'MAX_ANGLE'
    OFFSET = 'OFFSET'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Smooth geometry')
        self.group, self.i18n_group = self.trAlgorithm('Vector geometry tools')

        self.addParameter(ParameterVector(self.INPUT_LAYER,
                                          self.tr('Input layer'), [dataobjects.TYPE_VECTOR_POLYGON, dataobjects.TYPE_VECTOR_LINE]))
        self.addParameter(ParameterNumber(self.ITERATIONS,
                                          self.tr('Iterations'), default=1, minValue=1, maxValue=10))
        self.addParameter(ParameterNumber(self.OFFSET,
                                          self.tr('Offset'), default=0.25, minValue=0.0, maxValue=0.5))
        self.addParameter(ParameterNumber(self.MAX_ANGLE,
                                          self.tr('Maximum node angle to smooth'), default=180.0, minValue=0.0, maxValue=180.0))
        self.addOutput(OutputVector(self.OUTPUT_LAYER, self.tr('Smoothed')))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT_LAYER))
        iterations = self.getParameterValue(self.ITERATIONS)
        offset = self.getParameterValue(self.OFFSET)
        max_angle = self.getParameterValue(self.MAX_ANGLE)

        writer = self.getOutputFromName(
            self.OUTPUT_LAYER).getVectorWriter(
                layer.fields().toList(),
                layer.wkbType(),
                layer.crs())

        features = vector.features(layer)
        total = 100.0 / len(features)

        for current, input_feature in enumerate(features):
            output_feature = input_feature
            if input_feature.geometry():
                output_geometry = input_feature.geometry().smooth(iterations, offset, -1, max_angle)
                if not output_geometry:
                    raise GeoAlgorithmExecutionException(
                        self.tr('Error smoothing geometry'))

                output_feature.setGeometry(output_geometry)

            writer.addFeature(output_feature)
            progress.setPercentage(int(current * total))

        del writer
