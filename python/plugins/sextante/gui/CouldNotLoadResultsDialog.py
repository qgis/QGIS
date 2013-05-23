# -*- coding: utf-8 -*-

"""
***************************************************************************
    CouldNotLoadResultsDialog.py
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

from PyQt4 import QtCore, QtGui, QtWebKit
from PyQt4.QtCore import *
from PyQt4.QtGui import *
import webbrowser

class CouldNotLoadResultsDialog(QtGui.QDialog):
    def __init__(self, wrongLayers, alg):
        QtGui.QDialog.__init__(self, None, QtCore.Qt.WindowSystemMenuHint | QtCore.Qt.WindowTitleHint)
        self.alg = alg
        self.wrongLayers = wrongLayers
        self.setupUi()

    def setupUi(self):
        self.resize(800,400)
        self.setWindowTitle("Problem loading output layers")
        layout = QVBoxLayout()
        webView = QtWebKit.QWebView()
        webView.page().setLinkDelegationPolicy(QtWebKit.QWebPage.DelegateAllLinks)
        webView.connect(webView, SIGNAL("linkClicked(const QUrl&)"), self.linkClicked)
        html = self.alg.provider.getPostProcessingErrorMessage(self.wrongLayers)
        webView.setHtml(html)
        closeButton = QtGui.QPushButton()
        closeButton.setText("Close")
        QObject.connect(closeButton, QtCore.SIGNAL("clicked()"), self.closeButtonPressed)
        layout.addWidget(webView)
        layout.addWidget(closeButton)
        self.setLayout(layout)
        QtCore.QMetaObject.connectSlotsByName(self)

    def linkClicked(self, url):
        webbrowser.open(str(url.toString()))

    def closeButtonPressed(self):
        self.close()