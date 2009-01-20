# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'frmRandPoints.ui'
#
# Created: Mon Nov 10 00:01:16 2008
#      by: PyQt4 UI code generator 4.3.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

class Ui_Dialog(object):
    def setupUi(self, Dialog):
        Dialog.setObjectName("Dialog")
        Dialog.setWindowModality(QtCore.Qt.NonModal)
        Dialog.resize(QtCore.QSize(QtCore.QRect(0,0,373,435).size()).expandedTo(Dialog.minimumSizeHint()))
        Dialog.setSizeGripEnabled(True)

        self.gridlayout = QtGui.QGridLayout(Dialog)
        self.gridlayout.setObjectName("gridlayout")

        self.label_3 = QtGui.QLabel(Dialog)

        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed,QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.label_3.sizePolicy().hasHeightForWidth())
        self.label_3.setSizePolicy(sizePolicy)
        self.label_3.setObjectName("label_3")
        self.gridlayout.addWidget(self.label_3,0,0,1,1)

        self.inShape = QtGui.QComboBox(Dialog)
        self.inShape.setObjectName("inShape")
        self.gridlayout.addWidget(self.inShape,1,0,1,2)

        self.hboxlayout = QtGui.QHBoxLayout()
        self.hboxlayout.setObjectName("hboxlayout")

        self.chkMinimum = QtGui.QCheckBox(Dialog)
        self.chkMinimum.setEnabled(False)
        self.chkMinimum.setObjectName("chkMinimum")
        self.hboxlayout.addWidget(self.chkMinimum)

        self.spnMinimum = QtGui.QSpinBox(Dialog)
        self.spnMinimum.setEnabled(False)
        self.spnMinimum.setObjectName("spnMinimum")
        self.hboxlayout.addWidget(self.spnMinimum)
        self.gridlayout.addLayout(self.hboxlayout,2,0,1,2)

        self.groupBox = QtGui.QGroupBox(Dialog)
        self.groupBox.setObjectName("groupBox")

        self.gridlayout1 = QtGui.QGridLayout(self.groupBox)
        self.gridlayout1.setObjectName("gridlayout1")

        self.label = QtGui.QLabel(self.groupBox)

        font = QtGui.QFont()
        font.setPointSize(9)
        font.setWeight(75)
        font.setBold(True)
        self.label.setFont(font)
        self.label.setObjectName("label")
        self.gridlayout1.addWidget(self.label,0,0,1,1)

        self.hboxlayout1 = QtGui.QHBoxLayout()
        self.hboxlayout1.setObjectName("hboxlayout1")

        self.rdoUnstratified = QtGui.QRadioButton(self.groupBox)
        self.rdoUnstratified.setChecked(True)
        self.rdoUnstratified.setObjectName("rdoUnstratified")
        self.hboxlayout1.addWidget(self.rdoUnstratified)

        self.spnUnstratified = QtGui.QSpinBox(self.groupBox)
        self.spnUnstratified.setEnabled(True)
        self.spnUnstratified.setMinimum(1)
        self.spnUnstratified.setMaximum(9999)
        self.spnUnstratified.setObjectName("spnUnstratified")
        self.hboxlayout1.addWidget(self.spnUnstratified)
        self.gridlayout1.addLayout(self.hboxlayout1,1,0,1,1)

        self.label_4 = QtGui.QLabel(self.groupBox)

        font = QtGui.QFont()
        font.setPointSize(9)
        font.setWeight(75)
        font.setBold(True)
        self.label_4.setFont(font)
        self.label_4.setObjectName("label_4")
        self.gridlayout1.addWidget(self.label_4,2,0,1,1)

        self.hboxlayout2 = QtGui.QHBoxLayout()
        self.hboxlayout2.setObjectName("hboxlayout2")

        self.rdoStratified = QtGui.QRadioButton(self.groupBox)
        self.rdoStratified.setObjectName("rdoStratified")
        self.hboxlayout2.addWidget(self.rdoStratified)

        self.spnStratified = QtGui.QSpinBox(self.groupBox)
        self.spnStratified.setEnabled(False)
        self.spnStratified.setMinimum(1)
        self.spnStratified.setMaximum(9999)
        self.spnStratified.setObjectName("spnStratified")
        self.hboxlayout2.addWidget(self.spnStratified)
        self.gridlayout1.addLayout(self.hboxlayout2,3,0,1,1)

        self.hboxlayout3 = QtGui.QHBoxLayout()
        self.hboxlayout3.setObjectName("hboxlayout3")

        self.rdoDensity = QtGui.QRadioButton(self.groupBox)
        self.rdoDensity.setObjectName("rdoDensity")
        self.hboxlayout3.addWidget(self.rdoDensity)

        self.spnDensity = QtGui.QDoubleSpinBox(self.groupBox)
        self.spnDensity.setEnabled(False)
        self.spnDensity.setDecimals(4)
        self.spnDensity.setMinimum(0.0001)
        self.spnDensity.setSingleStep(0.0001)
        self.spnDensity.setProperty("value",QtCore.QVariant(0.0001))
        self.spnDensity.setObjectName("spnDensity")
        self.hboxlayout3.addWidget(self.spnDensity)
        self.gridlayout1.addLayout(self.hboxlayout3,4,0,1,1)

        self.hboxlayout4 = QtGui.QHBoxLayout()
        self.hboxlayout4.setObjectName("hboxlayout4")

        self.rdoField = QtGui.QRadioButton(self.groupBox)
        self.rdoField.setObjectName("rdoField")
        self.hboxlayout4.addWidget(self.rdoField)

        self.cmbField = QtGui.QComboBox(self.groupBox)
        self.cmbField.setEnabled(False)
        self.cmbField.setObjectName("cmbField")
        self.hboxlayout4.addWidget(self.cmbField)
        self.gridlayout1.addLayout(self.hboxlayout4,5,0,1,1)
        self.gridlayout.addWidget(self.groupBox,3,0,1,2)

        self.label_2 = QtGui.QLabel(Dialog)

        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed,QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.label_2.sizePolicy().hasHeightForWidth())
        self.label_2.setSizePolicy(sizePolicy)
        self.label_2.setObjectName("label_2")
        self.gridlayout.addWidget(self.label_2,4,0,1,1)

        self.hboxlayout5 = QtGui.QHBoxLayout()
        self.hboxlayout5.setObjectName("hboxlayout5")

        self.outShape = QtGui.QLineEdit(Dialog)
        self.outShape.setReadOnly(True)
        self.outShape.setObjectName("outShape")
        self.hboxlayout5.addWidget(self.outShape)

        self.toolOut = QtGui.QToolButton(Dialog)
        self.toolOut.setObjectName("toolOut")
        self.hboxlayout5.addWidget(self.toolOut)
        self.gridlayout.addLayout(self.hboxlayout5,5,0,1,2)

        self.progressBar = QtGui.QProgressBar(Dialog)
        self.progressBar.setProperty("value",QtCore.QVariant(24))
        self.progressBar.setAlignment(QtCore.Qt.AlignCenter)
        self.progressBar.setTextVisible(True)
        self.progressBar.setObjectName("progressBar")
        self.gridlayout.addWidget(self.progressBar,6,0,1,1)

        self.buttonBox_2 = QtGui.QDialogButtonBox(Dialog)
        self.buttonBox_2.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox_2.setStandardButtons(QtGui.QDialogButtonBox.Close|QtGui.QDialogButtonBox.Ok)
        self.buttonBox_2.setObjectName("buttonBox_2")
        self.gridlayout.addWidget(self.buttonBox_2,6,1,1,1)

        self.retranslateUi(Dialog)
        QtCore.QObject.connect(self.buttonBox_2,QtCore.SIGNAL("accepted()"),Dialog.accept)
        QtCore.QObject.connect(self.buttonBox_2,QtCore.SIGNAL("rejected()"),Dialog.close)
        QtCore.QObject.connect(self.rdoUnstratified,QtCore.SIGNAL("toggled(bool)"),self.spnUnstratified.setEnabled)
        QtCore.QObject.connect(self.rdoStratified,QtCore.SIGNAL("toggled(bool)"),self.spnStratified.setEnabled)
        QtCore.QObject.connect(self.rdoField,QtCore.SIGNAL("toggled(bool)"),self.cmbField.setEnabled)
        QtCore.QObject.connect(self.chkMinimum,QtCore.SIGNAL("toggled(bool)"),self.spnMinimum.setEnabled)
        QtCore.QObject.connect(self.rdoDensity,QtCore.SIGNAL("toggled(bool)"),self.spnDensity.setEnabled)
        QtCore.QMetaObject.connectSlotsByName(Dialog)

    def retranslateUi(self, Dialog):
        Dialog.setWindowTitle(QtGui.QApplication.translate("Dialog", "Generate Random Points", None, QtGui.QApplication.UnicodeUTF8))
        self.label_3.setText(QtGui.QApplication.translate("Dialog", "Input Boundary Layer", None, QtGui.QApplication.UnicodeUTF8))
        self.chkMinimum.setText(QtGui.QApplication.translate("Dialog", "Minimum distance between points", None, QtGui.QApplication.UnicodeUTF8))
        self.groupBox.setTitle(QtGui.QApplication.translate("Dialog", "Sample Size", None, QtGui.QApplication.UnicodeUTF8))
        self.label.setText(QtGui.QApplication.translate("Dialog", "Unstratified Sampling Design (Entire layer)", None, QtGui.QApplication.UnicodeUTF8))
        self.rdoUnstratified.setText(QtGui.QApplication.translate("Dialog", "Use this number of points", None, QtGui.QApplication.UnicodeUTF8))
        self.label_4.setText(QtGui.QApplication.translate("Dialog", "Stratified Sampling Design (Individual polygons)", None, QtGui.QApplication.UnicodeUTF8))
        self.rdoStratified.setText(QtGui.QApplication.translate("Dialog", "Use this number of points", None, QtGui.QApplication.UnicodeUTF8))
        self.rdoDensity.setText(QtGui.QApplication.translate("Dialog", "Use this density of points", None, QtGui.QApplication.UnicodeUTF8))
        self.rdoField.setText(QtGui.QApplication.translate("Dialog", "Use value from input field", None, QtGui.QApplication.UnicodeUTF8))
        self.label_2.setText(QtGui.QApplication.translate("Dialog", "Output Shapefile", None, QtGui.QApplication.UnicodeUTF8))
        self.toolOut.setText(QtGui.QApplication.translate("Dialog", "Browse", None, QtGui.QApplication.UnicodeUTF8))

