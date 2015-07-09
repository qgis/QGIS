# -*- coding: utf-8 -*-

"""
***************************************************************************
    FieldsCalculator.py
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

from PyQt4.QtCore import QVariant
from qgis.core import QgsExpression, QgsFeature, QgsField, QgsDistanceArea, QgsProject, GEO_NONE
from qgis.utils import iface
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterString
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterSelection
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector, system

from ui.FieldsCalculatorDialog import FieldsCalculatorDialog

class FieldsCalculator(GeoAlgorithm):

    INPUT_LAYER = 'INPUT_LAYER'
    NEW_FIELD = 'NEW_FIELD'
    FIELD_NAME = 'FIELD_NAME'
    FIELD_TYPE = 'FIELD_TYPE'
    FIELD_LENGTH = 'FIELD_LENGTH'
    FIELD_PRECISION = 'FIELD_PRECISION'
    FORMULA = 'FORMULA'
    OUTPUT_LAYER = 'OUTPUT_LAYER'

    TYPE_NAMES = ['Float', 'Integer', 'String', 'Date']
    TYPES = [QVariant.Double, QVariant.Int, QVariant.String, QVariant.Date]

    def defineCharacteristics(self):
        self.name = 'Field calculator'
        self.group = 'Vector table tools'
        self.addParameter(ParameterVector(self.INPUT_LAYER,
            self.tr('Input layer'), [ParameterVector.VECTOR_TYPE_ANY], False))
        self.addParameter(ParameterString(self.FIELD_NAME,
            self.tr('Result field name')))
        self.addParameter(ParameterSelection(self.FIELD_TYPE,
            self.tr('Field type'), self.TYPE_NAMES))
        self.addParameter(ParameterNumber(self.FIELD_LENGTH,
            self.tr('Field length'), 1, 255, 10))
        self.addParameter(ParameterNumber(self.FIELD_PRECISION,
            self.tr('Field precision'), 0, 15, 3))
        self.addParameter(ParameterBoolean(self.NEW_FIELD,
            self.tr('Create new field'), True))
        self.addParameter(ParameterString(self.FORMULA, self.tr('Formula')))
        self.addOutput(OutputVector(self.OUTPUT_LAYER, self.tr('Calculated')))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(self.getParameterValue(self.INPUT_LAYER))
        fieldName = self.getParameterValue(self.FIELD_NAME)
        fieldType = self.TYPES[self.getParameterValue(self.FIELD_TYPE)]
        width = self.getParameterValue(self.FIELD_LENGTH)
        precision = self.getParameterValue(self.FIELD_PRECISION)
        newField = self.getParameterValue(self.NEW_FIELD)
        formula = self.getParameterValue(self.FORMULA)

        output = self.getOutputFromName(self.OUTPUT_LAYER)

        if output.value == '':
            ext = output.getDefaultFileExtension(self)
            output.value = system.getTempFilenameInTempFolder(
                output.name + '.' + ext)

        provider = layer.dataProvider()
        fields = layer.pendingFields()
        if newField:
            fields.append(QgsField(fieldName, fieldType, '', width, precision))

        writer = output.getVectorWriter(fields, provider.geometryType(),
                                        layer.crs())

        exp = QgsExpression(formula)

        da = QgsDistanceArea()
        da.setSourceCrs(layer.crs().srsid())
        da.setEllipsoidalMode(
            iface.mapCanvas().mapSettings().hasCrsTransformEnabled())
        da.setEllipsoid(QgsProject.instance().readEntry(
            'Measure', '/Ellipsoid', GEO_NONE)[0])
        exp.setGeomCalculator(da)

        if not exp.prepare(layer.pendingFields()):
            raise GeoAlgorithmExecutionException(
                self.tr('Evaluation error: %s' % exp.evalErrorString()))

        outFeature = QgsFeature()
        outFeature.initAttributes(len(fields))
        outFeature.setFields(fields)

        error = ''
        calculationSuccess = True

        current = 0
        features = vector.features(layer)
        total = 100.0 / len(features)

        rownum = 1
        for current, f in enumerate(features):
            rownum = current + 1
            exp.setCurrentRowNumber(rownum)
            value = exp.evaluate(f)
            if exp.hasEvalError():
                calculationSuccess = False
                error = exp.evalErrorString()
                break
            else:
                outFeature.setGeometry(f.geometry())
                for fld in f.fields():
                    outFeature[fld.name()] = f[fld.name()]
                outFeature[fieldName] = value
                writer.addFeature(outFeature)

            progress.setPercentage(int(current * total))
        del writer

        if not calculationSuccess:
            raise GeoAlgorithmExecutionException(
                self.tr('An error occured while evaluating the calculation '
                        'string:\n%s' % error))

    def checkParameterValuesBeforeExecuting(self):
        newField = self.getParameterValue(self.NEW_FIELD)
        fieldName = self.getParameterValue(self.FIELD_NAME).strip()
        if newField and len(fieldName) == 0:
            return self.tr('Field name is not set. Please enter a field name')


    def getCustomParametersDialog(self):
        return FieldsCalculatorDialog(self)
