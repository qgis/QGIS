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

from qgis.core import (QgsGeometry,
                       QgsPoint,
                       QgsWkbTypes,
                       QgsApplication,
                       QgsProcessingUtils)

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterNumber
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class DensifyGeometries(GeoAlgorithm):

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

    def name(self):
        return 'densifygeometries'

    def displayName(self):
        return self.tr('Densify geometries')

    def defineCharacteristics(self):
        self.addParameter(ParameterVector(self.INPUT,
                                          self.tr('Input layer'),
                                          [dataobjects.TYPE_VECTOR_POLYGON, dataobjects.TYPE_VECTOR_LINE]))
        self.addParameter(ParameterNumber(self.VERTICES,
                                          self.tr('Vertices to add'), 1, 10000000, 1))

        self.addOutput(OutputVector(self.OUTPUT,
                                    self.tr('Densified')))

    def processAlgorithm(self, context, feedback):
        layer = dataobjects.getLayerFromString(
            self.getParameterValue(self.INPUT))
        vertices = self.getParameterValue(self.VERTICES)

        isPolygon = layer.geometryType() == QgsWkbTypes.PolygonGeometry

        writer = self.getOutputFromName(
            self.OUTPUT).getVectorWriter(layer.fields().toList(),
                                         layer.wkbType(), layer.crs())

        features = QgsProcessingUtils.getFeatures(layer, context)
        total = 100.0 / QgsProcessingUtils.featureCount(layer, context)
        for current, f in enumerate(features):
            feature = f
            if feature.hasGeometry():
                new_geometry = feature.geometry().densifyByCount(int(vertices))
                feature.setGeometry(new_geometry)
            writer.addFeature(feature)
            feedback.setProgress(int(current * total))

        del writer
