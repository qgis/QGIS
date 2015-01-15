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

from PyQt4.QtCore import *
from qgis.core import *
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import \
        GeoAlgorithmExecutionException
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterTableField
from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterString
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector


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
                 'contains'
                ]

    def defineCharacteristics(self):
        self.name = 'Select by attribute'
        self.group = 'Vector selection tools'

        self.addParameter(ParameterVector(self.INPUT,
            self.tr('Input Layer'), [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterTableField(self.FIELD,
            self.tr('Selection attribute'), self.INPUT))
        self.addParameter(ParameterSelection(self.OPERATOR,
            self.tr('Operator'), self.OPERATORS))
        self.addParameter(ParameterString(self.VALUE, self.tr('Value')))

        self.addOutput(OutputVector(self.OUTPUT, self.tr('Output')))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(
                self.getParameterValue(self.INPUT))
        fieldName = self.getParameterValue(self.FIELD)
        operator = self.OPERATORS[self.getParameterValue(self.OPERATOR)]
        value = self.getParameterValue(self.VALUE)

        fields = layer.pendingFields()

        idx = layer.fieldNameIndex(fieldName)
        fieldType = fields[idx].type()

        if fieldType != QVariant.String and operator in self.OPERATORS[-2:]:
            op = ''.join(['"%s", ' % o for o in self.OPERATORS[-2:]])
            raise GeoAlgorithmExecutionException(
                self.tr('Operators %s can be used only with string fields.' % op))

        if fieldType in [QVariant.Int, QVariant.Double]:
            progress.setInfo(self.tr('Numeric field'))
            expr = '"%s" %s %s' % (fieldName, operator, value)
            progress.setInfo(expr)
        elif fieldType == QVariant.String:
            progress.setInfo(self.tr('String field'))
            if operator not in self.OPERATORS[-2:]:
                expr = """"%s" %s '%s'""" % (fieldName, operator, value)
            elif operator == 'begins with':
                expr = """"%s" LIKE '%s%%'""" % (fieldName, value)
            elif operator == 'contains':
                expr = """"%s" LIKE '%%%s%%'""" % (fieldName, value)
            progress.setInfo(expr)
        elif fieldType in [QVariant.Date, QVariant.DateTime]:
            progress.setInfo(self.tr('Date field'))
            expr = """"%s" %s '%s'""" % (fieldX, operator, value)
            progress.setInfo(expr)
        else:
            raise GeoAlgorithmExecutionException(
                self.tr('Unsupported field type "%s"' % fields[idx].typeName()))

        expression = QgsExpression(expr)
        expression.prepare(fields)

        features = vector.features(layer)

        selected = []
        count = len(features)
        total = 100.0 / float(count)
        for count, f in enumerate(features):
            if expression.evaluate(f, fields):
                selected.append(f.id())
            progress.setPercentage(int(count * total))

        layer.setSelectedFeatures(selected)
        self.setOutputValue(self.OUTPUT, self.getParameterValue(self.INPUT))
