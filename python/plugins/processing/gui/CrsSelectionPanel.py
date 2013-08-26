# -*- coding: utf-8 -*-

"""
***************************************************************************
    CrsSelectionPanel.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from PyQt4 import QtGui, QtCore
from processing.gui.CrsSelectionDialog import CrsSelectionDialog

class CrsSelectionPanel(QtGui.QWidget):

    def __init__(self, default):
        super(CrsSelectionPanel, self).__init__(None)
        self.authid = default
        self.horizontalLayout = QtGui.QHBoxLayout(self)
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setMargin(0)
        self.text = QtGui.QLineEdit()
        self.text.setEnabled(False)
        self.text.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)
        self.horizontalLayout.addWidget(self.text)
        self.pushButton = QtGui.QPushButton()
        self.pushButton.setText("...")
        self.pushButton.clicked.connect(self.showSelectionDialog)
        self.horizontalLayout.addWidget(self.pushButton)
        self.setLayout(self.horizontalLayout)
        self.setText()

    def setAuthid(self, authid):
        self.authid = authid
        self.setText()

    def showSelectionDialog(self):
        dialog = CrsSelectionDialog()
        dialog.exec_()
        if dialog.authid:
            self.authid = str(dialog.authid)
            self.setText()

    def setText(self):
        if self.authid is not None:
            self.text.setText(str(self.authid))

    def getValue(self):
        return self.authid
