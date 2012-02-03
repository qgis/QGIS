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
from sextante.parameters.ParameterNumber import ParameterNumber
from sextante.parameters.ParameterRange import ParameterRange
from sextante.parameters.ParameterTableField import ParameterTableField
from sextante.parameters.ParameterTable import ParameterTable

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

    SAVE_TO_TEMP_FILE = "[Save to temporary file]"
    NOT_SELECTED = "[Not selected]"

    def setupUi(self, dialog, alg):
        self.alg = alg
        self.dialog = dialog
        self.valueItems = {}
        self.dependentItems = {}
        dialog.setObjectName(_fromUtf8("Parameters"))
        dialog.resize(650, 450)
        self.buttonBox = QtGui.QDialogButtonBox(dialog)
        self.buttonBox.setGeometry(QtCore.QRect(110, 400, 441, 32))
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Ok)
        self.buttonBox.setObjectName(_fromUtf8("buttonBox"))
        self.tableWidget = QtGui.QTableWidget(dialog)
        self.tableWidget.setGeometry(QtCore.QRect(5, 5, 640, 350))
        self.tableWidget.setSelectionMode(QtGui.QAbstractItemView.NoSelection)
        self.tableWidget.setColumnCount(2)
        self.tableWidget.setColumnWidth(0,300)
        self.tableWidget.setColumnWidth(1,300)
        self.tableWidget.setHorizontalHeaderItem(0, QtGui.QTableWidgetItem("Parameter"))
        self.tableWidget.setHorizontalHeaderItem(1, QtGui.QTableWidgetItem("Value"))
        self.tableWidget.setObjectName(_fromUtf8("tableWidget"))
        self.tableWidget.verticalHeader().setVisible(False)
        self.setTableContent()
        dialog.setWindowTitle(self.alg.name)
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
            layers = QGisLayers.getVectorLayers()
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
            if isinstance(param, ParameterNumber):
                item.setText("0")
            elif isinstance(param, ParameterRange):
                item.setText("0,1")


        return item

    def updateDependentFields(self):
        sender = self.dialog.sender()
        if not isinstance(sender, QComboBox):
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
            item = QtGui.QLineEdit()
            item.setText(self.SAVE_TO_TEMP_FILE)
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
            filename = str(self.valueItems[output.name].text())
            if filename.strip == "" or filename == self.SAVE_TO_TEMP_FILE:
                output.channel = None
            else:
                output.channel = filename

        return True

    def setParamValue(self, param, widget):

        if isinstance(param, ParameterRaster):
            param.setValue(widget.itemData(widget.currentIndex()).toPyObject())
        elif isinstance(param, ParameterVector):
            param.setValue(widget.itemData(widget.currentIndex()).toPyObject())
        elif isinstance(param, ParameterTable):
            param.setValue(widget.itemData(widget.currentIndex()).toPyObject())
        elif isinstance(param, ParameterBoolean):
            param.setValue(widget.currentIndex() == 0)
        elif isinstance(param, ParameterSelection):
            param.setValue(widget.currentIndex())
        elif isinstance(param, ParameterFixedTable):
            param.setValue(widget.table)
        elif isinstance(param, ParameterMultipleInput):
            if param.datatype == ParameterMultipleInput.TYPE_VECTOR_ANY:
                options = QGisLayers.getVectorLayers()
            else:
                options = QGisLayers.getRasterLayers()
            value = []
            for index in widget.selectedoptions:
                value.append(options[index])
            param.setValue(value)
        else:
            return param.setValue(widget.text())

        return True

    def accept(self):
        if self.setParamValues():
            self.dialog.alg = self.alg
            self.dialog.close()
        else:
            QMessageBox.warning(self.dialog, "Unable to execute algorithm", "Wrong or missing parameter values")

    def reject(self):
        self.dialog.alg = None
        self.dialog.close()




