# -*- coding: utf-8 -*-

"""
***************************************************************************
    MinimumBoundingGeometry.py
    --------------------------
    Date                 : September 2017
    Copyright            : (C) 2017 by Nyall Dawson
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
__date__ = 'September 2017'
__copyright__ = '(C) 2017, Nyall Dawson'

import os
import math

from qgis.PyQt.QtGui import QIcon
from qgis.PyQt.QtCore import QVariant

from qgis.core import (QgsApplication,
                       QgsField,
                       QgsFeatureSink,
                       QgsGeometry,
                       QgsWkbTypes,
                       QgsFeatureRequest,
                       QgsFields,
                       QgsRectangle,
                       QgsProcessingException,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterField,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessing,
                       QgsFeature,
                       QgsVertexId,
                       QgsMultiPoint)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class MinimumBoundingGeometry(QgisAlgorithm):
    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    TYPE = 'TYPE'
    FIELD = 'FIELD'

    def icon(self):
        return QgsApplication.getThemeIcon("/algorithms/mAlgorithmConvexHull.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("/algorithms/mAlgorithmConvexHull.svg")

    def group(self):
        return self.tr('Vector geometry')

    def groupId(self):
        return 'vectorgeometry'

    def __init__(self):
        super().__init__()
        self.type_names = [self.tr('Envelope (Bounding Box)'),
                           self.tr('Minimum Oriented Rectangle'),
                           self.tr('Minimum Enclosing Circle'),
                           self.tr('Convex Hull')]

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterField(self.FIELD,
                                                      self.tr(
                                                          'Field (optional, set if features should be grouped by class)'),
                                                      parentLayerParameterName=self.INPUT, optional=True))
        self.addParameter(QgsProcessingParameterEnum(self.TYPE,
                                                     self.tr('Geometry type'), options=self.type_names))
        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Bounding geometry'),
                                                            QgsProcessing.TypeVectorPolygon))

    def name(self):
        return 'minimumboundinggeometry'

    def displayName(self):
        return self.tr('Minimum bounding geometry')

    def tags(self):
        return self.tr(
            'bounding,box,bounds,envelope,minimum,oriented,rectangle,enclosing,circle,convex,hull,generalization').split(
            ',')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        if source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.INPUT))

        field_name = self.parameterAsString(parameters, self.FIELD, context)
        type = self.parameterAsEnum(parameters, self.TYPE, context)
        use_field = bool(field_name)

        field_index = -1

        fields = QgsFields()
        fields.append(QgsField('id', QVariant.Int, '', 20))

        if use_field:
            # keep original field type, name and parameters
            field_index = source.fields().lookupField(field_name)
            if field_index >= 0:
                fields.append(source.fields()[field_index])
        if type == 0:
            # envelope
            fields.append(QgsField('width', QVariant.Double, '', 20, 6))
            fields.append(QgsField('height', QVariant.Double, '', 20, 6))
            fields.append(QgsField('area', QVariant.Double, '', 20, 6))
            fields.append(QgsField('perimeter', QVariant.Double, '', 20, 6))
        elif type == 1:
            # oriented rect
            fields.append(QgsField('width', QVariant.Double, '', 20, 6))
            fields.append(QgsField('height', QVariant.Double, '', 20, 6))
            fields.append(QgsField('angle', QVariant.Double, '', 20, 6))
            fields.append(QgsField('area', QVariant.Double, '', 20, 6))
            fields.append(QgsField('perimeter', QVariant.Double, '', 20, 6))
        elif type == 2:
            # circle
            fields.append(QgsField('radius', QVariant.Double, '', 20, 6))
            fields.append(QgsField('area', QVariant.Double, '', 20, 6))
        elif type == 3:
            # convex hull
            fields.append(QgsField('area', QVariant.Double, '', 20, 6))
            fields.append(QgsField('perimeter', QVariant.Double, '', 20, 6))

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               fields, QgsWkbTypes.Polygon, source.sourceCrs())
        if sink is None:
            raise QgsProcessingException(self.invalidSinkError(parameters, self.OUTPUT))

        if field_index >= 0:
            geometry_dict = {}
            bounds_dict = {}
            total = 50.0 / source.featureCount() if source.featureCount() else 1
            features = source.getFeatures(QgsFeatureRequest().setSubsetOfAttributes([field_index]))
            for current, f in enumerate(features):
                if feedback.isCanceled():
                    break

                if not f.hasGeometry():
                    continue

                if type == 0:
                    # bounding boxes - calculate on the fly for efficiency
                    if f[field_index] not in bounds_dict:
                        bounds_dict[f[field_index]] = f.geometry().boundingBox()
                    else:
                        bounds_dict[f[field_index]].combineExtentWith(f.geometry().boundingBox())
                else:
                    if f[field_index] not in geometry_dict:
                        geometry_dict[f[field_index]] = [f.geometry()]
                    else:
                        geometry_dict[f[field_index]].append(f.geometry())

                feedback.setProgress(int(current * total))

            # bounding boxes
            current = 0
            if type == 0:
                total = 50.0 / len(bounds_dict) if bounds_dict else 1
                for group, rect in bounds_dict.items():
                    if feedback.isCanceled():
                        break

                    # envelope
                    feature = QgsFeature()
                    feature.setGeometry(QgsGeometry.fromRect(rect))
                    feature.setAttributes([current, group, rect.width(), rect.height(), rect.area(), rect.perimeter()])
                    sink.addFeature(feature, QgsFeatureSink.FastInsert)
                    geometry_dict[group] = None

                    feedback.setProgress(50 + int(current * total))
                    current += 1
            else:
                total = 50.0 / len(geometry_dict) if geometry_dict else 1

                for group, geometries in geometry_dict.items():
                    if feedback.isCanceled():
                        break

                    feature = self.createFeature(feedback, current, type, geometries, group)
                    sink.addFeature(feature, QgsFeatureSink.FastInsert)
                    geometry_dict[group] = None

                    feedback.setProgress(50 + int(current * total))
                    current += 1
        else:
            total = 80.0 / source.featureCount() if source.featureCount() else 1
            features = source.getFeatures(QgsFeatureRequest().setSubsetOfAttributes([]))
            geometry_queue = []
            bounds = QgsRectangle()
            for current, f in enumerate(features):
                if feedback.isCanceled():
                    break

                if not f.hasGeometry():
                    continue

                if type == 0:
                    # bounding boxes, calculate on the fly for efficiency
                    bounds.combineExtentWith(f.geometry().boundingBox())
                else:
                    geometry_queue.append(f.geometry())
                feedback.setProgress(int(current * total))

            if not feedback.isCanceled():
                if type == 0:
                    feature = QgsFeature()
                    feature.setGeometry(QgsGeometry.fromRect(bounds))
                    feature.setAttributes([0, bounds.width(), bounds.height(), bounds.area(), bounds.perimeter()])
                else:
                    feature = self.createFeature(feedback, 0, type, geometry_queue)
                sink.addFeature(feature, QgsFeatureSink.FastInsert)

        return {self.OUTPUT: dest_id}

    def createFeature(self, feedback, feature_id, type, geometries, class_field=None):
        attrs = [feature_id]
        if class_field is not None:
            attrs.append(class_field)

        multi_point = QgsMultiPoint()

        for g in geometries:
            if feedback.isCanceled():
                break

            vid = QgsVertexId()
            while True:
                if feedback.isCanceled():
                    break
                found, point = g.constGet().nextVertex(vid)
                if found:
                    multi_point.addGeometry(point)
                else:
                    break

        geometry = QgsGeometry(multi_point)
        output_geometry = None
        if type == 0:
            # envelope
            rect = geometry.boundingBox()
            output_geometry = QgsGeometry.fromRect(rect)
            attrs.append(rect.width())
            attrs.append(rect.height())
            attrs.append(rect.area())
            attrs.append(rect.perimeter())
        elif type == 1:
            # oriented rect
            output_geometry, area, angle, width, height = geometry.orientedMinimumBoundingBox()
            attrs.append(width)
            attrs.append(height)
            attrs.append(angle)
            attrs.append(area)
            attrs.append(2 * width + 2 * height)
        elif type == 2:
            # circle
            output_geometry, center, radius = geometry.minimalEnclosingCircle(segments=72)
            attrs.append(radius)
            attrs.append(math.pi * radius * radius)
        elif type == 3:
            # convex hull
            output_geometry = geometry.convexHull()
            attrs.append(output_geometry.constGet().area())
            attrs.append(output_geometry.constGet().perimeter())
        f = QgsFeature()
        f.setAttributes(attrs)
        f.setGeometry(output_geometry)
        return f
