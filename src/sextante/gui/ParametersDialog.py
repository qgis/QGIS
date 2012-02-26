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
from sextante.core.SextanteResults import SextanteResults

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class ParametersDialog(QtGui.QDialog):
    def __init__(self, alg):
        QtGui.QDialog.__init__(self)
        self.setModal(True)
        self.alg = None
        self.ui = Ui_ParametersDialog()
        self.ui.setupUi(self, alg)

class Ui_ParametersDialog(object):

    NOT_SELECTED = "[Not selected]"

    def setupUi(self, dialog, alg):
        self.alg = alg
        self.dialog = dialog

        self.valueItems = {}
        self.dependentItems = {}
        dialog.setObjectName(_fromUtf8("Parameters"))
        dialog.resize(650, 450)
        self.buttonBox = QtGui.QDialogButtonBox()
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Ok)
        self.buttonBox.setObjectName(_fromUtf8("buttonBox"))
        self.tableWidget = QtGui.QTableWidget()
        self.tableWidget.setSelectionMode(QtGui.QAbstractItemView.NoSelection)
        self.tableWidget.setColumnCount(2)
        self.tableWidget.setColumnWidth(0,300)
        self.tableWidget.setColumnWidth(1,300)
        self.tableWidget.setHorizontalHeaderItem(0, QtGui.QTableWidgetItem("Parameter"))
        self.tableWidget.setHorizontalHeaderItem(1, QtGui.QTableWidgetItem("Value"))
        self.tableWidget.setObjectName(_fromUtf8("tableWidget"))
        self.tableWidget.verticalHeader().setVisible(False)
        self.tableWidget.horizontalHeader().setResizeMode(QtGui.QHeaderView.Stretch)
        self.setTableContent()
        dialog.setWindowTitle(self.alg.name)
        self.progressLabel = QtGui.QLabel()
        self.progress = QtGui.QProgressBar()
        self.progress.setMinimum(0)
        self.progress.setMaximum(100)
        self.verticalLayout = QtGui.QVBoxLayout(dialog)
        self.verticalLayout.setSpacing(2)
        self.verticalLayout.setMargin(0)
        self.verticalLayout.setObjectName("hLayout")
        self.verticalLayout.addWidget(self.tableWidget)
        self.verticalLayout.addWidget(self.progressLabel)
        self.verticalLayout.addWidget(self.progress)
        self.verticalLayout.addWidget(self.buttonBox)
        dialog.setLayout(self.verticalLayout)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("accepted()")), self.accept)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("rejected()")), self.reject)
        QtCore.QMetaObject.connectSlotsByName(dialog)


    def getWidgetFromParameter(self, param):
        if isinstance(param, ParameterRaster):
            item = QtGui.QComboBox()
            layers = QGisLayers.getRasterLayers()
            if (param.optional):
                item.addItem(self.NOT_SELECTED, None)
            for layer in layers:
                item.addItem(layer.name(), layer)
        elif isinstance(param, ParameterVector):
            item = QtGui.QComboBox()
            layers = QGisLayers.getVectorLayers(param.shapetype)
            if (param.optional):
                item.addItem(self.NOT_SELECTED, None)
            for layer in layers:
                item.addItem(layer.name(), layer)
            item.currentIndexChanged.connect(self.updateDependentFields)
            item.name = param.name
        elif isinstance(param, ParameterTable):
            item = QtGui.QComboBox()
            layers = QGisLayers.getTables()
            if (param.optional):
                item.addItem(self.NOT_SELECTED, None)
            for layer in layers:
                item.addItem(layer.name(), layer)
            item.currentIndexChanged.connect(self.updateDependentFields)
            item.name = param.name
        elif isinstance(param, ParameterBoolean):
            item = QtGui.QComboBox()
            item.addItem("Yes")
            item.addItem("No")
        elif isinstance(param, ParameterTableField):
            item = QtGui.QComboBox()
            if param.parent in self.dependentItems:
                items = self.dependentItems[param.parent]
            else:
                items = []
                self.dependentItems[param.parent] = items
            items.append(param.name)
            layers = QGisLayers.getVectorLayers()
            if len(layers)>0:
                fields = self.getFields(layers[0])
                for i in fields:
                    item.addItem(fields[i].name())
        elif isinstance(param, ParameterSelection):
            item = QtGui.QComboBox()
            item.addItems(param.options)
        elif isinstance(param, ParameterFixedTable):
            item = FixedTablePanel(param)
        elif isinstance(param, ParameterMultipleInput):
            if param.datatype == ParameterMultipleInput.TYPE_VECTOR_ANY:
                options = QGisLayers.getVectorLayers()
            else:
                options = QGisLayers.getRasterLayers()
            opts = []
            for opt in options:
                opts.append(opt.name())
            item = MultipleInputPanel(opts)
        else:
            item = QtGui.QLineEdit()
            item.setText(str(param.default))

        return item

    def updateDependentFields(self):
        sender = self.dialog.sender()
        if not isinstance(sender, QComboBox):
            return
        if not sender.name in self.dependentItems:
            return
        layer = sender.itemData(sender.currentIndex()).toPyObject()
        children = self.dependentItems[sender.name]
        for child in children:
            widget = self.valueItems[child]
            widget.clear()
            fields = self.getFields(layer)
            for i in fields:
                widget.addItem(fields[i].name())


    def getFields(self, layer):
        return layer.dataProvider().fields()


    def setTableContent(self):
        params = self.alg.parameters
        outputs = self.alg.outputs
        numParams = len(self.alg.parameters)
        numOutputs = len(self.alg.outputs)
        self.tableWidget.setRowCount(numParams + numOutputs)

        i=0
        for param in params:
            item = QtGui.QTableWidgetItem(param.description)
            item.setFlags(QtCore.Qt.ItemIsEnabled)
            self.tableWidget.setItem(i,0, item)
            item = self.getWidgetFromParameter(param)
            self.valueItems[param.name] = item
            self.tableWidget.setCellWidget(i,1, item)
            self.tableWidget.setRowHeight(i,22)
            i+=1

        for output in outputs:
            item = QtGui.QTableWidgetItem(output.description + "<" + output.__module__.split(".")[-1] + ">")
            item.setFlags(QtCore.Qt.ItemIsEnabled)
            self.tableWidget.setItem(i,0, item)
            item = OutputSelectionPanel(output,self.alg)
            self.valueItems[output.name] = item
            self.tableWidget.setCellWidget(i,1, item)
            self.tableWidget.setRowHeight(i,22)
            i+=1

    def setParamValues(self):
        params = self.alg.parameters
        outputs = self.alg.outputs

        for param in params:
            if not self.setParamValue(param, self.valueItems[param.name]):
                return False

        for output in outputs:
            output.value = self.valueItems[output.name].getValue()

        return True

    def setParamValue(self, param, widget):

        if isinstance(param, ParameterRaster):
            return param.setValue(widget.itemData(widget.currentIndex()).toPyObject())
        elif isinstance(param, ParameterVector):
            return param.setValue(widget.itemData(widget.currentIndex()).toPyObject())
        elif isinstance(param, ParameterTable):
            return  param.setValue(widget.itemData(widget.currentIndex()).toPyObject())
        elif isinstance(param, ParameterBoolean):
            return param.setValue(widget.currentIndex() == 0)
        elif isinstance(param, ParameterSelection):
            return param.setValue(widget.currentIndex())
        elif isinstance(param, ParameterFixedTable):
            return param.setValue(widget.table)
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
        else:
            return param.setValue(widget.text())


    def accept(self):
        try:
            if self.setParamValues():
                QApplication.setOverrideCursor(QCursor(Qt.WaitCursor))
                SextanteLog.addToLog(SextanteLog.LOG_ALGORITHM, self.alg.getAsCommand())
                AlgorithmExecutor.runalg(self.alg, self)
                SextanteResults.handleAlgorithmResults(self.alg)
                QApplication.restoreOverrideCursor()
                self.dialog.alg = self.alg
                self.dialog.close()
            else:
                QMessageBox.critical(self.dialog, "Unable to execute algorithm", "Wrong or missing parameter values")
                return
        except GeoAlgorithmExecutionException, e :
            QApplication.restoreOverrideCursor()
            QMessageBox.critical(self, "Error",e.msg)
            self.dialog.close()



    def reject(self):
        self.dialog.alg = None
        self.dialog.close()

    def setPercentage(self, i):
        self.progress.setValue(i)

    def setFinished(self):
        pass

    def setText(self, text):
        self.progressLabel.setText(text)


