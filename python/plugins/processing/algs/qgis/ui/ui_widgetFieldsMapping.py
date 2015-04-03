# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'widgetFieldsMapping.ui'
#
# Created: Tue Jan 20 10:14:41 2015
#      by: PyQt4 UI code generator 4.10.4
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    def _fromUtf8(s):
        return s

try:
    _encoding = QtGui.QApplication.UnicodeUTF8
    def _translate(context, text, disambig):
        return QtGui.QApplication.translate(context, text, disambig, _encoding)
except AttributeError:
    def _translate(context, text, disambig):
        return QtGui.QApplication.translate(context, text, disambig)

class Ui_Form(object):
    def setupUi(self, Form):
        Form.setObjectName(_fromUtf8("Form"))
        Form.resize(590, 552)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Preferred, QtGui.QSizePolicy.Expanding)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(Form.sizePolicy().hasHeightForWidth())
        Form.setSizePolicy(sizePolicy)
        self.verticalLayout_2 = QtGui.QVBoxLayout(Form)
        self.verticalLayout_2.setMargin(0)
        self.verticalLayout_2.setObjectName(_fromUtf8("verticalLayout_2"))
        self.horizontalLayout = QtGui.QHBoxLayout()
        self.horizontalLayout.setObjectName(_fromUtf8("horizontalLayout"))
        self.fieldsView = QtGui.QTableView(Form)
        self.fieldsView.setSelectionMode(QtGui.QAbstractItemView.SingleSelection)
        self.fieldsView.setSelectionBehavior(QtGui.QAbstractItemView.SelectRows)
        self.fieldsView.setObjectName(_fromUtf8("fieldsView"))
        self.horizontalLayout.addWidget(self.fieldsView)
        self.buttonLayout = QtGui.QVBoxLayout()
        self.buttonLayout.setObjectName(_fromUtf8("buttonLayout"))
        self.addButton = QtGui.QToolButton(Form)
        self.addButton.setObjectName(_fromUtf8("addButton"))
        self.buttonLayout.addWidget(self.addButton)
        self.deleteButton = QtGui.QToolButton(Form)
        self.deleteButton.setObjectName(_fromUtf8("deleteButton"))
        self.buttonLayout.addWidget(self.deleteButton)
        self.upButton = QtGui.QToolButton(Form)
        self.upButton.setObjectName(_fromUtf8("upButton"))
        self.buttonLayout.addWidget(self.upButton)
        self.downButton = QtGui.QToolButton(Form)
        self.downButton.setObjectName(_fromUtf8("downButton"))
        self.buttonLayout.addWidget(self.downButton)
        self.resetButton = QtGui.QToolButton(Form)
        self.resetButton.setObjectName(_fromUtf8("resetButton"))
        self.buttonLayout.addWidget(self.resetButton)
        spacerItem = QtGui.QSpacerItem(20, 40, QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Expanding)
        self.buttonLayout.addItem(spacerItem)
        self.horizontalLayout.addLayout(self.buttonLayout)
        self.verticalLayout_2.addLayout(self.horizontalLayout)
        self.loadFromLayerLayout = QtGui.QHBoxLayout()
        self.loadFromLayerLayout.setObjectName(_fromUtf8("loadFromLayerLayout"))
        self.loadFromLayerLabel = QtGui.QLabel(Form)
        self.loadFromLayerLabel.setObjectName(_fromUtf8("loadFromLayerLabel"))
        self.loadFromLayerLayout.addWidget(self.loadFromLayerLabel)
        self.layerCombo = QtGui.QComboBox(Form)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.layerCombo.sizePolicy().hasHeightForWidth())
        self.layerCombo.setSizePolicy(sizePolicy)
        self.layerCombo.setObjectName(_fromUtf8("layerCombo"))
        self.loadFromLayerLayout.addWidget(self.layerCombo)
        self.loadLayerFieldsButton = QtGui.QPushButton(Form)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.loadLayerFieldsButton.sizePolicy().hasHeightForWidth())
        self.loadLayerFieldsButton.setSizePolicy(sizePolicy)
        self.loadLayerFieldsButton.setObjectName(_fromUtf8("loadLayerFieldsButton"))
        self.loadFromLayerLayout.addWidget(self.loadLayerFieldsButton)
        self.verticalLayout_2.addLayout(self.loadFromLayerLayout)

        self.retranslateUi(Form)
        QtCore.QMetaObject.connectSlotsByName(Form)

    def retranslateUi(self, Form):
        Form.setWindowTitle(_translate("Form", "Fields", None))
        self.addButton.setToolTip(_translate("Form", "Add new field", None))
        self.addButton.setText(_translate("Form", "add", None))
        self.deleteButton.setToolTip(_translate("Form", "Delete selected field", None))
        self.deleteButton.setText(_translate("Form", "delete", None))
        self.upButton.setToolTip(_translate("Form", "Move selected field up", None))
        self.upButton.setText(_translate("Form", "up", None))
        self.downButton.setToolTip(_translate("Form", "Move selected field down", None))
        self.downButton.setText(_translate("Form", "down", None))
        self.resetButton.setToolTip(_translate("Form", "Reset all fields", None))
        self.resetButton.setText(_translate("Form", "reset", None))
        self.loadFromLayerLabel.setText(_translate("Form", "Load fields from layer", None))
        self.loadLayerFieldsButton.setToolTip(_translate("Form", "Load fields from selected layer", None))
        self.loadLayerFieldsButton.setText(_translate("Form", "Load fields", None))

