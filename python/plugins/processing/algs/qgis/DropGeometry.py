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
                       QgsProcessingUtils)
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector


class DropGeometry(GeoAlgorithm):

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

    def name(self):
        return 'dropgeometries'

    def displayName(self):
        return self.tr('Drop geometries')

    def defineCharacteristics(self):
        self.addParameter(ParameterVector(self.INPUT_LAYER,
                                          self.tr('Input layer'), [dataobjects.TYPE_VECTOR_POINT,
                                                                   dataobjects.TYPE_VECTOR_LINE,
                                                                   dataobjects.TYPE_VECTOR_POLYGON]))
        self.addOutput(OutputVector(self.OUTPUT_TABLE, self.tr('Dropped geometry')))

    def processAlgorithm(self, context, feedback):
        layer = dataobjects.getLayerFromString(
            self.getParameterValue(self.INPUT_LAYER))
        writer = self.getOutputFromName(
            self.OUTPUT_TABLE).getVectorWriter(
                layer.fields(),
                QgsWkbTypes.NoGeometry,
                QgsCoordinateReferenceSystem())

        request = QgsFeatureRequest().setFlags(QgsFeatureRequest.NoGeometry)
        features = QgsProcessingUtils.getFeatures(layer, context, request)
        total = 100.0 / QgsProcessingUtils.featureCount(layer, context)

        for current, input_feature in enumerate(features):
            writer.addFeature(input_feature)
            feedback.setProgress(int(current * total))

        del writer
