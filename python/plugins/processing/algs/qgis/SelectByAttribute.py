# -*- coding: utf-8 -*-

"""
***************************************************************************
    SelectByAttribute.py
    ---------------------
    Date                 : May 2010
    Copyright            : (C) 2010 by Michael Minn
    Email                : pyqgis at michaelminn dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Michael Minn'
__date__ = 'May 2010'
__copyright__ = '(C) 2010, Michael Minn'

from qgis.PyQt.QtCore import QVariant
from qgis.core import (QgsExpression,
                       QgsVectorLayer,
                       QgsProcessing,
                       QgsProcessingException,
                       QgsProcessingAlgorithm,
                       QgsProcessingParameterVectorLayer,
                       QgsProcessingParameterField,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterString,
                       QgsProcessingOutputVectorLayer)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class SelectByAttribute(QgisAlgorithm):
    INPUT = 'INPUT'
    FIELD = 'FIELD'
    OPERATOR = 'OPERATOR'
    VALUE = 'VALUE'
    METHOD = 'METHOD'
    OUTPUT = 'OUTPUT'

    OPERATORS = ['=',
                 '<>',
                 '>',
                 '>=',
                 '<',
                 '<=',
                 'begins with',
                 'contains',
                 'is null',
                 'is not null',
                 'does not contain'
                 ]
    STRING_OPERATORS = ['begins with',
                        'contains',
                        'does not contain']

    def tags(self):
        return self.tr('select,attribute,value,contains,null,field').split(',')

    def group(self):
        return self.tr('Vector selection')

    def groupId(self):
        return 'vectorselection'

    def __init__(self):
        super().__init__()

    def flags(self):
        return super().flags() | QgsProcessingAlgorithm.FlagNoThreading

    def initAlgorithm(self, config=None):
        self.operators = ['=',
                          '≠',
                          '>',
                          '≥',
                          '<',
                          '≤',
                          self.tr('begins with'),
                          self.tr('contains'),
                          self.tr('is null'),
                          self.tr('is not null'),
                          self.tr('does not contain')
                          ]

        self.methods = [self.tr('creating new selection'),
                        self.tr('adding to current selection'),
                        self.tr('removing from current selection'),
                        self.tr('selecting within current selection')]

        self.addParameter(QgsProcessingParameterVectorLayer(self.INPUT,
                                                            self.tr('Input layer'),
                                                            types=[QgsProcessing.TypeVector]))
        self.addParameter(QgsProcessingParameterField(self.FIELD,
                                                      self.tr('Selection attribute'),
                                                      parentLayerParameterName=self.INPUT))
        self.addParameter(QgsProcessingParameterEnum(self.OPERATOR,
                                                     self.tr('Operator'), self.operators, defaultValue=0))
        self.addParameter(QgsProcessingParameterString(self.VALUE,
                                                       self.tr('Value'),
                                                       optional=True))
        self.addParameter(QgsProcessingParameterEnum(self.METHOD,
                                                     self.tr('Modify current selection by'),
                                                     self.methods,
                                                     defaultValue=0))

        self.addOutput(QgsProcessingOutputVectorLayer(self.OUTPUT, self.tr('Selected (attribute)')))

    def name(self):
        return 'selectbyattribute'

    def displayName(self):
        return self.tr('Select by attribute')

    def processAlgorithm(self, parameters, context, feedback):
        layer = self.parameterAsVectorLayer(parameters, self.INPUT, context)

        fieldName = self.parameterAsString(parameters, self.FIELD, context)
        operator = self.OPERATORS[self.parameterAsEnum(parameters, self.OPERATOR, context)]
        value = self.parameterAsString(parameters, self.VALUE, context)

        fields = layer.fields()

        idx = layer.fields().lookupField(fieldName)
        if idx < 0:
            raise QgsProcessingException(self.tr("Field '{}' was not found in layer").format(fieldName))

        fieldType = fields[idx].type()

        if fieldType != QVariant.String and operator in self.STRING_OPERATORS:
            op = ''.join(['"%s", ' % o for o in self.STRING_OPERATORS])
            raise QgsProcessingException(
                self.tr('Operators {0} can be used only with string fields.').format(op))

        field_ref = QgsExpression.quotedColumnRef(fieldName)
        quoted_val = QgsExpression.quotedValue(value)
        if operator == 'is null':
            expression_string = '{} IS NULL'.format(field_ref)
        elif operator == 'is not null':
            expression_string = '{} IS NOT NULL'.format(field_ref)
        elif operator == 'begins with':
            expression_string = "{} LIKE '{}%'".format(field_ref, value)
        elif operator == 'contains':
            expression_string = "{} LIKE '%{}%'".format(field_ref, value)
        elif operator == 'does not contain':
            expression_string = "{} NOT LIKE '%{}%'".format(field_ref, value)
        else:
            expression_string = '{} {} {}'.format(field_ref, operator, quoted_val)

        method = self.parameterAsEnum(parameters, self.METHOD, context)
        if method == 0:
            behavior = QgsVectorLayer.SetSelection
        elif method == 1:
            behavior = QgsVectorLayer.AddToSelection
        elif method == 2:
            behavior = QgsVectorLayer.RemoveFromSelection
        elif method == 3:
            behavior = QgsVectorLayer.IntersectSelection

        expression = QgsExpression(expression_string)
        if expression.hasParserError():
            raise QgsProcessingException(expression.parserErrorString())

        layer.selectByExpression(expression_string, behavior)

        return {self.OUTPUT: parameters[self.INPUT]}
