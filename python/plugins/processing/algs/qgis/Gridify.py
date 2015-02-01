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

from qgis.core import QGis, QgsGeometry, QgsFeature, QgsPoint
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.ProcessingLog import ProcessingLog
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterNumber
from processing.core.outputs import OutputVector

from processing.tools import dataobjects, vector

class Gridify(GeoAlgorithm):
    INPUT = 'INPUT'
    HSPACING = 'HSPACING'
    VSPACING = 'VSPACING'
    OUTPUT = 'OUTPUT'

    def defineCharacteristics(self):
        self.name = 'Snap points to grid'
        self.group = 'Vector general tools'

        self.addParameter(ParameterVector(self.INPUT,
            self.tr('Input Layer'), [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterNumber(self.HSPACING,
            self.tr('Horizontal spacing'), default=0.1))
        self.addParameter(ParameterNumber(self.VSPACING,
            self.tr('Vertical spacing'), default=0.1))

        self.addOutput(OutputVector(self.OUTPUT, self.tr('Output')))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(self.getParameterValue(self.INPUT))
        hSpacing = self.getParameterValue(self.HSPACING)
        vSpacing = self.getParameterValue(self.VSPACING)

        if hSpacing <= 0 or vSpacing <= 0:
            raise GeoAlgorithmExecutionException(
                self.tr('Invalid grid spacing: %s/%s' % (hSpacing, vSpacing)))

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(
            layer.pendingFields(), layer.wkbType(), layer.crs())

        features = vector.features(layer)

        count = len(features)
        total = 100.0 / float(count)

        for count, f in enumerate(features):
            geom = f.geometry()
            geomType = geom.wkbType()

            if geomType == QGis.WKBPoint:
                points = self._gridify([geom.asPoint()], hSpacing, vSpacing)
                newGeom = QgsGeometry.fromPoint(points[0])
            elif geomType == QGis.WKBMultiPoint:
                points = self._gridify(geom.aMultiPoint(), hSpacing, vSpacing)
                newGeom = QgsGeometry.fromMultiPoint(points)
            elif geomType == QGis.WKBLineString:
                points = self._gridify(geom.asPolyline(), hSpacing, vSpacing)
                if len(points) < 2:
                    ProcessingLog.addToLog(ProcessingLog.LOG_INFO,
                        self.tr('Failed to gridify feature with FID %s' % f.id()))
                    newGeom = None
                else:
                    newGeom = QgsGeometry.fromPolyline(points)
            elif geomType == QGis.WKBMultiLineString:
                polyline = []
                for line in geom.asMultiPolyline():
                    points = self._gridify(line, hSpacing, vSpacing)
                    if len(points) > 1:
                        polyline.append(points)
                if len(polyline) <= 0:
                    ProcessingLog.addToLog(ProcessingLog.LOG_INFO,
                        self.tr('Failed to gridify feature with FID %s' % f.id()))
                    newGeom = None
                else:
                    newGeom = QgsGeometry.fromMultiPolyline(polyline)

            elif geomType == QGis.WKBPolygon:
                polygon = []
                for line in geom.asPolygon():
                    points = self._gridify(line, hSpacing, vSpacing)
                    if len(points) > 1:
                        polygon.append(points)
                if len(polygon) <= 0:
                    ProcessingLog.addToLog(ProcessingLog.LOG_INFO,
                        self.tr('Failed to gridify feature with FID %s' % f.id()))
                    newGeom = None
                else:
                    newGeom = QgsGeometry.fromPolygon(polygon)
            elif geomType == QGis.WKBMultiPolygon:
                multipolygon = []
                for polygon in geom.asMultiPolygon():
                    newPolygon = []
                    for line in polygon:
                        points = self._gridify(line, hSpacing, vSpacing)
                        if len(points) > 2:
                            newPolygon.append(points)

                    if len(newPolygon) > 0:
                        multipolygon.append(newPolygon)

                if len(multipolygon) <= 0:
                    ProcessingLog.addToLog(ProcessingLog.LOG_INFO,
                        self.tr('Failed to gridify feature with FID %s' % f.id()))
                    newGeom = None
                else:
                    newGeom = QgsGeometry.fromMultiPolygon(multipolygon)

            if newGeom is not None:
                feat = QgsFeature()
                feat.setGeometry(newGeom)
                feat.setAttributes(f.attributes())
                writer.addFeature(feat)

            progress.setPercentage(int(count * total))

        del writer

    def _gridify(self, points, hSpacing, vSpacing):
        nPoints = []
        for p in points:
            nPoints.append(QgsPoint(round(p.x() / hSpacing, 0) * hSpacing,
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
