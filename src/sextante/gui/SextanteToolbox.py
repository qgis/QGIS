

from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4 import QtCore, QtGui
from sextante.core.Sextante import Sextante
from sextante.gui.ParametersDialog import ParametersDialog
import copy
from sextante.core.QGisLayers import QGisLayers

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s


class SextanteToolbox(QtGui.QDockWidget):
    def __init__(self, iface):
        QtGui.QDialog.__init__(self)
        self.iface=iface
        #      self.setModal(True)
        self.ui = Ui_SextanteToolbox()
        self.ui.setupUi(self)

class Ui_SextanteToolbox(object):

    def setupUi(self, SextanteToolbox):
        self.toolbox = SextanteToolbox
        SextanteToolbox.setFloating(False)
        SextanteToolbox.setObjectName(_fromUtf8("SextanteToolbox"))
        SextanteToolbox.resize(400, 500)
        SextanteToolbox.setWindowTitle("SEXTANTE Toolbox")
        self.contents = QtGui.QWidget()
        self.contents.setObjectName(_fromUtf8("contents"))
        self.verticalLayout = QtGui.QVBoxLayout(self.contents)
        self.verticalLayout.setSpacing(2)
        self.verticalLayout.setMargin(0)
        self.verticalLayout.setObjectName(_fromUtf8("verticalLayout"))
        self.searchBox = QtGui.QLineEdit(self.contents)
        self.searchBox.setObjectName(_fromUtf8("searchBox"))
        self.searchBox.textChanged.connect(self.fillTree)
        self.verticalLayout.addWidget(self.searchBox)

        self.algorithmTree = QtGui.QTreeWidget(self.contents)
        self.algorithmTree.setHeaderHidden(True)
        self.algorithmTree.setObjectName(_fromUtf8("algorithmTree"))
        self.algorithmTree.setContextMenuPolicy(Qt.CustomContextMenu)
        self.fillTree()
        self.toolbox.connect(self.algorithmTree,SIGNAL('customContextMenuRequested(QPoint)'),
                     self.showPopupMenu)
        self.verticalLayout.addWidget(self.algorithmTree)
        self.algorithmTree.doubleClicked.connect(self.executeAlgorithm)
        self.toolbox.setWidget(self.contents)
        self.toolbox.iface.addDockWidget(Qt.RightDockWidgetArea, self.toolbox)
        QtCore.QMetaObject.connectSlotsByName(SextanteToolbox)

    def showPopupMenu(self,point):
        item = self.algorithmTree.itemAt(point)
        if isinstance(item, TreeAlgorithmItem):
            alg = item.alg
            popupmenu = QMenu()
            executeAction = QtGui.QAction("Execute", self.algorithmTree)
            executeAction.triggered.connect(self.executeAlgorithm)
            popupmenu.addAction(executeAction)
            executeBatchAction = QtGui.QAction("Execute as batch process", self.algorithmTree)
            executeBatchAction.triggered.connect(self.executeAlgorithmAsBatchProcess)
            popupmenu.addAction(executeBatchAction)
            actions = Sextante.contextMenuActions
            for action in actions:
                if action.isEnabled(alg):
                    contextMenuAction = QtGui.QAction(action.name, self.algorithmTree)
                    contextMenuAction.triggered.connect(action.execute)
                    popupmenu.addAction(contextMenuAction)

            popupmenu.exec_(self.algorithmTree.mapToGlobal(point))


    def executeAlgorithmAsBatchProcess(self):
        pass

    def executeAlgorithm(self):
        item = self.algorithmTree.currentItem()
        if isinstance(item, TreeAlgorithmItem):
            alg = Sextante.getAlgorithm(item.alg.commandLineName())
            alg = copy.deepcopy(alg)
            dlg = ParametersDialog(alg)
            dlg.exec_()
            if dlg.alg != None:
                QMessageBox.critical(None, "hola", str(dlg.alg))
                dlg.alg.processAlgorithm()
        if isinstance(item, TreeActionItem):
            action = item.action
            action.execute()

    def fillTree(self):
        self.algorithmTree.clear()
        text = str(self.searchBox.text())
        for providerName in Sextante.algs.keys():
            groups = {}
            provider = Sextante.algs[providerName]
            algs = provider.values()
            #add algorithms
            for alg in algs:
                if text =="" or text.lower() in alg.name.lower():
                    if alg.group in groups:
                        groupItem = groups[alg.group]
                    else:
                        groupItem = QtGui.QTreeWidgetItem()
                        groupItem.setText(0,alg.group)
                        groups[alg.group] = groupItem
                    algItem = TreeAlgorithmItem(alg)
                    groupItem.addChild(algItem)
            #add actions
            actions = Sextante.actions[providerName]
            for action in actions:
                if text =="" or text.lower() in action.name.lower():
                    if action.group in groups:
                        groupItem = groups[action.group]
                    else:
                        groupItem = QtGui.QTreeWidgetItem()
                        groupItem.setText(0,action.group)
                        groups[action.group] = groupItem
                    algItem = TreeActionItem(action)
                    groupItem.addChild(algItem)

            providerItem = QtGui.QTreeWidgetItem()
            providerItem.setText(0,providerName)

            for groupItem in groups.values():
                #groupItem.sortChildren(0, Qt.AscendingOrder)
                providerItem.addChild(groupItem)
            self.algorithmTree.addTopLevelItem(providerItem)
            providerItem.setExpanded(True)
            for groupItem in groups.values():
                if text != "":
                    groupItem.setExpanded(True)
            self.algorithmTree.sortItems(0, Qt.AscendingOrder)


class TreeAlgorithmItem(QtGui.QTreeWidgetItem):

    def __init__(self, alg):
        QTreeWidgetItem.__init__(self)
        self.alg = alg
        self.setText(0, alg.name)
        self.setIcon(0, alg.icon)


class TreeActionItem(QtGui.QTreeWidgetItem):

    def __init__(self, action):
        QTreeWidgetItem.__init__(self)
        self.action = action
        self.setText(0, action.name)
        self.setIcon(0, action.icon)

