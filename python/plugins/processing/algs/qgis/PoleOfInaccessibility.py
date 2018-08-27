# -*- coding: utf-8 -*-

"""
***************************************************************************
    PoleOfInaccessibility.py
    ------------------------
    Date                 : November 2016
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
__date__ = 'November 2016'
__copyright__ = '(C) 2016, Nyall Dawson'

# This will get replaced with a git SHA1 when you do a git archive323

__revision__ = '$Format:%H$'

import os

from qgis.core import (QgsApplication,
                       QgsWkbTypes,
                       QgsField,
                       NULL,
                       QgsFeatureSink,
                       QgsProcessing,
                       QgsProcessingException,
                       QgsProcessingParameterDistance,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterFeatureSink)

from qgis.PyQt.QtCore import QVariant
from qgis.PyQt.QtGui import QIcon

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class PoleOfInaccessibility(QgisAlgorithm):

    INPUT = 'INPUT'
    TOLERANCE = 'TOLERANCE'
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QgsApplication.getThemeIcon("/algorithms/mAlgorithmCentroids.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("/algorithms/mAlgorithmCentroids.svg")

    def tags(self):
        return self.tr('furthest,point,distant,extreme,maximum,centroid,center,centre').split(',')

    def group(self):
        return self.tr('Vector geometry')

    def groupId(self):
        return 'vectorgeometry'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT, self.tr('Input layer'),
                                                              [QgsProcessing.TypeVectorPolygon]))
        self.addParameter(QgsProcessingParameterDistance(self.TOLERANCE,
                                                         self.tr('Tolerance'),
                                                         parentParameterName=self.INPUT,
                                                         defaultValue=1.0, minValue=0.0))

        self.addParameter(
            QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Point'), QgsProcessing.TypeVectorPoint))

    def name(self):
        return 'poleofinaccessibility'

    def displayName(self):
        return self.tr('Pole of inaccessibility')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        if source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.INPUT))

        tolerance = self.parameterAsDouble(parameters, self.TOLERANCE, context)

        fields = source.fields()
        fields.append(QgsField('dist_pole', QVariant.Double))
        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               fields, QgsWkbTypes.Point, source.sourceCrs())
        if sink is None:
            raise QgsProcessingException(self.invalidSinkError(parameters, self.OUTPUT))

        features = source.getFeatures()
        total = 100.0 / source.featureCount() if source.featureCount() else 0

        for current, input_feature in enumerate(features):
            if feedback.isCanceled():
                break

            output_feature = input_feature
            input_geometry = input_feature.geometry()
            if input_geometry:
                output_geometry, distance = input_geometry.poleOfInaccessibility(tolerance)
                if not output_geometry:
                    raise QgsProcessingException(
                        self.tr('Error calculating pole of inaccessibility'))
                attrs = input_feature.attributes()
                attrs.append(distance)
                output_feature.setAttributes(attrs)

                output_feature.setGeometry(output_geometry)
            else:
                attrs = input_feature.attributes()
                attrs.append(NULL)
                output_feature.setAttributes(attrs)

            sink.addFeature(output_feature, QgsFeatureSink.FastInsert)
            feedback.setProgress(int(current * total))

        return {self.OUTPUT: dest_id}
