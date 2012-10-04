# -*- coding: utf-8 -*-

"""
***************************************************************************
    help.py
    ---------------------
    Date                 : September 2012
    Copyright            : (C) 2012 by Salvatore Larosa
    Email                : lrssvtml at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Salvatore Larosa'
__date__ = 'September 2012'
__copyright__ = '(C) 2012, Salvatore Larosa'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from PyQt4 import QtCore, QtGui, QtWebKit
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
import os

class HelpDialog(QtGui.QDialog):

    def __init__(self):
        QtGui.QDialog.__init__(self)
        self.setModal(True)
        self.setupUi()
        
    def setupUi(self):
        self.setMaximumSize(500, 300)
        self.webView = QtWebKit.QWebView()
        self.setWindowTitle(QCoreApplication.translate("PythonConsole","Help Python Console"))
        self.verticalLayout= QtGui.QVBoxLayout()
        self.verticalLayout.setSpacing(2)
        self.verticalLayout.setMargin(0)
        self.verticalLayout.addWidget(self.webView)
        self.closeButton = QtGui.QPushButton()
        self.closeButton.setText("Close")
        self.closeButton.setMaximumWidth(150)
        self.horizontalLayout= QtGui.QHBoxLayout()
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setMargin(0)
        self.horizontalLayout.addStretch(1000)
        self.horizontalLayout.addWidget(self.closeButton)
        QObject.connect(self.closeButton, QtCore.SIGNAL("clicked()"), self.closeWindow)
        self.verticalLayout.addLayout(self.horizontalLayout)
        self.setLayout(self.verticalLayout)
        jQueryPath = QgsApplication.pkgDataPath()
        localeFullName = QSettings().value( "locale/userLocale", QVariant( "" ) ).toString()
        filename = os.path.dirname(__file__) + "/helpConsole/help.htm? \
                                                lang=" + localeFullName \
                                                + "&pkgDir=" + jQueryPath
                                                
        url = QtCore.QUrl(filename)
        self.webView.load(url)

    def closeWindow(self):
        self.close()
