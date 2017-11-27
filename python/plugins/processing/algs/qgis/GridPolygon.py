# -*- coding: utf-8 -*-

"""
***************************************************************************
    GridPolygon.py
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

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import math

from qgis.PyQt.QtGui import QIcon
from qgis.PyQt.QtCore import QVariant
from qgis.core import (QgsField,
                       QgsFeatureSink,
                       QgsFeature,
                       QgsGeometry,
                       QgsPointXY,
                       QgsWkbTypes,
                       QgsProcessing,
                       QgsProcessingException,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterExtent,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterCrs,
                       QgsProcessingParameterFeatureSink,
                       QgsFields)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class GridPolygon(QgisAlgorithm):
    TYPE = 'TYPE'
    EXTENT = 'EXTENT'
    HSPACING = 'HSPACING'
    VSPACING = 'VSPACING'
    HOVERLAY = 'HOVERLAY'
    VOVERLAY = 'VOVERLAY'
    CRS = 'CRS'
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'vector_grid.png'))

    def tags(self):
        return self.tr('grid,lines,vector,create,fishnet').split(',')

    def group(self):
        return self.tr('Vector creation')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.types = [self.tr('Rectangle (polygon)'),
                      self.tr('Diamond (polygon)'),
                      self.tr('Hexagon (polygon)')]

        self.addParameter(QgsProcessingParameterEnum(self.TYPE,
                                                     self.tr('Grid type'), self.types))

        self.addParameter(QgsProcessingParameterExtent(self.EXTENT, self.tr('Grid extent')))

        self.addParameter(QgsProcessingParameterNumber(self.HSPACING,
                                                       self.tr('Horizontal spacing'), QgsProcessingParameterNumber.Double,
                                                       0.0001, False, 0, 1000000000.0))
        self.addParameter(QgsProcessingParameterNumber(self.VSPACING,
                                                       self.tr('Vertical spacing'), QgsProcessingParameterNumber.Double,
                                                       0.0001, False, 0, 1000000000.0))
        self.addParameter(QgsProcessingParameterNumber(self.HOVERLAY,
                                                       self.tr('Horizontal overlay'), QgsProcessingParameterNumber.Double,
                                                       0.0, False, 0, 1000000000.0))
        self.addParameter(QgsProcessingParameterNumber(self.VOVERLAY,
                                                       self.tr('Vertical overlay'), QgsProcessingParameterNumber.Double,
                                                       0.0, False, 0, 1000000000.0))

        self.addParameter(QgsProcessingParameterCrs(self.CRS, 'Grid CRS', 'ProjectCrs'))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Grid'), type=QgsProcessing.TypeVectorPolygon))

    def name(self):
        return 'creategridpolygon'

    def displayName(self):
        return self.tr('Create grid (polygon)')

    def processAlgorithm(self, parameters, context, feedback):
        idx = self.parameterAsEnum(parameters, self.TYPE, context)

        hSpacing = self.parameterAsDouble(parameters, self.HSPACING, context)
        vSpacing = self.parameterAsDouble(parameters, self.VSPACING, context)
        hOverlay = self.parameterAsDouble(parameters, self.HOVERLAY, context)
        vOverlay = self.parameterAsDouble(parameters, self.VOVERLAY, context)

        crs = self.parameterAsCrs(parameters, self.CRS, context)
        bbox = self.parameterAsExtent(parameters, self.EXTENT, context, crs)

        width = bbox.width()
        height = bbox.height()
        originX = bbox.xMinimum()
        originY = bbox.yMaximum()

        if hSpacing <= 0 or vSpacing <= 0:
            raise QgsProcessingException(
                self.tr('Invalid grid spacing: {0}/{1}').format(hSpacing, vSpacing))

        if width < hSpacing:
            raise QgsProcessingException(
                self.tr('Horizontal spacing is too small for the covered area'))

        if hSpacing <= hOverlay or vSpacing <= vOverlay:
            raise QgsProcessingException(
                self.tr('Invalid overlay: {0}/{1}').format(hOverlay, vOverlay))

        if height < vSpacing:
            raise QgsProcessingException(
                self.tr('Vertical spacing is too small for the covered area'))

        fields = QgsFields()
        fields.append(QgsField('left', QVariant.Double, '', 24, 16))
        fields.append(QgsField('top', QVariant.Double, '', 24, 16))
        fields.append(QgsField('right', QVariant.Double, '', 24, 16))
        fields.append(QgsField('bottom', QVariant.Double, '', 24, 16))
        fields.append(QgsField('id', QVariant.Int, '', 10, 0))

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               fields, QgsWkbTypes.Polygon, crs)

        if idx == 0:
            self._rectangleGrid(
                sink, width, height, originX, originY, hSpacing, vSpacing, hOverlay, vOverlay, feedback)
        elif idx == 1:
            self._diamondGrid(
                sink, width, height, originX, originY, hSpacing, vSpacing, hOverlay, vOverlay, feedback)
        elif idx == 2:
            self._hexagonGrid(
                sink, width, height, originX, originY, hSpacing, vSpacing, hOverlay, vOverlay, feedback)

        return {self.OUTPUT: dest_id}

    def _rectangleGrid(self, sink, width, height, originX, originY,
                       hSpacing, vSpacing, hOverlay, vOverlay, feedback):
        ft = QgsFeature()

        columns = int(math.ceil(float(width) / (hSpacing - hOverlay)))
        rows = int(math.ceil(float(height) / (vSpacing - vOverlay)))

        cells = rows * columns
        count_update = cells * 0.05

        id = 1
        count = 0

        for col in range(columns):
            if feedback.isCanceled():
                break

            x1 = originX + (col * hSpacing - col * hOverlay)
            x2 = x1 + hSpacing

            for row in range(rows):
                y1 = originY - (row * vSpacing - row * vOverlay)
                y2 = y1 - vSpacing

                polyline = []
                polyline.append(QgsPointXY(x1, y1))
                polyline.append(QgsPointXY(x2, y1))
                polyline.append(QgsPointXY(x2, y2))
                polyline.append(QgsPointXY(x1, y2))
                polyline.append(QgsPointXY(x1, y1))

                ft.setGeometry(QgsGeometry.fromPolygonXY([polyline]))
                ft.setAttributes([x1, y1, x2, y2, id])
                sink.addFeature(ft, QgsFeatureSink.FastInsert)

                id += 1
                count += 1
                if int(math.fmod(count, count_update)) == 0:
                    feedback.setProgress(int(count / cells * 100))

    def _diamondGrid(self, sink, width, height, originX, originY,
                     hSpacing, vSpacing, hOverlay, vOverlay, feedback):
        ft = QgsFeature()

        halfHSpacing = hSpacing / 2
        halfVSpacing = vSpacing / 2

        halfHOverlay = hOverlay / 2
        halfVOverlay = vOverlay / 2

        columns = int(math.ceil(float(width) / (halfHSpacing - halfHOverlay)))
        rows = int(math.ceil(float(height) / (vSpacing - halfVOverlay)))

        cells = rows * columns
        count_update = cells * 0.05

        id = 1
        count = 0

        for col in range(columns):
            if feedback.isCanceled():
                break

            x = originX - (col * halfHOverlay)
            x1 = x + ((col + 0) * halfHSpacing)
            x2 = x + ((col + 1) * halfHSpacing)
            x3 = x + ((col + 2) * halfHSpacing)

            for row in range(rows):
                y = originY + (row * halfVOverlay)
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

                ft.setGeometry(QgsGeometry.fromPolygonXY([polyline]))
                ft.setAttributes([x1, y1, x3, y3, id])
                sink.addFeature(ft, QgsFeatureSink.FastInsert)
                id += 1
                count += 1
                if int(math.fmod(count, count_update)) == 0:
                    feedback.setProgress(int(count / cells * 100))

    def _hexagonGrid(self, sink, width, height, originX, originY,
                     hSpacing, vSpacing, hOverlay, vOverlay, feedback):
        ft = QgsFeature()

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

        columns = int(math.ceil(float(width) / hOverlay))
        rows = int(math.ceil(float(height) / (vSpacing - vOverlay)))

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
            x1 = originX + (col * hOverlay)                # far left
            x2 = x1 + (xVertexHi - xVertexLo)              # left
            x3 = originX + (col * hOverlay) + hSpacing     # right
            x4 = x3 + (xVertexHi - xVertexLo)              # far right

            for row in range(rows):
                if (col % 2) == 0:
                    y1 = originY + (row * vOverlay) - (((row * 2) + 0) * halfVSpacing)  # hi
                    y2 = originY + (row * vOverlay) - (((row * 2) + 1) * halfVSpacing)  # mid
                    y3 = originY + (row * vOverlay) - (((row * 2) + 2) * halfVSpacing)  # lo
                else:
                    y1 = originY + (row * vOverlay) - (((row * 2) + 1) * halfVSpacing)  # hi
                    y2 = originY + (row * vOverlay) - (((row * 2) + 2) * halfVSpacing)  # mid
                    y3 = originY + (row * vOverlay) - (((row * 2) + 3) * halfVSpacing)  # lo

                polyline = []
                polyline.append(QgsPointXY(x1, y2))
                polyline.append(QgsPointXY(x2, y1))
                polyline.append(QgsPointXY(x3, y1))
                polyline.append(QgsPointXY(x4, y2))
                polyline.append(QgsPointXY(x3, y3))
                polyline.append(QgsPointXY(x2, y3))
                polyline.append(QgsPointXY(x1, y2))

                ft.setGeometry(QgsGeometry.fromPolygonXY([polyline]))
                ft.setAttributes([x1, y1, x4, y3, id])
                sink.addFeature(ft, QgsFeatureSink.FastInsert)
                id += 1
                count += 1
                if int(math.fmod(count, count_update)) == 0:
                    feedback.setProgress(int(count / cells * 100))
