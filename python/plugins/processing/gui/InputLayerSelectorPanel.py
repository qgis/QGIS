# -*- coding: utf-8 -*-

"""
***************************************************************************
    InputLayerSelectorPanel.py
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

class InputLayerSelectorPanel(QtGui.QWidget):

    def __init__(self, options):
        super(InputLayerSelectorPanel, self).__init__(None)
        self.horizontalLayout = QtGui.QHBoxLayout(self)
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setMargin(0)
        self.text = QtGui.QComboBox()
        for name, value in options:
            self.text.addItem(name, value)
        self.text.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)
        self.horizontalLayout.addWidget(self.text)
        self.pushButton = QtGui.QPushButton()
        self.pushButton.setText("...")
        self.pushButton.clicked.connect(self.showSelectionDialog)
        self.horizontalLayout.addWidget(self.pushButton)
        self.setLayout(self.horizontalLayout)

    def showSelectionDialog(self):
        # find the file dialog's working directory
        settings = QtCore.QSettings()
        text = unicode(self.text.currentText())
        if os.path.isdir(text):
            path = text
        elif os.path.isdir( os.path.dirname(text) ):
            path = os.path.dirname(text)
        elif settings.contains("/ProcessingQGIS/LastInputPath"):
            path = unicode(settings.value( "/ProcessingQGIS/LastInputPath"))
        else:
            path = ""

        filename = QtGui.QFileDialog.getOpenFileName(self, "All files", path, "*.*")
        if filename:
            self.text.addItem(filename, filename)
            self.text.setCurrentIndex(self.text.count() - 1)
            settings.setValue("/ProcessingQGIS/LastInputPath", os.path.dirname(unicode(filename)))

    def getValue(self):
        return self.text.itemData(self.text.currentIndex())
