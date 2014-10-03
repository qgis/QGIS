# -*- coding: utf-8 -*-

"""
***************************************************************************
    DeleteColumn.py
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

from PyQt4.QtCore import *
from qgis.core import *
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import \
        GeoAlgorithmExecutionException
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterCrs
from processing.core.parameters import ParameterSelection
from processing.core.outputs import OutputVector


class Grid(GeoAlgorithm):
    TYPE = 'TYPE'
    WIDTH = 'WIDTH'
    HEIGHT = 'HEIGHT'
    HSPACING = 'HSPACING'
    VSPACING = 'VSPACING'
    CENTERX = 'CENTERX'
    CENTERY = 'CENTERY'
    CRS = 'CRS'
    OUTPUT = 'OUTPUT'

    TYPES = ['Rectangle (line)',
             'Rectangle (polygon)',
             'Diamond (polygon)',
             'Hexagon (polygon)'
            ]

    def defineCharacteristics(self):
        self.name = 'Create grid'
        self.group = 'Vector creation tools'

        self.addParameter(ParameterSelection(
            self.TYPE, 'Grid type', self.TYPES))
        self.addParameter(ParameterNumber(
            self.WIDTH, 'Width', default=360.0))
        self.addParameter(ParameterNumber(
            self.HEIGHT, 'Height', default=180.0))
        self.addParameter(ParameterNumber(
            self.HSPACING, 'Horizontal spacing', default=10.0))
        self.addParameter(ParameterNumber(
            self.VSPACING, 'Vertical spacing', default=10.0))
        self.addParameter(ParameterNumber(
            self.CENTERX, 'Center X', default=0.0))
        self.addParameter(ParameterNumber(
            self.CENTERY, 'Center Y', default=0.0))
        self.addParameter(ParameterCrs(self.CRS, 'Output CRS'))

        self.addOutput(OutputVector(self.OUTPUT, 'Output'))

    def processAlgorithm(self, progress):
        idx = self.getParameterValue(self.TYPE)
        width = self.getParameterValue(self.WIDTH)
        height = self.getParameterValue(self.HEIGHT)
        hSpacing = self.getParameterValue(self.HSPACING)
        vSpacing = self.getParameterValue(self.VSPACING)
        centerX = self.getParameterValue(self.CENTERX)
        centerY = self.getParameterValue(self.CENTERY)
        originX = centerX - width / 2.0
        originY = centerY - height / 2.0
        crs = QgsCoordinateReferenceSystem(self.getParameterValue(self.CRS))

        if hSpacing <= 0 or vSpacing <= 0:
            raise GeoAlgorithmExecutionException(
                'Invalid grid spacing: %s/%s' % (hSpacing, vSpacing))

        if width < hSpacing:
            raise GeoAlgorithmExecutionException(
                'Horizontal spacing is too small for the covered area')

        if height < vSpacing:
            raise GeoAlgorithmExecutionException(
                'Vertical spacing is too small for the covered area')

        if self.TYPES[idx].find('polygon') >= 0:
            geometryType = QGis.WKBPolygon
        else:
            geometryType = QGis.WKBLineString

        fields = [QgsField('lon', QVariant.Double, '', 24, 16),
                  QgsField('lat', QVariant.Double, '', 24, 16)
                 ]

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(fields,
            geometryType, crs)

        if idx == 0:
            self._rectangleGridLine(
                writer, width, height, originX, originY, hSpacing, vSpacing)
        elif idx == 1:
            self._rectangleGridPoly(
                writer, width, height, originX, originY, hSpacing, vSpacing)
        elif idx == 2:
            self._diamondGrid(
                writer, width, height, originX, originY, hSpacing, vSpacing)
        elif idx == 3:
            self._hexagonGrid(
                writer, width, height, originX, originY, hSpacing, vSpacing)

        del writer

    def _rectangleGridLine(self, writer, width, height, originX, originY, hSpacing, vSpacing):
        ft = QgsFeature()

        x = originX
        while x <= originX + width:
            polyline = []

            y = originY
            while y <= originY + height:
                polyline.append(QgsPoint(x, y))
                y += vSpacing

            ft.setGeometry(QgsGeometry.fromPolyline(polyline))
            ft.setAttributes([x, 0])
            writer.addFeature(ft)

            x += hSpacing

        y = originY
        while y <= originY + height:
            polyline = []

            x = originX
            while x <= originX + width:
                polyline.append(QgsPoint(x, y))
                x += hSpacing

            ft.setGeometry(QgsGeometry.fromPolyline(polyline))
            ft.setAttributes([0, y])
            writer.addFeature(ft)

            y += hSpacing

    def _rectangleGridPoly(self, writer, width, height, originX, originY, hSpacing, vSpacing):
        ft = QgsFeature()

        x = originX
        while x < originX + width:
            y = originY
            while y < originY + height:
                polyline = []
                polyline.append(QgsPoint(x, y))
                polyline.append(QgsPoint(x + hSpacing, y))
                polyline.append(QgsPoint(x + hSpacing, y + vSpacing))
                polyline.append(QgsPoint(x, y + vSpacing))

                ft.setGeometry(QgsGeometry.fromPolygon([polyline]))
                ft.setAttributes([x + hSpacing / 2.0, y + vSpacing / 2.0])
                writer.addFeature(ft)
                y += vSpacing

            x += hSpacing

    def _diamondGrid(self, writer, width, height, originX, originY, hSpacing, vSpacing):
        ft = QgsFeature()

        x = originX
        colNum = 0
        while x < originX + width:
            if colNum % 2 == 0:
                y = originY
            else:
                y = originY + vSpacing / 2.0

            while y < originY + height:
                polyline = []
                polyline.append(QgsPoint(x + hSpacing / 2.0, y))
                polyline.append(QgsPoint(x + hSpacing, y + vSpacing / 2.0))
                polyline.append(QgsPoint(x + hSpacing / 2.0, y + vSpacing))
                polyline.append(QgsPoint(x, y + vSpacing / 2.0))

                ft.setGeometry(QgsGeometry.fromPolygon([polyline]))
                ft.setAttributes([x + hSpacing / 2.0, y + vSpacing / 2.0])
                writer.addFeature(ft)
                y += vSpacing

            x += hSpacing / 2.0
            colNum += 1

    def _hexagonGrid(self, writer, width, height, originX, originY, hSpacing, vSpacing):
        ft = QgsFeature()

        xVertexLo = 0.288675134594813 * vSpacing
        xVertexHi = 0.577350269189626 * vSpacing
        hSpacing = xVertexLo + xVertexHi
        x = originX + xVertexHi

        colNum = 0
        while x < originX + width:
            if colNum % 2 == 0:
                y = originY + vSpacing / 2.0
            else:
                y = originY + vSpacing

            while y < originY + height:
                polyline = []
                polyline.append(QgsPoint(x + xVertexHi, y))
                polyline.append(QgsPoint(x + xVertexLo, y + vSpacing / 2.0))
                polyline.append(QgsPoint(x - xVertexLo, y + vSpacing / 2.0))
                polyline.append(QgsPoint(x - xVertexHi, y))
                polyline.append(QgsPoint(x - xVertexLo, y - vSpacing / 2.0))
                polyline.append(QgsPoint(x + xVertexLo, y - vSpacing / 2.0))

                ft.setGeometry(QgsGeometry.fromPolygon([polyline]))
                ft.setAttributes([x, y])
                writer.addFeature(ft)
                y += vSpacing

            x += hSpacing
            colNum += 1
