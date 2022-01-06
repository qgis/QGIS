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

from pprint import pformat
import time

from qgis.PyQt.QtWidgets import QPushButton, QDialogButtonBox
from qgis.PyQt.QtCore import Qt, QCoreApplication

from qgis.core import (QgsProcessingOutputHtml,
                       QgsProcessingOutputNumber,
                       QgsProcessingOutputString,
                       QgsProcessingOutputBoolean,
                       QgsProject,
                       QgsProcessingMultiStepFeedback,
                       QgsScopedProxyProgressTask,
                       QgsProcessingException)

from qgis.gui import QgsProcessingAlgorithmDialogBase
from qgis.utils import OverrideCursor, iface

from processing.gui.BatchPanel import BatchPanel
from processing.gui.AlgorithmExecutor import execute
from processing.gui.Postprocessing import handleAlgorithmResults

from processing.core.ProcessingResults import resultsList

from processing.tools.system import getTempFilename
from processing.tools import dataobjects

import codecs


class BatchFeedback(QgsProcessingMultiStepFeedback):

    def __init__(self, steps, feedback):
        super().__init__(steps, feedback)
        self.errors = []

    def reportError(self, error: str, fatalError: bool = False):
        self.errors.append(error)
        super().reportError(error, fatalError)


class BatchAlgorithmDialog(QgsProcessingAlgorithmDialogBase):

    def __init__(self, alg, parent=None):
        super().__init__(parent, mode=QgsProcessingAlgorithmDialogBase.DialogMode.Batch)

        self.setAlgorithm(alg)

        self.setWindowTitle(self.tr('Batch Processing - {0}').format(self.algorithm().displayName()))
        self.setMainWidget(BatchPanel(self, self.algorithm()))
        self.hideShortHelp()

        self.btnRunSingle = QPushButton(QCoreApplication.translate('BatchAlgorithmDialog', "Run as Single Process…"))
        self.btnRunSingle.clicked.connect(self.runAsSingle)
        self.buttonBox().addButton(self.btnRunSingle, QDialogButtonBox.ResetRole)  # reset role to ensure left alignment

        self.context = None
        self.updateRunButtonVisibility()

    def runAsSingle(self):
        self.close()

        from processing.gui.AlgorithmDialog import AlgorithmDialog
        dlg = AlgorithmDialog(self.algorithm().create(), parent=iface.mainWindow())
        dlg.show()
        dlg.exec_()

    def resetAdditionalGui(self):
        self.btnRunSingle.setEnabled(True)

    def blockAdditionalControlsWhileRunning(self):
        self.btnRunSingle.setEnabled(False)

    def processingContext(self):
        if self.context is None:
            self.feedback = self.createFeedback()
            self.context = dataobjects.createContext(self.feedback)
            self.context.setLogLevel(self.logLevel())
        return self.context

    def runAlgorithm(self):
        alg_parameters = []

        feedback = self.createFeedback()

        load_layers = self.mainWidget().checkLoadLayersOnCompletion.isChecked()
        project = QgsProject.instance() if load_layers else None

        for row in range(self.mainWidget().batchRowCount()):
            parameters, ok = self.mainWidget().parametersForRow(row, destinationProject=project, warnOnInvalid=True)
            if ok:
                alg_parameters.append(parameters)
        if not alg_parameters:
            return

        task = QgsScopedProxyProgressTask(self.tr('Batch Processing - {0}').format(self.algorithm().displayName()))
        multi_feedback = BatchFeedback(len(alg_parameters), feedback)
        feedback.progressChanged.connect(task.setProgress)

        algorithm_results = []
        errors = []

        with OverrideCursor(Qt.WaitCursor):

            self.blockControlsWhileRunning()
            self.setExecutedAnyResult(True)
            self.cancelButton().setEnabled(True)

            # Make sure the Log tab is visible before executing the algorithm
            try:
                self.showLog()
                self.repaint()
            except Exception:  # FIXME which one?
                pass

            start_time = time.time()

            for count, parameters in enumerate(alg_parameters):
                if feedback.isCanceled():
                    break
                self.setProgressText(
                    QCoreApplication.translate('BatchAlgorithmDialog', '\nProcessing algorithm {0}/{1}…').format(
                        count + 1, len(alg_parameters)))
                self.setInfo(self.tr('<b>Algorithm {0} starting&hellip;</b>').format(self.algorithm().displayName()),
                             escapeHtml=False)
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
                multi_feedback.errors = []
                results, ok = self.algorithm().run(parameters, context, multi_feedback)
                if ok:
                    self.setInfo(
                        QCoreApplication.translate('BatchAlgorithmDialog', 'Algorithm {0} correctly executed…').format(
                            self.algorithm().displayName()), escapeHtml=False)
                    feedback.pushInfo(
                        self.tr('Execution completed in {0:0.2f} seconds'.format(time.time() - alg_start_time)))
                    feedback.pushInfo(self.tr('Results:'))
                    feedback.pushCommandInfo(pformat(results))
                    feedback.pushInfo('')
                    algorithm_results.append({'parameters': parameters, 'results': results})

                    handleAlgorithmResults(self.algorithm(), context, multi_feedback, False, parameters)
                else:
                    err = [e for e in multi_feedback.errors]
                    self.setInfo(
                        QCoreApplication.translate('BatchAlgorithmDialog', 'Algorithm {0} failed…').format(
                            self.algorithm().displayName()), escapeHtml=False)
                    feedback.reportError(
                        self.tr('Execution failed after {0:0.2f} seconds'.format(time.time() - alg_start_time)),
                        fatalError=False)
                    errors.append({'parameters': parameters, 'errors': err})

        feedback.pushInfo(self.tr('Batch execution completed in {0:0.2f} seconds'.format(time.time() - start_time)))
        if errors:
            feedback.reportError(self.tr('{} executions failed. See log for further details.').format(len(errors)), fatalError=True)
        task = None

        self.finish(algorithm_results, errors)
        self.cancelButton().setEnabled(False)

    def finish(self, algorithm_results, errors):
        for count, results in enumerate(algorithm_results):
            self.loadHTMLResults(results['results'], count)

        self.createSummaryTable(algorithm_results, errors)
        self.resetGui()

    def loadHTMLResults(self, results, num):
        for out in self.algorithm().outputDefinitions():
            if isinstance(out, QgsProcessingOutputHtml) and out.name() in results and results[out.name()]:
                resultsList.addResult(icon=self.algorithm().icon(), name='{} [{}]'.format(out.description(), num),
                                      result=results[out.name()])

    def createSummaryTable(self, algorithm_results, errors):
        createTable = False

        for out in self.algorithm().outputDefinitions():
            if isinstance(out, (QgsProcessingOutputNumber, QgsProcessingOutputString, QgsProcessingOutputBoolean)):
                createTable = True
                break

        if not createTable and not errors:
            return

        outputFile = getTempFilename('html')
        with codecs.open(outputFile, 'w', encoding='utf-8') as f:
            if createTable:
                for i, res in enumerate(algorithm_results):
                    results = res['results']
                    params = res['parameters']
                    if i > 0:
                        f.write('<hr>\n')
                    f.write(self.tr('<h3>Parameters</h3>\n'))
                    f.write('<table>\n')
                    for param in self.algorithm().parameterDefinitions():
                        if not param.isDestination():
                            if param.name() in params:
                                f.write('<tr><th>{}</th><td>{}</td></tr>\n'.format(param.description(),
                                                                                   params[param.name()]))
                    f.write('</table>\n')
                    f.write(self.tr('<h3>Results</h3>\n'))
                    f.write('<table>\n')
                    for out in self.algorithm().outputDefinitions():
                        if out.name() in results:
                            f.write('<tr><th>{}</th><td>{}</td></tr>\n'.format(out.description(), results[out.name()]))
                    f.write('</table>\n')
            if errors:
                f.write('<h2 style="color: red">{}</h2>\n'.format(self.tr('Errors')))
            for i, res in enumerate(errors):
                errors = res['errors']
                params = res['parameters']
                if i > 0:
                    f.write('<hr>\n')
                f.write(self.tr('<h3>Parameters</h3>\n'))
                f.write('<table>\n')
                for param in self.algorithm().parameterDefinitions():
                    if not param.isDestination():
                        if param.name() in params:
                            f.write(
                                '<tr><th>{}</th><td>{}</td></tr>\n'.format(param.description(), params[param.name()]))
                f.write('</table>\n')
                f.write('<h3>{}</h3>\n'.format(self.tr('Error')))
                f.write('<p style="color: red">{}</p>\n'.format('<br>'.join(errors)))

        resultsList.addResult(icon=self.algorithm().icon(),
                              name='{} [summary]'.format(self.algorithm().name()), timestamp=time.localtime(),
                              result=outputFile)
