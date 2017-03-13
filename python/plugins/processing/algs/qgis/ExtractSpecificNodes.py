# -*- coding: utf-8 -*-

"""
***************************************************************************
    ExtractSpecificNodes.py
    --------------------
    Date                 : October 2016
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
__date__ = 'October 2016'
__copyright__ = '(C) 2016, Nyall Dawson'

# This will get replaced with a git SHA1 when you do a git archive323

__revision__ = '$Format:%H$'

import math
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import ParameterVector, ParameterString
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector

from qgis.core import QgsWkbTypes, QgsFeature, QgsGeometry, QgsField
from qgis.PyQt.QtCore import QVariant


class ExtractSpecificNodes(GeoAlgorithm):

    INPUT_LAYER = 'INPUT_LAYER'
    OUTPUT_LAYER = 'OUTPUT_LAYER'
    NODES = 'NODES'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Extract specific nodes')
        self.group, self.i18n_group = self.trAlgorithm('Vector geometry tools')

        self.addParameter(ParameterVector(self.INPUT_LAYER,
                                          self.tr('Input layer'), [dataobjects.TYPE_VECTOR_ANY]))
        self.addParameter(ParameterString(self.NODES,
                                          self.tr('Node indices'), default='0'))
        self.addOutput(OutputVector(self.OUTPUT_LAYER, self.tr('Nodes'), datatype=[dataobjects.TYPE_VECTOR_POINT]))

    def processAlgorithm(self, feedback):
        layer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT_LAYER))

        fields = layer.fields()
        fields.append(QgsField('node_pos', QVariant.Int))
        fields.append(QgsField('node_index', QVariant.Int))
        fields.append(QgsField('distance', QVariant.Double))
        fields.append(QgsField('angle', QVariant.Double))

        writer = self.getOutputFromName(
            self.OUTPUT_LAYER).getVectorWriter(
                fields,
                QgsWkbTypes.Point,
                layer.crs())

        node_indices_string = self.getParameterValue(self.NODES)
        indices = []
        for node in node_indices_string.split(','):
            try:
                indices.append(int(node))
            except:
                raise GeoAlgorithmExecutionException(
                    self.tr('\'{}\' is not a valid node index').format(node))

        features = vector.features(layer)
        total = 100.0 / len(features)

        for current, f in enumerate(features):

            input_geometry = f.geometry()
            if not input_geometry:
                writer.addFeature(f)
            else:
                total_nodes = input_geometry.geometry().nCoordinates()

                for node in indices:
                    if node < 0:
                        node_index = total_nodes + node
                    else:
                        node_index = node

                    if node_index < 0 or node_index >= total_nodes:
                        continue

                    distance = input_geometry.distanceToVertex(node_index)
                    angle = math.degrees(input_geometry.angleAtVertex(node_index))

                    output_feature = QgsFeature()
                    attrs = f.attributes()
                    attrs.append(node)
                    attrs.append(node_index)
                    attrs.append(distance)
                    attrs.append(angle)
                    output_feature.setAttributes(attrs)

                    point = input_geometry.vertexAt(node_index)
                    output_feature.setGeometry(QgsGeometry.fromPoint(point))

                    writer.addFeature(output_feature)

            feedback.setProgress(int(current * total))

        del writer
