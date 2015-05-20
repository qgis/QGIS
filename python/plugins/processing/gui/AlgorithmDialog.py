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

from PyQt4.QtCore import Qt
from PyQt4.QtGui import QMessageBox, QApplication, QCursor, QColor, QPalette

from processing.core.ProcessingLog import ProcessingLog
from processing.core.ProcessingConfig import ProcessingConfig

from processing.gui.ParametersPanel import ParametersPanel
from processing.gui.AlgorithmDialogBase import AlgorithmDialogBase
from processing.gui.AlgorithmExecutor import runalg, runalgIterating
from processing.gui.Postprocessing import handleAlgorithmResults

from processing.core.parameters import ParameterExtent
from processing.core.parameters import ParameterRaster
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterTable
from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterFixedTable
from processing.core.parameters import ParameterRange
from processing.core.parameters import ParameterTableField
from processing.core.parameters import ParameterMultipleInput
from processing.core.parameters import ParameterString
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterFile
from processing.core.parameters import ParameterCrs
from processing.core.parameters import ParameterGeometryPredicate

from processing.core.outputs import OutputRaster
from processing.core.outputs import OutputVector
from processing.core.outputs import OutputTable

from processing.tools import dataobjects


class AlgorithmDialog(AlgorithmDialogBase):

    def __init__(self, alg):
        AlgorithmDialogBase.__init__(self, alg)

        self.alg = alg

        self.mainWidget = ParametersPanel(self, alg)
        self.setMainWidget()

    def setParamValues(self):
        params = self.alg.parameters
        outputs = self.alg.outputs

        for param in params:
            if param.hidden:
                continue
            if isinstance(param, ParameterExtent):
                continue
            if not self.setParamValue(
                    param, self.mainWidget.valueItems[param.name]):
                raise AlgorithmDialogBase.InvalidParameterValue(param,
                        self.mainWidget.valueItems[param.name])

        for param in params:
            if isinstance(param, ParameterExtent):
                if not self.setParamValue(
                        param, self.mainWidget.valueItems[param.name]):
                    raise AlgorithmDialogBase.InvalidParameterValue(
                        param, self.mainWidget.valueItems[param.name])

        for output in outputs:
            if output.hidden:
                continue
            output.value = self.mainWidget.valueItems[output.name].getValue()
            if isinstance(output, (OutputRaster, OutputVector, OutputTable)):
                output.open = self.mainWidget.checkBoxes[output.name].isChecked()

        return True

    def setParamValue(self, param, widget, alg=None):
        if isinstance(param, ParameterRaster):
            return param.setValue(widget.getValue())
        elif isinstance(param, (ParameterVector, ParameterTable)):
            try:
                return param.setValue(widget.itemData(widget.currentIndex()))
            except:
                return param.setValue(widget.getValue())
        elif isinstance(param, ParameterBoolean):
            return param.setValue(widget.isChecked())
        elif isinstance(param, ParameterSelection):
            return param.setValue(widget.currentIndex())
        elif isinstance(param, ParameterFixedTable):
            return param.setValue(widget.table)
        elif isinstance(param, ParameterRange):
            return param.setValue(widget.getValue())
        if isinstance(param, ParameterTableField):
            if param.optional and widget.currentIndex() == 0:
                return param.setValue(None)
            return param.setValue(widget.currentText())
        elif isinstance(param, ParameterMultipleInput):
            if param.datatype == ParameterMultipleInput.TYPE_FILE:
                return param.setValue(widget.selectedoptions)
            else:
                if param.datatype == ParameterMultipleInput.TYPE_VECTOR_ANY:
                    options = dataobjects.getVectorLayers(sorting=False)
                else:
                    options = dataobjects.getRasterLayers(sorting=False)
                return param.setValue([options[i] for i in widget.selectedoptions])
        elif isinstance(param, (ParameterNumber, ParameterFile, ParameterCrs,
                        ParameterExtent)):
            return param.setValue(widget.getValue())
        elif isinstance(param, ParameterString):
            if param.multiline:
                return param.setValue(unicode(widget.toPlainText()))
            else:
                return param.setValue(unicode(widget.text()))
        elif isinstance(param, ParameterGeometryPredicate):
            return param.setValue(widget.value())
        else:
            return param.setValue(unicode(widget.text()))

    def accept(self):
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
            msg = self.alg.checkParameterValuesBeforeExecuting()
            if msg:
                QMessageBox.warning(
                    self, self.tr('Unable to execute algorithm'), msg)
                return
            self.btnRun.setEnabled(False)
            self.btnClose.setEnabled(False)
            buttons = self.mainWidget.iterateButtons
            self.iterateParam = None

            for i in range(len(buttons.values())):
                button = buttons.values()[i]
                if button.isChecked():
                    self.iterateParam = buttons.keys()[i]
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
        except AlgorithmDialogBase.InvalidParameterValue, e:
            try:
                self.buttonBox.accepted.connect(lambda :
                        e.widget.setPalette(QPalette()))
                palette = e.widget.palette()
                palette.setColor(QPalette.Base, QColor(255, 255, 0))
                e.widget.setPalette(palette)
                self.lblProgress.setText(
                    self.tr('<b>Missing parameter value: %s</b>') % e.parameter.description)
                return
            except:
                QMessageBox.critical(self,
                    self.tr('Unable to execute algorithm'),
                    self.tr('Wrong or missing parameter values'))

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
