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

from pprint import pformat
import time

from qgis.PyQt.QtWidgets import QMessageBox
from qgis.PyQt.QtCore import Qt, QCoreApplication

from qgis.core import (QgsProcessingParameterDefinition,
                       QgsProcessingParameterRasterDestination,
                       QgsProcessingParameterVectorDestination,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingOutputLayerDefinition,
                       QgsProcessingOutputHtml,
                       QgsProcessingOutputNumber,
                       QgsProcessingOutputString,
                       QgsProject,
                       QgsProcessingMultiStepFeedback,
                       Qgis,
                       QgsScopedProxyProgressTask)

from qgis.gui import QgsProcessingAlgorithmDialogBase
from qgis.utils import OverrideCursor, iface

from processing.gui.BatchPanel import BatchPanel
from processing.gui.AlgorithmExecutor import execute
from processing.gui.Postprocessing import handleAlgorithmResults

from processing.core.ProcessingResults import resultsList

from processing.tools.system import getTempFilename
from processing.tools import dataobjects

import codecs


class BatchAlgorithmDialog(QgsProcessingAlgorithmDialogBase):

    def __init__(self, alg, parent=None):
        super().__init__(parent)

        self.setAlgorithm(alg)

        self.setWindowTitle(self.tr('Batch Processing - {0}').format(self.algorithm().displayName()))
        self.setMainWidget(BatchPanel(self, self.algorithm()))
        self.hideShortHelp()

    def runAlgorithm(self):
        alg_parameters = []

        feedback = self.createFeedback()

        load_layers = self.mainWidget().checkLoadLayersOnCompletion.isChecked()
        project = QgsProject.instance() if load_layers else None

        for row in range(self.mainWidget().tblParameters.rowCount()):
            col = 0
            parameters = {}
            for param in self.algorithm().parameterDefinitions():
                if param.flags() & QgsProcessingParameterDefinition.FlagHidden or param.isDestination():
                    continue
                wrapper = self.mainWidget().wrappers[row][col]
                parameters[param.name()] = wrapper.parameterValue()
                if not param.checkValueIsAcceptable(wrapper.parameterValue()):
                    self.messageBar().pushMessage("", self.tr('Wrong or missing parameter value: {0} (row {1})').format(
                        param.description(), row + 1),
                        level=Qgis.Warning, duration=5)
                    return
                col += 1
            count_visible_outputs = 0
            for out in self.algorithm().destinationParameterDefinitions():
                if out.flags() & QgsProcessingParameterDefinition.FlagHidden:
                    continue

                count_visible_outputs += 1
                widget = self.mainWidget().tblParameters.cellWidget(row, col)
                text = widget.getValue()
                if out.checkValueIsAcceptable(text):
                    if isinstance(out, (QgsProcessingParameterRasterDestination,
                                        QgsProcessingParameterVectorDestination,
                                        QgsProcessingParameterFeatureSink)):
                        # load rasters and sinks on completion
                        parameters[out.name()] = QgsProcessingOutputLayerDefinition(text, project)
                    else:
                        parameters[out.name()] = text
                    col += 1
                else:
                    self.messageBar().pushMessage("", self.tr('Wrong or missing output value: {0} (row {1})').format(
                        out.description(), row + 1),
                        level=Qgis.Warning, duration=5)
                    return

            alg_parameters.append(parameters)

        task = QgsScopedProxyProgressTask(self.tr('Batch Processing - {0}').format(self.algorithm().displayName()))
        multi_feedback = QgsProcessingMultiStepFeedback(len(alg_parameters), feedback)
        feedback.progressChanged.connect(task.setProgress)

        with OverrideCursor(Qt.WaitCursor):

            self.mainWidget().setEnabled(False)
            self.cancelButton().setEnabled(True)

            # Make sure the Log tab is visible before executing the algorithm
            try:
                self.showLog()
                self.repaint()
            except:
                pass

            start_time = time.time()

            algorithm_results = []
            for count, parameters in enumerate(alg_parameters):
                if feedback.isCanceled():
                    break
                self.setProgressText(QCoreApplication.translate('BatchAlgorithmDialog', '\nProcessing algorithm {0}/{1}…').format(count + 1, len(alg_parameters)))
                self.setInfo(self.tr('<b>Algorithm {0} starting&hellip;</b>').format(self.algorithm().displayName()), escapeHtml=False)
                multi_feedback.setCurrentStep(count)

                parameters = self.algorithm().preprocessParameters(parameters)

                feedback.pushInfo(self.tr('Input parameters:'))
                feedback.pushCommandInfo(pformat(parameters))
                feedback.pushInfo('')

                # important - we create a new context for each iteration
                # this avoids holding onto resources and layers from earlier iterations,
                # and allows batch processing of many more items then is possible
                # if we hold on to these layers
                context = dataobjects.createContext(feedback)

                alg_start_time = time.time()
                ret, results = execute(self.algorithm(), parameters, context, multi_feedback)
                if ret:
                    self.setInfo(QCoreApplication.translate('BatchAlgorithmDialog', 'Algorithm {0} correctly executed…').format(self.algorithm().displayName()), escapeHtml=False)
                    feedback.pushInfo(
                        self.tr('Execution completed in {0:0.2f} seconds'.format(time.time() - alg_start_time)))
                    feedback.pushInfo(self.tr('Results:'))
                    feedback.pushCommandInfo(pformat(results))
                    feedback.pushInfo('')
                    algorithm_results.append(results)
                else:
                    break

                handleAlgorithmResults(self.algorithm(), context, multi_feedback, False)

        feedback.pushInfo(self.tr('Batch execution completed in {0:0.2f} seconds'.format(time.time() - start_time)))
        task = None

        self.finish(algorithm_results)
        self.cancelButton().setEnabled(False)

    def finish(self, algorithm_results):
        for count, results in enumerate(algorithm_results):
            self.loadHTMLResults(results, count)

        self.createSummaryTable(algorithm_results)
        self.mainWidget().setEnabled(True)

    def loadHTMLResults(self, results, num):
        for out in self.algorithm().outputDefinitions():
            if isinstance(out, QgsProcessingOutputHtml) and out.name() in results and results[out.name()]:
                resultsList.addResult(icon=self.algorithm().icon(), name='{} [{}]'.format(out.description(), num),
                                      result=results[out.name()])

    def createSummaryTable(self, algorithm_results):
        createTable = False

        for out in self.algorithm().outputDefinitions():
            if isinstance(out, (QgsProcessingOutputNumber, QgsProcessingOutputString)):
                createTable = True
                break

        if not createTable:
            return

        outputFile = getTempFilename('html')
        with codecs.open(outputFile, 'w', encoding='utf-8') as f:
            for res in algorithm_results:
                f.write('<hr>\n')
                for out in self.algorithm().outputDefinitions():
                    if isinstance(out, (QgsProcessingOutputNumber, QgsProcessingOutputString)) and out.name() in res:
                        f.write('<p>{}: {}</p>\n'.format(out.description(), res[out.name()]))
            f.write('<hr>\n')

        resultsList.addResult(self.algorithm().icon(),
                              '{} [summary]'.format(self.algorithm().name()), outputFile)
