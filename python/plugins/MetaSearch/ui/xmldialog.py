# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file '/home/tkralidi/work/foss4g/MetaSearch/MetaSearch/plugin/MetaSearch/ui/xmldialog.ui'
#
# Created: Thu Mar 20 21:56:35 2014
#      by: PyQt4 UI code generator 4.9.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_XMLDialog(object):
    def setupUi(self, XMLDialog):
        XMLDialog.setObjectName(_fromUtf8("XMLDialog"))
        XMLDialog.resize(812, 767)
        self.verticalLayout = QtGui.QVBoxLayout(XMLDialog)
        self.verticalLayout.setObjectName(_fromUtf8("verticalLayout"))
        self.lblXMLRequest = QtGui.QLabel(XMLDialog)
        self.lblXMLRequest.setObjectName(_fromUtf8("lblXMLRequest"))
        self.verticalLayout.addWidget(self.lblXMLRequest)
        self.txtbrXMLRequest = QtGui.QTextBrowser(XMLDialog)
        self.txtbrXMLRequest.setObjectName(_fromUtf8("txtbrXMLRequest"))
        self.verticalLayout.addWidget(self.txtbrXMLRequest)
        self.lblXMLResponse = QtGui.QLabel(XMLDialog)
        self.lblXMLResponse.setObjectName(_fromUtf8("lblXMLResponse"))
        self.verticalLayout.addWidget(self.lblXMLResponse)
        self.txtbrXMLResponse = QtGui.QTextBrowser(XMLDialog)
        self.txtbrXMLResponse.setLineWrapMode(QtGui.QTextEdit.NoWrap)
        self.txtbrXMLResponse.setOpenExternalLinks(True)
        self.txtbrXMLResponse.setObjectName(_fromUtf8("txtbrXMLResponse"))
        self.verticalLayout.addWidget(self.txtbrXMLResponse)
        self.buttonBox = QtGui.QDialogButtonBox(XMLDialog)
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Close)
        self.buttonBox.setObjectName(_fromUtf8("buttonBox"))
        self.verticalLayout.addWidget(self.buttonBox)

        self.retranslateUi(XMLDialog)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("accepted()")), XMLDialog.accept)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("rejected()")), XMLDialog.reject)
        QtCore.QMetaObject.connectSlotsByName(XMLDialog)

    def retranslateUi(self, XMLDialog):
        XMLDialog.setWindowTitle(QtGui.QApplication.translate("XMLDialog", "XML Request / Response", None, QtGui.QApplication.UnicodeUTF8))
        self.lblXMLRequest.setText(QtGui.QApplication.translate("XMLDialog", "Request", None, QtGui.QApplication.UnicodeUTF8))
        self.lblXMLResponse.setText(QtGui.QApplication.translate("XMLDialog", "Response", None, QtGui.QApplication.UnicodeUTF8))

