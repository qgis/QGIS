# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'frmJoinAttributes.ui'
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
        Dialog.resize(QtCore.QSize(QtCore.QRect(0,0,377,562).size()).expandedTo(Dialog.minimumSizeHint()))
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

        self.groupBox = QtGui.QGroupBox(Dialog)
        self.groupBox.setObjectName("groupBox")

        self.gridlayout1 = QtGui.QGridLayout(self.groupBox)
        self.gridlayout1.setObjectName("gridlayout1")

        self.vboxlayout2 = QtGui.QVBoxLayout()
        self.vboxlayout2.setObjectName("vboxlayout2")

        self.rdoVector = QtGui.QRadioButton(self.groupBox)
        self.rdoVector.setChecked(True)
        self.rdoVector.setObjectName("rdoVector")
        self.vboxlayout2.addWidget(self.rdoVector)

        self.joinShape = QtGui.QComboBox(self.groupBox)
        self.joinShape.setObjectName("joinShape")
        self.vboxlayout2.addWidget(self.joinShape)
        self.gridlayout1.addLayout(self.vboxlayout2,0,0,1,1)

        self.vboxlayout3 = QtGui.QVBoxLayout()
        self.vboxlayout3.setObjectName("vboxlayout3")

        self.rdoTable = QtGui.QRadioButton(self.groupBox)
        self.rdoTable.setObjectName("rdoTable")
        self.vboxlayout3.addWidget(self.rdoTable)

        self.hboxlayout = QtGui.QHBoxLayout()
        self.hboxlayout.setObjectName("hboxlayout")

        self.inTable = QtGui.QLineEdit(self.groupBox)
        self.inTable.setEnabled(False)
        self.inTable.setReadOnly(True)
        self.inTable.setObjectName("inTable")
        self.hboxlayout.addWidget(self.inTable)

        self.toolTable = QtGui.QToolButton(self.groupBox)
        self.toolTable.setEnabled(False)
        self.toolTable.setObjectName("toolTable")
        self.hboxlayout.addWidget(self.toolTable)
        self.vboxlayout3.addLayout(self.hboxlayout)
        self.gridlayout1.addLayout(self.vboxlayout3,1,0,1,1)

        self.vboxlayout4 = QtGui.QVBoxLayout()
        self.vboxlayout4.setObjectName("vboxlayout4")

        self.label_6 = QtGui.QLabel(self.groupBox)
        self.label_6.setObjectName("label_6")
        self.vboxlayout4.addWidget(self.label_6)

        self.joinField = QtGui.QComboBox(self.groupBox)
        self.joinField.setObjectName("joinField")
        self.vboxlayout4.addWidget(self.joinField)
        self.gridlayout1.addLayout(self.vboxlayout4,2,0,1,1)
        self.gridlayout.addWidget(self.groupBox,2,0,1,2)

        self.vboxlayout5 = QtGui.QVBoxLayout()
        self.vboxlayout5.setObjectName("vboxlayout5")

        self.label_2 = QtGui.QLabel(Dialog)
        self.label_2.setObjectName("label_2")
        self.vboxlayout5.addWidget(self.label_2)

        self.hboxlayout1 = QtGui.QHBoxLayout()
        self.hboxlayout1.setObjectName("hboxlayout1")

        self.outShape = QtGui.QLineEdit(Dialog)
        self.outShape.setReadOnly(True)
        self.outShape.setObjectName("outShape")
        self.hboxlayout1.addWidget(self.outShape)

        self.toolOut = QtGui.QToolButton(Dialog)
        self.toolOut.setObjectName("toolOut")
        self.hboxlayout1.addWidget(self.toolOut)
        self.vboxlayout5.addLayout(self.hboxlayout1)
        self.gridlayout.addLayout(self.vboxlayout5,3,0,1,2)

        self.groupBox_2 = QtGui.QGroupBox(Dialog)
        self.groupBox_2.setObjectName("groupBox_2")

        self.gridlayout2 = QtGui.QGridLayout(self.groupBox_2)
        self.gridlayout2.setObjectName("gridlayout2")

        self.rdoMatch = QtGui.QRadioButton(self.groupBox_2)
        self.rdoMatch.setChecked(True)
        self.rdoMatch.setObjectName("rdoMatch")
        self.gridlayout2.addWidget(self.rdoMatch,0,0,1,1)

        self.rdoKeep = QtGui.QRadioButton(self.groupBox_2)
        self.rdoKeep.setObjectName("rdoKeep")
        self.gridlayout2.addWidget(self.rdoKeep,1,0,1,1)
        self.gridlayout.addWidget(self.groupBox_2,4,0,1,2)

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
        QtCore.QObject.connect(self.rdoTable,QtCore.SIGNAL("clicked()"),self.joinField.clear)
        QtCore.QObject.connect(self.rdoVector,QtCore.SIGNAL("clicked()"),self.joinField.clear)
        QtCore.QObject.connect(self.rdoVector,QtCore.SIGNAL("toggled(bool)"),self.joinShape.setEnabled)
        QtCore.QObject.connect(self.rdoTable,QtCore.SIGNAL("toggled(bool)"),self.inTable.setEnabled)
        QtCore.QObject.connect(self.rdoTable,QtCore.SIGNAL("toggled(bool)"),self.toolTable.setEnabled)
        QtCore.QMetaObject.connectSlotsByName(Dialog)

    def retranslateUi(self, Dialog):
        Dialog.setWindowTitle(QtGui.QApplication.translate("Dialog", "Join Attributes", None, QtGui.QApplication.UnicodeUTF8))
        self.label_3.setText(QtGui.QApplication.translate("Dialog", "Target vector layer", None, QtGui.QApplication.UnicodeUTF8))
        self.label_4.setText(QtGui.QApplication.translate("Dialog", "Target join field", None, QtGui.QApplication.UnicodeUTF8))
        self.groupBox.setTitle(QtGui.QApplication.translate("Dialog", "Join data", None, QtGui.QApplication.UnicodeUTF8))
        self.rdoVector.setText(QtGui.QApplication.translate("Dialog", "Join vector layer", None, QtGui.QApplication.UnicodeUTF8))
        self.rdoTable.setText(QtGui.QApplication.translate("Dialog", "Join dbf table", None, QtGui.QApplication.UnicodeUTF8))
        self.toolTable.setText(QtGui.QApplication.translate("Dialog", "Browse", None, QtGui.QApplication.UnicodeUTF8))
        self.label_6.setText(QtGui.QApplication.translate("Dialog", "Join field", None, QtGui.QApplication.UnicodeUTF8))
        self.label_2.setText(QtGui.QApplication.translate("Dialog", "Output Shapefile", None, QtGui.QApplication.UnicodeUTF8))
        self.toolOut.setText(QtGui.QApplication.translate("Dialog", "Browse", None, QtGui.QApplication.UnicodeUTF8))
        self.groupBox_2.setTitle(QtGui.QApplication.translate("Dialog", "Output table", None, QtGui.QApplication.UnicodeUTF8))
        self.rdoMatch.setText(QtGui.QApplication.translate("Dialog", "Only keep matching records", None, QtGui.QApplication.UnicodeUTF8))
        self.rdoKeep.setText(QtGui.QApplication.translate("Dialog", "Keep all records (includeing non-matching target records)", None, QtGui.QApplication.UnicodeUTF8))

