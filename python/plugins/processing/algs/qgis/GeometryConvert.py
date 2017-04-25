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
                       QgsWkbTypes,
                       QgsApplication,
                       QgsProcessingUtils)
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterSelection
from processing.core.outputs import OutputVector

from processing.tools import dataobjects, vector


class GeometryConvert(GeoAlgorithm):
    INPUT = 'INPUT'
    TYPE = 'TYPE'
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QgsApplication.getThemeIcon("/providerQgis.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("providerQgis.svg")

    def group(self):
        return self.tr('Vector geometry tools')

    def name(self):
        return 'convertgeometrytype'

    def displayName(self):
        return self.tr('Convert geometry type')

    def defineCharacteristics(self):
        self.types = [self.tr('Centroids'),
                      self.tr('Nodes'),
                      self.tr('Linestrings'),
                      self.tr('Multilinestrings'),
                      self.tr('Polygons')]

        self.addParameter(ParameterVector(self.INPUT,
                                          self.tr('Input layer')))
        self.addParameter(ParameterSelection(self.TYPE,
                                             self.tr('New geometry type'), self.types))

        self.addOutput(OutputVector(self.OUTPUT, self.tr('Converted')))

    def processAlgorithm(self, context, feedback):
        layer = dataobjects.getLayerFromString(
            self.getParameterValue(self.INPUT))
        index = self.getParameterValue(self.TYPE)

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

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(
            layer.fields(), newType, layer.crs())

        features = QgsProcessingUtils.getFeatures(layer, context)
        total = 100.0 / QgsProcessingUtils.featureCount(layer, context)

        for current, f in enumerate(features):
            geom = f.geometry()
            geomType = geom.wkbType()

            if geomType in [QgsWkbTypes.Point, QgsWkbTypes.Point25D]:
                if newType == QgsWkbTypes.Point:
                    writer.addFeature(f)
                else:
                    raise GeoAlgorithmExecutionException(
                        self.tr('Cannot convert from {0} to {1}').format(geomType, newType))
            elif geomType in [QgsWkbTypes.MultiPoint, QgsWkbTypes.MultiPoint25D]:
                if newType == QgsWkbTypes.Point and splitNodes:
                    points = geom.asMultiPoint()
                    for p in points:
                        feat = QgsFeature()
                        feat.setAttributes(f.attributes())
                        feat.setGeometry(QgsGeometry.fromPoint(p))
                        writer.addFeature(feat)
                elif newType == QgsWkbTypes.Point:
                    feat = QgsFeature()
                    feat.setAttributes(f.attributes())
                    feat.setGeometry(geom.centroid())
                    writer.addFeature(feat)
                else:
                    raise GeoAlgorithmExecutionException(
                        self.tr('Cannot convert from {0} to {1}').format(geomType, newType))
            elif geomType in [QgsWkbTypes.LineString, QgsWkbTypes.LineString25D]:
                if newType == QgsWkbTypes.Point and splitNodes:
                    points = geom.asPolyline()
                    for p in points:
                        feat = QgsFeature()
                        feat.setAttributes(f.attributes())
                        feat.setGeometry(QgsGeometry.fromPoint(p))
                        writer.addFeature(feat)
                elif newType == QgsWkbTypes.Point:
                    feat = QgsFeature()
                    feat.setAttributes(f.attributes())
                    feat.setGeometry(geom.centroid())
                    writer.addFeature(feat)
                elif newType == QgsWkbTypes.LineString:
                    writer.addFeature(f)
                else:
                    raise GeoAlgorithmExecutionException(
                        self.tr('Cannot convert from {0} to {1}').format(geomType, newType))
            elif geomType in [QgsWkbTypes.MultiLineString, QgsWkbTypes.MultiLineString25D]:
                if newType == QgsWkbTypes.Point and splitNodes:
                    lines = geom.asMultiPolyline()
                    for line in lines:
                        for p in line:
                            feat = QgsFeature()
                            feat.setAttributes(f.attributes())
                            feat.setGeometry(QgsGeometry.fromPoint(p))
                            writer.addFeature(feat)
                elif newType == QgsWkbTypes.Point:
                    feat = QgsFeature()
                    feat.setAttributes(f.attributes())
                    feat.setGeometry(geom.centroid())
                    writer.addFeature(feat)
                elif newType == QgsWkbTypes.LineString:
                    lines = geom.asMultiPolyline()
                    for line in lines:
                        feat = QgsFeature()
                        feat.setAttributes(f.attributes())
                        feat.setGeometry(QgsGeometry.fromPolyline(line))
                        writer.addFeature(feat)
                elif newType == QgsWkbTypes.MultiLineString:
                    writer.addFeature(f)
                else:
                    raise GeoAlgorithmExecutionException(
                        self.tr('Cannot convert from {0} to {1}').format(geomType, newType))
            elif geomType in [QgsWkbTypes.Polygon, QgsWkbTypes.Polygon25D]:
                if newType == QgsWkbTypes.Point and splitNodes:
                    rings = geom.asPolygon()
                    for ring in rings:
                        for p in ring:
                            feat = QgsFeature()
                            feat.setAttributes(f.attributes())
                            feat.setGeometry(QgsGeometry.fromPoint(p))
                            writer.addFeature(feat)
                elif newType == QgsWkbTypes.Point:
                    feat = QgsFeature()
                    feat.setAttributes(f.attributes())
                    feat.setGeometry(geom.centroid())
                    writer.addFeature(feat)
                elif newType == QgsWkbTypes.MultiLineString:
                    rings = geom.asPolygon()
                    feat = QgsFeature()
                    feat.setAttributes(f.attributes())
                    feat.setGeometry(QgsGeometry.fromMultiPolyline(rings))
                    writer.addFeature(feat)
                elif newType == QgsWkbTypes.Polygon:
                    writer.addFeature(f)
                else:
                    raise GeoAlgorithmExecutionException(
                        self.tr('Cannot convert from {0} to {1}').format(geomType, newType))
            elif geomType in [QgsWkbTypes.MultiPolygon, QgsWkbTypes.MultiPolygon25D]:
                if newType == QgsWkbTypes.Point and splitNodes:
                    polygons = geom.asMultiPolygon()
                    for polygon in polygons:
                        for line in polygon:
                            for p in line:
                                feat = QgsFeature()
                                feat.setAttributes(f.attributes())
                                feat.setGeometry(QgsGeometry.fromPoint(p))
                                writer.addFeature(feat)
                elif newType == QgsWkbTypes.Point:
                    feat = QgsFeature()
                    feat.setAttributes(f.attributes())
                    feat.setGeometry(geom.centroid())
                    writer.addFeature(feat)
                elif newType == QgsWkbTypes.LineString:
                    polygons = geom.asMultiPolygon()
                    for polygons in polygons:
                        feat = QgsFeature()
                        feat.setAttributes(f.attributes())
                        feat.setGeometry(QgsGeometry.fromPolyline(polygon))
                        writer.addFeature(feat)
                elif newType == QgsWkbTypes.Polygon:
                    polygons = geom.asMultiPolygon()
                    for polygon in polygons:
                        feat = QgsFeature()
                        feat.setAttributes(f.attributes())
                        feat.setGeometry(QgsGeometry.fromPolygon(polygon))
                        writer.addFeature(feat)
                elif newType in [QgsWkbTypes.MultiLineString, QgsWkbTypes.MultiPolygon]:
                    writer.addFeature(f)
                else:
                    raise GeoAlgorithmExecutionException(
                        self.tr('Cannot convert from {0} to {1}').format(geomType, newType))

            feedback.setProgress(int(current * total))

        del writer
