from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4 import QtCore, QtGui, QtWebKit
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
from sextante.parameters.ParameterFile import ParameterFile
from sextante.outputs.OutputFile import OutputFile
from sextante.core.WrongHelpFileException import WrongHelpFileException
from sextante.parameters.ParameterExtent import ParameterExtent

class ModelerParametersDialog(QtGui.QDialog):

    ENTER_NAME = "[Enter name if this is a final result]"
    NOT_SELECTED = "[Not selected]"

    def __init__(self, alg, model, algIndex = None):
        QtGui.QDialog.__init__(self)
        self.setModal(True)
        self.alg = alg
        self.model = model
        self.algIndex = algIndex
        self.setupUi()
        self.params = None


    def setupUi(self):
        self.valueItems = {}
        self.dependentItems = {}
        self.resize(650, 450)
        self.buttonBox = QtGui.QDialogButtonBox()
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Ok)
        self.tableWidget = QtGui.QTableWidget()
        self.tableWidget.setSelectionMode(QtGui.QAbstractItemView.NoSelection)
        self.tableWidget.setColumnCount(2)
        self.tableWidget.setColumnWidth(0,300)
        self.tableWidget.setColumnWidth(1,300)
        self.tableWidget.setHorizontalHeaderItem(0, QtGui.QTableWidgetItem("Parameter"))
        self.tableWidget.setHorizontalHeaderItem(1, QtGui.QTableWidgetItem("Value"))
        self.tableWidget.verticalHeader().setVisible(False)
        self.tableWidget.horizontalHeader().setResizeMode(QtGui.QHeaderView.Stretch)
        self.setTableContent()
        self.setPreviousValues()
        self.setWindowTitle(self.alg.name)
        self.verticalLayout = QtGui.QVBoxLayout()
        self.verticalLayout.setSpacing(2)
        self.verticalLayout.setMargin(0)
        self.tabWidget = QtGui.QTabWidget()
        self.tabWidget.setMinimumWidth(300)
        self.tabWidget.addTab(self.tableWidget, "Parameters")
        self.webView = QtWebKit.QWebView()
        html = None
        try:
            if self.alg.helpFile():
                helpFile = self.alg.helpFile()
            else:
                html = "<h2>Sorry, no help is available for this algorithm.</h2>"
        except WrongHelpFileException, e:
            html = e.msg
            self.webView.setHtml("<h2>Could not open help file :-( </h2>")
        try:
            if html:
                self.webView.setHtml(html)
            else:
                url = QtCore.QUrl(helpFile)
                self.webView.load(url)
        except:
            self.webView.setHtml("<h2>Could not open help file :-( </h2>")
        self.tabWidget.addTab(self.webView, "Help")
        self.verticalLayout.addWidget(self.tabWidget)
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

        if self.algIndex is None:
            dependent = []
        else:
            dependent = self.model.getDependentAlgorithms(self.algIndex)
            dependent.append(self.algIndex)

        i=0
        for alg in self.model.algs:
            if i not in dependent:
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

        if self.algIndex is None:
            dependent = []
        else:
            dependent = self.model.getDependentAlgorithms(self.algIndex)
            dependent.append(self.algIndex)

        i=0
        for alg in self.model.algs:
            if i not in dependent:
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

        if self.algIndex is None:
            dependent = []
        else:
            dependent = self.model.getDependentAlgorithms(self.algIndex)
            dependent.append(self.algIndex)

        i=0
        for alg in self.model.algs:
            if i not in dependent:
                for out in alg.outputs:
                    if isinstance(out, OutputTable):
                        tables.append(AlgorithmAndParameter(i, out.name, alg.name, out.description))
            i+=1

        return tables

    def getExtents(self):
        extents = []
        params = self.model.parameters
        for param in params:
            if isinstance(param, ParameterExtent):
                extents.append(AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, param.name, "", param.description))
        return extents

    def getNumbers(self):
        numbers = []
        params = self.model.parameters
        for param in params:
            if isinstance(param, ParameterNumber):
                numbers.append(AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, param.name, "", param.description))

        if self.algIndex is None:
            dependent = []
        else:
            dependent = self.model.getDependentAlgorithms(self.algIndex)
            dependent.append(self.algIndex)

        i=0
        for alg in self.model.algs:
            if i not in dependent:
                for out in alg.outputs:
                    if isinstance(out, OutputNumber):
                        numbers.append(AlgorithmAndParameter(i, out.name, alg.name, out.description))
            i+=1
        return numbers

    def getFiles(self):
        files = []
        params = self.model.parameters
        for param in params:
            if isinstance(param, ParameterFile):
                files.append(AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, param.name, "", param.description))

        if self.algIndex is None:
            dependent = []
        else:
            dependent = self.model.getDependentAlgorithms(self.algIndex)
            dependent.append(self.algIndex)

        i=0
        for alg in self.model.algs:
            if i not in dependent:
                for out in alg.outputs:
                    if isinstance(out, OutputFile):
                        files.append(AlgorithmAndParameter(i, out.name, alg.name, out.description))
            i+=1
        return files

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
            item.setEditable(True)
            layers = self.getRasterLayers()
            if (param.optional):
                item.addItem(self.NOT_SELECTED, None)
            for layer in layers:
                item.addItem(layer.name(), layer)
        elif isinstance(param, ParameterVector):
            item = QtGui.QComboBox()
            item.setEditable(True)
            layers = self.getVectorLayers()
            if (param.optional):
                item.addItem(self.NOT_SELECTED, None)
            for layer in layers:
                item.addItem(layer.name(), layer)
        elif isinstance(param, ParameterTable):
            item = QtGui.QComboBox()
            item.setEditable(True)
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
        elif isinstance(param, ParameterExtent):
            item = QtGui.QComboBox()
            item.setEditable(True)
            extents = self.getExtents()
            for ex in extents:
                item.addItem(ex.name(), ex)
            item.setEditText(str(param.default))
        elif isinstance(param, ParameterFile):
            item = QtGui.QComboBox()
            item.setEditable(True)
            files = self.getFiles()
            for f in files:
                item.addItem(f.name(), f)
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
        numParams = 0
        for param in params:
            if not param.hidden:
                numParams += 1
        numOutputs = 0
        for output in outputs:
            if not output.hidden:
                numOutputs += 1
        self.tableWidget.setRowCount(numParams + numOutputs)

        i=0
        for param in params:
            if not param.hidden:
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
                if hasattr(item, 'setPlaceholderText'):
                    item.setPlaceholderText(ModelerParametersDialog.ENTER_NAME)
                self.valueItems[output.name] = item
                self.tableWidget.setCellWidget(i,1, item)
                self.tableWidget.setRowHeight(i,22)
                i+=1


    def setComboBoxValue(self, combo, value, param):
        items = [combo.itemData(i).toPyObject() for i in range(combo.count())]
        idx = 0
        for item in items:
            if item and value:
                if item.alg == value.alg and item.param == value.param:
                    combo.setCurrentIndex(idx)
                    return
            idx += 1
        if combo.isEditable():
            value = self.model.getValueFromAlgorithmAndParameter(value)
            if value:
                combo.setEditText(str(value))
        elif isinstance(param, ParameterSelection):
            value = self.model.getValueFromAlgorithmAndParameter(value)
            combo.setCurrentIndex(int(value))
        elif isinstance(param, ParameterBoolean):
            value = self.model.getValueFromAlgorithmAndParameter(value) == str(True)
            if value:
                combo.setCurrentIndex(0)
            else:
                combo.setCurrentIndex(1)


    def setPreviousValues(self):
        if self.algIndex is not None:
            for name, value in self.model.algParameters[self.algIndex].items():
                widget = self.valueItems[name]
                param = self.alg.getParameterFromName(name)
                if isinstance(param, (ParameterRaster, ParameterVector,
                                      ParameterTable, ParameterTableField,
                                      ParameterSelection, ParameterNumber,
                                      ParameterString,ParameterBoolean)):
                    self.setComboBoxValue(widget, value, param)
                elif isinstance(param, ParameterFixedTable):
                    pass
                elif isinstance(param, ParameterMultipleInput):
                    pass
                else:
                    pass

            for out in self.alg.outputs:
                if not out.hidden:
                    value = self.model.algOutputs[self.algIndex][out.name]
                    if value is not None:
                        widget = self.valueItems[out.name].setText(unicode(value))

            #TODO


    def setParamValues(self):
        self.params = {}
        self.values = {}
        self.outputs = {}

        params = self.alg.parameters
        outputs = self.alg.outputs
        for param in params:
            if param.hidden:
                continue
            if not self.setParamValue(param, self.valueItems[param.name]):
                return False
        for output in outputs:
            if output.hidden:
                continue
            name= unicode(self.valueItems[output.name].text())
            if name.strip()!="" and name != ModelerParametersDialog.ENTER_NAME:
                self.outputs[output.name]=name
            else:
                self.outputs[output.name] = None
        return True


    def setParamValueLayerOrTable(self, param, widget):
        idx = widget.findText(widget.currentText())
        if idx < 0:
            name =  self.getSafeNameForHarcodedParameter(param)
            value = AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, name)
            self.params[param.name] = value
            s = str(widget.currentText())
            self.values[name] = s
        else:
            value = widget.itemData(widget.currentIndex()).toPyObject()
            self.params[param.name] = value
        return True



        if widget.currentIndex() < 0:
            return False
        value = widget.itemData(widget.currentIndex()).toPyObject()
        self.params[param.name] = value
        return True

    def setParamBooleanValue(self, param, widget):
        if widget.currentIndex() < 2:
            name =  self.getSafeNameForHarcodedParameter(param)
            value = AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, name)
            self.params[param.name] = value
            self.values[name] = str(widget.currentIndex() == 0)
        else:
            value = widget.itemData(widget.currentIndex()).toPyObject()
            self.params[param.name] = value
        return True

    def setParamTableFieldValue(self, param, widget):
        idx = widget.findText(widget.currentText())
        if idx < 0:
            name =  self.getSafeNameForHarcodedParameter(param)
            value = AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, name)
            self.params[param.name] = value
            value = str(widget.currentText()).strip()
            if value == "":
                return False
            else:
                self.values[name] = str(widget.currentText())
                return True
        else:
            value = widget.itemData(widget.currentIndex()).toPyObject()
            self.params[param.name] = value
        return True

    def setParamStringValue(self, param, widget):
        idx = widget.findText(widget.currentText())
        if idx < 0:
            name =  self.getSafeNameForHarcodedParameter(param)
            value = AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, name)
            self.params[param.name] = value
            self.values[name] = str(widget.currentText())
        else:
            value = widget.itemData(widget.currentIndex()).toPyObject()
            self.params[param.name] = value
        return True

    def setParamFileValue(self, param, widget):
        idx = widget.findText(widget.currentText())
        if idx < 0:
            name = self.getSafeNameForHarcodedParameter(param)
            value = AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, name)
            self.params[param.name] = value
            s = str(widget.currentText())
            self.values[name] = s
        else:
            value = widget.itemData(widget.currentIndex()).toPyObject()
            self.params[param.name] = value
        return True

    def setParamNumberValue(self, param, widget):
        idx = widget.findText(widget.currentText())
        if idx < 0:
            name =  self.getSafeNameForHarcodedParameter(param)
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

    def setParamExtentValue(self, param, widget):
        idx = widget.findText(widget.currentText())
        if idx < 0:
            name =  self.getSafeNameForHarcodedParameter(param)
            value = AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, name)
            self.params[param.name] = value
            s = str(widget.currentText())
            try:
                tokens = s.split(",")
                if len(tokens) != 4:
                    return False
                for token in tokens:
                    float(token)
            except:
                return False
            self.values[name] = s
        else:
            value = widget.itemData(widget.currentIndex()).toPyObject()
            self.params[param.name] = value
        return True

    def setParamValue(self, param, widget):
        if isinstance(param, (ParameterRaster, ParameterVector, ParameterTable)):
            return self.setParamValueLayerOrTable(param, widget)
        elif isinstance(param, ParameterBoolean):
            return self.setParamBooleanValue(param, widget)
        elif isinstance(param, ParameterString):
            return self.setParamStringValue(param, widget)
        elif isinstance(param, ParameterNumber):
            return self.setParamNumberValue(param, widget)
        elif isinstance(param, ParameterExtent):
            return self.setParamExtentValue(param, widget)
        elif isinstance(param, ParameterFile):
            return self.setParamFileValue(param, widget)
        elif isinstance(param, ParameterSelection):
            name =  self.getSafeNameForHarcodedParameter(param)
            value = AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, name)
            self.params[param.name] = value
            self.values[name] = str(widget.currentIndex())
            return True
        elif isinstance(param, ParameterRange):
            name =  self.getSafeNameForHarcodedParameter(param)
            value = AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, name)
            self.params[param.name] = value
            self.values[name] = str(widget.getValue())
            return True
        elif isinstance(param, ParameterFixedTable):
            name =  self.getSafeNameForHarcodedParameter(param)
            value = AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, name)
            self.params[param.name] = value
            self.values[name] = ParameterFixedTable.tableToString(widget.table)
            return True
        elif isinstance(param, ParameterTableField):
            return self.setParamTableFieldValue(param, widget)
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
            name =  self.getSafeNameForHarcodedParameter(param)
            value = AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, name)
            self.params[param.name] = value
            self.values[name] = ";".join(values)
            return True
        else:
            name =  self.getSafeNameForHarcodedParameter(param)
            value = AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, name)
            self.params[param.name] = value
            self.values[name] = str(widget.getText())
            return True

    def getSafeNameForHarcodedParameter(self, param):
        if self.algIndex is None:
            return "HARDCODEDPARAMVALUE_" + param.name + "_" + str(len(self.model.algs))
        else:
            return "HARDCODEDPARAMVALUE_" + param.name + "_" + str(self.algIndex)


    def okPressed(self):
        if self.setParamValues():
            self.close()
        else:
            QMessageBox.critical(self, "Unable to add algorithm", "Wrong or missing parameter values")
            self.params = None


    def cancelPressed(self):
        self.params = None
        self.close()












