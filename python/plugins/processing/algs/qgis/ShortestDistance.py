# -*- coding: utf-8 -*-

"""
***************************************************************************
    ShortestDistance.py
    ---------------------
    Date                 : August 2021
    Copyright            : (C) 2021 by Matteo Ghetta
    Email                : matteo dot ghetta at faunalia dot eu
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Matteo Ghetta'
__date__ = 'August 2021'
__copyright__ = '(C) 2021, Matteo Ghetta'

from qgis.PyQt.QtCore import QCoreApplication, QVariant
from qgis.core import (QgsProcessing,
                       QgsFeatureSink,
                       QgsProcessingException,
                       QgsProcessingAlgorithm,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterDistance,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingUtils,
                       QgsSpatialIndex,
                       QgsFeature,
                       QgsField,
                       QgsWkbTypes,
                       QgsFeatureRequest,
                       QgsDistanceArea,
                       QgsProcessingParameterFeatureSink)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class ShortestLine(QgisAlgorithm):

    SOURCE = 'SOURCE'
    DESTINATION = 'DESTINATION'
    NEIGHBORS = 'NEIGHBORS'
    DISTANCE = 'DISTANCE'
    OUTPUT = 'OUTPUT'

    def __init__(self):
        super().__init__()

    def name(self):
        return 'shortestline'

    def displayName(self):
        return self.tr('Shortest Line between layers')

    def group(self):
        return self.tr('Vector analysis')

    def groupId(self):
        return 'vectoranalysis'

    def tags(self):
        return self.tr('distance,shortest,minimum').split(',')

    def initAlgorithm(self, config=None):

        self.addParameter(
            QgsProcessingParameterFeatureSource(
                self.SOURCE,
                self.tr('Source layer'),
                [QgsProcessing.TypeVectorAnyGeometry]
            )
        )

        self.addParameter(
            QgsProcessingParameterFeatureSource(
                self.DESTINATION,
                self.tr('Destination layer'),
                [QgsProcessing.TypeVectorAnyGeometry]
            )
        )

        self.addParameter(
            QgsProcessingParameterNumber(
                self.NEIGHBORS,
                self.tr('Maximum Numbers of Neighbors'),
                QgsProcessingParameterNumber.Integer,
                defaultValue=1,
                minValue=1
            )
        )

        self.addParameter(
            QgsProcessingParameterDistance(
                self.DISTANCE,
                self.tr("Maximum Distance"),
                parentParameterName=self.SOURCE,
                optional=True
            )
        )

        self.addParameter(
            QgsProcessingParameterFeatureSink(
                self.OUTPUT,
                self.tr('Shortest Line')
            )
        )

    def processAlgorithm(self, parameters, context, feedback):

        source = self.parameterAsSource(
            parameters,
            self.SOURCE,
            context
        )

        if source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.SOURCE))

        destination = self.parameterAsSource(
            parameters,
            self.DESTINATION,
            context
        )

        if destination is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.DESTINATION))

        neighbors = self.parameterAsInt(
            parameters,
            self.NEIGHBORS,
            context
        )
        if neighbors > destination.featureCount():
            neighbors = destination.featureCount()

        max_distance = self.parameterAsDouble(
            parameters,
            self.DISTANCE,
            context
        )

        max_dist = 0
        if max_distance:
            max_dist = max_distance

        fields = QgsProcessingUtils.combineFields(source.fields(), destination.fields())
        fields.append(QgsField("distance", QVariant.Double))

        (sink, dest_id) = self.parameterAsSink(
            parameters,
            self.OUTPUT,
            context,
            fields,
            QgsWkbTypes.MultiLineString,
            source.sourceCrs()
        )

        if sink is None:
            raise QgsProcessingException(self.invalidSinkError(parameters, self.OUTPUT))

        request = QgsFeatureRequest()
        request.setDestinationCrs(source.sourceCrs(), context.transformContext())

        index = QgsSpatialIndex(QgsSpatialIndex.FlagStoreFeatureGeometries)
        for i in destination.getFeatures(request):
            index.addFeature(i, QgsFeatureSink.FastInsert)

        total = 100.0 / source.featureCount() if source.featureCount() else 0

        da = QgsDistanceArea()
        da.setSourceCrs(source.sourceCrs(), context.transformContext())
        da.setEllipsoid(context.ellipsoid())

        total = 100.0 / source.featureCount() if source.featureCount() else 0

        for current, in_feature in enumerate(source.getFeatures()):

            if feedback.isCanceled():
                break

            igeom = in_feature.geometry()

            nearest_list = index.nearestNeighbor(igeom, neighbors, max_dist)
            request.setFilterFids(nearest_list)

            for dest_feature in destination.getFeatures(request):

                jgeom = dest_feature.geometry()

                shortest_line = igeom.shortestLine(jgeom)
                shortest_line_length = da.measureLength(shortest_line)

                feature = QgsFeature()

                attrs = in_feature.attributes()
                attrs.extend(dest_feature.attributes())
                attrs.append(shortest_line_length)

                feature.setAttributes(attrs)
                feature.setGeometry(shortest_line)

                sink.addFeature(feature, QgsFeatureSink.FastInsert)

            feedback.setProgress(int(current * total))

        return {self.OUTPUT: dest_id}
