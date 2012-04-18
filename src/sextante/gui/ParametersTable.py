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
import os

class ParametersTable(QtGui.QWidget):

    def __init__(self, alg, paramDialog):
        super(ParametersTable, self).__init__(None)
        self.alg = alg;
        self.paramDialog = paramDialog
        self.valueItems = {}
        self.dependentItems = {}
        self.iterateButtons = {}
        self.initGUI()


    def initGUI(self):
        self.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)
        self.verticalLayout = QtGui.QVBoxLayout()
        self.verticalLayout.setSpacing(5)
        self.verticalLayout.setMargin(20)

        for param in self.alg.parameters:
            label = QtGui.QLabel(param.description)
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
                self.verticalLayout.addWidget(label)
                self.verticalLayout.addWidget(widget)
            self.verticalLayout.addWidget(label)
            self.verticalLayout.addWidget(widget)

        for output in self.alg.outputs:
            if output.hidden:
                continue
            label = QtGui.QLabel(output.description)
            widget = OutputSelectionPanel(output,self.alg)
            self.verticalLayout.addWidget(label)
            self.verticalLayout.addWidget(widget)
            self.valueItems[output.name] = widget

        self.verticalLayout.addStretch(1000)
        self.setLayout(self.verticalLayout)

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
                fields = self.getFields(layers[0])
                for i in fields:
                    item.addItem(fields[i].name())
        elif isinstance(param, ParameterSelection):
            item = QtGui.QComboBox()
            item.addItems(param.options)
            item.setCurrentIndex(param.default)
        elif isinstance(param, ParameterFixedTable):
            item = FixedTablePanel(param)
        elif isinstance(param, ParameterRange):
            item = RangePanel(param)
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
            item = ExtentSelectionPanel(self.paramDialog, param.default)
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
            fields = self.getFields(layer)
            for i in fields:
                widget.addItem(fields[i].name())

    def getFields(self, layer):
        return layer.dataProvider().fields()

    def somethingDependsOnThisParameter(self, parent):
        for param in self.alg.parameters:
            if isinstance(param, ParameterTableField):
                if param.parent == parent.name:
                    return True
        return False