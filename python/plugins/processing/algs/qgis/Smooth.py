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

from qgis.core import QgsFeature
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import ParameterVector, ParameterNumber
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector


class Smooth(GeoAlgorithm):

    INPUT_LAYER = 'INPUT_LAYER'
    OUTPUT_LAYER = 'OUTPUT_LAYER'
    ITERATIONS = 'ITERATIONS'
    OFFSET = 'OFFSET'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Smooth geometry')
        self.group, self.i18n_group = self.trAlgorithm('Vector geometry tools')

        self.addParameter(ParameterVector(self.INPUT_LAYER,
                                          self.tr('Input layer'), [ParameterVector.VECTOR_TYPE_POLYGON, ParameterVector.VECTOR_TYPE_LINE]))
        self.addParameter(ParameterNumber(self.ITERATIONS,
                                          self.tr('Iterations'), default=1, minValue=1, maxValue=10))
        self.addParameter(ParameterNumber(self.OFFSET,
                                          self.tr('Offset'), default=0.25, minValue=0.0, maxValue=0.5))
        self.addOutput(OutputVector(self.OUTPUT_LAYER, self.tr('Smoothed')))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT_LAYER))
        provider = layer.dataProvider()
        iterations = self.getParameterValue(self.ITERATIONS)
        offset = self.getParameterValue(self.OFFSET)

        writer = self.getOutputFromName(
            self.OUTPUT_LAYER).getVectorWriter(
                layer.fields().toList(),
                provider.geometryType(),
                layer.crs())

        outFeat = QgsFeature()

        features = vector.features(layer)
        total = 100.0 / len(features)

        for current, inFeat in enumerate(features):
            inGeom = inFeat.constGeometry()
            attrs = inFeat.attributes()

            outGeom = inGeom.smooth(iterations, offset)
            if outGeom is None:
                raise GeoAlgorithmExecutionException(
                    self.tr('Error smoothing geometry'))

            outFeat.setGeometry(outGeom)
            outFeat.setAttributes(attrs)
            writer.addFeature(outFeat)
            progress.setPercentage(int(current * total))

        del writer
