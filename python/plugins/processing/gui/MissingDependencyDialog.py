# -*- coding: utf-8 -*-

"""
***************************************************************************
    MessingDependencyDialog.py
    ---------------------
    Date                 : April 2013
    Copyright            : (C) 2013 by Victor Olaya
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
import webbrowser

__author__ = 'Victor Olaya'
__date__ = 'April 2013'
__copyright__ = '(C) 2013, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from PyQt4 import QtCore, QtGui, QtWebKit
from PyQt4.QtCore import *
from PyQt4.QtGui import *

class MissingDependencyDialog(QtGui.QDialog):
    def __init__(self, msg):
        QtGui.QDialog.__init__(self, None, QtCore.Qt.WindowSystemMenuHint | QtCore.Qt.WindowTitleHint)
        self.msg = "<h3>Missing dependency.This algorithm cannot be run :-( </h3>" + msg
        self.setupUi()

    def setupUi(self):
        self.resize(500,300)
        self.setWindowTitle("Missing dependency")
        layout = QVBoxLayout()
        browser = QtGui.QTextBrowser()
        browser.setOpenLinks(False)
        browser.anchorClicked.connect(self.linkClicked)
        browser.setHtml(self.msg)
        button = QPushButton()
        button.setText("Close")
        button.clicked.connect(self.closeButtonPressed)
        buttonBox = QtGui.QDialogButtonBox()
        buttonBox.setOrientation(QtCore.Qt.Horizontal)
        buttonBox.addButton(button, QDialogButtonBox.ActionRole)
        layout.addWidget(browser)
        layout.addWidget(buttonBox)
        self.setLayout(layout)
        QtCore.QMetaObject.connectSlotsByName(self)

    def linkClicked(self, url):
        webbrowser.open(url.toString())

    def closeButtonPressed(self):
        self.close()