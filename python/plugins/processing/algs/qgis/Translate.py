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

from qgis.core import QgsGeometry, QgsWkbTypes

from qgis.PyQt.QtGui import QIcon

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import ParameterVector, ParameterSelection, ParameterNumber
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class Translate(GeoAlgorithm):

    INPUT_LAYER = 'INPUT_LAYER'
    OUTPUT_LAYER = 'OUTPUT_LAYER'
    DELTA_X = 'DELTA_X'
    DELTA_Y = 'DELTA_Y'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Translate geometry')
        self.group, self.i18n_group = self.trAlgorithm('Vector geometry tools')

        self.addParameter(ParameterVector(self.INPUT_LAYER,
                                          self.tr('Input layer'), [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterNumber(self.DELTA_X,
                                          self.tr('Offset distance (x-axis)'), default=1.0))
        self.addParameter(ParameterNumber(self.DELTA_Y,
                                          self.tr('Offset distance (y-axis)'), default=0.0))

        self.addOutput(OutputVector(self.OUTPUT_LAYER, self.tr('Translated')))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT_LAYER))

        writer = self.getOutputFromName(
            self.OUTPUT_LAYER).getVectorWriter(
                layer.fields().toList(),
                layer.wkbType(),
                layer.crs())

        delta_x = self.getParameterValue(self.DELTA_X)
        delta_y = self.getParameterValue(self.DELTA_Y)

        features = vector.features(layer)
        total = 100.0 / len(features)

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
            progress.setPercentage(int(current * total))

        del writer
