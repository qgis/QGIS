# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'settings.ui'
#
# Created: Sun Oct 14 23:50:57 2012
#      by: PyQt4 UI code generator 4.9.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_SettingsDialog(object):
    def setupUi(self, SettingsDialog):
        SettingsDialog.setObjectName(_fromUtf8("SettingsDialog"))
        SettingsDialog.setWindowModality(QtCore.Qt.NonModal)
        SettingsDialog.resize(472, 388)
        SettingsDialog.setModal(True)
        self.gridLayout = QtGui.QGridLayout(SettingsDialog)
        self.gridLayout.setObjectName(_fromUtf8("gridLayout"))
        self.buttonBox = QtGui.QDialogButtonBox(SettingsDialog)
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Ok)
        self.buttonBox.setObjectName(_fromUtf8("buttonBox"))
        self.gridLayout.addWidget(self.buttonBox, 1, 0, 1, 1)
        self.tabWidget = QtGui.QTabWidget(SettingsDialog)
        self.tabWidget.setFocusPolicy(QtCore.Qt.NoFocus)
        self.tabWidget.setObjectName(_fromUtf8("tabWidget"))
        self.tabGeneral = QtGui.QWidget()
        self.tabGeneral.setObjectName(_fromUtf8("tabGeneral"))
        self.gridLayout_5 = QtGui.QGridLayout(self.tabGeneral)
        self.gridLayout_5.setObjectName(_fromUtf8("gridLayout_5"))
        self.preloadAPI = QtGui.QCheckBox(self.tabGeneral)
        self.preloadAPI.setObjectName(_fromUtf8("preloadAPI"))
        self.gridLayout_5.addWidget(self.preloadAPI, 1, 0, 1, 1)
        self.horizontalLayout_2 = QtGui.QHBoxLayout()
        self.horizontalLayout_2.setObjectName(_fromUtf8("horizontalLayout_2"))
        self.label_2 = QtGui.QLabel(self.tabGeneral)
        self.label_2.setObjectName(_fromUtf8("label_2"))
        self.horizontalLayout_2.addWidget(self.label_2)
        self.lineEdit = QtGui.QLineEdit(self.tabGeneral)
        self.lineEdit.setObjectName(_fromUtf8("lineEdit"))
        self.horizontalLayout_2.addWidget(self.lineEdit)
        self.browseButton = QtGui.QPushButton(self.tabGeneral)
        self.browseButton.setObjectName(_fromUtf8("browseButton"))
        self.horizontalLayout_2.addWidget(self.browseButton)
        self.gridLayout_5.addLayout(self.horizontalLayout_2, 2, 0, 1, 1)
        self.gridLayout_4 = QtGui.QGridLayout()
        self.gridLayout_4.setObjectName(_fromUtf8("gridLayout_4"))
        self.spinBox = QtGui.QSpinBox(self.tabGeneral)
        self.spinBox.setMinimumSize(QtCore.QSize(51, 26))
        self.spinBox.setMaximumSize(QtCore.QSize(51, 26))
        self.spinBox.setMinimum(6)
        self.spinBox.setMaximum(15)
        self.spinBox.setProperty("value", 10)
        self.spinBox.setObjectName(_fromUtf8("spinBox"))
        self.gridLayout_4.addWidget(self.spinBox, 0, 3, 1, 1)
        self.label = QtGui.QLabel(self.tabGeneral)
        self.label.setObjectName(_fromUtf8("label"))
        self.gridLayout_4.addWidget(self.label, 0, 0, 1, 1)
        self.fontComboBox = QtGui.QFontComboBox(self.tabGeneral)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.fontComboBox.sizePolicy().hasHeightForWidth())
        self.fontComboBox.setSizePolicy(sizePolicy)
        self.fontComboBox.setObjectName(_fromUtf8("fontComboBox"))
        self.gridLayout_4.addWidget(self.fontComboBox, 0, 1, 1, 1)
        self.label_3 = QtGui.QLabel(self.tabGeneral)
        self.label_3.setObjectName(_fromUtf8("label_3"))
        self.gridLayout_4.addWidget(self.label_3, 0, 2, 1, 1)
        self.gridLayout_5.addLayout(self.gridLayout_4, 0, 0, 1, 1)
        self.gridLayout_3 = QtGui.QGridLayout()
        self.gridLayout_3.setObjectName(_fromUtf8("gridLayout_3"))
        self.tableWidget = QtGui.QTableWidget(self.tabGeneral)
        self.tableWidget.setEditTriggers(QtGui.QAbstractItemView.NoEditTriggers)
        self.tableWidget.setTabKeyNavigation(False)
        self.tableWidget.setProperty("showDropIndicator", False)
        self.tableWidget.setDragDropOverwriteMode(False)
        self.tableWidget.setVerticalScrollMode(QtGui.QAbstractItemView.ScrollPerItem)
        self.tableWidget.setHorizontalScrollMode(QtGui.QAbstractItemView.ScrollPerPixel)
        self.tableWidget.setRowCount(0)
        self.tableWidget.setObjectName(_fromUtf8("tableWidget"))
        self.tableWidget.setColumnCount(0)
        self.gridLayout_3.addWidget(self.tableWidget, 0, 0, 2, 1)
        self.gridLayout_2 = QtGui.QGridLayout()
        self.gridLayout_2.setObjectName(_fromUtf8("gridLayout_2"))
        self.addAPIpath = QtGui.QPushButton(self.tabGeneral)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Minimum)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.addAPIpath.sizePolicy().hasHeightForWidth())
        self.addAPIpath.setSizePolicy(sizePolicy)
        self.addAPIpath.setMinimumSize(QtCore.QSize(32, 32))
        self.addAPIpath.setMaximumSize(QtCore.QSize(32, 32))
        self.addAPIpath.setText(_fromUtf8(""))
        self.addAPIpath.setObjectName(_fromUtf8("addAPIpath"))
        self.gridLayout_2.addWidget(self.addAPIpath, 0, 0, 1, 1)
        self.removeAPIpath = QtGui.QPushButton(self.tabGeneral)
        self.removeAPIpath.setMinimumSize(QtCore.QSize(32, 32))
        self.removeAPIpath.setMaximumSize(QtCore.QSize(32, 32))
        self.removeAPIpath.setText(_fromUtf8(""))
        self.removeAPIpath.setObjectName(_fromUtf8("removeAPIpath"))
        self.gridLayout_2.addWidget(self.removeAPIpath, 1, 0, 1, 1)
        self.gridLayout_3.addLayout(self.gridLayout_2, 0, 1, 1, 1)
        spacerItem = QtGui.QSpacerItem(20, 40, QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Expanding)
        self.gridLayout_3.addItem(spacerItem, 1, 1, 1, 1)
        self.gridLayout_5.addLayout(self.gridLayout_3, 3, 0, 1, 1)
        self.tabWidget.addTab(self.tabGeneral, _fromUtf8(""))
        self.tabAbout = QtGui.QWidget()
        self.tabAbout.setObjectName(_fromUtf8("tabAbout"))
        self.gridLayout_6 = QtGui.QGridLayout(self.tabAbout)
        self.gridLayout_6.setObjectName(_fromUtf8("gridLayout_6"))
        self.textEdit = QtGui.QTextEdit(self.tabAbout)
        self.textEdit.setReadOnly(True)
        self.textEdit.setObjectName(_fromUtf8("textEdit"))
        self.gridLayout_6.addWidget(self.textEdit, 0, 0, 1, 1)
        self.tabWidget.addTab(self.tabAbout, _fromUtf8(""))
        self.gridLayout.addWidget(self.tabWidget, 0, 0, 1, 1)

        self.retranslateUi(SettingsDialog)
        self.tabWidget.setCurrentIndex(0)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("accepted()")), SettingsDialog.accept)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("rejected()")), SettingsDialog.reject)
        QtCore.QMetaObject.connectSlotsByName(SettingsDialog)

    def retranslateUi(self, SettingsDialog):
        self.preloadAPI.setText(QtGui.QApplication.translate("SettingsDialog", "Preload API files", None, QtGui.QApplication.UnicodeUTF8))
        self.label_2.setText(QtGui.QApplication.translate("SettingsDialog", "API file", None, QtGui.QApplication.UnicodeUTF8))
        self.browseButton.setText(QtGui.QApplication.translate("SettingsDialog", "Browse", None, QtGui.QApplication.UnicodeUTF8))
        self.label.setText(QtGui.QApplication.translate("SettingsDialog", "Font", None, QtGui.QApplication.UnicodeUTF8))
        self.label_3.setText(QtGui.QApplication.translate("SettingsDialog", "Size", None, QtGui.QApplication.UnicodeUTF8))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.tabGeneral), QtGui.QApplication.translate("SettingsDialog", "Tab 1", None, QtGui.QApplication.UnicodeUTF8))
        self.textEdit.setHtml(QtGui.QApplication.translate("SettingsDialog", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:\'Aurulent Sans\'; font-size:9pt; font-weight:400; font-style:normal;\">\n"
"<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:14pt; font-weight:600;\">Python Console for QGIS</span></p>\n"
"<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:10pt;\">Developed by Salvatore Larosa</span></p>\n"
"<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:10pt;\">---------------------------</span></p>\n"
"<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:10pt;\">This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or  (at your option) any later version.</span></p></body></html>", None, QtGui.QApplication.UnicodeUTF8))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.tabAbout), QtGui.QApplication.translate("SettingsDialog", "Tab 2", None, QtGui.QApplication.UnicodeUTF8))

