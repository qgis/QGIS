"""
***************************************************************************
    CheckValidity.py
    ---------------------
    Date                 : May 2015
    Copyright            : (C) 2015 by Arnaud Morvan
    Email                : arnaud dot morvan at camptocamp dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Arnaud Morvan'
__date__ = 'May 2015'
__copyright__ = '(C) 2015, Arnaud Morvan'

import os

from qgis.PyQt.QtGui import QIcon
from qgis.PyQt.QtCore import QVariant

from qgis.core import (Qgis,
                       QgsApplication,
                       QgsSettings,
                       QgsGeometry,
                       QgsFeature,
                       QgsField,
                       QgsFeatureRequest,
                       QgsFeatureSink,
                       QgsWkbTypes,
                       QgsFields,
                       QgsProcessing,
                       QgsProcessingException,
                       QgsProcessingFeatureSource,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingOutputNumber,
                       QgsProcessingParameterBoolean)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm

settings_method_key = "/digitizing/validate-geometries"
pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class CheckValidity(QgisAlgorithm):
    INPUT_LAYER = 'INPUT_LAYER'
    METHOD = 'METHOD'
    VALID_OUTPUT = 'VALID_OUTPUT'
    VALID_COUNT = 'VALID_COUNT'
    INVALID_OUTPUT = 'INVALID_OUTPUT'
    INVALID_COUNT = 'INVALID_COUNT'
    ERROR_OUTPUT = 'ERROR_OUTPUT'
    ERROR_COUNT = 'ERROR_COUNT'
    IGNORE_RING_SELF_INTERSECTION = 'IGNORE_RING_SELF_INTERSECTION'

    def icon(self):
        return QgsApplication.getThemeIcon("/algorithms/mAlgorithmCheckGeometry.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("/algorithms/mAlgorithmCheckGeometry.svg")

    def group(self):
        return self.tr('Vector geometry')

    def groupId(self):
        return 'vectorgeometry'

    def tags(self):
        return self.tr('valid,invalid,detect,error').split(',')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.methods = [self.tr('The one selected in digitizing settings'),
                        'QGIS',
                        'GEOS']

        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT_LAYER,
                                                              self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterEnum(self.METHOD,
                                                     self.tr('Method'), self.methods, defaultValue=2))
        self.parameterDefinition(self.METHOD).setMetadata({
            'widget_wrapper': {
                'useCheckBoxes': True,
                'columns': 3}})

        self.addParameter(QgsProcessingParameterBoolean(self.IGNORE_RING_SELF_INTERSECTION,
                                                        self.tr('Ignore ring self intersections'), defaultValue=False))

        self.addParameter(QgsProcessingParameterFeatureSink(self.VALID_OUTPUT, self.tr('Valid output'), QgsProcessing.SourceType.TypeVectorAnyGeometry, None, True))
        self.addOutput(QgsProcessingOutputNumber(self.VALID_COUNT, self.tr('Count of valid features')))

        self.addParameter(QgsProcessingParameterFeatureSink(self.INVALID_OUTPUT, self.tr('Invalid output'), QgsProcessing.SourceType.TypeVectorAnyGeometry, None, True))
        self.addOutput(QgsProcessingOutputNumber(self.INVALID_COUNT, self.tr('Count of invalid features')))

        self.addParameter(QgsProcessingParameterFeatureSink(self.ERROR_OUTPUT, self.tr('Error output'), QgsProcessing.SourceType.TypeVectorAnyGeometry, None, True))
        self.addOutput(QgsProcessingOutputNumber(self.ERROR_COUNT, self.tr('Count of errors')))

    def name(self):
        return 'checkvalidity'

    def displayName(self):
        return self.tr('Check validity')

    def processAlgorithm(self, parameters, context, feedback):
        ignore_ring_self_intersection = self.parameterAsBoolean(parameters, self.IGNORE_RING_SELF_INTERSECTION, context)
        method_param = self.parameterAsEnum(parameters, self.METHOD, context)
        if method_param == 0:
            settings = QgsSettings()
            method = int(settings.value(settings_method_key, 0)) - 1
            method = max(method, 0)
        else:
            method = method_param - 1

        return self.doCheck(
            method, parameters, context, feedback, ignore_ring_self_intersection
        )

    def doCheck(self, method, parameters, context, feedback, ignore_ring_self_intersection):
        flags = QgsGeometry.ValidityFlag.FlagAllowSelfTouchingHoles if ignore_ring_self_intersection else QgsGeometry.ValidityFlags()
        source = self.parameterAsSource(parameters, self.INPUT_LAYER, context)
        if source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.INPUT_LAYER))

        (valid_output_sink, valid_output_dest_id) = self.parameterAsSink(parameters, self.VALID_OUTPUT, context,
                                                                         source.fields(), source.wkbType(), source.sourceCrs())
        valid_count = 0

        invalid_fields = source.fields()
        invalid_fields.append(QgsField('_errors', QVariant.String, 'string', 255))
        (invalid_output_sink, invalid_output_dest_id) = self.parameterAsSink(parameters, self.INVALID_OUTPUT, context,
                                                                             invalid_fields, source.wkbType(), source.sourceCrs())
        invalid_count = 0

        error_fields = QgsFields()
        error_fields.append(QgsField('message', QVariant.String, 'string', 255))
        (error_output_sink, error_output_dest_id) = self.parameterAsSink(parameters, self.ERROR_OUTPUT, context,
                                                                         error_fields, QgsWkbTypes.Type.Point, source.sourceCrs())
        error_count = 0

        features = source.getFeatures(QgsFeatureRequest(), QgsProcessingFeatureSource.Flag.FlagSkipGeometryValidityChecks)
        total = 100.0 / source.featureCount() if source.featureCount() else 0
        for current, inFeat in enumerate(features):
            if feedback.isCanceled():
                break
            geom = inFeat.geometry()
            attrs = inFeat.attributes()

            valid = True
            if not geom.isNull() and not geom.isEmpty():
                errors = list(geom.validateGeometry(Qgis.GeometryValidationEngine(method), flags))
                if errors:
                    valid = False
                    reasons = []
                    for error in errors:
                        errFeat = QgsFeature()
                        error_geom = QgsGeometry.fromPointXY(error.where())
                        errFeat.setGeometry(error_geom)
                        errFeat.setAttributes([error.what()])
                        if error_output_sink:
                            error_output_sink.addFeature(errFeat, QgsFeatureSink.Flag.FastInsert)
                        error_count += 1

                        reasons.append(error.what())

                    reason = "\n".join(reasons)
                    if len(reason) > 255:
                        reason = reason[:252] + '…'
                    attrs.append(reason)

            outFeat = QgsFeature()
            outFeat.setGeometry(geom)
            outFeat.setAttributes(attrs)

            if valid:
                if valid_output_sink:
                    valid_output_sink.addFeature(outFeat, QgsFeatureSink.Flag.FastInsert)
                valid_count += 1

            else:
                if invalid_output_sink:
                    invalid_output_sink.addFeature(outFeat, QgsFeatureSink.Flag.FastInsert)
                invalid_count += 1

            feedback.setProgress(int(current * total))

        results = {
            self.VALID_COUNT: valid_count,
            self.INVALID_COUNT: invalid_count,
            self.ERROR_COUNT: error_count
        }
        if valid_output_sink:
            results[self.VALID_OUTPUT] = valid_output_dest_id
        if invalid_output_sink:
            results[self.INVALID_OUTPUT] = invalid_output_dest_id
        if error_output_sink:
            results[self.ERROR_OUTPUT] = error_output_dest_id
        return results
