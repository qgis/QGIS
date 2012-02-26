from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4 import QtCore, QtGui
from sextante.core.SextanteLog import SextanteLog

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
        QObject.connect(self.tree, QtCore.SIGNAL("itemClicked(QTreeWidgetItem*, int)"), self.changeText)
        self.groupIcon = QtGui.QIcon()
        self.groupIcon.addPixmap(self.style().standardPixmap(QtGui.QStyle.SP_DirClosedIcon),
                QtGui.QIcon.Normal, QtGui.QIcon.Off)
        self.groupIcon.addPixmap(self.style().standardPixmap(QtGui.QStyle.SP_DirOpenIcon),
                QtGui.QIcon.Normal, QtGui.QIcon.On)
        self.keyIcon = QtGui.QIcon()
        self.keyIcon.addPixmap(self.style().standardPixmap(QtGui.QStyle.SP_FileIcon))
        self.fillTree()
        self.text = QtGui.QTextEdit(self)
        self.text.setGeometry(QtCore.QRect(5, 260, 640, 245))
        self.text.setObjectName("text")
        self.text.setReadOnly(True)
        self.setWindowTitle("History")
        QtCore.QMetaObject.connectSlotsByName(self)

    def fillTree(self):
        elements = SextanteLog.getLogEntries()
        for category in elements.keys():
            groupItem = QtGui.QTreeWidgetItem()
            groupItem.setText(0,category)
            groupItem.setIcon(0, self.groupIcon)
            for entry in elements[category]:
                item = TreeLogEntryItem(entry, category==SextanteLog.LOG_ALGORITHM)
                item.setIcon(0, self.keyIcon)
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
                self.text.setText(item.entry.text.replace("|","\n"))


class TreeLogEntryItem(QtGui.QTreeWidgetItem):
    def __init__(self, entry, isAlg):
        QTreeWidgetItem.__init__(self)
        self.entry = entry
        self.isAlg = isAlg
        self.setText(0, "[" + entry.date + "] " + entry.text.split("|")[0])
