# -*- coding: utf-8 -*-

"""
***************************************************************************
    AlgorithmExecutionDialog.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
                           (C) 2013 by CS Systemes d'information (CS SI)
    Email                : volayaf at gmail dot com
                           otb at c-s dot fr (CS SI)
    Contributors         : Victor Olaya
                           Alexia Mondot (CS SI) - managing the new parameter
                           ParameterMultipleExternalInput
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

import os
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4.QtWebKit import *

from processing.core.ProcessingLog import ProcessingLog
from processing.core.ProcessingConfig import ProcessingConfig
from processing.gui.Postprocessing import handleAlgorithmResults
from processing.gui.AlgorithmExecutor import runalg, runalgIterating
from processing.core.parameters import *
from processing.core.outputs import OutputRaster
from processing.core.outputs import OutputVector
from processing.core.outputs import OutputTable
from processing.tools import dataobjects
from qgis.utils import iface

class AlgorithmExecutionDialog(QDialog):

    class InvalidParameterValue(Exception):

        def __init__(self, param, widget):
            (self.parameter, self.widget) = (param, widget)

    def __init__(self, alg, mainWidget):
        QDialog.__init__(self, iface.mainWindow(),
            Qt.WindowSystemMenuHint | Qt.WindowTitleHint)
        self.executed = False
        self.mainWidget = mainWidget
        self.alg = alg
        self.resize(650, 450)
        self.buttonBox = QDialogButtonBox()
        self.buttonBox.setOrientation(Qt.Horizontal)
        self.buttonBox.setStandardButtons(QDialogButtonBox.Close)
        self.runButton = QPushButton()
        self.runButton.setText(self.tr('Run'))
        self.buttonBox.addButton(self.runButton, QDialogButtonBox.ActionRole)
        self.runButton.clicked.connect(self.accept)
        self.setWindowTitle(self.alg.name)
        self.progressLabel = QLabel()
        self.progress = QProgressBar()
        self.progress.setMinimum(0)
        self.progress.setMaximum(100)
        self.progress.setValue(0)
        self.verticalLayout = QVBoxLayout(self)
        self.verticalLayout.setSpacing(6)
        self.verticalLayout.setMargin(9)
        self.tabWidget = QTabWidget()
        self.tabWidget.setMinimumWidth(300)
        self.tabWidget.addTab(self.mainWidget, self.tr('Parameters'))
        self.verticalLayout.addWidget(self.tabWidget)
        self.logText = QTextEdit()
        self.logText.readOnly = True
        self.tabWidget.addTab(self.logText, self.tr('Log'))
        self.webView = QWebView()
        html = None
        url = None
        isText, help = self.alg.help()
        if help is not None:
            if isText:
                html = help;
            else:
                url = QUrl(help)
        else:
            html = self.tr('<h2>Sorry, no help is available for this '
                           'algorithm.</h2>')
        try:
            if html:
                self.webView.setHtml(html)
            elif url:
                self.webView.load(url)
        except:
            self.webView.setHtml(
                self.tr('<h2>Could not open help file :-( </h2>'))
        self.tabWidget.addTab(self.webView, 'Help')
        self.verticalLayout.addWidget(self.progressLabel)
        self.verticalLayout.addWidget(self.progress)
        self.verticalLayout.addWidget(self.buttonBox)
        self.setLayout(self.verticalLayout)
        self.buttonBox.rejected.connect(self.close)

        self.showDebug = ProcessingConfig.getSetting(
                ProcessingConfig.SHOW_DEBUG_IN_DIALOG)

    def setParamValues(self):
        params = self.alg.parameters
        outputs = self.alg.outputs

        for param in params:
            if param.hidden:
                continue
            if isinstance(param, ParameterExtent):
                continue
            if not self.setParamValue(
                    param, self.paramTable.valueItems[param.name]):
                raise AlgorithmExecutionDialog.InvalidParameterValue(param,
                        self.paramTable.valueItems[param.name])

        for param in params:
            if isinstance(param, ParameterExtent):
                if not self.setParamValue(
                        param, self.paramTable.valueItems[param.name]):
                    raise AlgorithmExecutionDialog.InvalidParameterValue(
                        param, self.paramTable.valueItems[param.name])

        for output in outputs:
            if output.hidden:
                continue
            output.value = self.paramTable.valueItems[output.name].getValue()
            if isinstance(output, (OutputRaster, OutputVector, OutputTable)):
                output.open = self.paramTable.checkBoxes[output.name].isChecked()

        return True

    def setParamValue(self, param, widget):
        """
        set the .value of the parameter according to the given widget
        the way to get the value is different for each value,
        so there is a code for each kind of parameter

        param : -il <ParameterMultipleInput> or -method <ParameterSelection> ...
        """
        if isinstance(param, ParameterRaster):
            return param.setValue(widget.getValue())
        elif isinstance(param, (ParameterVector, ParameterTable)):
            try:
                return param.setValue(widget.itemData(widget.currentIndex()))
            except:
                return param.setValue(widget.getValue())
        elif isinstance(param, ParameterBoolean):
            return param.setValue(widget.isChecked())
        elif isinstance(param, ParameterSelection):
            return param.setValue(widget.currentIndex())
        elif isinstance(param, ParameterFixedTable):
            return param.setValue(widget.table)
        elif isinstance(param, ParameterRange):
            return param.setValue(widget.getValue())
        if isinstance(param, ParameterTableField):
            if param.optional and widget.currentIndex() == 0:
                return param.setValue(None)
            return param.setValue(widget.currentText())
        elif isinstance(param, ParameterMultipleInput):
            if param.datatype == ParameterMultipleInput.TYPE_FILE:
                return param.setValue(widget.selectedoptions)
            else:
                if param.datatype == ParameterMultipleInput.TYPE_VECTOR_ANY:
                    options = dataobjects.getVectorLayers(sorting=False)
                else:
                    options = dataobjects.getRasterLayers(sorting=False)
                return param.setValue([options[i] for i in widget.selectedoptions])
        elif isinstance(param, (ParameterNumber, ParameterFile, ParameterCrs,
                        ParameterExtent)):
            return param.setValue(widget.getValue())
        elif isinstance(param, ParameterString):
            if param.multiline:
                return param.setValue(unicode(widget.toPlainText()))
            else:
                return param.setValue(unicode(widget.text()))
        else:
            return param.setValue(unicode(widget.text()))

    def accept(self):
        checkCRS = ProcessingConfig.getSetting(
                ProcessingConfig.WARN_UNMATCHING_CRS)
        try:
            self.setParamValues()
            if checkCRS and not self.alg.checkInputCRS():
                reply = QMessageBox.question(self, self.tr("Unmatching CRS's"),
                    self.tr('Layers do not all use the same CRS. This can '
                            'cause unexpected results.\nDo you want to '
                            'continue?'),
                    QMessageBox.Yes | QMessageBox.No,
                    QMessageBox.No)
                if reply == QMessageBox.No:
                    return
            msg = self.alg.checkParameterValuesBeforeExecuting()
            if msg:
                QMessageBox.warning(
                    self, self.tr('Unable to execute algorithm'), msg)
                return
            self.runButton.setEnabled(False)
            self.buttonBox.button(
                    QDialogButtonBox.Close).setEnabled(False)
            buttons = self.paramTable.iterateButtons
            self.iterateParam = None

            for i in range(len(buttons.values())):
                button = buttons.values()[i]
                if button.isChecked():
                    self.iterateParam = buttons.keys()[i]
                    break

            self.tabWidget.setCurrentIndex(1)  # Log tab
            self.progress.setMaximum(0)
            self.progressLabel.setText(self.tr('Processing algorithm...'))
            QApplication.setOverrideCursor(QCursor(Qt.WaitCursor))

            self.setInfo(
                self.tr('<b>Algorithm %s starting...</b>') % self.alg.name)
            # make sure the log tab is visible before executing the algorithm
            try:
                self.repaint()
            except:
                pass
            if self.iterateParam:
                if runalgIterating(self.alg,
                        self.iterateParam, self):
                    self.finish()
                else:
                    QApplication.restoreOverrideCursor()
                    self.resetGUI()
            else:
                command = self.alg.getAsCommand()
                if command:
                    ProcessingLog.addToLog(ProcessingLog.LOG_ALGORITHM,
                            command)
                if runalg(self.alg, self):
                    self.finish()
                else:
                    QApplication.restoreOverrideCursor()
                    self.resetGUI()
        except AlgorithmExecutionDialog.InvalidParameterValue, ex:
            try:
                self.buttonBox.accepted.connect(lambda :
                        ex.widget.setPalette(QPalette()))
                palette = ex.widget.palette()
                palette.setColor(QPalette.Base, QColor(255, 255, 0))
                ex.widget.setPalette(palette)
                self.progressLabel.setText(
                    self.tr('<b>Missing parameter value: %s</b>' % ex.parameter.description))
                return
            except:
                QMessageBox.critical(self,
                    self.tr('Unable to execute algorithm'),
                    self.tr('Wrong or missing parameter values'))

    def finish(self):
        keepOpen = ProcessingConfig.getSetting(
                ProcessingConfig.KEEP_DIALOG_OPEN)
        if self.iterateParam is None:
            handleAlgorithmResults(self.alg, self, not keepOpen)
        self.executed = True
        self.setInfo('Algorithm %s finished' % self.alg.name)
        QApplication.restoreOverrideCursor()
        if not keepOpen:
            self.close()
        else:
            self.resetGUI()
            if self.alg.getHTMLOutputsCount() > 0:
                self.setInfo(
                    self.tr('HTML output has been generated by this algorithm.'
                            '\nOpen the results dialog to check it.'))

    def error(self, msg):
        QApplication.restoreOverrideCursor()
        self.setInfo(msg, True)
        self.resetGUI()
        self.tabWidget.setCurrentIndex(1)  # log tab

    def resetGUI(self):
        QApplication.restoreOverrideCursor()
        self.progressLabel.setText('')
        self.progress.setMaximum(100)
        self.progress.setValue(0)
        self.runButton.setEnabled(True)
        self.buttonBox.button(QDialogButtonBox.Close).setEnabled(True)

    def setInfo(self, msg, error=False):
        if error:
            self.logText.append('<span style="color:red">' + msg + '</span>')
        else:
            self.logText.append(msg)
        QCoreApplication.processEvents()

    def setCommand(self, cmd):
        if self.showDebug:
            self.setInfo('<tt>' + cmd + '<tt>')
        QCoreApplication.processEvents()

    def setDebugInfo(self, msg):
        if self.showDebug:
            self.setInfo('<span style="color:blue">' + msg + '</span>')
        QCoreApplication.processEvents()

    def setConsoleInfo(self, msg):
        if self.showDebug:
            self.setCommand('<span style="color:darkgray">' + msg + '</span>')
        QCoreApplication.processEvents()

    def setPercentage(self, i):
        if self.progress.maximum() == 0:
            self.progress.setMaximum(100)
        self.progress.setValue(i)
        QCoreApplication.processEvents()

    def setText(self, text):
        self.progressLabel.setText(text)
        self.setInfo(text, False)
        QCoreApplication.processEvents()
