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
                       QgsProcessing,
                       QgsProcessingParameterString,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterEnum)
from processing.algs.qgis.QgisAlgorithm import QgisFeatureBasedAlgorithm


class AddTableField(QgisFeatureBasedAlgorithm):

    FIELD_NAME = 'FIELD_NAME'
    FIELD_TYPE = 'FIELD_TYPE'
    FIELD_LENGTH = 'FIELD_LENGTH'
    FIELD_PRECISION = 'FIELD_PRECISION'

    TYPES = [QVariant.Int, QVariant.Double, QVariant.String]

    def group(self):
        return self.tr('Vector table')

    def __init__(self):
        super().__init__()
        self.type_names = [self.tr('Integer'),
                           self.tr('Float'),
                           self.tr('String')]
        self.field = None

    def initParameters(self, config=None):
        self.addParameter(QgsProcessingParameterString(self.FIELD_NAME,
                                                       self.tr('Field name')))
        self.addParameter(QgsProcessingParameterEnum(self.FIELD_TYPE,
                                                     self.tr('Field type'), self.type_names))
        self.addParameter(QgsProcessingParameterNumber(self.FIELD_LENGTH,
                                                       self.tr('Field length'), QgsProcessingParameterNumber.Integer,
                                                       10, False, 1, 255))
        self.addParameter(QgsProcessingParameterNumber(self.FIELD_PRECISION,
                                                       self.tr('Field precision'), QgsProcessingParameterNumber.Integer, 0, False, 0, 10))

    def name(self):
        return 'addfieldtoattributestable'

    def displayName(self):
        return self.tr('Add field to attributes table')

    def outputName(self):
        return self.tr('Added')

    def inputLayerTypes(self):
        return [QgsProcessing.TypeVector]

    def prepareAlgorithm(self, parameters, context, feedback):
        field_type = self.parameterAsEnum(parameters, self.FIELD_TYPE, context)
        field_name = self.parameterAsString(parameters, self.FIELD_NAME, context)
        field_length = self.parameterAsInt(parameters, self.FIELD_LENGTH, context)
        field_precision = self.parameterAsInt(parameters, self.FIELD_PRECISION, context)

        self.field = QgsField(field_name, self.TYPES[field_type], '',
                              field_length, field_precision)
        return True

    def outputFields(self, inputFields):
        inputFields.append(self.field)
        return inputFields

    def processFeature(self, feature, feedback):
        attributes = feature.attributes()
        attributes.append(None)
        feature.setAttributes(attributes)
        return feature
