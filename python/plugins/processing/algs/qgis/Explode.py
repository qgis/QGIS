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
                       QgsApplication,
                       QgsProcessingUtils)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.outputs import OutputVector
from processing.tools import dataobjects


class Explode(QgisAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'

    def group(self):
        return self.tr('Vector geometry tools')

    def __init__(self):
        super().__init__()
        self.addParameter(ParameterVector(self.INPUT,
                                          self.tr('Input layer'),
                                          [dataobjects.TYPE_VECTOR_LINE]))
        self.addOutput(OutputVector(self.OUTPUT, self.tr('Exploded'), datatype=[dataobjects.TYPE_VECTOR_LINE]))

    def name(self):
        return 'explodelines'

    def displayName(self):
        return self.tr('Explode lines')

    def processAlgorithm(self, parameters, context, feedback):
        vlayer = QgsProcessingUtils.mapLayerFromString(self.getParameterValue(self.INPUT), context)
        output = self.getOutputFromName(self.OUTPUT)
        fields = vlayer.fields()
        writer = output.getVectorWriter(fields, QgsWkbTypes.LineString, vlayer.crs(), context)
        outFeat = QgsFeature()
        features = QgsProcessingUtils.getFeatures(vlayer, context)
        total = 100.0 / vlayer.featureCount() if vlayer.featureCount() else 0
        for current, feature in enumerate(features):
            feedback.setProgress(int(current * total))
            inGeom = feature.geometry()
            atMap = feature.attributes()
            segments = self.extractAsSingleSegments(inGeom)
            outFeat.setAttributes(atMap)
            for segment in segments:
                outFeat.setGeometry(segment)
                writer.addFeature(outFeat, QgsFeatureSink.FastInsert)
        del writer

    def extractAsSingleSegments(self, geom):
        segments = []
        if geom.isMultipart():
            multi = geom.asMultiPolyline()
            for polyline in multi:
                segments.extend(self.getPolylineAsSingleSegments(polyline))
        else:
            segments.extend(self.getPolylineAsSingleSegments(
                geom.asPolyline()))
        return segments

    def getPolylineAsSingleSegments(self, polyline):
        segments = []
        for i in range(len(polyline) - 1):
            ptA = polyline[i]
            ptB = polyline[i + 1]
            segment = QgsGeometry.fromPolyline([ptA, ptB])
            segments.append(segment)
        return segments
