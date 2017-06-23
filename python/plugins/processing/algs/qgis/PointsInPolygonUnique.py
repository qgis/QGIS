# -*- coding: utf-8 -*-

"""
***************************************************************************
    PointsInPolygon.py
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

from qgis.PyQt.QtCore import QVariant
from qgis.core import (QgsApplication,
                       QgsField,
                       QgsFeatureRequest,
                       QgsFeatureSink,
                       QgsFeature,
                       QgsGeometry,
                       QgsProcessingUtils)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterString
from processing.core.parameters import ParameterTableField
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector


class PointsInPolygonUnique(QgisAlgorithm):

    POLYGONS = 'POLYGONS'
    POINTS = 'POINTS'
    OUTPUT = 'OUTPUT'
    FIELD = 'FIELD'
    CLASSFIELD = 'CLASSFIELD'

    def icon(self):
        return QgsApplication.getThemeIcon("/providerQgis.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("providerQgis.svg")

    def group(self):
        return self.tr('Vector analysis tools')

    def __init__(self):
        super().__init__()
        self.addParameter(ParameterVector(self.POLYGONS,
                                          self.tr('Polygons'), [dataobjects.TYPE_VECTOR_POLYGON]))
        self.addParameter(ParameterVector(self.POINTS,
                                          self.tr('Points'), [dataobjects.TYPE_VECTOR_POINT]))
        self.addParameter(ParameterTableField(self.CLASSFIELD,
                                              self.tr('Class field'), self.POINTS))
        self.addParameter(ParameterString(self.FIELD,
                                          self.tr('Count field name'), 'NUMPOINTS'))
        self.addOutput(OutputVector(self.OUTPUT, self.tr('Unique count'), datatype=[dataobjects.TYPE_VECTOR_POLYGON]))

    def name(self):
        return 'countuniquepointsinpolygon'

    def displayName(self):
        return self.tr('Count unique points in polygon')

    def processAlgorithm(self, parameters, context, feedback):
        polyLayer = QgsProcessingUtils.mapLayerFromString(self.getParameterValue(self.POLYGONS), context)
        pointLayer = QgsProcessingUtils.mapLayerFromString(self.getParameterValue(self.POINTS), context)
        fieldName = self.getParameterValue(self.FIELD)
        classFieldName = self.getParameterValue(self.CLASSFIELD)

        fields = polyLayer.fields()
        fields.append(QgsField(fieldName, QVariant.Int))

        classFieldIndex = pointLayer.fields().lookupField(classFieldName)
        (idxCount, fieldList) = vector.findOrCreateField(polyLayer,
                                                         polyLayer.fields(), fieldName)

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(fields, polyLayer.wkbType(),
                                                                     polyLayer.crs(), context)

        spatialIndex = QgsProcessingUtils.createSpatialIndex(pointLayer, context)

        ftPoint = QgsFeature()
        outFeat = QgsFeature()
        geom = QgsGeometry()

        features = QgsProcessingUtils.getFeatures(polyLayer, context)
        total = 100.0 / polyLayer.featureCount() if polyLayer.featureCount() else 0
        for current, ftPoly in enumerate(features):
            geom = ftPoly.geometry()
            engine = QgsGeometry.createGeometryEngine(geom.geometry())
            engine.prepareGeometry()

            attrs = ftPoly.attributes()

            classes = set()
            points = spatialIndex.intersects(geom.boundingBox())
            if len(points) > 0:
                request = QgsFeatureRequest().setFilterFids(points).setSubsetOfAttributes([classFieldIndex])
                fit = pointLayer.getFeatures(request)
                ftPoint = QgsFeature()
                while fit.nextFeature(ftPoint):
                    tmpGeom = QgsGeometry(ftPoint.geometry())
                    if engine.contains(tmpGeom.geometry()):
                        clazz = ftPoint.attributes()[classFieldIndex]
                        if clazz not in classes:
                            classes.add(clazz)

            outFeat.setGeometry(geom)
            if idxCount == len(attrs):
                attrs.append(len(classes))
            else:
                attrs[idxCount] = len(classes)
            outFeat.setAttributes(attrs)
            writer.addFeature(outFeat, QgsFeatureSink.FastInsert)

            feedback.setProgress(int(current * total))

        del writer
