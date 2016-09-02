# -*- coding: utf-8 -*-

"""
***************************************************************************
    ExtractNodes.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import math

from qgis.PyQt.QtGui import QIcon
from qgis.PyQt.QtCore import QVariant

from qgis.core import QgsFeature, QgsGeometry, QgsWkbTypes, QgsField

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class ExtractNodes(GeoAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'extract_nodes.png'))

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Extract nodes')
        self.group, self.i18n_group = self.trAlgorithm('Vector geometry tools')

        self.addParameter(ParameterVector(self.INPUT,
                                          self.tr('Input layer'),
                                          [dataobjects.TYPE_VECTOR_POLYGON,
                                           dataobjects.TYPE_VECTOR_LINE]))

        self.addOutput(OutputVector(self.OUTPUT, self.tr('Nodes'), datatype=[dataobjects.TYPE_VECTOR_POINT]))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT))

        fields = layer.fields()
        fields.append(QgsField('node_index', QVariant.Int))
        fields.append(QgsField('distance', QVariant.Double))
        fields.append(QgsField('angle', QVariant.Double))

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(
            fields, QgsWkbTypes.Point, layer.crs())

        features = vector.features(layer)
        total = 100.0 / len(features)
        for current, f in enumerate(features):
            input_geometry = f.geometry()
            if not input_geometry:
                writer.addFeature(f)
            else:
                points = vector.extractPoints(input_geometry)

                for i, point in enumerate(points):
                    distance = input_geometry.distanceToVertex(i)
                    angle = math.degrees(input_geometry.angleAtVertex(i))
                    attrs = f.attributes()
                    attrs.append(i)
                    attrs.append(distance)
                    attrs.append(angle)
                    output_feature = QgsFeature()
                    output_feature.setAttributes(attrs)
                    output_feature.setGeometry(QgsGeometry.fromPoint(point))
                    writer.addFeature(output_feature)

            progress.setPercentage(int(current * total))

        del writer
