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

from qgis.core import (QgsFeature,
                       QgsGeometry,
                       QgsGeometryCollection,
                       QgsPolygonV2,
                       QgsMultiPolygonV2,
                       QgsMultiSurface,
                       QgsWkbTypes,
                       QgsFeatureSink,
                       QgsProcessing,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingUtils)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.tools import dataobjects, vector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class LinesToPolygons(QgisAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'to_lines.png'))

    def tags(self):
        return self.tr('line,polygon,convert').split(',')

    def group(self):
        return self.tr('Vector geometry tools')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer'),
                                                              [QgsProcessing.TypeVectorLine]))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT,
                                                            self.tr('Lines to polygons'),
                                                            QgsProcessing.TypeVectorPolygon))

    def name(self):
        return 'linestopolygons'

    def displayName(self):
        return self.tr('Lines to polygons')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)

        geomType = self.convertWkbToPolygons(source.wkbType())

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               source.fields(), geomType, source.sourceCrs())

        outFeat = QgsFeature()

        total = 100.0 / source.featureCount() if source.featureCount() else 0
        count = 0

        for feat in source.getFeatures():
            if feedback.isCanceled():
                break

            if feat.hasGeometry():
                outFeat.setGeometry(QgsGeometry(self.convertToPolygons(feat.geometry())))
                attrs = feat.attributes()
                outFeat.setAttributes(attrs)
                sink.addFeature(outFeat, QgsFeatureSink.FastInsert)
                if outFeat.geometry().isEmpty():
                    feedback.reportError(self.tr("One or more line ignored due to geometry not having a minimum of three vertices."))
            else:
                sink.addFeature(feat, QgsFeatureSink.FastInsert)

            count += 1
            feedback.setProgress(int(count * total))

        return {self.OUTPUT: dest_id}

    def convertWkbToPolygons(self, wkb):
        multi_wkb = None
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
        surfaces = self.getSurfaces(geometry.geometry())
        output_wkb = self.convertWkbToPolygons(geometry.wkbType())
        out_geom = None
        if QgsWkbTypes.flatType(output_wkb) == QgsWkbTypes.MultiPolygon:
            out_geom = QgsMultiPolygonV2()
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
                surface = QgsPolygonV2()
                surface.setExteriorRing(geometry.clone())
                surfaces.append(surface)

        return surfaces
