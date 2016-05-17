# -*- coding: utf-8 -*-

"""
***************************************************************************
    RandomPointsAlongLines.py
    ---------------------
    Date                 : April 2014
    Copyright            : (C) 2014 by Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Alexander Bruy'
__date__ = 'April 2014'
__copyright__ = '(C) 2014, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import random

from qgis.PyQt.QtCore import QVariant
from qgis.core import (QGis, QgsFields, QgsField, QgsGeometry, QgsSpatialIndex,
                       QgsDistanceArea, QgsFeatureRequest, QgsFeature,
                       QgsPoint)

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.ProcessingLog import ProcessingLog
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterNumber
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector


class RandomPointsAlongLines(GeoAlgorithm):

    VECTOR = 'VECTOR'
    POINT_NUMBER = 'POINT_NUMBER'
    MIN_DISTANCE = 'MIN_DISTANCE'
    OUTPUT = 'OUTPUT'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Random points along line')
        self.group, self.i18n_group = self.trAlgorithm('Vector creation tools')
        self.addParameter(ParameterVector(self.VECTOR,
                                          self.tr('Input layer'), [ParameterVector.VECTOR_TYPE_LINE]))
        self.addParameter(ParameterNumber(self.POINT_NUMBER,
                                          self.tr('Number of points'), 1, None, 1))
        self.addParameter(ParameterNumber(self.MIN_DISTANCE,
                                          self.tr('Minimum distance'), 0.0, None, 0.0))

        self.addOutput(OutputVector(self.OUTPUT, self.tr('Random points')))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.VECTOR))
        pointCount = float(self.getParameterValue(self.POINT_NUMBER))
        minDistance = float(self.getParameterValue(self.MIN_DISTANCE))

        fields = QgsFields()
        fields.append(QgsField('id', QVariant.Int, '', 10, 0))
        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(
            fields, QGis.WKBPoint, layer.dataProvider().crs())

        nPoints = 0
        nIterations = 0
        maxIterations = pointCount * 200
        featureCount = layer.featureCount()
        total = 100.0 / pointCount

        index = QgsSpatialIndex()
        points = dict()

        da = QgsDistanceArea()
        request = QgsFeatureRequest()

        random.seed()

        while nIterations < maxIterations and nPoints < pointCount:
            # pick random feature
            fid = random.randint(0, featureCount - 1)
            f = layer.getFeatures(request.setFilterFid(fid)).next()
            fGeom = QgsGeometry(f.geometry())

            if fGeom.isMultipart():
                lines = fGeom.asMultiPolyline()
                # pick random line
                lineId = random.randint(0, len(lines) - 1)
                vertices = lines[lineId]
            else:
                vertices = fGeom.asPolyline()

            # pick random segment
            if len(vertices) == 2:
                vid = 0
            else:
                vid = random.randint(0, len(vertices) - 2)
            startPoint = vertices[vid]
            endPoint = vertices[vid + 1]
            length = da.measureLine(startPoint, endPoint)
            dist = length * random.random()

            if dist > minDistance:
                d = dist / (length - dist)
                rx = (startPoint.x() + d * endPoint.x()) / (1 + d)
                ry = (startPoint.y() + d * endPoint.y()) / (1 + d)

                # generate random point
                pnt = QgsPoint(rx, ry)
                geom = QgsGeometry.fromPoint(pnt)
                if vector.checkMinDistance(pnt, index, minDistance, points):
                    f = QgsFeature(nPoints)
                    f.initAttributes(1)
                    f.setFields(fields)
                    f.setAttribute('id', nPoints)
                    f.setGeometry(geom)
                    writer.addFeature(f)
                    index.insertFeature(f)
                    points[nPoints] = pnt
                    nPoints += 1
                    progress.setPercentage(int(nPoints * total))
            nIterations += 1

        if nPoints < pointCount:
            ProcessingLog.addToLog(ProcessingLog.LOG_INFO,
                                   self.tr('Can not generate requested number of random points. '
                                           'Maximum number of attempts exceeded.'))

        del writer
