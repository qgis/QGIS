# -*- coding: utf-8 -*-

"""
***************************************************************************
    HubDistanceLines.py
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

from qgis.PyQt.QtCore import QVariant
from qgis.core import (QgsField,
                       QgsGeometry,
                       QgsDistanceArea,
                       QgsFeature,
                       QgsFeatureSink,
                       QgsFeatureRequest,
                       QgsWkbTypes,
                       QgsUnitTypes,
                       QgsProcessing,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterField,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingException,
                       QgsSpatialIndex)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm

from math import sqrt


class HubDistanceLines(QgisAlgorithm):
    INPUT = 'INPUT'
    HUBS = 'HUBS'
    FIELD = 'FIELD'
    UNIT = 'UNIT'
    OUTPUT = 'OUTPUT'

    LAYER_UNITS = 'LAYER_UNITS'

    UNITS = [QgsUnitTypes.DistanceMeters,
             QgsUnitTypes.DistanceFeet,
             QgsUnitTypes.DistanceMiles,
             QgsUnitTypes.DistanceKilometers,
             LAYER_UNITS
             ]

    def group(self):
        return self.tr('Vector analysis')

    def groupId(self):
        return 'vectoranalysis'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.units = [self.tr('Meters'),
                      self.tr('Feet'),
                      self.tr('Miles'),
                      self.tr('Kilometers'),
                      self.tr('Layer units')]

        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Source points layer')))
        self.addParameter(QgsProcessingParameterFeatureSource(self.HUBS,
                                                              self.tr('Destination hubs layer')))
        self.addParameter(QgsProcessingParameterField(self.FIELD,
                                                      self.tr('Hub layer name attribute'), parentLayerParameterName=self.HUBS))
        self.addParameter(QgsProcessingParameterEnum(self.UNIT,
                                                     self.tr('Measurement unit'), self.units))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Hub distance'), QgsProcessing.TypeVectorLine))

    def name(self):
        return 'distancetonearesthublinetohub'

    def displayName(self):
        return self.tr('Distance to nearest hub (line to hub)')

    def processAlgorithm(self, parameters, context, feedback):
        if parameters[self.INPUT] == parameters[self.HUBS]:
            raise QgsProcessingException(
                self.tr('Same layer given for both hubs and spokes'))

        point_source = self.parameterAsSource(parameters, self.INPUT, context)
        if point_source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.INPUT))

        hub_source = self.parameterAsSource(parameters, self.HUBS, context)
        if hub_source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.HUBS))

        fieldName = self.parameterAsString(parameters, self.FIELD, context)

        units = self.UNITS[self.parameterAsEnum(parameters, self.UNIT, context)]

        fields = point_source.fields()
        fields.append(QgsField('HubName', QVariant.String))
        fields.append(QgsField('HubDist', QVariant.Double))

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               fields, QgsWkbTypes.LineString, point_source.sourceCrs())
        if sink is None:
            raise QgsProcessingException(self.invalidSinkError(parameters, self.OUTPUT))

        index = QgsSpatialIndex(hub_source.getFeatures(QgsFeatureRequest().setSubsetOfAttributes([]).setDestinationCrs(point_source.sourceCrs(), context.transformContext())))

        distance = QgsDistanceArea()
        distance.setSourceCrs(point_source.sourceCrs(), context.transformContext())
        distance.setEllipsoid(context.ellipsoid())

        # Scan source points, find nearest hub, and write to output file
        features = point_source.getFeatures()
        total = 100.0 / point_source.featureCount() if point_source.featureCount() else 0
        for current, f in enumerate(features):
            if feedback.isCanceled():
                break

            if not f.hasGeometry():
                sink.addFeature(f, QgsFeatureSink.FastInsert)
                continue
            src = f.geometry().boundingBox().center()

            neighbors = index.nearestNeighbor(src, 1)
            if len(neighbors) == 0:
                continue

            ft = next(hub_source.getFeatures(QgsFeatureRequest().setFilterFid(neighbors[0]).setSubsetOfAttributes([fieldName], hub_source.fields()).setDestinationCrs(point_source.sourceCrs(), context.transformContext())))
            closest = ft.geometry().boundingBox().center()
            hubDist = distance.measureLine(src, closest)

            if units != self.LAYER_UNITS:
                hub_dist_in_desired_units = distance.convertLengthMeasurement(hubDist, units)
            else:
                hub_dist_in_desired_units = hubDist

            attributes = f.attributes()
            attributes.append(ft[fieldName])
            attributes.append(hub_dist_in_desired_units)

            feat = QgsFeature()
            feat.setAttributes(attributes)

            feat.setGeometry(QgsGeometry.fromPolylineXY([src, closest]))

            sink.addFeature(feat, QgsFeatureSink.FastInsert)
            feedback.setProgress(int(current * total))

        return {self.OUTPUT: dest_id}
