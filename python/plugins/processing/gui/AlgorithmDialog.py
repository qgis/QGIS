# -*- coding: utf-8 -*-

"""
***************************************************************************
    AlgorithmDialog.py
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
import datetime
import time

from qgis.PyQt.QtCore import QCoreApplication
from qgis.PyQt.QtWidgets import QMessageBox, QPushButton, QDialogButtonBox
from qgis.PyQt.QtGui import QColor, QPalette

from qgis.core import (Qgis,
                       QgsApplication,
                       QgsProcessingAlgRunnerTask,
                       QgsProcessingOutputHtml,
                       QgsProcessingAlgorithm,
                       QgsProxyProgressTask,
                       QgsProcessingFeatureSourceDefinition)
from qgis.gui import (QgsGui,
                      QgsProcessingAlgorithmDialogBase,
                      QgsProcessingParametersGenerator,
                      QgsProcessingContextGenerator)
from qgis.utils import iface

from processing.core.ProcessingConfig import ProcessingConfig
from processing.core.ProcessingResults import resultsList
from processing.gui.ParametersPanel import ParametersPanel
from processing.gui.BatchAlgorithmDialog import BatchAlgorithmDialog
from processing.gui.AlgorithmDialogBase import AlgorithmDialogBase
from processing.gui.AlgorithmExecutor import executeIterating, execute, execute_in_place
from processing.gui.Postprocessing import handleAlgorithmResults

from processing.tools import dataobjects


class AlgorithmDialog(QgsProcessingAlgorithmDialogBase):

    def __init__(self, alg, in_place=False, parent=None):
        super().__init__(parent)

        self.feedback_dialog = None
        self.in_place = in_place
        self.active_layer = iface.activeLayer() if self.in_place else None

        self.context = None
        self.feedback = None
        self.history_log_id = None
        self.history_details = {}

        self.setAlgorithm(alg)
        self.setMainWidget(self.getParametersPanel(alg, self))

        if not self.in_place:
            self.runAsBatchButton = QPushButton(QCoreApplication.translate("AlgorithmDialog", "Run as Batch Process…"))
            self.runAsBatchButton.clicked.connect(self.runAsBatch)
            self.buttonBox().addButton(self.runAsBatchButton,
                                       QDialogButtonBox.ResetRole)  # reset role to ensure left alignment
        else:
            in_place_input_parameter_name = 'INPUT'
            if hasattr(alg, 'inputParameterName'):
                in_place_input_parameter_name = alg.inputParameterName()

            self.mainWidget().setParameters({in_place_input_parameter_name: self.active_layer})

            self.runAsBatchButton = None
            has_selection = self.active_layer and (self.active_layer.selectedFeatureCount() > 0)
            self.buttonBox().button(QDialogButtonBox.Ok).setText(
                QCoreApplication.translate("AlgorithmDialog", "Modify Selected Features")
                if has_selection else QCoreApplication.translate("AlgorithmDialog", "Modify All Features"))
            self.setWindowTitle(self.windowTitle() + ' | ' + self.active_layer.name())

        self.updateRunButtonVisibility()

    def getParametersPanel(self, alg, parent):
        panel = ParametersPanel(parent, alg, self.in_place, self.active_layer)
        return panel

    def runAsBatch(self):
        self.close()
        dlg = BatchAlgorithmDialog(self.algorithm().create(), parent=iface.mainWindow())
        dlg.show()
        dlg.exec_()

    def resetAdditionalGui(self):
        if not self.in_place:
            self.runAsBatchButton.setEnabled(True)

    def blockAdditionalControlsWhileRunning(self):
        if not self.in_place:
            self.runAsBatchButton.setEnabled(False)

    def setParameters(self, parameters):
        self.mainWidget().setParameters(parameters)

    def flag_invalid_parameter_value(self, message: str, widget):
        """
        Highlights a parameter with an invalid value
        """
        try:
            self.buttonBox().accepted.connect(lambda w=widget:
                                              w.setPalette(QPalette()))
            palette = widget.palette()
            palette.setColor(QPalette.Base, QColor(255, 255, 0))
            widget.setPalette(palette)
        except:
            pass
        self.messageBar().clearWidgets()
        self.messageBar().pushMessage("", self.tr("Wrong or missing parameter value: {0}").format(
            message),
            level=Qgis.Warning, duration=5)

    def flag_invalid_output_extension(self, message: str, widget):
        """
        Highlights a parameter with an invalid output extension
        """
        try:
            self.buttonBox().accepted.connect(lambda w=widget:
                                              w.setPalette(QPalette()))
            palette = widget.palette()
            palette.setColor(QPalette.Base, QColor(255, 255, 0))
            widget.setPalette(palette)
        except:
            pass
        self.messageBar().clearWidgets()
        self.messageBar().pushMessage("", message,
                                      level=Qgis.Warning, duration=5)

    def createProcessingParameters(self, flags=QgsProcessingParametersGenerator.Flags()):
        if self.mainWidget() is None:
            return {}

        try:
            return self.mainWidget().createProcessingParameters(flags)
        except AlgorithmDialogBase.InvalidParameterValue as e:
            self.flag_invalid_parameter_value(e.parameter.description(), e.widget)
        except AlgorithmDialogBase.InvalidOutputExtension as e:
            self.flag_invalid_output_extension(e.message, e.widget)
        return {}

    def processingContext(self):
        if self.context is None:
            self.feedback = self.createFeedback()
            self.context = dataobjects.createContext(self.feedback)
            self.context.setLogLevel(self.logLevel())
        return self.context

    def runAlgorithm(self):
        self.feedback = self.createFeedback()
        self.context = dataobjects.createContext(self.feedback)
        self.context.setLogLevel(self.logLevel())

        checkCRS = ProcessingConfig.getSetting(ProcessingConfig.WARN_UNMATCHING_CRS)
        try:
            # messy as all heck, but we don't want to call the dialog's implementation of
            # createProcessingParameters as we want to catch the exceptions raised by the
            # parameter panel instead...
            parameters = {} if self.mainWidget() is None else self.mainWidget().createProcessingParameters()

            if checkCRS and not self.algorithm().validateInputCrs(parameters, self.context):
                reply = QMessageBox.question(self, self.tr("Unmatching CRS's"),
                                             self.tr('Parameters do not all use the same CRS. This can '
                                                     'cause unexpected results.\nDo you want to '
                                                     'continue?'),
                                             QMessageBox.Yes | QMessageBox.No,
                                             QMessageBox.No)
                if reply == QMessageBox.No:
                    return
            ok, msg = self.algorithm().checkParameterValues(parameters, self.context)
            if not ok:
                QMessageBox.warning(
                    self, self.tr('Unable to execute algorithm'), msg)
                return

            self.blockControlsWhileRunning()
            self.setExecutedAnyResult(True)
            self.cancelButton().setEnabled(False)

            self.iterateParam = None

            for param in self.algorithm().parameterDefinitions():
                if isinstance(parameters.get(param.name(), None), QgsProcessingFeatureSourceDefinition) and parameters[
                        param.name()].flags & QgsProcessingFeatureSourceDefinition.FlagCreateIndividualOutputPerInputFeature:
                    self.iterateParam = param.name()
                    break

            self.clearProgress()
            self.feedback.pushVersionInfo(self.algorithm().provider())
            if self.algorithm().provider().warningMessage():
                self.feedback.reportError(self.algorithm().provider().warningMessage())

            self.feedback.pushInfo(
                QCoreApplication.translate('AlgorithmDialog', 'Algorithm started at: {}').format(
                    datetime.datetime.now().replace(microsecond=0).isoformat()
                )
            )

            self.setInfo(
                QCoreApplication.translate('AlgorithmDialog', '<b>Algorithm \'{0}\' starting&hellip;</b>').format(
                    self.algorithm().displayName()), escapeHtml=False)

            self.feedback.pushInfo(self.tr('Input parameters:'))
            display_params = []
            for k, v in parameters.items():
                display_params.append(
                    "'" + k + "' : " + self.algorithm().parameterDefinition(k).valueAsPythonString(v, self.context))
            self.feedback.pushCommandInfo('{ ' + ', '.join(display_params) + ' }')
            self.feedback.pushInfo('')
            start_time = time.time()

            def elapsed_time(start_time, result):
                delta_t = time.time() - start_time
                hours = int(delta_t / 3600)
                minutes = int((delta_t % 3600) / 60)
                seconds = delta_t - hours * 3600 - minutes * 60

                str_hours = [self.tr("hour"), self.tr("hours")][hours > 1]
                str_minutes = [self.tr("minute"), self.tr("minutes")][minutes > 1]
                str_seconds = [self.tr("second"), self.tr("seconds")][seconds != 1]

                if hours > 0:
                    elapsed = '{0} {1:0.2f} {2} ({3} {4} {5} {6} {7:0.0f} {2})'.format(
                        result, delta_t, str_seconds, hours, str_hours, minutes, str_minutes, seconds)
                elif minutes > 0:
                    elapsed = '{0} {1:0.2f} {2} ({3} {4} {5:0.0f} {2})'.format(
                        result, delta_t, str_seconds, minutes, str_minutes, seconds)
                else:
                    elapsed = '{0} {1:0.2f} {2}'.format(
                        result, delta_t, str_seconds)

                return(elapsed)

            if self.iterateParam:
                # Make sure the Log tab is visible before executing the algorithm
                try:
                    self.showLog()
                    self.repaint()
                except:
                    pass

                self.cancelButton().setEnabled(self.algorithm().flags() & QgsProcessingAlgorithm.FlagCanCancel)
                if executeIterating(self.algorithm(), parameters, self.iterateParam, self.context, self.feedback):
                    self.feedback.pushInfo(
                        self.tr(elapsed_time(start_time, 'Execution completed in')))
                    self.cancelButton().setEnabled(False)
                    self.finish(True, parameters, self.context, self.feedback)
                else:
                    self.cancelButton().setEnabled(False)
                    self.resetGui()
            else:
                self.history_details = {
                    'python_command': self.algorithm().asPythonCommand(parameters, self.context),
                    'algorithm_id': self.algorithm().id(),
                    'parameters': self.algorithm().asMap(parameters, self.context)
                }
                process_command, command_ok = self.algorithm().asQgisProcessCommand(parameters, self.context)
                if command_ok:
                    self.history_details['process_command'] = process_command
                self.history_log_id, _ = QgsGui.historyProviderRegistry().addEntry('processing', self.history_details)

                QgsGui.instance().processingRecentAlgorithmLog().push(self.algorithm().id())
                self.cancelButton().setEnabled(self.algorithm().flags() & QgsProcessingAlgorithm.FlagCanCancel)

                def on_complete(ok, results):
                    if ok:
                        self.feedback.pushInfo(
                            self.tr(elapsed_time(start_time, 'Execution completed in')))
                        self.feedback.pushInfo(self.tr('Results:'))
                        r = {k: v for k, v in results.items() if k not in ('CHILD_RESULTS', 'CHILD_INPUTS')}
                        self.feedback.pushCommandInfo(pformat(r))
                    else:
                        self.feedback.reportError(
                            self.tr(elapsed_time(start_time, 'Execution failed after')))
                    self.feedback.pushInfo('')

                    if self.history_log_id is not None:
                        # can't deepcopy this!
                        self.history_details['results'] = {k: v for k, v in results.items() if k != 'CHILD_INPUTS'}

                        QgsGui.historyProviderRegistry().updateEntry(self.history_log_id, self.history_details)

                    if self.feedback_dialog is not None:
                        self.feedback_dialog.close()
                        self.feedback_dialog.deleteLater()
                        self.feedback_dialog = None

                    self.cancelButton().setEnabled(False)

                    self.finish(ok, results, self.context, self.feedback, in_place=self.in_place)

                    self.feedback = None
                    self.context = None

                if not self.in_place and not (self.algorithm().flags() & QgsProcessingAlgorithm.FlagNoThreading):
                    # Make sure the Log tab is visible before executing the algorithm
                    self.showLog()

                    task = QgsProcessingAlgRunnerTask(self.algorithm(), parameters, self.context, self.feedback)
                    if task.isCanceled():
                        on_complete(False, {})
                    else:
                        task.executed.connect(on_complete)
                        self.setCurrentTask(task)
                else:
                    self.proxy_progress = QgsProxyProgressTask(
                        QCoreApplication.translate("AlgorithmDialog", "Executing “{}”").format(
                            self.algorithm().displayName()))
                    QgsApplication.taskManager().addTask(self.proxy_progress)
                    self.feedback.progressChanged.connect(self.proxy_progress.setProxyProgress)
                    self.feedback_dialog = self.createProgressDialog()
                    self.feedback_dialog.show()
                    if self.in_place:
                        ok, results = execute_in_place(self.algorithm(), parameters, self.context, self.feedback)
                    else:
                        ok, results = execute(self.algorithm(), parameters, self.context, self.feedback)
                    self.feedback.progressChanged.disconnect()
                    self.proxy_progress.finalize(ok)
                    on_complete(ok, results)

        except AlgorithmDialogBase.InvalidParameterValue as e:
            self.flag_invalid_parameter_value(e.parameter.description(), e.widget)
        except AlgorithmDialogBase.InvalidOutputExtension as e:
            self.flag_invalid_output_extension(e.message, e.widget)

    def finish(self, successful, result, context, feedback, in_place=False):
        keepOpen = not successful or ProcessingConfig.getSetting(ProcessingConfig.KEEP_DIALOG_OPEN)

        if not in_place and self.iterateParam is None:

            # add html results to results dock
            for out in self.algorithm().outputDefinitions():
                if isinstance(out, QgsProcessingOutputHtml) and out.name() in result and result[out.name()]:
                    resultsList.addResult(icon=self.algorithm().icon(), name=out.description(),
                                          timestamp=time.localtime(),
                                          result=result[out.name()])
            if not handleAlgorithmResults(self.algorithm(), context, feedback, not keepOpen, result):
                self.resetGui()
                return

        self.setExecuted(True)
        self.setResults(result)
        self.setInfo(self.tr('Algorithm \'{0}\' finished').format(self.algorithm().displayName()), escapeHtml=False)
        self.algorithmFinished.emit(successful, result)

        if not in_place and not keepOpen:
            self.close()
        else:
            self.resetGui()
            if self.algorithm().hasHtmlOutputs():
                self.setInfo(
                    self.tr('HTML output has been generated by this algorithm.'
                            '\nOpen the results dialog to check it.'), escapeHtml=False)
