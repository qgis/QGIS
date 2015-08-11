# -*- coding: utf-8 -*-

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

# This will get replaced with a git SHA1 when you do a git archive323

__revision__ = '$Format:%H$'

from PyQt4 import QtCore
from qgis.core import QGis, QgsGeometry, QgsFeature, QgsField
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterSelection
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector

settings_method_key = "/qgis/digitizing/validate_geometries"


class CheckValidity(GeoAlgorithm):

    INPUT_LAYER = 'INPUT_LAYER'
    METHOD = 'METHOD'
    VALID_OUTPUT = 'VALID_OUTPUT'
    INVALID_OUTPUT = 'INVALID_OUTPUT'
    ERROR_OUTPUT = 'ERROR_OUTPUT'

    METHODS = ['The one selected in digitizing settings',
               'QGIS',
               'GEOS']

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Check validity')
        self.group, self.i18n_group = self.trAlgorithm('Vector geometry tools')

        self.addParameter(ParameterVector(
            self.INPUT_LAYER,
            self.tr('Input layer'),
            [ParameterVector.VECTOR_TYPE_ANY]))

        self.addParameter(ParameterSelection(
            self.METHOD,
            self.tr('Method'),
            self.METHODS))

        self.addOutput(OutputVector(
            self.VALID_OUTPUT,
            self.tr('Valid output')))

        self.addOutput(OutputVector(
            self.INVALID_OUTPUT,
            self.tr('Invalid output')))

        self.addOutput(OutputVector(
            self.ERROR_OUTPUT,
            self.tr('Error output')))

    def processAlgorithm(self, progress):
        settings = QtCore.QSettings()
        initial_method_setting = settings.value(settings_method_key, 1)

        method = self.getParameterValue(self.METHOD)
        if method != 0:
            settings.setValue(settings_method_key, method)
        try:
            self.doCheck(progress)
        finally:
            settings.setValue(settings_method_key, initial_method_setting)

    def doCheck(self, progress):
        layer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT_LAYER))
        provider = layer.dataProvider()

        settings = QtCore.QSettings()
        method = int(settings.value(settings_method_key, 1))

        valid_ouput = self.getOutputFromName(self.VALID_OUTPUT)
        valid_fields = layer.pendingFields().toList()
        valid_writer = valid_ouput.getVectorWriter(
            valid_fields,
            provider.geometryType(),
            layer.crs())
        valid_count = 0

        invalid_ouput = self.getOutputFromName(self.INVALID_OUTPUT)
        invalid_fields = layer.pendingFields().toList() + [
            QgsField(name='_errors',
                     type=QtCore.QVariant.String,
                     len=255)]
        invalid_writer = invalid_ouput.getVectorWriter(
            invalid_fields,
            provider.geometryType(),
            layer.crs())
        invalid_count = 0

        error_ouput = self.getOutputFromName(self.ERROR_OUTPUT)
        error_fields = [
            QgsField(name='message',
                     type=QtCore.QVariant.String,
                     len=255)]
        error_writer = error_ouput.getVectorWriter(
            error_fields,
            QGis.WKBPoint,
            layer.crs())
        error_count = 0

        features = vector.features(layer)
        count = len(features)
        for current, inFeat in enumerate(features):
            geom = QgsGeometry(inFeat.geometry())
            attrs = inFeat.attributes()

            valid = True
            if not geom.isGeosEmpty():
                errors = list(geom.validateGeometry())
                if errors:
                    # QGIS method return a summary at the end
                    if method == 1:
                        errors.pop()
                    valid = False
                    reasons = []
                    for error in errors:
                        errFeat = QgsFeature()
                        error_geom = QgsGeometry.fromPoint(error.where())
                        errFeat.setGeometry(error_geom)
                        errFeat.setAttributes([error.what()])
                        error_writer.addFeature(errFeat)
                        error_count += 1

                        reasons.append(error.what())

                    reason = "\n".join(reasons)
                    if len(reason) > 255:
                        reason = reason[:252] + '...'
                    attrs.append(reason)

            outFeat = QgsFeature()
            outFeat.setGeometry(geom)
            outFeat.setAttributes(attrs)

            if valid:
                valid_writer.addFeature(outFeat)
                valid_count += 1

            else:
                invalid_writer.addFeature(outFeat)
                invalid_count += 1

            progress.setPercentage(100 * current / float(count))

        del valid_writer
        del invalid_writer
        del error_writer

        if valid_count == 0:
            valid_ouput.open = False
        if invalid_count == 0:
            invalid_ouput.open = False
        if error_count == 0:
            error_ouput.open = False
