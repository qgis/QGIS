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
        self.resize(600, 500)
        self.webView = QtWebKit.QWebView()
        self.webView.setObjectName("webView")
        self.setWindowTitle("About SEXTANTE")
        self.verticalLayout= QtGui.QVBoxLayout()
        self.verticalLayout.setSpacing(2)
        self.verticalLayout.setMargin(0)
        self.verticalLayout.setObjectName("horizontalLayout")
        self.verticalLayout.addWidget(self.webView)
        self.closeButton = QtGui.QPushButton()
        self.closeButton.setObjectName("closeButton")
        self.closeButton.setText("Close")
        QObject.connect(self.closeButton, QtCore.SIGNAL("clicked()"), self.closeWindow)
        self.verticalLayout.addWidget(self.closeButton)
        self.setLayout(self.verticalLayout)
        filename = os.path.dirname(__file__) + "/about.htm"
        url = QtCore.QUrl(filename)
        self.webView.load(url)

    def closeWindow(self):
        self.close()