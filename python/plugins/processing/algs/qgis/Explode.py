# -*- coding: utf-8 -*-

"""
***************************************************************************
    Explode.py
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

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.core import (QgsFeature,
                       QgsGeometry,
                       QgsFeatureSink,
                       QgsWkbTypes,
                       QgsProcessing,
                       QgsLineString)
from processing.algs.qgis.QgisAlgorithm import QgisFeatureBasedAlgorithm


class Explode(QgisFeatureBasedAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'

    def group(self):
        return self.tr('Vector geometry')

    def groupId(self):
        return 'vectorgeometry'

    def __init__(self):
        super().__init__()

    def inputLayerTypes(self):
        return [QgsProcessing.TypeVectorLine]

    def outputName(self):
        return self.tr('Exploded')

    def outputWkbType(self, inputWkb):
        return QgsWkbTypes.singleType(inputWkb)

    def outputLayerType(self):
        return QgsProcessing.TypeVectorLine

    def name(self):
        return 'explodelines'

    def displayName(self):
        return self.tr('Explode lines')

    def processFeature(self, feature, context, feedback):
        if not feature.hasGeometry():
            return [feature]

        segments = self.extractAsSingleSegments(feature.geometry())
        output_features = []
        for segment in segments:
            output_feature = QgsFeature()
            output_feature.setAttributes(feature.attributes())
            output_feature.setGeometry(segment)
            output_features.append(output_feature)
        return output_features

    def extractAsSingleSegments(self, geom):
        segments = []
        if geom.isMultipart():
            for part in range(geom.constGet().numGeometries()):
                segments.extend(self.getPolylineAsSingleSegments(geom.constGet().geometryN(part)))
        else:
            segments.extend(self.getPolylineAsSingleSegments(
                geom.constGet()))
        return segments

    def getPolylineAsSingleSegments(self, polyline):
        segments = []
        for i in range(polyline.numPoints() - 1):
            ptA = polyline.pointN(i)
            ptB = polyline.pointN(i + 1)
            segment = QgsGeometry(QgsLineString([ptA, ptB]))
            segments.append(segment)
        return segments
