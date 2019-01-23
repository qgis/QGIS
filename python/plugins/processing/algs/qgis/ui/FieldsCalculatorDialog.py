# -*- coding: utf-8 -*-

"""
***************************************************************************
    FieldsCalculatorDialog.py
    ---------------------
    Date                 : October 2013
    Copyright            : (C) 2013 by Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Alexander Bruy'
__date__ = 'October 2013'
__copyright__ = '(C) 2013, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import re
import warnings

from qgis.PyQt import uic
from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtWidgets import QDialog, QFileDialog, QApplication, QMessageBox
from qgis.PyQt.QtGui import QCursor
from qgis.core import (Qgis,
                       QgsExpressionContextUtils,
                       QgsProcessingFeedback,
                       QgsSettings,
                       QgsMapLayerProxyModel,
                       QgsProperty,
                       QgsProject,
                       QgsMessageLog,
                       QgsMapLayer,
                       QgsProcessingOutputLayerDefinition)
from qgis.gui import QgsEncodingFileDialog, QgsGui
from qgis.utils import OverrideCursor, iface

from processing.core.ProcessingConfig import ProcessingConfig
from processing.core.ProcessingLog import ProcessingLog
from processing.gui.AlgorithmExecutor import execute
from processing.tools import dataobjects
from processing.gui.Postprocessing import handleAlgorithmResults
from processing.gui.PostgisTableSelector import PostgisTableSelector
from processing.gui.ParameterGuiUtils import getFileFilter

pluginPath = os.path.dirname(__file__)
with warnings.catch_warnings():
    warnings.filterwarnings("ignore", category=DeprecationWarning)
    WIDGET, BASE = uic.loadUiType(
        os.path.join(pluginPath, 'DlgFieldsCalculator.ui'))


class FieldCalculatorFeedback(QgsProcessingFeedback):

    """
    Directs algorithm feedback to an algorithm dialog
    """

    def __init__(self, dialog):
        QgsProcessingFeedback.__init__(self)
        self.dialog = dialog

    def reportError(self, msg, fatalError=False):
        self.dialog.error(msg)


class FieldsCalculatorDialog(BASE, WIDGET):

    def __init__(self, alg):
        super(FieldsCalculatorDialog, self).__init__(None)
        self.setupUi(self)

        self.executed = False
        self._wasExecuted = False
        self.alg = alg
        self.layer = None

        self.cmbInputLayer.setFilters(QgsMapLayerProxyModel.VectorLayer)
        try:
            if iface.activeLayer().type() == QgsMapLayer.VectorLayer:
                self.cmbInputLayer.setLayer(iface.activeLayer())
        except:
            pass

        self.cmbInputLayer.layerChanged.connect(self.updateLayer)
        self.btnBrowse.clicked.connect(self.selectFile)
        self.mNewFieldGroupBox.toggled.connect(self.toggleExistingGroup)
        self.mUpdateExistingGroupBox.toggled.connect(self.toggleNewGroup)
        self.mOutputFieldTypeComboBox.currentIndexChanged.connect(self.setupSpinboxes)

        # Default values for field width and precision
        self.mOutputFieldWidthSpinBox.setValue(10)
        self.mOutputFieldPrecisionSpinBox.setValue(3)

        # Output is a shapefile, so limit maximum field name length
        self.mOutputFieldNameLineEdit.setMaxLength(10)

        self.manageGui()

    def manageGui(self):
        if hasattr(self.leOutputFile, 'setPlaceholderText'):
            self.leOutputFile.setPlaceholderText(
                self.tr('[Save to temporary file]'))

        self.mOutputFieldTypeComboBox.blockSignals(True)
        for t in self.alg.type_names:
            self.mOutputFieldTypeComboBox.addItem(t)
        self.mOutputFieldTypeComboBox.blockSignals(False)
        self.builder.loadRecent('fieldcalc')

        self.updateLayer(self.cmbInputLayer.currentLayer())

    def initContext(self):
        exp_context = self.builder.expressionContext()
        exp_context.appendScopes(QgsExpressionContextUtils.globalProjectLayerScopes(self.layer))
        exp_context.lastScope().setVariable("row_number", 1)
        exp_context.setHighlightedVariables(["row_number"])
        self.builder.setExpressionContext(exp_context)

    def updateLayer(self, layer):
        self.layer = layer
        self.builder.setLayer(self.layer)
        self.initContext()
        self.populateFields()

    def setupSpinboxes(self, index):
        if index != 0:
            self.mOutputFieldPrecisionSpinBox.setEnabled(False)
        else:
            self.mOutputFieldPrecisionSpinBox.setEnabled(True)

        if index == 0:
            self.mOutputFieldWidthSpinBox.setRange(1, 20)
            self.mOutputFieldWidthSpinBox.setValue(10)
            self.mOutputFieldPrecisionSpinBox.setRange(0, 15)
            self.mOutputFieldPrecisionSpinBox.setValue(3)
        elif index == 1:
            self.mOutputFieldWidthSpinBox.setRange(1, 10)
            self.mOutputFieldWidthSpinBox.setValue(10)
        elif index == 2:
            self.mOutputFieldWidthSpinBox.setRange(1, 255)
            self.mOutputFieldWidthSpinBox.setValue(80)
        else:
            self.mOutputFieldWidthSpinBox.setEnabled(False)
            self.mOutputFieldPrecisionSpinBox.setEnabled(False)

    def selectFile(self):
        output = self.alg.parameterDefinition('OUTPUT')
        fileFilter = getFileFilter(output)

        settings = QgsSettings()
        if settings.contains('/Processing/LastOutputPath'):
            path = settings.value('/Processing/LastOutputPath')
        else:
            path = ProcessingConfig.getSetting(ProcessingConfig.OUTPUT_FOLDER)
        lastEncoding = settings.value('/Processing/encoding', 'System')
        fileDialog = QgsEncodingFileDialog(self,
                                           self.tr('Save file'),
                                           path,
                                           fileFilter,
                                           lastEncoding)
        fileDialog.setFileMode(QFileDialog.AnyFile)
        fileDialog.setAcceptMode(QFileDialog.AcceptSave)
        fileDialog.setOption(QFileDialog.DontConfirmOverwrite, False)
        if fileDialog.exec_() == QDialog.Accepted:
            files = fileDialog.selectedFiles()
            encoding = str(fileDialog.encoding())
            output.encoding = encoding
            filename = str(files[0])
            selectedFileFilter = str(fileDialog.selectedNameFilter())
            if not filename.lower().endswith(
                    tuple(re.findall("\\*(\\.[a-z]{1,10})", fileFilter))):
                ext = re.search("\\*(\\.[a-z]{1,10})", selectedFileFilter)
                if ext:
                    filename = filename + ext.group(1)
            self.leOutputFile.setText(filename)
            settings.setValue('/Processing/LastOutputPath',
                              os.path.dirname(filename))
            settings.setValue('/Processing/encoding', encoding)

    def toggleExistingGroup(self, toggled):
        self.mUpdateExistingGroupBox.setChecked(not toggled)

    def toggleNewGroup(self, toggled):
        self.mNewFieldGroupBox.setChecked(not toggled)

    def populateFields(self):
        if self.layer is None:
            return

        self.mExistingFieldComboBox.clear()
        fields = self.layer.fields()
        for f in fields:
            self.mExistingFieldComboBox.addItem(f.name())

    def getParamValues(self):
        if self.mUpdateExistingGroupBox.isChecked():
            fieldName = self.mExistingFieldComboBox.currentText()
        else:
            fieldName = self.mOutputFieldNameLineEdit.text()

        layer = self.cmbInputLayer.currentLayer()

        context = dataobjects.createContext()

        parameters = {}
        parameters['INPUT'] = layer
        parameters['FIELD_NAME'] = fieldName
        parameters['FIELD_TYPE'] = self.mOutputFieldTypeComboBox.currentIndex()
        parameters['FIELD_LENGTH'] = self.mOutputFieldWidthSpinBox.value()
        parameters['FIELD_PRECISION'] = self.mOutputFieldPrecisionSpinBox.value()
        parameters['NEW_FIELD'] = self.mNewFieldGroupBox.isChecked()
        parameters['FORMULA'] = self.builder.expressionText()
        output = QgsProcessingOutputLayerDefinition()
        if self.leOutputFile.text().strip():
            output.sink = QgsProperty.fromValue(self.leOutputFile.text().strip())
        else:
            output.sink = QgsProperty.fromValue('memory:')
        output.destinationProject = context.project()
        parameters['OUTPUT'] = output

        ok, msg = self.alg.checkParameterValues(parameters, context)
        if not ok:
            QMessageBox.warning(
                self, self.tr('Unable to execute algorithm'), msg)
            return {}
        return parameters

    def accept(self):
        keepOpen = ProcessingConfig.getSetting(ProcessingConfig.KEEP_DIALOG_OPEN)
        parameters = self.getParamValues()
        if parameters:
            with OverrideCursor(Qt.WaitCursor):
                self.feedback = FieldCalculatorFeedback(self)
                self.feedback.progressChanged.connect(self.setPercentage)

                context = dataobjects.createContext()
                ProcessingLog.addToLog(self.alg.asPythonCommand(parameters, context))
                QgsGui.instance().processingRecentAlgorithmLog().push(self.alg.id())

                self.executed, results = execute(self.alg, parameters, context, self.feedback)
                self.setPercentage(0)

                if self.executed:
                    handleAlgorithmResults(self.alg,
                                           context,
                                           self.feedback,
                                           not keepOpen,
                                           parameters)
                self._wasExecuted = self.executed or self._wasExecuted
                if not keepOpen:
                    QDialog.reject(self)

    def reject(self):
        self.executed = False
        QDialog.reject(self)

    def setPercentage(self, i):
        self.progressBar.setValue(i)

    def error(self, text):
        QMessageBox.critical(self, "Error", text)
        QgsMessageLog.logMessage(text, self.tr('Processing'), Qgis.Critical)

    def wasExecuted(self):
        return self._wasExecuted
