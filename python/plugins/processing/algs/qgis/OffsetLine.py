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

from qgis.core import (QgsFeatureSink,
                       QgsProcessing,
                       QgsProcessingException,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterFeatureSink)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class OffsetLine(QgisAlgorithm):
    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    DISTANCE = 'DISTANCE'
    SEGMENTS = 'SEGMENTS'
    JOIN_STYLE = 'JOIN_STYLE'
    MITRE_LIMIT = 'MITRE_LIMIT'

    def group(self):
        return self.tr('Vector geometry tools')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT, self.tr('Input layer'),
                                                              [QgsProcessing.TypeVectorLine]))
        self.addParameter(QgsProcessingParameterNumber(self.DISTANCE,
                                                       self.tr('Distance'),
                                                       type=QgsProcessingParameterNumber.Double,
                                                       defaultValue=10.0))
        self.addParameter(QgsProcessingParameterNumber(self.SEGMENTS,
                                                       self.tr('Segments'),
                                                       type=QgsProcessingParameterNumber.Integer,
                                                       minValue=1, defaultValue=8))
        self.join_styles = [self.tr('Round'),
                            'Mitre',
                            'Bevel']
        self.addParameter(QgsProcessingParameterEnum(
            self.JOIN_STYLE,
            self.tr('Join style'),
            options=self.join_styles))
        self.addParameter(QgsProcessingParameterNumber(self.MITRE_LIMIT,
                                                       self.tr('Mitre limit'), type=QgsProcessingParameterNumber.Double,
                                                       minValue=1, defaultValue=2))

        self.addParameter(
            QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Offset'), QgsProcessing.TypeVectorLine))

    def name(self):
        return 'offsetline'

    def displayName(self):
        return self.tr('Offset line')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               source.fields(), source.wkbType(), source.sourceCrs())

        distance = self.parameterAsDouble(parameters, self.DISTANCE, context)
        segments = self.parameterAsInt(parameters, self.SEGMENTS, context)
        join_style = self.parameterAsEnum(parameters, self.JOIN_STYLE, context) + 1
        miter_limit = self.parameterAsDouble(parameters, self.MITRE_LIMIT, context)

        features = source.getFeatures()
        total = 100.0 / source.featureCount() if source.featureCount() else 0

        for current, input_feature in enumerate(features):
            if feedback.isCanceled():
                break

            output_feature = input_feature
            input_geometry = input_feature.geometry()
            if input_geometry:
                output_geometry = input_geometry.offsetCurve(distance, segments, join_style, miter_limit)
                if not output_geometry:
                    raise QgsProcessingException(
                        self.tr('Error calculating line offset'))

                output_feature.setGeometry(output_geometry)

            sink.addFeature(output_feature, QgsFeatureSink.FastInsert)
            feedback.setProgress(int(current * total))

        return {self.OUTPUT: dest_id}
