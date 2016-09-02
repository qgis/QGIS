# -*- coding: utf-8 -*-

"""
***************************************************************************
    VectorGridLines.py
    ---------------------
    Date                 : September 2014
    Copyright            : (C) 2014 by Alexander Bruy
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
__date__ = 'September 2014'
__copyright__ = '(C) 2014, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import math

from qgis.PyQt.QtGui import QIcon
from qgis.PyQt.QtCore import QVariant

from qgis.core import Qgis, QgsRectangle, QgsFields, QgsField, QgsFeature, QgsGeometry, QgsPoint, QgsWkbTypes
from qgis.utils import iface

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterExtent
from processing.core.parameters import ParameterNumber
from processing.core.outputs import OutputVector
from processing.tools import dataobjects

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class VectorGridLines(GeoAlgorithm):

    EXTENT = 'EXTENT'
    STEP_X = 'STEP_X'
    STEP_Y = 'STEP_Y'
    OUTPUT = 'OUTPUT'

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'vector_grid.png'))

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Vector grid (lines)')
        self.group, self.i18n_group = self.trAlgorithm('Vector creation tools')

        self.addParameter(ParameterExtent(self.EXTENT,
                                          self.tr('Grid extent')))
        self.addParameter(ParameterNumber(self.STEP_X,
                                          self.tr('X spacing'), 0.0, 1000000000.0, 0.0001))
        self.addParameter(ParameterNumber(self.STEP_Y,
                                          self.tr('Y spacing'), 0.0, 1000000000.0, 0.0001))

        self.addOutput(OutputVector(self.OUTPUT, self.tr('Grid'), datatype=[dataobjects.TYPE_VECTOR_LINE]))

    def processAlgorithm(self, progress):
        extent = self.getParameterValue(self.EXTENT).split(',')
        xSpace = self.getParameterValue(self.STEP_X)
        ySpace = self.getParameterValue(self.STEP_Y)

        bbox = QgsRectangle(float(extent[0]), float(extent[2]),
                            float(extent[1]), float(extent[3]))

        mapCRS = iface.mapCanvas().mapSettings().destinationCrs()

        fields = QgsFields()
        fields.append(QgsField('id', QVariant.Int, '', 10, 0))

        fields.append(QgsField('coord', QVariant.Double, '', 24, 15))
        fieldCount = 2
        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(
            fields, QgsWkbTypes.LineString, mapCRS)

        feat = QgsFeature()
        feat.initAttributes(fieldCount)
        feat.setFields(fields)
        geom = QgsGeometry()
        idVar = 0

        count = 0
        count_max = (bbox.yMaximum() - bbox.yMinimum()) / ySpace
        count_update = count_max * 0.10
        y = bbox.yMaximum()
        while y >= bbox.yMinimum():
            pt1 = QgsPoint(bbox.xMinimum(), y)
            pt2 = QgsPoint(bbox.xMaximum(), y)
            line = [pt1, pt2]
            feat.setGeometry(geom.fromPolyline(line))
            feat.setAttribute(0, idVar)
            feat.setAttribute(1, y)
            writer.addFeature(feat)
            y = y - ySpace
            idVar += 1
            count += 1
            if int(math.fmod(count, count_update)) == 0:
                progress.setPercentage(int(count / count_max * 50))

        progress.setPercentage(50)
        # counters for progressbar - update every 5%
        count = 0
        count_max = (bbox.xMaximum() - bbox.xMinimum()) / xSpace
        count_update = count_max * 0.10
        x = bbox.xMinimum()
        while x <= bbox.xMaximum():
            pt1 = QgsPoint(x, bbox.yMaximum())
            pt2 = QgsPoint(x, bbox.yMinimum())
            line = [pt1, pt2]
            feat.setGeometry(geom.fromPolyline(line))
            feat.setAttribute(0, idVar)
            feat.setAttribute(1, x)
            writer.addFeature(feat)
            x = x + xSpace
            idVar += 1
            count += 1
            if int(math.fmod(count, count_update)) == 0:
                progress.setPercentage(50 + int(count / count_max * 50))

        del writer
