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

from qgis.core import (QgsFeature,
                       QgsGeometry,
                       QgsGeometryCollection,
                       QgsMultiLineString,
                       QgsMultiCurve,
                       QgsWkbTypes,
                       QgsFeatureSink,
                       QgsProcessing,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingUtils)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.tools import dataobjects

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class PolygonsToLines(QgisAlgorithm):

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
                                                              [QgsProcessing.TypeVectorPolygon]))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT,
                                                            self.tr('Polygons to lines'),
                                                            QgsProcessing.TypeVectorLine))

    def name(self):
        return 'polygonstolines'

    def displayName(self):
        return self.tr('Polygons to lines')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)

        geomType = self.convertWkbToLines(source.wkbType())

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               source.fields(), geomType, source.sourceCrs())

        outFeat = QgsFeature()

        total = 100.0 / source.featureCount() if source.featureCount() else 0
        count = 0

        for feat in source.getFeatures():
            if feedback.isCanceled():
                break

            if feat.hasGeometry():
                outFeat.setGeometry(QgsGeometry(self.convertToLines(feat.geometry())))
                attrs = feat.attributes()
                outFeat.setAttributes(attrs)
                sink.addFeature(outFeat, QgsFeatureSink.FastInsert)
            else:
                sink.addFeature(feat, QgsFeatureSink.FastInsert)

            count += 1
            feedback.setProgress(int(count * total))

        return {self.OUTPUT: dest_id}

    def convertWkbToLines(self, wkb):
        multi_wkb = None
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
        rings = self.getRings(geometry.geometry())
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
