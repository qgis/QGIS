from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4 import QtCore, QtGui
from sextante.parameters.ParameterRaster import ParameterRaster
from sextante.parameters.ParameterVector import ParameterVector
from sextante.parameters.ParameterBoolean import ParameterBoolean
from sextante.parameters.ParameterSelection import ParameterSelection
from sextante.parameters.ParameterMultipleInput import ParameterMultipleInput
from sextante.gui.MultipleInputPanel import MultipleInputPanel
from sextante.parameters.ParameterFixedTable import ParameterFixedTable
from sextante.gui.FixedTablePanel import FixedTablePanel
from sextante.parameters.ParameterNumber import ParameterNumber
from sextante.parameters.ParameterTableField import ParameterTableField
from sextante.parameters.ParameterTable import ParameterTable
from sextante.parameters.ParameterString import ParameterString
from sextante.outputs.OutputRaster import OutputRaster
from sextante.outputs.OutputVector import OutputVector
from sextante.outputs.OutputTable import OutputTable
from sextante.modeler.ModelerAlgorithm import AlgorithmAndParameter
from sextante.parameters.ParameterRange import ParameterRange
from sextante.gui.RangePanel import RangePanel
from sextante.outputs.OutputNumber import OutputNumber

class ModelerParametersDialog(QtGui.QDialog):

    ENTER_NAME = "[Enter name if this is a final result]"

    NOT_SELECTED = "[Not selected]"

    def __init__(self, alg, model):
        QtGui.QDialog.__init__(self)
        self.setModal(True)
        self.alg = alg
        self.model = model
        self.setupUi()
        self.params = None

    def setupUi(self):
        self.valueItems = {}
        self.dependentItems = {}
        self.setObjectName("Parameters")
        self.resize(650, 450)
        self.buttonBox = QtGui.QDialogButtonBox()
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Ok)
        self.buttonBox.setObjectName("buttonBox")
        self.tableWidget = QtGui.QTableWidget()
        self.tableWidget.setSelectionMode(QtGui.QAbstractItemView.NoSelection)
        self.tableWidget.setColumnCount(2)
        self.tableWidget.setColumnWidth(0,300)
        self.tableWidget.setColumnWidth(1,300)
        self.tableWidget.setHorizontalHeaderItem(0, QtGui.QTableWidgetItem("Parameter"))
        self.tableWidget.setHorizontalHeaderItem(1, QtGui.QTableWidgetItem("Value"))
        self.tableWidget.setObjectName("tableWidget")
        self.tableWidget.verticalHeader().setVisible(False)
        self.tableWidget.horizontalHeader().setResizeMode(QtGui.QHeaderView.Stretch)
        self.setTableContent()
        self.setWindowTitle(self.alg.name)
        self.verticalLayout = QtGui.QVBoxLayout()
        self.verticalLayout.setSpacing(2)
        self.verticalLayout.setMargin(0)
        self.verticalLayout.setObjectName("hLayout")
        self.verticalLayout.addWidget(self.tableWidget)
        self.verticalLayout.addWidget(self.buttonBox)
        self.setLayout(self.verticalLayout)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL("accepted()"), self.okPressed)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL("rejected()"), self.cancelPressed)
        QtCore.QMetaObject.connectSlotsByName(self)


    def getRasterLayers(self):
        layers = []
        params = self.model.parameters
        for param in params:
            if isinstance(param, ParameterRaster):
                layers.append(AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, param.name, "", param.description))

        i=0
        for alg in self.model.algs:
            for out in alg.outputs:
                if isinstance(out, OutputRaster):
                    layers.append(AlgorithmAndParameter(i, out.name, alg.name, out.description))
            i+=1

        return layers

    def getVectorLayers(self):
        layers = []
        params = self.model.parameters
        for param in params:
            if isinstance(param, ParameterVector):
                layers.append(AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, param.name, "", param.description))

        i=0
        for alg in self.model.algs:
            for out in alg.outputs:
                if isinstance(out, OutputVector):
                    layers.append(AlgorithmAndParameter(i, out.name, alg.name, out.description))
            i+=1

        return layers

    def getTables(self):
        tables = []
        params = self.model.parameters
        for param in params:
            if isinstance(param, ParameterTable):
                tables.append(AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, param.name, "", param.description))

        i=0
        for alg in self.model.algs:
            for out in alg.outputs:
                if isinstance(out, OutputTable):
                    tables.append(AlgorithmAndParameter(i, out.name, alg.name, out.description))
            i+=1

        return tables

    def getNumbers(self):
        numbers = []
        params = self.model.parameters
        for param in params:
            if isinstance(param, ParameterNumber):
                numbers.append(AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, param.name, "", param.description))

        i=0
        for alg in self.model.algs:
            for out in alg.outputs:
                if isinstance(out, OutputNumber):
                    numbers.append(AlgorithmAndParameter(i, out.name, alg.name, out.description))
            i+=1
        return numbers


    def getBooleans(self):
        booleans = []
        params = self.model.parameters
        for param in params:
            if isinstance(param, ParameterBoolean):
                booleans.append(AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, param.name, "", param.description))
        return booleans

    def getStrings(self):
        strings = []
        params = self.model.parameters
        for param in params:
            if isinstance(param, ParameterString):
                strings.append(AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, param.name, "", param.description))
        return strings

    def getTableFields(self):
        strings = []
        params = self.model.parameters
        for param in params:
            if isinstance(param, ParameterTableField):
                strings.append(AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, param.name, "", param.description))
        return strings

    def getWidgetFromParameter(self, param):
        if isinstance(param, ParameterRaster):
            item = QtGui.QComboBox()
            layers = self.getRasterLayers()
            if (param.optional):
                item.addItem(self.NOT_SELECTED, None)
            for layer in layers:
                item.addItem(layer.name(), layer)
        elif isinstance(param, ParameterVector):
            item = QtGui.QComboBox()
            layers = self.getVectorLayers()
            if (param.optional):
                item.addItem(self.NOT_SELECTED, None)
            for layer in layers:
                item.addItem(layer.name(), layer)
        elif isinstance(param, ParameterTable):
            item = QtGui.QComboBox()
            layers = self.getTables()
            if (param.optional):
                item.addItem(self.NOT_SELECTED, None)
            for layer in layers:
                item.addItem(layer.name(), layer)
        elif isinstance(param, ParameterBoolean):
            item = QtGui.QComboBox()
            item.addItem("Yes")
            item.addItem("No")
            bools = self.getBooleans()
            for b in bools:
                item.addItem(b.name(), b)
        elif isinstance(param, ParameterTableField):
            item = QtGui.QLineEdit()
        elif isinstance(param, ParameterSelection):
            item = QtGui.QComboBox()
            item.addItems(param.options)
        elif isinstance(param, ParameterFixedTable):
            item = FixedTablePanel(param)
        elif isinstance(param, ParameterRange):
            item = RangePanel(param)
        elif isinstance(param, ParameterMultipleInput):
            if param.datatype == ParameterMultipleInput.TYPE_VECTOR_ANY:
                options = self.getVectorLayers()
            else:
                options = self.getRasterLayers()
            opts = []
            for opt in options:
                opts.append(opt.name())
            item = MultipleInputPanel(opts)
        elif isinstance(param, ParameterString):
            item = QtGui.QComboBox()
            item.setEditable(True)
            strings = self.getStrings()
            for s in strings:
                item.addItem(s.name(), s)
            item.setEditText(str(param.default))
        elif isinstance(param, ParameterTableField):
            item = QtGui.QComboBox()
            item.setEditable(True)
            fields = self.getTableFields()
            for f in fields:
                item.addItem(f.name(), f)
        elif isinstance(param, ParameterNumber):
            item = QtGui.QComboBox()
            item.setEditable(True)
            numbers = self.getNumbers()
            for n in numbers:
                item.addItem(n.name(), n)
            item.setEditText(str(param.default))
        else:
            item = QtGui.QLineEdit()
            try:
                item.setText(str(param.default))
            except:
                pass
        return item


    def setTableContent(self):
        params = self.alg.parameters
        outputs = self.alg.outputs
        numParams = len(self.alg.parameters)
        numOutputs = 0
        for output in outputs:
            if not output.hidden:
                numOutputs += 1
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
            if not output.hidden:
                item = QtGui.QTableWidgetItem(output.description + "<" + output.__module__.split(".")[-1] + ">")
                item.setFlags(QtCore.Qt.ItemIsEnabled)
                self.tableWidget.setItem(i,0, item)
                item = QLineEdit()
                if hasattr(self.item, 'setPlaceholderText'):
                    item.setPlaceholderText(ModelerParametersDialog.ENTER_NAME)
                self.valueItems[output.name] = item
                self.tableWidget.setCellWidget(i,1, item)
                self.tableWidget.setRowHeight(i,22)
                i+=1

    def setParamValues(self):
        self.params = {}
        self.values = {}
        self.outputs = {}

        params = self.alg.parameters
        outputs = self.alg.outputs
        for param in params:
            if not self.setParamValue(param, self.valueItems[param.name]):
                return False
        for output in outputs:
            if output.hidden:
                continue
            name= str(self.valueItems[output.name].text())
            if name.strip()!="" and name != ModelerParametersDialog.ENTER_NAME:
                self.outputs[output.name]=name
            else:
                self.outputs[output.name] = None
        return True


    def setParamRasterValue(self, param, widget):
        if widget.currentIndex() < 0:
            return False
        value = widget.itemData(widget.currentIndex()).toPyObject()
        self.params[param.name] = value
        return True

    def setParamVectorValue(self, param, widget):
        if widget.currentIndex() < 0:
            return False
        value = widget.itemData(widget.currentIndex()).toPyObject()
        self.params[param.name] = value
        return True

    def setParamTableValue(self, param, widget):
        if widget.currentIndex() < 0:
            return False
        value = widget.itemData(widget.currentIndex()).toPyObject()
        self.params[param.name] = value
        return True

    def setParamBooleanValue(self, param, widget):
        if widget.currentIndex() < 2:
            name =  self.model.getSafeNameForHarcodedParameter(param)
            value = AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, name)
            self.params[param.name] = value
            self.values[name] = str(widget.currentIndex() == 0)
        else:
            value = widget.itemData(widget.currentIndex()).toPyObject()
            self.params[param.name] = value
        return True

    def setParamStringValue(self, param, widget):
        if widget.currentIndex() < 0:
            name =  self.model.getSafeNameForHarcodedParameter(param)
            value = AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, name)
            self.params[param.name] = value
            self.values[name] = str(widget.currentText())
        else:
            value = widget.itemData(widget.currentIndex()).toPyObject()
            self.params[param.name] = value
        return True


    def setParamNumberValue(self, param, widget):
        if widget.currentIndex() < 0:
            name =  self.model.getSafeNameForHarcodedParameter(param)
            value = AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, name)
            self.params[param.name] = value
            s = str(widget.currentText())
            try:
                float(s)
                self.values[name] = s
            except:
                return False
        else:
            value = widget.itemData(widget.currentIndex()).toPyObject()
            self.params[param.name] = value
        return True

    def setParamValue(self, param, widget):
        if isinstance(param, ParameterRaster):
            return self.setParamRasterValue(param, widget)
        elif isinstance(param, ParameterVector):
            return self.setParamVectorValue(param, widget)
        elif isinstance(param, ParameterTable):
            return self.setParamTableValue(param, widget)
        elif isinstance(param, ParameterBoolean):
            return self.setParamBooleanValue(param, widget)
        elif isinstance(param, ParameterString):
            return self.setParamStringValue(param, widget)
        elif isinstance(param, ParameterNumber):
            return self.setParamNumberValue(param, widget)
        elif isinstance(param, ParameterSelection):
            name =  self.model.getSafeNameForHarcodedParameter(param)
            value = AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, name)
            self.params[param.name] = value
            self.values[name] = str(widget.currentIndex())
            return True
        elif isinstance(param, ParameterRange):
            name =  self.model.getSafeNameForHarcodedParameter(param)
            value = AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, name)
            self.params[param.name] = value
            self.values[name] = str(widget.getValue())
            return True
        elif isinstance(param, ParameterFixedTable):
            name =  self.model.getSafeNameForHarcodedParameter(param)
            value = AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, name)
            self.params[param.name] = value
            self.values[name] = ParameterFixedTable.tableToString(widget.table)
            return True
        if isinstance(param, ParameterTableField):
            name =  self.model.getSafeNameForHarcodedParameter(param)
            value = AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, name)
            self.params[param.name] = value
            self.values[name] = str(widget.text())
            return True
        elif isinstance(param, ParameterMultipleInput):
            if param.datatype == ParameterMultipleInput.TYPE_VECTOR_ANY:
                options = self.getVectorLayers()
            else:
                options = self.getRasterLayers()
            values = []
            for index in widget.selectedoptions:
                values.append(options[index].serialize())
            if len(values) == 0 and not param.optional:
                return False
            name =  self.model.getSafeNameForHarcodedParameter(param)
            value = AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, name)
            self.params[param.name] = value
            self.values[name] = ";".join(values)
            return True


    def okPressed(self):
        if self.setParamValues():
            self.close()
        else:
            QMessageBox.critical(self, "Unable to add algorithm", "Wrong or missing parameter values")
            self.params = None


    def cancelPressed(self):
        self.params = None
        self.close()

