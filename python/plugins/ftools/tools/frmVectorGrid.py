# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'frmVectorGrid.ui'
#
# Created: Mon Nov 10 00:06:05 2008
#      by: PyQt4 UI code generator 4.3.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

class Ui_Dialog(object):
    def setupUi(self, Dialog):
        Dialog.setObjectName("Dialog")
        Dialog.setWindowModality(QtCore.Qt.NonModal)
        Dialog.resize(QtCore.QSize(QtCore.QRect(0,0,374,483).size()).expandedTo(Dialog.minimumSizeHint()))
        Dialog.setSizeGripEnabled(True)

        self.gridlayout = QtGui.QGridLayout(Dialog)
        self.gridlayout.setObjectName("gridlayout")

        self.groupBox = QtGui.QGroupBox(Dialog)
        self.groupBox.setObjectName("groupBox")

        self.gridlayout1 = QtGui.QGridLayout(self.groupBox)
        self.gridlayout1.setObjectName("gridlayout1")

        self.rdoBoundary = QtGui.QRadioButton(self.groupBox)
        self.rdoBoundary.setChecked(True)
        self.rdoBoundary.setObjectName("rdoBoundary")
        self.gridlayout1.addWidget(self.rdoBoundary,0,0,1,2)

        self.inShape = QtGui.QComboBox(self.groupBox)
        self.inShape.setObjectName("inShape")
        self.gridlayout1.addWidget(self.inShape,1,0,1,2)

        self.rdoCoordinates = QtGui.QRadioButton(self.groupBox)
        self.rdoCoordinates.setObjectName("rdoCoordinates")
        self.gridlayout1.addWidget(self.rdoCoordinates,2,0,1,2)

        self.hboxlayout = QtGui.QHBoxLayout()
        self.hboxlayout.setObjectName("hboxlayout")

        self.label = QtGui.QLabel(self.groupBox)
        self.label.setEnabled(False)
        self.label.setObjectName("label")
        self.hboxlayout.addWidget(self.label)

        self.xMin = QtGui.QLineEdit(self.groupBox)
        self.xMin.setObjectName("xMin")
        self.hboxlayout.addWidget(self.xMin)
        self.gridlayout1.addLayout(self.hboxlayout,3,0,1,1)

        self.hboxlayout1 = QtGui.QHBoxLayout()
        self.hboxlayout1.setObjectName("hboxlayout1")

        self.label_4 = QtGui.QLabel(self.groupBox)
        self.label_4.setEnabled(False)
        self.label_4.setObjectName("label_4")
        self.hboxlayout1.addWidget(self.label_4)

        self.yMin = QtGui.QLineEdit(self.groupBox)
        self.yMin.setObjectName("yMin")
        self.hboxlayout1.addWidget(self.yMin)
        self.gridlayout1.addLayout(self.hboxlayout1,3,1,1,1)

        self.hboxlayout2 = QtGui.QHBoxLayout()
        self.hboxlayout2.setObjectName("hboxlayout2")

        self.label_3 = QtGui.QLabel(self.groupBox)
        self.label_3.setEnabled(False)
        self.label_3.setObjectName("label_3")
        self.hboxlayout2.addWidget(self.label_3)

        self.xMax = QtGui.QLineEdit(self.groupBox)
        self.xMax.setObjectName("xMax")
        self.hboxlayout2.addWidget(self.xMax)
        self.gridlayout1.addLayout(self.hboxlayout2,4,0,1,1)

        self.hboxlayout3 = QtGui.QHBoxLayout()
        self.hboxlayout3.setObjectName("hboxlayout3")

        self.label_5 = QtGui.QLabel(self.groupBox)
        self.label_5.setEnabled(False)
        self.label_5.setObjectName("label_5")
        self.hboxlayout3.addWidget(self.label_5)

        self.yMax = QtGui.QLineEdit(self.groupBox)
        self.yMax.setObjectName("yMax")
        self.hboxlayout3.addWidget(self.yMax)
        self.gridlayout1.addLayout(self.hboxlayout3,4,1,1,1)
        self.gridlayout.addWidget(self.groupBox,0,0,1,2)

        self.gridBox = QtGui.QGroupBox(Dialog)
        self.gridBox.setObjectName("gridBox")

        self.gridlayout2 = QtGui.QGridLayout(self.gridBox)
        self.gridlayout2.setObjectName("gridlayout2")

        spacerItem = QtGui.QSpacerItem(40,20,QtGui.QSizePolicy.Expanding,QtGui.QSizePolicy.Minimum)
        self.gridlayout2.addItem(spacerItem,0,0,1,1)

        self.label_7 = QtGui.QLabel(self.gridBox)
        self.label_7.setObjectName("label_7")
        self.gridlayout2.addWidget(self.label_7,0,1,1,1)

        self.spnX = QtGui.QDoubleSpinBox(self.gridBox)
        self.spnX.setDecimals(4)
        self.spnX.setMinimum(0.0001)
        self.spnX.setMaximum(9999.0)
        self.spnX.setSingleStep(0.0001)
        self.spnX.setObjectName("spnX")
        self.gridlayout2.addWidget(self.spnX,0,2,1,1)

        self.chkLock = QtGui.QCheckBox(self.gridBox)
        self.chkLock.setChecked(True)
        self.chkLock.setObjectName("chkLock")
        self.gridlayout2.addWidget(self.chkLock,0,3,2,1)

        spacerItem1 = QtGui.QSpacerItem(40,20,QtGui.QSizePolicy.Expanding,QtGui.QSizePolicy.Minimum)
        self.gridlayout2.addItem(spacerItem1,0,4,1,1)

        spacerItem2 = QtGui.QSpacerItem(40,20,QtGui.QSizePolicy.Expanding,QtGui.QSizePolicy.Minimum)
        self.gridlayout2.addItem(spacerItem2,1,0,1,1)

        self.label_8 = QtGui.QLabel(self.gridBox)
        self.label_8.setEnabled(False)
        self.label_8.setObjectName("label_8")
        self.gridlayout2.addWidget(self.label_8,1,1,1,1)

        self.spnY = QtGui.QDoubleSpinBox(self.gridBox)
        self.spnY.setEnabled(False)
        self.spnY.setDecimals(4)
        self.spnY.setMinimum(0.0001)
        self.spnY.setMaximum(9999.0)
        self.spnY.setSingleStep(0.0001)
        self.spnY.setObjectName("spnY")
        self.gridlayout2.addWidget(self.spnY,1,2,1,1)

        self.rdoPolygons = QtGui.QRadioButton(self.gridBox)
        self.rdoPolygons.setChecked(True)
        self.rdoPolygons.setObjectName("rdoPolygons")
        self.gridlayout2.addWidget(self.rdoPolygons,2,0,1,3)

        self.rdoLines = QtGui.QRadioButton(self.gridBox)
        self.rdoLines.setObjectName("rdoLines")
        self.gridlayout2.addWidget(self.rdoLines,3,0,1,3)
        self.gridlayout.addWidget(self.gridBox,1,0,1,2)

        self.label_2 = QtGui.QLabel(Dialog)

        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed,QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.label_2.sizePolicy().hasHeightForWidth())
        self.label_2.setSizePolicy(sizePolicy)
        self.label_2.setObjectName("label_2")
        self.gridlayout.addWidget(self.label_2,2,0,1,1)

        self.hboxlayout4 = QtGui.QHBoxLayout()
        self.hboxlayout4.setObjectName("hboxlayout4")

        self.outShape = QtGui.QLineEdit(Dialog)
        self.outShape.setReadOnly(True)
        self.outShape.setObjectName("outShape")
        self.hboxlayout4.addWidget(self.outShape)

        self.toolOut = QtGui.QToolButton(Dialog)
        self.toolOut.setObjectName("toolOut")
        self.hboxlayout4.addWidget(self.toolOut)
        self.gridlayout.addLayout(self.hboxlayout4,3,0,1,2)

        self.progressBar = QtGui.QProgressBar(Dialog)
        self.progressBar.setProperty("value",QtCore.QVariant(0))
        self.progressBar.setAlignment(QtCore.Qt.AlignCenter)
        self.progressBar.setTextVisible(True)
        self.progressBar.setObjectName("progressBar")
        self.gridlayout.addWidget(self.progressBar,4,0,1,1)

        self.buttonBox_2 = QtGui.QDialogButtonBox(Dialog)
        self.buttonBox_2.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox_2.setStandardButtons(QtGui.QDialogButtonBox.Close|QtGui.QDialogButtonBox.Ok)
        self.buttonBox_2.setObjectName("buttonBox_2")
        self.gridlayout.addWidget(self.buttonBox_2,4,1,1,1)

        self.retranslateUi(Dialog)
        QtCore.QObject.connect(self.buttonBox_2,QtCore.SIGNAL("accepted()"),Dialog.accept)
        QtCore.QObject.connect(self.buttonBox_2,QtCore.SIGNAL("rejected()"),Dialog.close)
        QtCore.QObject.connect(self.rdoBoundary,QtCore.SIGNAL("toggled(bool)"),self.inShape.setEnabled)
        QtCore.QObject.connect(self.rdoCoordinates,QtCore.SIGNAL("toggled(bool)"),self.xMin.setEnabled)
        QtCore.QObject.connect(self.rdoCoordinates,QtCore.SIGNAL("toggled(bool)"),self.xMax.setEnabled)
        QtCore.QObject.connect(self.rdoCoordinates,QtCore.SIGNAL("toggled(bool)"),self.yMin.setEnabled)
        QtCore.QObject.connect(self.rdoCoordinates,QtCore.SIGNAL("toggled(bool)"),self.yMax.setEnabled)
        QtCore.QObject.connect(self.rdoCoordinates,QtCore.SIGNAL("toggled(bool)"),self.label.setEnabled)
        QtCore.QObject.connect(self.rdoCoordinates,QtCore.SIGNAL("toggled(bool)"),self.label_3.setEnabled)
        QtCore.QObject.connect(self.rdoCoordinates,QtCore.SIGNAL("toggled(bool)"),self.label_4.setEnabled)
        QtCore.QObject.connect(self.rdoCoordinates,QtCore.SIGNAL("toggled(bool)"),self.label_5.setEnabled)
        QtCore.QObject.connect(self.chkLock,QtCore.SIGNAL("clicked(bool)"),self.spnY.setDisabled)
        QtCore.QObject.connect(self.chkLock,QtCore.SIGNAL("toggled(bool)"),self.label_8.setDisabled)
        QtCore.QMetaObject.connectSlotsByName(Dialog)

    def retranslateUi(self, Dialog):
        Dialog.setWindowTitle(QtGui.QApplication.translate("Dialog", "Generate Regular Points", None, QtGui.QApplication.UnicodeUTF8))
        self.groupBox.setTitle(QtGui.QApplication.translate("Dialog", "Grid Extent", None, QtGui.QApplication.UnicodeUTF8))
        self.rdoBoundary.setText(QtGui.QApplication.translate("Dialog", "Input Boundary Layer", None, QtGui.QApplication.UnicodeUTF8))
        self.rdoCoordinates.setText(QtGui.QApplication.translate("Dialog", "Input Coordinates", None, QtGui.QApplication.UnicodeUTF8))
        self.label.setText(QtGui.QApplication.translate("Dialog", "X Min", None, QtGui.QApplication.UnicodeUTF8))
        self.label_4.setText(QtGui.QApplication.translate("Dialog", "Y Min", None, QtGui.QApplication.UnicodeUTF8))
        self.label_3.setText(QtGui.QApplication.translate("Dialog", "X Max", None, QtGui.QApplication.UnicodeUTF8))
        self.label_5.setText(QtGui.QApplication.translate("Dialog", "Y Max", None, QtGui.QApplication.UnicodeUTF8))
        self.gridBox.setTitle(QtGui.QApplication.translate("Dialog", "Parameters", None, QtGui.QApplication.UnicodeUTF8))
        self.label_7.setText(QtGui.QApplication.translate("Dialog", "X", None, QtGui.QApplication.UnicodeUTF8))
        self.chkLock.setText(QtGui.QApplication.translate("Dialog", "Lock 1:1 ratio", None, QtGui.QApplication.UnicodeUTF8))
        self.label_8.setText(QtGui.QApplication.translate("Dialog", "Y", None, QtGui.QApplication.UnicodeUTF8))
        self.rdoPolygons.setText(QtGui.QApplication.translate("Dialog", "Output as polygons", None, QtGui.QApplication.UnicodeUTF8))
        self.rdoLines.setText(QtGui.QApplication.translate("Dialog", "Output as lines", None, QtGui.QApplication.UnicodeUTF8))
        self.label_2.setText(QtGui.QApplication.translate("Dialog", "Output Shapefile", None, QtGui.QApplication.UnicodeUTF8))
        self.toolOut.setText(QtGui.QApplication.translate("Dialog", "Browse", None, QtGui.QApplication.UnicodeUTF8))

