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

    #def getIcon(self):
    #    return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'vector_grid.png'))

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Create grid (polygon)')
        self.group, self.i18n_group = self.trAlgorithm('Vector creation tools')

        self.types = [self.tr('Rectangle (polygon)'),
                      self.tr('Diamond (polygon)'),
                      self.tr('Hexagon (polygon)')]

        self.addParameter(ParameterSelection(self.TYPE,
                                             self.tr('Grid type'), self.types))
        self.addParameter(ParameterExtent(self.EXTENT,
                                          self.tr('Grid extent')))
        self.addParameter(ParameterNumber(self.HSPACING,
                                          self.tr('Horizontal spacing'), default=10.0))
        self.addParameter(ParameterNumber(self.VSPACING,
                                          self.tr('Vertical spacing'), default=10.0))
        self.addParameter(ParameterNumber(self.HOVERLAY,
                                          self.tr('Horizontal overlay'), default=0.0))
        self.addParameter(ParameterNumber(self.VOVERLAY,
                                          self.tr('Vertical overlay'), default=0.0))
        self.addParameter(ParameterCrs(self.CRS, 'Grid CRS'))

        self.addOutput(OutputVector(self.OUTPUT, self.tr('Grid'), datatype=[dataobjects.TYPE_VECTOR_POLYGON]))

    def processAlgorithm(self, progress):
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
                
        if hSpacing <= hOverlay or vSpacing <= vOverlay:
            raise GeoAlgorithmExecutionException(
                self.tr('Invalid overaly: %s/%s' % (hOverlay, vOverlay)))

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
                                                                     QgsWkbTypes.Polygon, crs)

        if idx == 0:
            self._rectangleGrid(
                writer, width, height, originX, originY, hSpacing, vSpacing, hOverlay, vOverlay)
        elif idx == 1:
            self._diamondGrid(
                writer, width, height, originX, originY, hSpacing, vSpacing, hOverlay, vOverlay)
        elif idx == 2:
            self._hexagonGrid(
                writer, width, height, originX, originY, hSpacing, vSpacing, hOverlay, vOverlay)

        del writer

    def _rectangleGridPoly(self, writer, width, height, originX, originY,
                           hSpacing, vSpacing, hOverlay, vOverlay):
        ft = QgsFeature()

        columns = int(math.ceil(float(width) / (hSpacing-hOverlay)))
        rows = int(math.ceil(float(height) / (vSpacing-vOverlay)))

        for col in xrange(0, columns):
            x1 = originX + (col * hSpacing - col * hOverlay)
            x2 = x1 + hSpacing
            for row in xrange(0, rows):
                y1 = originY + (row * vSpacing - row * vOverlay)
                y2 = y1 + vSpacing

                polyline = []
                polyline.append(QgsPoint(x1, y1))
                polyline.append(QgsPoint(x2, y1))
                polyline.append(QgsPoint(x2, y2))
                polyline.append(QgsPoint(x1, y2))
                polyline.append(QgsPoint(x1, y1))

                ft.setGeometry(QgsGeometry.fromPolygon([polyline]))
                ft.setAttributes([x1, y1, x2, y2])
                writer.addFeature(ft)

    def _diamond(self, xc, yc, h, w):
        x0 = xc - w
        x1 = xc 
        x2 = xc + w
        y0 = yc - h
        y1 = yc 
        y2 = yc + h
        return [QgsPoint(x0, y1), QgsPoint(x1, y2),
                QgsPoint(x2, y1), QgsPoint(x1, y0)]
        
    def _diamondGrid(self, writer, width, height, originX, originY,
                     hSpacing, vSpacing, hOverlay, vOverlay):
        ft = QgsFeature()

        halfHSpacing = hSpacing / 2.0
        halfVSpacing = vSpacing / 2.0
        ho = halfHSpacing - hOverlay/2.0
        vo = halfVSpacing - vOverlay/2.0

        columns = int(math.ceil(float(width) / ho))
        rows = int(math.ceil(float(height) / vo))

        xc = originX
        yc = originY
        
        for col in xrange(0, columns):
            if col % 2 == 0:
                yc = originY
            else:
                yc = originY + vo
            
            for row in xrange(0, rows):
                if row % 2 == 0:
                    diamond = self._diamond(xc, yc, halfVSpacing, halfHSpacing)
                    ft.setGeometry(QgsGeometry.fromPolygon([diamond]))
                    
                yc += vo
                ft.setAttributes([diamond[0].x(), diamond[3].y(),
                                  diamond[2].x(), diamond[1].y()])
                writer.addFeature(ft)
            xc += ho

            
    def _hexagon(self, xc, yc, xVLo, xVHi, h, w):
        x0 = xc - w
        x1 = xc + (xVLo - xVHi)
        x3 = xc + w
        x2 = xc - (xVLo - xVHi)
        y0 = yc - h
        y1 = yc 
        y2 = yc + h
        return [QgsPoint(x0, y1), QgsPoint(x1, y2), QgsPoint(x2, y2),
                QgsPoint(x3, y1), QgsPoint(x2, y0), QgsPoint(x1, y0)]
        
    def _hexagonGrid(self, writer, width, height, originX, originY,
                     hSpacing, vSpacing, hOverlay, vOverlay):
        ft = QgsFeature()
        
        # To preserve symmetry, hspacing is fixed relative to vspacing
        xVertexLo = 0.288675134594813 * vSpacing
        xVertexHi = 0.577350269189626 * vSpacing
        hSpacing = xVertexLo + xVertexHi
        
        halfHSpacing = hSpacing / 2.0
        halfVSpacing = vSpacing / 2.0

        vo = (xVertexHi + 0.211324865405215 * vSpacing) - vOverlay
        ho = halfHSpacing - hOverlay / 2.0
        
        columns = int(math.ceil(float(width) / vo))
        rows = int(math.ceil(float(height) / ho))
        
        xc = originX
        yc = originY
        
        for col in xrange(0, columns):
            if col % 2 == 0:
                yc = originY
            else:
                yc = originY + ho
            
            for row in xrange(0, rows):
                if row % 2 == 0:
                    hexagon = self._hexagon(xc, yc, xVertexLo, xVertexHi, halfHSpacing, halfVSpacing)
                    ft.setGeometry(QgsGeometry.fromPolygon([hexagon]))
                    
                yc += ho
                ft.setAttributes([hexagon[0].x(), hexagon[4].y(), hexagon[3].x(), hexagon[1].y()])
                writer.addFeature(ft)
            xc += vo
