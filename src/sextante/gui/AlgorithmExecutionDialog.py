#!/usr/bin/env python
# -*- coding: latin-1 -*-
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4 import QtCore, QtGui, QtWebKit
from sextante.core.QGisLayers import QGisLayers
from sextante.parameters.ParameterRaster import ParameterRaster
from sextante.parameters.ParameterVector import ParameterVector
from sextante.parameters.ParameterBoolean import ParameterBoolean
from sextante.parameters.ParameterSelection import ParameterSelection
from sextante.parameters.ParameterMultipleInput import ParameterMultipleInput
from sextante.parameters.ParameterFixedTable import ParameterFixedTable
from sextante.parameters.ParameterTableField import ParameterTableField
from sextante.parameters.ParameterTable import ParameterTable
from sextante.gui.AlgorithmExecutor import AlgorithmExecutor
from sextante.core.SextanteLog import SextanteLog
from sextante.gui.SextantePostprocessing import SextantePostprocessing
from sextante.parameters.ParameterRange import ParameterRange
from sextante.parameters.ParameterNumber import ParameterNumber

from sextante.gui.ParametersPanel import ParametersPanel
from sextante.parameters.ParameterFile import ParameterFile
from sextante.parameters.ParameterCrs import ParameterCrs
from sextante.core.SextanteConfig import SextanteConfig
from sextante.parameters.ParameterExtent import ParameterExtent
from sextante.outputs.OutputHTML import OutputHTML
from sextante.outputs.OutputRaster import OutputRaster
from sextante.outputs.OutputVector import OutputVector
from sextante.outputs.OutputTable import OutputTable
from sextante.core.WrongHelpFileException import WrongHelpFileException
import os
from sextante.gui.UnthreadedAlgorithmExecutor import UnthreadedAlgorithmExecutor

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class AlgorithmExecutionDialog(QtGui.QDialog):
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
            QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Close|QtGui.QDialogButtonBox.Ok)
        self.buttonBox.button(QtGui.QDialogButtonBox.Cancel).setEnabled(False)

        self.scrollArea = QtGui.QScrollArea()
        if self.mainWidget:
            self.scrollArea.setWidget(self.mainWidget)
        self.scrollArea.setWidgetResizable(True)
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
        self.tabWidget.addTab(self.scrollArea, "Parameters")
        self.verticalLayout.addWidget(self.tabWidget)
        self.logText = QTextEdit()
        self.logText.readOnly = True
        if SextanteConfig.getSetting(SextanteConfig.USE_THREADS):
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
        self.buttonBox.accepted.connect(self.accept)
        self.buttonBox.rejected.connect(self.close)
        self.buttonBox.button(QtGui.QDialogButtonBox.Cancel).clicked.connect(self.cancel)
        #~ QtCore.QMetaObject.connectSlotsByName(self)


    def setParamValues(self):
        params = self.alg.parameters
        outputs = self.alg.outputs

        for param in params:
            if param.hidden:
                continue
            if isinstance(param, ParameterExtent):
                continue
            if not self.setParamValue(param, self.paramTable.valueItems[param.name]):
                return False

        for param in params:
            if isinstance(param, ParameterExtent):
                value = self.paramTable.valueItems[param.name].getValue()
                if value is not None:
                    param.value = value
                else:
                    return False

        for output in outputs:
            if output.hidden:
                continue
            output.value = self.paramTable.valueItems[output.name].getValue()
            if not SextanteConfig.getSetting(SextanteConfig.TABLE_LIKE_PARAM_PANEL):
                if isinstance(output, (OutputRaster, OutputVector, OutputTable, OutputHTML)):
                    output.open = self.paramTable.checkBoxes[output.name].isChecked()

        return True

    def setParamValue(self, param, widget):
        if isinstance(param, ParameterRaster):
            return param.setValue(widget.getValue())
        elif isinstance(param, (ParameterVector, ParameterTable)):
            try:
                return param.setValue(widget.itemData(widget.currentIndex()).toPyObject())
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
                options = QGisLayers.getVectorLayers()
            else:
                options = QGisLayers.getRasterLayers()
            value = []
            for index in widget.selectedoptions:
                value.append(options[index])
            return param.setValue(value)
        elif isinstance(param, (ParameterNumber, ParameterFile, ParameterCrs, ParameterExtent)):
            return param.setValue(widget.getValue())
        else:
            return param.setValue(str(widget.text()))

    @pyqtSlot()
    def accept(self):
        if self.setParamValues():
            msg = self.alg.checkParameterValuesBeforeExecuting()
            if msg:
                QMessageBox.critical(self, "Unable to execute algorithm", msg)
                return
            self.buttonBox.button(QtGui.QDialogButtonBox.Ok).setEnabled(False)
            self.buttonBox.button(QtGui.QDialogButtonBox.Close).setEnabled(False)
            buttons = self.paramTable.iterateButtons
            iterateParam = None

            for i in range(len(buttons.values())):
                button = buttons.values()[i]
                if button.isChecked():
                    iterateParam = buttons.keys()[i]
                    break

            self.progress.setMaximum(0)
            self.progressLabel.setText("Processing algorithm...")
            QApplication.setOverrideCursor(QCursor(Qt.WaitCursor))
            useThread = SextanteConfig.getSetting(SextanteConfig.USE_THREADS)
            if useThread:
                if iterateParam:
                    self.algEx = AlgorithmExecutor(self.alg, iterateParam)
                else:
                    command = self.alg.getAsCommand()
                    if command:
                        SextanteLog.addToLog(SextanteLog.LOG_ALGORITHM, command)
                    self.algEx = AlgorithmExecutor(self.alg)
                self.algEx.finished.connect(self.finish)
                self.algEx.error.connect(self.error)
                self.algEx.percentageChanged.connect(self.setPercentage)
                self.algEx.textChanged.connect(self.setText)
                self.algEx.iterated.connect(self.iterate)
                self.algEx.infoSet.connect(self.setInfo)
                self.algEx.start()
                self.setInfo("Algorithm %s started" % self.alg.name)
                self.buttonBox.button(QtGui.QDialogButtonBox.Cancel).setEnabled(True)
                self.tabWidget.setCurrentIndex(1) # log tab
            else:
                if iterateParam:
                    UnthreadedAlgorithmExecutor.runalgIterating(self.alg, iterateParam, self)
                else:
                    command = self.alg.getAsCommand()
                    if command:
                        SextanteLog.addToLog(SextanteLog.LOG_ALGORITHM, command)
                    if UnthreadedAlgorithmExecutor.runalg(self.alg, self):
                        self.finish()
        else:
            QMessageBox.critical(self, "Unable to execute algorithm", "Wrong or missing parameter values")

    @pyqtSlot()
    def finish(self):
        keepOpen = SextanteConfig.getSetting(SextanteConfig.KEEP_DIALOG_OPEN)
        SextantePostprocessing.handleAlgorithmResults(self.alg, not keepOpen)
        self.executed = True
        self.setInfo("Algorithm %s finished correctly" % self.alg.name)
        QApplication.restoreOverrideCursor()
        if not keepOpen:
            self.close()
        else:
            self.progressLabel.setText("")
            self.progress.setMaximum(100)
            self.progress.setValue(0)
            self.buttonBox.button(QtGui.QDialogButtonBox.Ok).setEnabled(True)
            self.buttonBox.button(QtGui.QDialogButtonBox.Close).setEnabled(True)
        self.buttonBox.button(QtGui.QDialogButtonBox.Cancel).setEnabled(False)

    @pyqtSlot(str)
    def error(self, msg):
        self.algEx.finished.disconnect()
        QApplication.restoreOverrideCursor()
        self.setInfo(msg, True)
        QMessageBox.critical(self, "Error", msg)
        keepOpen = SextanteConfig.getSetting(SextanteConfig.KEEP_DIALOG_OPEN)
        if not keepOpen:
            self.close()
        else:
            self.progressLabel.setText("")
            self.progress.setValue(0)
            self.buttonBox.button(QtGui.QDialogButtonBox.Ok).setEnabled(True)

    @pyqtSlot(int)
    def iterate(self, i):
        self.setInfo("Algorithm %s iteration #%i completed" % (self.alg.name, i))

    @pyqtSlot()
    def cancel(self):
        self.setInfo("Algorithm %s canceled" % self.alg.name)
        try:
            self.algEx.finished.disconnect()
            self.algEx.terminate()
            QApplication.restoreOverrideCursor()
            self.buttonBox.button(QtGui.QDialogButtonBox.Cancel).setEnabled(False)
        except:
            pass

    @pyqtSlot(str)
    def setInfo(self, msg, error = False):
        if error:
            SextanteLog.addToLog(SextanteLog.LOG_ERROR, msg)
            self.logText.append('<b>' + msg + '</b>')
        else:
            SextanteLog.addToLog(SextanteLog.LOG_INFO, msg)
            self.logText.append(msg)

    def setPercentage(self, i):
        if self.progress.maximum() == 0:
            self.progress.setMaximum(100)
        self.progress.setValue(i)

    def setText(self, text):
        self.progressLabel.setText(text)
        self.setInfo(text, False)
