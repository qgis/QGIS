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




class Ui_SextanteToolbox(object):

    def setupUi(self, SextanteToolbox):
        self.toolbox = SextanteToolbox
        SextanteToolbox.setObjectName(_fromUtf8("SextanteToolbox"))
        SextanteToolbox.resize(400, 300)
        self.verticalLayoutWidget = QtGui.QWidget(SextanteToolbox)
        self.verticalLayoutWidget.setGeometry(QtCore.QRect(10, 9, 381, 281))
        self.verticalLayoutWidget.setObjectName(_fromUtf8("verticalLayoutWidget"))
        self.verticalLayout = QtGui.QVBoxLayout(self.verticalLayoutWidget)
        self.verticalLayout.setMargin(0)
        self.verticalLayout.setObjectName(_fromUtf8("verticalLayout"))
        self.algorithmTree = QtGui.QTreeWidget(self.verticalLayoutWidget)
        self.algorithmTree.setObjectName(_fromUtf8("algorithmTree"))
        self.algorithmTree.setHeaderHidden(True)
        self.verticalLayout.addWidget(self.algorithmTree)
        self.searchBox = QtGui.QLineEdit(self.verticalLayoutWidget)
        self.searchBox.setObjectName(_fromUtf8("searchBox"))
        self.verticalLayout.addWidget(self.searchBox)
        self.algorithmTree.doubleClicked.connect(self.executeAlgorithm)
        self.fillTree()

        self.retranslateUi(SextanteToolbox)
        QtCore.QMetaObject.connectSlotsByName(SextanteToolbox)

    def executeAlgorithm(self):

        item = self.algorithmTree.currentItem()
        if isinstance(item, TreeAlgorithmItem):
            alg = Sextante.getAlgorithm(item.getAlg().commandLineName())
            alg = copy.deepcopy(alg)
            dlg = ParametersDialog(alg)
            dlg.exec_()


    def fillTree(self):
        groups = {}
        layersCount = QGisLayers.getLayersCount()
        algs = Sextante.algs.values()
        for alg in algs:
            if alg.group in groups:
                groupItem = groups[alg.group]
            else:
                groupItem = QtGui.QTreeWidgetItem()
                groupItem.setText(0,alg.group)
                groups[alg.group] = groupItem
            algItem = TreeAlgorithmItem(alg, layersCount)
            groupItem.addChild(algItem)
        for groupItem in groups.values():
            self.algorithmTree.addTopLevelItem(groupItem)



    def retranslateUi(self, SextantePlugin):
        SextantePlugin.setWindowTitle("SEXTANTE Toolbox")

class TreeAlgorithmItem(QtGui.QTreeWidgetItem):

    def __init__(self, alg, layersCount):
        QTreeWidgetItem.__init__(self)
        self.alg = alg
        self.setText(0,alg.name)
        if not alg.canBeExecuted(layersCount):
            self.setForeground(0, QtGui.QBrush(QtGui.QColor(150,150,150)))
    def getAlg(self):
        return self.alg