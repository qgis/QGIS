from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4 import QtCore, QtGui, QtWebKit
import os
import pickle

class HelpEditionDialog(QtGui.QDialog):

    ALG_DESC = "ALG_DESC"
    ALG_CREATOR = "ALG_CREATOR"
    ALG_HELP_CREATOR = "ALG_HELP_CREATOR"

    def __init__(self, alg):
        self.alg = alg
        QtGui.QDialog.__init__(self)
        self.setModal(True)
        self.descriptions =  {}
        if self.alg.descriptionFile is not None:
            helpfile = alg.descriptionFile + ".help"
            if os.path.exists(helpfile):
                f = open(helpfile, "rb")
                self.descriptions = pickle.load(f)
        self.currentName = self.ALG_DESC
        self.setupUi()

    def setupUi(self):
        self.resize(700, 500)
        self.tree = QtGui.QTreeWidget()
        self.tree.setHeaderHidden(True)
        self.tree.setMinimumWidth(300)
        QObject.connect(self.tree, QtCore.SIGNAL("itemClicked(QTreeWidgetItem*, int)"), self.changeItem)
        self.groupIcon = QtGui.QIcon()
        self.groupIcon.addPixmap(self.style().standardPixmap(QtGui.QStyle.SP_DirClosedIcon),
                QtGui.QIcon.Normal, QtGui.QIcon.Off)
        self.groupIcon.addPixmap(self.style().standardPixmap(QtGui.QStyle.SP_DirOpenIcon),
                QtGui.QIcon.Normal, QtGui.QIcon.On)
        self.keyIcon = QtGui.QIcon()
        self.keyIcon.addPixmap(self.style().standardPixmap(QtGui.QStyle.SP_FileIcon))
        self.fillTree()
        self.setWindowTitle("Help editor")
        self.horizontalLayout= QtGui.QHBoxLayout()
        self.horizontalLayout.setSpacing(15)
        self.horizontalLayout.setMargin(0)
        self.label = QtGui.QLabel()
        self.label.setText("Select elements on the tree and fill their description in the text box below")
        self.labelName = QtGui.QLabel()
        self.labelName.setText("Algorithm description")
        self.text = QtGui.QTextEdit()
        self.text.setMinimumHeight(200)
        self.verticalLayout= QtGui.QVBoxLayout()
        self.verticalLayout.setSpacing(5)
        self.verticalLayout.setMargin(0)
        self.verticalLayout.addWidget(self.tree)
        self.verticalLayout.addSpacing(20)
        self.verticalLayout.addWidget(self.label)
        self.verticalLayout.addSpacing(20)
        self.verticalLayout.addWidget(self.labelName)
        self.verticalLayout.addWidget(self.text)
        self.horizontalLayout.addLayout(self.verticalLayout)
        self.webView = QtWebKit.QWebView()
        self.webView.setMinimumWidth(300)
        self.webView.setHtml(self.getHtml())
        self.horizontalLayout.addWidget(self.webView)
        self.closeButton = QtGui.QPushButton()
        self.closeButton.setText("Cancel")
        self.saveButton = QtGui.QPushButton()
        self.saveButton.setText("OK")
        self.horizontalLayout2= QtGui.QHBoxLayout()
        self.horizontalLayout2.setSpacing(2)
        self.horizontalLayout2.setMargin(0)
        self.horizontalLayout2.addStretch(1000)
        self.horizontalLayout2.addWidget(self.saveButton)
        self.horizontalLayout2.addWidget(self.closeButton)
        QObject.connect(self.closeButton, QtCore.SIGNAL("clicked()"), self.closeWindow)
        QObject.connect(self.saveButton, QtCore.SIGNAL("clicked()"), self.saveHelp)
        self.verticalLayout2= QtGui.QVBoxLayout()
        self.verticalLayout2.setSpacing(2)
        self.verticalLayout2.setMargin(0)
        self.verticalLayout2.addLayout(self.horizontalLayout)
        self.verticalLayout2.addLayout(self.horizontalLayout2)
        self.setLayout(self.verticalLayout2)
        QtCore.QMetaObject.connectSlotsByName(self)
        self.updateHtmlView()

    def closeWindow(self):
        self.descriptions = None
        self.close()

    def saveHelp(self):
        self.descriptions[self.currentName] = str(self.text.toPlainText())
        if self.alg.descriptionFile is not None:
            f = open(self.alg.descriptionFile + ".help", "wb")
            pickle.dump(self.descriptions, f)
            f.close()
        self.close()

    def getHtml(self):
        s = "<h2>Algorithm description</h2>\n"
        s += "<p>" + self.getDescription(self.ALG_DESC) + "</p>\n"
        s += "<h2>Input parameters</h2>\n"
        for param in self.alg.parameters:
            s += "<h3>" + param.description + "</h3>\n"
            s += "<p>" + self.getDescription(param.name) + "</p>\n"
        s += "<h2>Outputs</h2>\n"
        for out in self.alg.outputs:
            s += "<h3>" + out.description + "</h3>\n"
            s += "<p>" + self.getDescription(out.name) + "</p>\n"
        return s

    def fillTree(self):
        item = TreeDescriptionItem("Algorithm description", self.ALG_DESC)
        self.tree.addTopLevelItem(item)
        parametersItem = TreeDescriptionItem("Input parameters", None)
        self.tree.addTopLevelItem(parametersItem)
        for param in self.alg.parameters:
            item = TreeDescriptionItem(param.description, param.name)
            parametersItem.addChild(item)
        outputsItem = TreeDescriptionItem("Outputs", None)
        self.tree.addTopLevelItem(outputsItem)
        for out in self.alg.outputs:
            item = TreeDescriptionItem(out.description, out.name)
            outputsItem.addChild(item)
        item = TreeDescriptionItem("Algorithm created by", self.ALG_CREATOR)
        self.tree.addTopLevelItem(item)
        item = TreeDescriptionItem("Algorithm help written by", self.ALG_HELP_CREATOR)
        self.tree.addTopLevelItem(item)

    def changeItem(self):
        item = self.tree.currentItem()
        if isinstance(item, TreeDescriptionItem):
            if self.currentName:
                self.descriptions[self.currentName] = str(self.text.toPlainText())
            name = item.name
            if name:
                self.updateHtmlView()
                self.currentName = name
                self.labelName.setText(item.description)
                if name in self.descriptions:
                    self.text.setText(self.descriptions[name])
                else:
                    self.text.setText("")

    def updateHtmlView(self):
        self.webView.setHtml(self.getHtml())

    def getDescription(self, name):
        if name in self.descriptions    :
            return self.descriptions[name]
        else:
            return ""

class TreeDescriptionItem(QtGui.QTreeWidgetItem):
    def __init__(self, description, name):
        QTreeWidgetItem.__init__(self)
        self.name = name
        self.description = description
        self.setText(0, description)
