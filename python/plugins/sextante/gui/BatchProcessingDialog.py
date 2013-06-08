# -*- coding: utf-8 -*-

"""
***************************************************************************
    BatchProcessingDialog.py
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

from PyQt4 import QtCore, QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from sextante.gui.SextantePostprocessing import SextantePostprocessing
from sextante.parameters.ParameterFile import ParameterFile
from sextante.gui.FileSelectionPanel import FileSelectionPanel
from sextante.parameters.ParameterRaster import ParameterRaster
from sextante.parameters.ParameterTable import ParameterTable
from sextante.parameters.ParameterVector import ParameterVector
from sextante.gui.BatchInputSelectionPanel import BatchInputSelectionPanel
from sextante.gui.AlgorithmExecutionDialog import AlgorithmExecutionDialog
from sextante.gui.CrsSelectionPanel import CrsSelectionPanel
from sextante.outputs.OutputNumber import OutputNumber
from sextante.outputs.OutputString import OutputString
from sextante.core.SextanteUtils import SextanteUtils
from sextante.parameters.ParameterExtent import ParameterExtent
from sextante.parameters.ParameterCrs import ParameterCrs
from sextante.gui.ExtentSelectionPanel import ExtentSelectionPanel
from sextante.parameters.ParameterBoolean import ParameterBoolean
from sextante.parameters.ParameterSelection import ParameterSelection
from sextante.parameters.ParameterFixedTable import ParameterFixedTable
from sextante.gui.FixedTablePanel import FixedTablePanel
from sextante.parameters.ParameterMultipleInput import ParameterMultipleInput
from sextante.gui.BatchOutputSelectionPanel import BatchOutputSelectionPanel
from sextante.gui.AlgorithmExecutor import AlgorithmExecutor
from sextante.outputs.OutputHTML import OutputHTML
from sextante.core.SextanteResults import SextanteResults
from sextante.core.SextanteLog import SextanteLog
from sextante.core.SextanteConfig import SextanteConfig
from sextante.gui.UnthreadedAlgorithmExecutor import UnthreadedAlgorithmExecutor

class BatchProcessingDialog(AlgorithmExecutionDialog):
    def __init__(self, alg):
        self.algs = None
        self.showAdvanced = False
        self.table = QtGui.QTableWidget(None)
        AlgorithmExecutionDialog.__init__(self, alg, self.table)
        self.setWindowModality(1)
        self.algEx = None
        self.resize(800, 500)
        self.setWindowTitle("Batch Processing - " + self.alg.name)
        for param in self.alg.parameters:
            if param.isAdvanced:
                self.advancedButton = QtGui.QPushButton()
                self.advancedButton.setText("Show advanced parameters")
                self.advancedButton.setMaximumWidth(150)
                self.buttonBox.addButton(self.advancedButton, QtGui.QDialogButtonBox.ActionRole)
                self.advancedButton.clicked.connect(self.showAdvancedParametersClicked)
                break
        self.addRowButton = QtGui.QPushButton()
        self.addRowButton.setText("Add row")
        self.buttonBox.addButton(self.addRowButton, QtGui.QDialogButtonBox.ActionRole)
        self.deleteRowButton = QtGui.QPushButton()
        self.deleteRowButton.setText("Delete row")
        self.buttonBox.addButton(self.addRowButton, QtGui.QDialogButtonBox.ActionRole)
        self.buttonBox.addButton(self.deleteRowButton, QtGui.QDialogButtonBox.ActionRole)

        self.table.setColumnCount(self.alg.getVisibleParametersCount() + self.alg.getVisibleOutputsCount() + 1)
        self.setTableContent()
        self.table.horizontalHeader().setStretchLastSection(True)
        self.table.verticalHeader().setVisible(False)
        self.table.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)
        self.addRowButton.clicked.connect(self.addRow)
        self.deleteRowButton.clicked.connect(self.deleteRow)
        self.table.horizontalHeader().sectionDoubleClicked.connect(self.headerDoubleClicked)


    def headerDoubleClicked(self, col):
        widget = self.table.cellWidget(0, col)
        if isinstance(widget, QtGui.QComboBox):
            widgetValue = widget.currentIndex()
            for row in range(1, self.table.rowCount()):
                self.table.cellWidget(row, col).setCurrentIndex(widgetValue)
        elif isinstance(widget, ExtentSelectionPanel):
            widgetValue = widget.getValue()
            for row in range(1, self.table.rowCount()):
                if widgetValue != None:
                    self.table.cellWidget(row, col).text.setText(widgetValue)
                else:
                    self.table.cellWidget(row, col).text.setText("")
        elif isinstance(widget, CrsSelectionPanel):
            widgetValue = widget.getValue()
            for row in range(1, self.table.rowCount()):
                self.table.cellWidget(row, col).setAuthid(widgetValue)
        elif isinstance(widget, FileSelectionPanel):
            widgetValue = widget.getValue()
            for row in range(1, self.table.rowCount()):
                self.table.cellWidget(row, col).setText(widgetValue)
        elif isinstance(widget, QtGui.QLineEdit):
            widgetValue = widget.text()
            for row in range(1, self.table.rowCount()):
                self.table.cellWidget(row, col).setText(widgetValue)
        elif isinstance(widget, BatchInputSelectionPanel):
            widgetValue = widget.getText()
            for row in range(1, self.table.rowCount()):
                self.table.cellWidget(row, col).setText(widgetValue)

        else:
            pass


    def setTableContent(self):
        i = 0
        for param in self.alg.parameters:
            self.table.setColumnWidth(i,250)
            self.table.setHorizontalHeaderItem(i, QtGui.QTableWidgetItem(param.description))
            if param.isAdvanced:
                self.table.setColumnHidden(i, not self.showAdvanced)
            i+=1
        for out in self.alg.outputs:
            self.table.setColumnWidth(i,250)
            self.table.setHorizontalHeaderItem(i, QtGui.QTableWidgetItem(out.description))
            i+=1

        self.table.setColumnWidth(i, 200)
        self.table.setHorizontalHeaderItem(i, QtGui.QTableWidgetItem("Load in QGIS"))

        for i in range(3):
            self.addRow()


    def accept(self):
        self.algs = []
        self.load = []
        for row in range(self.table.rowCount()):
            alg = self.alg.getCopy()#copy.deepcopy(self.alg)
            col = 0
            for param in alg.parameters:
                if param.hidden:
                    continue
                widget = self.table.cellWidget(row, col)
                if not self.setParameterValueFromWidget(param, widget, alg):
                    self.progressLabel.setText("<b>Missing parameter value: " + param.description + " (row " + str(row + 1) + ")</b>")                    
                    self.algs = None
                    return
                col+=1
            for out in alg.outputs:
                if out.hidden:
                    continue
                widget = self.table.cellWidget(row, col)
                text = widget.getValue()
                if text.strip() != "":
                    out.value = text
                    col+=1
                else:
                    self.progressLabel.setText("<b>Wrong or missing parameter value: " + out.description + " (row " + str(row + 1) + ")</b>")                    
                    self.algs = None
                    return
            self.algs.append(alg)
            widget = self.table.cellWidget(row, col)
            self.load.append(widget.currentIndex() == 0)

        QApplication.setOverrideCursor(QCursor(Qt.WaitCursor))
        self.table.setEnabled(False)
        if SextanteConfig.getSetting(SextanteConfig.USE_THREADS):
            self.tabWidget.setCurrentIndex(1)
            self.nextAlg(0)
        else:
            i=0
            self.progress.setMaximum(len(self.algs))
            for alg in self.algs:
                self.setBaseText("Processing algorithm " + str(i+1) + "/" + str(len(self.algs)) + "...")
                if UnthreadedAlgorithmExecutor.runalg(alg, self):
                    if self.load[i]:
                        SextantePostprocessing.handleAlgorithmResults(alg, self, False)
                    i+=1
                else:
                    QApplication.restoreOverrideCursor()
                    return

            self.finishAll()

    def loadHTMLResults(self, alg, i):
        for out in alg.outputs:
            if out.hidden or not out.open:
                continue
            if isinstance(out, OutputHTML):
                SextanteResults.addResult(out.description + "[" + str(i) + "]", out.value)

    def cancel(self):
        self.algs = None
        if self.algEx:
            self.algEx.terminate()
        self.table.setEnabled(True)

    @pyqtSlot()
    def finish(self, i):
        if not self.stop:
            if self.load[i]:
                SextantePostprocessing.handleAlgorithmResults(self.algs[i], self, False)
            i += 1
            if len(self.algs) == i:
                self.finishAll()
                self.algEx = None
            else:
                self.nextAlg(i)

    @pyqtSlot(str)
    def error(self, msg):
        QApplication.restoreOverrideCursor()
        QMessageBox.critical(self, "Error", msg)
        SextanteLog.addToLog(SextanteLog.LOG_ERROR, msg)
        if self.algEx:
            self.algEx.terminate()
        self.table.setEnabled(True)


    def nextAlg(self, i):
        self.stop = False
        self.setBaseText("Processing algorithm " + str(i + 1) + "/" + str(len(self.algs)) + "...")
        self.algEx = AlgorithmExecutor(self.algs[i]);
        self.algEx.percentageChanged.connect(self.setPercentage)
        self.algEx.textChanged.connect(self.setText)
        self.algEx.error.connect(self.error)
        self.algEx.finished.connect(lambda: self.finish(i))
        self.algEx.infoSet.connect(self.setInfo)
        if SextanteConfig.getSetting(SextanteConfig.SHOW_DEBUG_IN_DIALOG):
            self.algEx.commandSet.connect(self.setCommand)
            self.algEx.debugInfoSet.connect(self.setDebugInfo)
            self.algEx.consoleInfoSet.connect(self.setConsoleInfo)
        self.algEx.start()

    def createSummaryTable(self):
        createTable = False
        for out in self.algs[0].outputs:
            if isinstance(out, (OutputNumber,OutputString)):
                createTable = True
                break
        if not createTable:
            return
        outputFile = SextanteUtils.getTempFilename("html")
        f = open(outputFile, "w")
        for alg in self.algs:
            f.write("<hr>\n")
            for out in alg.outputs:
                if isinstance(out, (OutputNumber,OutputString)):
                    f.write("<p>" + out.description + ": " + str(out.value) + "</p>\n")
        f.write("<hr>\n")
        f.close()
        SextanteResults.addResult(self.algs[0].name + "[summary]", outputFile)

    def finishAll(self):
        i = 0
        for alg in self.algs:
            self.loadHTMLResults(alg, i)
            i = i + 1
        self.createSummaryTable()
        QApplication.restoreOverrideCursor()
        self.table.setEnabled(True)
        QMessageBox.information(self, "Batch processing", "Batch processing successfully completed!")

    def setParameterValueFromWidget(self, param, widget, alg = None):
        if isinstance(param, (ParameterRaster, ParameterVector, ParameterTable, ParameterMultipleInput)):
            value = widget.getText()
            if unicode(value).strip() == "":
                value = None
            return param.setValue(value)
        elif isinstance(param, ParameterBoolean):
            return param.setValue(widget.currentIndex() == 0)
        elif isinstance(param, ParameterSelection):
            return param.setValue(widget.currentIndex())
        elif isinstance(param, ParameterFixedTable):
            return param.setValue(widget.table)
        elif isinstance(param, (ParameterExtent)):
            if alg != None:
                widget.useNewAlg(alg)
            return param.setValue(widget.getValue())
        elif isinstance(param, (ParameterCrs, ParameterFile)):
            return param.setValue(widget.getValue())
        else:
            return param.setValue(widget.text())

    def getWidgetFromParameter(self, param, row, col):
        if isinstance(param, (ParameterRaster, ParameterVector, ParameterTable, ParameterMultipleInput)):
            item = BatchInputSelectionPanel(param, row, col, self)
        elif isinstance(param, ParameterBoolean):
            item = QtGui.QComboBox()
            item.addItem("Yes")
            item.addItem("No")
            if param.default:
                item.setCurrentIndex(0)
            else:
                item.setCurrentIndex(1)
        elif isinstance(param, ParameterSelection):
            item = QtGui.QComboBox()
            item.addItems(param.options)
        elif isinstance(param, ParameterFixedTable):
            item = FixedTablePanel(param)
        elif isinstance(param, ParameterExtent):
            item = ExtentSelectionPanel(self, self.alg, param.default)
        elif isinstance(param, ParameterCrs):
            item = CrsSelectionPanel(param.default)
        elif isinstance(param, ParameterFile):
            item = FileSelectionPanel(param.isFolder)
        else:
            item = QtGui.QLineEdit()
            try:
                item.setText(str(param.default))
            except:
                pass

        return item

    def deleteRow(self):
        if self.table.rowCount() > 2:
            self.table.setRowCount(self.table.rowCount()-1)

    def addRow(self):
        self.table.setRowCount(self.table.rowCount()+1)
        self.table.setRowHeight(self.table.rowCount()-1, 22)
        i=0
        for param in self.alg.parameters:
            self.table.setCellWidget(self.table.rowCount()-1,i, self.getWidgetFromParameter(param, self.table.rowCount()-1, i))
            i+=1
        for out in self.alg.outputs:
            self.table.setCellWidget(self.table.rowCount()-1,i, BatchOutputSelectionPanel(out, self.alg, self.table.rowCount()-1, i, self))
            i+=1

        item = QtGui.QComboBox()
        item.addItem("Yes")
        item.addItem("No")
        item.setCurrentIndex(0)
        self.table.setCellWidget(self.table.rowCount()-1, i, item)

    def showAdvancedParametersClicked(self):
        self.showAdvanced = not self.showAdvanced
        if self.showAdvanced:
            self.advancedButton.setText("Hide advanced parameters")
        else:
            self.advancedButton.setText("Show advanced parameters")
        i = 0
        for param in self.alg.parameters:
            if param.isAdvanced:
                self.table.setColumnHidden(i, not self.showAdvanced)
            i+=1

    def setText(self, text):
        self.progressLabel.setText(self.baseText + "   --- [" + text + "]")

    def setBaseText(self, text):
        self.baseText = text