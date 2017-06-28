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
                       QgsFeatureSink,
                       QgsCoordinateReferenceSystem,
                       QgsApplication,
                       QgsProcessingParameterDefinition,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingOutputVectorLayer)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class DropGeometry(QgisAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'

    def tags(self):
        return self.tr('remove,drop,delete,geometry,objects').split(',')

    def group(self):
        return self.tr('Vector general tools')

    def __init__(self):
        super().__init__()

        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT, self.tr('Input layer'), [QgsProcessingParameterDefinition.TypeVectorPoint, QgsProcessingParameterDefinition.TypeVectorLine, QgsProcessingParameterDefinition.TypeVectorPolygon]))
        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Dropped geometry')))
        self.addOutput(QgsProcessingOutputVectorLayer(self.OUTPUT, self.tr("Dropped geometry")))

        self.source = None
        self.sink = None
        self.dest_id = None

    def name(self):
        return 'dropgeometries'

    def displayName(self):
        return self.tr('Drop geometries')

    def prepareAlgorithm(self, parameters, context, feedback):
        self.source = self.parameterAsSource(parameters, self.INPUT, context)
        (self.sink, self.dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                                         self.source.fields(), QgsWkbTypes.NoGeometry, QgsCoordinateReferenceSystem())
        return True

    def processAlgorithm(self, context, feedback):
        request = QgsFeatureRequest().setFlags(QgsFeatureRequest.NoGeometry)
        features = self.source.getFeatures(request)
        total = 100.0 / self.source.featureCount() if self.source.featureCount() else 0

        for current, input_feature in enumerate(features):
            if feedback.isCanceled():
                break

            input_feature.clearGeometry()
            self.sink.addFeature(input_feature, QgsFeatureSink.FastInsert)
            feedback.setProgress(int(current * total))

        return True

    def postProcessAlgorithm(self, context, feedback):
        return {self.OUTPUT: self.dest_id}
