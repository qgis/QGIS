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
from builtins import range

__author__ = 'Michael Minn'
__date__ = 'May 2010'
__copyright__ = '(C) 2010, Michael Minn'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import math

from qgis.PyQt.QtCore import QVariant
from qgis.core import QgsRectangle, QgsCoordinateReferenceSystem, Qgis, QgsField, QgsFeature, QgsGeometry, QgsPoint, QgsWkbTypes
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import ParameterExtent
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterCrs
from processing.core.outputs import OutputVector
from processing.tools import dataobjects

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class GridLine(GeoAlgorithm):
    EXTENT = 'EXTENT'
    HSPACING = 'HSPACING'
    VSPACING = 'VSPACING'
    CRS = 'CRS'
    OUTPUT = 'OUTPUT'

    #def getIcon(self):
    #    return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'vector_grid.png'))

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Create grid (lines)')
        self.group, self.i18n_group = self.trAlgorithm('Vector creation tools')

        self.addParameter(ParameterExtent(self.EXTENT,
                                          self.tr('Grid extent')))
        self.addParameter(ParameterNumber(self.HSPACING,
                                          self.tr('Horizontal spacing'), default=10.0))
        self.addParameter(ParameterNumber(self.VSPACING,
                                          self.tr('Vertical spacing'), default=10.0))
        self.addParameter(ParameterCrs(self.CRS, 'Grid CRS'))

        self.addOutput(OutputVector(self.OUTPUT, self.tr('Grid'), datatype=[dataobjects.TYPE_VECTOR_LINE]))

    def processAlgorithm(self, progress):
        extent = self.getParameterValue(self.EXTENT).split(',')
        hSpacing = self.getParameterValue(self.HSPACING)
        vSpacing = self.getParameterValue(self.VSPACING)
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

        if height < vSpacing:
            raise GeoAlgorithmExecutionException(
                self.tr('Vertical spacing is too small for the covered area'))

        fields = [QgsField('left', QVariant.Double, '', 24, 16),
                  QgsField('top', QVariant.Double, '', 24, 16),
                  QgsField('right', QVariant.Double, '', 24, 16),
                  QgsField('bottom', QVariant.Double, '', 24, 16)
                  ]

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(fields,
                                                                     QgsWkbTypes.LineString, crs)

        self._rectangleGridLine(
            writer, width, height, originX, originY, hSpacing, vSpacing)

    def _rectangleGridLine(self, writer, width, height, originX, originY,
                           hSpacing, vSpacing):
        ft = QgsFeature()

        columns = int(math.ceil(float(width) / hSpacing))
        rows = int(math.ceil(float(height) / vSpacing))

        # Longitude lines
        for col in range(columns + 1):
            polyline = []
            x = originX + (col * hSpacing)
            for row in range(rows + 1):
                y = originY - (row * vSpacing)
                polyline.append(QgsPoint(x, y))

            ft.setGeometry(QgsGeometry.fromPolyline(polyline))
            ft.setAttributes([x, originY, x, originY + (rows * vSpacing)])
            writer.addFeature(ft)

        # Latitude lines
        for row in range(rows + 1):
            polyline = []
            y = originY - (row * vSpacing)
            for col in range(columns + 1):
                x = originX + (col * hSpacing)
                polyline.append(QgsPoint(x, y))

            ft.setGeometry(QgsGeometry.fromPolyline(polyline))
            ft.setAttributes([originX, y, originX + (col * hSpacing), y])
            writer.addFeature(ft)
