# -*- coding: utf-8 -*-

"""
***************************************************************************
    OffsetLine.py
    --------------
    Date                 : July 2016
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
__date__ = 'July 2016'
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


class OffsetLine(GeoAlgorithm):

    INPUT_LAYER = 'INPUT_LAYER'
    OUTPUT_LAYER = 'OUTPUT_LAYER'
    DISTANCE = 'DISTANCE'
    SEGMENTS = 'SEGMENTS'
    JOIN_STYLE = 'JOIN_STYLE'
    MITRE_LIMIT = 'MITRE_LIMIT'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Offset line')
        self.group, self.i18n_group = self.trAlgorithm('Vector geometry tools')

        self.addParameter(ParameterVector(self.INPUT_LAYER,
                                          self.tr('Input layer'), [dataobjects.TYPE_VECTOR_LINE]))
        self.addParameter(ParameterNumber(self.DISTANCE,
                                          self.tr('Distance'), default=10.0))
        self.addParameter(ParameterNumber(self.SEGMENTS,
                                          self.tr('Segments'), 1, default=8))
        self.join_styles = [self.tr('Round'),
                            'Mitre',
                            'Bevel']
        self.addParameter(ParameterSelection(
            self.JOIN_STYLE,
            self.tr('Join style'),
            self.join_styles))
        self.addParameter(ParameterNumber(self.MITRE_LIMIT,
                                          self.tr('Mitre limit'), 1, default=2))

        self.addOutput(OutputVector(self.OUTPUT_LAYER, self.tr('Offset'), datatype=[dataobjects.TYPE_VECTOR_LINE]))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT_LAYER))

        writer = self.getOutputFromName(
            self.OUTPUT_LAYER).getVectorWriter(
                layer.fields().toList(),
                QgsWkbTypes.LineString,
                layer.crs())

        distance = self.getParameterValue(self.DISTANCE)
        segments = int(self.getParameterValue(self.SEGMENTS))
        join_style = self.getParameterValue(self.JOIN_STYLE) + 1
        miter_limit = self.getParameterValue(self.MITRE_LIMIT)

        features = vector.features(layer)
        total = 100.0 / len(features)

        for current, input_feature in enumerate(features):
            output_feature = input_feature
            input_geometry = input_feature.geometry()
            if input_geometry:
                output_geometry = input_geometry.offsetCurve(distance, segments, join_style, miter_limit)
                if not output_geometry:
                    raise GeoAlgorithmExecutionException(
                        self.tr('Error calculating line offset'))

                output_feature.setGeometry(output_geometry)

            writer.addFeature(output_feature)
            progress.setPercentage(int(current * total))

        del writer
