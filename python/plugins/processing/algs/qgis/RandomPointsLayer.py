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

import random

from PyQt4.QtCore import QVariant
from qgis.core import QGis, QgsGeometry, QgsFields, QgsField, QgsSpatialIndex, QgsPoint, QgsFeature, QgsFeatureRequest

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.ProcessingLog import ProcessingLog
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterNumber
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector


class RandomPointsLayer(GeoAlgorithm):

    VECTOR = 'VECTOR'
    POINT_NUMBER = 'POINT_NUMBER'
    MIN_DISTANCE = 'MIN_DISTANCE'
    OUTPUT = 'OUTPUT'

    def defineCharacteristics(self):
        self.name = 'Random points in layer bounds'
        self.group = 'Vector creation tools'
        self.addParameter(ParameterVector(self.VECTOR,
            self.tr('Input layer'), [ParameterVector.VECTOR_TYPE_POLYGON]))
        self.addParameter(ParameterNumber(self.POINT_NUMBER,
            self.tr('Points number'), 1, 9999999, 1))
        self.addParameter(ParameterNumber(self.MIN_DISTANCE,
            self.tr('Minimum distance'), 0.0, 9999999, 0.0))

        self.addOutput(OutputVector(self.OUTPUT, self.tr('Random points')))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.VECTOR))
        pointCount = int(self.getParameterValue(self.POINT_NUMBER))
        minDistance = float(self.getParameterValue(self.MIN_DISTANCE))

        bbox = layer.extent()
        idxLayer = vector.spatialindex(layer)

        fields = QgsFields()
        fields.append(QgsField('id', QVariant.Int, '', 10, 0))
        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(
            fields, QGis.WKBPoint, layer.dataProvider().crs())

        nPoints = 0
        nIterations = 0
        maxIterations = pointCount * 200
        total = 100.0 / pointCount

        index = QgsSpatialIndex()
        points = dict()

        request = QgsFeatureRequest()

        random.seed()

        while nIterations < maxIterations and nPoints < pointCount:
            rx = bbox.xMinimum() + bbox.width() * random.random()
            ry = bbox.yMinimum() + bbox.height() * random.random()

            pnt = QgsPoint(rx, ry)
            geom = QgsGeometry.fromPoint(pnt)
            ids = idxLayer.intersects(geom.buffer(5, 5).boundingBox())
            if len(ids) > 0 and \
                    vector.checkMinDistance(pnt, index, minDistance, points):
                for i in ids:
                    f = layer.getFeatures(request.setFilterFid(i)).next()
                    tmpGeom = QgsGeometry(f.geometry())
                    if geom.within(tmpGeom):
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
