from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4 import QtCore, QtGui
from sextante.outputs.OutputRaster import OutputRaster
from sextante.outputs.OutputVector import OutputVector
from sextante.gui.RenderingStyles import RenderingStyles
from sextante.gui.RenderingStyleFilePanel import RenderingStyleFilePanel

class EditRenderingStylesDialog(QtGui.QDialog):
    def __init__(self, alg):
        QtGui.QDialog.__init__(self)
        self.setModal(True)
        self.alg = alg
        self.setupUi()

    def setupUi(self):
        self.valueItems = {}
        self.dependentItems = {}
        self.resize(650, 450)
        self.buttonBox = QtGui.QDialogButtonBox()
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Cancel | QtGui.QDialogButtonBox.Ok)
        self.tableWidget = QtGui.QTableWidget()
        self.tableWidget.setSelectionMode(QtGui.QAbstractItemView.NoSelection)
        self.tableWidget.setColumnCount(2)
        self.tableWidget.setColumnWidth(0, 300)
        self.tableWidget.setColumnWidth(1, 300)
        self.tableWidget.setHorizontalHeaderItem(0, QtGui.QTableWidgetItem("Output"))
        self.tableWidget.setHorizontalHeaderItem(1, QtGui.QTableWidgetItem("Style"))
        self.tableWidget.verticalHeader().setVisible(False)
        self.tableWidget.horizontalHeader().setResizeMode(QtGui.QHeaderView.Stretch)
        self.setTableContent()
        self.setWindowTitle(self.alg.name)
        self.verticalLayout = QtGui.QVBoxLayout()
        self.verticalLayout.setSpacing(2)
        self.verticalLayout.setMargin(0)
        self.verticalLayout.addWidget(self.tableWidget)
        self.verticalLayout.addWidget(self.buttonBox)
        self.setLayout(self.verticalLayout)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL("accepted()"), self.okPressed)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL("rejected()"), self.cancelPressed)
        QtCore.QMetaObject.connectSlotsByName(self)

    def setTableContent(self):
        numOutputs = 0
        for output in self.alg.outputs:
            if isinstance(output, (OutputVector, OutputRaster)):
                if not output.hidden:
                    numOutputs += 1
        self.tableWidget.setRowCount(numOutputs)

        i = 0
        for output in self.alg.outputs:
            if isinstance(output, (OutputVector, OutputRaster)):
                if not output.hidden:
                    item = QtGui.QTableWidgetItem(output.description + "<" + output.__module__.split(".")[-1] + ">")
                    item.setFlags(QtCore.Qt.ItemIsEnabled)
                    self.tableWidget.setItem(i, 0, item)
                    item = RenderingStyleFilePanel()
                    style = RenderingStyles.getStyle(self.alg.commandLineName(), output.name)
                    if style:
                        item.setText(str(style))
                    self.valueItems[output.name] = item
                    self.tableWidget.setCellWidget(i, 1, item)
                    self.tableWidget.setRowHeight(i, 22)
            i += 1

    def okPressed(self):
        styles = {}
        for key in self.valueItems.keys():
            styles[key] = str(self.valueItems[key].getValue())
        RenderingStyles.addAlgStylesAndSave(self.alg.commandLineName(), styles)
        self.close()

    def cancelPressed(self):
        self.close()
