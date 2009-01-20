# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'frmSpatialJoin.ui'
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
        Dialog.resize(QtCore.QSize(QtCore.QRect(0,0,377,466).size()).expandedTo(Dialog.minimumSizeHint()))
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
        self.gridlayout.addLayout(self.vboxlayout,0,0,1,3)

        self.vboxlayout1 = QtGui.QVBoxLayout()
        self.vboxlayout1.setObjectName("vboxlayout1")

        self.label = QtGui.QLabel(Dialog)
        self.label.setObjectName("label")
        self.vboxlayout1.addWidget(self.label)

        self.joinShape = QtGui.QComboBox(Dialog)
        self.joinShape.setObjectName("joinShape")
        self.vboxlayout1.addWidget(self.joinShape)
        self.gridlayout.addLayout(self.vboxlayout1,1,0,1,3)

        self.groupBox_2 = QtGui.QGroupBox(Dialog)
        self.groupBox_2.setObjectName("groupBox_2")

        self.gridlayout1 = QtGui.QGridLayout(self.groupBox_2)
        self.gridlayout1.setObjectName("gridlayout1")

        self.rdoFirst = QtGui.QRadioButton(self.groupBox_2)
        self.rdoFirst.setChecked(True)
        self.rdoFirst.setObjectName("rdoFirst")
        self.gridlayout1.addWidget(self.rdoFirst,0,0,1,6)

        self.rdoSummary = QtGui.QRadioButton(self.groupBox_2)
        self.rdoSummary.setObjectName("rdoSummary")
        self.gridlayout1.addWidget(self.rdoSummary,1,0,1,6)

        spacerItem = QtGui.QSpacerItem(40,20,QtGui.QSizePolicy.Expanding,QtGui.QSizePolicy.Minimum)
        self.gridlayout1.addItem(spacerItem,2,0,1,1)

        self.chkMean = QtGui.QCheckBox(self.groupBox_2)
        self.chkMean.setEnabled(False)
        self.chkMean.setChecked(True)
        self.chkMean.setObjectName("chkMean")
        self.gridlayout1.addWidget(self.chkMean,2,1,1,1)

        self.chkMin = QtGui.QCheckBox(self.groupBox_2)
        self.chkMin.setEnabled(False)
        self.chkMin.setObjectName("chkMin")
        self.gridlayout1.addWidget(self.chkMin,2,2,1,1)

        self.chkMax = QtGui.QCheckBox(self.groupBox_2)
        self.chkMax.setEnabled(False)
        self.chkMax.setObjectName("chkMax")
        self.gridlayout1.addWidget(self.chkMax,2,3,1,1)

        self.chkSum = QtGui.QCheckBox(self.groupBox_2)
        self.chkSum.setEnabled(False)
        self.chkSum.setObjectName("chkSum")
        self.gridlayout1.addWidget(self.chkSum,2,4,1,1)

        spacerItem1 = QtGui.QSpacerItem(40,20,QtGui.QSizePolicy.Expanding,QtGui.QSizePolicy.Minimum)
        self.gridlayout1.addItem(spacerItem1,2,5,1,1)
        self.gridlayout.addWidget(self.groupBox_2,2,0,1,3)

        self.vboxlayout2 = QtGui.QVBoxLayout()
        self.vboxlayout2.setObjectName("vboxlayout2")

        self.label_2 = QtGui.QLabel(Dialog)
        self.label_2.setObjectName("label_2")
        self.vboxlayout2.addWidget(self.label_2)

        self.hboxlayout = QtGui.QHBoxLayout()
        self.hboxlayout.setObjectName("hboxlayout")

        self.outShape = QtGui.QLineEdit(Dialog)
        self.outShape.setReadOnly(True)
        self.outShape.setObjectName("outShape")
        self.hboxlayout.addWidget(self.outShape)

        self.toolOut = QtGui.QToolButton(Dialog)
        self.toolOut.setObjectName("toolOut")
        self.hboxlayout.addWidget(self.toolOut)
        self.vboxlayout2.addLayout(self.hboxlayout)
        self.gridlayout.addLayout(self.vboxlayout2,3,0,1,3)

        self.groupBox_3 = QtGui.QGroupBox(Dialog)
        self.groupBox_3.setObjectName("groupBox_3")

        self.gridlayout2 = QtGui.QGridLayout(self.groupBox_3)
        self.gridlayout2.setObjectName("gridlayout2")

        self.rdoMatch = QtGui.QRadioButton(self.groupBox_3)
        self.rdoMatch.setChecked(True)
        self.rdoMatch.setObjectName("rdoMatch")
        self.gridlayout2.addWidget(self.rdoMatch,0,0,1,1)

        self.rdoKeep = QtGui.QRadioButton(self.groupBox_3)
        self.rdoKeep.setObjectName("rdoKeep")
        self.gridlayout2.addWidget(self.rdoKeep,1,0,1,1)
        self.gridlayout.addWidget(self.groupBox_3,4,0,1,3)

        spacerItem2 = QtGui.QSpacerItem(359,16,QtGui.QSizePolicy.Minimum,QtGui.QSizePolicy.Expanding)
        self.gridlayout.addItem(spacerItem2,5,0,3,3)

        self.progressBar = QtGui.QProgressBar(Dialog)
        self.progressBar.setProperty("value",QtCore.QVariant(24))
        self.progressBar.setAlignment(QtCore.Qt.AlignCenter)
        self.progressBar.setTextVisible(True)
        self.progressBar.setObjectName("progressBar")
        self.gridlayout.addWidget(self.progressBar,7,1,1,1)

        self.buttonBox_2 = QtGui.QDialogButtonBox(Dialog)
        self.buttonBox_2.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox_2.setStandardButtons(QtGui.QDialogButtonBox.Close|QtGui.QDialogButtonBox.Ok)
        self.buttonBox_2.setObjectName("buttonBox_2")
        self.gridlayout.addWidget(self.buttonBox_2,6,2,2,1)

        self.retranslateUi(Dialog)
        QtCore.QObject.connect(self.buttonBox_2,QtCore.SIGNAL("accepted()"),Dialog.accept)
        QtCore.QObject.connect(self.buttonBox_2,QtCore.SIGNAL("rejected()"),Dialog.close)
        QtCore.QObject.connect(self.rdoSummary,QtCore.SIGNAL("toggled(bool)"),self.chkMin.setEnabled)
        QtCore.QObject.connect(self.rdoSummary,QtCore.SIGNAL("toggled(bool)"),self.chkMax.setEnabled)
        QtCore.QObject.connect(self.rdoSummary,QtCore.SIGNAL("toggled(bool)"),self.chkSum.setEnabled)
        QtCore.QObject.connect(self.rdoSummary,QtCore.SIGNAL("toggled(bool)"),self.chkMean.setEnabled)
        QtCore.QMetaObject.connectSlotsByName(Dialog)

    def retranslateUi(self, Dialog):
        Dialog.setWindowTitle(QtGui.QApplication.translate("Dialog", "Spatial Join", None, QtGui.QApplication.UnicodeUTF8))
        self.label_3.setText(QtGui.QApplication.translate("Dialog", "Target vector layer", None, QtGui.QApplication.UnicodeUTF8))
        self.label.setText(QtGui.QApplication.translate("Dialog", "Join vector layer", None, QtGui.QApplication.UnicodeUTF8))
        self.groupBox_2.setTitle(QtGui.QApplication.translate("Dialog", "Attribute Summary", None, QtGui.QApplication.UnicodeUTF8))
        self.rdoFirst.setText(QtGui.QApplication.translate("Dialog", "Take attributes of first located feature", None, QtGui.QApplication.UnicodeUTF8))
        self.rdoSummary.setText(QtGui.QApplication.translate("Dialog", "Take summary of intersecting features", None, QtGui.QApplication.UnicodeUTF8))
        self.chkMean.setText(QtGui.QApplication.translate("Dialog", "Mean", None, QtGui.QApplication.UnicodeUTF8))
        self.chkMin.setText(QtGui.QApplication.translate("Dialog", "Min", None, QtGui.QApplication.UnicodeUTF8))
        self.chkMax.setText(QtGui.QApplication.translate("Dialog", "Max", None, QtGui.QApplication.UnicodeUTF8))
        self.chkSum.setText(QtGui.QApplication.translate("Dialog", "Sum", None, QtGui.QApplication.UnicodeUTF8))
        self.label_2.setText(QtGui.QApplication.translate("Dialog", "Output Shapefile:", None, QtGui.QApplication.UnicodeUTF8))
        self.toolOut.setText(QtGui.QApplication.translate("Dialog", "Browse", None, QtGui.QApplication.UnicodeUTF8))
        self.groupBox_3.setTitle(QtGui.QApplication.translate("Dialog", "Output table", None, QtGui.QApplication.UnicodeUTF8))
        self.rdoMatch.setText(QtGui.QApplication.translate("Dialog", "Only keep matching records", None, QtGui.QApplication.UnicodeUTF8))
        self.rdoKeep.setText(QtGui.QApplication.translate("Dialog", "Keep all records (includeing non-matching target records)", None, QtGui.QApplication.UnicodeUTF8))

