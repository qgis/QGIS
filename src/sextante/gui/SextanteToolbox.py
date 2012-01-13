

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
        treeidx=self.algorithmTree.indexAt(point)
        popupmenu = QMenu()
        executeAction = QtGui.QAction("Execute", self.algorithmTree)
        executeAction.triggered.connect(self.executeAlgorithm)
        popupmenu.addAction(executeAction)
        executeBatchAction = QtGui.QAction("Execute as batch process", self.algorithmTree)
        executeBatchAction.triggered.connect(self.executeAlgorithmAsBatchProcess)
        popupmenu.addAction(executeBatchAction)
        popupmenu.exec_(self.algorithmTree.mapToGlobal(point))

    def executeAlgorithmAsBatchProcess(self):
        pass

    def executeAlgorithm(self):
        item = self.algorithmTree.currentItem()
        if isinstance(item, TreeAlgorithmItem):
            alg = Sextante.getAlgorithm(item.getAlg().commandLineName())
            alg = copy.deepcopy(alg)
            dlg = ParametersDialog(alg)
            dlg.exec_()
            if dlg.alg != None:
                QMessageBox.critical(None, "hola", str(dlg.alg))
                dlg.alg.processAlgorithm()

    def fillTree(self):
        self.algorithmTree.clear()
        text = str(self.searchBox.text())
        layersCount = QGisLayers.getLayersCount()
        for providerName in Sextante.algs.keys():
            groups = {}
            provider = Sextante.algs[providerName]
            algs = provider.values()
            for alg in algs:
                if text =="" or text.lower() in alg.name.lower():
                    if alg.group in groups:
                        groupItem = groups[alg.group]
                    else:
                        groupItem = QtGui.QTreeWidgetItem()
                        groupItem.setText(0,alg.group)
                        groups[alg.group] = groupItem
                    algItem = TreeAlgorithmItem(alg, layersCount)
                    groupItem.addChild(algItem)
            providerItem = QtGui.QTreeWidgetItem()
            providerItem.setText(0,providerName)
            for groupItem in groups.values():
                providerItem.addChild(groupItem)
            self.algorithmTree.addTopLevelItem(providerItem)
            providerItem.setExpanded(True)
            for groupItem in groups.values():
                if text != "":
                    groupItem.setExpanded(True)


class TreeAlgorithmItem(QtGui.QTreeWidgetItem):

    def __init__(self, alg, layersCount):
        QTreeWidgetItem.__init__(self)
        self.alg = alg
        self.setText(0,alg.name)
        if not alg.canBeExecuted(layersCount):
            self.setForeground(0, QtGui.QBrush(QtGui.QColor(150,150,150)))

    def getAlg(self):
        return self.alg