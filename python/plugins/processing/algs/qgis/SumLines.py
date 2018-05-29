# -*- coding: utf-8 -*-

"""
***************************************************************************
    SumLines.py
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
from qgis.PyQt.QtCore import QVariant
from qgis.core import (QgsApplication,
                       QgsFeature,
                       QgsFeatureSink,
                       QgsField,
                       QgsGeometry,
                       QgsFeatureRequest,
                       QgsDistanceArea,
                       QgsProject,
                       QgsProcessing,
                       QgsProcessingException,
                       QgsProcessingParameterString,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink,
                       QgsSpatialIndex)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class SumLines(QgisAlgorithm):

    LINES = 'LINES'
    POLYGONS = 'POLYGONS'
    LEN_FIELD = 'LEN_FIELD'
    COUNT_FIELD = 'COUNT_FIELD'
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QgsApplication.getThemeIcon("/algorithms/mAlgorithmSumLengthLines.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("/algorithms/mAlgorithmSumLengthLines.svg")

    def group(self):
        return self.tr('Vector analysis')

    def groupId(self):
        return 'vectoranalysis'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.LINES,
                                                              self.tr('Lines'), [QgsProcessing.TypeVectorLine]))
        self.addParameter(QgsProcessingParameterFeatureSource(self.POLYGONS,
                                                              self.tr('Polygons'), [QgsProcessing.TypeVectorPolygon]))
        self.addParameter(QgsProcessingParameterString(self.LEN_FIELD,
                                                       self.tr('Lines length field name'), defaultValue='LENGTH'))
        self.addParameter(QgsProcessingParameterString(self.COUNT_FIELD,
                                                       self.tr('Lines count field name'), defaultValue='COUNT'))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Line length'), QgsProcessing.TypeVectorPolygon))

    def name(self):
        return 'sumlinelengths'

    def displayName(self):
        return self.tr('Sum line lengths')

    def processAlgorithm(self, parameters, context, feedback):
        line_source = self.parameterAsSource(parameters, self.LINES, context)
        if line_source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.LINES))

        poly_source = self.parameterAsSource(parameters, self.POLYGONS, context)
        if poly_source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.POLYGONS))

        length_field_name = self.parameterAsString(parameters, self.LEN_FIELD, context)
        count_field_name = self.parameterAsString(parameters, self.COUNT_FIELD, context)

        fields = poly_source.fields()
        if fields.lookupField(length_field_name) < 0:
            fields.append(QgsField(length_field_name, QVariant.Double))
        length_field_index = fields.lookupField(length_field_name)
        if fields.lookupField(count_field_name) < 0:
            fields.append(QgsField(count_field_name, QVariant.Int))
        count_field_index = fields.lookupField(count_field_name)

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               fields, poly_source.wkbType(), poly_source.sourceCrs())
        if sink is None:
            raise QgsProcessingException(self.invalidSinkError(parameters, self.OUTPUT))

        spatialIndex = QgsSpatialIndex(line_source.getFeatures(
            QgsFeatureRequest().setSubsetOfAttributes([]).setDestinationCrs(poly_source.sourceCrs(), context.transformContext())), feedback)

        distArea = QgsDistanceArea()
        distArea.setSourceCrs(poly_source.sourceCrs(), context.transformContext())
        distArea.setEllipsoid(context.project().ellipsoid())

        features = poly_source.getFeatures()
        total = 100.0 / poly_source.featureCount() if poly_source.featureCount() else 0
        for current, poly_feature in enumerate(features):
            if feedback.isCanceled():
                break

            output_feature = QgsFeature()
            count = 0
            length = 0
            if poly_feature.hasGeometry():
                poly_geom = poly_feature.geometry()
                has_intersections = False
                lines = spatialIndex.intersects(poly_geom.boundingBox())
                engine = None
                if len(lines) > 0:
                    has_intersections = True
                    # use prepared geometries for faster intersection tests
                    engine = QgsGeometry.createGeometryEngine(poly_geom.constGet())
                    engine.prepareGeometry()

                if has_intersections:
                    request = QgsFeatureRequest().setFilterFids(lines).setSubsetOfAttributes([]).setDestinationCrs(poly_source.sourceCrs(), context.transformContext())
                    for line_feature in line_source.getFeatures(request):
                        if feedback.isCanceled():
                            break

                        if engine.intersects(line_feature.geometry().constGet()):
                            outGeom = poly_geom.intersection(line_feature.geometry())
                            length += distArea.measureLength(outGeom)
                            count += 1

                output_feature.setGeometry(poly_geom)

            attrs = poly_feature.attributes()
            if length_field_index == len(attrs):
                attrs.append(length)
            else:
                attrs[length_field_index] = length
            if count_field_index == len(attrs):
                attrs.append(count)
            else:
                attrs[count_field_index] = count
            output_feature.setAttributes(attrs)
            sink.addFeature(output_feature, QgsFeatureSink.FastInsert)

            feedback.setProgress(int(current * total))

        return {self.OUTPUT: dest_id}
