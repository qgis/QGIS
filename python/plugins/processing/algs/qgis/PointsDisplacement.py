# -*- coding: utf-8 -*-

"""
***************************************************************************
    PointsDisplacement.py
    ---------------------
    Date                 : July 2013
    Copyright            : (C) 2013 by Alexander Bruy
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
__date__ = 'July 2013'
__copyright__ = '(C) 2013, Alexander Bruy'

import math
from qgis.core import (QgsFeatureSink,
                       QgsGeometry,
                       QgsPointXY,
                       QgsSpatialIndex,
                       QgsRectangle,
                       QgsProcessing,
                       QgsProcessingException,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterDistance,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterFeatureSink)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class PointsDisplacement(QgisAlgorithm):
    INPUT = 'INPUT'
    DISTANCE = 'DISTANCE'
    PROXIMITY = 'PROXIMITY'
    HORIZONTAL = 'HORIZONTAL'
    OUTPUT = 'OUTPUT'

    def group(self):
        return self.tr('Vector geometry')

    def groupId(self):
        return 'vectorgeometry'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer'), [QgsProcessing.TypeVectorPoint]))
        param = QgsProcessingParameterDistance(self.PROXIMITY,
                                               self.tr('Minimum distance to other points'), parentParameterName='INPUT',
                                               minValue=0.00001, defaultValue=1.0)
        param.setMetadata({'widget_wrapper': {'decimals': 5}})
        self.addParameter(param)

        param = QgsProcessingParameterDistance(self.DISTANCE,
                                               self.tr('Displacement distance'), parentParameterName='INPUT',
                                               minValue=0.00001, defaultValue=1.0)
        param.setMetadata({'widget_wrapper': {'decimals': 5}})
        self.addParameter(param)

        self.addParameter(QgsProcessingParameterBoolean(self.HORIZONTAL,
                                                        self.tr('Horizontal distribution for two point case')))
        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Displaced'), QgsProcessing.TypeVectorPoint))

    def name(self):
        return 'pointsdisplacement'

    def displayName(self):
        return self.tr('Points displacement')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        if source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.INPUT))

        proximity = self.parameterAsDouble(parameters, self.PROXIMITY, context)
        radius = self.parameterAsDouble(parameters, self.DISTANCE, context)
        horizontal = self.parameterAsBoolean(parameters, self.HORIZONTAL, context)

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               source.fields(), source.wkbType(), source.sourceCrs())
        if sink is None:
            raise QgsProcessingException(self.invalidSinkError(parameters, self.OUTPUT))

        features = source.getFeatures()

        total = 100.0 / source.featureCount() if source.featureCount() else 0

        def searchRect(p):
            return QgsRectangle(p.x() - proximity, p.y() - proximity, p.x() + proximity, p.y() + proximity)

        index = QgsSpatialIndex()

        # NOTE: this is a Python port of QgsPointDistanceRenderer::renderFeature. If refining this algorithm,
        # please port the changes to QgsPointDistanceRenderer::renderFeature also!

        clustered_groups = []
        group_index = {}
        group_locations = {}
        for current, f in enumerate(features):
            if feedback.isCanceled():
                break

            if not f.hasGeometry():
                continue

            point = f.geometry().asPoint()

            other_features_within_radius = index.intersects(searchRect(point))
            if not other_features_within_radius:
                index.addFeature(f)
                group = [f]
                clustered_groups.append(group)
                group_index[f.id()] = len(clustered_groups) - 1
                group_locations[f.id()] = point
            else:
                # find group with closest location to this point (may be more than one within search tolerance)
                min_dist_feature_id = other_features_within_radius[0]
                min_dist = group_locations[min_dist_feature_id].distance(point)
                for i in range(1, len(other_features_within_radius)):
                    candidate_id = other_features_within_radius[i]
                    new_dist = group_locations[candidate_id].distance(point)
                    if new_dist < min_dist:
                        min_dist = new_dist
                        min_dist_feature_id = candidate_id

                group_index_pos = group_index[min_dist_feature_id]
                group = clustered_groups[group_index_pos]

                # calculate new centroid of group
                old_center = group_locations[min_dist_feature_id]
                group_locations[min_dist_feature_id] = QgsPointXY((old_center.x() * len(group) + point.x()) / (len(group) + 1.0),
                                                                  (old_center.y() * len(group) + point.y()) / (len(group) + 1.0))
                # add to a group
                clustered_groups[group_index_pos].append(f)
                group_index[f.id()] = group_index_pos

            feedback.setProgress(int(current * total))

        current = 0
        total = 100.0 / len(clustered_groups) if clustered_groups else 1
        feedback.setProgress(0)

        fullPerimeter = 2 * math.pi

        for group in clustered_groups:
            if feedback.isCanceled():
                break

            count = len(group)
            if count == 1:
                sink.addFeature(group[0], QgsFeatureSink.FastInsert)
            else:
                angleStep = fullPerimeter / count
                if count == 2 and horizontal:
                    currentAngle = math.pi / 2
                else:
                    currentAngle = 0

                old_point = group_locations[group[0].id()]

                for f in group:
                    if feedback.isCanceled():
                        break

                    sinusCurrentAngle = math.sin(currentAngle)
                    cosinusCurrentAngle = math.cos(currentAngle)
                    dx = radius * sinusCurrentAngle
                    dy = radius * cosinusCurrentAngle

                    # we want to keep any existing m/z values
                    point = f.geometry().constGet().clone()
                    point.setX(old_point.x() + dx)
                    point.setY(old_point.y() + dy)
                    f.setGeometry(QgsGeometry(point))

                    sink.addFeature(f, QgsFeatureSink.FastInsert)
                    currentAngle += angleStep

            current += 1
            feedback.setProgress(int(current * total))

        return {self.OUTPUT: dest_id}
