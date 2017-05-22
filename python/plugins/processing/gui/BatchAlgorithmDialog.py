# -*- coding: utf-8 -*-

"""
***************************************************************************
    BatchAlgorithmDialog.py
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
from builtins import range

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.PyQt.QtWidgets import QApplication, QMessageBox, QSizePolicy
from qgis.PyQt.QtGui import QCursor
from qgis.PyQt.QtCore import Qt

from qgis.core import QgsProcessingParameterDefinition
from qgis.gui import QgsMessageBar

from processing.gui.BatchPanel import BatchPanel
from processing.gui.AlgorithmDialogBase import AlgorithmDialogBase
from processing.gui.AlgorithmExecutor import execute
from processing.gui.Postprocessing import handleAlgorithmResults

from processing.core.ProcessingResults import ProcessingResults

from processing.core.outputs import OutputNumber
from processing.core.outputs import OutputString
from processing.core.outputs import OutputHTML

from processing.tools.system import getTempFilename
from processing.tools import dataobjects

import codecs


class BatchAlgorithmDialog(AlgorithmDialogBase):

    def __init__(self, alg):
        AlgorithmDialogBase.__init__(self, alg)

        self.alg = alg
        self.alg_parameters = []

        self.setWindowTitle(self.tr('Batch Processing - {0}').format(self.alg.displayName()))

        self.setMainWidget(BatchPanel(self, self.alg))

        self.textShortHelp.setVisible(False)

        self.bar = QgsMessageBar()
        self.bar.setSizePolicy(QSizePolicy.Minimum, QSizePolicy.Fixed)
        self.layout().insertWidget(0, self.bar)

    def accept(self):
        self.alg_parameters = []
        self.load = []
        self.canceled = False

        context = dataobjects.createContext()

        for row in range(self.mainWidget.tblParameters.rowCount()):
            col = 0
            parameters = {}
            for param in self.alg.parameterDefinitions():
                if param.flags() & QgsProcessingParameterDefinition.FlagHidden or param.isDestination():
                    continue
                wrapper = self.mainWidget.wrappers[row][col]
                parameters[param.name()] = wrapper.value()
                if not param.checkValueIsAcceptable(wrapper.value(), context):
                    self.bar.pushMessage("", self.tr('Wrong or missing parameter value: {0} (row {1})').format(
                                         param.description(), row + 1),
                                         level=QgsMessageBar.WARNING, duration=5)
                    self.algs = None
                    return
                col += 1
            for out in alg.destinationParameterDefinitions():
                if out.flags() & QgsProcessingParameterDefinition.FlagHidden:
                    continue

                widget = self.mainWidget.tblParameters.cellWidget(row, col)
                text = widget.getValue()
                if text.strip() != '':
                    out.value = text
                    col += 1
                else:
                    self.bar.pushMessage("", self.tr('Wrong or missing output value: {0} (row {1})').format(
                                         out.description(), row + 1),
                                         level=QgsMessageBar.WARNING, duration=5)
                    self.algs = None
                    return

            self.alg_parameters.append(parameters)
            if self.alg.countVisibleOutputs():
                widget = self.mainWidget.tblParameters.cellWidget(row, col)
                self.load.append(widget.currentIndex() == 0)
            else:
                self.load.append(False)

        QApplication.setOverrideCursor(QCursor(Qt.WaitCursor))
        self.mainWidget.setEnabled(False)

        self.progressBar.setMaximum(len(self.algs))
        # Make sure the Log tab is visible before executing the algorithm
        try:
            self.tabWidget.setCurrentIndex(1)
            self.repaint()
        except:
            pass

        for count, parameters in enumerate(self.alg_parameters):
            self.setText(self.tr('\nProcessing algorithm {0}/{1}...').format(count + 1, len(self.alg_parameters)))
            self.setInfo(self.tr('<b>Algorithm {0} starting...</b>').format(self.alg.displayName()))
            ret, results = execute(self.alg, parameters, context, self.feedback)
            if ret and not self.canceled:
                if self.load[count]:
                    handleAlgorithmResults(self.alg, context, self.feedback, False)
                self.setInfo(self.tr('Algorithm {0} correctly executed...').format(self.alg.displayName()))
            else:
                QApplication.restoreOverrideCursor()
                return

        self.finish()

    def finish(self):
        for count, parameters in enumerate(self.alg_parameters):
            self.loadHTMLResults(self.alg, count)

        self.createSummaryTable()
        QApplication.restoreOverrideCursor()

        self.mainWidget.setEnabled(True)
        QMessageBox.information(self, self.tr('Batch processing'),
                                self.tr('Batch processing completed'))

    def loadHTMLResults(self, alg, num):
        for out in alg.outputs:
            if out.flags() & QgsProcessingParameterDefinition.FlagHidden or not out.open:
                continue

            if isinstance(out, OutputHTML):
                ProcessingResults.addResult(
                    '{} [{}]'.format(out.description(), num), out.value)

    def createSummaryTable(self):
        createTable = False

        for out in self.algs[0].outputs:
            if isinstance(out, (OutputNumber, OutputString)):
                createTable = True
                break

        if not createTable:
            return

        outputFile = getTempFilename('html')
        with codecs.open(outputFile, 'w', encoding='utf-8') as f:
            for alg in self.algs:
                f.write('<hr>\n')
                for out in alg.outputs:
                    if isinstance(out, (OutputNumber, OutputString)):
                        f.write('<p>{}: {}</p>\n'.format(out.description(), out.value))
            f.write('<hr>\n')

        ProcessingResults.addResult(
            '{} [summary]'.format(self.algs[0].name), outputFile)
