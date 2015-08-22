# -*- coding: utf-8 -*-

"""
***************************************************************************
    widgetBatchBase.py
    ---------------------
    Date                 : June 2010
    Copyright            : (C) 2010 by Giuseppe Sucameli
    Email                : brush dot tyler at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Giuseppe Sucameli'
__date__ = 'June 2010'
__copyright__ = '(C) 2010, Giuseppe Sucameli'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from PyQt4.QtCore import Qt, QFile, QFileInfo
from PyQt4.QtGui import QMessageBox, QErrorMessage

from widgetPluginBase import GdalToolsBasePluginWidget as BasePluginWidget
import GdalTools_utils as Utils


class GdalToolsBaseBatchWidget(BasePluginWidget):

    def __init__(self, iface, commandName):
        BasePluginWidget.__init__(self, iface, commandName)

    def getBatchArguments(self, inFile, outFile=None):
        arguments = []
        arguments.extend(self.getArguments())
        arguments.append(inFile)
        if outFile is not None:
            arguments.append(outFile)
        return arguments

    def isBatchEnabled(self):
        return False

    def isRecursiveScanEnabled(self):
        return False

    def setProgressRange(self, maximum):
        pass

    def updateProgress(self, value, maximum):
        pass

    def getBatchOutputFileName(self, fn):
        inDir = self.getInputFileName()
        outDir = self.getOutputFileName()

        # if overwrites existent files
        if outDir is None or outDir == inDir:
            return fn + ".tmp"

        return outDir + fn[len(inDir):]

    def onRun(self):
        if not self.isBatchEnabled():
            BasePluginWidget.onRun(self)
            return

        self.batchRun()

    def batchRun(self):
        self.inFiles = Utils.getRasterFiles(self.getInputFileName(), self.isRecursiveScanEnabled())
        if len(self.inFiles) == 0:
            QMessageBox.warning(self, self.tr("Warning"), self.tr("No input files to process."))
            return

        self.outFiles = []
        for f in self.inFiles:
            self.outFiles.append(self.getBatchOutputFileName(f))

        self.base.enableRun(False)
        self.base.setCursor(Qt.WaitCursor)

        self.errors = []
        self.batchIndex = 0
        self.batchTotal = len(self.inFiles)
        self.setProgressRange(self.batchTotal)

        self.runItem(self.batchIndex, self.batchTotal)

    def runItem(self, index, total):
        self.updateProgress(index, total)

        if index >= total:
            self.batchFinished()
            return

        outFile = None
        if len(self.outFiles) > index:
            outFile = self.outFiles[index]

        args = self.getBatchArguments(self.inFiles[index], outFile)
        self.base.refreshArgs(args)
        BasePluginWidget.onRun(self)

    def onFinished(self, exitCode, status):
        if not self.isBatchEnabled():
            BasePluginWidget.onFinished(self, exitCode, status)
            return

        msg = bytes.decode(bytes(self.base.process.readAllStandardError()))
        if msg != '':
            self.errors.append(">> " + self.inFiles[self.batchIndex] + "<br>" + msg.replace("\n", "<br>"))

        self.base.process.close()

        # overwrite existent files
        inDir = self.getInputFileName()
        outDir = self.getOutputFileName()
        if outDir is None or inDir == outDir:
            oldFile = QFile(self.inFiles[self.batchIndex])
            newFile = QFile(self.outFiles[self.batchIndex])
            if oldFile.remove():
                newFile.rename(self.inFiles[self.batchIndex])

        self.batchIndex += 1
        self.runItem(self.batchIndex, self.batchTotal)

    def batchFinished(self):
        self.base.stop()

        if len(self.errors) > 0:
            msg = u"Processing of the following files ended with error: <br><br>" + "<br><br>".join(self.errors)
            QErrorMessage(self).showMessage(msg)

        inDir = self.getInputFileName()
        outDir = self.getOutputFileName()
        if outDir is None or inDir == outDir:
            self.outFiles = self.inFiles

        # load layers managing the render flag to avoid waste of time
        canvas = self.iface.mapCanvas()
        previousRenderFlag = canvas.renderFlag()
        canvas.setRenderFlag(False)
        notCreatedList = []
        for item in self.outFiles:
            fileInfo = QFileInfo(item)
            if fileInfo.exists():
                if self.base.loadCheckBox.isChecked():
                    self.addLayerIntoCanvas(fileInfo)
            else:
                notCreatedList.append(item)
        canvas.setRenderFlag(previousRenderFlag)

        if len(notCreatedList) == 0:
            QMessageBox.information(self, self.tr("Finished"), self.tr("Operation completed."))
        else:
            QMessageBox.warning(self, self.tr("Warning"), self.tr("The following files were not created: \n{0}").format(', '.join(notCreatedList)))
