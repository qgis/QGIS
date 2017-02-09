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
from qgis.core import QgsRectangle, QgsCoordinateReferenceSystem, Qgis, QgsField, QgsFeature, QgsGeometry, QgsPoint, QgsWkbTypes
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import ParameterExtent
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterCrs
from processing.core.outputs import OutputVector
from processing.tools import dataobjects

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class GridPolygon(GeoAlgorithm):
    TYPE = 'TYPE'
    EXTENT = 'EXTENT'
    HSPACING = 'HSPACING'
    VSPACING = 'VSPACING'
    HOVERLAY = 'HOVERLAY'
    VOVERLAY = 'VOVERLAY'
    CRS = 'CRS'
    OUTPUT = 'OUTPUT'

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'vector_grid.png'))

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Create grid (polygon)')
        self.group, self.i18n_group = self.trAlgorithm('Vector creation tools')
        self.tags = self.tr('grid,polygons,vector,create,fishnet')

        self.types = [self.tr('Rectangle (polygon)'),
                      self.tr('Diamond (polygon)'),
                      self.tr('Hexagon (polygon)')]

        self.addParameter(ParameterSelection(self.TYPE,
                                             self.tr('Grid type'), self.types))
        self.addParameter(ParameterExtent(self.EXTENT,
                                          self.tr('Grid extent'), optional=False))
        self.addParameter(ParameterNumber(self.HSPACING,
                                          self.tr('Horizontal spacing'), 0.0, 1000000000.0, 0.0001))
        self.addParameter(ParameterNumber(self.VSPACING,
                                          self.tr('Vertical spacing'), 0.0, 1000000000.0, 0.0001))
        self.addParameter(ParameterNumber(self.HOVERLAY,
                                          self.tr('Horizontal overlay'), 0.0, 1000000000.0, 0.0))
        self.addParameter(ParameterNumber(self.VOVERLAY,
                                          self.tr('Vertical overlay'), 0.0, 1000000000.0, 0.0))
        self.addParameter(ParameterCrs(self.CRS, 'Grid CRS', 'EPSG:4326'))

        self.addOutput(OutputVector(self.OUTPUT, self.tr('Grid'), datatype=[dataobjects.TYPE_VECTOR_POLYGON]))

    def processAlgorithm(self, feedback):
        idx = self.getParameterValue(self.TYPE)
        extent = self.getParameterValue(self.EXTENT).split(',')
        hSpacing = self.getParameterValue(self.HSPACING)
        vSpacing = self.getParameterValue(self.VSPACING)
        hOverlay = self.getParameterValue(self.HOVERLAY)
        vOverlay = self.getParameterValue(self.VOVERLAY)
        crs = QgsCoordinateReferenceSystem(self.getParameterValue(self.CRS))

        bbox = QgsRectangle(float(extent[0]), float(extent[2]),
                            float(extent[1]), float(extent[3]))

        width = bbox.width()
        height = bbox.height()
        originX = bbox.xMinimum()
        originY = bbox.yMaximum()

        if hSpacing <= 0 or vSpacing <= 0:
            raise GeoAlgorithmExecutionException(
                self.tr('Invalid grid spacing: %s/%s' % (hSpacing, vSpacing)))

        if width < hSpacing:
            raise GeoAlgorithmExecutionException(
                self.tr('Horizontal spacing is too small for the covered area'))

        if hSpacing <= hOverlay or vSpacing <= vOverlay:
            raise GeoAlgorithmExecutionException(
                self.tr('Invalid overlay: %s/%s' % (hOverlay, vOverlay)))

        if height < vSpacing:
            raise GeoAlgorithmExecutionException(
                self.tr('Vertical spacing is too small for the covered area'))

        fields = [QgsField('left', QVariant.Double, '', 24, 16),
                  QgsField('top', QVariant.Double, '', 24, 16),
                  QgsField('right', QVariant.Double, '', 24, 16),
                  QgsField('bottom', QVariant.Double, '', 24, 16),
                  QgsField('id', QVariant.Int, '', 10, 0)
                  ]

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(fields,
                                                                     QgsWkbTypes.Polygon, crs)

        if idx == 0:
            self._rectangleGrid(
                writer, width, height, originX, originY, hSpacing, vSpacing, hOverlay, vOverlay, feedback)
        elif idx == 1:
            self._diamondGrid(
                writer, width, height, originX, originY, hSpacing, vSpacing, hOverlay, vOverlay, feedback)
        elif idx == 2:
            self._hexagonGrid(
                writer, width, height, originX, originY, hSpacing, vSpacing, hOverlay, vOverlay, feedback)

        del writer

    def _rectangleGrid(self, writer, width, height, originX, originY,
                       hSpacing, vSpacing, hOverlay, vOverlay, feedback):
        ft = QgsFeature()

        columns = int(math.ceil(float(width) / (hSpacing - hOverlay)))
        rows = int(math.ceil(float(height) / (vSpacing - vOverlay)))

        cells = rows * columns
        count_update = cells * 0.05

        id = 1
        count = 0

        for col in range(columns):
            x1 = originX + (col * hSpacing - col * hOverlay)
            x2 = x1 + hSpacing

            for row in range(rows):
                y1 = originY - (row * vSpacing - row * vOverlay)
                y2 = y1 - vSpacing

                polyline = []
                polyline.append(QgsPoint(x1, y1))
                polyline.append(QgsPoint(x2, y1))
                polyline.append(QgsPoint(x2, y2))
                polyline.append(QgsPoint(x1, y2))
                polyline.append(QgsPoint(x1, y1))

                ft.setGeometry(QgsGeometry.fromPolygon([polyline]))
                ft.setAttributes([x1, y1, x2, y2, id])
                writer.addFeature(ft)

                id += 1
                count += 1
                if int(math.fmod(count, count_update)) == 0:
                    feedback.setProgress(int(count / cells * 100))

    def _diamondGrid(self, writer, width, height, originX, originY,
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
                polyline.append(QgsPoint(x1, y2))
                polyline.append(QgsPoint(x2, y1))
                polyline.append(QgsPoint(x3, y2))
                polyline.append(QgsPoint(x2, y3))
                polyline.append(QgsPoint(x1, y2))

                ft.setGeometry(QgsGeometry.fromPolygon([polyline]))
                ft.setAttributes([x1, y1, x3, y3, id])
                writer.addFeature(ft)
                id += 1
                count += 1
                if int(math.fmod(count, count_update)) == 0:
                    feedback.setProgress(int(count / cells * 100))

    def _hexagonGrid(self, writer, width, height, originX, originY,
                     hSpacing, vSpacing, hOverlay, vOverlay, feedback):
        ft = QgsFeature()

        # To preserve symmetry, hspacing is fixed relative to vspacing
        xVertexLo = 0.288675134594813 * vSpacing
        xVertexHi = 0.577350269189626 * vSpacing
        hSpacing = xVertexLo + xVertexHi

        hOverlay = hSpacing - hOverlay
        if hOverlay < 0:
            raise GeoAlgorithmExecutionException(
                self.tr('To preserve symmetry, hspacing is fixed relative to vspacing\n \
                        hspacing is fixed at: %s and hoverlay is fixed at: %s\n \
                        hoverlay cannot be negative. Increase hoverlay.' % (hSpacing, hOverlay)))

        halfHSpacing = hSpacing / 2.0
        halfVSpacing = vSpacing / 2.0

        columns = int(math.ceil(float(width) / hOverlay))
        rows = int(math.ceil(float(height) / (vSpacing - vOverlay)))

        cells = rows * columns
        count_update = cells * 0.05

        id = 1
        count = 0

        for col in range(columns):
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
                polyline.append(QgsPoint(x1, y2))
                polyline.append(QgsPoint(x2, y1))
                polyline.append(QgsPoint(x3, y1))
                polyline.append(QgsPoint(x4, y2))
                polyline.append(QgsPoint(x3, y3))
                polyline.append(QgsPoint(x2, y3))
                polyline.append(QgsPoint(x1, y2))

                ft.setGeometry(QgsGeometry.fromPolygon([polyline]))
                ft.setAttributes([x1, y1, x4, y3, id])
                writer.addFeature(ft)
                id += 1
                count += 1
                if int(math.fmod(count, count_update)) == 0:
                    feedback.setProgress(int(count / cells * 100))
