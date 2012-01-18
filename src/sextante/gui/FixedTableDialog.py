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
        self.table = table
        self.ui = Ui_FixedTableDialog()
        self.ui.setupUi(self)
        self.table = None

class Ui_FixedTableDialog(object):
    def setupUi(self, dialog):
        self.dialog = dialog
        dialog.setObjectName(_fromUtf8("Dialog"))
        dialog.resize(600, 350)
        dialog.setWindowTitle("Fixed Table")
        self.buttonBox = QtGui.QDialogButtonBox(dialog)
        self.buttonBox.setGeometry(QtCore.QRect(490, 10, 81, 61))
        self.buttonBox.setOrientation(QtCore.Qt.Vertical)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Ok)
        self.buttonBox.setObjectName(_fromUtf8("buttonBox"))
        self.table = QtGui.QTableWidget(dialog)
        self.table.setGeometry(QtCore.QRect(10, 10, 470, 300))
        self.table.setObjectName(_fromUtf8("table"))
        self.table.setColumnCount(len(self.dialog.param.cols))
        for i in range(len(self.dialog.param.cols)):
            self.table.setColumnWidth(i,380 / len(self.dialog.param.cols))
            self.table.setHorizontalHeaderItem(i, QtGui.QTableWidgetItem(self.dialog.param.cols[i]))
        self.table.setRowCount(len(self.dialog.table))
        for i in range(len(self.dialog.table)):
            self.table.setRowHeight(i,22)
        self.table.verticalHeader().setVisible(False)
        self.addRowButton = QtGui.QPushButton(dialog)
        self.addRowButton.setGeometry(QtCore.QRect(490, 290, 81, 23))
        self.addRowButton.setObjectName(_fromUtf8("addRowButton"))
        self.addRowButton.setText("Add row")
        self.setTableContent()
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("accepted()")), self.accept)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("rejected()")), self.reject)
        QObject.connect(self.addRowButton, QtCore.SIGNAL(_fromUtf8("clicked()")), self.addRow)
        QtCore.QMetaObject.connectSlotsByName(dialog)

    def setTableContent(self):
        for i in range(len(self.dialog.table)):
            for j in range(len(self.dialog.table[0])):
                self.table.setItem(i,j,QtGui.QTableWidgetItem(self.dialog.table[i][j]))

    def accept(self):
        self.dialog.table = []
        for i in range(self.table.rowCount()):
            self.dialog.table.append(list())
            for j in range(self.table.columnCount()):
                self.dialog.table[i].append(str(self.table.item(i,j).text()))
        self.dialog.close()

    def reject(self):
        self.dialog.table = None
        self.dialog.close()

    def addRow(self):
        self.table.setRowCount(self.table.rowCount()+1)
        self.table.setRowHeight(self.table.rowCount()-1, 22)
        for i in range(self.table.columnCount()):
            self.table.setItem(self.table.rowCount()-1,i,QtGui.QTableWidgetItem("0"))
