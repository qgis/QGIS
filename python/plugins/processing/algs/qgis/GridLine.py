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

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import math

from qgis.PyQt.QtGui import QIcon
from qgis.PyQt.QtCore import QVariant
from qgis.core import (QgsRectangle,
                       QgsCoordinateReferenceSystem,
                       Qgis,
                       QgsField,
                       QgsFeature,
                       QgsGeometry,
                       QgsPoint,
                       QgsLineString,
                       QgsWkbTypes)
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
    HOVERLAY = 'HOVERLAY'
    VOVERLAY = 'VOVERLAY'
    CRS = 'CRS'
    OUTPUT = 'OUTPUT'

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'vector_grid.png'))

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Create grid (lines)')
        self.group, self.i18n_group = self.trAlgorithm('Vector creation tools')
        self.tags = self.tr('grid,lines,vector,create,fishnet')

        self.addParameter(ParameterExtent(self.EXTENT,
                                          self.tr('Grid extent'), optional=False))
        self.addParameter(ParameterNumber(self.HSPACING,
                                          self.tr('Horizontal spacing'), 0.0, 1000000000.0, default=0.0001))
        self.addParameter(ParameterNumber(self.VSPACING,
                                          self.tr('Vertical spacing'), 0.0, 1000000000.0, default=0.0001))
        self.addParameter(ParameterNumber(self.HOVERLAY,
                                          self.tr('Horizontal overlay'), 0.0, 1000000000.0, default=0.0))
        self.addParameter(ParameterNumber(self.VOVERLAY,
                                          self.tr('Vertical overlay'), 0.0, 1000000000.0, default=0.0))
        self.addParameter(ParameterCrs(self.CRS, 'Grid CRS', 'EPSG:4326'))

        self.addOutput(OutputVector(self.OUTPUT, self.tr('Grid'), datatype=[dataobjects.TYPE_VECTOR_LINE]))

    def processAlgorithm(self, progress):
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

        if hSpacing <= 0 or vSpacing <= 0:
            raise GeoAlgorithmExecutionException(
                self.tr('Invalid grid spacing: %s/%s' % (hSpacing, vSpacing)))

        if hSpacing <= hOverlay or vSpacing <= vOverlay:
            raise GeoAlgorithmExecutionException(
                self.tr('Invalid overlay: %s/%s' % (hOverlay, vOverlay)))

        if width < hSpacing:
            raise GeoAlgorithmExecutionException(
                self.tr('Horizontal spacing is too small for the covered area'))

        if height < vSpacing:
            raise GeoAlgorithmExecutionException(
                self.tr('Vertical spacing is too small for the covered area'))

        fields = [QgsField('left', QVariant.Double, '', 24, 16),
                  QgsField('top', QVariant.Double, '', 24, 16),
                  QgsField('right', QVariant.Double, '', 24, 16),
                  QgsField('bottom', QVariant.Double, '', 24, 16),
                  QgsField('id', QVariant.Int, '', 10, 0),
                  QgsField('coord', QVariant.Double, '', 24, 15)
                  ]

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(fields,
                                                                     QgsWkbTypes.LineString, crs)

        feat = QgsFeature()
        feat.initAttributes(len(fields))

        count = 0
        id = 1

        originX = bbox.xMinimum()
        originY = bbox.yMinimum()

        ho = hSpacing - hOverlay
        vo = vSpacing - vOverlay

        columns = int(math.ceil(float(width) / hSpacing))
        rows = int(math.ceil(float(height) / vSpacing))

        if ((float(width) % hSpacing) != columns) or (float(width) % hSpacing == 0.0):
            columns += 1
        if ((float(height) % vSpacing) != rows) or (float(height) % vSpacing == 0.0):
            rows += 1

        if hOverlay > 0:
            columns *= 2
            columns -= 1
            hSpace = [ho, hOverlay]
        else:
            hSpace = [hSpacing, hSpacing]

        if vOverlay > 0:
            rows *= 2
            rows -= 1
            vSpace = [vo, vOverlay]
        else:
            vSpace = [vSpacing, vSpacing]

        long_polyline = []

        x = originX
        y = originY
        count_max = columns + rows
        #longitude lines
        #create longitude lines (will be extended after)
        count_update = columns * 0.10
        for col in range(columns):
            polyline = []
            polyline.append(QgsPoint(x, y))
            polyline.append(QgsPoint(x, y))

            long_polyline.append(polyline)
            x += hSpace[col % 2]

            count += 1
            if int(math.fmod(count, count_update)) == 0:
                progress.setPercentage(int(count / count_max * 25))

        progress.setPercentage(25)


        # latitude lines
        x1 = originX
        x2 = x - hSpace[col % 2]
        y = originY

        count = 0
        count_update = rows * 0.10

        for row in range(rows):
            polyline = []
            polyline.append(QgsPoint(x1, y))
            polyline.append(QgsPoint(x2, y))

            feat.setGeometry(QgsGeometry.fromPolyline(polyline))
            feat.setAttributes([x1,
                                y,
                                x2,
                                y,
                                id,
                                y])
            writer.addFeature(feat)

            y += vSpace[row % 2]

            id += 1
            count += 1
            if int(math.fmod(count, count_update)) == 0:
                progress.setPercentage(50 + int(count / count_max * 50))

        progress.setPercentage(75)

        #extend longitude lines since we know y maximum
        y2 = y - vSpace[row % 2]

        count = 0
        count_update = columns * 0.10

        for line in long_polyline:
            line[1].setY(y2)
            feat.setGeometry(QgsGeometry.fromPolyline(line))
            feat.setAttributes([line[0][0],
                                line[1][1],
                                line[1][0],
                                line[0][1],
                                id,
                                line[0][0]])
            writer.addFeature(feat)

            count += 1
            if int(math.fmod(count, count_update)) == 0:
                progress.setPercentage(50 + int(count / count_max * 50))

        del writer
