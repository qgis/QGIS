# -*- coding: utf-8 -*-

"""
***************************************************************************
    NumberInputPanel.py
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

from PyQt4 import QtGui
from sextante.gui.NumberInputDialog import NumberInputDialog

class NumberInputPanel(QtGui.QWidget):

    def __init__(self, number, isInteger):
        super(NumberInputPanel, self).__init__(None)
        self.horizontalLayout = QtGui.QHBoxLayout(self)
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setMargin(0)
        self.isInteger = isInteger
        if isInteger:
            self.spin = QtGui.QSpinBox()
            self.spin.setMaximum(1000000000)
            self.spin.setMinimum(-1000000000)
            self.spin.setValue(number)
            self.horizontalLayout.addWidget(self.spin)
            self.setLayout(self.horizontalLayout)
        else:
            self.text = QtGui.QLineEdit()
            self.text.setText(str(number))
            self.text.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)
            self.horizontalLayout.addWidget(self.text)
            self.pushButton = QtGui.QPushButton()
            self.pushButton.setText("...")
            self.pushButton.clicked.connect(self.showNumberInputDialog)
            self.horizontalLayout.addWidget(self.pushButton)
            self.setLayout(self.horizontalLayout)

    def showNumberInputDialog(self):
        pass
        dlg = NumberInputDialog()
        dlg.exec_()
        if dlg.value != None:
            self.text.setText(str(dlg.value))

    def getValue(self):
        if self.isInteger:
            return self.spin.value()
        else:
            return self.text.text()
