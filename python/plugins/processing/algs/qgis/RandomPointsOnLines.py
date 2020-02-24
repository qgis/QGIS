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
from qgis.core import (QgsDistanceArea,
                       QgsFeature,
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

import time


class RandomPointsOnLines(QgisAlgorithm):

    INPUT = 'INPUT'
    POINTS_NUMBER = 'POINTS_NUMBER'
    MIN_DISTANCE = 'MIN_DISTANCE'
    OUTPUT = 'OUTPUT'
    MAXTRIESPERPOINT = 'MAX_TRIES_PER_POINT'
    OUTPUT_POINTS = 'NUMBERS_OF_POINTS_GENERATED'
    RANDOM_SEED = 'SEED'

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
                          self.tr('Number of points on each line'),
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
                          self.tr('Integer seed to use for random'),
                          QgsProcessingParameterNumber.Integer,
                          None, True, 1, 1000000000))
        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT,
                          self.tr('Random points'),
                          type=QgsProcessing.TypeVectorPoint))
        self.addOutput(QgsProcessingOutputNumber(
                         self.OUTPUT_POINTS,
                         self.tr('Number of point generated')
                       ))

    def name(self):
        return 'randompointsonlines'

    def displayName(self):
        return self.tr('Random points on lines')

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
        MaxTriesPerPoint = self.parameterAsInt(parameters,
                                               self.MAXTRIESPERPOINT, context)
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

        da = QgsDistanceArea()
        da.setSourceCrs(source.sourceCrs(), context.transformContext())
        da.setEllipsoid(context.project().ellipsoid())

        starttime = time.time()
        maxIterations = pointCount * MaxTriesPerPoint
        # Go through all the features of the layer
        for f in source.getFeatures():
            if feedback.isCanceled():
                break
            lineGeoms = []  # vector of line elements (>=1 for multi)
            #lineCount = 0
            fGeom = f.geometry()  # get the geometry
            #totLineLength = da.measureLength(fGeom)
            totLineLength = fGeom.length()
            #plaintotLineLength = fGeom.length()
            #feedback.pushInfo('da length:: ' + str(totLineLength) +
            #                  'plain length:: ' + str(plaintotLineLength))

            if fGeom.isMultipart():
                # Explode multi-part geometry
                for aLine in fGeom.asMultiPolyline():
                    lineGeoms.append(aLine)
            else:  # Single-part - just add the line geom
                lineGeoms.append(fGeom.asPolyline())

            # Generate points on the line geometry / geometries
            nPoints = 0  # number of random points generated for
                         # this (multi)line
            nIterations = 0  # number of attempts for this geometry
            while nIterations < maxIterations and nPoints < pointCount:
                # Try to generate a point
                if feedback.isCanceled():
                    break
                # Get the random "position" along the line for this point
                randomLength = random.random() * totLineLength
                feedback.pushInfo('randomLength: ' + str(randomLength))

                accLength = 0  # Accumulated line length as we move along
                               # the geometry
                # Go through the parts to find the random point location
                for l in lineGeoms:
                    # Check if the point belongs in this part of the
                    # geometry. If it does, add it and break the loop
                    if feedback.isCanceled():
                        break
                    currGeom = QgsGeometry.fromPolylineXY(l)
                    #lineLength = da.measureLength(currGeom)
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
                    #accLength += lineLength
                    # Skip to the next if we can't get far enough on this one
                    #if accLength < randomLength:
                    #    continue
                    #else:
                    #    # We are on the target part, set accLength to
                    #    # the value at the start of this part
                    #    accLength -= lineLength

                    vertices = l
                    # we are now on the correct line - find the
                    # segment for the new point and calculate the
                    # offset (remainDistance) on that segment
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
                    feedback.pushInfo('match - vid: ' + str(vid)) 
                    # We should now have found the target segment
                    # (from vertex # vid to # vid+1)
                    # Shall it be at the start point of the segment?
                    if remainDist == 0.0:
                        # A perfect hit!
                        feedback.pushInfo('**** perfect hit: ' + str(vid) +
                                          ' - len: ' + str(len(vertices)))
                        p = QgsPointXY(vertices[vid])
                    # Shall it be at the end point of the segment?
                    elif ((vid == len(vertices) - 2) and
                         (remainDist == currGeom.distanceToVertex(vid+1) -
                                        currGeom.distanceToVertex(vid))):
                        # A perfect hit at the end of this segment
                        feedback.pushInfo('******** perfect hit (end): ' +
                                          str(vid) + ' - len: ' +
                                          str(len(vertices)))
                        p = QgsPointXY(vertices[vid+1])
                        #remainDist = currGeom.distanceToVertex(vid): 
                    else:
                        # The point shall be remainDist along this segment
                        startPoint = vertices[vid]
                        endPoint = vertices[vid+1]
                        #length = da.measureLine(startPoint, endPoint)
                        length = startPoint.distance(endPoint)
                        feedback.pushInfo('length: ' + str(length))
                        feedback.pushInfo('remainDist: ' + str(remainDist))  

                        #if length == 0.0:
                        #    continue
                        #if length - remainDist == 0
                        d = remainDist / (length - remainDist)
                        feedback.pushInfo('d: ' + str(d))
                        if (1.0 + d) == 0.0:
                            feedback.reportError('Remaining dist. longer' +
                                                 ' than segment -' +
                                                 ' please report!',
                                                 False)
                            continue
                        rx = (startPoint.x() + d * endPoint.x()) / (1.0 + d)
                        ry = (startPoint.y() + d * endPoint.y()) / (1.0 + d)
                        # create the point
                        p = QgsPointXY(rx, ry)
                    # check that the point is not too close to an existing
                    # point
                    if minDistance == 0 or vector.checkMinDistance(p, index, minDistance, points):
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
            feedback.reportError('Only ' + str(totNPoints) + ' (out of ' +
                                 str(pointCount * featureCount) +
                                 ') requested points generated.',
                                 False)
        feedback.pushInfo('Time spent: ' + str(time.time() - starttime))
        return {self.OUTPUT: dest_id, self.OUTPUT_POINTS: totNPoints}
