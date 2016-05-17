# -*- coding: utf-8 -*-

"""
***************************************************************************
    RandomPointsPolygonsFixed.py
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
from qgis.core import (QGis, QgsFields, QgsField, QgsDistanceArea, QgsGeometry,
                       QgsSpatialIndex, QgsPoint, QgsFeature)

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.ProcessingLog import ProcessingLog
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterSelection
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class RandomPointsPolygonsFixed(GeoAlgorithm):

    VECTOR = 'VECTOR'
    VALUE = 'VALUE'
    MIN_DISTANCE = 'MIN_DISTANCE'
    STRATEGY = 'STRATEGY'
    OUTPUT = 'OUTPUT'

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'random_points.png'))

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Random points inside polygons (fixed)')
        self.group, self.i18n_group = self.trAlgorithm('Vector creation tools')

        self.strategies = [self.tr('Points count'),
                           self.tr('Points density')]

        self.addParameter(ParameterVector(self.VECTOR,
                                          self.tr('Input layer'), [ParameterVector.VECTOR_TYPE_POLYGON]))
        self.addParameter(ParameterSelection(self.STRATEGY,
                                             self.tr('Sampling strategy'), self.strategies, 0))
        self.addParameter(ParameterNumber(self.VALUE,
                                          self.tr('Number or density of points'), 0.0001, None, 1.0))
        self.addParameter(ParameterNumber(self.MIN_DISTANCE,
                                          self.tr('Minimum distance'), 0.0, None, 0.0))

        self.addOutput(OutputVector(self.OUTPUT, self.tr('Random points')))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.VECTOR))
        value = float(self.getParameterValue(self.VALUE))
        minDistance = float(self.getParameterValue(self.MIN_DISTANCE))
        strategy = self.getParameterValue(self.STRATEGY)

        fields = QgsFields()
        fields.append(QgsField('id', QVariant.Int, '', 10, 0))
        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(
            fields, QGis.WKBPoint, layer.dataProvider().crs())

        da = QgsDistanceArea()

        features = vector.features(layer)
        for current, f in enumerate(features):
            fGeom = QgsGeometry(f.geometry())
            bbox = fGeom.boundingBox()
            if strategy == 0:
                pointCount = int(value)
            else:
                pointCount = int(round(value * da.measure(fGeom)))

            index = QgsSpatialIndex()
            points = dict()

            nPoints = 0
            nIterations = 0
            maxIterations = pointCount * 200
            total = 100.0 / pointCount

            random.seed()

            while nIterations < maxIterations and nPoints < pointCount:
                rx = bbox.xMinimum() + bbox.width() * random.random()
                ry = bbox.yMinimum() + bbox.height() * random.random()

                pnt = QgsPoint(rx, ry)
                geom = QgsGeometry.fromPoint(pnt)
                if geom.within(fGeom) and \
                        vector.checkMinDistance(pnt, index, minDistance, points):
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
                                       self.tr('Can not generate requested number of random '
                                               'points. Maximum number of attempts exceeded.'))

            progress.setPercentage(0)

        del writer
