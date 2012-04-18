from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4 import QtCore, QtGui
from sextante.core.QGisLayers import QGisLayers
from sextante.parameters.ParameterRaster import ParameterRaster
from sextante.parameters.ParameterVector import ParameterVector
from sextante.parameters.ParameterBoolean import ParameterBoolean
from sextante.parameters.ParameterSelection import ParameterSelection
from sextante.parameters.ParameterMultipleInput import ParameterMultipleInput
from sextante.gui.MultipleInputPanel import MultipleInputPanel
from sextante.parameters.ParameterFixedTable import ParameterFixedTable
from sextante.gui.FixedTablePanel import FixedTablePanel
from sextante.parameters.ParameterTableField import ParameterTableField
from sextante.parameters.ParameterTable import ParameterTable
from sextante.gui.OutputSelectionPanel import OutputSelectionPanel
from sextante.gui.AlgorithmExecutor import AlgorithmExecutor
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from sextante.core.SextanteLog import SextanteLog
from sextante.gui.SextantePostprocessing import SextantePostprocessing
from sextante.gui.RangePanel import RangePanel
from sextante.parameters.ParameterRange import ParameterRange
from sextante.gui.HTMLViewerDialog import HTMLViewerDialog
from sextante.gui.NumberInputPanel import NumberInputPanel
from sextante.parameters.ParameterNumber import ParameterNumber
from sextante.gui.InputLayerSelectorPanel import InputLayerSelectorPanel
from sextante.parameters.ParameterExtent import ParameterExtent
from sextante.gui.ExtentSelectionPanel import ExtentSelectionPanel
from sextante.gui.ParametersTable import ParametersTable

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class ParametersDialog(QtGui.QDialog):
    '''the default parameters dialog, to be used when an algorithm is called from the toolbox'''
    def __init__(self, alg):
        QtGui.QDialog.__init__(self)
        #self.setModal(False)
        self.ui = Ui_ParametersDialog()
        self.ui.setupUi(self, alg)
        self.executed = False

class Ui_ParametersDialog(object):

    NOT_SELECTED = "[Not selected]"

    def setupUi(self, dialog, alg):
        self.alg = alg
        self.dialog = dialog
        #self.valueItems = {}
        #self.dependentItems = {}
        dialog.resize(650, 450)
        self.buttonBox = QtGui.QDialogButtonBox()
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Ok)
        if self.alg.helpFile():
            self.showHelpButton = QtGui.QPushButton()
            self.showHelpButton.setText("Show help")
            self.buttonBox.addButton(self.showHelpButton, QtGui.QDialogButtonBox.ActionRole)
            QtCore.QObject.connect(self.showHelpButton, QtCore.SIGNAL("clicked()"), self.showHelp)
        #=======================================================================
        # self.tableWidget = QtGui.QTableWidget()
        # self.tableWidget.setSelectionMode(QtGui.QAbstractItemView.NoSelection)
        # self.tableWidget.setColumnCount(2)
        # self.tableWidget.setColumnWidth(0,300)
        # self.tableWidget.setColumnWidth(1,300)
        # self.tableWidget.setHorizontalHeaderItem(0, QtGui.QTableWidgetItem("Parameter"))
        # self.tableWidget.setHorizontalHeaderItem(1, QtGui.QTableWidgetItem("Value"))
        # self.tableWidget.verticalHeader().setVisible(False)
        # self.tableWidget.horizontalHeader().setResizeMode(QtGui.QHeaderView.Stretch)
        # self.setTableContent()
        #=======================================================================
        self.paramTable = ParametersTable(self.alg, self.dialog)
        self.scrollArea = QtGui.QScrollArea()
        self.scrollArea.setWidget(self.paramTable)
        self.scrollArea.setWidgetResizable(True)
        dialog.setWindowTitle(self.alg.name)
        self.progressLabel = QtGui.QLabel()
        self.progress = QtGui.QProgressBar()
        self.progress.setMinimum(0)
        self.progress.setMaximum(100)
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

#===============================================================================
#    def somethingDependsOnThisParameter(self, parent):
#        for param in self.alg.parameters:
#            if isinstance(param, ParameterTableField):
#                if param.parent == parent.name:
#                    return True
#        return False
#
#    def getWidgetFromParameter(self, param):
#        if isinstance(param, ParameterRaster):
#            layers = QGisLayers.getRasterLayers()
#            items = []
#            if (param.optional):
#                items.append((self.NOT_SELECTED, None))
#            for layer in layers:
#                items.append((layer.name(), layer))
#            item = InputLayerSelectorPanel(items)
#        elif isinstance(param, ParameterVector):
#            if self.somethingDependsOnThisParameter(param):
#                item = QtGui.QComboBox()
#                layers = QGisLayers.getVectorLayers(param.shapetype)
#                if (param.optional):
#                    item.addItem(self.NOT_SELECTED, None)
#                for layer in layers:
#                    item.addItem(layer.name(), layer)
#                item.currentIndexChanged.connect(self.updateDependentFields)
#                item.name = param.name
#            else:
#                layers = QGisLayers.getVectorLayers(param.shapetype)
#                items = []
#                if (param.optional):
#                    items.append((self.NOT_SELECTED, None))
#                for layer in layers:
#                    items.append((layer.name(), layer))
#                item = InputLayerSelectorPanel(items)
#        elif isinstance(param, ParameterTable):
#            if self.somethingDependsOnThisParameter(param):
#                item = QtGui.QComboBox()
#                layers = QGisLayers.getTables()
#                if (param.optional):
#                    item.addItem(self.NOT_SELECTED, None)
#                for layer in layers:
#                    item.addItem(layer.name(), layer)
#                item.currentIndexChanged.connect(self.updateDependentFields)
#                item.name = param.name
#            else:
#                layers = QGisLayers.getTables()
#                items = []
#                if (param.optional):
#                    items.append((self.NOT_SELECTED, None))
#                for layer in layers:
#                    items.append((layer.name(), layer))
#                item = InputLayerSelectorPanel(items)
#        elif isinstance(param, ParameterBoolean):
#            item = QtGui.QComboBox()
#            item.addItem("Yes")
#            item.addItem("No")
#            if param.default:
#                item.setCurrentIndex(0)
#            else:
#                item.setCurrentIndex(1)
#        elif isinstance(param, ParameterTableField):
#            item = QtGui.QComboBox()
#            if param.parent in self.dependentItems:
#                items = self.dependentItems[param.parent]
#            else:
#                items = []
#                self.dependentItems[param.parent] = items
#            items.append(param.name)
#            parent = self.alg.getParameterFromName(param.parent)
#            if isinstance(parent, ParameterVector):
#                layers = QGisLayers.getVectorLayers(parent.shapetype)
#            else:
#                layers = QGisLayers.getTables()
#            if len(layers)>0:
#                fields = self.getFields(layers[0])
#                for i in fields:
#                    item.addItem(fields[i].name())
#        elif isinstance(param, ParameterSelection):
#            item = QtGui.QComboBox()
#            item.addItems(param.options)
#            item.setCurrentIndex(param.default)
#        elif isinstance(param, ParameterFixedTable):
#            item = FixedTablePanel(param)
#        elif isinstance(param, ParameterRange):
#            item = RangePanel(param)
#        elif isinstance(param, ParameterMultipleInput):
#            if param.datatype == ParameterMultipleInput.TYPE_RASTER:
#                options = QGisLayers.getRasterLayers()
#            elif param.datatype == ParameterMultipleInput.TYPE_VECTOR_ANY:
#                options = QGisLayers.getVectorLayers()
#            else:
#                options = QGisLayers.getVectorLayers(param.datatype)
#            opts = []
#            for opt in options:
#                opts.append(opt.name())
#            item = MultipleInputPanel(opts)
#        elif isinstance(param, ParameterNumber):
#            item = NumberInputPanel(param.default, param.isInteger)
#        elif isinstance(param, ParameterExtent):
#            item = ExtentSelectionPanel(self.dialog, param.default)
#        else:
#            item = QtGui.QLineEdit()
#            item.setText(str(param.default))
#
#        return item
#
#    def updateDependentFields(self):
#        sender = self.dialog.sender()
#        if not isinstance(sender, QComboBox):
#            return
#        if not sender.name in self.dependentItems:
#            return
#        layer = sender.itemData(sender.currentIndex()).toPyObject()
#        children = self.dependentItems[sender.name]
#        for child in children:
#            widget = self.valueItems[child]
#            widget.clear()
#            fields = self.getFields(layer)
#            for i in fields:
#                widget.addItem(fields[i].name())
#
#
#    def getFields(self, layer):
#        return layer.dataProvider().fields()
#===============================================================================

    def showHelp(self):
        if self.alg.helpFile():
            dlg = HTMLViewerDialog(self.alg.helpFile())
            dlg.exec_()
        else:
            QMessageBox.warning(self.dialog, "No help available", "No help is available for the current algorithm.")

#===============================================================================
#    def setTableContent(self):
#        params = self.alg.parameters
#        outputs = self.alg.outputs
#        numParams = len(self.alg.parameters)
#        numOutputs = 0
#        for output in outputs:
#            if not output.hidden:
#                numOutputs += 1
#        self.tableWidget.setRowCount(numParams + numOutputs)
#
#        i=0
#        for param in params:
#            item = QtGui.QTableWidgetItem(param.description)
#            item.setFlags(QtCore.Qt.ItemIsEnabled)
#            self.tableWidget.setItem(i,0, item)
#            item = self.getWidgetFromParameter(param)
#            self.valueItems[param.name] = item
#            self.tableWidget.setCellWidget(i,1, item)
#            self.tableWidget.setRowHeight(i,22)
#            i+=1
#
#        for output in outputs:
#            if output.hidden:
#                continue
#            item = QtGui.QTableWidgetItem(output.description + "<" + output.__module__.split(".")[-1] + ">")
#            item.setFlags(QtCore.Qt.ItemIsEnabled)
#            self.tableWidget.setItem(i,0, item)
#            item = OutputSelectionPanel(output,self.alg)
#            self.valueItems[output.name] = item
#            self.tableWidget.setCellWidget(i,1, item)
#            self.tableWidget.setRowHeight(i,22)
#            i+=1
#
#        self.fillParameterValuesFromHistory()
#
#    def fillParameterValuesFromHistory(self):
#        pass
#===============================================================================

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
        elif isinstance(param, ParameterNumber):
            return param.setValue(widget.getValue())
        else:
            return param.setValue(str(widget.text()))


    def accept(self):
        try:
            if self.setParamValues():
                QApplication.setOverrideCursor(QCursor(Qt.WaitCursor))
                SextanteLog.addToLog(SextanteLog.LOG_ALGORITHM, self.alg.getAsCommand())
                ret = AlgorithmExecutor.runalg(self.alg, self)
                QApplication.restoreOverrideCursor()
                if ret:
                    SextantePostprocessing.handleAlgorithmResults(self.alg)
                self.dialog.executed = True
                self.dialog.close()

            else:
                QMessageBox.critical(self.dialog, "Unable to execute algorithm", "Wrong or missing parameter values")
                return
        except GeoAlgorithmExecutionException, e :
            QApplication.restoreOverrideCursor()
            QMessageBox.critical(self, "Error",e.msg)
            SextanteLog.addToLog(SextanteLog.LOG_ERROR, e.msg)
            self.dialog.executed = False
            self.dialog.close()


    def reject(self):
        self.dialog.executed = False
        self.dialog.close()

    def setPercentage(self, i):
        self.progress.setValue(i)

    def setText(self, text):
        self.progressLabel.setText(text)


