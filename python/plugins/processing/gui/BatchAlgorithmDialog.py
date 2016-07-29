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

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.PyQt.QtWidgets import QApplication, QMessageBox
from qgis.PyQt.QtGui import QCursor
from qgis.PyQt.QtCore import Qt

from processing.gui.BatchPanel import BatchPanel
from processing.gui.AlgorithmDialogBase import AlgorithmDialogBase
from processing.gui.AlgorithmExecutor import runalg
from processing.gui.Postprocessing import handleAlgorithmResults

from processing.core.ProcessingResults import ProcessingResults

from processing.core.parameters import ParameterExtent
from processing.core.outputs import OutputNumber
from processing.core.outputs import OutputString
from processing.core.outputs import OutputHTML

from processing.tools.system import getTempFilename

import codecs


class BatchAlgorithmDialog(AlgorithmDialogBase):

    def __init__(self, alg):
        AlgorithmDialogBase.__init__(self, alg)

        self.alg = alg

        self.setWindowTitle(self.tr('Batch Processing - %s') % self.alg.name)

        self.mainWidget = BatchPanel(self, self.alg)
        self.setMainWidget()

        self.textShortHelp.setVisible(False)

    def accept(self):
        self.algs = []
        self.load = []
        self.canceled = False

        for row in range(self.mainWidget.tblParameters.rowCount()):
            alg = self.alg.getCopy()
            col = 0
            for param in alg.parameters:
                if param.hidden:
                    continue
                if isinstance(param, ParameterExtent):
                    col += 1
                    continue
                widget = self.mainWidget.tblParameters.cellWidget(row, col)
                if not self.mainWidget.setParamValue(param, widget, alg):
                    self.lblProgress.setText(
                        self.tr('<b>Missing parameter value: %s (row %d)</b>') % (param.description, row + 1))
                    self.algs = None
                    return
                col += 1
            col = 0
            for param in alg.parameters:
                if param.hidden:
                    continue
                if isinstance(param, ParameterExtent):
                    widget = self.mainWidget.tblParameters.cellWidget(row, col)
                    if not self.mainWidget.setParamValue(param, widget, alg):
                        self.lblProgress.setText(
                            self.tr('<b>Missing parameter value: %s (row %d)</b>') % (param.description, row + 1))
                        self.algs = None
                        return
                col += 1
            for out in alg.outputs:
                if out.hidden:
                    continue

                widget = self.mainWidget.tblParameters.cellWidget(row, col)
                text = widget.getValue()
                if text.strip() != '':
                    out.value = text
                    col += 1
                else:
                    self.lblProgress.setText(
                        self.tr('<b>Wrong or missing parameter value: %s (row %d)</b>') % (out.description, row + 1))
                    self.algs = None
                    return

            self.algs.append(alg)
            if self.alg.getVisibleOutputsCount():
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

        for count, alg in enumerate(self.algs):
            self.setText(self.tr('\nProcessing algorithm %d/%d...') % (count + 1, len(self.algs)))
            self.setInfo(self.tr('<b>Algorithm %s starting...</b>' % alg.name))
            if runalg(alg, self) and not self.canceled:
                if self.load[count]:
                    handleAlgorithmResults(alg, self, False)
                self.setInfo(self.tr('Algorithm %s correctly executed...') % alg.name)
            else:
                QApplication.restoreOverrideCursor()
                return

        self.finish()

    def finish(self):
        for count, alg in enumerate(self.algs):
            self.loadHTMLResults(alg, count)

        self.createSummaryTable()
        QApplication.restoreOverrideCursor()

        self.mainWidget.setEnabled(True)
        QMessageBox.information(self, self.tr('Batch processing'),
                                self.tr('Batch processing completed'))

    def loadHTMLResults(self, alg, num):
        for out in alg.outputs:
            if out.hidden or not out.open:
                continue

            if isinstance(out, OutputHTML):
                ProcessingResults.addResult(
                    '{} [{}]'.format(out.description, num), out.value)

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
                        f.write('<p>{}: {}</p>\n'.format(out.description, out.value))
            f.write('<hr>\n')

        ProcessingResults.addResult(
            '{} [summary]'.format(self.algs[0].name), outputFile)
