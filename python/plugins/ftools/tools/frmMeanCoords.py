# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'frmMeanCoords.ui'
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
        Dialog.resize(QtCore.QSize(QtCore.QRect(0,0,374,346).size()).expandedTo(Dialog.minimumSizeHint()))
        Dialog.setSizeGripEnabled(True)

        self.gridlayout = QtGui.QGridLayout(Dialog)
        self.gridlayout.setObjectName("gridlayout")

        self.vboxlayout = QtGui.QVBoxLayout()
        self.vboxlayout.setObjectName("vboxlayout")

        self.label_3 = QtGui.QLabel(Dialog)
        self.label_3.setObjectName("label_3")
        self.vboxlayout.addWidget(self.label_3)

        self.inShape = QtGui.QComboBox(Dialog)
        self.inShape.setObjectName("inShape")
        self.vboxlayout.addWidget(self.inShape)
        self.gridlayout.addLayout(self.vboxlayout,0,0,1,2)

        self.vboxlayout1 = QtGui.QVBoxLayout()
        self.vboxlayout1.setObjectName("vboxlayout1")

        self.label_weight = QtGui.QLabel(Dialog)
        self.label_weight.setObjectName("label_weight")
        self.vboxlayout1.addWidget(self.label_weight)

        self.weightField = QtGui.QComboBox(Dialog)
        self.weightField.setObjectName("weightField")
        self.vboxlayout1.addWidget(self.weightField)
        self.gridlayout.addLayout(self.vboxlayout1,1,0,1,2)

        self.vboxlayout2 = QtGui.QVBoxLayout()
        self.vboxlayout2.setObjectName("vboxlayout2")

        self.label_unique = QtGui.QLabel(Dialog)
        self.label_unique.setObjectName("label_unique")
        self.vboxlayout2.addWidget(self.label_unique)

        self.uniqueField = QtGui.QComboBox(Dialog)
        self.uniqueField.setObjectName("uniqueField")
        self.vboxlayout2.addWidget(self.uniqueField)
        self.gridlayout.addLayout(self.vboxlayout2,2,0,1,2)

        self.hboxlayout = QtGui.QHBoxLayout()
        self.hboxlayout.setObjectName("hboxlayout")

        self.label_size = QtGui.QLabel(Dialog)
        self.label_size.setObjectName("label_size")
        self.hboxlayout.addWidget(self.label_size)

        self.sizeValue = QtGui.QSpinBox(Dialog)
        self.sizeValue.setMinimum(1)
        self.sizeValue.setMaximum(3)
        self.sizeValue.setObjectName("sizeValue")
        self.hboxlayout.addWidget(self.sizeValue)
        self.gridlayout.addLayout(self.hboxlayout,3,0,1,2)

        self.vboxlayout3 = QtGui.QVBoxLayout()
        self.vboxlayout3.setObjectName("vboxlayout3")

        self.label_2 = QtGui.QLabel(Dialog)
        self.label_2.setObjectName("label_2")
        self.vboxlayout3.addWidget(self.label_2)

        self.hboxlayout1 = QtGui.QHBoxLayout()
        self.hboxlayout1.setObjectName("hboxlayout1")

        self.outShape = QtGui.QLineEdit(Dialog)
        self.outShape.setReadOnly(True)
        self.outShape.setObjectName("outShape")
        self.hboxlayout1.addWidget(self.outShape)

        self.toolOut = QtGui.QToolButton(Dialog)
        self.toolOut.setObjectName("toolOut")
        self.hboxlayout1.addWidget(self.toolOut)
        self.vboxlayout3.addLayout(self.hboxlayout1)
        self.gridlayout.addLayout(self.vboxlayout3,4,0,1,2)

        self.progressBar = QtGui.QProgressBar(Dialog)
        self.progressBar.setProperty("value",QtCore.QVariant(0))
        self.progressBar.setAlignment(QtCore.Qt.AlignCenter)
        self.progressBar.setTextVisible(True)
        self.progressBar.setObjectName("progressBar")
        self.gridlayout.addWidget(self.progressBar,5,0,1,1)

        self.buttonBox_2 = QtGui.QDialogButtonBox(Dialog)
        self.buttonBox_2.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox_2.setStandardButtons(QtGui.QDialogButtonBox.Close|QtGui.QDialogButtonBox.Ok)
        self.buttonBox_2.setObjectName("buttonBox_2")
        self.gridlayout.addWidget(self.buttonBox_2,5,1,1,1)

        self.retranslateUi(Dialog)
        QtCore.QObject.connect(self.buttonBox_2,QtCore.SIGNAL("accepted()"),Dialog.accept)
        QtCore.QObject.connect(self.buttonBox_2,QtCore.SIGNAL("rejected()"),Dialog.close)
        QtCore.QMetaObject.connectSlotsByName(Dialog)

    def retranslateUi(self, Dialog):
        Dialog.setWindowTitle(QtGui.QApplication.translate("Dialog", "Generate Centroids", None, QtGui.QApplication.UnicodeUTF8))
        self.label_3.setText(QtGui.QApplication.translate("Dialog", "Input vector layer", None, QtGui.QApplication.UnicodeUTF8))
        self.label_weight.setText(QtGui.QApplication.translate("Dialog", "Weight field", None, QtGui.QApplication.UnicodeUTF8))
        self.label_unique.setText(QtGui.QApplication.translate("Dialog", "Unique ID field", None, QtGui.QApplication.UnicodeUTF8))
        self.label_size.setText(QtGui.QApplication.translate("Dialog", "Number of standard deviations", None, QtGui.QApplication.UnicodeUTF8))
        self.sizeValue.setSuffix(QtGui.QApplication.translate("Dialog", "Std. Dev.", None, QtGui.QApplication.UnicodeUTF8))
        self.label_2.setText(QtGui.QApplication.translate("Dialog", "Output shapefile", None, QtGui.QApplication.UnicodeUTF8))
        self.toolOut.setText(QtGui.QApplication.translate("Dialog", "Browse", None, QtGui.QApplication.UnicodeUTF8))

