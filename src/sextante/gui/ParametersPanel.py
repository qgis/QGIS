import os
import locale
from PyQt4 import QtCore, QtGui
from sextante.gui.OutputSelectionPanel import OutputSelectionPanel
from sextante.core.QGisLayers import QGisLayers
from sextante.parameters.ParameterRaster import ParameterRaster
from sextante.gui.InputLayerSelectorPanel import InputLayerSelectorPanel
from sextante.parameters.ParameterVector import ParameterVector
from sextante.parameters.ParameterTable import ParameterTable
from sextante.parameters.ParameterBoolean import ParameterBoolean
from sextante.parameters.ParameterTableField import ParameterTableField
from sextante.parameters.ParameterSelection import ParameterSelection
from sextante.parameters.ParameterFixedTable import ParameterFixedTable
from sextante.gui.FixedTablePanel import FixedTablePanel
from sextante.parameters.ParameterRange import ParameterRange
from sextante.gui.RangePanel import RangePanel
from sextante.parameters.ParameterMultipleInput import ParameterMultipleInput
from sextante.parameters.ParameterNumber import ParameterNumber
from sextante.gui.MultipleInputPanel import MultipleInputPanel
from sextante.gui.NumberInputPanel import NumberInputPanel
from sextante.gui.ExtentSelectionPanel import ExtentSelectionPanel
from sextante.parameters.ParameterExtent import ParameterExtent
from sextante.core.SextanteConfig import SextanteConfig
from sextante.parameters.ParameterFile import ParameterFile
from sextante.gui.FileSelectionPanel import FileSelectionPanel
from sextante.parameters.ParameterCrs import ParameterCrs
from sextante.gui.CrsSelectionPanel import CrsSelectionPanel
from sextante.outputs.OutputHTML import OutputHTML
from sextante.outputs.OutputRaster import OutputRaster
from sextante.outputs.OutputTable import OutputTable
from sextante.outputs.OutputVector import OutputVector

class ParametersPanel(QtGui.QWidget):

    NOT_SELECTED = "[Not selected]"

    def __init__(self, parent, alg):
        super(ParametersPanel, self).__init__(None)
        self.parent = parent
        self.alg = alg;
        self.valueItems = {}
        self.labels = {}
        self.widgets = {}
        self.checkBoxes = {}
        self.dependentItems = {}
        self.iterateButtons = {}
        self.showAdvanced = False
        self.initGUI()

    def initGUI(self):
        tooltips = self.alg.getParameterDescriptions()
        tableLike = SextanteConfig.getSetting(SextanteConfig.TABLE_LIKE_PARAM_PANEL)
        if tableLike:
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
            self.verticalLayout = QtGui.QVBoxLayout()
            self.verticalLayout.setSpacing(0)
            self.verticalLayout.setMargin(0)
            self.verticalLayout.addWidget(self.tableWidget)
            self.setLayout(self.verticalLayout)
        else:
            self.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)
            self.verticalLayout = QtGui.QVBoxLayout()
            self.verticalLayout.setSpacing(5)
            self.verticalLayout.setMargin(20)
            for param in self.alg.parameters:
                if param.isAdvanced:
                    self.advancedButton = QtGui.QPushButton()
                    self.advancedButton.setText("Show advanced parameters")
                    self.advancedButton.setMaximumWidth(150)
                    QtCore.QObject.connect(self.advancedButton, QtCore.SIGNAL("clicked()"), self.showAdvancedParametersClicked)
                    self.verticalLayout.addWidget(self.advancedButton)
                    break
            for param in self.alg.parameters:
                if param.hidden:
                    continue
                desc = param.description
                if isinstance(param, ParameterExtent):
                    desc += "(xmin, xmax, ymin, ymax)"
                label = QtGui.QLabel(desc)
                self.labels[param.name] = label
                widget = self.getWidgetFromParameter(param)
                self.valueItems[param.name] = widget
                if isinstance(param, ParameterVector):
                    layout = QtGui.QHBoxLayout()
                    layout.setSpacing(2)
                    layout.setMargin(0)
                    layout.addWidget(widget)
                    button = QtGui.QToolButton()
                    icon = QtGui.QIcon(os.path.dirname(__file__) + "/../images/iterate.png")
                    button.setIcon(icon)
                    button.setToolTip("Iterate over this layer")
                    button.setCheckable(True)
                    button.setMaximumWidth(30)
                    button.setMaximumHeight(30)
                    layout.addWidget(button)
                    self.iterateButtons[param.name] = button
                    QtCore.QObject.connect(button, QtCore.SIGNAL("toggled(bool)"), self.buttonToggled)
                    widget = QtGui.QWidget()
                    widget.setLayout(layout)
                if param.name in tooltips.keys():
                    tooltip = tooltips[param.name]
                else:
                    tooltip = param.description
                label.setToolTip(tooltip)
                widget.setToolTip(tooltip)
                if param.isAdvanced:
                    label.setVisible(self.showAdvanced)
                    widget.setVisible(self.showAdvanced)
                    self.widgets[param.name] = widget
                self.verticalLayout.addWidget(label)
                self.verticalLayout.addWidget(widget)

            for output in self.alg.outputs:
                if output.hidden:
                    continue
                label = QtGui.QLabel(output.description)
                widget = OutputSelectionPanel(output,self.alg)
                self.verticalLayout.addWidget(label)
                self.verticalLayout.addWidget(widget)
                if isinstance(output, (OutputRaster, OutputVector, OutputTable, OutputHTML)):
                    check = QtGui.QCheckBox()
                    check.setText("Open output file after running algorithm")
                    check.setChecked(True)
                    self.verticalLayout.addWidget(check)
                    self.checkBoxes[output.name] = check
                self.valueItems[output.name] = widget

            self.verticalLayout.addStretch(1000)
            self.setLayout(self.verticalLayout)

        #=======================================================================
        # for param in self.alg.parameters:
        #    if isinstance(param, ParameterExtent):
        #        self.widgets[param.name].useMinCovering()
        #=======================================================================

    def showAdvancedParametersClicked(self):
        self.showAdvanced = not self.showAdvanced
        if self.showAdvanced:
            self.advancedButton.setText("Hide advanced parameters")
        else:
            self.advancedButton.setText("Show advanced parameters")
        for param in self.alg.parameters:
            if param.isAdvanced:
                self.labels[param.name].setVisible(self.showAdvanced)
                self.widgets[param.name].setVisible(self.showAdvanced)

    def buttonToggled(self, value):
        if value:
            sender = self.sender()
            for button in self.iterateButtons.values():
                if button is not sender:
                    button.setChecked(False)

    def getWidgetFromParameter(self, param):
        if isinstance(param, ParameterRaster):
            layers = QGisLayers.getRasterLayers()
            items = []
            if (param.optional):
                items.append((self.NOT_SELECTED, None))
            for layer in layers:
                items.append((layer.name(), layer))
            item = InputLayerSelectorPanel(items)
        elif isinstance(param, ParameterVector):
            if self.somethingDependsOnThisParameter(param):
                item = QtGui.QComboBox()
                layers = QGisLayers.getVectorLayers(param.shapetype)
                if (param.optional):
                    item.addItem(self.NOT_SELECTED, None)
                for layer in layers:
                    item.addItem(layer.name(), layer)
                item.currentIndexChanged.connect(self.updateDependentFields)
                item.name = param.name
            else:
                layers = QGisLayers.getVectorLayers(param.shapetype)
                items = []
                if (param.optional):
                    items.append((self.NOT_SELECTED, None))
                for layer in layers:
                    items.append((layer.name(), layer))
                item = InputLayerSelectorPanel(items)
        elif isinstance(param, ParameterTable):
            if self.somethingDependsOnThisParameter(param):
                item = QtGui.QComboBox()
                layers = QGisLayers.getTables()
                if (param.optional):
                    item.addItem(self.NOT_SELECTED, None)
                for layer in layers:
                    item.addItem(layer.name(), layer)
                item.currentIndexChanged.connect(self.updateDependentFields)
                item.name = param.name
            else:
                layers = QGisLayers.getTables()
                items = []
                if (param.optional):
                    items.append((self.NOT_SELECTED, None))
                for layer in layers:
                    items.append((layer.name(), layer))
                item = InputLayerSelectorPanel(items)
        elif isinstance(param, ParameterBoolean):
            item = QtGui.QComboBox()
            item.addItem("Yes")
            item.addItem("No")
            if param.default:
                item.setCurrentIndex(0)
            else:
                item.setCurrentIndex(1)
        elif isinstance(param, ParameterTableField):
            item = QtGui.QComboBox()
            if param.parent in self.dependentItems:
                items = self.dependentItems[param.parent]
            else:
                items = []
                self.dependentItems[param.parent] = items
            items.append(param.name)
            parent = self.alg.getParameterFromName(param.parent)
            if isinstance(parent, ParameterVector):
                layers = QGisLayers.getVectorLayers(parent.shapetype)
            else:
                layers = QGisLayers.getTables()
            if len(layers)>0:
                item.addItems(self.getFields(layers[0], param.datatype))
        elif isinstance(param, ParameterSelection):
            item = QtGui.QComboBox()
            item.addItems(param.options)
            item.setCurrentIndex(param.default)
        elif isinstance(param, ParameterFixedTable):
            item = FixedTablePanel(param)
        elif isinstance(param, ParameterRange):
            item = RangePanel(param)
        elif isinstance(param, ParameterFile):
            item = FileSelectionPanel(param.isFolder)
        elif isinstance(param, ParameterMultipleInput):
            if param.datatype == ParameterMultipleInput.TYPE_RASTER:
                options = QGisLayers.getRasterLayers()
            elif param.datatype == ParameterMultipleInput.TYPE_VECTOR_ANY:
                options = QGisLayers.getVectorLayers()
            else:
                options = QGisLayers.getVectorLayers(param.datatype)
            opts = []
            for opt in options:
                opts.append(opt.name())
            item = MultipleInputPanel(opts)
        elif isinstance(param, ParameterNumber):
            item = NumberInputPanel(param.default, param.isInteger)
        elif isinstance(param, ParameterExtent):
            item = ExtentSelectionPanel(self.parent, self.alg, param.default)
        elif isinstance(param, ParameterCrs):
            item = CrsSelectionPanel(param.default)
        else:
            item = QtGui.QLineEdit()
            item.setText(str(param.default))

        return item

    def updateDependentFields(self):
        sender = self.sender()
        if not isinstance(sender, QtGui.QComboBox):
            return
        if not sender.name in self.dependentItems:
            return
        layer = sender.itemData(sender.currentIndex()).toPyObject()
        children = self.dependentItems[sender.name]
        for child in children:
            widget = self.valueItems[child]
            widget.clear()
            widget.addItems(self.getFields(layer, self.alg.getParameterFromName(child).datatype))

    def getFields(self, layer, datatype):
        fieldTypes = []
        if datatype == ParameterTableField.DATA_TYPE_STRING:
            fieldTypes = [QtCore.QVariant.String]
        elif datatype == ParameterTableField.DATA_TYPE_NUMBER:
            fieldTypes = [QtCore.QVariant.Int, QtCore.QVariant.Double]

        fieldNames = []
        fieldMap = layer.pendingFields()
        if len(fieldTypes) == 0:
            for idx, field in fieldMap.iteritems():
                if not field.name() in fieldNames:
                    fieldNames.append( unicode( field.name() ) )
        else:
            for idx, field in fieldMap.iteritems():
                if field.type() in fieldTypes and not field.name() in fieldNames:
                    fieldNames.append( unicode( field.name() ) )
        return sorted( fieldNames, cmp=locale.strcoll )

    def somethingDependsOnThisParameter(self, parent):
        for param in self.alg.parameters:
            if isinstance(param, ParameterTableField):
                if param.parent == parent.name:
                    return True
        return False

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
            if param.hidden:
                continue
            item = QtGui.QTableWidgetItem(param.description)
            item.setFlags(QtCore.Qt.ItemIsEnabled)
            self.tableWidget.setItem(i,0, item)
            item = self.getWidgetFromParameter(param)
            self.valueItems[param.name] = item
            self.tableWidget.setCellWidget(i,1, item)
            self.tableWidget.setRowHeight(i,22)
            i+=1

        for output in outputs:
            if output.hidden:
                continue
            item = QtGui.QTableWidgetItem(output.description + "<" + output.__module__.split(".")[-1] + ">")
            item.setFlags(QtCore.Qt.ItemIsEnabled)
            self.tableWidget.setItem(i,0, item)
            item = OutputSelectionPanel(output,self.alg)
            self.valueItems[output.name] = item
            self.tableWidget.setCellWidget(i,1, item)
            self.tableWidget.setRowHeight(i,22)
            i+=1

