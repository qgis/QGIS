# -*- coding: utf-8 -*-

"""
***************************************************************************
    ExtractSpecificVertices.py
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
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm

from qgis.core import (QgsWkbTypes,
                       QgsVertexId,
                       QgsFeature,
                       QgsFeatureSink,
                       QgsFeatureRequest,
                       QgsGeometry,
                       QgsField,
                       QgsProcessing,
                       QgsProcessingFeatureSource,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterString,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingException)
from qgis.PyQt.QtCore import QVariant


class ExtractSpecificVertices(QgisAlgorithm):
    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    VERTICES = 'VERTICES'

    def group(self):
        return self.tr('Vector geometry')

    def groupId(self):
        return 'vectorgeometry'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer'), [QgsProcessing.TypeVectorAnyGeometry]))
        self.addParameter(QgsProcessingParameterString(self.VERTICES,
                                                       self.tr('Vertex indices'), defaultValue='0'))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Vertices'), QgsProcessing.TypeVectorPoint))

    def name(self):
        return 'extractspecificvertices'

    def displayName(self):
        return self.tr('Extract specific vertices')

    def tags(self):
        return self.tr('points,vertex,nodes').split(',')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        if source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.INPUT))

        fields = source.fields()
        fields.append(QgsField('vertex_pos', QVariant.Int))
        fields.append(QgsField('vertex_index', QVariant.Int))
        fields.append(QgsField('vertex_part', QVariant.Int))
        if QgsWkbTypes.geometryType(source.wkbType()) == QgsWkbTypes.PolygonGeometry:
            fields.append(QgsField('vertex_part_ring', QVariant.Int))
        fields.append(QgsField('vertex_part_index', QVariant.Int))
        fields.append(QgsField('distance', QVariant.Double))
        fields.append(QgsField('angle', QVariant.Double))

        wkb_type = QgsWkbTypes.Point
        if QgsWkbTypes.hasM(source.wkbType()):
            wkb_type = QgsWkbTypes.addM(wkb_type)
        if QgsWkbTypes.hasZ(source.wkbType()):
            wkb_type = QgsWkbTypes.addZ(wkb_type)

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               fields, wkb_type, source.sourceCrs(), QgsFeatureSink.RegeneratePrimaryKey)
        if sink is None:
            raise QgsProcessingException(self.invalidSinkError(parameters, self.OUTPUT))

        vertex_indices_string = self.parameterAsString(parameters, self.VERTICES, context)
        indices = []
        for vertex in vertex_indices_string.split(','):
            try:
                indices.append(int(vertex))
            except:
                raise QgsProcessingException(
                    self.tr('\'{}\' is not a valid vertex index').format(vertex))

        features = source.getFeatures(QgsFeatureRequest(), QgsProcessingFeatureSource.FlagSkipGeometryValidityChecks)
        total = 100.0 / source.featureCount() if source.featureCount() else 0

        for current, f in enumerate(features):
            if feedback.isCanceled():
                break

            input_geometry = f.geometry()
            if not input_geometry:
                sink.addFeature(f, QgsFeatureSink.FastInsert)
            else:
                total_vertices = input_geometry.constGet().nCoordinates()

                for vertex in indices:
                    if vertex < 0:
                        vertex_index = total_vertices + vertex
                    else:
                        vertex_index = vertex

                    if vertex_index < 0 or vertex_index >= total_vertices:
                        continue

                    (success, vertex_id) = input_geometry.vertexIdFromVertexNr(vertex_index)

                    distance = input_geometry.distanceToVertex(vertex_index)
                    angle = math.degrees(input_geometry.angleAtVertex(vertex_index))

                    output_feature = QgsFeature()
                    attrs = f.attributes()
                    attrs.append(vertex)
                    attrs.append(vertex_index)
                    attrs.append(vertex_id.part)
                    if QgsWkbTypes.geometryType(source.wkbType()) == QgsWkbTypes.PolygonGeometry:
                        attrs.append(vertex_id.ring)
                    attrs.append(vertex_id.vertex)
                    attrs.append(distance)
                    attrs.append(angle)
                    output_feature.setAttributes(attrs)

                    point = input_geometry.vertexAt(vertex_index)
                    output_feature.setGeometry(QgsGeometry(point))

                    sink.addFeature(output_feature, QgsFeatureSink.FastInsert)

            feedback.setProgress(int(current * total))

        return {self.OUTPUT: dest_id}
