# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'frmSubsetSelect.ui'
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
        Dialog.resize(QtCore.QSize(QtCore.QRect(0,0,355,285).size()).expandedTo(Dialog.minimumSizeHint()))
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

        self.label_4 = QtGui.QLabel(Dialog)
        self.label_4.setObjectName("label_4")
        self.vboxlayout1.addWidget(self.label_4)

        self.inField = QtGui.QComboBox(Dialog)
        self.inField.setObjectName("inField")
        self.vboxlayout1.addWidget(self.inField)
        self.gridlayout.addLayout(self.vboxlayout1,1,0,1,2)

        self.vboxlayout2 = QtGui.QVBoxLayout()
        self.vboxlayout2.setObjectName("vboxlayout2")

        self.vboxlayout3 = QtGui.QVBoxLayout()
        self.vboxlayout3.setObjectName("vboxlayout3")

        self.label_2 = QtGui.QLabel(Dialog)
        self.label_2.setObjectName("label_2")
        self.vboxlayout3.addWidget(self.label_2)

        self.hboxlayout = QtGui.QHBoxLayout()
        self.hboxlayout.setObjectName("hboxlayout")

        self.rdoNumber = QtGui.QRadioButton(Dialog)
        self.rdoNumber.setChecked(True)
        self.rdoNumber.setObjectName("rdoNumber")
        self.hboxlayout.addWidget(self.rdoNumber)

        spacerItem = QtGui.QSpacerItem(40,20,QtGui.QSizePolicy.Expanding,QtGui.QSizePolicy.Minimum)
        self.hboxlayout.addItem(spacerItem)

        self.spnNumber = QtGui.QSpinBox(Dialog)
        self.spnNumber.setMinimum(1)
        self.spnNumber.setObjectName("spnNumber")
        self.hboxlayout.addWidget(self.spnNumber)
        self.vboxlayout3.addLayout(self.hboxlayout)
        self.vboxlayout2.addLayout(self.vboxlayout3)

        self.hboxlayout1 = QtGui.QHBoxLayout()
        self.hboxlayout1.setObjectName("hboxlayout1")

        self.rdoPercent = QtGui.QRadioButton(Dialog)
        self.rdoPercent.setObjectName("rdoPercent")
        self.hboxlayout1.addWidget(self.rdoPercent)

        spacerItem1 = QtGui.QSpacerItem(40,20,QtGui.QSizePolicy.Expanding,QtGui.QSizePolicy.Minimum)
        self.hboxlayout1.addItem(spacerItem1)

        self.spnPercent = QtGui.QSpinBox(Dialog)
        self.spnPercent.setMinimum(1)
        self.spnPercent.setMaximum(100)
        self.spnPercent.setProperty("value",QtCore.QVariant(50))
        self.spnPercent.setObjectName("spnPercent")
        self.hboxlayout1.addWidget(self.spnPercent)
        self.vboxlayout2.addLayout(self.hboxlayout1)
        self.gridlayout.addLayout(self.vboxlayout2,2,0,1,2)

        spacerItem2 = QtGui.QSpacerItem(20,20,QtGui.QSizePolicy.Minimum,QtGui.QSizePolicy.Expanding)
        self.gridlayout.addItem(spacerItem2,3,0,1,2)

        self.progressBar = QtGui.QProgressBar(Dialog)
        self.progressBar.setProperty("value",QtCore.QVariant(24))
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
        QtCore.QMetaObject.connectSlotsByName(Dialog)

    def retranslateUi(self, Dialog):
        Dialog.setWindowTitle(QtGui.QApplication.translate("Dialog", "Random Selection From Within Subsets", None, QtGui.QApplication.UnicodeUTF8))
        self.label_3.setText(QtGui.QApplication.translate("Dialog", "Input Vector Layer", None, QtGui.QApplication.UnicodeUTF8))
        self.label_4.setText(QtGui.QApplication.translate("Dialog", "Input subset field (unique ID field)", None, QtGui.QApplication.UnicodeUTF8))
        self.label_2.setText(QtGui.QApplication.translate("Dialog", "Randomly Select", None, QtGui.QApplication.UnicodeUTF8))
        self.rdoNumber.setText(QtGui.QApplication.translate("Dialog", "Number of Features", None, QtGui.QApplication.UnicodeUTF8))
        self.rdoPercent.setText(QtGui.QApplication.translate("Dialog", "Percentage of Features", None, QtGui.QApplication.UnicodeUTF8))
        self.spnPercent.setSuffix(QtGui.QApplication.translate("Dialog", "%", None, QtGui.QApplication.UnicodeUTF8))

