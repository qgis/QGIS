from PyQt4 import QtCore, QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *

class MultipleInputDialog(QtGui.QDialog):
    def __init__(self, options, selectedoptions):
        self.options = options
        self.selectedoptions = selectedoptions
        QtGui.QDialog.__init__(self)
        self.setModal(True)
        self.setupUi()
        self.selectedoptions = None

    def setupUi(self):
        self.resize(381, 320)
        self.setWindowTitle("Multiple selection")
        self.horizontalLayout = QtGui.QHBoxLayout(self)
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setMargin(0)
        self.buttonBox = QtGui.QDialogButtonBox()
        self.buttonBox.setOrientation(QtCore.Qt.Vertical)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Ok)
        self.table = QtGui.QTableWidget()
        self.table.setColumnCount(1)
        self.table.setColumnWidth(0,270)
        self.table.verticalHeader().setVisible(False)
        self.table.horizontalHeader().setVisible(False)
        self.table.horizontalHeader().setResizeMode(QtGui.QHeaderView.Stretch)
        self.selectAllButton = QtGui.QPushButton()
        self.selectAllButton.setText("(de)Select all")
        self.setTableContent()
        self.buttonBox.addButton(self.selectAllButton, QtGui.QDialogButtonBox.ActionRole)
        self.horizontalLayout.addWidget(self.table)
        self.horizontalLayout.addWidget(self.buttonBox)
        self.setLayout(self.horizontalLayout)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL("accepted()"), self.okPressed)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL("rejected()"), self.cancelPressed)
        QtCore.QObject.connect(self.selectAllButton, QtCore.SIGNAL("clicked()"), self.selectAll)
        QtCore.QMetaObject.connectSlotsByName(self)

    def setTableContent(self):
        self.table.setRowCount(len(self.options))
        for i in range(len(self.options)):
            item = QtGui.QCheckBox()
            item.setText(self.options[i])
            if i in self.selectedoptions:
                item.setChecked(True)
            self.table.setCellWidget(i,0, item)

    def okPressed(self):
        self.selectedoptions = []
        for i in range(len(self.options)):
            widget = self.table.cellWidget(i, 0)
            if widget.isChecked():
                self.selectedoptions.append(i)
        self.close()

    def cancelPressed(self):
        self.selectedoptions = None
        self.close()

    def selectAll(self):
        checked = False
        for i in range(len(self.options)):
            widget = self.table.cellWidget(i, 0)
            if not widget.isChecked():
                checked = True
                break
        for i in range(len(self.options)):
            widget = self.table.cellWidget(i, 0)
            widget.setChecked(checked)