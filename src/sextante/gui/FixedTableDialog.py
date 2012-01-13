from PyQt4 import QtCore, QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s


class FixedTableDialog(QtGui.QDialog):
    def __init__(self, param, table):
        QtGui.QDialog.__init__(self)
        self.setModal(True)
        self.param = param
        if table != None:
            self.table = table
        else:
            self.table = []
            for i in range(param.numRows):
                self.table.append(list())
                for j in range(len(param.cols)):
                    self.table[i].append("0")
        self.ui = Ui_FixedTableDialog()
        self.ui.setupUi(self)
        self.table = None

class Ui_FixedTableDialog(object):
    def setupUi(self, dialog):
        self.dialog = dialog
        dialog.setObjectName(_fromUtf8("Dialog"))
        dialog.resize(400, 350)
        dialog.setWindowTitle("Fixed Table")
        self.buttonBox = QtGui.QDialogButtonBox(dialog)
        self.buttonBox.setGeometry(QtCore.QRect(290, 10, 81, 61))
        self.buttonBox.setOrientation(QtCore.Qt.Vertical)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Ok)
        self.buttonBox.setObjectName(_fromUtf8("buttonBox"))
        self.table = QtGui.QTableWidget(dialog)
        self.table.setGeometry(QtCore.QRect(10, 10, 270, 300))
        self.table.setObjectName(_fromUtf8("table"))
        self.table.setColumnCount(len(self.dialog.param.cols))
        for i in range(len(self.dialog.param.cols)):
            self.table.setColumnWidth(i,380 / len(self.dialog.param.cols))
            self.table.setHorizontalHeaderItem(i, QtGui.QTableWidgetItem(self.dialog.param.cols[i]))
        self.table.setRowCount(self.dialog.param.numRows)
        self.table.verticalHeader().setVisible(False)
        self.addRow = QtGui.QPushButton(dialog)
        self.addRow.setGeometry(QtCore.QRect(290, 290, 81, 23))
        self.addRow.setObjectName(_fromUtf8("addRow"))
        self.addRow.setText("Add row")
        self.setTableContent()
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("accepted()")), self.accept)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("rejected()")), self.reject)
        QtCore.QObject.connect(self.addRow, QtCore.SIGNAL(_fromUtf8("clicked()")), self.addRow)
        QtCore.QMetaObject.connectSlotsByName(dialog)

    def setTableContent(self):
        for i in range(len(self.dialog.table)):
            for j in range(len(self.dialog.table)):
                self.table.setItem(i,j,QtGui.QTableWidgetItem(self.dialog.table[i][j]))

    def accept(self):
        self.dialog.selectedoptions = []
        for i in range(len(self.dialog.options)):
            widget = self.table.cellWidget(i, 0)
            if widget.isChecked():
                self.dialog.selectedoptions.append(i)
        self.dialog.close()

    def reject(self):
        self.dialog.selectedoptions = None
        self.dialog.close()

    def addRow(self):
        self.table.addRow()