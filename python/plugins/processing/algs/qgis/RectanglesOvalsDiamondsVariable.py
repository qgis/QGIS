# -*- coding: utf-8 -*-

"""
***************************************************************************
    RectanglesOvalsDiamondsVariable.py
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

import math

from qgis.PyQt.QtCore import QCoreApplication
from qgis.core import (NULL,
                       QgsWkbTypes,
                       QgsFeature,
                       QgsFeatureSink,
                       QgsGeometry,
                       QgsPointXY,
                       QgsProcessing,
                       QgsProcessingException,
                       QgsProcessingAlgorithm,
                       QgsProcessingParameterField,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterFeatureSink)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class RectanglesOvalsDiamondsVariable(QgisAlgorithm):
    INPUT = 'INPUT'
    SHAPE = 'SHAPE'
    WIDTH = 'WIDTH'
    HEIGHT = 'HEIGHT'
    ROTATION = 'ROTATION'
    SEGMENTS = 'SEGMENTS'
    OUTPUT = 'OUTPUT'

    def group(self):
        return self.tr('Vector geometry')

    def groupId(self):
        return 'vectorgeometry'

    def __init__(self):
        super().__init__()

    def flags(self):
        return super().flags() | QgsProcessingAlgorithm.FlagDeprecated

    def initAlgorithm(self, config=None):
        self.shapes = [self.tr('Rectangles'), self.tr('Diamonds'), self.tr('Ovals')]

        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer'),
                                                              [QgsProcessing.TypeVectorPoint]))

        self.addParameter(QgsProcessingParameterEnum(self.SHAPE,
                                                     self.tr('Buffer shape'), options=self.shapes))

        self.addParameter(QgsProcessingParameterField(self.WIDTH,
                                                      self.tr('Width field'),
                                                      parentLayerParameterName=self.INPUT,
                                                      type=QgsProcessingParameterField.Numeric))
        self.addParameter(QgsProcessingParameterField(self.HEIGHT,
                                                      self.tr('Height field'),
                                                      parentLayerParameterName=self.INPUT,
                                                      type=QgsProcessingParameterField.Numeric))
        self.addParameter(QgsProcessingParameterField(self.ROTATION,
                                                      self.tr('Rotation field'),
                                                      parentLayerParameterName=self.INPUT,
                                                      type=QgsProcessingParameterField.Numeric,
                                                      optional=True))
        self.addParameter(QgsProcessingParameterNumber(self.SEGMENTS,
                                                       self.tr('Number of segments'),
                                                       minValue=1,
                                                       defaultValue=36))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT,
                                                            self.tr('Output'),
                                                            type=QgsProcessing.TypeVectorPolygon))

    def name(self):
        return 'rectanglesovalsdiamondsvariable'

    def displayName(self):
        return self.tr('Rectangles, ovals, diamonds (variable)')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        if source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.INPUT))

        shape = self.parameterAsEnum(parameters, self.SHAPE, context)

        width_field = self.parameterAsString(parameters, self.WIDTH, context)
        height_field = self.parameterAsString(parameters, self.HEIGHT, context)
        rotation_field = self.parameterAsString(parameters, self.ROTATION, context)
        segments = self.parameterAsInt(parameters, self.SEGMENTS, context)

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               source.fields(), QgsWkbTypes.Polygon, source.sourceCrs())
        if sink is None:
            raise QgsProcessingException(self.invalidSinkError(parameters, self.OUTPUT))

        width = source.fields().lookupField(width_field)
        height = source.fields().lookupField(height_field)
        rotation = source.fields().lookupField(rotation_field)
        if shape == 0:
            self.rectangles(sink, source, width, height, rotation, feedback)
        elif shape == 1:
            self.diamonds(sink, source, width, height, rotation, feedback)
        else:
            self.ovals(sink, source, width, height, rotation, segments, feedback)

        return {self.OUTPUT: dest_id}

    def rectangles(self, sink, source, width, height, rotation, feedback):
        ft = QgsFeature()
        features = source.getFeatures()

        total = 100.0 / source.featureCount() if source.featureCount() else 0

        if rotation >= 0:
            for current, feat in enumerate(features):
                if feedback.isCanceled():
                    break

                if not feat.hasGeometry():
                    continue

                w = feat[width]
                h = feat[height]
                angle = feat[rotation]
                # block 0/NULL width or height, but allow 0 as angle value
                if not w or not h:
                    feedback.pushInfo(QCoreApplication.translate('RectanglesOvalsDiamondsVariable', 'Feature {} has empty '
                                                                 'width or height. '
                                                                 'Skipping…').format(feat.id()))
                    continue
                if angle is NULL:
                    feedback.pushInfo(QCoreApplication.translate('RectanglesOvalsDiamondsVariable', 'Feature {} has empty '
                                                                 'angle. '
                                                                 'Skipping…').format(feat.id()))
                    continue

                xOffset = w / 2.0
                yOffset = h / 2.0
                phi = angle * math.pi / 180

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

                w = feat[width]
                h = feat[height]
                if not w or not h:
                    feedback.pushInfo(QCoreApplication.translate('RectanglesOvalsDiamondsVariable', 'Feature {} has empty '
                                                                 'width or height. '
                                                                 'Skipping…').format(feat.id()))
                    continue

                xOffset = w / 2.0
                yOffset = h / 2.0

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

        total = 100.0 / source.featureCount() if source.featureCount() else 0
        if rotation >= 0:
            for current, feat in enumerate(features):
                if feedback.isCanceled():
                    break

                if not feat.hasGeometry():
                    continue

                w = feat[width]
                h = feat[height]
                angle = feat[rotation]
                # block 0/NULL width or height, but allow 0 as angle value
                if not w or not h:
                    feedback.pushInfo(QCoreApplication.translate('RectanglesOvalsDiamondsVariable', 'Feature {} has empty '
                                                                 'width or height. '
                                                                 'Skipping…').format(feat.id()))
                    continue
                if angle is NULL:
                    feedback.pushInfo(QCoreApplication.translate('RectanglesOvalsDiamondsVariable', 'Feature {} has empty '
                                                                 'angle. '
                                                                 'Skipping…').format(feat.id()))
                    continue

                xOffset = w / 2.0
                yOffset = h / 2.0
                phi = angle * math.pi / 180

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

                w = feat[width]
                h = feat[height]
                if not w or not h:
                    feedback.pushInfo(QCoreApplication.translate('RectanglesOvalsDiamondsVariable', 'Feature {} has empty '
                                                                 'width or height. '
                                                                 'Skipping…').format(feat.id()))
                    continue

                xOffset = w / 2.0
                yOffset = h / 2.0

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

        total = 100.0 / source.featureCount() if source.featureCount() else 0
        if rotation >= 0:
            for current, feat in enumerate(features):
                if feedback.isCanceled():
                    break

                if not feat.hasGeometry():
                    continue

                w = feat[width]
                h = feat[height]
                angle = feat[rotation]
                # block 0/NULL width or height, but allow 0 as angle value
                if not w or not h:
                    feedback.pushInfo(QCoreApplication.translate('RectanglesOvalsDiamondsVariable', 'Feature {} has empty '
                                                                 'width or height. '
                                                                 'Skipping…').format(feat.id()))
                    continue
                if angle == NULL:
                    feedback.pushInfo(QCoreApplication.translate('RectanglesOvalsDiamondsVariable', 'Feature {} has empty '
                                                                 'angle. '
                                                                 'Skipping…').format(feat.id()))
                    continue

                xOffset = w / 2.0
                yOffset = h / 2.0
                phi = angle * math.pi / 180

                point = feat.geometry().asPoint()
                x = point.x()
                y = point.y()
                points = [
                    (xOffset * math.cos(t), yOffset * math.sin(t))
                    for t in [(2 * math.pi) / segments * i for i in range(segments)]
                ]

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

                w = feat[width]
                h = feat[height]
                if not w or not h:
                    feedback.pushInfo(QCoreApplication.translate('RectanglesOvalsDiamondsVariable', 'Feature {} has empty '
                                                                 'width or height. '
                                                                 'Skipping…').format(feat.id()))
                    continue

                xOffset = w / 2.0
                yOffset = h / 2.0

                point = feat.geometry().asPoint()
                x = point.x()
                y = point.y()
                points = [
                    (xOffset * math.cos(t), yOffset * math.sin(t))
                    for t in [(2 * math.pi) / segments * i for i in range(segments)]
                ]

                polygon = [[QgsPointXY(i[0] + x, i[1] + y) for i in points]]

                ft.setGeometry(QgsGeometry.fromPolygonXY(polygon))
                ft.setAttributes(feat.attributes())
                sink.addFeature(ft, QgsFeatureSink.FastInsert)
                feedback.setProgress(int(current * total))
