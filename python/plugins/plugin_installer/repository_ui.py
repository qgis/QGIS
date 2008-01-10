# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'repository.ui'
#
# Created: Thu Jan 10 14:22:08 2008
#      by: PyQt4 UI code generator 4.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

class Ui_RepositoryDetailsDialog(object):
    def setupUi(self, RepositoryDetailsDialog):
        RepositoryDetailsDialog.setObjectName("RepositoryDetailsDialog")
        RepositoryDetailsDialog.resize(QtCore.QSize(QtCore.QRect(0,0,395,169).size()).expandedTo(RepositoryDetailsDialog.minimumSizeHint()))

        self.gridlayout = QtGui.QGridLayout(RepositoryDetailsDialog)
        self.gridlayout.setObjectName("gridlayout")

        self.label = QtGui.QLabel(RepositoryDetailsDialog)
        self.label.setObjectName("label")
        self.gridlayout.addWidget(self.label,0,0,1,1)

        self.editName = QtGui.QLineEdit(RepositoryDetailsDialog)
        self.editName.setObjectName("editName")
        self.gridlayout.addWidget(self.editName,0,1,1,1)

        self.label_2 = QtGui.QLabel(RepositoryDetailsDialog)
        self.label_2.setObjectName("label_2")
        self.gridlayout.addWidget(self.label_2,1,0,1,1)

        self.editURL = QtGui.QLineEdit(RepositoryDetailsDialog)
        self.editURL.setObjectName("editURL")
        self.gridlayout.addWidget(self.editURL,1,1,1,1)

        spacerItem = QtGui.QSpacerItem(181,40,QtGui.QSizePolicy.Minimum,QtGui.QSizePolicy.Expanding)
        self.gridlayout.addItem(spacerItem,2,0,1,2)

        self.buttonBox = QtGui.QDialogButtonBox(RepositoryDetailsDialog)
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.NoButton|QtGui.QDialogButtonBox.Ok)
        self.buttonBox.setObjectName("buttonBox")
        self.gridlayout.addWidget(self.buttonBox,3,0,1,2)

        self.retranslateUi(RepositoryDetailsDialog)
        QtCore.QObject.connect(self.buttonBox,QtCore.SIGNAL("accepted()"),RepositoryDetailsDialog.accept)
        QtCore.QObject.connect(self.buttonBox,QtCore.SIGNAL("rejected()"),RepositoryDetailsDialog.reject)
        QtCore.QMetaObject.connectSlotsByName(RepositoryDetailsDialog)

    def retranslateUi(self, RepositoryDetailsDialog):
        RepositoryDetailsDialog.setWindowTitle(QtGui.QApplication.translate("RepositoryDetailsDialog", "Repository details", None, QtGui.QApplication.UnicodeUTF8))
        self.label.setText(QtGui.QApplication.translate("RepositoryDetailsDialog", "Name:", None, QtGui.QApplication.UnicodeUTF8))
        self.label_2.setText(QtGui.QApplication.translate("RepositoryDetailsDialog", "URL:", None, QtGui.QApplication.UnicodeUTF8))
        self.editURL.setText(QtGui.QApplication.translate("RepositoryDetailsDialog", "http://", None, QtGui.QApplication.UnicodeUTF8))

