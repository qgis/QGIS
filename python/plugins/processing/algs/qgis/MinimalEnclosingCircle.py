# -*- coding: utf-8 -*-

"""
***************************************************************************
    MinimalEnclosingCircle.py
    ---------------------
    Date                 : September 2017
    Copyright            : (C) 2017, Loïc BARTOLETTI
    Email                : lbartoletti at tuxfamily dot org
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
__date__ = 'September 2017'
__copyright__ = '(C) 2017, Loïc BARTOLETTI'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.PyQt.QtCore import QVariant
from qgis.core import (QgsField,
                       QgsFields,
                       QgsFeatureSink,
                       QgsGeometry,
                       QgsFeature,
                       QgsWkbTypes,
                       QgsFeatureRequest,
                       QgsProcessing,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingException)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class MinimalEnclosingCircle(QgisAlgorithm):

    INPUT = 'INPUT'
    BY_FEATURE = 'BY_FEATURE'

    OUTPUT = 'OUTPUT'

    def group(self):
        return self.tr('Vector general')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer'), [QgsProcessing.TypeVectorAnyGeometry]))
        self.addParameter(QgsProcessingParameterBoolean(self.BY_FEATURE,
                                                        self.tr('Calculate bounds for each feature separately'), defaultValue=True))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Enclosing circles'), QgsProcessing.TypeVectorPolygon))

    def name(self):
        return 'minimalenclosingcircle'

    def displayName(self):
        return self.tr('Minimal enclosing circle')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        by_feature = self.parameterAsBool(parameters, self.BY_FEATURE, context)

        if not by_feature and QgsWkbTypes.geometryType(source.wkbType()) == QgsWkbTypes.PointGeometry and source.featureCount() <= 2:
            raise QgsProcessingException(self.tr("Can't calculate a minimal enclosing circle for each point, it's a point. The number of points must be greater than 2"))

        if by_feature:
            fields = source.fields()
        else:
            fields = QgsFields()
        fields.append(QgsField('center_x', QVariant.Double))
        fields.append(QgsField('center_y', QVariant.Double))
        fields.append(QgsField('radius', QVariant.Double))

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               fields, QgsWkbTypes.Polygon, source.sourceCrs())

        if by_feature:
            self.featureMec(source, context, sink, feedback)
        else:
            self.layerMec(source, context, sink, feedback)

        return {self.OUTPUT: dest_id}

    def layerMec(self, source, context, sink, feedback):
        req = QgsFeatureRequest().setSubsetOfAttributes([])
        features = source.getFeatures(req)
        total = 100.0 / source.featureCount() if source.featureCount() else 0
        newgeometry = QgsGeometry()
        first = True
        geometries = []
        for current, inFeat in enumerate(features):
            if feedback.isCanceled():
                break

            if inFeat.hasGeometry():
                geometries.append(inFeat.geometry())
            feedback.setProgress(int(current * total))

        newgeometry = QgsGeometry.unaryUnion(geometries)
        geometry, center, radius = newgeometry.minimalEnclosingCircle()

        if geometry:
            outFeat = QgsFeature()

            outFeat.setGeometry(geometry)
            outFeat.setAttributes([center.x(),
                                   center.y(),
                                   radius])
            sink.addFeature(outFeat, QgsFeatureSink.FastInsert)

    def featureMec(self, source, context, sink, feedback):
        features = source.getFeatures()
        total = 100.0 / source.featureCount() if source.featureCount() else 0
        outFeat = QgsFeature()
        for current, inFeat in enumerate(features):
            if feedback.isCanceled():
                break

            geometry, center, radius = inFeat.geometry().minimalEnclosingCircle()
            if geometry:
                outFeat.setGeometry(geometry)
                attrs = inFeat.attributes()
                attrs.extend([center.x(),
                              center.y(),
                              radius])
                outFeat.setAttributes(attrs)
                sink.addFeature(outFeat, QgsFeatureSink.FastInsert)
            else:
                feedback.pushInfo(self.tr("Can't calculate a minimal enclosing circle for feature {0}.").format(inFeat.id()))
            feedback.setProgress(int(current * total))
