# -*- coding: utf-8 -*-

"""
***************************************************************************
    AddTableField.py
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
from qgis.core import (QgsField,
                       QgsFeature,
                       QgsFeatureSink,
                       QgsApplication,
                       QgsProcessingUtils,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterString,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingOutputVectorLayer)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class AddTableField(QgisAlgorithm):

    OUTPUT_LAYER = 'OUTPUT_LAYER'
    INPUT_LAYER = 'INPUT_LAYER'
    FIELD_NAME = 'FIELD_NAME'
    FIELD_TYPE = 'FIELD_TYPE'
    FIELD_LENGTH = 'FIELD_LENGTH'
    FIELD_PRECISION = 'FIELD_PRECISION'

    TYPES = [QVariant.Int, QVariant.Double, QVariant.String]

    def group(self):
        return self.tr('Vector table tools')

    def __init__(self):
        super().__init__()
        self.type_names = [self.tr('Integer'),
                           self.tr('Float'),
                           self.tr('String')]

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT_LAYER,
                                                              self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterString(self.FIELD_NAME,
                                                       self.tr('Field name')))
        self.addParameter(QgsProcessingParameterEnum(self.FIELD_TYPE,
                                                     self.tr('Field type'), self.type_names))
        self.addParameter(QgsProcessingParameterNumber(self.FIELD_LENGTH,
                                                       self.tr('Field length'), QgsProcessingParameterNumber.Integer,
                                                       10, False, 1, 255))
        self.addParameter(QgsProcessingParameterNumber(self.FIELD_PRECISION,
                                                       self.tr('Field precision'), QgsProcessingParameterNumber.Integer, 0, False, 0, 10))
        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT_LAYER, self.tr('Added')))
        self.addOutput(QgsProcessingOutputVectorLayer(self.OUTPUT_LAYER, self.tr('Added')))

    def name(self):
        return 'addfieldtoattributestable'

    def displayName(self):
        return self.tr('Add field to attributes table')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT_LAYER, context)

        fieldType = self.parameterAsEnum(parameters, self.FIELD_TYPE, context)
        fieldName = self.parameterAsString(parameters, self.FIELD_NAME, context)
        fieldLength = self.parameterAsInt(parameters, self.FIELD_LENGTH, context)
        fieldPrecision = self.parameterAsInt(parameters, self.FIELD_PRECISION, context)

        fields = source.fields()
        fields.append(QgsField(fieldName, self.TYPES[fieldType], '',
                               fieldLength, fieldPrecision))
        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT_LAYER, context,
                                               fields, source.wkbType(), source.sourceCrs())

        features = source.getFeatures()
        total = 100.0 / source.featureCount() if source.featureCount() else 0

        for current, input_feature in enumerate(features):
            if feedback.isCanceled():
                break

            output_feature = input_feature
            attributes = input_feature.attributes()
            attributes.append(None)
            output_feature.setAttributes(attributes)

            sink.addFeature(output_feature, QgsFeatureSink.FastInsert)
            feedback.setProgress(int(current * total))

        return {self.OUTPUT_LAYER: dest_id}
