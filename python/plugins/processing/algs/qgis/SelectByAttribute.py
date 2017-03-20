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

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.PyQt.QtCore import QVariant
from qgis.core import QgsExpression, QgsFeatureRequest
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterTableField
from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterString
from processing.core.outputs import OutputVector
from processing.tools import dataobjects


class SelectByAttribute(GeoAlgorithm):
    INPUT = 'INPUT'
    FIELD = 'FIELD'
    OPERATOR = 'OPERATOR'
    VALUE = 'VALUE'
    OUTPUT = 'OUTPUT'

    OPERATORS = ['=',
                 '!=',
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

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Select by attribute')
        self.group, self.i18n_group = self.trAlgorithm('Vector selection tools')
        self.tags = self.tr('select,attribute,value,contains,null,field')

        self.i18n_operators = ['=',
                               '!=',
                               '>',
                               '>=',
                               '<',
                               '<=',
                               self.tr('begins with'),
                               self.tr('contains'),
                               self.tr('is null'),
                               self.tr('is not null'),
                               self.tr('does not contain')
                               ]

        self.addParameter(ParameterVector(self.INPUT,
                                          self.tr('Input Layer')))
        self.addParameter(ParameterTableField(self.FIELD,
                                              self.tr('Selection attribute'), self.INPUT))
        self.addParameter(ParameterSelection(self.OPERATOR,
                                             self.tr('Operator'), self.i18n_operators))
        self.addParameter(ParameterString(self.VALUE, self.tr('Value')))

        self.addOutput(OutputVector(self.OUTPUT, self.tr('Selected (attribute)'), True))

    def processAlgorithm(self, feedback):
        fileName = self.getParameterValue(self.INPUT)
        layer = dataobjects.getObjectFromUri(fileName)
        fieldName = self.getParameterValue(self.FIELD)
        operator = self.OPERATORS[self.getParameterValue(self.OPERATOR)]
        value = self.getParameterValue(self.VALUE)

        fields = layer.fields()

        idx = layer.fields().lookupField(fieldName)
        fieldType = fields[idx].type()

        if fieldType != QVariant.String and operator in self.STRING_OPERATORS:
            op = ''.join(['"%s", ' % o for o in self.STRING_OPERATORS])
            raise GeoAlgorithmExecutionException(
                self.tr('Operators {0} can be used only with string fields.').format(op))

        field_ref = QgsExpression.quotedColumnRef(fieldName)
        quoted_val = QgsExpression.quotedValue(value)
        if operator == 'is null':
            expression_string = '{} IS NULL'.format(field_ref)
        elif operator == 'is not null':
            expression_string = '{} IS NOT NULL'.format(field_ref)
        elif operator == 'begins with':
            expression_string = """%s LIKE '%s%%'""" % (field_ref, value)
        elif operator == 'contains':
            expression_string = """%s LIKE '%%%s%%'""" % (field_ref, value)
        elif operator == 'does not contain':
            expression_string = """%s NOT LIKE '%%%s%%'""" % (field_ref, value)
        else:
            expression_string = '{} {} {}'.format(field_ref, operator, quoted_val)

        expression = QgsExpression(expression_string)
        if expression.hasParserError():
            raise GeoAlgorithmExecutionException(expression.parserErrorString())

        layer.selectByExpression(expression_string)
        self.setOutputValue(self.OUTPUT, fileName)
