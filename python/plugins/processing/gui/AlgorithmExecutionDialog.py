#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
***************************************************************************
    AlgorithmExecutionDialog.py
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

# -*- coding: latin-1 -*-
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4 import QtCore, QtGui, QtWebKit
from processing.tools import dataobjects
from processing.parameters.ParameterRaster import ParameterRaster
from processing.parameters.ParameterVector import ParameterVector
from processing.parameters.ParameterBoolean import ParameterBoolean
from processing.parameters.ParameterSelection import ParameterSelection
from processing.parameters.ParameterMultipleInput import ParameterMultipleInput
from processing.parameters.ParameterFixedTable import ParameterFixedTable
from processing.parameters.ParameterTableField import ParameterTableField
from processing.parameters.ParameterTable import ParameterTable
from processing.core.ProcessingLog import ProcessingLog
from processing.gui.Postprocessing import Postprocessing
from processing.parameters.ParameterRange import ParameterRange
from processing.parameters.ParameterNumber import ParameterNumber
from processing.parameters.ParameterFile import ParameterFile
from processing.parameters.ParameterCrs import ParameterCrs
from processing.core.ProcessingConfig import ProcessingConfig
from processing.parameters.ParameterExtent import ParameterExtent
from processing.outputs.OutputRaster import OutputRaster
from processing.outputs.OutputVector import OutputVector
from processing.outputs.OutputTable import OutputTable
from processing.core.WrongHelpFileException import WrongHelpFileException
import os
from processing.gui.UnthreadedAlgorithmExecutor import UnthreadedAlgorithmExecutor
from processing.parameters.ParameterString import ParameterString

class AlgorithmExecutionDialog(QtGui.QDialog):
    class InvalidParameterValue(Exception):
        def __init__(self, param, widget):
            self.parameter, self.widget = param, widget

    '''Base class for dialogs that execute algorithms'''
    def __init__(self, alg, mainWidget):
        QtGui.QDialog.__init__(self, None, QtCore.Qt.WindowSystemMenuHint | QtCore.Qt.WindowTitleHint)
        self.executed = False
        self.mainWidget = mainWidget
        self.alg = alg
        self.resize(650, 450)
        self.buttonBox = QtGui.QDialogButtonBox()
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(
            QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Close)
        self.buttonBox.button(QtGui.QDialogButtonBox.Cancel).setEnabled(False)
        self.runButton = QtGui.QPushButton()
        self.runButton.setText("Run")
        self.buttonBox.addButton(self.runButton, QtGui.QDialogButtonBox.ActionRole)
        self.runButton.clicked.connect(self.accept)        
        self.setWindowTitle(self.alg.name)
        self.progressLabel = QtGui.QLabel()
        self.progress = QtGui.QProgressBar()
        self.progress.setMinimum(0)
        self.progress.setMaximum(100)
        self.progress.setValue(0)
        self.verticalLayout = QtGui.QVBoxLayout(self)
        self.verticalLayout.setSpacing(2)
        self.verticalLayout.setMargin(0)
        self.tabWidget = QtGui.QTabWidget()
        self.tabWidget.setMinimumWidth(300)
        self.tabWidget.addTab(self.mainWidget, "Parameters")
        self.verticalLayout.addWidget(self.tabWidget)
        self.logText = QTextEdit()
        self.logText.readOnly = True
        self.tabWidget.addTab(self.logText, "Log")
        self.webView = QtWebKit.QWebView()
        cssUrl = QtCore.QUrl(os.path.join(os.path.dirname(__file__), "help", "help.css"))
        self.webView.settings().setUserStyleSheetUrl(cssUrl)
        html = None
        try:
            if self.alg.helpFile():
                helpFile = self.alg.helpFile()
            else:
                html = "<h2>Sorry, no help is available for this algorithm.</h2>"
        except WrongHelpFileException, e:
            html = e.msg
            self.webView.setHtml("<h2>Could not open help file :-( </h2>")
        try:
            if html:
                self.webView.setHtml(html)
            else:
                url = QtCore.QUrl(helpFile)
                self.webView.load(url)
        except:
            self.webView.setHtml("<h2>Could not open help file :-( </h2>")
        self.tabWidget.addTab(self.webView, "Help")
        self.verticalLayout.addWidget(self.progressLabel)
        self.verticalLayout.addWidget(self.progress)
        self.verticalLayout.addWidget(self.buttonBox)
        self.setLayout(self.verticalLayout)
        self.buttonBox.rejected.connect(self.close)
        self.buttonBox.button(QtGui.QDialogButtonBox.Cancel).clicked.connect(self.cancel)


    def setParamValues(self):
        params = self.alg.parameters
        outputs = self.alg.outputs

        for param in params:
            if param.hidden:
                continue
            if isinstance(param, ParameterExtent):
                continue
            if not self.setParamValue(param, self.paramTable.valueItems[param.name]):
                raise AlgorithmExecutionDialog.InvalidParameterValue(param, self.paramTable.valueItems[param.name])

        for param in params:
            if isinstance(param, ParameterExtent):
                if not self.setParamValue(param, self.paramTable.valueItems[param.name]):
                    raise AlgorithmExecutionDialog.InvalidParameterValue(param, self.paramTable.valueItems[param.name])

        for output in outputs:
            if output.hidden:
                continue
            output.value = self.paramTable.valueItems[output.name].getValue()
            if isinstance(output, (OutputRaster, OutputVector, OutputTable)):
                output.open = self.paramTable.checkBoxes[output.name].isChecked()

        return True

    def setParamValue(self, param, widget):
        if isinstance(param, ParameterRaster):
            return param.setValue(widget.getValue())
        elif isinstance(param, (ParameterVector, ParameterTable)):
            try:
                return param.setValue(widget.itemData(widget.currentIndex()))
            except:
                return param.setValue(widget.getValue())
        elif isinstance(param, ParameterBoolean):
            return param.setValue(widget.currentIndex() == 0)
        elif isinstance(param, ParameterSelection):
            return param.setValue(widget.currentIndex())
        elif isinstance(param, ParameterFixedTable):
            return param.setValue(widget.table)
        elif isinstance(param, ParameterRange):
            return param.setValue(widget.getValue())
        if isinstance(param, ParameterTableField):
            return param.setValue(widget.currentText())
        elif isinstance(param, ParameterMultipleInput):
            if param.datatype == ParameterMultipleInput.TYPE_VECTOR_ANY:
                options = dataobjects.getVectorLayers()
            else:
                options = dataobjects.getRasterLayers()
            value = []
            for index in widget.selectedoptions:
                value.append(options[index])
            return param.setValue(value)
        elif isinstance(param, (ParameterNumber, ParameterFile, ParameterCrs, ParameterExtent)):
            return param.setValue(widget.getValue())
        elif isinstance(param, ParameterString):
            if param.multiline:
                return param.setValue(unicode(widget.toPlainText()))
            else:
                return param.setValue(unicode(widget.text()))
        else:
            return param.setValue(unicode(widget.text()))

    @pyqtSlot()
    def accept(self):
        checkCRS = ProcessingConfig.getSetting(ProcessingConfig.WARN_UNMATCHING_CRS)
        keepOpen = ProcessingConfig.getSetting(ProcessingConfig.KEEP_DIALOG_OPEN)        
        self.showDebug = ProcessingConfig.getSetting(ProcessingConfig.SHOW_DEBUG_IN_DIALOG)
        try:
            self.setParamValues()
            if checkCRS and not self.alg.checkInputCRS():
                reply = QMessageBox.question(self, "Unmatching CRS's",
                        "Layers do not all use the same CRS.\n" +
                        "This can cause unexpected results.\n" +
                        "Do you want to continue?",
                        QtGui.QMessageBox.Yes | QtGui.QMessageBox.No, QtGui.QMessageBox.No)
                if reply == QtGui.QMessageBox.No:
                    return
            msg = self.alg.checkParameterValuesBeforeExecuting()
            if msg:
                QMessageBox.critical(self, "Unable to execute algorithm", msg)
                return
            self.runButton.setEnabled(False)
            self.buttonBox.button(QtGui.QDialogButtonBox.Close).setEnabled(False)
            buttons = self.paramTable.iterateButtons
            self.iterateParam = None

            for i in range(len(buttons.values())):
                button = buttons.values()[i]
                if button.isChecked():
                    self.iterateParam = buttons.keys()[i]
                    break

            self.tabWidget.setCurrentIndex(1) # log tab
            self.progress.setMaximum(0)
            self.progressLabel.setText("Processing algorithm...")
            QApplication.setOverrideCursor(QCursor(Qt.WaitCursor))
              
            self.setInfo("<b>Algorithm %s starting...</b>" % self.alg.name)
            if self.iterateParam:
                if UnthreadedAlgorithmExecutor.runalgIterating(self.alg, self.iterateParam, self):
                    self.finish()
                else:
                    QApplication.restoreOverrideCursor()
                    if not keepOpen:
                        self.close()
                    else:
                        self.resetGUI()
            else:
                command = self.alg.getAsCommand()
                if command:
                    ProcessingLog.addToLog(ProcessingLog.LOG_ALGORITHM, command)
                if UnthreadedAlgorithmExecutor.runalg(self.alg, self):
                    self.finish()
                else:
                    QApplication.restoreOverrideCursor()
                    if not keepOpen:
                        self.close()
                    else:
                        self.resetGUI()            
        except AlgorithmExecutionDialog.InvalidParameterValue as ex:
            try:
                self.buttonBox.accepted.connect(lambda: ex.widget.setPalette(QPalette()))
                palette = ex.widget.palette()
                palette.setColor(QPalette.Base, QColor(255, 255, 0))
                ex.widget.setPalette(palette)
                self.progressLabel.setText("<b>Missing parameter value: " + ex.parameter.description + "</b>")
                return
            except:
                QMessageBox.critical(self, "Unable to execute algorithm", "Wrong or missing parameter values")

    @pyqtSlot()
    def finish(self):
        keepOpen = ProcessingConfig.getSetting(ProcessingConfig.KEEP_DIALOG_OPEN)
        if self.iterateParam is None:
            Postprocessing.handleAlgorithmResults(self.alg, self, not keepOpen)
        self.executed = True
        self.setInfo("Algorithm %s finished" % self.alg.name)
        QApplication.restoreOverrideCursor()
        if not keepOpen:
            self.close()
        else:
            self.resetGUI()
            if self.alg.getHTMLOutputsCount() > 0:
                self.setInfo("HTML output has been generated by this algorithm.\nOpen the results dialog to check it.")

    @pyqtSlot(str)
    def error(self, msg):
        QApplication.restoreOverrideCursor()
        keepOpen = ProcessingConfig.getSetting(ProcessingConfig.KEEP_DIALOG_OPEN)
        self.setInfo(msg, True)
        if not keepOpen:
            QMessageBox.critical(self, "Error", msg)
            self.close()
        else:
            self.resetGUI()
            #self.setInfo(msg, True)
            self.tabWidget.setCurrentIndex(1) # log tab

    @pyqtSlot(int)
    def iterate(self, i):
        self.setInfo("<b>Algorithm %s iteration #%i completed</b>" % (self.alg.name, i))

    @pyqtSlot()
    def cancel(self):
        self.setInfo("<b>Algorithm %s canceled</b>" % self.alg.name)
        try:
            self.algEx.algExecuted.disconnect()
            self.algEx.terminate()
        except:
            pass
        self.resetGUI()

    def resetGUI(self):
        QApplication.restoreOverrideCursor()
        self.progressLabel.setText("")
        self.progress.setValue(0)        
        self.runButton.setEnabled(True)
        self.buttonBox.button(QtGui.QDialogButtonBox.Close).setEnabled(True)
        self.buttonBox.button(QtGui.QDialogButtonBox.Cancel).setEnabled(False)


    @pyqtSlot(str)
    @pyqtSlot(str, bool)
    def setInfo(self, msg, error = False):
        if error:
            self.logText.append('<span style="color:red">' + msg + '</span>')
        else:
            self.logText.append(msg)

    @pyqtSlot(str)
    def setCommand(self, cmd):
        if self.showDebug:
            self.setInfo('<tt>' + cmd + '<tt>')

    @pyqtSlot(str)
    def setDebugInfo(self, msg):
        if self.showDebug:
            self.setInfo('<span style="color:blue">' + msg + '</span>')

    @pyqtSlot(str)
    def setConsoleInfo(self, msg):
        if self.showDebug:
            self.setCommand('<span style="color:darkgray">' + msg + '</span>')

    def setPercentage(self, i):
        if self.progress.maximum() == 0:
            self.progress.setMaximum(100)
        self.progress.setValue(i)

    def setText(self, text):
        self.progressLabel.setText(text)
        self.setInfo(text, False)
