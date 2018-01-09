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

from qgis.core import (QgsProject,
                       QgsApplication,
                       QgsProcessingUtils,
                       QgsProcessingParameterDefinition,
                       QgsProcessingAlgRunnerTask,
                       QgsProcessingOutputHtml,
                       QgsProcessingParameterVectorDestination,
                       QgsProcessingOutputLayerDefinition,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingParameterRasterDestination,
                       QgsProcessingAlgorithm)
from qgis.gui import (QgsMessageBar,
                      QgsProcessingAlgorithmDialogBase)
from qgis.utils import iface

from processing.core.ProcessingLog import ProcessingLog
from processing.core.ProcessingConfig import ProcessingConfig
from processing.core.ProcessingResults import resultsList
from processing.gui.ParametersPanel import ParametersPanel
from processing.gui.BatchAlgorithmDialog import BatchAlgorithmDialog
from processing.gui.AlgorithmDialogBase import AlgorithmDialogBase
from processing.gui.AlgorithmExecutor import executeIterating, execute
from processing.gui.Postprocessing import handleAlgorithmResults

from processing.tools import dataobjects


class AlgorithmDialog(QgsProcessingAlgorithmDialogBase):

    def __init__(self, alg):
        super().__init__()
        self.feedback_dialog = None

        self.setAlgorithm(alg)
        self.setMainWidget(self.getParametersPanel(alg, self))

        self.runAsBatchButton = QPushButton(QCoreApplication.translate("AlgorithmDialog", "Run as Batch Process…"))
        self.runAsBatchButton.clicked.connect(self.runAsBatch)
        self.buttonBox().addButton(self.runAsBatchButton, QDialogButtonBox.ResetRole) # reset role to ensure left alignment

    def getParametersPanel(self, alg, parent):
        return ParametersPanel(parent, alg)

    def runAsBatch(self):
        self.close()
        dlg = BatchAlgorithmDialog(self.algorithm())
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
                wrapper = self.mainWidget().wrappers[param.name()]
                value = None
                if wrapper.widget:
                    value = wrapper.value()
                    parameters[param.name()] = value

                    if not param.checkValueIsAcceptable(value):
                        raise AlgorithmDialogBase.InvalidParameterValue(param, wrapper.widget)
            else:
                dest_project = None
                if not param.flags() & QgsProcessingParameterDefinition.FlagHidden and \
                        isinstance(param, (QgsProcessingParameterRasterDestination, QgsProcessingParameterFeatureSink, QgsProcessingParameterVectorDestination)):
                    if self.mainWidget().checkBoxes[param.name()].isChecked():
                        dest_project = QgsProject.instance()

                value = self.mainWidget().outputWidgets[param.name()].getValue()
                if value and isinstance(value, QgsProcessingOutputLayerDefinition):
                    value.destinationProject = dest_project
                if value:
                    parameters[param.name()] = value

        return parameters

    def checkExtentCRS(self):
        unmatchingCRS = False
        hasExtent = False
        context = dataobjects.createContext()
        projectCRS = iface.mapCanvas().mapSettings().destinationCrs()
        layers = QgsProcessingUtils.compatibleLayers(QgsProject.instance())
        for param in self.algorithm().parameterDefinitions():
            if isinstance(param, (ParameterRaster, ParameterVector, ParameterMultipleInput)):
                if param.value:
                    if isinstance(param, ParameterMultipleInput):
                        inputlayers = param.value.split(';')
                    else:
                        inputlayers = [param.value]
                    for inputlayer in inputlayers:
                        for layer in layers:
                            if layer.source() == inputlayer:
                                if layer.crs() != projectCRS:
                                    unmatchingCRS = True

                        p = QgsProcessingUtils.mapLayerFromString(inputlayer, context)
                        if p is not None:
                            if p.crs() != projectCRS:
                                unmatchingCRS = True
            if isinstance(param, ParameterExtent):
                if param.skip_crs_check:
                    continue

                value = self.mainWidget().wrappers[param.name()].widget.leText.text().strip()
                if value:
                    hasExtent = True

        return hasExtent and unmatchingCRS

    def accept(self):
        feedback = self.createFeedback()
        context = dataobjects.createContext(feedback)

        checkCRS = ProcessingConfig.getSetting(ProcessingConfig.WARN_UNMATCHING_CRS)
        try:
            parameters = self.getParameterValues()

            if checkCRS and not self.algorithm().validateInputCrs(parameters, context):
                reply = QMessageBox.question(self, self.tr("Unmatching CRS's"),
                                             self.tr('Layers do not all use the same CRS. This can '
                                                     'cause unexpected results.\nDo you want to '
                                                     'continue?'),
                                             QMessageBox.Yes | QMessageBox.No,
                                             QMessageBox.No)
                if reply == QMessageBox.No:
                    return
            checkExtentCRS = ProcessingConfig.getSetting(ProcessingConfig.WARN_UNMATCHING_EXTENT_CRS)
            # TODO
            if False and checkExtentCRS and self.checkExtentCRS():
                reply = QMessageBox.question(self, self.tr("Extent CRS"),
                                             self.tr('Extent parameters must use the same CRS as the input layers.\n'
                                                     'Your input layers do not have the same extent as the project, '
                                                     'so the extent might be in a wrong CRS if you have selected it from the canvas.\n'
                                                     'Do you want to continue?'),
                                             QMessageBox.Yes | QMessageBox.No,
                                             QMessageBox.No)
                if reply == QMessageBox.No:
                    return
            ok, msg = self.algorithm().checkParameterValues(parameters, context)
            if msg:
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
            self.setProgressText(self.tr('Processing algorithm...'))

            self.setInfo(
                self.tr('<b>Algorithm \'{0}\' starting...</b>').format(self.algorithm().displayName()), escapeHtml=False)

            feedback.pushInfo(self.tr('Input parameters:'))
            display_params = []
            for k, v in parameters.items():
                display_params.append("'" + k + "' : " + self.algorithm().parameterDefinition(k).valueAsPythonString(v, context))
            feedback.pushCommandInfo('{ ' + ', '.join(display_params) + ' }')
            feedback.pushInfo('')
            start_time = time.time()

            if self.iterateParam:
                # Make sure the Log tab is visible before executing the algorithm
                try:
                    self.showLog()
                    self.repaint()
                except:
                    pass

                self.cancelButton().setEnabled(self.algorithm().flags() & QgsProcessingAlgorithm.FlagCanCancel)
                if executeIterating(self.algorithm(), parameters, self.iterateParam, context, feedback):
                    feedback.pushInfo(
                        self.tr('Execution completed in {0:0.2f} seconds'.format(time.time() - start_time)))
                    self.cancelButton().setEnabled(False)
                    self.finish(True, parameters, context, feedback)
                else:
                    self.cancelButton().setEnabled(False)
                    self.resetGui()
            else:
                command = self.algorithm().asPythonCommand(parameters, context)
                if command:
                    ProcessingLog.addToLog(command)
                self.cancelButton().setEnabled(self.algorithm().flags() & QgsProcessingAlgorithm.FlagCanCancel)

                def on_complete(ok, results):
                    if ok:
                        feedback.pushInfo(self.tr('Execution completed in {0:0.2f} seconds'.format(time.time() - start_time)))
                        feedback.pushInfo(self.tr('Results:'))
                        feedback.pushCommandInfo(pformat(results))
                    else:
                        feedback.reportError(
                            self.tr('Execution failed after {0:0.2f} seconds'.format(time.time() - start_time)))
                    feedback.pushInfo('')

                    if self.feedback_dialog is not None:
                        self.feedback_dialog.close()
                        self.feedback_dialog.deleteLater()
                        self.feedback_dialog = None

                    self.cancelButton().setEnabled(False)

                    self.finish(ok, results, context, feedback)

                if self.algorithm().flags() & QgsProcessingAlgorithm.FlagCanRunInBackground:
                    # Make sure the Log tab is visible before executing the algorithm
                    self.showLog()

                    task = QgsProcessingAlgRunnerTask(self.algorithm(), parameters, context, feedback)
                    task.executed.connect(on_complete)
                    QgsApplication.taskManager().addTask(task)
                else:
                    self.feedback_dialog = self.createProgressDialog()
                    self.feedback_dialog.show()
                    ok, results = execute(self.algorithm(), parameters, context, feedback)
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
                                          level=QgsMessageBar.WARNING, duration=5)

    def finish(self, successful, result, context, feedback):
        keepOpen = not successful or ProcessingConfig.getSetting(ProcessingConfig.KEEP_DIALOG_OPEN)

        if self.iterateParam is None:

            # add html results to results dock
            for out in self.algorithm().outputDefinitions():
                if isinstance(out, QgsProcessingOutputHtml) and out.name() in result and result[out.name()]:
                    resultsList.addResult(icon=self.algorithm().icon(), name=out.description(),
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
