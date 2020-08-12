# -*- coding: utf-8 -*-

"""
***************************************************************************
    PointDistance.py
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

import os
import math

from qgis.PyQt.QtGui import QIcon
from qgis.PyQt.QtCore import QVariant

from qgis.core import (QgsApplication,
                       QgsFeatureRequest,
                       QgsField,
                       QgsFields,
                       QgsProject,
                       QgsFeature,
                       QgsGeometry,
                       QgsDistanceArea,
                       QgsFeatureSink,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessing,
                       QgsProcessingException,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterField,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterFeatureSink,
                       QgsSpatialIndex,
                       QgsWkbTypes)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class PointDistance(QgisAlgorithm):
    INPUT = 'INPUT'
    INPUT_FIELD = 'INPUT_FIELD'
    TARGET = 'TARGET'
    TARGET_FIELD = 'TARGET_FIELD'
    MATRIX_TYPE = 'MATRIX_TYPE'
    NEAREST_POINTS = 'NEAREST_POINTS'
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QgsApplication.getThemeIcon("/algorithms/mAlgorithmDistanceMatrix.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("/algorithms/mAlgorithmDistanceMatrix.svg")

    def group(self):
        return self.tr('Vector analysis')

    def groupId(self):
        return 'vectoranalysis'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.mat_types = [self.tr('Linear (N*k x 3) distance matrix'),
                          self.tr('Standard (N x T) distance matrix'),
                          self.tr('Summary distance matrix (mean, std. dev., min, max)')]

        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input point layer'),
                                                              [QgsProcessing.TypeVectorPoint]))
        self.addParameter(QgsProcessingParameterField(self.INPUT_FIELD,
                                                      self.tr('Input unique ID field'),
                                                      parentLayerParameterName=self.INPUT,
                                                      type=QgsProcessingParameterField.Any))
        self.addParameter(QgsProcessingParameterFeatureSource(self.TARGET,
                                                              self.tr('Target point layer'),
                                                              [QgsProcessing.TypeVectorPoint]))
        self.addParameter(QgsProcessingParameterField(self.TARGET_FIELD,
                                                      self.tr('Target unique ID field'),
                                                      parentLayerParameterName=self.TARGET,
                                                      type=QgsProcessingParameterField.Any))
        self.addParameter(QgsProcessingParameterEnum(self.MATRIX_TYPE,
                                                     self.tr('Output matrix type'), options=self.mat_types, defaultValue=0))
        self.addParameter(QgsProcessingParameterNumber(self.NEAREST_POINTS,
                                                       self.tr('Use only the nearest (k) target points'), type=QgsProcessingParameterNumber.Integer, minValue=0, defaultValue=0))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Distance matrix'), QgsProcessing.TypeVectorPoint))

    def name(self):
        return 'distancematrix'

    def displayName(self):
        return self.tr('Distance matrix')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        if source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.INPUT))

        if QgsWkbTypes.isMultiType(source.wkbType()):
            raise QgsProcessingException(self.tr('Input point layer is a MultiPoint layer - first convert to single points before using this algorithm.'))

        source_field = self.parameterAsString(parameters, self.INPUT_FIELD, context)
        target_source = self.parameterAsSource(parameters, self.TARGET, context)
        if target_source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.TARGET))

        if QgsWkbTypes.isMultiType(target_source.wkbType()):
            raise QgsProcessingException(self.tr('Target point layer is a MultiPoint layer - first convert to single points before using this algorithm.'))

        target_field = self.parameterAsString(parameters, self.TARGET_FIELD, context)
        same_source_and_target = parameters[self.INPUT] == parameters[self.TARGET]
        matType = self.parameterAsEnum(parameters, self.MATRIX_TYPE, context)
        nPoints = self.parameterAsInt(parameters, self.NEAREST_POINTS, context)

        if nPoints < 1:
            nPoints = target_source.featureCount()

        if matType == 0:
            # Linear distance matrix
            return self.linearMatrix(parameters, context, source, source_field, target_source, target_field, same_source_and_target,
                                     matType, nPoints, feedback)
        elif matType == 1:
            # Standard distance matrix
            return self.regularMatrix(parameters, context, source, source_field, target_source, target_field,
                                      nPoints, feedback)
        elif matType == 2:
            # Summary distance matrix
            return self.linearMatrix(parameters, context, source, source_field, target_source, target_field, same_source_and_target,
                                     matType, nPoints, feedback)

    def linearMatrix(self, parameters, context, source, inField, target_source, targetField, same_source_and_target,
                     matType, nPoints, feedback):

        if same_source_and_target:
            # need to fetch an extra point from the index, since the closest match will always be the same
            # as the input feature
            nPoints += 1

        inIdx = source.fields().lookupField(inField)
        outIdx = target_source.fields().lookupField(targetField)

        fields = QgsFields()
        input_id_field = source.fields()[inIdx]
        input_id_field.setName('InputID')
        fields.append(input_id_field)
        if matType == 0:
            target_id_field = target_source.fields()[outIdx]
            target_id_field.setName('TargetID')
            fields.append(target_id_field)
            fields.append(QgsField('Distance', QVariant.Double))
        else:
            fields.append(QgsField('MEAN', QVariant.Double))
            fields.append(QgsField('STDDEV', QVariant.Double))
            fields.append(QgsField('MIN', QVariant.Double))
            fields.append(QgsField('MAX', QVariant.Double))

        out_wkb = QgsWkbTypes.multiType(source.wkbType()) if matType == 0 else source.wkbType()
        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               fields, out_wkb, source.sourceCrs())
        if sink is None:
            raise QgsProcessingException(self.invalidSinkError(parameters, self.OUTPUT))

        index = QgsSpatialIndex(target_source.getFeatures(QgsFeatureRequest().setSubsetOfAttributes([]).setDestinationCrs(source.sourceCrs(), context.transformContext())), feedback)

        distArea = QgsDistanceArea()
        distArea.setSourceCrs(source.sourceCrs(), context.transformContext())
        distArea.setEllipsoid(context.ellipsoid())

        features = source.getFeatures(QgsFeatureRequest().setSubsetOfAttributes([inIdx]))
        total = 100.0 / source.featureCount() if source.featureCount() else 0
        for current, inFeat in enumerate(features):
            if feedback.isCanceled():
                break

            inGeom = inFeat.geometry()
            inID = str(inFeat[inIdx])
            featList = index.nearestNeighbor(inGeom.asPoint(), nPoints)
            distList = []
            vari = 0.0
            request = QgsFeatureRequest().setFilterFids(featList).setSubsetOfAttributes([outIdx]).setDestinationCrs(source.sourceCrs(), context.transformContext())
            for outFeat in target_source.getFeatures(request):
                if feedback.isCanceled():
                    break

                if same_source_and_target and inFeat.id() == outFeat.id():
                    continue

                outID = outFeat[outIdx]
                outGeom = outFeat.geometry()
                dist = distArea.measureLine(inGeom.asPoint(),
                                            outGeom.asPoint())

                if matType == 0:
                    out_feature = QgsFeature()
                    out_geom = QgsGeometry.unaryUnion([inFeat.geometry(), outFeat.geometry()])
                    out_feature.setGeometry(out_geom)
                    out_feature.setAttributes([inID, outID, dist])
                    sink.addFeature(out_feature, QgsFeatureSink.FastInsert)
                else:
                    distList.append(float(dist))

            if matType != 0:
                mean = sum(distList) / len(distList)
                for i in distList:
                    vari += (i - mean) * (i - mean)
                vari = math.sqrt(vari / len(distList))

                out_feature = QgsFeature()
                out_feature.setGeometry(inFeat.geometry())
                out_feature.setAttributes([inID, mean, vari, min(distList), max(distList)])
                sink.addFeature(out_feature, QgsFeatureSink.FastInsert)

            feedback.setProgress(int(current * total))

        return {self.OUTPUT: dest_id}

    def regularMatrix(self, parameters, context, source, inField, target_source, targetField,
                      nPoints, feedback):

        distArea = QgsDistanceArea()
        distArea.setSourceCrs(source.sourceCrs(), context.transformContext())
        distArea.setEllipsoid(context.ellipsoid())

        inIdx = source.fields().lookupField(inField)
        targetIdx = target_source.fields().lookupField(targetField)

        index = QgsSpatialIndex(target_source.getFeatures(QgsFeatureRequest().setSubsetOfAttributes([]).setDestinationCrs(source.sourceCrs(), context.transformContext())), feedback)

        first = True
        sink = None
        dest_id = None
        features = source.getFeatures(QgsFeatureRequest().setSubsetOfAttributes([inIdx]))
        total = 100.0 / source.featureCount() if source.featureCount() else 0
        for current, inFeat in enumerate(features):
            if feedback.isCanceled():
                break

            inGeom = inFeat.geometry()
            if first:
                featList = index.nearestNeighbor(inGeom.asPoint(), nPoints)
                first = False
                fields = QgsFields()
                input_id_field = source.fields()[inIdx]
                input_id_field.setName('ID')
                fields.append(input_id_field)
                for f in target_source.getFeatures(QgsFeatureRequest().setFilterFids(featList).setSubsetOfAttributes([targetIdx]).setDestinationCrs(source.sourceCrs(), context.transformContext())):
                    fields.append(QgsField(str(f[targetField]), QVariant.Double))

                (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                                       fields, source.wkbType(), source.sourceCrs())
                if sink is None:
                    raise QgsProcessingException(self.invalidSinkError(parameters, self.OUTPUT))

            data = [inFeat[inField]]
            for target in target_source.getFeatures(QgsFeatureRequest().setSubsetOfAttributes([]).setFilterFids(featList).setDestinationCrs(source.sourceCrs(), context.transformContext())):
                if feedback.isCanceled():
                    break
                outGeom = target.geometry()
                dist = distArea.measureLine(inGeom.asPoint(),
                                            outGeom.asPoint())
                data.append(dist)

            out_feature = QgsFeature()
            out_feature.setGeometry(inGeom)
            out_feature.setAttributes(data)
            sink.addFeature(out_feature, QgsFeatureSink.FastInsert)
            feedback.setProgress(int(current * total))

        return {self.OUTPUT: dest_id}
