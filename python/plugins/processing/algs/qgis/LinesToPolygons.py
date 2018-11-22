# -*- coding: utf-8 -*-

"""
***************************************************************************
    LinesToPolygons.py
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
                       QgsFeature,
                       QgsGeometry,
                       QgsGeometryCollection,
                       QgsPolygon,
                       QgsMultiPolygon,
                       QgsMultiSurface,
                       QgsWkbTypes,
                       QgsFeatureSink,
                       QgsProcessing,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingUtils)

from processing.algs.qgis.QgisAlgorithm import QgisFeatureBasedAlgorithm
from processing.tools import dataobjects, vector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class LinesToPolygons(QgisFeatureBasedAlgorithm):

    def icon(self):
        return QgsApplication.getThemeIcon("/algorithms/mAlgorithmLineToPolygon.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("/algorithms/mAlgorithmLineToPolygon.svg")

    def tags(self):
        return self.tr('line,polygon,convert').split(',')

    def group(self):
        return self.tr('Vector geometry')

    def groupId(self):
        return 'vectorgeometry'

    def __init__(self):
        super().__init__()

    def name(self):
        return 'linestopolygons'

    def displayName(self):
        return self.tr('Lines to polygons')

    def outputName(self):
        return self.tr('Polygons')

    def outputType(self):
        return QgsProcessing.TypeVectorPolygon

    def inputLayerTypes(self):
        return [QgsProcessing.TypeVectorLine]

    def outputWkbType(self, input_wkb_type):
        return self.convertWkbToPolygons(input_wkb_type)

    def processFeature(self, feature, context, feedback):
        if feature.hasGeometry():
            feature.setGeometry(QgsGeometry(self.convertToPolygons(feature.geometry())))
            if feature.geometry().isEmpty():
                feedback.reportError(self.tr("One or more line ignored due to geometry not having a minimum of three vertices."))
        return [feature]

    def supportInPlaceEdit(self, layer):
        return False

    def convertWkbToPolygons(self, wkb):
        multi_wkb = QgsWkbTypes.NoGeometry
        if QgsWkbTypes.singleType(QgsWkbTypes.flatType(wkb)) == QgsWkbTypes.LineString:
            multi_wkb = QgsWkbTypes.MultiPolygon
        elif QgsWkbTypes.singleType(QgsWkbTypes.flatType(wkb)) == QgsWkbTypes.CompoundCurve:
            multi_wkb = QgsWkbTypes.MultiSurface
        if QgsWkbTypes.hasM(wkb):
            multi_wkb = QgsWkbTypes.addM(multi_wkb)
        if QgsWkbTypes.hasZ(wkb):
            multi_wkb = QgsWkbTypes.addZ(multi_wkb)

        return multi_wkb

    def convertToPolygons(self, geometry):
        surfaces = self.getSurfaces(geometry.constGet())
        output_wkb = self.convertWkbToPolygons(geometry.wkbType())
        out_geom = None
        if QgsWkbTypes.flatType(output_wkb) == QgsWkbTypes.MultiPolygon:
            out_geom = QgsMultiPolygon()
        else:
            out_geom = QgsMultiSurface()

        for surface in surfaces:
            out_geom.addGeometry(surface)

        return out_geom

    def getSurfaces(self, geometry):
        surfaces = []
        if isinstance(geometry, QgsGeometryCollection):
            # collection
            for i in range(geometry.numGeometries()):
                surfaces.extend(self.getSurfaces(geometry.geometryN(i)))
        else:
            # not collection
            if geometry.vertexCount() > 2:
                surface = QgsPolygon()
                surface.setExteriorRing(geometry.clone())
                surfaces.append(surface)

        return surfaces
