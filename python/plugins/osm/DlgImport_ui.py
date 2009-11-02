# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'ui_files/DlgImport.ui'
#
# Created: Tue Jul 14 14:44:27 2009
#      by: PyQt4 UI code generator 4.4.4
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

class Ui_DlgImport(object):
    def setupUi(self, DlgImport):
        DlgImport.setObjectName("DlgImport")
        DlgImport.setWindowModality(QtCore.Qt.ApplicationModal)
        DlgImport.resize(248, 228)
        DlgImport.setModal(True)
        self.vboxlayout = QtGui.QVBoxLayout(DlgImport)
        self.vboxlayout.setObjectName("vboxlayout")
        self.label = QtGui.QLabel(DlgImport)
        self.label.setWordWrap(True)
        self.label.setObjectName("label")
        self.vboxlayout.addWidget(self.label)
        spacerItem = QtGui.QSpacerItem(20, 29, QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Expanding)
        self.vboxlayout.addItem(spacerItem)
        self.hboxlayout = QtGui.QHBoxLayout()
        self.hboxlayout.setObjectName("hboxlayout")
        self.label_2 = QtGui.QLabel(DlgImport)
        self.label_2.setObjectName("label_2")
        self.hboxlayout.addWidget(self.label_2)
        self.cboLayer = QtGui.QComboBox(DlgImport)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.cboLayer.sizePolicy().hasHeightForWidth())
        self.cboLayer.setSizePolicy(sizePolicy)
        self.cboLayer.setObjectName("cboLayer")
        self.hboxlayout.addWidget(self.cboLayer)
        self.vboxlayout.addLayout(self.hboxlayout)
        self.chkOnlySelection = QtGui.QCheckBox(DlgImport)
        self.chkOnlySelection.setObjectName("chkOnlySelection")
        self.vboxlayout.addWidget(self.chkOnlySelection)
        spacerItem1 = QtGui.QSpacerItem(20, 40, QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Expanding)
        self.vboxlayout.addItem(spacerItem1)
        self.buttonBox = QtGui.QDialogButtonBox(DlgImport)
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.NoButton|QtGui.QDialogButtonBox.Ok)
        self.buttonBox.setObjectName("buttonBox")
        self.vboxlayout.addWidget(self.buttonBox)

        self.retranslateUi(DlgImport)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL("rejected()"), DlgImport.reject)
        QtCore.QMetaObject.connectSlotsByName(DlgImport)
        DlgImport.setTabOrder(self.cboLayer, self.chkOnlySelection)
        DlgImport.setTabOrder(self.chkOnlySelection, self.buttonBox)

    def retranslateUi(self, DlgImport):
        DlgImport.setWindowTitle(QtGui.QApplication.translate("DlgImport", "Import data to OSM", None, QtGui.QApplication.UnicodeUTF8))
        self.label.setText(QtGui.QApplication.translate("DlgImport", "In this dialog you can import a layer loaded in QGIS into active OSM data.", None, QtGui.QApplication.UnicodeUTF8))
        self.label_2.setText(QtGui.QApplication.translate("DlgImport", "Layer", None, QtGui.QApplication.UnicodeUTF8))
        self.chkOnlySelection.setText(QtGui.QApplication.translate("DlgImport", "Import only current selection", None, QtGui.QApplication.UnicodeUTF8))

