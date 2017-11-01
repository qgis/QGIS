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
                       QgsMultiPoint,
                       QgsMultiLineString,
                       QgsLineString,
                       QgsPolygon,
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
        return self.tr('Vector geometry')

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

        if index == 0:
            newType = QgsWkbTypes.Point
        elif index == 1:
            newType = QgsWkbTypes.Point
            if QgsWkbTypes.hasM(source.wkbType()):
                newType = QgsWkbTypes.addM(newType)
            if QgsWkbTypes.hasZ(source.wkbType()):
                newType = QgsWkbTypes.addZ(newType)
        elif index == 2:
            newType = QgsWkbTypes.LineString
            if QgsWkbTypes.hasM(source.wkbType()):
                newType = QgsWkbTypes.addM(newType)
            if QgsWkbTypes.hasZ(source.wkbType()):
                newType = QgsWkbTypes.addZ(newType)
        elif index == 3:
            newType = QgsWkbTypes.MultiLineString
            if QgsWkbTypes.hasM(source.wkbType()):
                newType = QgsWkbTypes.addM(newType)
            if QgsWkbTypes.hasZ(source.wkbType()):
                newType = QgsWkbTypes.addZ(newType)
        else:
            newType = QgsWkbTypes.Polygon
            if QgsWkbTypes.hasM(source.wkbType()):
                newType = QgsWkbTypes.addM(newType)
            if QgsWkbTypes.hasZ(source.wkbType()):
                newType = QgsWkbTypes.addZ(newType)

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               source.fields(), newType, source.sourceCrs())

        features = source.getFeatures()
        total = 100.0 / source.featureCount() if source.featureCount() else 0

        for current, f in enumerate(features):
            if feedback.isCanceled():
                break

            if not f.hasGeometry():
                sink.addFeature(f, QgsFeatureSink.FastInsert)
            else:
                for p in self.convertGeometry(f.geometry(), index):
                    feat = QgsFeature()
                    feat.setAttributes(f.attributes())
                    feat.setGeometry(p)
                    sink.addFeature(feat, QgsFeatureSink.FastInsert)

            feedback.setProgress(int(current * total))

        return {self.OUTPUT: dest_id}

    def convertGeometry(self, geom, target_type):
        # returns an array of output geometries for the input geometry
        if target_type == 0:
            #centroid
            return self.convertToCentroid(geom)
        elif target_type == 1:
            #nodes
            return self.convertToNodes(geom)
        elif target_type == 2:
            #linestrings
            return self.convertToLineStrings(geom)
        elif target_type == 3:
            #multilinestrings
            return self.convertToMultiLineStrings(geom)
        elif target_type == 4:
            #polygon
            return self.convertToPolygon(geom)

    def convertToCentroid(self, geom):
        return [geom.centroid()]

    def convertToNodes(self, geom):
        mp = QgsMultiPoint()
        # TODO: mega inefficient - needs rework when geometry iterators land
        # (but at least it doesn't lose Z/M values)
        for g in geom.constGet().coordinateSequence():
            for r in g:
                for p in r:
                    mp.addGeometry(p)
        return [QgsGeometry(mp)]

    def convertToLineStrings(self, geom):
        if QgsWkbTypes.geometryType(geom.wkbType()) == QgsWkbTypes.PointGeometry:
            raise QgsProcessingException(
                self.tr('Cannot convert from {0} to LineStrings').format(QgsWkbTypes.displayString(geom.wkbType())))
        elif QgsWkbTypes.geometryType(geom.wkbType()) == QgsWkbTypes.LineGeometry:
            if QgsWkbTypes.isMultiType(geom.wkbType()):
                return geom.asGeometryCollection()
            else:
                #line to line
                return [geom]
        else:
            # polygons to lines
            # we just use the boundary here - that consists of all rings in the (multi)polygon
            boundary = QgsGeometry(geom.constGet().boundary())
            # boundary will be multipart
            return boundary.asGeometryCollection()

    def convertToMultiLineStrings(self, geom):
        if QgsWkbTypes.geometryType(geom.wkbType()) == QgsWkbTypes.PointGeometry:
            raise QgsProcessingException(
                self.tr('Cannot convert from {0} to MultiLineStrings').format(QgsWkbTypes.displayString(geom.wkbType())))
        elif QgsWkbTypes.geometryType(geom.wkbType()) == QgsWkbTypes.LineGeometry:
            if QgsWkbTypes.isMultiType(geom.wkbType()):
                return [geom]
            else:
                # line to multiLine
                ml = QgsMultiLineString()
                ml.addGeometry(geom.constGet().clone())
                return [QgsGeometry(ml)]
        else:
            # polygons to multilinestring
            # we just use the boundary here - that consists of all rings in the (multi)polygon
            return [QgsGeometry(geom.constGet().boundary())]

    def convertToPolygon(self, geom):
        if QgsWkbTypes.geometryType(geom.wkbType()) == QgsWkbTypes.PointGeometry and geom.constGet().nCoordinates() < 3:
            raise QgsProcessingException(
                self.tr('Cannot convert from Point to Polygon').format(QgsWkbTypes.displayString(geom.wkbType())))
        elif QgsWkbTypes.geometryType(geom.wkbType()) == QgsWkbTypes.PointGeometry:
            # multipoint with at least 3 points
            # TODO: mega inefficient - needs rework when geometry iterators land
            # (but at least it doesn't lose Z/M values)
            points = []
            for g in geom.constGet().coordinateSequence():
                for r in g:
                    for p in r:
                        points.append(p)
            linestring = QgsLineString(points)
            linestring.close()
            p = QgsPolygon()
            p.setExteriorRing(linestring)
            return [QgsGeometry(p)]
        elif QgsWkbTypes.geometryType(geom.wkbType()) == QgsWkbTypes.LineGeometry:
            if QgsWkbTypes.isMultiType(geom):
                parts = []
                for i in range(geom.constGet().numGeometries()):
                    p = QgsPolygon()
                    linestring = geom.constGet().geometryN(i).clone()
                    linestring.close()
                    p.setExteriorRing(linestring)
                    parts.append(QgsGeometry(p))
                return QgsGeometry.collectGeometry(parts)
            else:
                # linestring to polygon
                p = QgsPolygon()
                linestring = geom.constGet().clone()
                linestring.close()
                p.setExteriorRing(linestring)
                return [QgsGeometry(p)]
        else:
            #polygon
            if QgsWkbTypes.isMultiType(geom):
                return geom.asGeometryCollection()
            else:
                return [geom]
