# -*- coding: utf-8 -*-

"""
***************************************************************************
    Grid.py
    ---------------------
    Date                 : May 2010
    Copyright            : (C) 2010 by Michael Minn
    Email                : pyqgis at michaelminn dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Michael Minn'
__date__ = 'May 2010'
__copyright__ = '(C) 2010, Michael Minn'

import os
import math

from qgis.PyQt.QtGui import QIcon
from qgis.PyQt.QtCore import QVariant
from qgis.core import (QgsApplication,
                       QgsField,
                       QgsFeatureSink,
                       QgsFeature,
                       QgsGeometry,
                       QgsLineString,
                       QgsPoint,
                       QgsPointXY,
                       QgsWkbTypes,
                       QgsProcessing,
                       QgsProcessingException,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterExtent,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterDistance,
                       QgsProcessingParameterCrs,
                       QgsProcessingParameterFeatureSink,
                       QgsFields)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class Grid(QgisAlgorithm):
    TYPE = 'TYPE'
    EXTENT = 'EXTENT'
    HSPACING = 'HSPACING'
    VSPACING = 'VSPACING'
    HOVERLAY = 'HOVERLAY'
    VOVERLAY = 'VOVERLAY'
    CRS = 'CRS'
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QgsApplication.getThemeIcon("/algorithms/mAlgorithmCreateGrid.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("/algorithms/mAlgorithmCreateGrid.svg")

    def tags(self):
        return self.tr('grid,lines,polygons,vector,create,fishnet,diamond,hexagon').split(',')

    def group(self):
        return self.tr('Vector creation')

    def groupId(self):
        return 'vectorcreation'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.types = [self.tr('Point'),
                      self.tr('Line'),
                      self.tr('Rectangle (polygon)'),
                      self.tr('Diamond (polygon)'),
                      self.tr('Hexagon (polygon)')]

        self.addParameter(QgsProcessingParameterEnum(self.TYPE,
                                                     self.tr('Grid type'), self.types))

        self.addParameter(QgsProcessingParameterExtent(self.EXTENT, self.tr('Grid extent')))

        self.addParameter(QgsProcessingParameterDistance(self.HSPACING,
                                                         self.tr('Horizontal spacing'),
                                                         1.0, self.CRS, False, 0, 1000000000.0))
        self.addParameter(QgsProcessingParameterDistance(self.VSPACING,
                                                         self.tr('Vertical spacing'),
                                                         1.0, self.CRS, False, 0, 1000000000.0))
        self.addParameter(QgsProcessingParameterDistance(self.HOVERLAY,
                                                         self.tr('Horizontal overlay'),
                                                         0.0, self.CRS, False, 0, 1000000000.0))
        self.addParameter(QgsProcessingParameterDistance(self.VOVERLAY,
                                                         self.tr('Vertical overlay'),
                                                         0.0, self.CRS, False, 0, 1000000000.0))

        self.addParameter(QgsProcessingParameterCrs(self.CRS, 'Grid CRS', 'ProjectCrs'))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Grid'), type=QgsProcessing.TypeVectorPolygon))

    def name(self):
        return 'creategrid'

    def displayName(self):
        return self.tr('Create grid')

    def processAlgorithm(self, parameters, context, feedback):
        idx = self.parameterAsEnum(parameters, self.TYPE, context)

        hSpacing = self.parameterAsDouble(parameters, self.HSPACING, context)
        vSpacing = self.parameterAsDouble(parameters, self.VSPACING, context)
        hOverlay = self.parameterAsDouble(parameters, self.HOVERLAY, context)
        vOverlay = self.parameterAsDouble(parameters, self.VOVERLAY, context)

        crs = self.parameterAsCrs(parameters, self.CRS, context)
        bbox = self.parameterAsExtent(parameters, self.EXTENT, context, crs)

        if hSpacing <= 0 or vSpacing <= 0:
            raise QgsProcessingException(
                self.tr('Invalid grid spacing: {0}/{1}').format(hSpacing, vSpacing))

        if bbox.width() < hSpacing:
            raise QgsProcessingException(
                self.tr('Horizontal spacing is too large for the covered area'))

        if hSpacing <= hOverlay or vSpacing <= vOverlay:
            raise QgsProcessingException(
                self.tr('Invalid overlay: {0}/{1}').format(hOverlay, vOverlay))

        if bbox.height() < vSpacing:
            raise QgsProcessingException(
                self.tr('Vertical spacing is too large for the covered area'))

        fields = QgsFields()
        fields.append(QgsField('left', QVariant.Double, '', 24, 16))
        fields.append(QgsField('top', QVariant.Double, '', 24, 16))
        fields.append(QgsField('right', QVariant.Double, '', 24, 16))
        fields.append(QgsField('bottom', QVariant.Double, '', 24, 16))
        fields.append(QgsField('id', QVariant.Int, '', 10, 0))

        if idx == 0:
            outputWkb = QgsWkbTypes.Point
        elif idx == 1:
            outputWkb = QgsWkbTypes.LineString
        else:
            outputWkb = QgsWkbTypes.Polygon
        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               fields, outputWkb, crs)
        if sink is None:
            raise QgsProcessingException(self.invalidSinkError(parameters, self.OUTPUT))

        if idx == 0:
            self._pointGrid(
                sink, bbox, hSpacing, vSpacing, hOverlay, vOverlay, feedback)
        elif idx == 1:
            self._lineGrid(
                sink, bbox, hSpacing, vSpacing, hOverlay, vOverlay, feedback)
        elif idx == 2:
            self._rectangleGrid(
                sink, bbox, hSpacing, vSpacing, hOverlay, vOverlay, feedback)
        elif idx == 3:
            self._diamondGrid(
                sink, bbox, hSpacing, vSpacing, hOverlay, vOverlay, feedback)
        elif idx == 4:
            self._hexagonGrid(
                sink, bbox, hSpacing, vSpacing, hOverlay, vOverlay, feedback)

        return {self.OUTPUT: dest_id}

    def _pointGrid(self, sink, bbox, hSpacing, vSpacing, hOverlay, vOverlay, feedback):
        feat = QgsFeature()

        columns = int(math.ceil(float(bbox.width()) / (hSpacing - hOverlay)))
        rows = int(math.ceil(float(bbox.height()) / (vSpacing - vOverlay)))

        cells = rows * columns
        count_update = cells * 0.05

        id = 1
        count = 0

        for col in range(columns):
            for row in range(rows):
                x = bbox.xMinimum() + (col * hSpacing - col * hOverlay)
                y = bbox.yMaximum() - (row * vSpacing - row * vOverlay)
                feat.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(x, y)))
                feat.setAttributes([x, y, x + hSpacing, y + vSpacing, id])
                sink.addFeature(feat, QgsFeatureSink.FastInsert)

                id += 1
                count += 1
                if int(math.fmod(count, count_update)) == 0:
                    feedback.setProgress(int(count / cells * 100))

    def _lineGrid(self, sink, bbox, hSpacing, vSpacing, hOverlay, vOverlay, feedback):
        feat = QgsFeature()

        if hOverlay > 0:
            hSpace = [hSpacing - hOverlay, hOverlay]
        else:
            hSpace = [hSpacing, hSpacing]

        if vOverlay > 0:
            vSpace = [vSpacing - vOverlay, vOverlay]
        else:
            vSpace = [vSpacing, vSpacing]

        count = 0
        id = 1

        # latitude lines
        count_max = bbox.height() / vSpacing
        count_update = count_max * 0.10
        y = bbox.yMaximum()
        while y >= bbox.yMinimum():
            if feedback.isCanceled():
                break

            pt1 = QgsPoint(bbox.xMinimum(), y)
            pt2 = QgsPoint(bbox.xMaximum(), y)
            line = QgsLineString([pt1, pt2])
            feat.setGeometry(QgsGeometry(line))
            feat.setAttributes([bbox.xMinimum(),
                                y,
                                bbox.xMaximum(),
                                y,
                                id,
                                y])
            sink.addFeature(feat, QgsFeatureSink.FastInsert)
            y = y - vSpace[count % 2]
            id += 1
            count += 1
            if int(math.fmod(count, count_update)) == 0:
                feedback.setProgress(int(count / count_max * 50))

        feedback.setProgress(50)

        # longitude lines
        # counters for progressbar - update every 5%
        count = 0
        count_max = bbox.width() / hSpacing
        count_update = count_max * 0.10
        x = bbox.xMinimum()
        while x <= bbox.xMaximum():
            if feedback.isCanceled():
                break

            pt1 = QgsPoint(x, bbox.yMaximum())
            pt2 = QgsPoint(x, bbox.yMinimum())
            line = QgsLineString([pt1, pt2])
            feat.setGeometry(QgsGeometry(line))
            feat.setAttributes([x,
                                bbox.yMaximum(),
                                x,
                                bbox.yMinimum(),
                                id,
                                x])
            sink.addFeature(feat, QgsFeatureSink.FastInsert)
            x = x + hSpace[count % 2]
            id += 1
            count += 1
            if int(math.fmod(count, count_update)) == 0:
                feedback.setProgress(50 + int(count / count_max * 50))

    def _rectangleGrid(self, sink, bbox, hSpacing, vSpacing, hOverlay, vOverlay, feedback):
        feat = QgsFeature()

        columns = int(math.ceil(float(bbox.width()) / (hSpacing - hOverlay)))
        rows = int(math.ceil(float(bbox.height()) / (vSpacing - vOverlay)))

        cells = rows * columns
        count_update = cells * 0.05

        id = 1
        count = 0

        for col in range(columns):
            if feedback.isCanceled():
                break

            x1 = bbox.xMinimum() + (col * hSpacing - col * hOverlay)
            x2 = x1 + hSpacing

            for row in range(rows):
                y1 = bbox.yMaximum() - (row * vSpacing - row * vOverlay)
                y2 = y1 - vSpacing

                polyline = []
                polyline.append(QgsPointXY(x1, y1))
                polyline.append(QgsPointXY(x2, y1))
                polyline.append(QgsPointXY(x2, y2))
                polyline.append(QgsPointXY(x1, y2))
                polyline.append(QgsPointXY(x1, y1))

                feat.setGeometry(QgsGeometry.fromPolygonXY([polyline]))
                feat.setAttributes([x1, y1, x2, y2, id])
                sink.addFeature(feat, QgsFeatureSink.FastInsert)

                id += 1
                count += 1
                if int(math.fmod(count, count_update)) == 0:
                    feedback.setProgress(int(count / cells * 100))

    def _diamondGrid(self, sink, bbox, hSpacing, vSpacing, hOverlay, vOverlay, feedback):
        feat = QgsFeature()

        halfHSpacing = hSpacing / 2
        halfVSpacing = vSpacing / 2

        halfHOverlay = hOverlay / 2
        halfVOverlay = vOverlay / 2

        columns = int(math.ceil(float(bbox.width()) / (halfHSpacing - halfHOverlay)))
        rows = int(math.ceil(float(bbox.height()) / (vSpacing - halfVOverlay)))

        cells = rows * columns
        count_update = cells * 0.05

        id = 1
        count = 0

        for col in range(columns):
            if feedback.isCanceled():
                break

            x = bbox.xMinimum() - (col * halfHOverlay)
            x1 = x + ((col + 0) * halfHSpacing)
            x2 = x + ((col + 1) * halfHSpacing)
            x3 = x + ((col + 2) * halfHSpacing)

            for row in range(rows):
                y = bbox.yMaximum() + (row * halfVOverlay)
                if (col % 2) == 0:
                    y1 = y - (((row * 2) + 0) * halfVSpacing)
                    y2 = y - (((row * 2) + 1) * halfVSpacing)
                    y3 = y - (((row * 2) + 2) * halfVSpacing)
                else:
                    y1 = y - (((row * 2) + 1) * halfVSpacing)
                    y2 = y - (((row * 2) + 2) * halfVSpacing)
                    y3 = y - (((row * 2) + 3) * halfVSpacing)

                polyline = []
                polyline.append(QgsPointXY(x1, y2))
                polyline.append(QgsPointXY(x2, y1))
                polyline.append(QgsPointXY(x3, y2))
                polyline.append(QgsPointXY(x2, y3))
                polyline.append(QgsPointXY(x1, y2))

                feat.setGeometry(QgsGeometry.fromPolygonXY([polyline]))
                feat.setAttributes([x1, y1, x3, y3, id])
                sink.addFeature(feat, QgsFeatureSink.FastInsert)
                id += 1
                count += 1
                if int(math.fmod(count, count_update)) == 0:
                    feedback.setProgress(int(count / cells * 100))

    def _hexagonGrid(self, sink, bbox, hSpacing, vSpacing, hOverlay, vOverlay, feedback):
        feat = QgsFeature()

        # To preserve symmetry, hspacing is fixed relative to vspacing
        xVertexLo = 0.288675134594813 * vSpacing
        xVertexHi = 0.577350269189626 * vSpacing
        hSpacing = xVertexLo + xVertexHi

        hOverlay = hSpacing - hOverlay
        if hOverlay < 0:
            raise QgsProcessingException(
                self.tr('To preserve symmetry, hspacing is fixed relative to vspacing\n \
                        hspacing is fixed at: {0} and hoverlay is fixed at: {1}\n \
                        hoverlay cannot be negative. Increase hoverlay.').format(hSpacing, hOverlay)
            )

        halfVSpacing = vSpacing / 2.0

        columns = int(math.ceil(float(bbox.width()) / hOverlay))
        rows = int(math.ceil(float(bbox.height()) / (vSpacing - vOverlay)))

        cells = rows * columns
        count_update = cells * 0.05

        id = 1
        count = 0

        for col in range(columns):
            if feedback.isCanceled():
                break

            # (column + 1) and (row + 1) calculation is used to maintain
            # topology between adjacent shapes and avoid overlaps/holes
            # due to rounding errors
            x1 = bbox.xMinimum() + (col * hOverlay)                # far left
            x2 = x1 + (xVertexHi - xVertexLo)              # left
            x3 = bbox.xMinimum() + (col * hOverlay) + hSpacing     # right
            x4 = x3 + (xVertexHi - xVertexLo)              # far right

            for row in range(rows):
                if (col % 2) == 0:
                    y1 = bbox.yMaximum() + (row * vOverlay) - (((row * 2) + 0) * halfVSpacing)  # hi
                    y2 = bbox.yMaximum() + (row * vOverlay) - (((row * 2) + 1) * halfVSpacing)  # mid
                    y3 = bbox.yMaximum() + (row * vOverlay) - (((row * 2) + 2) * halfVSpacing)  # lo
                else:
                    y1 = bbox.yMaximum() + (row * vOverlay) - (((row * 2) + 1) * halfVSpacing)  # hi
                    y2 = bbox.yMaximum() + (row * vOverlay) - (((row * 2) + 2) * halfVSpacing)  # mid
                    y3 = bbox.yMaximum() + (row * vOverlay) - (((row * 2) + 3) * halfVSpacing)  # lo

                polyline = []
                polyline.append(QgsPointXY(x1, y2))
                polyline.append(QgsPointXY(x2, y1))
                polyline.append(QgsPointXY(x3, y1))
                polyline.append(QgsPointXY(x4, y2))
                polyline.append(QgsPointXY(x3, y3))
                polyline.append(QgsPointXY(x2, y3))
                polyline.append(QgsPointXY(x1, y2))

                feat.setGeometry(QgsGeometry.fromPolygonXY([polyline]))
                feat.setAttributes([x1, y1, x4, y3, id])
                sink.addFeature(feat, QgsFeatureSink.FastInsert)
                id += 1
                count += 1
                if int(math.fmod(count, count_update)) == 0:
                    feedback.setProgress(int(count / cells * 100))
