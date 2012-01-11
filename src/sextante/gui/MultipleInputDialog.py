# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'D:\projects\sextante\workspaces\qgis-plugin\sextante\src\sextante\gui\MultipleSelectionDialog.ui'
#
# Created: Wed Jan 11 17:48:49 2012
#      by: PyQt4 UI code generator 4.9
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s


class MultipleInputDialog(QtGui.QDialog):
    def __init__(self, options, selectedoptions):
        self.options = options
        self.selectedoptions = selectedoptions
        QtGui.QDialog.__init__(self)
        self.setModal(True)
        self.ui = Ui_MultipleInputDialog()
        self.ui.setupUi(self)

class Ui_MultipleInputDialog(object):
    def setupUi(self, dialog):
        self.dialog = dialog
        dialog.setObjectName(_fromUtf8("Dialog"))
        dialog.resize(381, 320)
        dialog.setWindowTitle("Multiple selection")
        self.buttonBox = QtGui.QDialogButtonBox(dialog)
        self.buttonBox.setGeometry(QtCore.QRect(290, 10, 81, 61))
        self.buttonBox.setOrientation(QtCore.Qt.Vertical)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Ok)
        self.buttonBox.setObjectName(_fromUtf8("buttonBox"))
        self.table = QtGui.QTableWidget(dialog)
        self.table.setGeometry(QtCore.QRect(10, 10, 271, 301))
        self.table.setObjectName(_fromUtf8("table"))
        self.table.setColumnCount(1)
        self.table.verticalHeader().setVisible(True)
        self.table.horizontalHeader().setVisible(True)
        self.selectAllButton = QtGui.QPushButton(dialog)
        self.selectAllButton.setGeometry(QtCore.QRect(290, 290, 81, 23))
        self.selectAllButton.setObjectName(_fromUtf8("selectAllButton"))
        self.selectAllButton.setText("(de)Select all")
        self.setTableContent()
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("accepted()")), self.accept)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("rejected()")), self.reject)
        QtCore.QObject.connect(self.selectAllButton, QtCore.SIGNAL(_fromUtf8("clicked()")), self.selectAll)
        QtCore.QMetaObject.connectSlotsByName(dialog)

    def setTableContent(self):
        for i in range(len(self.dialog.options)):
            item = QtGui.QCheckBox()
            item.setText(self.dialog.options[i])
            self.table.setCellWidget(i,0, item)


    def accept(self):
        self.dialog.selectedoptions = []
        for i in range(len(self.dialog.options)):
            widget = self.table.cellWidget(i, 0)
            if widget.isChecked():
                self.dialog.selectedoptions.append(i)


    def reject(self):
        self.dialog.selectedoptions = None
        self.dialog.close()

    def selectAll(self):
        checked = False
        for i in range(len(self.dialog.options)):
            widget = self.table.cellWidget(i, 0)
            if not widget.isChecked():
                checked = True
                break
        for i in range(len(self.dialog.options)):
            widget = self.table.cellWidget(i, 0)
            widget.setChecked(checked)