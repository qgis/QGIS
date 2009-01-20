# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'frmPointDistance.ui'
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
        Dialog.resize(QtCore.QSize(QtCore.QRect(0,0,382,529).size()).expandedTo(Dialog.minimumSizeHint()))
        Dialog.setSizeGripEnabled(True)

        self.gridlayout = QtGui.QGridLayout(Dialog)
        self.gridlayout.setObjectName("gridlayout")

        self.vboxlayout = QtGui.QVBoxLayout()
        self.vboxlayout.setObjectName("vboxlayout")

        self.label_4 = QtGui.QLabel(Dialog)
        self.label_4.setObjectName("label_4")
        self.vboxlayout.addWidget(self.label_4)

        self.inPoint1 = QtGui.QComboBox(Dialog)
        self.inPoint1.setObjectName("inPoint1")
        self.vboxlayout.addWidget(self.inPoint1)
        self.gridlayout.addLayout(self.vboxlayout,0,0,1,2)

        self.vboxlayout1 = QtGui.QVBoxLayout()
        self.vboxlayout1.setObjectName("vboxlayout1")

        self.label_6 = QtGui.QLabel(Dialog)
        self.label_6.setObjectName("label_6")
        self.vboxlayout1.addWidget(self.label_6)

        self.inField1 = QtGui.QComboBox(Dialog)
        self.inField1.setObjectName("inField1")
        self.vboxlayout1.addWidget(self.inField1)
        self.gridlayout.addLayout(self.vboxlayout1,1,0,1,2)

        self.vboxlayout2 = QtGui.QVBoxLayout()
        self.vboxlayout2.setObjectName("vboxlayout2")

        self.label_3 = QtGui.QLabel(Dialog)
        self.label_3.setObjectName("label_3")
        self.vboxlayout2.addWidget(self.label_3)

        self.inPoint2 = QtGui.QComboBox(Dialog)
        self.inPoint2.setObjectName("inPoint2")
        self.vboxlayout2.addWidget(self.inPoint2)
        self.gridlayout.addLayout(self.vboxlayout2,2,0,1,2)

        self.vboxlayout3 = QtGui.QVBoxLayout()
        self.vboxlayout3.setObjectName("vboxlayout3")

        self.label_5 = QtGui.QLabel(Dialog)
        self.label_5.setObjectName("label_5")
        self.vboxlayout3.addWidget(self.label_5)

        self.inField2 = QtGui.QComboBox(Dialog)
        self.inField2.setObjectName("inField2")
        self.vboxlayout3.addWidget(self.inField2)
        self.gridlayout.addLayout(self.vboxlayout3,3,0,1,2)

        self.groupBox_2 = QtGui.QGroupBox(Dialog)
        self.groupBox_2.setObjectName("groupBox_2")

        self.gridlayout1 = QtGui.QGridLayout(self.groupBox_2)
        self.gridlayout1.setObjectName("gridlayout1")

        self.rdoLinear = QtGui.QRadioButton(self.groupBox_2)
        self.rdoLinear.setChecked(True)
        self.rdoLinear.setObjectName("rdoLinear")
        self.gridlayout1.addWidget(self.rdoLinear,0,0,1,1)

        self.rdoStandard = QtGui.QRadioButton(self.groupBox_2)
        self.rdoStandard.setObjectName("rdoStandard")
        self.gridlayout1.addWidget(self.rdoStandard,1,0,1,1)

        self.rdoSummary = QtGui.QRadioButton(self.groupBox_2)
        self.rdoSummary.setObjectName("rdoSummary")
        self.gridlayout1.addWidget(self.rdoSummary,2,0,1,2)

        self.chkNearest = QtGui.QCheckBox(self.groupBox_2)
        self.chkNearest.setObjectName("chkNearest")
        self.gridlayout1.addWidget(self.chkNearest,3,0,1,1)

        self.spnNearest = QtGui.QSpinBox(self.groupBox_2)
        self.spnNearest.setEnabled(False)
        self.spnNearest.setMinimum(1)
        self.spnNearest.setMaximum(9999)
        self.spnNearest.setObjectName("spnNearest")
        self.gridlayout1.addWidget(self.spnNearest,3,1,1,1)
        self.gridlayout.addWidget(self.groupBox_2,4,0,1,2)

        spacerItem = QtGui.QSpacerItem(20,16,QtGui.QSizePolicy.Minimum,QtGui.QSizePolicy.Expanding)
        self.gridlayout.addItem(spacerItem,5,0,1,1)

        self.vboxlayout4 = QtGui.QVBoxLayout()
        self.vboxlayout4.setObjectName("vboxlayout4")

        self.label_2 = QtGui.QLabel(Dialog)
        self.label_2.setObjectName("label_2")
        self.vboxlayout4.addWidget(self.label_2)

        self.hboxlayout = QtGui.QHBoxLayout()
        self.hboxlayout.setObjectName("hboxlayout")

        self.outFile = QtGui.QLineEdit(Dialog)
        self.outFile.setReadOnly(True)
        self.outFile.setObjectName("outFile")
        self.hboxlayout.addWidget(self.outFile)

        self.btnFile = QtGui.QToolButton(Dialog)
        self.btnFile.setObjectName("btnFile")
        self.hboxlayout.addWidget(self.btnFile)
        self.vboxlayout4.addLayout(self.hboxlayout)
        self.gridlayout.addLayout(self.vboxlayout4,6,0,1,2)

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
        QtCore.QObject.connect(self.chkNearest,QtCore.SIGNAL("toggled(bool)"),self.spnNearest.setEnabled)
        QtCore.QObject.connect(self.rdoStandard,QtCore.SIGNAL("toggled(bool)"),self.chkNearest.setDisabled)
        QtCore.QObject.connect(self.rdoStandard,QtCore.SIGNAL("toggled(bool)"),self.spnNearest.setDisabled)
        QtCore.QMetaObject.connectSlotsByName(Dialog)

    def retranslateUi(self, Dialog):
        Dialog.setWindowTitle(QtGui.QApplication.translate("Dialog", "Create Distance Matrix", None, QtGui.QApplication.UnicodeUTF8))
        self.label_4.setText(QtGui.QApplication.translate("Dialog", "Input point layer", None, QtGui.QApplication.UnicodeUTF8))
        self.label_6.setText(QtGui.QApplication.translate("Dialog", "Input unique ID field", None, QtGui.QApplication.UnicodeUTF8))
        self.label_3.setText(QtGui.QApplication.translate("Dialog", "Target point layer", None, QtGui.QApplication.UnicodeUTF8))
        self.label_5.setText(QtGui.QApplication.translate("Dialog", "Target unique ID field", None, QtGui.QApplication.UnicodeUTF8))
        self.groupBox_2.setTitle(QtGui.QApplication.translate("Dialog", "Output matrix type", None, QtGui.QApplication.UnicodeUTF8))
        self.rdoLinear.setText(QtGui.QApplication.translate("Dialog", "Linear (N*k x 3) distance matrix", None, QtGui.QApplication.UnicodeUTF8))
        self.rdoStandard.setText(QtGui.QApplication.translate("Dialog", "Standard (N x T) distance matrix", None, QtGui.QApplication.UnicodeUTF8))
        self.rdoSummary.setText(QtGui.QApplication.translate("Dialog", "Summary distance matrix (mean, std. dev., min, max)", None, QtGui.QApplication.UnicodeUTF8))
        self.chkNearest.setText(QtGui.QApplication.translate("Dialog", "Use only the nearest (k) target points:", None, QtGui.QApplication.UnicodeUTF8))
        self.label_2.setText(QtGui.QApplication.translate("Dialog", "Output distance matrix", None, QtGui.QApplication.UnicodeUTF8))
        self.btnFile.setText(QtGui.QApplication.translate("Dialog", "Browse", None, QtGui.QApplication.UnicodeUTF8))

