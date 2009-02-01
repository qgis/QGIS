# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'frmReProject.ui'
#
# Created: Mon Nov 10 00:01:16 2008
#      by: PyQt4 UI code generator 4.3.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

class Ui_Dialog(object):
    def setupUi(self, Dialog):
        Dialog.setObjectName("Dialog")
        Dialog.setWindowModality(QtCore.Qt.WindowModal)
        Dialog.resize(QtCore.QSize(QtCore.QRect(0,0,377,473).size()).expandedTo(Dialog.minimumSizeHint()))
        Dialog.setSizeGripEnabled(True)

        self.gridlayout = QtGui.QGridLayout(Dialog)
        self.gridlayout.setObjectName("gridlayout")

        self.label_3 = QtGui.QLabel(Dialog)
        self.label_3.setObjectName("label_3")
        self.gridlayout.addWidget(self.label_3,0,0,1,2)

        self.inShape = QtGui.QComboBox(Dialog)
        self.inShape.setObjectName("inShape")
        self.gridlayout.addWidget(self.inShape,1,0,1,2)

        self.label_4 = QtGui.QLabel(Dialog)
        self.label_4.setObjectName("label_4")
        self.gridlayout.addWidget(self.label_4,2,0,1,2)

        self.inRef = QtGui.QLineEdit(Dialog)
        self.inRef.setReadOnly(True)
        self.inRef.setObjectName("inRef")
        self.gridlayout.addWidget(self.inRef,3,0,1,2)

        self.groupBox = QtGui.QGroupBox(Dialog)
        self.groupBox.setObjectName("groupBox")

        self.gridlayout1 = QtGui.QGridLayout(self.groupBox)
        self.gridlayout1.setObjectName("gridlayout1")

        self.rdoProjection = QtGui.QRadioButton(self.groupBox)
        self.rdoProjection.setChecked(True)
        self.rdoProjection.setObjectName("rdoProjection")
        self.gridlayout1.addWidget(self.rdoProjection,0,0,1,1)

        self.hboxlayout = QtGui.QHBoxLayout()
        self.hboxlayout.setObjectName("hboxlayout")

        self.txtProjection = QtGui.QLineEdit(self.groupBox)
        self.txtProjection.setReadOnly(True)
        self.txtProjection.setObjectName("txtProjection")
        self.hboxlayout.addWidget(self.txtProjection)

        self.btnProjection = QtGui.QToolButton(self.groupBox)
        self.btnProjection.setObjectName("btnProjection")
        self.hboxlayout.addWidget(self.btnProjection)
        self.gridlayout1.addLayout(self.hboxlayout,1,0,1,1)

        self.radioButton_2 = QtGui.QRadioButton(self.groupBox)
        self.radioButton_2.setObjectName("radioButton_2")
        self.gridlayout1.addWidget(self.radioButton_2,2,0,1,1)

        self.cmbLayer = QtGui.QComboBox(self.groupBox)
        self.cmbLayer.setEnabled(False)
        self.cmbLayer.setObjectName("cmbLayer")
        self.gridlayout1.addWidget(self.cmbLayer,3,0,1,1)

        self.label_5 = QtGui.QLabel(self.groupBox)
        self.label_5.setEnabled(False)
        self.label_5.setObjectName("label_5")
        self.gridlayout1.addWidget(self.label_5,4,0,1,1)

        self.outRef = QtGui.QLineEdit(self.groupBox)
        self.outRef.setEnabled(False)
        self.outRef.setReadOnly(True)
        self.outRef.setObjectName("outRef")
        self.gridlayout1.addWidget(self.outRef,5,0,1,1)
        self.gridlayout.addWidget(self.groupBox,4,0,1,2)

        self.label_2 = QtGui.QLabel(Dialog)
        self.label_2.setObjectName("label_2")
        self.gridlayout.addWidget(self.label_2,5,0,1,2)

        self.hboxlayout1 = QtGui.QHBoxLayout()
        self.hboxlayout1.setObjectName("hboxlayout1")

        self.outShape = QtGui.QLineEdit(Dialog)
        self.outShape.setReadOnly(True)
        self.outShape.setObjectName("outShape")
        self.hboxlayout1.addWidget(self.outShape)

        self.toolOut = QtGui.QToolButton(Dialog)
        self.toolOut.setObjectName("toolOut")
        self.hboxlayout1.addWidget(self.toolOut)
        self.gridlayout.addLayout(self.hboxlayout1,6,0,1,2)

        self.progressBar = QtGui.QProgressBar(Dialog)
        self.progressBar.setProperty("value",QtCore.QVariant(0))
        self.progressBar.setAlignment(QtCore.Qt.AlignCenter)
        self.progressBar.setTextVisible(True)
        self.progressBar.setObjectName("progressBar")
        self.gridlayout.addWidget(self.progressBar,7,0,1,1)

        self.buttonBox_2 = QtGui.QDialogButtonBox(Dialog)
        self.buttonBox_2.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox_2.setStandardButtons(QtGui.QDialogButtonBox.Close|QtGui.QDialogButtonBox.Ok)
        self.buttonBox_2.setObjectName("buttonBox_2")
        self.gridlayout.addWidget(self.buttonBox_2,7,1,1,1)

        self.retranslateUi(Dialog)
        QtCore.QObject.connect(self.buttonBox_2,QtCore.SIGNAL("accepted()"),Dialog.accept)
        QtCore.QObject.connect(self.buttonBox_2,QtCore.SIGNAL("rejected()"),Dialog.close)
        QtCore.QObject.connect(self.rdoProjection,QtCore.SIGNAL("toggled(bool)"),self.txtProjection.setEnabled)
        QtCore.QObject.connect(self.rdoProjection,QtCore.SIGNAL("toggled(bool)"),self.btnProjection.setEnabled)
        QtCore.QObject.connect(self.radioButton_2,QtCore.SIGNAL("toggled(bool)"),self.cmbLayer.setEnabled)
        QtCore.QObject.connect(self.radioButton_2,QtCore.SIGNAL("toggled(bool)"),self.label_5.setEnabled)
        QtCore.QObject.connect(self.radioButton_2,QtCore.SIGNAL("toggled(bool)"),self.outRef.setEnabled)
        QtCore.QMetaObject.connectSlotsByName(Dialog)

    def retranslateUi(self, Dialog):
        Dialog.setWindowTitle(QtGui.QApplication.translate("Dialog", "Projection Management Tool", None, QtGui.QApplication.UnicodeUTF8))
        self.label_3.setText(QtGui.QApplication.translate("Dialog", "Input vector layer", None, QtGui.QApplication.UnicodeUTF8))
        self.label_4.setText(QtGui.QApplication.translate("Dialog", "Input spatial reference system", None, QtGui.QApplication.UnicodeUTF8))
        self.groupBox.setTitle(QtGui.QApplication.translate("Dialog", "Output spatial reference system", None, QtGui.QApplication.UnicodeUTF8))
        self.rdoProjection.setText(QtGui.QApplication.translate("Dialog", "Use predefined spatial reference system", None, QtGui.QApplication.UnicodeUTF8))
        self.btnProjection.setText(QtGui.QApplication.translate("Dialog", "Choose", None, QtGui.QApplication.UnicodeUTF8))
        self.radioButton_2.setText(QtGui.QApplication.translate("Dialog", "Import spatial reference system from existing layer", None, QtGui.QApplication.UnicodeUTF8))
        self.label_5.setText(QtGui.QApplication.translate("Dialog", "Import spatial reference system:", None, QtGui.QApplication.UnicodeUTF8))
        self.label_2.setText(QtGui.QApplication.translate("Dialog", "Output Shapefile", None, QtGui.QApplication.UnicodeUTF8))
        self.toolOut.setText(QtGui.QApplication.translate("Dialog", "Browse", None, QtGui.QApplication.UnicodeUTF8))

