# -*- coding: utf-8 -*-

"""
***************************************************************************
    Gridify.py
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

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.core import (QgsFeature,
                       QgsGeometry,
                       QgsFeatureSink,
                       QgsWkbTypes,
                       QgsProcessingException,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterFeatureSink)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class GeometryConvert(QgisAlgorithm):
    INPUT = 'INPUT'
    TYPE = 'TYPE'
    OUTPUT = 'OUTPUT'

    def group(self):
        return self.tr('Vector geometry tools')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.types = [self.tr('Centroids'),
                      self.tr('Nodes'),
                      self.tr('Linestrings'),
                      self.tr('Multilinestrings'),
                      self.tr('Polygons')]

        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterEnum(self.TYPE,
                                                     self.tr('New geometry type'), options=self.types))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT,
                                                            self.tr('Converted')))

    def name(self):
        return 'convertgeometrytype'

    def displayName(self):
        return self.tr('Convert geometry type')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        index = self.parameterAsEnum(parameters, self.TYPE, context)

        splitNodes = False
        if index == 0:
            newType = QgsWkbTypes.Point
        elif index == 1:
            newType = QgsWkbTypes.Point
            splitNodes = True
        elif index == 2:
            newType = QgsWkbTypes.LineString
        elif index == 3:
            newType = QgsWkbTypes.MultiLineString
        elif index == 4:
            newType = QgsWkbTypes.Polygon
        else:
            newType = QgsWkbTypes.Point

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               source.fields(), newType, source.sourceCrs())

        features = source.getFeatures()
        total = 100.0 / source.featureCount() if source.featureCount() else 0

        for current, f in enumerate(features):
            if feedback.isCanceled():
                break

            if not f.hasGeometry():
                sink.addFeature(f, QgsFeatureSink.FastInsert)

            geom = f.geometry()
            geomType = geom.wkbType()

            if QgsWkbTypes.geometryType(geomType) == QgsWkbTypes.PointGeometry and not QgsWkbTypes.isMultiType(geomType):
                if newType == QgsWkbTypes.Point:
                    sink.addFeature(f, QgsFeatureSink.FastInsert)
                else:
                    raise QgsProcessingException(
                        self.tr('Cannot convert from {0} to {1}').format(geomType, newType))
            elif QgsWkbTypes.geometryType(geomType) == QgsWkbTypes.PointGeometry and QgsWkbTypes.isMultiType(geomType):
                if newType == QgsWkbTypes.Point and splitNodes:
                    points = geom.asMultiPoint()
                    for p in points:
                        feat = QgsFeature()
                        feat.setAttributes(f.attributes())
                        feat.setGeometry(QgsGeometry.fromPoint(p))
                        sink.addFeature(feat, QgsFeatureSink.FastInsert)
                elif newType == QgsWkbTypes.Point:
                    feat = QgsFeature()
                    feat.setAttributes(f.attributes())
                    feat.setGeometry(geom.centroid())
                    sink.addFeature(feat, QgsFeatureSink.FastInsert)
                else:
                    raise QgsProcessingException(
                        self.tr('Cannot convert from {0} to {1}').format(geomType, newType))
            elif QgsWkbTypes.geometryType(geomType) == QgsWkbTypes.LineGeometry and not QgsWkbTypes.isMultiType(geomType):
                if newType == QgsWkbTypes.Point and splitNodes:
                    points = geom.asPolyline()
                    for p in points:
                        feat = QgsFeature()
                        feat.setAttributes(f.attributes())
                        feat.setGeometry(QgsGeometry.fromPoint(p))
                        sink.addFeature(feat, QgsFeatureSink.FastInsert)
                elif newType == QgsWkbTypes.Point:
                    feat = QgsFeature()
                    feat.setAttributes(f.attributes())
                    feat.setGeometry(geom.centroid())
                    sink.addFeature(feat, QgsFeatureSink.FastInsert)
                elif newType == QgsWkbTypes.LineString:
                    sink.addFeature(f, QgsFeatureSink.FastInsert)
                else:
                    raise QgsProcessingException(
                        self.tr('Cannot convert from {0} to {1}').format(geomType, newType))
            elif QgsWkbTypes.geometryType(geomType) == QgsWkbTypes.LineGeometry and QgsWkbTypes.isMultiType(
                    geomType):
                if newType == QgsWkbTypes.Point and splitNodes:
                    lines = geom.asMultiPolyline()
                    for line in lines:
                        for p in line:
                            feat = QgsFeature()
                            feat.setAttributes(f.attributes())
                            feat.setGeometry(QgsGeometry.fromPoint(p))
                            sink.addFeature(feat, QgsFeatureSink.FastInsert)
                elif newType == QgsWkbTypes.Point:
                    feat = QgsFeature()
                    feat.setAttributes(f.attributes())
                    feat.setGeometry(geom.centroid())
                    sink.addFeature(feat, QgsFeatureSink.FastInsert)
                elif newType == QgsWkbTypes.LineString:
                    lines = geom.asMultiPolyline()
                    for line in lines:
                        feat = QgsFeature()
                        feat.setAttributes(f.attributes())
                        feat.setGeometry(QgsGeometry.fromPolyline(line))
                        sink.addFeature(feat, QgsFeatureSink.FastInsert)
                elif newType == QgsWkbTypes.MultiLineString:
                    sink.addFeature(f, QgsFeatureSink.FastInsert)
                else:
                    raise QgsProcessingException(
                        self.tr('Cannot convert from {0} to {1}').format(geomType, newType))
            elif QgsWkbTypes.geometryType(geomType) == QgsWkbTypes.PolygonGeometry and not QgsWkbTypes.isMultiType(
                    geomType):
                if newType == QgsWkbTypes.Point and splitNodes:
                    rings = geom.asPolygon()
                    for ring in rings:
                        for p in ring:
                            feat = QgsFeature()
                            feat.setAttributes(f.attributes())
                            feat.setGeometry(QgsGeometry.fromPoint(p))
                            sink.addFeature(feat, QgsFeatureSink.FastInsert)
                elif newType == QgsWkbTypes.Point:
                    feat = QgsFeature()
                    feat.setAttributes(f.attributes())
                    feat.setGeometry(geom.centroid())
                    sink.addFeature(feat, QgsFeatureSink.FastInsert)
                elif newType == QgsWkbTypes.MultiLineString:
                    rings = geom.asPolygon()
                    feat = QgsFeature()
                    feat.setAttributes(f.attributes())
                    feat.setGeometry(QgsGeometry.fromMultiPolyline(rings))
                    sink.addFeature(feat, QgsFeatureSink.FastInsert)
                elif newType == QgsWkbTypes.Polygon:
                    sink.addFeature(f, QgsFeatureSink.FastInsert)
                else:
                    raise QgsProcessingException(
                        self.tr('Cannot convert from {0} to {1}').format(geomType, newType))
            elif QgsWkbTypes.geometryType(
                    geomType) == QgsWkbTypes.PolygonGeometry and QgsWkbTypes.isMultiType(
                    geomType):
                if newType == QgsWkbTypes.Point and splitNodes:
                    polygons = geom.asMultiPolygon()
                    for polygon in polygons:
                        for line in polygon:
                            for p in line:
                                feat = QgsFeature()
                                feat.setAttributes(f.attributes())
                                feat.setGeometry(QgsGeometry.fromPoint(p))
                                sink.addFeature(feat, QgsFeatureSink.FastInsert)
                elif newType == QgsWkbTypes.Point:
                    feat = QgsFeature()
                    feat.setAttributes(f.attributes())
                    feat.setGeometry(geom.centroid())
                    sink.addFeature(feat, QgsFeatureSink.FastInsert)
                elif newType == QgsWkbTypes.LineString:
                    polygons = geom.asMultiPolygon()
                    for polygon in polygons:
                        feat = QgsFeature()
                        feat.setAttributes(f.attributes())
                        feat.setGeometry(QgsGeometry.fromPolyline(polygon))
                        sink.addFeature(feat, QgsFeatureSink.FastInsert)
                elif newType == QgsWkbTypes.Polygon:
                    polygons = geom.asMultiPolygon()
                    for polygon in polygons:
                        feat = QgsFeature()
                        feat.setAttributes(f.attributes())
                        feat.setGeometry(QgsGeometry.fromPolygon(polygon))
                        sink.addFeature(feat, QgsFeatureSink.FastInsert)
                elif newType in [QgsWkbTypes.MultiLineString, QgsWkbTypes.MultiPolygon]:
                    sink.addFeature(f, QgsFeatureSink.FastInsert)
                else:
                    raise QgsProcessingException(
                        self.tr('Cannot convert from {0} to {1}').format(geomType, newType))

            feedback.setProgress(int(current * total))

        return {self.OUTPUT: dest_id}
