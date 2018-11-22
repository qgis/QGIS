# -*- coding: utf-8 -*-

"""
***************************************************************************
    PointsAlongGeometry.py
    ---------------------
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

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import math

from qgis.PyQt.QtCore import QVariant
from qgis.PyQt.QtGui import QIcon

from qgis.core import (QgsApplication,
                       QgsFeature,
                       QgsFeatureSink,
                       QgsWkbTypes,
                       QgsField,
                       QgsProcessing,
                       QgsProcessingException,
                       QgsProcessingUtils,
                       QgsProcessingParameterDistance,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class PointsAlongGeometry(QgisAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    DISTANCE = 'DISTANCE'
    START_OFFSET = 'START_OFFSET'
    END_OFFSET = 'END_OFFSET'

    def icon(self):
        return QgsApplication.getThemeIcon("/algorithms/mAlgorithmExtractVertices.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("/algorithms/mAlgorithmExtractVertices.svg")

    def tags(self):
        return self.tr('create,interpolate,points,lines,regular,distance,by').split(',')

    def group(self):
        return self.tr('Vector geometry')

    def groupId(self):
        return 'vectorgeometry'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer'), [QgsProcessing.TypeVectorPolygon, QgsProcessing.TypeVectorLine]))
        self.addParameter(QgsProcessingParameterDistance(self.DISTANCE,
                                                         self.tr('Distance'), parentParameterName=self.INPUT, minValue=0.0, defaultValue=1.0))
        self.addParameter(QgsProcessingParameterDistance(self.START_OFFSET,
                                                         self.tr('Start offset'), parentParameterName=self.INPUT, minValue=0.0, defaultValue=0.0))
        self.addParameter(QgsProcessingParameterDistance(self.END_OFFSET,
                                                         self.tr('End offset'), parentParameterName=self.INPUT, minValue=0.0, defaultValue=0.0))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Points'), QgsProcessing.TypeVectorPoint))

    def name(self):
        return 'pointsalonglines'

    def displayName(self):
        return self.tr('Points along geometry')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        if source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.INPUT))

        distance = self.parameterAsDouble(parameters, self.DISTANCE, context)
        start_offset = self.parameterAsDouble(parameters, self.START_OFFSET, context)
        end_offset = self.parameterAsDouble(parameters, self.END_OFFSET, context)

        fields = source.fields()
        fields.append(QgsField('distance', QVariant.Double))
        fields.append(QgsField('angle', QVariant.Double))

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               fields, QgsWkbTypes.Point, source.sourceCrs(), QgsFeatureSink.RegeneratePrimaryKey)
        if sink is None:
            raise QgsProcessingException(self.invalidSinkError(parameters, self.OUTPUT))

        features = source.getFeatures()
        total = 100.0 / source.featureCount() if source.featureCount() else 0
        for current, input_feature in enumerate(features):
            if feedback.isCanceled():
                break

            input_geometry = input_feature.geometry()
            if not input_geometry:
                sink.addFeature(input_feature, QgsFeatureSink.FastInsert)
            else:
                if input_geometry.type == QgsWkbTypes.PolygonGeometry:
                    length = input_geometry.constGet().perimeter()
                else:
                    length = input_geometry.length() - end_offset
                current_distance = start_offset

                while current_distance <= length:
                    point = input_geometry.interpolate(current_distance)
                    angle = math.degrees(input_geometry.interpolateAngle(current_distance))

                    output_feature = QgsFeature()
                    output_feature.setGeometry(point)
                    attrs = input_feature.attributes()
                    attrs.append(current_distance)
                    attrs.append(angle)
                    output_feature.setAttributes(attrs)
                    sink.addFeature(output_feature, QgsFeatureSink.FastInsert)

                    current_distance += distance

            feedback.setProgress(int(current * total))

        return {self.OUTPUT: dest_id}
