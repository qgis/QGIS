from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4 import QtCore, QtGui
from sextante.core.SextanteUtils import SextanteUtils

class HistoryDialog(QtGui.QDialog):
    def __init__(self):
        QtGui.QDialog.__init__(self)
        self.setModal(True)
        self.setupUi()

    def setupUi(self):
        self.setObjectName("HistoryDialog")
        self.resize(650, 505)
        self.tree = QtGui.QTreeWidget(self)
        self.tree.setGeometry(QtCore.QRect(5, 5, 640, 245))
        self.tree.setHeaderHidden(True)
        self.tree.doubleClicked.connect(self.executeAlgorithm)
        QObject.connect(self.tree, QtCore.SIGNAL("clicked()"), self.changeText)
        self.fillTree()
        self.text = QtGui.QTextEdit(self)
        self.text.setGeometry(QtCore.QRect(5, 260, 640, 245))
        self.text.setObjectName("text")
        self.text.setReadOnly(True)
        self.setWindowTitle("History")
        QtCore.QMetaObject.connectSlotsByName(self)

    def fillTree(self):
        elements = SextanteUtils.getLogEntries()
        for category in elements.keys():
            groupItem = QtGui.QTreeWidgetItem()
            groupItem.setText(0,category)
            for entry in elements[category]:
                item = TreeLogEntryItem(entry, category==SextanteUtils.LOG_ALGORITHM)
                groupItem.addChild(item)
            self.tree.addTopLevelItem(groupItem)


    def executeAlgorithm(self):
        item = self.tree.currentItem()
        if isinstance(item, TreeLogEntryItem):
            if item.isAlg:
                script = "from sextante.core.Sextante import Sextante\n"
                script+=item.entry.text.replace("runalg(","runandload(")
                exec(script)


    def changeText(self):
        item = self.tree.currentItem()
        if isinstance(item, TreeLogEntryItem):
                self.text.setText(item.entry.msg)


class TreeLogEntryItem(QtGui.QTreeWidgetItem):
    def __init__(self, entry, isAlg):
        QTreeWidgetItem.__init__(self)
        self.entry = entry
        self.isAlg = isAlg
        self.setText(0, "[" + entry.date + "]" + entry.text)#.split("\n")[0])
