# -*- coding: utf-8 -*-

"""
***************************************************************************
    Centroids.py
    ---------------------
    Date                 : August 2012
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

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive323

__revision__ = '$Format:%H$'

from PyQt4.QtCore import *
from qgis.core import *
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import \
        GeoAlgorithmExecutionException
from processing.core.parameters import ParameterVector
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector


class Centroids(GeoAlgorithm):

    INPUT_LAYER = 'INPUT_LAYER'
    OUTPUT_LAYER = 'OUTPUT_LAYER'

    def defineCharacteristics(self):
        self.name = 'Polygon centroids'
        self.group = 'Vector geometry tools'

        self.addParameter(ParameterVector(self.INPUT_LAYER,
            self.tr('Input layer'), [ParameterVector.VECTOR_TYPE_POLYGON]))

        self.addOutput(OutputVector(self.OUTPUT_LAYER, self.tr('Output layer')))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(
                self.getParameterValue(self.INPUT_LAYER))

        writer = self.getOutputFromName(
                self.OUTPUT_LAYER).getVectorWriter(
                        layer.pendingFields().toList(),
                        QGis.WKBPoint,
                        layer.crs())

        outFeat = QgsFeature()

        features = vector.features(layer)
        total = 100.0 / float(len(features))
        current = 0

        for inFeat in features:
            inGeom = inFeat.geometry()
            attrs = inFeat.attributes()

            outGeom = QgsGeometry(inGeom.centroid())
            if outGeom is None:
                raise GeoAlgorithmExecutionException(
                        self.tr('Error calculating centroid'))

            outFeat.setGeometry(outGeom)
            outFeat.setAttributes(attrs)
            writer.addFeature(outFeat)
            current += 1
            progress.setPercentage(int(current * total))

        del writer
