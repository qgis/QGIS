# -*- coding: utf-8 -*-

"""
***************************************************************************
    PolygonsToLines.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt.QtGui import QIcon

from qgis.core import (QgsApplication,
                       QgsGeometry,
                       QgsGeometryCollection,
                       QgsMultiLineString,
                       QgsMultiCurve,
                       QgsWkbTypes,
                       QgsProcessing)

from processing.algs.qgis.QgisAlgorithm import QgisFeatureBasedAlgorithm

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class PolygonsToLines(QgisFeatureBasedAlgorithm):

    def icon(self):
        return QgsApplication.getThemeIcon("/algorithms/mAlgorithmPolygonToLine.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("/algorithms/mAlgorithmPolygonToLine.svg")

    def tags(self):
        return self.tr('line,polygon,convert').split(',')

    def group(self):
        return self.tr('Vector geometry')

    def groupId(self):
        return 'vectorgeometry'

    def __init__(self):
        super().__init__()

    def name(self):
        return 'polygonstolines'

    def displayName(self):
        return self.tr('Polygons to lines')

    def outputName(self):
        return self.tr('Lines')

    def outputType(self):
        return QgsProcessing.TypeVectorLine

    def inputLayerTypes(self):
        return [QgsProcessing.TypeVectorPolygon]

    def outputWkbType(self, input_wkb_type):
        return self.convertWkbToLines(input_wkb_type)

    def processFeature(self, feature, context, feedback):
        if feature.hasGeometry():
            feature.setGeometry(QgsGeometry(self.convertToLines(feature.geometry())))
        return [feature]

    def supportInPlaceEdit(self, layer):
        return False

    def convertWkbToLines(self, wkb):
        multi_wkb = QgsWkbTypes.NoGeometry
        if QgsWkbTypes.singleType(QgsWkbTypes.flatType(wkb)) == QgsWkbTypes.Polygon:
            multi_wkb = QgsWkbTypes.MultiLineString
        elif QgsWkbTypes.singleType(QgsWkbTypes.flatType(wkb)) == QgsWkbTypes.CurvePolygon:
            multi_wkb = QgsWkbTypes.MultiCurve
        if QgsWkbTypes.hasM(wkb):
            multi_wkb = QgsWkbTypes.addM(multi_wkb)
        if QgsWkbTypes.hasZ(wkb):
            multi_wkb = QgsWkbTypes.addZ(multi_wkb)

        return multi_wkb

    def convertToLines(self, geometry):
        rings = self.getRings(geometry.constGet())
        output_wkb = self.convertWkbToLines(geometry.wkbType())
        out_geom = None
        if QgsWkbTypes.flatType(output_wkb) == QgsWkbTypes.MultiLineString:
            out_geom = QgsMultiLineString()
        else:
            out_geom = QgsMultiCurve()

        for ring in rings:
            out_geom.addGeometry(ring)

        return out_geom

    def getRings(self, geometry):
        rings = []
        if isinstance(geometry, QgsGeometryCollection):
            # collection
            for i in range(geometry.numGeometries()):
                rings.extend(self.getRings(geometry.geometryN(i)))
        else:
            # not collection
            rings.append(geometry.exteriorRing().clone())
            for i in range(geometry.numInteriorRings()):
                rings.append(geometry.interiorRing(i).clone())

        return rings
