# -*- coding: utf-8 -*-

"""
***************************************************************************
    DropGeometry.py
    --------------
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

from qgis.core import (QgsFeatureRequest,
                       QgsWkbTypes,
                       QgsCoordinateReferenceSystem,
                       QgsApplication,
                       QgsProcessingParameterDefinition,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingOutputVectorLayer)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class DropGeometry(QgisAlgorithm):

    INPUT_LAYER = 'INPUT_LAYER'
    OUTPUT_TABLE = 'OUTPUT_TABLE'

    def icon(self):
        return QgsApplication.getThemeIcon("/providerQgis.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("providerQgis.svg")

    def tags(self):
        return self.tr('remove,drop,delete,geometry,objects').split(',')

    def group(self):
        return self.tr('Vector general tools')

    def __init__(self):
        super().__init__()

        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT_LAYER, self.tr('Input layer'), [QgsProcessingParameterDefinition.TypeVectorPoint, QgsProcessingParameterDefinition.TypeVectorLine, QgsProcessingParameterDefinition.TypeVectorPolygon]))
        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT_TABLE, self.tr('Dropped geometry')))
        self.addOutput(QgsProcessingOutputVectorLayer(self.OUTPUT_TABLE, self.tr("Dropped geometry")))

    def name(self):
        return 'dropgeometries'

    def displayName(self):
        return self.tr('Drop geometries')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT_LAYER, context)
        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT_TABLE, context,
                                               source.fields(), QgsWkbTypes.NoGeometry, QgsCoordinateReferenceSystem())

        request = QgsFeatureRequest().setFlags(QgsFeatureRequest.NoGeometry)
        features = source.getFeatures(request)
        total = 100.0 / source.featureCount()

        for current, input_feature in enumerate(features):
            if feedback.isCanceled():
                break

            input_feature.clearGeometry()
            sink.addFeature(input_feature)
            feedback.setProgress(int(current * total))

        return {self.OUTPUT_TABLE: dest_id}
