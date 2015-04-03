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

from PyQt4.QtCore import Qt, QSettings
from PyQt4.QtGui import QDialog, QFileDialog, QApplication, QCursor, QMessageBox
from qgis.gui import QgsEncodingFileDialog

from processing.core.ProcessingConfig import ProcessingConfig
from processing.core.ProcessingLog import ProcessingLog
from processing.gui.AlgorithmExecutor import runalg
from processing.tools import dataobjects
from processing.gui.Postprocessing import handleAlgorithmResults

from ui_DlgFieldsCalculator import Ui_FieldsCalculator


class FieldsCalculatorDialog(QDialog, Ui_FieldsCalculator):
    def __init__(self, alg):
        QDialog.__init__(self)
        self.setupUi(self)

        self.executed = False
        self.alg = alg
        self.layer = None

        self.cmbInputLayer.currentIndexChanged.connect(self.updateLayer)
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
        for t in self.alg.TYPE_NAMES:
            self.mOutputFieldTypeComboBox.addItem(t)
        self.mOutputFieldTypeComboBox.blockSignals(False)

        self.cmbInputLayer.blockSignals(True)
        layers = dataobjects.getVectorLayers()
        for layer in layers:
            self.cmbInputLayer.addItem(layer.name())
        self.cmbInputLayer.blockSignals(False)

        self.updateLayer()

    def updateLayer(self):
        self.layer = dataobjects.getObject(self.cmbInputLayer.currentText())

        self.builder.setLayer(self.layer)
        self.builder.loadFieldNames()

        self.populateFields()
        #populateOutputFieldTypes()

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
        output = self.alg.getOutputFromName('OUTPUT_LAYER')
        fileFilter = output.getFileFilter(self.alg)

        settings = QSettings()
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
        fileDialog.setConfirmOverwrite(True)
        if fileDialog.exec_() == QDialog.Accepted:
            files = fileDialog.selectedFiles()
            encoding = unicode(fileDialog.encoding())
            output.encoding = encoding
            filename = unicode(files[0])
            selectedFileFilter = unicode(fileDialog.selectedNameFilter())
            if not filename.lower().endswith(
                    tuple(re.findall("\*(\.[a-z]{1,5})", fileFilter))):
                ext = re.search("\*(\.[a-z]{1,5})", selectedFileFilter)
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

        fields = self.layer.pendingFields()
        for f in fields:
            self.mExistingFieldComboBox.addItem(f.name())

    def setParamValues(self):
        if self.mUpdateExistingGroupBox.isChecked():
            fieldName = self.mExistingFieldComboBox.currentText()
        else:
            fieldName = self.mOutputFieldNameLineEdit.text()

        layer = dataobjects.getObjectFromName(self.cmbInputLayer.currentText())

        self.alg.setParameterValue('INPUT_LAYER', layer)
        self.alg.setParameterValue('FIELD_NAME', fieldName)
        self.alg.setParameterValue('FIELD_TYPE',
                self.mOutputFieldTypeComboBox.currentIndex())
        self.alg.setParameterValue('FIELD_LENGTH',
                self.mOutputFieldWidthSpinBox.value())
        self.alg.setParameterValue('FIELD_PRECISION',
                self.mOutputFieldPrecisionSpinBox.value())
        self.alg.setParameterValue('NEW_FIELD',
                self.mNewFieldGroupBox.isChecked())
        self.alg.setParameterValue('FORMULA', self.builder.expressionText())
        self.alg.setOutputValue('OUTPUT_LAYER',
                self.leOutputFile.text())
        return True

    def accept(self):
        keepOpen = ProcessingConfig.getSetting(ProcessingConfig.KEEP_DIALOG_OPEN)
        try:
            if self.setParamValues():
                QApplication.setOverrideCursor(QCursor(Qt.WaitCursor))
                ProcessingLog.addToLog(ProcessingLog.LOG_ALGORITHM,
                                     self.alg.getAsCommand())

                self.executed =  runalg(self.alg, self)
                if self.executed:
                    handleAlgorithmResults(self.alg,
                                           self,
                                           not keepOpen)
                if not keepOpen:
                    QDialog.reject(self)
            else:
                QMessageBox.critical(self,
                                     self.tr('Unable to execute algorithm'),
                                     self.tr('Wrong or missing parameter '
                                             'values'))
        finally:
            QApplication.restoreOverrideCursor()

    def reject(self):
        self.executed = False
        QDialog.reject(self)

    def setPercentage(self, i):
        self.progressBar.setValue(i)

    def setText(self, text):
        pass

    def error(self, text):
        QMessageBox.critical(self, "Error", text)
        ProcessingLog.addToLog(ProcessingLog.LOG_ERROR, text)
