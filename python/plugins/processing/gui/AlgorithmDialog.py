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
from builtins import str
from builtins import range

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtWidgets import QMessageBox, QApplication, QPushButton, QWidget, QVBoxLayout, QSizePolicy
from qgis.PyQt.QtGui import QCursor, QColor, QPalette

from qgis.core import QgsMapLayerRegistry
from qgis.gui import QgsMessageBar
from qgis.utils import iface

from processing.core.ProcessingLog import ProcessingLog
from processing.core.ProcessingConfig import ProcessingConfig

from processing.gui.BatchAlgorithmDialog import BatchAlgorithmDialog
from processing.gui.ParametersPanel import ParametersPanel
from processing.gui.AlgorithmDialogBase import AlgorithmDialogBase
from processing.gui.AlgorithmExecutor import runalg, runalgIterating
from processing.gui.Postprocessing import handleAlgorithmResults

from processing.core.parameters import ParameterRaster
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterExtent
from processing.core.parameters import ParameterMultipleInput

from processing.core.outputs import OutputRaster
from processing.core.outputs import OutputVector
from processing.core.outputs import OutputTable

from processing.tools import dataobjects


class AlgorithmDialog(AlgorithmDialogBase):

    def __init__(self, alg):
        AlgorithmDialogBase.__init__(self, alg)

        self.alg = alg

        self.setMainWidget(ParametersPanel(self, alg))

        self.bar = QgsMessageBar()
        self.bar.setSizePolicy(QSizePolicy.Minimum, QSizePolicy.Fixed)
        self.layout().insertWidget(0, self.bar)

        self.cornerWidget = QWidget()
        layout = QVBoxLayout()
        layout.setContentsMargins(0, 0, 0, 5)
        self.tabWidget.setStyleSheet("QTabBar::tab { height: 30px; }")
        self.runAsBatchButton = QPushButton(self.tr("Run as batch process..."))
        self.runAsBatchButton.clicked.connect(self.runAsBatch)
        layout.addWidget(self.runAsBatchButton)
        self.cornerWidget.setLayout(layout)
        self.tabWidget.setCornerWidget(self.cornerWidget)

    def runAsBatch(self):
        self.close()
        dlg = BatchAlgorithmDialog(self.alg)
        dlg.show()
        dlg.exec_()

    def setParamValues(self):
        params = self.alg.parameters
        outputs = self.alg.outputs

        for param in params:
            if param.hidden:
                continue
            wrapper = self.mainWidget.wrappers[param.name]
            if not self.setParamValue(param, wrapper):
                raise AlgorithmDialogBase.InvalidParameterValue(param, wrapper.widget)

        for output in outputs:
            if output.hidden:
                continue
            output.value = self.mainWidget.valueItems[output.name].getValue()
            if isinstance(output, (OutputRaster, OutputVector, OutputTable)):
                output.open = self.mainWidget.checkBoxes[output.name].isChecked()

        return True

    def setParamValue(self, param, wrapper, alg=None):
        return param.setValue(wrapper.value())

    def checkExtentCRS(self):
        unmatchingCRS = False
        hasExtent = False
        projectCRS = iface.mapCanvas().mapSettings().destinationCrs()
        layers = dataobjects.getAllLayers()
        for param in self.alg.parameters:
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

                        p = dataobjects.getObjectFromUri(inputlayer)
                        if p is not None:
                            if p.crs() != projectCRS:
                                unmatchingCRS = True
            if isinstance(param, ParameterExtent):
                value = self.mainWidget.valueItems[param.name].leText.text().strip()
                if value:
                    hasExtent = True

        return hasExtent and unmatchingCRS

    def accept(self):
        self.settings.setValue("/Processing/dialogBase", self.saveGeometry())

        checkCRS = ProcessingConfig.getSetting(ProcessingConfig.WARN_UNMATCHING_CRS)
        try:
            self.setParamValues()
            if checkCRS and not self.alg.checkInputCRS():
                reply = QMessageBox.question(self, self.tr("Unmatching CRS's"),
                                             self.tr('Layers do not all use the same CRS. This can '
                                                     'cause unexpected results.\nDo you want to '
                                                     'continue?'),
                                             QMessageBox.Yes | QMessageBox.No,
                                             QMessageBox.No)
                if reply == QMessageBox.No:
                    return
            checkExtentCRS = ProcessingConfig.getSetting(ProcessingConfig.WARN_UNMATCHING_EXTENT_CRS)
            if checkExtentCRS and self.checkExtentCRS():
                reply = QMessageBox.question(self, self.tr("Extent CRS"),
                                             self.tr('Extent parameters must use the same CRS as the input layers.\n'
                                                     'Your input layers do not have the same extent as the project, '
                                                     'so the extent might be in a wrong CRS if you have selected it from the canvas.\n'
                                                     'Do you want to continue?'),
                                             QMessageBox.Yes | QMessageBox.No,
                                             QMessageBox.No)
                if reply == QMessageBox.No:
                    return
            msg = self.alg._checkParameterValuesBeforeExecuting()
            if msg:
                QMessageBox.warning(
                    self, self.tr('Unable to execute algorithm'), msg)
                return
            self.btnRun.setEnabled(False)
            self.btnClose.setEnabled(False)
            buttons = self.mainWidget.iterateButtons
            self.iterateParam = None

            for i in range(len(list(buttons.values()))):
                button = list(buttons.values())[i]
                if button.isChecked():
                    self.iterateParam = list(buttons.keys())[i]
                    break

            self.progressBar.setMaximum(0)
            self.lblProgress.setText(self.tr('Processing algorithm...'))
            # Make sure the Log tab is visible before executing the algorithm
            try:
                self.tabWidget.setCurrentIndex(1)
                self.repaint()
            except:
                pass

            QApplication.setOverrideCursor(QCursor(Qt.WaitCursor))

            self.setInfo(
                self.tr('<b>Algorithm %s starting...</b>') % self.alg.name)

            if self.iterateParam:
                if runalgIterating(self.alg, self.iterateParam, self):
                    self.finish()
                else:
                    QApplication.restoreOverrideCursor()
                    self.resetGUI()
            else:
                command = self.alg.getAsCommand()
                if command:
                    ProcessingLog.addToLog(
                        ProcessingLog.LOG_ALGORITHM, command)
                if runalg(self.alg, self):
                    self.finish()
                else:
                    QApplication.restoreOverrideCursor()
                    self.resetGUI()
        except AlgorithmDialogBase.InvalidParameterValue as e:
            try:
                self.buttonBox.accepted.connect(lambda:
                                                e.widget.setPalette(QPalette()))
                palette = e.widget.palette()
                palette.setColor(QPalette.Base, QColor(255, 255, 0))
                e.widget.setPalette(palette)
            except:
                pass
            self.bar.clearWidgets()
            self.bar.pushMessage("", "Wrong or missing parameter value: %s" % e.parameter.description,
                                 level=QgsMessageBar.WARNING, duration=5)

    def finish(self):
        keepOpen = ProcessingConfig.getSetting(ProcessingConfig.KEEP_DIALOG_OPEN)

        if self.iterateParam is None:
            if not handleAlgorithmResults(self.alg, self, not keepOpen):
                self.resetGUI()
                return

        self.executed = True
        self.setInfo('Algorithm %s finished' % self.alg.name)
        QApplication.restoreOverrideCursor()

        if not keepOpen:
            self.close()
        else:
            self.resetGUI()
            if self.alg.getHTMLOutputsCount() > 0:
                self.setInfo(
                    self.tr('HTML output has been generated by this algorithm.'
                            '\nOpen the results dialog to check it.'))

    def closeEvent(self, evt):
        QgsMapLayerRegistry.instance().layerWasAdded.disconnect(self.mainWidget.layerRegistryChanged)
        QgsMapLayerRegistry.instance().layersWillBeRemoved.disconnect(self.mainWidget.layerRegistryChanged)
        super(AlgorithmDialog, self).closeEvent(evt)
