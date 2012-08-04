from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4 import QtCore, QtGui, QtWebKit
from sextante.core.SextanteResults import SextanteResults

class ResultsDialog(QtGui.QDialog):
    def __init__(self):
        QtGui.QDialog.__init__(self)
        self.setModal(True)
        self.setupUi()

    def setupUi(self):
        self.resize(800, 600)
        self.tree = QtGui.QTreeWidget()
        self.tree.setHeaderHidden(True)
        self.tree.setMinimumWidth(300)
        QObject.connect(self.tree, QtCore.SIGNAL("itemClicked(QTreeWidgetItem*, int)"), self.changeResult)
        self.groupIcon = QtGui.QIcon()
        self.groupIcon.addPixmap(self.style().standardPixmap(QtGui.QStyle.SP_DirClosedIcon),
                QtGui.QIcon.Normal, QtGui.QIcon.Off)
        self.groupIcon.addPixmap(self.style().standardPixmap(QtGui.QStyle.SP_DirOpenIcon),
                QtGui.QIcon.Normal, QtGui.QIcon.On)
        self.keyIcon = QtGui.QIcon()
        self.keyIcon.addPixmap(self.style().standardPixmap(QtGui.QStyle.SP_FileIcon))
        self.fillTree()
        self.webView = QtWebKit.QWebView()
        self.setWindowTitle("Results")
        self.horizontalLayout= QtGui.QHBoxLayout()
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setMargin(0)
        self.horizontalLayout.addWidget(self.tree)
        self.horizontalLayout.addWidget(self.webView)
        self.closeButton = QtGui.QPushButton()
        self.closeButton.setText("Close")
        self.horizontalLayout2= QtGui.QHBoxLayout()
        self.horizontalLayout2.setSpacing(2)
        self.horizontalLayout2.setMargin(0)
        self.horizontalLayout2.addStretch(1000)
        self.horizontalLayout2.addWidget(self.closeButton)
        QObject.connect(self.closeButton, QtCore.SIGNAL("clicked()"), self.closeWindow)
        self.verticalLayout= QtGui.QVBoxLayout()
        self.verticalLayout.setSpacing(2)
        self.verticalLayout.setMargin(0)
        self.verticalLayout.addLayout(self.horizontalLayout)
        self.verticalLayout.addLayout(self.horizontalLayout2)
        self.setLayout(self.verticalLayout)
        QtCore.QMetaObject.connectSlotsByName(self)
        if self.lastUrl:
            self.webView.load(self.lastUrl)

    def closeWindow(self):
        self.close()

    def fillTree(self):
        elements = SextanteResults.getResults()
        if len(elements) == 0:
            self.lastUrl = None
            return
        for element in elements:
            item = TreeResultItem(element)
            item.setIcon(0, self.keyIcon)
            self.tree.addTopLevelItem(item)
        self.lastUrl = QtCore.QUrl(elements[-1].filename)

    def changeResult(self):
        item = self.tree.currentItem()
        if isinstance(item, TreeResultItem):
            url = QtCore.QUrl(item.filename)
            self.webView.load(url)


class TreeResultItem(QtGui.QTreeWidgetItem):
    def __init__(self, result):
        QTreeWidgetItem.__init__(self)
        self.filename = result.filename
        self.setText(0, result.name)
