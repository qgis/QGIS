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

from qgis.core import (QgsGeometry,
                       QgsFeature,
                       QgsFeatureSink,
                       QgsPointXY,
                       QgsWkbTypes,
                       QgsApplication,
                       QgsProcessingException,
                       QgsProcessingParameterNumber)
from processing.algs.qgis.QgisAlgorithm import QgisFeatureBasedAlgorithm


class Gridify(QgisFeatureBasedAlgorithm):

    HSPACING = 'HSPACING'
    VSPACING = 'VSPACING'

    def group(self):
        return self.tr('Vector geometry')

    def __init__(self):
        super().__init__()
        self.h_spacing = None
        self.v_spacing = None

    def initParameters(self, config=None):
        self.addParameter(QgsProcessingParameterNumber(self.HSPACING,
                                                       self.tr('Horizontal spacing'), type=QgsProcessingParameterNumber.Double, minValue=0.0, defaultValue=0.1))
        self.addParameter(QgsProcessingParameterNumber(self.VSPACING,
                                                       self.tr('Vertical spacing'), type=QgsProcessingParameterNumber.Double, minValue=0.0, defaultValue=0.1))

    def name(self):
        return 'snappointstogrid'

    def displayName(self):
        return self.tr('Snap points to grid')

    def outputName(self):
        return self.tr('Snapped')

    def prepareAlgorithm(self, parameters, context, feedback):
        self.h_spacing = self.parameterAsDouble(parameters, self.HSPACING, context)
        self.v_spacing = self.parameterAsDouble(parameters, self.VSPACING, context)
        if self.h_spacing <= 0 or self.v_spacing <= 0:
            raise QgsProcessingException(
                self.tr('Invalid grid spacing: {0}/{1}').format(self.h_spacing, self.v_spacing))

        return True

    def processFeature(self, feature, feedback):
        if feature.hasGeometry():
            geom = feature.geometry()
            geomType = QgsWkbTypes.flatType(geom.wkbType())
            newGeom = None

            if geomType == QgsWkbTypes.Point:
                points = self._gridify([geom.asPoint()], self.h_spacing, self.v_spacing)
                newGeom = QgsGeometry.fromPoint(points[0])
            elif geomType == QgsWkbTypes.MultiPoint:
                points = self._gridify(geom.asMultiPoint(), self.h_spacing, self.v_spacing)
                newGeom = QgsGeometry.fromMultiPoint(points)
            elif geomType == QgsWkbTypes.LineString:
                points = self._gridify(geom.asPolyline(), self.h_spacing, self.v_spacing)
                if len(points) < 2:
                    feedback.reportError(self.tr('Failed to gridify feature with FID {0}').format(feature.id()))
                    newGeom = None
                else:
                    newGeom = QgsGeometry.fromPolyline(points)
            elif geomType == QgsWkbTypes.MultiLineString:
                polyline = []
                for line in geom.asMultiPolyline():
                    points = self._gridify(line, self.h_spacing, self.v_spacing)
                    if len(points) > 1:
                        polyline.append(points)
                if len(polyline) <= 0:
                    feedback.reportError(self.tr('Failed to gridify feature with FID {0}').format(feature.id()))
                    newGeom = None
                else:
                    newGeom = QgsGeometry.fromMultiPolyline(polyline)

            elif geomType == QgsWkbTypes.Polygon:
                polygon = []
                for line in geom.asPolygon():
                    points = self._gridify(line, self.h_spacing, self.v_spacing)
                    if len(points) > 1:
                        polygon.append(points)
                if len(polygon) <= 0:
                    feedback.reportError(self.tr('Failed to gridify feature with FID {0}').format(feature.id()))
                    newGeom = None
                else:
                    newGeom = QgsGeometry.fromPolygon(polygon)
            elif geomType == QgsWkbTypes.MultiPolygon:
                multipolygon = []
                for polygon in geom.asMultiPolygon():
                    newPolygon = []
                    for line in polygon:
                        points = self._gridify(line, self.h_spacing, self.v_spacing)
                        if len(points) > 2:
                            newPolygon.append(points)

                    if len(newPolygon) > 0:
                        multipolygon.append(newPolygon)

                if len(multipolygon) <= 0:
                    feedback.reportError(self.tr('Failed to gridify feature with FID {0}').format(feature.id()))
                    newGeom = None
                else:
                    newGeom = QgsGeometry.fromMultiPolygon(multipolygon)

            if newGeom is not None:
                feature.setGeometry(newGeom)
            else:
                feature.clearGeometry()
        return feature

    def _gridify(self, points, hSpacing, vSpacing):
        nPoints = []
        for p in points:
            nPoints.append(QgsPointXY(round(p.x() / hSpacing, 0) * hSpacing,
                                      round(p.y() / vSpacing, 0) * vSpacing))

        i = 0
        # Delete overlapping points
        while i < len(nPoints) - 2:
            if nPoints[i] == nPoints[i + 1]:
                nPoints.pop(i + 1)
            else:
                i += 1

        i = 0
        # Delete line points that go out and return to the same place
        while i < len(nPoints) - 3:
            if nPoints[i] == nPoints[i + 2]:
                nPoints.pop(i + 1)
                nPoints.pop(i + 1)

                # Step back to catch arcs
                if i > 0:
                    i -= 1
            else:
                i += 1

        i = 0
        # Delete overlapping start/end points
        while len(nPoints) > 1 and nPoints[0] == nPoints[len(nPoints) - 1]:
            nPoints.pop(len(nPoints) - 1)

        return nPoints
