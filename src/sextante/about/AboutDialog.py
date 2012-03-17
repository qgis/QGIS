from PyQt4 import QtCore, QtGui, QtWebKit
from PyQt4.QtCore import *
from PyQt4.QtGui import *
import os

class AboutDialog(QtGui.QDialog):

    def __init__(self):
        QtGui.QDialog.__init__(self)
        self.setModal(True)
        self.setupUi()

    def setupUi(self):
        self.setObjectName("AboutDialog")
        self.resize(500, 400)
        self.webView = QtWebKit.QWebView()
        self.webView.setObjectName("webView")
        self.setWindowTitle("About SEXTANTE")
        self.horizontalLayout= QtGui.QHBoxLayout()
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setMargin(0)
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.horizontalLayout.addWidget(self.webView)
        self.setLayout(self.horizontalLayout)
        filename = os.path.dirname(__file__) + "/about.htm"
        url = QtCore.QUrl(filename)
        self.webView.load(url)

