# -*- coding: utf-8 -*-

"""
***************************************************************************
    PointsAlongGeometry.py
    ---------------------
    Date                 : August 2016
    Copyright            : (C) 2016 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Nyall Dawson'
__date__ = 'August 2016'
__copyright__ = '(C) 2016, Nyall Dawson'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import math

from qgis.PyQt.QtCore import QVariant
from qgis.PyQt.QtGui import QIcon

from qgis.core import QgsFeature, QgsGeometry, QgsWkbTypes, QgsField

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector, ParameterNumber
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class PointsAlongGeometry(GeoAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    DISTANCE = 'DISTANCE'
    START_OFFSET = 'START_OFFSET'
    END_OFFSET = 'END_OFFSET'

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'extract_nodes.png'))

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Points along lines')
        self.group, self.i18n_group = self.trAlgorithm('Vector geometry tools')

        self.addParameter(ParameterVector(self.INPUT,
                                          self.tr('Input layer'),
                                          [dataobjects.TYPE_VECTOR_POLYGON, dataobjects.TYPE_VECTOR_LINE]))
        self.addParameter(ParameterNumber(self.DISTANCE,
                                          self.tr('Distance'), default=1.0))
        self.addParameter(ParameterNumber(self.START_OFFSET,
                                          self.tr('Start offset'), default=0.0))
        self.addParameter(ParameterNumber(self.END_OFFSET,
                                          self.tr('End offset'), default=0.0))
        self.addOutput(OutputVector(self.OUTPUT, self.tr('Points'), datatype=[dataobjects.TYPE_VECTOR_POINT]))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT))
        distance = self.getParameterValue(self.DISTANCE)
        start_offset = self.getParameterValue(self.START_OFFSET)
        end_offset = self.getParameterValue(self.END_OFFSET)

        fields = layer.fields()
        fields.append(QgsField('distance', QVariant.Double))
        fields.append(QgsField('angle', QVariant.Double))

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(
            fields, QgsWkbTypes.Point, layer.crs())

        features = vector.features(layer)
        total = 100.0 / len(features)
        for current, input_feature in enumerate(features):
            input_geometry = input_feature.geometry()
            if not input_geometry:
                writer.addFeature(input_feature)
            else:
                if input_geometry.type == QgsWkbTypes.PolygonGeometry:
                    length = input_geometry.geometry().perimeter()
                else:
                    length = input_geometry.length() - end_offset
                current_distance = start_offset

                while current_distance <= length:
                    point = input_geometry.interpolate(current_distance)
                    angle = math.degrees(input_geometry.interpolateAngle(current_distance))

                    output_feature = QgsFeature()
                    output_feature.setGeometry(point)
                    attrs = input_feature.attributes()
                    attrs.append(current_distance)
                    attrs.append(angle)
                    output_feature.setAttributes(attrs)
                    writer.addFeature(output_feature)

                    current_distance += distance

            progress.setPercentage(int(current * total))

        del writer
