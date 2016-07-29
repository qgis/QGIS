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

from qgis.core import Qgis, QgsFeature, QgsGeometry
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

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Convert geometry type')
        self.group, self.i18n_group = self.trAlgorithm('Vector geometry tools')

        self.types = [self.tr('Centroids'),
                      self.tr('Nodes'),
                      self.tr('Linestrings'),
                      self.tr('Multilinestrings'),
                      self.tr('Polygons')]

        self.addParameter(ParameterVector(self.INPUT,
                                          self.tr('Input layer'), [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterSelection(self.TYPE,
                                             self.tr('New geometry type'), self.types))

        self.addOutput(OutputVector(self.OUTPUT, self.tr('Converted')))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT))
        index = self.getParameterValue(self.TYPE)

        splitNodes = False
        if index == 0:
            newType = Qgis.WKBPoint
        elif index == 1:
            newType = Qgis.WKBPoint
            splitNodes = True
        elif index == 2:
            newType = Qgis.WKBLineString
        elif index == 3:
            newType = Qgis.WKBMultiLineString
        elif index == 4:
            newType = Qgis.WKBPolygon
        else:
            newType = Qgis.WKBPoint

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(
            layer.pendingFields(), newType, layer.crs())

        features = vector.features(layer)
        total = 100.0 / len(features)

        for current, f in enumerate(features):
            geom = f.geometry()
            geomType = geom.wkbType()

            if geomType in [Qgis.WKBPoint, Qgis.WKBPoint25D]:
                if newType == Qgis.WKBPoint:
                    writer.addFeature(f)
                else:
                    raise GeoAlgorithmExecutionException(
                        self.tr('Cannot convert from %s to %s', geomType, newType))
            elif geomType in [Qgis.WKBMultiPoint, Qgis.WKBMultiPoint25D]:
                if newType == Qgis.WKBPoint and splitNodes:
                    points = geom.asMultiPoint()
                    for p in points:
                        feat = QgsFeature()
                        feat.setAttributes(f.attributes())
                        feat.setGeometry(QgsGeometry.fromPoint(p))
                        writer.addFeature(feat)
                elif newType == Qgis.WKBPoint:
                    feat = QgsFeature()
                    feat.setAttributes(f.attributes())
                    feat.setGeometry(geom.centroid())
                    writer.addFeature(feat)
                else:
                    raise GeoAlgorithmExecutionException(
                        self.tr('Cannot convert from %s to %s', geomType, newType))
            elif geomType in [Qgis.WKBLineString, Qgis.WKBLineString25D]:
                if newType == Qgis.WKBPoint and splitNodes:
                    points = geom.asPolyline()
                    for p in points:
                        feat = QgsFeature()
                        feat.setAttributes(f.attributes())
                        feat.setGeometry(QgsGeometry.fromPoint(p))
                        writer.addFeature(feat)
                elif newType == Qgis.WKBPoint:
                    feat = QgsFeature()
                    feat.setAttributes(f.attributes())
                    feat.setGeometry(geom.centroid())
                    writer.addFeature(feat)
                elif newType == Qgis.WKBLineString:
                    writer.addFeature(f)
                else:
                    raise GeoAlgorithmExecutionException(
                        self.tr('Cannot convert from %s to %s', geomType, newType))
            elif geomType in [Qgis.WKBMultiLineString, Qgis.WKBMultiLineString25D]:
                if newType == Qgis.WKBPoint and splitNodes:
                    lines = geom.asMultiPolyline()
                    for line in lines:
                        for p in line:
                            feat = QgsFeature()
                            feat.setAttributes(f.attributes())
                            feat.setGeometry(QgsGeometry.fromPoint(p))
                            writer.addFeature(feat)
                elif newType == Qgis.WKBPoint:
                    feat = QgsFeature()
                    feat.setAttributes(f.attributes())
                    feat.setGeometry(geom.centroid())
                    writer.addFeature(feat)
                elif newType == Qgis.WKBLineString:
                    lines = geom.asMultiPolyline()
                    for line in lines:
                        feat = QgsFeature()
                        feat.setAttributes(f.attributes())
                        feat.setGeometry(QgsGeometry.fromPolyline(line))
                        writer.addFeature(feat)
                elif newType == Qgis.WKBMultiLineString:
                    writer.addFeature(f)
                else:
                    raise GeoAlgorithmExecutionException(
                        self.tr('Cannot convert from %s to %s', geomType, newType))
            elif geomType in [Qgis.WKBPolygon, Qgis.WKBPolygon25D]:
                if newType == Qgis.WKBPoint and splitNodes:
                    rings = geom.asPolygon()
                    for ring in rings:
                        for p in ring:
                            feat = QgsFeature()
                            feat.setAttributes(f.attributes())
                            feat.setGeometry(QgsGeometry.fromPoint(p))
                            writer.addFeature(feat)
                elif newType == Qgis.WKBPoint:
                    feat = QgsFeature()
                    feat.setAttributes(f.attributes())
                    feat.setGeometry(geom.centroid())
                    writer.addFeature(feat)
                elif newType == Qgis.WKBMultiLineString:
                    rings = geom.asPolygon()
                    feat = QgsFeature()
                    feat.setAttributes(f.attributes())
                    feat.setGeometry(QgsGeometry.fromMultiPolyline(rings))
                    writer.addFeature(feat)
                elif newType == Qgis.WKBPolygon:
                    writer.addFeature(f)
                else:
                    raise GeoAlgorithmExecutionException(
                        self.tr('Cannot convert from %s to %s', geomType, newType))
            elif geomType in [Qgis.WKBMultiPolygon, Qgis.WKBMultiPolygon25D]:
                if newType == Qgis.WKBPoint and splitNodes:
                    polygons = geom.asMultiPolygon()
                    for polygon in polygons:
                        for line in polygon:
                            for p in line:
                                feat = QgsFeature()
                                feat.setAttributes(f.attributes())
                                feat.setGeometry(QgsGeometry.fromPoint(p))
                                writer.addFeature(feat)
                elif newType == Qgis.WKBPoint:
                    feat = QgsFeature()
                    feat.setAttributes(f.attributes())
                    feat.setGeometry(geom.centroid())
                    writer.addFeature(feat)
                elif newType == Qgis.WKBLineString:
                    polygons = geom.asMultiPolygon()
                    for polygons in polygons:
                        feat = QgsFeature()
                        feat.setAttributes(f.attributes())
                        feat.setGeometry(QgsGeometry.fromPolyline(polygon))
                        writer.addFeature(feat)
                elif newType == Qgis.WKBPolygon:
                    polygons = geom.asMultiPolygon()
                    for polygon in polygons:
                        feat = QgsFeature()
                        feat.setAttributes(f.attributes())
                        feat.setGeometry(QgsGeometry.fromPolygon(polygon))
                        writer.addFeature(feat)
                elif newType in [Qgis.WKBMultiLineString, Qgis.WKBMultiPolygon]:
                    writer.addFeature(f)
                else:
                    raise GeoAlgorithmExecutionException(
                        self.tr('Cannot convert from %s to %s', geomType, newType))

            progress.setPercentage(int(current * total))

        del writer
