# -*- coding: utf-8 -*-

"""
***************************************************************************
    RandomPointsOnLines.py
    ----------------------------
    Date                 : February 2020
    Copyright            : (C) 2020 by Håvard Tveite
    Email                : havard dot tveite at nmbu dot no
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
    Starting point: RandomPointsAlongLines.py (C) 2014 by Alexander Bruy
"""

__author__ = 'Håvard Tveite'
__date__ = 'February 2020'
__copyright__ = '(C) 2020, Håvard Tveite'

import random

from qgis.PyQt.QtCore import QVariant
from qgis.core import (QgsFeature,
                       QgsFeatureSink,
                       QgsField,
                       QgsFields,
                       QgsGeometry,
                       QgsPointXY,
                       QgsProcessing,
                       QgsProcessingException,
                       QgsProcessingParameterDistance,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingOutputNumber,
                       QgsSpatialIndex,
                       QgsWkbTypes)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.tools import vector


class RandomPointsOnLines(QgisAlgorithm):

    INPUT = 'INPUT'
    POINTS_NUMBER = 'POINTS_NUMBER'
    MIN_DISTANCE = 'MIN_DISTANCE'
    OUTPUT = 'OUTPUT'
    MAXTRIESPERPOINT = 'MAX_TRIES_PER_POINT'
    OUTPUT_POINTS = 'NUMBERS_OF_POINTS_GENERATED'
    RANDOM_SEED = 'SEED'

    def name(self):
        return 'randompointsonlines'

    def displayName(self):
        return self.tr('Random points on lines')

    def group(self):
        return self.tr('Vector creation')

    def groupId(self):
        return 'vectorcreation'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer'),
                                                              [QgsProcessing.TypeVectorLine]))
        self.addParameter(QgsProcessingParameterNumber(self.POINTS_NUMBER,
                                                       self.tr('Number of points on each feature'),
                                                       QgsProcessingParameterNumber.Integer,
                                                       1, False, 1, 1000000000))
        self.addParameter(QgsProcessingParameterDistance(self.MIN_DISTANCE,
                                                         self.tr('Minimum distance between points'),
                                                         0, self.INPUT, False, 0, 1000000000))
        self.addParameter(QgsProcessingParameterNumber(self.MAXTRIESPERPOINT,
                                                       self.tr('Maximum number of attempts per point'),
                                                       QgsProcessingParameterNumber.Integer,
                                                       10, False, 1, 1000))
        self.addParameter(QgsProcessingParameterNumber(self.RANDOM_SEED,
                                                       self.tr('Random number seed'),
                                                       QgsProcessingParameterNumber.Integer,
                                                       None, True, 1, 1000000000))
        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT,
                                                            self.tr('Random points'),
                                                            type=QgsProcessing.TypeVectorPoint))
        self.addOutput(QgsProcessingOutputNumber(self.OUTPUT_POINTS,
                                                 self.tr('Total number of points generated')
                                                 ))

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        if source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters,
                                                                 self.INPUT))

        pointCount = self.parameterAsInt(parameters, self.POINTS_NUMBER,
                                         context)
        minDistance = self.parameterAsDouble(parameters, self.MIN_DISTANCE,
                                             context)
        randSeed = self.parameterAsInt(parameters, self.RANDOM_SEED, context)
        maxTriesPerPoint = self.parameterAsInt(parameters,
                                               self.MAXTRIESPERPOINT,
                                               context)
        fields = QgsFields()
        fields.append(QgsField('id', QVariant.Int, '', 10, 0))

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT,
                                               context, fields,
                                               QgsWkbTypes.Point,
                                               source.sourceCrs(),
                                               QgsFeatureSink.RegeneratePrimaryKey)
        if sink is None:
            raise QgsProcessingException(
                self.invalidSinkError(parameters, self.OUTPUT))

        totNPoints = 0  # The total number of points generated
        missedPoints = 0  # The number of misses (too close)
        featureCount = source.featureCount()
        total = 100.0 / (pointCount * featureCount) if pointCount else 1
        if randSeed:
            random.seed(randSeed)
        else:
            random.seed()

        index = QgsSpatialIndex()
        points = dict()

        maxIterations = pointCount * MaxTriesPerPoint
        # Go through all the features of the layer
        for f in source.getFeatures():
            if feedback.isCanceled():
                break
            lineGeoms = []  # vector of line elements (>=1 for multi)
            fGeom = f.geometry()  # get the geometry
            if fGeom is None:
                feedback.reportError('Null geometry - skipping!', False)
                continue
            if fGeom.isEmpty():
                feedback.reportError('Empty geometry - skipping!', False)
                continue
            totLineLength = fGeom.length()
            if fGeom.isMultipart():
                # Explode multi-part geometry
                for aLine in fGeom.asMultiPolyline():
                    lineGeoms.append(aLine)
            else:  # Single-part - just add the line geom
                lineGeoms.append(fGeom.asPolyline())

            # Generate points on the line geometry / geometries
            # number of random points generated for this (multi)line
            nPoints = 0
            nIterations = 0  # number of attempts for this geometry
            while nIterations < maxIterations and nPoints < pointCount:
                # Try to generate a point
                if feedback.isCanceled():
                    break
                # Get the random "position" along the line for this point
                randomLength = random.random() * totLineLength
                # To accumulated line length as we move along the geometry:
                accLength = 0
                # Go through the parts to find the random point location
                for l in lineGeoms:
                    # Check if the point belongs in this part of the
                    # geometry. If it does, add it and break the loop
                    if feedback.isCanceled():
                        break
                    currGeom = QgsGeometry.fromPolylineXY(l)
                    lineLength = currGeom.length()

                    # Skip to the next part if we can't get far enough
                    # on this one
                    if (accLength + lineLength) < randomLength:
                        accLength += lineLength
                        continue
                    # Now we know that the point belongs on this part.
                    # We have to go through the vertices to find the
                    # exact position.
                    # The point could be at the start of this part, and
                    # at the end.
                    vertices = l
                    remainDist = randomLength - accLength
                    if len(vertices) == 1:
                        feedback.reportError('Line with only one point!',
                                             False)
                        continue
                    if len(vertices) == 2:
                        # One segment only
                        vid = 0  # first vertex
                    else:
                        # More segments, find the right one
                        vid = 0  # first vertex
                        while (currGeom.distanceToVertex(vid + 1) <= remainDist
                               and vid < len(vertices) - 2):
                            # The next vertex is not far enough away...
                            vid += 1
                        remainDist = (remainDist -
                                      currGeom.distanceToVertex(vid))
                    # We should now have found the target segment
                    # (from vertex # vid to # vid+1)
                    # Shall it be at the start point of the segment?
                    if remainDist == 0.0:
                        # A perfect hit!
                        p = QgsPointXY(vertices[vid])
                    # Shall it be at the end point of the segment?
                    elif ((vid == len(vertices) - 2) and
                          (remainDist == currGeom.distanceToVertex(vid + 1) -
                           currGeom.distanceToVertex(vid))):
                        # A perfect hit at the end of this segment
                        p = QgsPointXY(vertices[vid + 1])
                    else:
                        # The point shall be remainDist along this segment
                        startPoint = vertices[vid]
                        endPoint = vertices[vid + 1]
                        length = startPoint.distance(endPoint)
                        d = remainDist / (length - remainDist)
                        if (1.0 + d) == 0.0:
                            continue
                        rx = (startPoint.x() + d * endPoint.x()) / (1.0 + d)
                        ry = (startPoint.y() + d * endPoint.y()) / (1.0 + d)
                        # create the point
                        p = QgsPointXY(rx, ry)
                    # check that the point is not too close to an existing
                    # point
                    if (minDistance == 0 or
                        vector.checkMinDistance(p, index, minDistance,
                                                points)):
                        f = QgsFeature(totNPoints)
                        f.initAttributes(1)
                        f.setFields(fields)
                        f.setAttribute('id', totNPoints)
                        f.setGeometry(QgsGeometry.fromPointXY(p))
                        sink.addFeature(f, QgsFeatureSink.FastInsert)
                        index.addFeature(f)
                        points[totNPoints] = p

                        nPoints += 1
                        totNPoints += 1
                        feedback.setProgress(int((totNPoints + missedPoints) *
                                                 total))
                        break  # Point added, so no need to continue
                nIterations += 1
            if nPoints < pointCount:
                missedPoints += pointCount - nPoints
        if totNPoints < pointCount * featureCount:
            feedback.pushInfo(str(totNPoints) + ' (out of ' +
                              str(pointCount * featureCount) +
                              ') requested points were generated.')
        return {self.OUTPUT: dest_id, self.OUTPUT_POINTS: totNPoints}
