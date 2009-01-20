# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'frmRegPoints.ui'
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
        Dialog.resize(QtCore.QSize(QtCore.QRect(0,0,373,451).size()).expandedTo(Dialog.minimumSizeHint()))
        Dialog.setSizeGripEnabled(True)

        self.gridlayout = QtGui.QGridLayout(Dialog)
        self.gridlayout.setObjectName("gridlayout")

        self.rdoBoundary = QtGui.QRadioButton(Dialog)
        self.rdoBoundary.setChecked(True)
        self.rdoBoundary.setObjectName("rdoBoundary")
        self.gridlayout.addWidget(self.rdoBoundary,0,0,1,1)

        self.inShape = QtGui.QComboBox(Dialog)
        self.inShape.setObjectName("inShape")
        self.gridlayout.addWidget(self.inShape,1,0,1,2)

        self.rdoCoordinates = QtGui.QRadioButton(Dialog)
        self.rdoCoordinates.setObjectName("rdoCoordinates")
        self.gridlayout.addWidget(self.rdoCoordinates,2,0,1,1)

        self.hboxlayout = QtGui.QHBoxLayout()
        self.hboxlayout.setObjectName("hboxlayout")

        self.label = QtGui.QLabel(Dialog)
        self.label.setObjectName("label")
        self.hboxlayout.addWidget(self.label)

        self.xMin = QtGui.QLineEdit(Dialog)
        self.xMin.setObjectName("xMin")
        self.hboxlayout.addWidget(self.xMin)
        self.gridlayout.addLayout(self.hboxlayout,3,0,1,1)

        self.hboxlayout1 = QtGui.QHBoxLayout()
        self.hboxlayout1.setObjectName("hboxlayout1")

        self.label_4 = QtGui.QLabel(Dialog)
        self.label_4.setObjectName("label_4")
        self.hboxlayout1.addWidget(self.label_4)

        self.yMin = QtGui.QLineEdit(Dialog)
        self.yMin.setObjectName("yMin")
        self.hboxlayout1.addWidget(self.yMin)
        self.gridlayout.addLayout(self.hboxlayout1,3,1,1,1)

        self.hboxlayout2 = QtGui.QHBoxLayout()
        self.hboxlayout2.setObjectName("hboxlayout2")

        self.label_3 = QtGui.QLabel(Dialog)
        self.label_3.setObjectName("label_3")
        self.hboxlayout2.addWidget(self.label_3)

        self.xMax = QtGui.QLineEdit(Dialog)
        self.xMax.setObjectName("xMax")
        self.hboxlayout2.addWidget(self.xMax)
        self.gridlayout.addLayout(self.hboxlayout2,4,0,1,1)

        self.hboxlayout3 = QtGui.QHBoxLayout()
        self.hboxlayout3.setObjectName("hboxlayout3")

        self.label_5 = QtGui.QLabel(Dialog)
        self.label_5.setObjectName("label_5")
        self.hboxlayout3.addWidget(self.label_5)

        self.yMax = QtGui.QLineEdit(Dialog)
        self.yMax.setObjectName("yMax")
        self.hboxlayout3.addWidget(self.yMax)
        self.gridlayout.addLayout(self.hboxlayout3,4,1,1,1)

        self.gridBox = QtGui.QGroupBox(Dialog)
        self.gridBox.setObjectName("gridBox")

        self.gridlayout1 = QtGui.QGridLayout(self.gridBox)
        self.gridlayout1.setObjectName("gridlayout1")

        self.hboxlayout4 = QtGui.QHBoxLayout()
        self.hboxlayout4.setObjectName("hboxlayout4")

        self.rdoSpacing = QtGui.QRadioButton(self.gridBox)
        self.rdoSpacing.setChecked(True)
        self.rdoSpacing.setObjectName("rdoSpacing")
        self.hboxlayout4.addWidget(self.rdoSpacing)

        self.spnSpacing = QtGui.QDoubleSpinBox(self.gridBox)
        self.spnSpacing.setDecimals(4)
        self.spnSpacing.setMinimum(0.0001)
        self.spnSpacing.setMaximum(9999.0)
        self.spnSpacing.setSingleStep(0.0001)
        self.spnSpacing.setObjectName("spnSpacing")
        self.hboxlayout4.addWidget(self.spnSpacing)
        self.gridlayout1.addLayout(self.hboxlayout4,0,0,1,2)

        self.hboxlayout5 = QtGui.QHBoxLayout()
        self.hboxlayout5.setObjectName("hboxlayout5")

        self.rdoNumber = QtGui.QRadioButton(self.gridBox)
        self.rdoNumber.setObjectName("rdoNumber")
        self.hboxlayout5.addWidget(self.rdoNumber)

        self.spnNumber = QtGui.QSpinBox(self.gridBox)
        self.spnNumber.setMinimum(1)
        self.spnNumber.setMaximum(9999)
        self.spnNumber.setObjectName("spnNumber")
        self.hboxlayout5.addWidget(self.spnNumber)
        self.gridlayout1.addLayout(self.hboxlayout5,1,0,1,2)

        self.chkRandom = QtGui.QCheckBox(self.gridBox)
        self.chkRandom.setObjectName("chkRandom")
        self.gridlayout1.addWidget(self.chkRandom,2,0,1,2)

        self.spnInset = QtGui.QDoubleSpinBox(self.gridBox)
        self.spnInset.setDecimals(4)
        self.spnInset.setMaximum(9999.0)
        self.spnInset.setSingleStep(0.0001)
        self.spnInset.setObjectName("spnInset")
        self.gridlayout1.addWidget(self.spnInset,3,1,1,1)

        self.label_6 = QtGui.QLabel(self.gridBox)
        self.label_6.setObjectName("label_6")
        self.gridlayout1.addWidget(self.label_6,3,0,1,1)
        self.gridlayout.addWidget(self.gridBox,5,0,1,2)

        self.label_2 = QtGui.QLabel(Dialog)

        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed,QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.label_2.sizePolicy().hasHeightForWidth())
        self.label_2.setSizePolicy(sizePolicy)
        self.label_2.setObjectName("label_2")
        self.gridlayout.addWidget(self.label_2,6,0,1,1)

        self.hboxlayout6 = QtGui.QHBoxLayout()
        self.hboxlayout6.setObjectName("hboxlayout6")

        self.outShape = QtGui.QLineEdit(Dialog)
        self.outShape.setReadOnly(True)
        self.outShape.setObjectName("outShape")
        self.hboxlayout6.addWidget(self.outShape)

        self.toolOut = QtGui.QToolButton(Dialog)
        self.toolOut.setObjectName("toolOut")
        self.hboxlayout6.addWidget(self.toolOut)
        self.gridlayout.addLayout(self.hboxlayout6,7,0,1,2)

        self.progressBar = QtGui.QProgressBar(Dialog)
        self.progressBar.setProperty("value",QtCore.QVariant(0))
        self.progressBar.setAlignment(QtCore.Qt.AlignCenter)
        self.progressBar.setTextVisible(True)
        self.progressBar.setObjectName("progressBar")
        self.gridlayout.addWidget(self.progressBar,8,0,1,1)

        self.buttonBox_2 = QtGui.QDialogButtonBox(Dialog)
        self.buttonBox_2.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox_2.setStandardButtons(QtGui.QDialogButtonBox.Close|QtGui.QDialogButtonBox.Ok)
        self.buttonBox_2.setObjectName("buttonBox_2")
        self.gridlayout.addWidget(self.buttonBox_2,8,1,1,1)

        self.retranslateUi(Dialog)
        QtCore.QObject.connect(self.buttonBox_2,QtCore.SIGNAL("accepted()"),Dialog.accept)
        QtCore.QObject.connect(self.buttonBox_2,QtCore.SIGNAL("rejected()"),Dialog.close)
        QtCore.QObject.connect(self.rdoBoundary,QtCore.SIGNAL("toggled(bool)"),self.inShape.setEnabled)
        QtCore.QObject.connect(self.rdoCoordinates,QtCore.SIGNAL("toggled(bool)"),self.xMin.setEnabled)
        QtCore.QObject.connect(self.rdoCoordinates,QtCore.SIGNAL("toggled(bool)"),self.xMax.setEnabled)
        QtCore.QObject.connect(self.rdoCoordinates,QtCore.SIGNAL("toggled(bool)"),self.yMin.setEnabled)
        QtCore.QObject.connect(self.rdoCoordinates,QtCore.SIGNAL("toggled(bool)"),self.yMax.setEnabled)
        QtCore.QObject.connect(self.rdoSpacing,QtCore.SIGNAL("toggled(bool)"),self.spnSpacing.setEnabled)
        QtCore.QObject.connect(self.rdoNumber,QtCore.SIGNAL("toggled(bool)"),self.spnNumber.setEnabled)
        QtCore.QMetaObject.connectSlotsByName(Dialog)

    def retranslateUi(self, Dialog):
        Dialog.setWindowTitle(QtGui.QApplication.translate("Dialog", "Generate Regular Points", None, QtGui.QApplication.UnicodeUTF8))
        self.rdoBoundary.setText(QtGui.QApplication.translate("Dialog", "Input Boundary Layer", None, QtGui.QApplication.UnicodeUTF8))
        self.rdoCoordinates.setText(QtGui.QApplication.translate("Dialog", "Input Coordinates", None, QtGui.QApplication.UnicodeUTF8))
        self.label.setText(QtGui.QApplication.translate("Dialog", "X Min", None, QtGui.QApplication.UnicodeUTF8))
        self.label_4.setText(QtGui.QApplication.translate("Dialog", "Y Min", None, QtGui.QApplication.UnicodeUTF8))
        self.label_3.setText(QtGui.QApplication.translate("Dialog", "X Max", None, QtGui.QApplication.UnicodeUTF8))
        self.label_5.setText(QtGui.QApplication.translate("Dialog", "Y Max", None, QtGui.QApplication.UnicodeUTF8))
        self.gridBox.setTitle(QtGui.QApplication.translate("Dialog", "Grid Spacing", None, QtGui.QApplication.UnicodeUTF8))
        self.rdoSpacing.setText(QtGui.QApplication.translate("Dialog", "Use this point spacing", None, QtGui.QApplication.UnicodeUTF8))
        self.rdoNumber.setText(QtGui.QApplication.translate("Dialog", "Use this number of points", None, QtGui.QApplication.UnicodeUTF8))
        self.chkRandom.setText(QtGui.QApplication.translate("Dialog", "Apply random offset to point spacing", None, QtGui.QApplication.UnicodeUTF8))
        self.label_6.setText(QtGui.QApplication.translate("Dialog", "Initial inset from corner (LH side)", None, QtGui.QApplication.UnicodeUTF8))
        self.label_2.setText(QtGui.QApplication.translate("Dialog", "Output Shapefile", None, QtGui.QApplication.UnicodeUTF8))
        self.toolOut.setText(QtGui.QApplication.translate("Dialog", "Browse", None, QtGui.QApplication.UnicodeUTF8))

