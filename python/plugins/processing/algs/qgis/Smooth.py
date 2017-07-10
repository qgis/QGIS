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

from qgis.core import (QgsApplication,
                       QgsFeatureSink,
                       QgsProcessingUtils,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingParameterNumber,
                       QgsProcessingOutputVectorLayer)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.tools import dataobjects


class Smooth(QgisAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    ITERATIONS = 'ITERATIONS'
    MAX_ANGLE = 'MAX_ANGLE'
    OFFSET = 'OFFSET'

    def group(self):
        return self.tr('Vector geometry tools')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer'), [dataobjects.TYPE_VECTOR_POLYGON, dataobjects.TYPE_VECTOR_LINE]))
        self.addParameter(QgsProcessingParameterNumber(self.ITERATIONS,
                                                       self.tr('Iterations'),
                                                       defaultValue=1, minValue=1, maxValue=10))
        self.addParameter(QgsProcessingParameterNumber(self.OFFSET,
                                                       self.tr('Offset'), QgsProcessingParameterNumber.Double,
                                                       defaultValue=0.25, minValue=0.0, maxValue=0.5))
        self.addParameter(QgsProcessingParameterNumber(self.MAX_ANGLE,
                                                       self.tr('Maximum node angle to smooth'), QgsProcessingParameterNumber.Double,
                                                       defaultValue=180.0, minValue=0.0, maxValue=180.0))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Smoothed')))
        self.addOutput(QgsProcessingOutputVectorLayer(self.OUTPUT, self.tr('Smoothed')))

    def name(self):
        return 'smoothgeometry'

    def displayName(self):
        return self.tr('Smooth geometry')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        iterations = self.parameterAsInt(parameters, self.ITERATIONS, context)
        offset = self.parameterAsDouble(parameters, self.OFFSET, context)
        max_angle = self.parameterAsDouble(parameters, self.MAX_ANGLE, context)

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               source.fields(), source.wkbType(), source.sourceCrs())

        features = source.getFeatures()
        total = 100.0 / source.featureCount() if source.featureCount() else 0

        for current, input_feature in enumerate(features):
            if feedback.isCanceled():
                break
            output_feature = input_feature
            if input_feature.geometry():
                output_geometry = input_feature.geometry().smooth(iterations, offset, -1, max_angle)
                if not output_geometry:
                    raise GeoAlgorithmExecutionException(
                        self.tr('Error smoothing geometry'))

                output_feature.setGeometry(output_geometry)

            sink.addFeature(output_feature, QgsFeatureSink.FastInsert)
            feedback.setProgress(int(current * total))

        return {self.OUTPUT: dest_id}
