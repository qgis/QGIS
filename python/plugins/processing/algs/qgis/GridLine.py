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
                       QgsField,
                       QgsFeatureSink,
                       QgsFeature,
                       QgsGeometry,
                       QgsPoint,
                       QgsLineString,
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


class GridLine(QgisAlgorithm):
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

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Grid'), type=QgsProcessing.TypeVectorLine))

    def name(self):
        return 'creategridlines'

    def displayName(self):
        return self.tr('Create grid (lines)')

    def processAlgorithm(self, parameters, context, feedback):
        hSpacing = self.parameterAsDouble(parameters, self.HSPACING, context)
        vSpacing = self.parameterAsDouble(parameters, self.VSPACING, context)
        hOverlay = self.parameterAsDouble(parameters, self.HOVERLAY, context)
        vOverlay = self.parameterAsDouble(parameters, self.VOVERLAY, context)

        crs = self.parameterAsCrs(parameters, self.CRS, context)
        bbox = self.parameterAsExtent(parameters, self.EXTENT, context, crs)

        width = bbox.width()
        height = bbox.height()

        if hSpacing <= 0 or vSpacing <= 0:
            raise QgsProcessingException(
                self.tr('Invalid grid spacing: {0}/{1}').format(hSpacing, vSpacing))

        if hSpacing <= hOverlay or vSpacing <= vOverlay:
            raise QgsProcessingException(
                self.tr('Invalid overlay: {0}/{1}').format(hOverlay, vOverlay))

        if width < hSpacing:
            raise QgsProcessingException(
                self.tr('Horizontal spacing is too small for the covered area'))

        if height < vSpacing:
            raise QgsProcessingException(
                self.tr('Vertical spacing is too small for the covered area'))

        fields = QgsFields()
        fields.append(QgsField('left', QVariant.Double, '', 24, 16))
        fields.append(QgsField('top', QVariant.Double, '', 24, 16))
        fields.append(QgsField('right', QVariant.Double, '', 24, 16))
        fields.append(QgsField('bottom', QVariant.Double, '', 24, 16))
        fields.append(QgsField('id', QVariant.Int, '', 10, 0))
        fields.append(QgsField('coord', QVariant.Double, '', 24, 15))

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               fields, QgsWkbTypes.LineString, crs)

        if hOverlay > 0:
            hSpace = [hSpacing - hOverlay, hOverlay]
        else:
            hSpace = [hSpacing, hSpacing]

        if vOverlay > 0:
            vSpace = [vSpacing - vOverlay, vOverlay]
        else:
            vSpace = [vSpacing, vSpacing]

        feat = QgsFeature()
        feat.initAttributes(len(fields))

        count = 0
        id = 1

        # latitude lines
        count_max = height / vSpacing
        count_update = count_max * 0.10
        y = bbox.yMaximum()
        while y >= bbox.yMinimum():
            if feedback.isCanceled():
                break

            pt1 = QgsPoint(bbox.xMinimum(), y)
            pt2 = QgsPoint(bbox.xMaximum(), y)
            line = QgsLineString()
            line.setPoints([pt1, pt2])
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
        count_max = width / hSpacing
        count_update = count_max * 0.10
        x = bbox.xMinimum()
        while x <= bbox.xMaximum():
            if feedback.isCanceled():
                break

            pt1 = QgsPoint(x, bbox.yMaximum())
            pt2 = QgsPoint(x, bbox.yMinimum())
            line = QgsLineString()
            line.setPoints([pt1, pt2])
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

        return {self.OUTPUT: dest_id}
