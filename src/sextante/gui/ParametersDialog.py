from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4 import QtCore, QtGui
from sextante.core.QGisLayers import QGisLayers
from sextante.parameters.ParameterRaster import ParameterRaster
from sextante.parameters.ParameterVector import ParameterVector
from sextante.parameters.ParameterBoolean import ParameterBoolean
from sextante.parameters.ParameterSelection import ParameterSelection
from sextante.parameters.ParameterMultipleInput import ParameterMultipleInput
from sextante.parameters.ParameterFixedTable import ParameterFixedTable
from sextante.parameters.ParameterTableField import ParameterTableField
from sextante.parameters.ParameterTable import ParameterTable
from sextante.gui.AlgorithmExecutor import AlgorithmExecutor
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from sextante.core.SextanteLog import SextanteLog
from sextante.gui.SextantePostprocessing import SextantePostprocessing
from sextante.parameters.ParameterRange import ParameterRange
from sextante.gui.HTMLViewerDialog import HTMLViewerDialog
from sextante.parameters.ParameterNumber import ParameterNumber

from sextante.gui.ParametersPanel import ParametersPanel
from sextante.parameters.ParameterFile import ParameterFile
from sextante.parameters.ParameterCrs import ParameterCrs
from sextante.core.SextanteConfig import SextanteConfig
from sextante.parameters.ParameterExtent import ParameterExtent

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class ParametersDialog(QtGui.QDialog):
    '''the default parameters dialog, to be used when an algorithm is called from the toolbox'''
    def __init__(self, alg):
        QtGui.QDialog.__init__(self, None, QtCore.Qt.WindowSystemMenuHint | QtCore.Qt.WindowTitleHint)
        self.ui = Ui_ParametersDialog()
        self.ui.setupUi(self, alg)
        self.executed = False

class Ui_ParametersDialog(object):

    NOT_SELECTED = "[Not selected]"

    def setupUi(self, dialog, alg):
        self.alg = alg
        self.dialog = dialog
        dialog.resize(650, 450)
        self.buttonBox = QtGui.QDialogButtonBox()
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Ok)
        if self.alg.helpFile():
            self.showHelpButton = QtGui.QPushButton()
            self.showHelpButton.setText("Show help")
            self.buttonBox.addButton(self.showHelpButton, QtGui.QDialogButtonBox.ActionRole)
            QtCore.QObject.connect(self.showHelpButton, QtCore.SIGNAL("clicked()"), self.showHelp)
        self.paramTable = ParametersPanel(self.alg, self.dialog)
        self.scrollArea = QtGui.QScrollArea()
        self.scrollArea.setWidget(self.paramTable)
        self.scrollArea.setWidgetResizable(True)
        dialog.setWindowTitle(self.alg.name)
        self.progressLabel = QtGui.QLabel()
        self.progress = QtGui.QProgressBar()
        self.progress.setMinimum(0)
        self.progress.setMaximum(100)
        self.progress.setValue(0)
        self.verticalLayout = QtGui.QVBoxLayout(dialog)
        self.verticalLayout.setSpacing(2)
        self.verticalLayout.setMargin(0)
        self.verticalLayout.addWidget(self.scrollArea)
        self.verticalLayout.addWidget(self.progressLabel)
        self.verticalLayout.addWidget(self.progress)
        self.verticalLayout.addWidget(self.buttonBox)
        dialog.setLayout(self.verticalLayout)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("accepted()")), self.accept)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("rejected()")), self.reject)
        QtCore.QMetaObject.connectSlotsByName(dialog)

    def showHelp(self):
        if self.alg.helpFile():
            dlg = HTMLViewerDialog(self.alg.helpFile())
            dlg.exec_()
        else:
            QMessageBox.warning(self.dialog, "No help available", "No help is available for the current algorithm.")


    def setParamValues(self):
        params = self.alg.parameters
        outputs = self.alg.outputs

        for param in params:
            if not self.setParamValue(param, self.paramTable.valueItems[param.name]):
                return False

        for output in outputs:
            if output.hidden:
                continue
            output.value = self.paramTable.valueItems[output.name].getValue()

        return True

    def setParamValue(self, param, widget):
        if isinstance(param, ParameterRaster):
            return param.setValue(widget.getValue())
        elif isinstance(param, (ParameterVector, ParameterTable)):
            try:
                return param.setValue(widget.itemData(widget.currentIndex()).toPyObject())
            except:
                return param.setValue(widget.getValue())
        elif isinstance(param, ParameterBoolean):
            return param.setValue(widget.currentIndex() == 0)
        elif isinstance(param, ParameterSelection):
            return param.setValue(widget.currentIndex())
        elif isinstance(param, ParameterFixedTable):
            return param.setValue(widget.table)
        elif isinstance(param, ParameterRange):
            return param.setValue(widget.getValue())
        if isinstance(param, ParameterTableField):
            return param.setValue(widget.currentText())
        elif isinstance(param, ParameterMultipleInput):
            if param.datatype == ParameterMultipleInput.TYPE_VECTOR_ANY:
                options = QGisLayers.getVectorLayers()
            else:
                options = QGisLayers.getRasterLayers()
            value = []
            for index in widget.selectedoptions:
                value.append(options[index])
            return param.setValue(value)
        elif isinstance(param, (ParameterNumber, ParameterFile, ParameterCrs, ParameterExtent)):
            return param.setValue(widget.getValue())
        else:
            return param.setValue(str(widget.text()))


    def accept(self):
        try:
            keepOpen = SextanteConfig.getSetting(SextanteConfig.KEEP_DIALOG_OPEN)
            if self.setParamValues():
                msg = self.alg.checkParameterValuesBeforeExecuting()
                if msg:
                    QMessageBox.critical(self.dialog, "Unable to execute algorithm", msg)
                    return
                keepOpen = SextanteConfig.getSetting(SextanteConfig.KEEP_DIALOG_OPEN)
                self.buttonBox.button(QtGui.QDialogButtonBox.Ok).setEnabled(False)
                buttons = self.paramTable.iterateButtons
                iterateParam = None
                for i in range(len(buttons.values())):
                    button = buttons.values()[i]
                    if button.isChecked():
                        iterateParam = buttons.keys()[i]
                        break
                self.progressLabel.setText("Processing algorithm...")
                if iterateParam:
                    QApplication.setOverrideCursor(QCursor(Qt.WaitCursor))
                    AlgorithmExecutor.runalgIterating(self.alg, iterateParam, self)
                    QApplication.restoreOverrideCursor()
                else:
                    QApplication.setOverrideCursor(QCursor(Qt.WaitCursor))
                    command = self.alg.getAsCommand()
                    if command:
                        SextanteLog.addToLog(SextanteLog.LOG_ALGORITHM, command)
                    ret = AlgorithmExecutor.runalg(self.alg, self)
                    QApplication.restoreOverrideCursor()
                    if ret:
                        SextantePostprocessing.handleAlgorithmResults(self.alg, not keepOpen)

                self.dialog.executed = True
                if not keepOpen:
                    self.dialog.close()
                else:
                    self.progressLabel.setText("")
                    self.progress.setValue(0)
                    self.buttonBox.button(QtGui.QDialogButtonBox.Ok).setEnabled(True)

            else:
                QMessageBox.critical(self.dialog, "Unable to execute algorithm", "Wrong or missing parameter values")
                return
        except GeoAlgorithmExecutionException, e :
            QApplication.restoreOverrideCursor()
            QMessageBox.critical(self, "Error",e.msg)
            SextanteLog.addToLog(SextanteLog.LOG_ERROR, e.msg)
            if not keepOpen:
                self.dialog.close()
            else:
                self.progressLabel.setText("")
                self.progress.setValue(0)
                self.buttonBox.button(QtGui.QDialogButtonBox.Ok).setEnabled(True)


    def reject(self):
        self.dialog.close()

    def setPercentage(self, i):
        self.progress.setValue(i)

    def setText(self, text):
        self.progressLabel.setText(text)


