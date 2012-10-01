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
