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
                       QgsPointV2,
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
        self.addParameter(ParameterCrs(self.CRS, 'Grid CRS', 'EPSG:4326'))

        self.addOutput(OutputVector(self.OUTPUT, self.tr('Grid'), datatype=[dataobjects.TYPE_VECTOR_LINE]))

    def processAlgorithm(self, feedback):
        extent = self.getParameterValue(self.EXTENT).split(',')
        hSpacing = self.getParameterValue(self.HSPACING)
        vSpacing = self.getParameterValue(self.VSPACING)
        crs = QgsCoordinateReferenceSystem(self.getParameterValue(self.CRS))

        bbox = QgsRectangle(float(extent[0]), float(extent[2]),
                            float(extent[1]), float(extent[3]))

        width = bbox.width()
        height = bbox.height()

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

        # latitude lines
        count_max = height / vSpacing
        count_update = count_max * 0.10
        y = bbox.yMaximum()
        while y >= bbox.yMinimum():
            pt1 = QgsPointV2(bbox.xMinimum(), y)
            pt2 = QgsPointV2(bbox.xMaximum(), y)
            line = QgsLineString()
            line.setPoints([pt1, pt2])
            feat.setGeometry(QgsGeometry(line))
            feat.setAttributes([bbox.xMinimum(),
                                y,
                                bbox.xMaximum(),
                                y,
                                id,
                                y])
            writer.addFeature(feat)
            y = y - vSpacing
            id += 1
            count += 1
            if int(math.fmod(count, count_update)) == 0:
                feedback.setProgress(int(count / count_max * 50))

        feedback.setProgress(50)

        # longitude lines
        # counters for progressbar - update every 5%
        count = 0
        count_max = width / hSpacing
        count_update = count_max * 0.10
        x = bbox.xMinimum()
        while x <= bbox.xMaximum():
            pt1 = QgsPointV2(x, bbox.yMaximum())
            pt2 = QgsPointV2(x, bbox.yMinimum())
            line = QgsLineString()
            line.setPoints([pt1, pt2])
            feat.setGeometry(QgsGeometry(line))
            feat.setAttributes([x,
                                bbox.yMaximum(),
                                x,
                                bbox.yMinimum(),
                                id,
                                x])
            writer.addFeature(feat)
            x = x + hSpacing
            id += 1
            count += 1
            if int(math.fmod(count, count_update)) == 0:
                feedback.setProgress(50 + int(count / count_max * 50))

        del writer
