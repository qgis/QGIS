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
        self.setObjectName("AboutDialog")
        self.resize(80, 600)
        self.webView = QtWebKit.QWebView()
        self.webView.setObjectName("webView")
        self.setWindowTitle("")
        self.horizontalLayout= QtGui.QHBoxLayout()
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setMargin(0)
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.horizontalLayout.addWidget(self.webView)
        self.setLayout(self.horizontalLayout)
        url = QtCore.QUrl(self.filename)
        self.webView.load(url)

