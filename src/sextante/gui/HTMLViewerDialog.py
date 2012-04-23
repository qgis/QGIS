from PyQt4 import QtCore, QtGui, QtWebKit
from PyQt4.QtCore import *
from PyQt4.QtGui import *

class HTMLViewerDialog(QtGui.QDialog):

    def __init__(self, filename):
        QtGui.QDialog.__init__(self)
        self.filename = filename
        self.setModal(True)
        self.setupUi()

    def setupUi(self):
        self.resize(600, 500)
        self.webView = QtWebKit.QWebView()
        self.setWindowTitle("Help")
        self.closeButton = QtGui.QPushButton()
        self.closeButton.setText("Close")
        self.closeButton.setMaximumWidth(150)
        self.horizontalLayout= QtGui.QHBoxLayout()
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setMargin(0)
        self.horizontalLayout.addStretch(1000)
        self.horizontalLayout.addWidget(self.closeButton)
        self.verticalLayout= QtGui.QVBoxLayout()
        self.verticalLayout.setSpacing(2)
        self.verticalLayout.setMargin(0)
        self.verticalLayout.addWidget(self.webView)
        self.verticalLayout.addLayout(self.horizontalLayout)
        self.setLayout(self.verticalLayout)
        QObject.connect(self.closeButton, QtCore.SIGNAL("clicked()"), self.closeWindow)
        try:
            url = QtCore.QUrl(self.filename)
            self.webView.load(url)
        except:
            pass

    def closeWindow(self):
        self.close()
