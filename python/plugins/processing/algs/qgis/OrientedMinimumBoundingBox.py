# -*- coding: utf-8 -*-

"""
***************************************************************************
    OrientedMinimumBoundingBox.py
    ---------------------
    Date                 : June 2015
    Copyright            : (C) 2015, Loïc BARTOLETTI
    Email                : coder at tuxfamily dot org
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Loïc BARTOLETTI'
__date__ = 'June 2015'
__copyright__ = '(C) 2015, Loïc BARTOLETTI'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from math import degrees, atan2
from qgis.PyQt.QtCore import QVariant
from qgis.core import QGis, QgsField, QgsPoint, QgsGeometry, QgsFeature
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterBoolean
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector


class OrientedMinimumBoundingBox(GeoAlgorithm):

    INPUT_LAYER = 'INPUT_LAYER'
    BY_FEATURE = 'BY_FEATURE'

    OUTPUT = 'OUTPUT'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Oriented minimum bounding box')
        self.group, self.i18n_group = self.trAlgorithm('Vector general tools')

        self.addParameter(ParameterVector(self.INPUT_LAYER,
                                          self.tr('Input layer'), [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterBoolean(self.BY_FEATURE,
                                           self.tr('Calculate OMBB for each feature separately'), True))

        self.addOutput(OutputVector(self.OUTPUT, self.tr('Oriented_MBBox')))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT_LAYER))
        byFeature = self.getParameterValue(self.BY_FEATURE)

        if byFeature and layer.geometryType() == QGis.Point and layer.featureCount() <= 2:
            raise GeoAlgorithmExecutionException(self.tr("Can't calculate an OMBB for each point, it's a point. The number of points must be greater than 2"))

        fields = [
            QgsField('AREA', QVariant.Double),
            QgsField('PERIMETER', QVariant.Double),
            QgsField('ANGLE', QVariant.Double),
            QgsField('WIDTH', QVariant.Double),
            QgsField('HEIGHT', QVariant.Double),
        ]

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(fields,
                                                                     QGis.WKBPolygon, layer.crs())

        if byFeature:
            self.featureOmbb(layer, writer, progress)
        else:
            self.layerOmmb(layer, writer, progress)

        del writer

    def layerOmmb(self, layer, writer, progress):
        current = 0
        vprovider = layer.dataProvider()

        fit = vprovider.getFeatures()
        inFeat = QgsFeature()
        total = 100.0 / vprovider.featureCount()
        newgeometry = QgsGeometry()
        first = True
        while fit.nextFeature(inFeat):
            if first:
                newgeometry = QgsGeometry(inFeat.geometry())
                first = False
            else:
                newgeometry = newgeometry.combine(inFeat.geometry())
            current += 1
            progress.setPercentage(int(current * total))

        geometry, area, perim, angle, width, height = self.OMBBox(newgeometry)

        if geometry:
            outFeat = QgsFeature()

            outFeat.setGeometry(geometry)
            outFeat.setAttributes([area,
                                   perim,
                                   angle,
                                   width,
                                   height])
            writer.addFeature(outFeat)

    def featureOmbb(self, layer, writer, progress):
        features = vector.features(layer)
        total = 100.0 / len(features)
        outFeat = QgsFeature()
        for current, inFeat in enumerate(features):
            geometry, area, perim, angle, width, height = self.OMBBox(
                inFeat.geometry())
            if geometry:
                outFeat.setGeometry(geometry)
                outFeat.setAttributes([area,
                                       perim,
                                       angle,
                                       width,
                                       height])
                writer.addFeature(outFeat)
            else:
                progress.setInfo(self.tr("Can't calculate an OMBB for feature {0}.").format(inFeat.id()))
            progress.setPercentage(int(current * total))

    def GetAngleOfLineBetweenTwoPoints(self, p1, p2, angle_unit="degrees"):
        xDiff = p2.x() - p1.x()
        yDiff = p2.y() - p1.y()
        if angle_unit == "radians":
            return atan2(yDiff, xDiff)
        else:
            return degrees(atan2(yDiff, xDiff))

    def OMBBox(self, geom):
        g = geom.convexHull()

        if g.type() != QGis.Polygon:
            return None, None, None, None, None, None
        r = g.asPolygon()[0]

        p0 = QgsPoint(r[0][0], r[0][1])

        i = 0
        l = len(r)
        OMBBox = QgsGeometry()
        gBBox = g.boundingBox()
        OMBBox_area = gBBox.height() * gBBox.width()
        OMBBox_angle = 0
        OMBBox_width = 0
        OMBBox_heigth = 0
        OMBBox_perim = 0
        while i < l - 1:
            x = QgsGeometry(g)
            angle = self.GetAngleOfLineBetweenTwoPoints(r[i], r[i + 1])
            x.rotate(angle, p0)
            bbox = x.boundingBox()
            bb = QgsGeometry.fromWkt(bbox.asWktPolygon())
            bb.rotate(-angle, p0)

            areabb = bb.area()
            if areabb <= OMBBox_area:
                OMBBox = QgsGeometry(bb)
                OMBBox_area = areabb
                OMBBox_angle = angle
                OMBBox_width = bbox.width()
                OMBBox_heigth = bbox.height()
                OMBBox_perim = 2 * OMBBox_width + 2 * OMBBox_heigth
            i += 1

        return OMBBox, OMBBox_area, OMBBox_perim, OMBBox_angle, OMBBox_width, OMBBox_heigth
