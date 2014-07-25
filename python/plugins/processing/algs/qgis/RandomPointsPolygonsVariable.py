# -*- coding: utf-8 -*-

"""
***************************************************************************
    RandomPointsPolygonsVariable.py
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

import math
import random

from PyQt4.QtCore import *

from qgis.core import *

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.ProcessingLog import ProcessingLog
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterTableField
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterSelection
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector


class RandomPointsPolygonsVariable(GeoAlgorithm):

    VECTOR = 'VECTOR'
    FIELD = 'FIELD'
    MIN_DISTANCE = 'MIN_DISTANCE'
    STRATEGY = 'STRATEGY'
    OUTPUT = 'OUTPUT'

    STRATEGIES = ['Points count',
                  'Points density'
                 ]

    def defineCharacteristics(self):
        self.name = 'Random points inside polygons (variable)'
        self.group = 'Vector creation tools'
        self.addParameter(ParameterVector(self.VECTOR,
            'Input layer',[ParameterVector.VECTOR_TYPE_POLYGON]))
        self.addParameter(ParameterSelection(
            self.STRATEGY, 'Sampling strategy', self.STRATEGIES, 0))
        self.addParameter(
            ParameterTableField(self.FIELD, 'Number field',
                self.VECTOR, ParameterTableField.DATA_TYPE_NUMBER))
        self.addParameter(ParameterNumber(
            self.MIN_DISTANCE, 'Minimum distance', 0.0, 9999999, 0.0))
        self.addOutput(OutputVector(self.OUTPUT, 'Random points'))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.VECTOR))
        fieldName = self.getParameterValue(self.FIELD)
        minDistance = float(self.getParameterValue(self.MIN_DISTANCE))
        strategy = self.getParameterValue(self.STRATEGY)

        fields = QgsFields()
        fields.append(QgsField('id', QVariant.Int, '', 10, 0))
        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(
            fields, QGis.WKBPoint, layer.dataProvider().crs())

        request = QgsFeatureRequest()
        da = QgsDistanceArea()

        features = vector.features(layer)
        for current, f in enumerate(features):
            fGeom = QgsGeometry(f.geometry())
            bbox = fGeom.boundingBox()
            if strategy == 0:
                pointCount = int(f[fieldName])
            else:
                pointCount = int(round(f[fieldName] * da.measure(fGeom)))

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
                 ProcessingLog.addToLog(
                     ProcessingLog.LOG_INFO,
                     'Can not generate requested number of random points. Maximum '
                     'number of attempts exceeded.')

            progress.setPercentage(0)

        del writer
