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

import random

from qgis.PyQt.QtCore import QVariant
from qgis.core import (QgsField,
                       QgsFeatureSink,
                       QgsFeature,
                       QgsFields,
                       QgsGeometry,
                       QgsPointXY,
                       QgsWkbTypes,
                       QgsSpatialIndex,
                       QgsFeatureRequest,
                       QgsDistanceArea,
                       QgsProject,
                       QgsProcessing,
                       QgsProcessingException,
                       QgsProcessingParameterDistance,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingParameterDefinition)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.tools import vector


class RandomPointsAlongLines(QgisAlgorithm):
    INPUT = 'INPUT'
    POINTS_NUMBER = 'POINTS_NUMBER'
    MIN_DISTANCE = 'MIN_DISTANCE'
    OUTPUT = 'OUTPUT'

    def group(self):
        return self.tr('Vector creation')

    def groupId(self):
        return 'vectorcreation'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer'),
                                                              [QgsProcessing.SourceType.TypeVectorLine]))
        self.addParameter(QgsProcessingParameterNumber(self.POINTS_NUMBER,
                                                       self.tr('Number of points'),
                                                       QgsProcessingParameterNumber.Type.Integer,
                                                       1, False, 1, 1000000000))
        self.addParameter(QgsProcessingParameterDistance(self.MIN_DISTANCE,
                                                         self.tr('Minimum distance between points'),
                                                         0, self.INPUT, False, 0, 1000000000))
        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT,
                                                            self.tr('Random points'),
                                                            type=QgsProcessing.SourceType.TypeVectorPoint))

    def name(self):
        return 'randompointsalongline'

    def displayName(self):
        return self.tr('Random points along line')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        if source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.INPUT))

        pointCount = self.parameterAsDouble(parameters, self.POINTS_NUMBER, context)
        minDistance = self.parameterAsDouble(parameters, self.MIN_DISTANCE, context)

        fields = QgsFields()
        fields.append(QgsField('id', QVariant.Int, '', 10, 0))

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               fields, QgsWkbTypes.Type.Point, source.sourceCrs(), QgsFeatureSink.SinkFlag.RegeneratePrimaryKey)
        if sink is None:
            raise QgsProcessingException(self.invalidSinkError(parameters, self.OUTPUT))

        nPoints = 0
        nIterations = 0
        maxIterations = pointCount * 200
        total = 100.0 / pointCount if pointCount else 1

        index = QgsSpatialIndex()
        points = {}

        da = QgsDistanceArea()
        da.setSourceCrs(source.sourceCrs(), context.transformContext())
        da.setEllipsoid(context.ellipsoid())

        request = QgsFeatureRequest()

        random.seed()

        ids = source.allFeatureIds()

        while ids and nIterations < maxIterations and nPoints < pointCount:
            if feedback.isCanceled():
                break

            # pick random feature
            fid = random.choice(ids)
            try:
                f = next(source.getFeatures(request.setFilterFid(fid).setSubsetOfAttributes([])))
            except StopIteration:
                ids.remove(fid)
                continue

            fGeom = f.geometry()
            if fGeom.isEmpty():
                ids.remove(fid)
                continue

            if fGeom.isMultipart():
                lines = fGeom.asMultiPolyline()
                # pick random line
                lineId = random.randint(0, len(lines) - 1)
                vertices = lines[lineId]
            else:
                vertices = fGeom.asPolyline()

            # pick random segment
            nVertices = len(vertices)
            if nVertices < 2:
                nIterations += 1
                continue
            if nVertices == 2:
                vid = 0
            else:
                vid = random.randint(0, nVertices - 2)
            startPoint = vertices[vid]
            endPoint = vertices[vid + 1]
            length = da.measureLine(startPoint, endPoint)
            if length == 0:
                nIterations += 1
                continue
            dist = length * random.random()

            d = dist / (length - dist)
            rx = (startPoint.x() + d * endPoint.x()) / (1 + d)
            ry = (startPoint.y() + d * endPoint.y()) / (1 + d)

            # generate random point
            p = QgsPointXY(rx, ry)
            geom = QgsGeometry.fromPointXY(p)
            if vector.checkMinDistance(p, index, minDistance, points):
                f = QgsFeature(nPoints)
                f.initAttributes(1)
                f.setFields(fields)
                f.setAttribute('id', nPoints)
                f.setGeometry(geom)
                sink.addFeature(f, QgsFeatureSink.Flag.FastInsert)
                index.addFeature(f)
                points[nPoints] = p
                nPoints += 1
                feedback.setProgress(int(nPoints * total))
            nIterations += 1

        if nPoints < pointCount:
            feedback.pushInfo(self.tr('Could not generate requested number of random points. '
                                      'Maximum number of attempts exceeded.'))

        return {self.OUTPUT: dest_id}
