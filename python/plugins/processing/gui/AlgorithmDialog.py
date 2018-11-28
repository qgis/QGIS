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

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from pprint import pformat
import time

from qgis.PyQt.QtCore import QCoreApplication, Qt
from qgis.PyQt.QtWidgets import QMessageBox, QPushButton, QSizePolicy, QDialogButtonBox
from qgis.PyQt.QtGui import QColor, QPalette

from qgis.core import (Qgis,
                       QgsProject,
                       QgsApplication,
                       QgsProcessingUtils,
                       QgsProcessingParameterDefinition,
                       QgsProcessingAlgRunnerTask,
                       QgsProcessingOutputHtml,
                       QgsProcessingParameterVectorDestination,
                       QgsProcessingOutputLayerDefinition,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingParameterRasterDestination,
                       QgsProcessingAlgorithm,
                       QgsProxyProgressTask,
                       QgsTaskManager)
from qgis.gui import (QgsGui,
                      QgsMessageBar,
                      QgsProcessingAlgorithmDialogBase)
from qgis.utils import iface

from processing.core.ProcessingLog import ProcessingLog
from processing.core.ProcessingConfig import ProcessingConfig
from processing.core.ProcessingResults import resultsList
from processing.gui.ParametersPanel import ParametersPanel
from processing.gui.BatchAlgorithmDialog import BatchAlgorithmDialog
from processing.gui.AlgorithmDialogBase import AlgorithmDialogBase
from processing.gui.AlgorithmExecutor import executeIterating, execute, execute_in_place
from processing.gui.Postprocessing import handleAlgorithmResults
from processing.gui.wrappers import WidgetWrapper

from processing.tools import dataobjects


class AlgorithmDialog(QgsProcessingAlgorithmDialogBase):

    def __init__(self, alg, in_place=False, parent=None):
        super().__init__(parent)

        self.feedback_dialog = None
        self.in_place = in_place
        self.active_layer = None

        self.context = None
        self.feedback = None

        self.setAlgorithm(alg)
        self.setMainWidget(self.getParametersPanel(alg, self))

        if not self.in_place:
            self.runAsBatchButton = QPushButton(QCoreApplication.translate("AlgorithmDialog", "Run as Batch Process…"))
            self.runAsBatchButton.clicked.connect(self.runAsBatch)
            self.buttonBox().addButton(self.runAsBatchButton, QDialogButtonBox.ResetRole) # reset role to ensure left alignment
        else:
            self.active_layer = iface.activeLayer()
            self.runAsBatchButton = None
            has_selection = self.active_layer and (self.active_layer.selectedFeatureCount() > 0)
            self.buttonBox().button(QDialogButtonBox.Ok).setText(QCoreApplication.translate("AlgorithmDialog", "Modify Selected Features")
                                                                 if has_selection else QCoreApplication.translate("AlgorithmDialog", "Modify All Features"))
            self.buttonBox().button(QDialogButtonBox.Close).setText(QCoreApplication.translate("AlgorithmDialog", "Cancel"))
            self.setWindowTitle(self.windowTitle() + ' | ' + self.active_layer.name())

    def getParametersPanel(self, alg, parent):
        return ParametersPanel(parent, alg, self.in_place)

    def runAsBatch(self):
        self.close()
        dlg = BatchAlgorithmDialog(self.algorithm().create(), parent=iface.mainWindow())
        dlg.show()
        dlg.exec_()

    def setParameters(self, parameters):
        self.mainWidget().setParameters(parameters)

    def getParameterValues(self):
        parameters = {}

        if self.mainWidget() is None:
            return parameters

        for param in self.algorithm().parameterDefinitions():
            if param.flags() & QgsProcessingParameterDefinition.FlagHidden:
                continue
            if not param.isDestination():

                if self.in_place and param.name() == 'INPUT':
                    parameters[param.name()] = self.active_layer
                    continue

                try:
                    wrapper = self.mainWidget().wrappers[param.name()]
                except KeyError:
                    continue

                # For compatibility with 3.x API, we need to check whether the wrapper is
                # the deprecated WidgetWrapper class. If not, it's the newer
                # QgsAbstractProcessingParameterWidgetWrapper class
                # TODO QGIS 4.0 - remove
                if issubclass(wrapper.__class__, WidgetWrapper):
                    widget = wrapper.widget
                else:
                    widget = wrapper.wrappedWidget()

                if widget is None:
                    continue

                value = wrapper.parameterValue()
                parameters[param.name()] = value

                if not param.checkValueIsAcceptable(value):
                    raise AlgorithmDialogBase.InvalidParameterValue(param, widget)
            else:
                if self.in_place and param.name() == 'OUTPUT':
                    parameters[param.name()] = 'memory:'
                    continue

                dest_project = None
                if not param.flags() & QgsProcessingParameterDefinition.FlagHidden and \
                        isinstance(param, (QgsProcessingParameterRasterDestination,
                                           QgsProcessingParameterFeatureSink,
                                           QgsProcessingParameterVectorDestination)):
                    if self.mainWidget().checkBoxes[param.name()].isChecked():
                        dest_project = QgsProject.instance()

                value = self.mainWidget().outputWidgets[param.name()].getValue()
                if value and isinstance(value, QgsProcessingOutputLayerDefinition):
                    value.destinationProject = dest_project
                if value:
                    parameters[param.name()] = value

        return self.algorithm().preprocessParameters(parameters)

    def runAlgorithm(self):
        self.feedback = self.createFeedback()
        self.context = dataobjects.createContext(self.feedback)

        checkCRS = ProcessingConfig.getSetting(ProcessingConfig.WARN_UNMATCHING_CRS)
        try:
            parameters = self.getParameterValues()

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
            self.runButton().setEnabled(False)
            self.cancelButton().setEnabled(False)
            buttons = self.mainWidget().iterateButtons
            self.iterateParam = None

            for i in range(len(list(buttons.values()))):
                button = list(buttons.values())[i]
                if button.isChecked():
                    self.iterateParam = list(buttons.keys())[i]
                    break

            self.clearProgress()
            self.setProgressText(QCoreApplication.translate('AlgorithmDialog', 'Processing algorithm…'))

            self.setInfo(
                QCoreApplication.translate('AlgorithmDialog', '<b>Algorithm \'{0}\' starting&hellip;</b>').format(self.algorithm().displayName()), escapeHtml=False)

            self.feedback.pushInfo(self.tr('Input parameters:'))
            display_params = []
            for k, v in parameters.items():
                display_params.append("'" + k + "' : " + self.algorithm().parameterDefinition(k).valueAsPythonString(v, self.context))
            self.feedback.pushCommandInfo('{ ' + ', '.join(display_params) + ' }')
            self.feedback.pushInfo('')
            start_time = time.time()

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
                        self.tr('Execution completed in {0:0.2f} seconds').format(time.time() - start_time))
                    self.cancelButton().setEnabled(False)
                    self.finish(True, parameters, self.context, self.feedback)
                else:
                    self.cancelButton().setEnabled(False)
                    self.resetGui()
            else:
                command = self.algorithm().asPythonCommand(parameters, self.context)
                if command:
                    ProcessingLog.addToLog(command)
                QgsGui.instance().processingRecentAlgorithmLog().push(self.algorithm().id())
                self.cancelButton().setEnabled(self.algorithm().flags() & QgsProcessingAlgorithm.FlagCanCancel)

                def on_complete(ok, results):
                    if ok:
                        self.feedback.pushInfo(self.tr('Execution completed in {0:0.2f} seconds').format(time.time() - start_time))
                        self.feedback.pushInfo(self.tr('Results:'))
                        self.feedback.pushCommandInfo(pformat(results))
                    else:
                        self.feedback.reportError(
                            self.tr('Execution failed after {0:0.2f} seconds').format(time.time() - start_time))
                    self.feedback.pushInfo('')

                    if self.feedback_dialog is not None:
                        self.feedback_dialog.close()
                        self.feedback_dialog.deleteLater()
                        self.feedback_dialog = None

                    self.cancelButton().setEnabled(False)

                    if not self.in_place:
                        self.finish(ok, results, self.context, self.feedback)
                    elif ok:
                        self.close()

                    self.feedback = None
                    self.context = None

                if not self.in_place and not (self.algorithm().flags() & QgsProcessingAlgorithm.FlagNoThreading):
                    # Make sure the Log tab is visible before executing the algorithm
                    self.showLog()

                    task = QgsProcessingAlgRunnerTask(self.algorithm(), parameters, self.context, self.feedback)
                    task.executed.connect(on_complete)
                    self.setCurrentTask(task)
                else:
                    self.proxy_progress = QgsProxyProgressTask(QCoreApplication.translate("AlgorithmDialog", "Executing “{}”").format(self.algorithm().displayName()))
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
            try:
                self.buttonBox().accepted.connect(lambda e=e:
                                                  e.widget.setPalette(QPalette()))
                palette = e.widget.palette()
                palette.setColor(QPalette.Base, QColor(255, 255, 0))
                e.widget.setPalette(palette)
            except:
                pass
            self.messageBar().clearWidgets()
            self.messageBar().pushMessage("", self.tr("Wrong or missing parameter value: {0}").format(e.parameter.description()),
                                          level=Qgis.Warning, duration=5)

    def finish(self, successful, result, context, feedback):
        keepOpen = not successful or ProcessingConfig.getSetting(ProcessingConfig.KEEP_DIALOG_OPEN)

        if self.iterateParam is None:

            # add html results to results dock
            for out in self.algorithm().outputDefinitions():
                if isinstance(out, QgsProcessingOutputHtml) and out.name() in result and result[out.name()]:
                    resultsList.addResult(icon=self.algorithm().icon(), name=out.description(), timestamp=time.localtime(),
                                          result=result[out.name()])

            if not handleAlgorithmResults(self.algorithm(), context, feedback, not keepOpen):
                self.resetGui()
                return

        self.setExecuted(True)
        self.setResults(result)
        self.setInfo(self.tr('Algorithm \'{0}\' finished').format(self.algorithm().displayName()), escapeHtml=False)

        if not keepOpen:
            self.close()
        else:
            self.resetGui()
            if self.algorithm().hasHtmlOutputs():
                self.setInfo(
                    self.tr('HTML output has been generated by this algorithm.'
                            '\nOpen the results dialog to check it.'), escapeHtml=False)
