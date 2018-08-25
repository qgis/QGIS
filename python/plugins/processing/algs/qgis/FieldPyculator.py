# -*- coding: utf-8 -*-

"""
***************************************************************************
    FieldPyculator.py
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

__author__ = 'Victor Olaya & NextGIS'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya & NextGIS'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import sys

from qgis.PyQt.QtCore import QVariant
from qgis.core import (QgsProcessingException,
                       QgsField,
                       QgsFeatureSink,
                       QgsProcessing,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterString,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterFeatureSink)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class FieldsPyculator(QgisAlgorithm):
    INPUT = 'INPUT'
    FIELD_NAME = 'FIELD_NAME'
    FIELD_TYPE = 'FIELD_TYPE'
    FIELD_LENGTH = 'FIELD_LENGTH'
    FIELD_PRECISION = 'FIELD_PRECISION'
    GLOBAL = 'GLOBAL'
    FORMULA = 'FORMULA'
    OUTPUT = 'OUTPUT'
    RESULT_VAR_NAME = 'value'

    TYPES = [QVariant.LongLong, QVariant.Double, QVariant.String]

    def group(self):
        return self.tr('Vector table')

    def groupId(self):
        return 'vectortable'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.type_names = [self.tr('Integer'),
                           self.tr('Float'),
                           self.tr('String')]

        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT, self.tr('Input layer'),
                                                              types=[QgsProcessing.TypeVector]))
        self.addParameter(QgsProcessingParameterString(self.FIELD_NAME,
                                                       self.tr('Result field name'), defaultValue='NewField'))
        self.addParameter(QgsProcessingParameterEnum(self.FIELD_TYPE,
                                                     self.tr('Field type'), options=self.type_names))
        self.addParameter(QgsProcessingParameterNumber(self.FIELD_LENGTH,
                                                       self.tr('Field length'), minValue=1, maxValue=255,
                                                       defaultValue=10))
        self.addParameter(QgsProcessingParameterNumber(self.FIELD_PRECISION,
                                                       self.tr('Field precision'), minValue=0, maxValue=15,
                                                       defaultValue=3))
        self.addParameter(QgsProcessingParameterString(self.GLOBAL,
                                                       self.tr('Global expression'), multiLine=True, optional=True))
        self.addParameter(QgsProcessingParameterString(self.FORMULA,
                                                       self.tr('Formula'), defaultValue='value = ', multiLine=True))
        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT,
                                                            self.tr('Calculated')))

    def name(self):
        return 'advancedpythonfieldcalculator'

    def displayName(self):
        return self.tr('Advanced Python field calculator')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        if source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.INPUT))

        field_name = self.parameterAsString(parameters, self.FIELD_NAME, context)
        field_type = self.TYPES[self.parameterAsEnum(parameters, self.FIELD_TYPE, context)]
        width = self.parameterAsInt(parameters, self.FIELD_LENGTH, context)
        precision = self.parameterAsInt(parameters, self.FIELD_PRECISION, context)
        code = self.parameterAsString(parameters, self.FORMULA, context)
        globalExpression = self.parameterAsString(parameters, self.GLOBAL, context)

        fields = source.fields()
        field = QgsField(field_name, field_type, '', width, precision)
        fields.append(field)
        new_ns = {}

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               fields, source.wkbType(), source.sourceCrs())
        if sink is None:
            raise QgsProcessingException(self.invalidSinkError(parameters, self.OUTPUT))

        # Run global code
        if globalExpression.strip() != '':
            try:
                bytecode = compile(globalExpression, '<string>', 'exec')
                exec(bytecode, new_ns)
            except:
                raise QgsProcessingException(
                    self.tr("FieldPyculator code execute error.Global code block can't be executed!\n{0}\n{1}").format(
                        str(sys.exc_info()[0].__name__), str(sys.exc_info()[1])))

        # Replace all fields tags
        fields = source.fields()
        num = 0
        for field in fields:
            field_name = str(field.name())
            replval = '__attr[' + str(num) + ']'
            code = code.replace('<' + field_name + '>', replval)
            num += 1

        # Replace all special vars
        code = code.replace('$id', '__id')
        code = code.replace('$geom', '__geom')
        need_id = code.find('__id') != -1
        need_geom = code.find('__geom') != -1
        need_attrs = code.find('__attr') != -1

        # Compile
        try:
            bytecode = compile(code, '<string>', 'exec')
        except:
            raise QgsProcessingException(
                self.tr("FieldPyculator code execute error. Field code block can't be executed!\n{0}\n{1}").format(
                    str(sys.exc_info()[0].__name__), str(sys.exc_info()[1])))

        # Run
        features = source.getFeatures()
        total = 100.0 / source.featureCount() if source.featureCount() else 0

        for current, feat in enumerate(features):
            if feedback.isCanceled():
                break

            feedback.setProgress(int(current * total))
            attrs = feat.attributes()
            feat_id = feat.id()

            # Add needed vars
            if need_id:
                new_ns['__id'] = feat_id

            if need_geom:
                geom = feat.geometry()
                new_ns['__geom'] = geom

            if need_attrs:
                pyattrs = [a for a in attrs]
                new_ns['__attr'] = pyattrs

            # Clear old result
            if self.RESULT_VAR_NAME in new_ns:
                del new_ns[self.RESULT_VAR_NAME]

            # Exec
            exec(bytecode, new_ns)

            # Check result
            if self.RESULT_VAR_NAME not in new_ns:
                raise QgsProcessingException(
                    self.tr("FieldPyculator code execute error\n"
                            "Field code block does not return '{0}' variable! "
                            "Please declare this variable in your code!").format(self.RESULT_VAR_NAME))

            # Write feature
            attrs.append(new_ns[self.RESULT_VAR_NAME])
            feat.setAttributes(attrs)
            sink.addFeature(feat, QgsFeatureSink.FastInsert)

        return {self.OUTPUT: dest_id}

    def checkParameterValues(self, parameters, context):
        # TODO check that formula is correct and fields exist
        return super(FieldsPyculator, self).checkParameterValues(parameters, context)
