# -*- coding: utf-8 -*-

"""
***************************************************************************
    DensifyGeometries.py
    ---------------------
    Date                 : October 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""
from builtins import range

__author__ = 'Victor Olaya'
__date__ = 'October 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.core import (QgsWkbTypes,
                       QgsFeatureSink,
                       QgsApplication,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingOutputVectorLayer,
                       QgsProcessingParameterDefinition)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class DensifyGeometries(QgisAlgorithm):

    INPUT = 'INPUT'
    VERTICES = 'VERTICES'
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QgsApplication.getThemeIcon("/providerQgis.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("providerQgis.svg")

    def tags(self):
        return self.tr('add,vertices,points').split(',')

    def group(self):
        return self.tr('Vector geometry tools')

    def __init__(self):
        super().__init__()

        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer'), [QgsProcessingParameterDefinition.TypeVectorPolygon, QgsProcessingParameterDefinition.TypeVectorLine]))
        self.addParameter(QgsProcessingParameterNumber(self.VERTICES,
                                                       self.tr('Vertices to add'), QgsProcessingParameterNumber.Integer,
                                                       1, False, 1, 10000000))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Densified')))
        self.addOutput(QgsProcessingOutputVectorLayer(self.OUTPUT, self.tr('Densified')))

    def name(self):
        return 'densifygeometries'

    def displayName(self):
        return self.tr('Densify geometries')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        vertices = self.parameterAsInt(parameters, self.VERTICES, context)

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               source.fields(), source.wkbType(), source.sourceCrs())

        features = source.getFeatures()
        total = 100.0 / source.featureCount() if source.featureCount() else 0

        for current, f in enumerate(features):
            if feedback.isCanceled():
                break

            feature = f
            if feature.hasGeometry():
                new_geometry = feature.geometry().densifyByCount(vertices)
                feature.setGeometry(new_geometry)
            sink.addFeature(feature, QgsFeatureSink.FastInsert)
            feedback.setProgress(int(current * total))

        return {self.OUTPUT: dest_id}
