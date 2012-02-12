from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4 import QtCore, QtGui
from sextante.core.Sextante import Sextante
from sextante.gui.ParametersDialog import ParametersDialog
import copy
from sextante.core.QGisLayers import QGisLayers
from sextante.gui.AlgorithmExecutor import AlgorithmExecutor, SilentProgress
from sextante.core.SextanteUtils import SextanteUtils
from sextante.gui.BatchProcessingDialog import BatchProcessingDialog
from sextante.gui.ModelerParameterDefinitionDialog import ModelerParameterDefinitionDialog
from sextante.core.ModelerAlgorithm import ModelerAlgorithm

class ModelerDialog(QtGui.QDialog):
    def __init__(self):
        QtGui.QDialog.__init__(self)
        self.setupUi()
        self.setWindowFlags(self.windowFlags() | QtCore.Qt.WindowSystemMenuHint |
                            QtCore.Qt.WindowMinMaxButtonsHint)
        self.alg = ModelerAlgorithm()

    def setupUi(self):
        self.setObjectName("ModelerDialog")
        self.resize(1000, 600)
        self.setWindowTitle("SEXTANTE Modeler")
        self.tabWidget = QtGui.QTabWidget()
        self.tabWidget.setMaximumSize(QtCore.QSize(350, 10000))
        self.tabWidget.setObjectName("tabWidget")

        self.inputsTree = QtGui.QTreeWidget()
        self.inputsTree.setHeaderHidden(True)
        self.inputsTree.setObjectName("inputsTree")
        self.fillInputsTree()
        self.inputsTree.doubleClicked.connect(self.addInput)
        self.tabWidget.addTab(self.inputsTree, "Inputs")

        self.verticalLayout = QtGui.QVBoxLayout()
        self.verticalLayout.setSpacing(2)
        self.verticalLayout.setMargin(0)
        self.verticalLayout.setObjectName("verticalLayout")
        self.searchBox = QtGui.QLineEdit()
        self.searchBox.setObjectName("searchBox")
        self.searchBox.textChanged.connect(self.fillAlgorithmTree)
        self.verticalLayout.addWidget(self.searchBox)
        self.algorithmTree = QtGui.QTreeWidget()
        self.algorithmTree.setHeaderHidden(True)
        self.algorithmTree.setObjectName("algorithmTree")
        self.fillAlgorithmTree()
        self.verticalLayout.addWidget(self.algorithmTree)
        self.algorithmTree.doubleClicked.connect(self.addAlgorithm)

        self.algorithmsTab = QtGui.QWidget()
        self.algorithmsTab.setObjectName("AlgorithmsTab")
        self.algorithmsTab.setLayout(self.verticalLayout)
        self.tabWidget.addTab(self.algorithmsTab, "Algorithms")

        self.textEdit = QtGui.QTextEdit()
        self.textEdit.setObjectName("textEdit")

        self.horizontalLayout = QtGui.QHBoxLayout()
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setMargin(0)
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.horizontalLayout.addWidget(self.tabWidget)
        self.horizontalLayout.addWidget(self.textEdit)

        self.buttonBox = QtGui.QDialogButtonBox()
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setObjectName("buttonBox")
        self.openButton = QtGui.QPushButton()
        self.openButton.setObjectName("openButton")
        self.openButton.setText("Open")
        self.buttonBox.addButton(self.openButton, QtGui.QDialogButtonBox.ActionRole)
        self.saveButton = QtGui.QPushButton()
        self.saveButton.setObjectName("saveButton")
        self.saveButton.setText("Save")
        self.buttonBox.addButton(self.openButton, QtGui.QDialogButtonBox.ActionRole)
        self.buttonBox.addButton(self.saveButton, QtGui.QDialogButtonBox.ActionRole)
        QObject.connect(self.openButton, QtCore.SIGNAL("clicked()"), self.openModel)
        QObject.connect(self.saveButton, QtCore.SIGNAL("clicked()"), self.saveModel)

        self.globalLayout = QtGui.QVBoxLayout()
        self.globalLayout.setSpacing(2)
        self.globalLayout.setMargin(0)
        self.globalLayout.setObjectName("globalLayout")
        self.globalLayout.addLayout(self.horizontalLayout)
        self.globalLayout.addWidget(self.buttonBox)
        self.setLayout(self.globalLayout)
        QtCore.QMetaObject.connectSlotsByName(self)

    def openModel(self):
        pass

    def saveModel(self):
        pass


    def repaintModel(self):
        self.textEdit.setText(str(self.alg))


    def addInput(self):
        item = self.inputsTree.currentItem()
        paramType = str(item.text(0))
        if paramType in ModelerParameterDefinitionDialog.paramTypes:
            dlg = ModelerParameterDefinitionDialog(self.alg, paramType)
            dlg.exec_()
            if dlg.param != None:
                self.alg.putParameter(dlg.param)
                self.repaintModel()

    def fillInputsTree(self):
        inputsItem = QtGui.QTreeWidgetItem()
        inputsItem.setText(0,"Inputs")
        for paramType in ModelerParameterDefinitionDialog.paramTypes:
            inputItem = QtGui.QTreeWidgetItem()
            inputItem.setText(0,paramType)
            inputsItem.addChild(inputItem)
        self.inputsTree.addTopLevelItem(inputsItem)
        inputsItem.setExpanded(True)


    def addAlgorithm(self):
        item = self.algorithmTree.currentItem()
        if isinstance(item, TreeAlgorithmItem):
            alg = Sextante.getAlgorithm(item.alg.commandLineName())
            alg = copy.deepcopy(alg)
            dlg = ParametersDialog(alg)
            dlg.exec_()
            if dlg.OK:
                self.alg.addAlgorithm(alg.commandLineName(), dlg.parameters)
                self.repaintModel()

    def fillAlgorithmTree(self):
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

            if len(groups)>0:
                providerItem = QtGui.QTreeWidgetItem()
                providerItem.setText(0,providerName)

                for groupItem in groups.values():
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


