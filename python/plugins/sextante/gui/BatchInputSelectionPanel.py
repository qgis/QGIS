# -*- coding: utf-8 -*-

"""
***************************************************************************
    BatchInputSelectionPanel.py
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

import os
from PyQt4 import QtGui, QtCore
from sextante.parameters.ParameterMultipleInput import ParameterMultipleInput


class BatchInputSelectionPanel(QtGui.QWidget):

    def __init__(self, param, row, col, batchDialog, parent = None):
        super(BatchInputSelectionPanel, self).__init__(parent)
        self.param = param
        self.batchDialog = batchDialog
        self.table = batchDialog.table
        self.row = row
        self.col = col
        self.horizontalLayout = QtGui.QHBoxLayout(self)
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setMargin(0)
        self.text = QtGui.QLineEdit()
        self.text.setText("")
        self.text.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)
        self.horizontalLayout.addWidget(self.text)
        self.pushButton = QtGui.QPushButton()
        self.pushButton.setText("...")
        self.pushButton.clicked.connect(self.showSelectionDialog)
        self.horizontalLayout.addWidget(self.pushButton)
        self.setLayout(self.horizontalLayout)

    def showSelectionDialog(self):
        settings = QtCore.QSettings()
        text = unicode(self.text.text())
        if os.path.isdir(text):
            path = text
        elif os.path.isdir(os.path.dirname(text)):
            path = os.path.dirname(text)
        elif settings.contains("/SextanteQGIS/LastInputPath"):
            path = settings.value( "/SextanteQGIS/LastInputPath")
        else:
            path = ""

        ret = QtGui.QFileDialog.getOpenFileNames(self, "Open file", path, self.param.getFileFilter())
        if ret:
            files = list(ret)
            if len(files) == 1:
                settings.setValue("/SextanteQGIS/LastInputPath", os.path.dirname(str(files[0])))
                self.text.setText(str(files[0]))
            else:
                settings.setValue("/SextanteQGIS/LastInputPath", os.path.dirname(str(files[0])))
                if isinstance(self.param, ParameterMultipleInput):
                    self.text.setText(";".join(str(f) for f in files))
                else:
                    rowdif = len(files) - (self.table.rowCount() - self.row)
                    for i in range(rowdif):
                        self.batchDialog.addRow()
                    for i in range(len(files)):
                        self.table.cellWidget(i+self.row, self.col).setText(files[i])


    def setText(self, text):
        return self.text.setText(text)


    def getText(self):
        return self.text.text()
