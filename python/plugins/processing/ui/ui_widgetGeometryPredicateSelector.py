# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'widgetGeometryPredicateSelector.ui'
#
# Created: Mon Jan 19 11:52:29 2015
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
        Form.resize(609, 213)
        self.gridLayout = QtGui.QGridLayout(Form)
        self.gridLayout.setMargin(0)
        self.gridLayout.setObjectName(_fromUtf8("gridLayout"))
        self.equalsBox = QtGui.QCheckBox(Form)
        self.equalsBox.setObjectName(_fromUtf8("equalsBox"))
        self.gridLayout.addWidget(self.equalsBox, 0, 0, 1, 1)
        self.containsBox = QtGui.QCheckBox(Form)
        self.containsBox.setObjectName(_fromUtf8("containsBox"))
        self.gridLayout.addWidget(self.containsBox, 1, 0, 1, 1)
        self.touchesBox = QtGui.QCheckBox(Form)
        self.touchesBox.setObjectName(_fromUtf8("touchesBox"))
        self.gridLayout.addWidget(self.touchesBox, 3, 0, 1, 1)
        self.intersectsBox = QtGui.QCheckBox(Form)
        self.intersectsBox.setObjectName(_fromUtf8("intersectsBox"))
        self.gridLayout.addWidget(self.intersectsBox, 2, 0, 1, 1)
        self.withinBox = QtGui.QCheckBox(Form)
        self.withinBox.setObjectName(_fromUtf8("withinBox"))
        self.gridLayout.addWidget(self.withinBox, 0, 1, 1, 1)
        self.overlapsBox = QtGui.QCheckBox(Form)
        self.overlapsBox.setObjectName(_fromUtf8("overlapsBox"))
        self.gridLayout.addWidget(self.overlapsBox, 1, 1, 1, 1)
        self.crossesBox = QtGui.QCheckBox(Form)
        self.crossesBox.setObjectName(_fromUtf8("crossesBox"))
        self.gridLayout.addWidget(self.crossesBox, 2, 1, 1, 1)
        self.disjointBox = QtGui.QCheckBox(Form)
        self.disjointBox.setObjectName(_fromUtf8("disjointBox"))
        self.gridLayout.addWidget(self.disjointBox, 3, 1, 1, 1)

        self.retranslateUi(Form)
        QtCore.QMetaObject.connectSlotsByName(Form)

    def retranslateUi(self, Form):
        Form.setWindowTitle(_translate("Form", "Form", None))
        self.equalsBox.setText(_translate("Form", "equals", None))
        self.containsBox.setText(_translate("Form", "contains", None))
        self.touchesBox.setText(_translate("Form", "touches", None))
        self.intersectsBox.setText(_translate("Form", "intersects", None))
        self.withinBox.setText(_translate("Form", "within", None))
        self.overlapsBox.setText(_translate("Form", "overlaps", None))
        self.crossesBox.setText(_translate("Form", "crosses", None))
        self.disjointBox.setText(_translate("Form", "disjoint", None))

