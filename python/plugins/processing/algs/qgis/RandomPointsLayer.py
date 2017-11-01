# -*- coding: utf-8 -*-

"""
***************************************************************************
    RandomPointsLayer.py
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

import os
import random

from qgis.PyQt.QtGui import QIcon
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
                       QgsProcessing,
                       QgsProcessingException,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingParameterDefinition)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.tools import vector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class RandomPointsLayer(QgisAlgorithm):

    INPUT = 'INPUT'
    POINTS_NUMBER = 'POINTS_NUMBER'
    MIN_DISTANCE = 'MIN_DISTANCE'
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'random_points.png'))

    def group(self):
        return self.tr('Vector creation')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer'),
                                                              [QgsProcessing.TypeVectorPolygon]))
        self.addParameter(QgsProcessingParameterNumber(self.POINTS_NUMBER,
                                                       self.tr('Number of points'),
                                                       QgsProcessingParameterNumber.Integer,
                                                       1, False, 1, 1000000000))
        self.addParameter(QgsProcessingParameterNumber(self.MIN_DISTANCE,
                                                       self.tr('Minimum distance between points'),
                                                       QgsProcessingParameterNumber.Double,
                                                       0, False, 0, 1000000000))
        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT,
                                                            self.tr('Random points'),
                                                            type=QgsProcessing.TypeVectorPoint))

    def name(self):
        return 'randompointsinlayerbounds'

    def displayName(self):
        return self.tr('Random points in layer bounds')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        pointCount = self.parameterAsDouble(parameters, self.POINTS_NUMBER, context)
        minDistance = self.parameterAsDouble(parameters, self.MIN_DISTANCE, context)

        bbox = source.sourceExtent()
        sourceIndex = QgsSpatialIndex(source, feedback)

        fields = QgsFields()
        fields.append(QgsField('id', QVariant.Int, '', 10, 0))

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               fields, QgsWkbTypes.Point, source.sourceCrs())

        nPoints = 0
        nIterations = 0
        maxIterations = pointCount * 200
        total = 100.0 / pointCount if pointCount else 1

        index = QgsSpatialIndex()
        points = dict()

        random.seed()

        while nIterations < maxIterations and nPoints < pointCount:
            if feedback.isCanceled():
                break

            rx = bbox.xMinimum() + bbox.width() * random.random()
            ry = bbox.yMinimum() + bbox.height() * random.random()

            p = QgsPointXY(rx, ry)
            geom = QgsGeometry.fromPointXY(p)
            ids = sourceIndex.intersects(geom.buffer(5, 5).boundingBox())
            if len(ids) > 0 and \
                    vector.checkMinDistance(p, index, minDistance, points):
                request = QgsFeatureRequest().setFilterFids(ids).setSubsetOfAttributes([])
                for f in source.getFeatures(request):
                    if feedback.isCanceled():
                        break

                    tmpGeom = f.geometry()
                    if geom.within(tmpGeom):
                        f = QgsFeature(nPoints)
                        f.initAttributes(1)
                        f.setFields(fields)
                        f.setAttribute('id', nPoints)
                        f.setGeometry(geom)
                        sink.addFeature(f, QgsFeatureSink.FastInsert)
                        index.insertFeature(f)
                        points[nPoints] = p
                        nPoints += 1
                        feedback.setProgress(int(nPoints * total))
            nIterations += 1

        if nPoints < pointCount:
            feedback.pushInfo(self.tr('Could not generate requested number of random points. '
                                      'Maximum number of attempts exceeded.'))

        return {self.OUTPUT: dest_id}
