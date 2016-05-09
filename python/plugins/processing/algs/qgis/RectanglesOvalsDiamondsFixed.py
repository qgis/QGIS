# -*- coding: utf-8 -*-

"""
***************************************************************************
    RectanglesOvalsDiamondsFixed.py
    ---------------------
    Date                 : April 2016
    Copyright            : (C) 2016 by Alexander Bruy
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
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive323

__revision__ = '$Format:%H$'

import os
import math

from qgis.PyQt.QtGui import QIcon

from qgis.core import QGis, QgsFeature, QgsGeometry, QgsPoint

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.ProcessingLog import ProcessingLog
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterNumber
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector


class RectanglesOvalsDiamondsFixed(GeoAlgorithm):

    INPUT_LAYER = 'INPUT_LAYER'
    SHAPE = 'SHAPE'
    WIDTH = 'WIDTH'
    HEIGHT = 'HEIGHT'
    ROTATION = 'ROTATION'
    SEGMENTS = 'SEGMENTS'
    OUTPUT_LAYER = 'OUTPUT_LAYER'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Rectangles, ovals, diamonds (fixed)')
        self.group, self.i18n_group = self.trAlgorithm('Vector geometry tools')

        self.shapes = [self.tr('Rectangles'), self.tr('Diamonds'), self.tr('Ovals')]

        self.addParameter(ParameterVector(self.INPUT_LAYER,
                                          self.tr('Input layer'),
                                          [ParameterVector.VECTOR_TYPE_POINT]))
        self.addParameter(ParameterSelection(self.SHAPE,
                                             self.tr('Buffer shape'), self.shapes))
        self.addParameter(ParameterNumber(self.WIDTH, self.tr('Width'),
                                          0.0000001, 999999999.0, 1.0))
        self.addParameter(ParameterNumber(self.HEIGHT, self.tr('Height'),
                                          0.0000001, 999999999.0, 1.0))
        self.addParameter(ParameterNumber(self.ROTATION, self.tr('Rotation'),
                                          0.0, 360.0, optional=True))
        self.addParameter(ParameterNumber(self.SEGMENTS,
                                          self.tr('Number of segments'),
                                          1,
                                          999999999,
                                          36))

        self.addOutput(OutputVector(self.OUTPUT_LAYER,
                                    self.tr('Output')))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT_LAYER))
        shape = self.getParameterValue(self.SHAPE)
        width = self.getParameterValue(self.WIDTH)
        height = self.getParameterValue(self.HEIGHT)
        rotation = self.getParameterValue(self.ROTATION)
        segments = self.getParameterValue(self.SEGMENTS)

        writer = self.getOutputFromName(
            self.OUTPUT_LAYER).getVectorWriter(
                layer.pendingFields().toList(),
                QGis.WKBPolygon,
                layer.crs())

        outFeat = QgsFeature()

        features = vector.features(layer)
        total = 100.0 / len(features)

        if shape == 0:
            self.rectangles(writer, features, width, height, rotation)
        elif shape == 1:
            self.diamonds(writer, features, width, height, rotation)
        else:
            self.ovals(writer, features, width, height, rotation, segments)

        del writer

    def rectangles(self, writer, features, width, height, rotation):
        ft = QgsFeature()

        xOffset = width / 2.0
        yOffset = height / 2.0

        if rotation is not None:
            phi = rotation * math.pi / 180
            for current, feat in enumerate(features):
                point = feat.constGeometry().asPoint()
                x = point.x()
                y = point.y()
                points = [(-xOffset, -yOffset), (-xOffset, yOffset), (xOffset, yOffset), (xOffset, -yOffset)]
                polygon = [[QgsPoint(i[0] * math.cos(phi) + i[1] * math.sin(phi) + x,
                                     -i[0] * math.sin(phi) + i[1] * math.cos(phi) + y) for i in points]]

                ft.setGeometry(QgsGeometry.fromPolygon(polygon))
                ft.setAttributes(feat.attributes())
                writer.addFeature(ft)
        else:
            for current, feat in enumerate(features):
                point = feat.constGeometry().asPoint()
                x = point.x()
                y = point.y()
                points = [(-xOffset, -yOffset), (-xOffset, yOffset), (xOffset, yOffset), (xOffset, -yOffset)]
                polygon = [[QgsPoint(i[0] + x, i[1] + y) for i in points]]

                ft.setGeometry(QgsGeometry.fromPolygon(polygon))
                ft.setAttributes(feat.attributes())
                writer.addFeature(ft)

    def diamonds(self, writer, features, width, height, rotation):
        ft = QgsFeature()

        xOffset = width / 2.0
        yOffset = height / 2.0

        if rotation is not None:
            phi = rotation * math.pi / 180
            for current, feat in enumerate(features):
                point = feat.constGeometry().asPoint()
                x = point.x()
                y = point.y()
                points = [(0.0, -yOffset), (-xOffset, 0.0), (0.0, yOffset), (xOffset, 0.0)]
                polygon = [[QgsPoint(i[0] * math.cos(phi) + i[1] * math.sin(phi) + x,
                                     -i[0] * math.sin(phi) + i[1] * math.cos(phi) + y) for i in points]]

                ft.setGeometry(QgsGeometry.fromPolygon(polygon))
                ft.setAttributes(feat.attributes())
                writer.addFeature(ft)
        else:
            for current, feat in enumerate(features):
                point = feat.constGeometry().asPoint()
                x = point.x()
                y = point.y()
                points = [(0.0, -yOffset), (-xOffset, 0.0), (0.0, yOffset), (xOffset, 0.0)]
                polygon = [[QgsPoint(i[0] + x, i[1] + y) for i in points]]

                ft.setGeometry(QgsGeometry.fromPolygon(polygon))
                ft.setAttributes(feat.attributes())
                writer.addFeature(ft)

    def ovals(self, writer, features, width, height, rotation, segments):
        ft = QgsFeature()

        xOffset = width / 2.0
        yOffset = height / 2.0

        if rotation is not None:
            phi = rotation * math.pi / 180
            for current, feat in enumerate(features):
                point = feat.constGeometry().asPoint()
                x = point.x()
                y = point.y()
                points = []
                for t in [(2 * math.pi) / segments * i for i in xrange(segments)]:
                    points.append((xOffset * math.cos(t), yOffset * math.sin(t)))
                polygon = [[QgsPoint(i[0] * math.cos(phi) + i[1] * math.sin(phi) + x,
                                     -i[0] * math.sin(phi) + i[1] * math.cos(phi) + y) for i in points]]

                ft.setGeometry(QgsGeometry.fromPolygon(polygon))
                ft.setAttributes(feat.attributes())
                writer.addFeature(ft)
        else:
            for current, feat in enumerate(features):
                point = feat.constGeometry().asPoint()
                x = point.x()
                y = point.y()
                points = []
                for t in [(2 * math.pi) / segments * i for i in xrange(segments)]:
                    points.append((xOffset * math.cos(t), yOffset * math.sin(t)))
                polygon = [[QgsPoint(i[0] + x, i[1] + y) for i in points]]

                ft.setGeometry(QgsGeometry.fromPolygon(polygon))
                ft.setAttributes(feat.attributes())
                writer.addFeature(ft)
