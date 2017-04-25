# -*- coding: utf-8 -*-

"""
***************************************************************************
    DeleteDuplicateGeometries.py
    ---------------------
    Date                 : May 2010
    Copyright            : (C) 2010 by Michael Minn
    Email                : pyqgis at michaelminn dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Michael Minn'
__date__ = 'May 2010'
__copyright__ = '(C) 2010, Michael Minn'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.core import (QgsFeatureRequest,
                       QgsApplication,
                       QgsProcessingUtils)
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector


class DeleteDuplicateGeometries(GeoAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QgsApplication.getThemeIcon("/providerQgis.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("providerQgis.svg")

    def group(self):
        return self.tr('Vector general tools')

    def name(self):
        return 'deleteduplicategeometries'

    def displayName(self):
        return self.tr('Delete duplicate geometries')

    def defineCharacteristics(self):
        self.addParameter(ParameterVector(self.INPUT,
                                          self.tr('Input layer')))
        self.addOutput(OutputVector(self.OUTPUT, self.tr('Cleaned')))

    def processAlgorithm(self, context, feedback):
        layer = dataobjects.getLayerFromString(
            self.getParameterValue(self.INPUT))

        fields = layer.fields()

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(fields,
                                                                     layer.wkbType(), layer.crs())

        features = QgsProcessingUtils.getFeatures(layer, context)

        total = 100.0 / QgsProcessingUtils.featureCount(layer, context)
        geoms = dict()
        for current, f in enumerate(features):
            geoms[f.id()] = f.geometry()
            feedback.setProgress(int(current * total))

        cleaned = dict(geoms)

        for i, g in list(geoms.items()):
            for j in list(cleaned.keys()):
                if i == j or i not in cleaned:
                    continue
                if g.isGeosEqual(cleaned[j]):
                    del cleaned[j]

        total = 100.0 / len(cleaned)
        request = QgsFeatureRequest().setFilterFids(list(cleaned.keys()))
        for current, f in enumerate(layer.getFeatures(request)):
            writer.addFeature(f)
            feedback.setProgress(int(current * total))

        del writer
