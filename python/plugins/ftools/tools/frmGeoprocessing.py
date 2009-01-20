# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'frmGeoprocessing.ui'
#
# Created: Thu Nov 13 23:17:35 2008
#      by: PyQt4 UI code generator 4.3.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

class Ui_Dialog(object):
    def setupUi(self, Dialog):
        Dialog.setObjectName("Dialog")
        Dialog.resize(QtCore.QSize(QtCore.QRect(0,0,422,405).size()).expandedTo(Dialog.minimumSizeHint()))

        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Minimum,QtGui.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(Dialog.sizePolicy().hasHeightForWidth())
        Dialog.setSizePolicy(sizePolicy)

        self.gridlayout = QtGui.QGridLayout(Dialog)
        self.gridlayout.setObjectName("gridlayout")

        self.vboxlayout = QtGui.QVBoxLayout()
        self.vboxlayout.setObjectName("vboxlayout")

        self.label_1 = QtGui.QLabel(Dialog)
        self.label_1.setObjectName("label_1")
        self.vboxlayout.addWidget(self.label_1)

        self.inShapeA = QtGui.QComboBox(Dialog)

        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Minimum,QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.inShapeA.sizePolicy().hasHeightForWidth())
        self.inShapeA.setSizePolicy(sizePolicy)
        self.inShapeA.setObjectName("inShapeA")
        self.vboxlayout.addWidget(self.inShapeA)
        self.gridlayout.addLayout(self.vboxlayout,0,0,1,2)

        self.vboxlayout1 = QtGui.QVBoxLayout()
        self.vboxlayout1.setObjectName("vboxlayout1")

        self.label_2 = QtGui.QLabel(Dialog)
        self.label_2.setObjectName("label_2")
        self.vboxlayout1.addWidget(self.label_2)

        self.inShapeB = QtGui.QComboBox(Dialog)

        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Minimum,QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.inShapeB.sizePolicy().hasHeightForWidth())
        self.inShapeB.setSizePolicy(sizePolicy)
        self.inShapeB.setObjectName("inShapeB")
        self.vboxlayout1.addWidget(self.inShapeB)
        self.gridlayout.addLayout(self.vboxlayout1,1,0,1,2)

        self.hboxlayout = QtGui.QHBoxLayout()
        self.hboxlayout.setObjectName("hboxlayout")

        self.hboxlayout1 = QtGui.QHBoxLayout()
        self.hboxlayout1.setObjectName("hboxlayout1")

        self.rdoBuffer = QtGui.QRadioButton(Dialog)
        self.rdoBuffer.setChecked(True)
        self.rdoBuffer.setObjectName("rdoBuffer")
        self.hboxlayout1.addWidget(self.rdoBuffer)
        self.hboxlayout.addLayout(self.hboxlayout1)

        self.param = QtGui.QLineEdit(Dialog)
        self.param.setEnabled(True)

        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Minimum,QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.param.sizePolicy().hasHeightForWidth())
        self.param.setSizePolicy(sizePolicy)
        self.param.setLayoutDirection(QtCore.Qt.LeftToRight)
        self.param.setCursorPosition(0)
        self.param.setObjectName("param")
        self.hboxlayout.addWidget(self.param)
        self.gridlayout.addLayout(self.hboxlayout,2,0,1,2)

        self.vboxlayout2 = QtGui.QVBoxLayout()
        self.vboxlayout2.setObjectName("vboxlayout2")

        self.hboxlayout2 = QtGui.QHBoxLayout()
        self.hboxlayout2.setObjectName("hboxlayout2")

        self.rdoField = QtGui.QRadioButton(Dialog)
        self.rdoField.setObjectName("rdoField")
        self.hboxlayout2.addWidget(self.rdoField)

        self.label_4 = QtGui.QLabel(Dialog)
        self.label_4.setObjectName("label_4")
        self.hboxlayout2.addWidget(self.label_4)

        spacerItem = QtGui.QSpacerItem(40,20,QtGui.QSizePolicy.Expanding,QtGui.QSizePolicy.Minimum)
        self.hboxlayout2.addItem(spacerItem)
        self.vboxlayout2.addLayout(self.hboxlayout2)

        self.attrib = QtGui.QComboBox(Dialog)
        self.attrib.setEnabled(False)

        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Minimum,QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.attrib.sizePolicy().hasHeightForWidth())
        self.attrib.setSizePolicy(sizePolicy)
        self.attrib.setObjectName("attrib")
        self.vboxlayout2.addWidget(self.attrib)
        self.gridlayout.addLayout(self.vboxlayout2,3,0,1,2)

        self.hboxlayout3 = QtGui.QHBoxLayout()
        self.hboxlayout3.setSpacing(6)
        self.hboxlayout3.setMargin(0)
        self.hboxlayout3.setObjectName("hboxlayout3")

        self.mergeOutput = QtGui.QCheckBox(Dialog)
        self.mergeOutput.setEnabled(True)
        self.mergeOutput.setObjectName("mergeOutput")
        self.hboxlayout3.addWidget(self.mergeOutput)
        self.gridlayout.addLayout(self.hboxlayout3,4,0,1,1)

        spacerItem1 = QtGui.QSpacerItem(20,40,QtGui.QSizePolicy.Minimum,QtGui.QSizePolicy.Expanding)
        self.gridlayout.addItem(spacerItem1,5,0,1,1)

        self.vboxlayout3 = QtGui.QVBoxLayout()
        self.vboxlayout3.setObjectName("vboxlayout3")

        self.label_5 = QtGui.QLabel(Dialog)
        self.label_5.setObjectName("label_5")
        self.vboxlayout3.addWidget(self.label_5)

        self.hboxlayout4 = QtGui.QHBoxLayout()
        self.hboxlayout4.setObjectName("hboxlayout4")

        self.outShape = QtGui.QLineEdit(Dialog)
        self.outShape.setReadOnly(True)
        self.outShape.setObjectName("outShape")
        self.hboxlayout4.addWidget(self.outShape)

        self.btnBrowse = QtGui.QPushButton(Dialog)
        self.btnBrowse.setObjectName("btnBrowse")
        self.hboxlayout4.addWidget(self.btnBrowse)
        self.vboxlayout3.addLayout(self.hboxlayout4)
        self.gridlayout.addLayout(self.vboxlayout3,6,0,1,2)

        self.progressBar = QtGui.QProgressBar(Dialog)
        self.progressBar.setProperty("value",QtCore.QVariant(0))
        self.progressBar.setAlignment(QtCore.Qt.AlignCenter)
        self.progressBar.setOrientation(QtCore.Qt.Horizontal)
        self.progressBar.setObjectName("progressBar")
        self.gridlayout.addWidget(self.progressBar,7,0,1,1)

        self.buttonBox_2 = QtGui.QDialogButtonBox(Dialog)
        self.buttonBox_2.setStandardButtons(QtGui.QDialogButtonBox.Close|QtGui.QDialogButtonBox.Ok)
        self.buttonBox_2.setObjectName("buttonBox_2")
        self.gridlayout.addWidget(self.buttonBox_2,7,1,1,1)

        self.retranslateUi(Dialog)
        QtCore.QObject.connect(self.rdoField,QtCore.SIGNAL("toggled(bool)"),self.attrib.setEnabled)
        QtCore.QObject.connect(self.rdoBuffer,QtCore.SIGNAL("toggled(bool)"),self.param.setEnabled)
        QtCore.QObject.connect(self.buttonBox_2,QtCore.SIGNAL("rejected()"),Dialog.reject)
        QtCore.QObject.connect(self.buttonBox_2,QtCore.SIGNAL("accepted()"),Dialog.accept)
        QtCore.QMetaObject.connectSlotsByName(Dialog)

    def retranslateUi(self, Dialog):
        Dialog.setWindowTitle(QtGui.QApplication.translate("Dialog", "Geoprocessing", None, QtGui.QApplication.UnicodeUTF8))
        self.label_1.setText(QtGui.QApplication.translate("Dialog", "Input vector layer", None, QtGui.QApplication.UnicodeUTF8))
        self.label_2.setText(QtGui.QApplication.translate("Dialog", "Intersect layer", None, QtGui.QApplication.UnicodeUTF8))
        self.rdoBuffer.setText(QtGui.QApplication.translate("Dialog", "Buffer distance", None, QtGui.QApplication.UnicodeUTF8))
        self.rdoField.setText(QtGui.QApplication.translate("Dialog", "Buffer distance field", None, QtGui.QApplication.UnicodeUTF8))
        self.label_4.setText(QtGui.QApplication.translate("Dialog", "Dissolve field", None, QtGui.QApplication.UnicodeUTF8))
        self.mergeOutput.setText(QtGui.QApplication.translate("Dialog", "Dissolve buffer results", None, QtGui.QApplication.UnicodeUTF8))
        self.label_5.setText(QtGui.QApplication.translate("Dialog", "Output shapefile", None, QtGui.QApplication.UnicodeUTF8))
        self.btnBrowse.setText(QtGui.QApplication.translate("Dialog", "Browse", None, QtGui.QApplication.UnicodeUTF8))

