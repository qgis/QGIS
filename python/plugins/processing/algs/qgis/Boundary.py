# -*- coding: utf-8 -*-

"""
***************************************************************************
    Boundary.py
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

from qgis.core import (QgsGeometry,
                       QgsWkbTypes,
                       QgsFeatureSink,
                       QgsProcessingUtils,
                       QgsProcessingParameterDefinition,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingOutputVectorLayer)

from qgis.PyQt.QtGui import QIcon

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import ParameterVector
from processing.core.outputs import OutputVector
from processing.tools import dataobjects

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class Boundary(QgisAlgorithm):

    INPUT_LAYER = 'INPUT_LAYER'
    OUTPUT_LAYER = 'OUTPUT_LAYER'

    def __init__(self):
        super().__init__()
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT_LAYER, self.tr('Input layer'), [QgsProcessingParameterDefinition.TypeVectorLine, QgsProcessingParameterDefinition.TypeVectorPolygon]))
        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT_LAYER, self.tr('Boundary')))
        self.addOutput(QgsProcessingOutputVectorLayer(self.OUTPUT_LAYER, self.tr("Boundaries")))

        self.source = None
        self.sink = None
        self.dest_id = None

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'convex_hull.png'))

    def group(self):
        return self.tr('Vector geometry tools')

    def name(self):
        return 'boundary'

    def displayName(self):
        return self.tr('Boundary')

    def prepareAlgorithm(self, parameters, context, feedback):
        self.source = self.parameterAsSource(parameters, self.INPUT_LAYER, context)

        input_wkb = self.source.wkbType()
        output_wkb = None
        if QgsWkbTypes.geometryType(input_wkb) == QgsWkbTypes.LineGeometry:
            output_wkb = QgsWkbTypes.MultiPoint
        elif QgsWkbTypes.geometryType(input_wkb) == QgsWkbTypes.PolygonGeometry:
            output_wkb = QgsWkbTypes.MultiLineString
        if QgsWkbTypes.hasZ(input_wkb):
            output_wkb = QgsWkbTypes.addZ(output_wkb)
        if QgsWkbTypes.hasM(input_wkb):
            output_wkb = QgsWkbTypes.addM(output_wkb)

        (self.sink, self.dest_id) = self.parameterAsSink(parameters, self.OUTPUT_LAYER, context,
                                                         self.source.fields(), output_wkb, self.source.sourceCrs())
        return True

    def processAlgorithm(self, context, feedback):
        features = self.source.getFeatures()
        total = 100.0 / self.source.featureCount() if self.source.featureCount() else 0

        for current, input_feature in enumerate(features):
            if feedback.isCanceled():
                break
            output_feature = input_feature
            input_geometry = input_feature.geometry()
            if input_geometry:
                output_geometry = QgsGeometry(input_geometry.geometry().boundary())
                if not output_geometry:
                    raise GeoAlgorithmExecutionException(
                        self.tr('Error calculating boundary'))

                output_feature.setGeometry(output_geometry)

            self.sink.addFeature(output_feature, QgsFeatureSink.FastInsert)
            feedback.setProgress(int(current * total))
        return True

    def postProcessAlgorithm(self, context, feedback):
        return {self.OUTPUT_LAYER: self.dest_id}
