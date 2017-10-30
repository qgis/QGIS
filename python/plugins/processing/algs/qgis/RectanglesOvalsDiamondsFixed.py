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
from builtins import range

__author__ = 'Alexander Bruy'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive323

__revision__ = '$Format:%H$'

import math

from qgis.core import (QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterFeatureSink,
                       QgsFeature,
                       QgsFeatureSink,
                       QgsGeometry,
                       QgsPointXY,
                       QgsWkbTypes,
                       QgsProcessing)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class RectanglesOvalsDiamondsFixed(QgisAlgorithm):

    INPUT = 'INPUT'
    SHAPE = 'SHAPE'
    WIDTH = 'WIDTH'
    HEIGHT = 'HEIGHT'
    ROTATION = 'ROTATION'
    SEGMENTS = 'SEGMENTS'
    OUTPUT = 'OUTPUT'

    def group(self):
        return self.tr('Vector geometry')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.shapes = [self.tr('Rectangles'), self.tr('Diamonds'), self.tr('Ovals')]

        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer'),
                                                              [QgsProcessing.TypeVectorPoint]))
        self.addParameter(QgsProcessingParameterEnum(self.SHAPE,
                                                     self.tr('Buffer shape'), options=self.shapes))
        self.addParameter(QgsProcessingParameterNumber(self.WIDTH, self.tr('Width'), type=QgsProcessingParameterNumber.Double,
                                                       minValue=0.0000001, maxValue=999999999.0, defaultValue=1.0))
        self.addParameter(QgsProcessingParameterNumber(self.HEIGHT, self.tr('Height'), type=QgsProcessingParameterNumber.Double,
                                                       minValue=0.0000001, maxValue=999999999.0, defaultValue=1.0))
        self.addParameter(QgsProcessingParameterNumber(self.ROTATION, self.tr('Rotation'), type=QgsProcessingParameterNumber.Double,
                                                       minValue=0.0, maxValue=360.0, optional=True))
        self.addParameter(QgsProcessingParameterNumber(self.SEGMENTS,
                                                       self.tr('Number of segments'),
                                                       minValue=1,
                                                       maxValue=999999999,
                                                       defaultValue=36))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT,
                                                            self.tr('Output'),
                                                            type=QgsProcessing.TypeVectorPolygon))

    def name(self):
        return 'rectanglesovalsdiamondsfixed'

    def displayName(self):
        return self.tr('Rectangles, ovals, diamonds (fixed)')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        shape = self.parameterAsEnum(parameters, self.SHAPE, context)
        width = self.parameterAsDouble(parameters, self.WIDTH, context)
        height = self.parameterAsDouble(parameters, self.HEIGHT, context)
        rotation = self.parameterAsDouble(parameters, self.ROTATION, context)
        segments = self.parameterAsInt(parameters, self.SEGMENTS, context)

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               source.fields(), QgsWkbTypes.Polygon, source.sourceCrs())

        if shape == 0:
            self.rectangles(sink, source, width, height, rotation, feedback)
        elif shape == 1:
            self.diamonds(sink, source, width, height, rotation, feedback)
        else:
            self.ovals(sink, source, width, height, rotation, segments, feedback)

        return {self.OUTPUT: dest_id}

    def rectangles(self, sink, source, width, height, rotation, feedback):

        features = source.getFeatures()
        ft = QgsFeature()

        xOffset = width / 2.0
        yOffset = height / 2.0

        total = 100.0 / source.featureCount() if source.featureCount() else 0

        if rotation is not None:
            phi = rotation * math.pi / 180
            for current, feat in enumerate(features):
                if feedback.isCanceled():
                    break

                if not feat.hasGeometry():
                    continue

                point = feat.geometry().asPoint()
                x = point.x()
                y = point.y()
                points = [(-xOffset, -yOffset), (-xOffset, yOffset), (xOffset, yOffset), (xOffset, -yOffset)]
                polygon = [[QgsPointXY(i[0] * math.cos(phi) + i[1] * math.sin(phi) + x,
                                       -i[0] * math.sin(phi) + i[1] * math.cos(phi) + y) for i in points]]

                ft.setGeometry(QgsGeometry.fromPolygonXY(polygon))
                ft.setAttributes(feat.attributes())
                sink.addFeature(ft, QgsFeatureSink.FastInsert)

                feedback.setProgress(int(current * total))
        else:
            for current, feat in enumerate(features):
                if feedback.isCanceled():
                    break

                if not feat.hasGeometry():
                    continue

                point = feat.geometry().asPoint()
                x = point.x()
                y = point.y()
                points = [(-xOffset, -yOffset), (-xOffset, yOffset), (xOffset, yOffset), (xOffset, -yOffset)]
                polygon = [[QgsPointXY(i[0] + x, i[1] + y) for i in points]]

                ft.setGeometry(QgsGeometry.fromPolygonXY(polygon))
                ft.setAttributes(feat.attributes())
                sink.addFeature(ft, QgsFeatureSink.FastInsert)

                feedback.setProgress(int(current * total))

    def diamonds(self, sink, source, width, height, rotation, feedback):
        features = source.getFeatures()
        ft = QgsFeature()

        xOffset = width / 2.0
        yOffset = height / 2.0

        total = 100.0 / source.featureCount() if source.featureCount() else 0
        if rotation is not None:
            phi = rotation * math.pi / 180
            for current, feat in enumerate(features):
                if feedback.isCanceled():
                    break

                if not feat.hasGeometry():
                    continue

                point = feat.geometry().asPoint()
                x = point.x()
                y = point.y()
                points = [(0.0, -yOffset), (-xOffset, 0.0), (0.0, yOffset), (xOffset, 0.0)]
                polygon = [[QgsPointXY(i[0] * math.cos(phi) + i[1] * math.sin(phi) + x,
                                       -i[0] * math.sin(phi) + i[1] * math.cos(phi) + y) for i in points]]

                ft.setGeometry(QgsGeometry.fromPolygonXY(polygon))
                ft.setAttributes(feat.attributes())
                sink.addFeature(ft, QgsFeatureSink.FastInsert)
                feedback.setProgress(int(current * total))
        else:
            for current, feat in enumerate(features):
                if feedback.isCanceled():
                    break

                if not feat.hasGeometry():
                    continue

                point = feat.geometry().asPoint()
                x = point.x()
                y = point.y()
                points = [(0.0, -yOffset), (-xOffset, 0.0), (0.0, yOffset), (xOffset, 0.0)]
                polygon = [[QgsPointXY(i[0] + x, i[1] + y) for i in points]]

                ft.setGeometry(QgsGeometry.fromPolygonXY(polygon))
                ft.setAttributes(feat.attributes())
                sink.addFeature(ft, QgsFeatureSink.FastInsert)
                feedback.setProgress(int(current * total))

    def ovals(self, sink, source, width, height, rotation, segments, feedback):
        features = source.getFeatures()
        ft = QgsFeature()

        xOffset = width / 2.0
        yOffset = height / 2.0

        total = 100.0 / source.featureCount() if source.featureCount() else 0
        if rotation is not None:
            phi = rotation * math.pi / 180
            for current, feat in enumerate(features):
                if feedback.isCanceled():
                    break

                if not feat.hasGeometry():
                    continue

                point = feat.geometry().asPoint()
                x = point.x()
                y = point.y()
                points = []
                for t in [(2 * math.pi) / segments * i for i in range(segments)]:
                    points.append((xOffset * math.cos(t), yOffset * math.sin(t)))
                polygon = [[QgsPointXY(i[0] * math.cos(phi) + i[1] * math.sin(phi) + x,
                                       -i[0] * math.sin(phi) + i[1] * math.cos(phi) + y) for i in points]]

                ft.setGeometry(QgsGeometry.fromPolygonXY(polygon))
                ft.setAttributes(feat.attributes())
                sink.addFeature(ft, QgsFeatureSink.FastInsert)
                feedback.setProgress(int(current * total))
        else:
            for current, feat in enumerate(features):
                if feedback.isCanceled():
                    break

                if not feat.hasGeometry():
                    continue

                point = feat.geometry().asPoint()
                x = point.x()
                y = point.y()
                points = []
                for t in [(2 * math.pi) / segments * i for i in range(segments)]:
                    points.append((xOffset * math.cos(t), yOffset * math.sin(t)))
                polygon = [[QgsPointXY(i[0] + x, i[1] + y) for i in points]]

                ft.setGeometry(QgsGeometry.fromPolygonXY(polygon))
                ft.setAttributes(feat.attributes())
                sink.addFeature(ft, QgsFeatureSink.FastInsert)
                feedback.setProgress(int(current * total))
