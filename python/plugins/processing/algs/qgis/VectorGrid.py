# -*- coding: utf-8 -*-

"""
***************************************************************************
    VectorGrid.py
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

from qgis.core import QGis, QgsRectangle, QgsFields, QgsField, QgsFeature, QgsGeometry, QgsPoint
from qgis.utils import iface

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterExtent
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterSelection
from processing.core.outputs import OutputVector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class VectorGrid(GeoAlgorithm):

    EXTENT = 'EXTENT'
    STEP_X = 'STEP_X'
    STEP_Y = 'STEP_Y'
    TYPE = 'TYPE'
    OUTPUT = 'OUTPUT'

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'vector_grid.png'))

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Vector grid')
        self.group, self.i18n_group = self.trAlgorithm('Vector creation tools')

        self.types = [self.tr('Output grid as polygons'),
                      self.tr('Output grid as lines')]

        self.addParameter(ParameterExtent(self.EXTENT,
                                          self.tr('Grid extent')))
        self.addParameter(ParameterNumber(self.STEP_X,
                                          self.tr('X spacing'), 0.0, 1000000000.0, 0.0001))
        self.addParameter(ParameterNumber(self.STEP_Y,
                                          self.tr('Y spacing'), 0.0, 1000000000.0, 0.0001))
        self.addParameter(ParameterSelection(self.TYPE,
                                             self.tr('Grid type'), self.types))

        self.addOutput(OutputVector(self.OUTPUT, self.tr('Grid')))

    def processAlgorithm(self, progress):
        extent = self.getParameterValue(self.EXTENT).split(',')
        xSpace = self.getParameterValue(self.STEP_X)
        ySpace = self.getParameterValue(self.STEP_Y)
        polygon = self.getParameterValue(self.TYPE) == 0

        bbox = QgsRectangle(float(extent[0]), float(extent[2]),
                            float(extent[1]), float(extent[3]))

        mapCRS = iface.mapCanvas().mapSettings().destinationCrs()

        fields = QgsFields()
        fields.append(QgsField('id', QVariant.Int, '', 10, 0))

        if polygon:
            fields.append(QgsField('xmin', QVariant.Double, '', 24, 15))
            fields.append(QgsField('xmax', QVariant.Double, '', 24, 15))
            fields.append(QgsField('ymin', QVariant.Double, '', 24, 15))
            fields.append(QgsField('ymax', QVariant.Double, '', 24, 15))
            fieldCount = 5
            writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(
                fields, QGis.WKBPolygon, mapCRS)
        else:
            fields.append(QgsField('coord', QVariant.Double, '', 24, 15))
            fieldCount = 2
            writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(
                fields, QGis.WKBLineString, mapCRS)

        feat = QgsFeature()
        feat.initAttributes(fieldCount)
        feat.setFields(fields)
        geom = QgsGeometry()
        idVar = 0

        if not polygon:
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
        else:
            # counters for progressbar - update every 5%
            count = 0
            count_max = (bbox.yMaximum() - bbox.yMinimum()) / ySpace
            count_update = count_max * 0.05
            y = bbox.yMaximum()
            while y >= bbox.yMinimum():
                x = bbox.xMinimum()
                while x <= bbox.xMaximum():
                    pt1 = QgsPoint(x, y)
                    pt2 = QgsPoint(x + xSpace, y)
                    pt3 = QgsPoint(x + xSpace, y - ySpace)
                    pt4 = QgsPoint(x, y - ySpace)
                    pt5 = QgsPoint(x, y)
                    polygon = [[pt1, pt2, pt3, pt4, pt5]]
                    feat.setGeometry(geom.fromPolygon(polygon))
                    feat.setAttribute(0, idVar)
                    feat.setAttribute(1, x)
                    feat.setAttribute(2, x + xSpace)
                    feat.setAttribute(3, y - ySpace)
                    feat.setAttribute(4, y)
                    writer.addFeature(feat)
                    idVar += 1
                    x = x + xSpace
                y = y - ySpace
                count += 1
                if int(math.fmod(count, count_update)) == 0:
                    progress.setPercentage(int(count / count_max * 100))

        del writer
