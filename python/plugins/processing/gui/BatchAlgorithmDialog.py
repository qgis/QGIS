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

from pprint import pformat
import time

from qgis.PyQt.QtWidgets import QApplication, QMessageBox, QSizePolicy
from qgis.PyQt.QtGui import QCursor
from qgis.PyQt.QtCore import Qt

from qgis.core import (QgsProcessingParameterDefinition,
                       QgsProcessingParameterRasterDestination,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingOutputLayerDefinition,
                       QgsProcessingOutputHtml,
                       QgsProcessingOutputNumber,
                       QgsProcessingOutputString,
                       QgsProject)

from qgis.gui import QgsMessageBar
from qgis.utils import OverrideCursor

from processing.gui.BatchPanel import BatchPanel
from processing.gui.AlgorithmDialogBase import AlgorithmDialogBase
from processing.gui.AlgorithmExecutor import execute
from processing.gui.Postprocessing import handleAlgorithmResults

from processing.core.ProcessingResults import resultsList

from processing.tools.system import getTempFilename
from processing.tools import dataobjects

import codecs


class BatchAlgorithmDialog(AlgorithmDialogBase):

    def __init__(self, alg):
        AlgorithmDialogBase.__init__(self, alg)

        self.alg = alg

        self.setWindowTitle(self.tr('Batch Processing - {0}').format(self.alg.displayName()))

        self.setMainWidget(BatchPanel(self, self.alg))

        self.textShortHelp.setVisible(False)

        self.bar = QgsMessageBar()
        self.bar.setSizePolicy(QSizePolicy.Minimum, QSizePolicy.Fixed)
        self.layout().insertWidget(0, self.bar)

    def accept(self):
        alg_parameters = []
        load = []

        feedback = self.createFeedback()

        load_layers = self.mainWidget.checkLoadLayersOnCompletion.isChecked()
        project = QgsProject.instance() if load_layers else None

        for row in range(self.mainWidget.tblParameters.rowCount()):
            col = 0
            parameters = {}
            for param in self.alg.parameterDefinitions():
                if param.flags() & QgsProcessingParameterDefinition.FlagHidden or param.isDestination():
                    continue
                wrapper = self.mainWidget.wrappers[row][col]
                parameters[param.name()] = wrapper.value()
                if not param.checkValueIsAcceptable(wrapper.value()):
                    self.bar.pushMessage("", self.tr('Wrong or missing parameter value: {0} (row {1})').format(
                                         param.description(), row + 1),
                                         level=QgsMessageBar.WARNING, duration=5)
                    return
                col += 1
            count_visible_outputs = 0
            for out in self.alg.destinationParameterDefinitions():
                if out.flags() & QgsProcessingParameterDefinition.FlagHidden:
                    continue

                count_visible_outputs += 1
                widget = self.mainWidget.tblParameters.cellWidget(row, col)
                text = widget.getValue()
                if out.checkValueIsAcceptable(text):
                    if isinstance(out, (QgsProcessingParameterRasterDestination,
                                        QgsProcessingParameterFeatureSink)):
                        # load rasters and sinks on completion
                        parameters[out.name()] = QgsProcessingOutputLayerDefinition(text, project)
                    else:
                        parameters[out.name()] = text
                    col += 1
                else:
                    self.bar.pushMessage("", self.tr('Wrong or missing output value: {0} (row {1})').format(
                                         out.description(), row + 1),
                                         level=QgsMessageBar.WARNING, duration=5)
                    return

            alg_parameters.append(parameters)

        with OverrideCursor(Qt.WaitCursor):

            self.mainWidget.setEnabled(False)
            self.buttonCancel.setEnabled(True)

            # Make sure the Log tab is visible before executing the algorithm
            try:
                self.tabWidget.setCurrentIndex(1)
                self.repaint()
            except:
                pass

            start_time = time.time()

            algorithm_results = []
            for count, parameters in enumerate(alg_parameters):
                if feedback.isCanceled():
                    break
                self.setText(self.tr('\nProcessing algorithm {0}/{1}...').format(count + 1, len(alg_parameters)))
                self.setInfo(self.tr('<b>Algorithm {0} starting...</b>').format(self.alg.displayName()), escape_html=False)

                feedback.pushInfo(self.tr('Input parameters:'))
                feedback.pushCommandInfo(pformat(parameters))
                feedback.pushInfo('')

                # important - we create a new context for each iteration
                # this avoids holding onto resources and layers from earlier iterations,
                # and allows batch processing of many more items then is possible
                # if we hold on to these layers
                context = dataobjects.createContext(feedback)

                alg_start_time = time.time()
                ret, results = execute(self.alg, parameters, context, feedback)
                if ret:
                    self.setInfo(self.tr('Algorithm {0} correctly executed...').format(self.alg.displayName()), escape_html=False)
                    feedback.setProgress(100)
                    feedback.pushInfo(
                        self.tr('Execution completed in {0:0.2f} seconds'.format(time.time() - alg_start_time)))
                    feedback.pushInfo(self.tr('Results:'))
                    feedback.pushCommandInfo(pformat(results))
                    feedback.pushInfo('')
                    algorithm_results.append(results)
                else:
                    break

                handleAlgorithmResults(self.alg, context, feedback, False)

        feedback.pushInfo(self.tr('Batch execution completed in {0:0.2f} seconds'.format(time.time() - start_time)))

        self.finish(algorithm_results)
        self.buttonCancel.setEnabled(False)

    def finish(self, algorithm_results):
        for count, results in enumerate(algorithm_results):
            self.loadHTMLResults(results, count)

        self.createSummaryTable(algorithm_results)
        self.mainWidget.setEnabled(True)
        QMessageBox.information(self, self.tr('Batch processing'),
                                self.tr('Batch processing completed'))

    def loadHTMLResults(self, results, num):
        for out in self.alg.outputDefinitions():
            if isinstance(out, QgsProcessingOutputHtml) and out.name() in results and results[out.name()]:
                resultsList.addResult(icon=self.alg.icon(), name='{} [{}]'.format(out.description(), num),
                                      result=results[out.name()])

    def createSummaryTable(self, algorithm_results):
        createTable = False

        for out in self.alg.outputDefinitions():
            if isinstance(out, (QgsProcessingOutputNumber, QgsProcessingOutputString)):
                createTable = True
                break

        if not createTable:
            return

        outputFile = getTempFilename('html')
        with codecs.open(outputFile, 'w', encoding='utf-8') as f:
            for res in algorithm_results:
                f.write('<hr>\n')
                for out in self.alg.outputDefinitions():
                    if isinstance(out, (QgsProcessingOutputNumber, QgsProcessingOutputString)) and out.name() in res:
                        f.write('<p>{}: {}</p>\n'.format(out.description(), res[out.name()]))
            f.write('<hr>\n')

        resultsList.addResult(self.alg.icon(),
                              '{} [summary]'.format(self.alg.name()), outputFile)
