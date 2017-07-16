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

from qgis.core import (QgsFeatureSink,
                       QgsProcessingException,
                       QgsProcessing,
                       QgsProcessingParameterDefinition,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterFeatureSink)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class Orthogonalize(QgisAlgorithm):
    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    MAX_ITERATIONS = 'MAX_ITERATIONS'
    DISTANCE_THRESHOLD = 'DISTANCE_THRESHOLD'
    ANGLE_TOLERANCE = 'ANGLE_TOLERANCE'

    def tags(self):
        return self.tr('rectangle,perpendicular,right,angles,square,quadrilateralise').split(',')

    def group(self):
        return self.tr('Vector geometry tools')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT, self.tr('Input layer'),
                                                              [QgsProcessing.TypeVectorLine,
                                                               QgsProcessing.TypeVectorPolygon]))

        self.addParameter(QgsProcessingParameterNumber(self.ANGLE_TOLERANCE,
                                                       self.tr('Maximum angle tolerance (degrees)'),
                                                       type=QgsProcessingParameterNumber.Double,
                                                       minValue=0.0, maxValue=45.0, defaultValue=15.0))

        max_iterations = QgsProcessingParameterNumber(self.MAX_ITERATIONS,
                                                      self.tr('Maximum algorithm iterations'),
                                                      type=QgsProcessingParameterNumber.Integer,
                                                      minValue=1, maxValue=10000, defaultValue=1000)
        max_iterations.setFlags(max_iterations.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        self.addParameter(max_iterations)

        self.addParameter(
            QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Orthogonalized')))

    def name(self):
        return 'orthogonalize'

    def displayName(self):
        return self.tr('Orthogonalize')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        max_iterations = self.parameterAsInt(parameters, self.MAX_ITERATIONS, context)
        angle_tolerance = self.parameterAsDouble(parameters, self.ANGLE_TOLERANCE, context)

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               source.fields(), source.wkbType(), source.sourceCrs())

        features = source.getFeatures()
        total = 100.0 / source.featureCount() if source.featureCount() else 0

        for current, input_feature in enumerate(features):
            if feedback.isCanceled():
                break

            output_feature = input_feature
            input_geometry = input_feature.geometry()
            if input_geometry:
                output_geometry = input_geometry.orthogonalize(1.0e-8, max_iterations, angle_tolerance)
                if not output_geometry:
                    raise QgsProcessingException(
                        self.tr('Error orthogonalizing geometry'))

                output_feature.setGeometry(output_geometry)

            sink.addFeature(output_feature, QgsFeatureSink.FastInsert)
            feedback.setProgress(int(current * total))

        return {self.OUTPUT: dest_id}
