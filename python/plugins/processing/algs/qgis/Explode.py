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
from builtins import range

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
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink,
                       QgsLineString)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class Explode(QgisAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'

    def group(self):
        return self.tr('Vector geometry')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer'), [QgsProcessing.TypeVectorLine]))
        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT,
                                                            self.tr('Exploded'), QgsProcessing.TypeVectorLine))

    def name(self):
        return 'explodelines'

    def displayName(self):
        return self.tr('Explode lines')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               source.fields(), QgsWkbTypes.singleType(source.wkbType()), source.sourceCrs())

        features = source.getFeatures()
        total = 100.0 / source.featureCount() if source.featureCount() else 0
        for current, feature in enumerate(features):
            if feedback.isCanceled():
                break

            feedback.setProgress(int(current * total))

            if not feature.hasGeometry():
                sink.addFeature(feature, QgsFeatureSink.FastInsert)
                continue

            outFeat = QgsFeature()
            inGeom = feature.geometry()
            segments = self.extractAsSingleSegments(inGeom)
            outFeat.setAttributes(feature.attributes())
            for segment in segments:
                outFeat.setGeometry(segment)
                sink.addFeature(outFeat, QgsFeatureSink.FastInsert)
        return {self.OUTPUT: dest_id}

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
