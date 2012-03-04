from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4 import QtCore, QtGui
from sextante.modeler.ModelerParameterDefinitionDialog import ModelerParameterDefinitionDialog
from sextante.modeler.ModelerAlgorithm import ModelerAlgorithm
from sextante.modeler.ModelerParametersDialog import ModelerParametersDialog
from sextante.modeler.ModelerUtils import ModelerUtils
from sextante.modeler.WrongModelException import WrongModelException
from sextante.modeler.ModelerScene import ModelerScene
import copy
from sextante.modeler.ProviderIcons import ProviderIcons

class ModelerDialog(QtGui.QDialog):
    def __init__(self, alg=None):
        QtGui.QDialog.__init__(self)
        self.setupUi()
        self.setWindowFlags(self.windowFlags() | QtCore.Qt.WindowSystemMenuHint |
                            QtCore.Qt.WindowMinMaxButtonsHint)
        if alg:
            self.alg = alg
            self.textGroup.setText(alg.group)
            self.textName.setText(alg.name)
            self.repaintModel()
            self.view.ensureVisible(self.scene.getLastAlgorithmItem())
        else:
            self.alg = ModelerAlgorithm()
        self.update = False #indicates whether to update or not the toolbox after closing this dialog

    def setupUi(self):
        self.setObjectName("ModelerDialog")
        self.resize(1000, 600)
        self.setWindowTitle("SEXTANTE Modeler")
        self.tabWidget = QtGui.QTabWidget()
        self.tabWidget.setMaximumSize(QtCore.QSize(350, 10000))
        self.tabWidget.setMinimumWidth(300)
        self.tabWidget.setObjectName("tabWidget")

        #right hand side part
        #==================================
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

        #right hand side part
        #==================================
        self.textName = QtGui.QLineEdit()
        self.textName.setText("[Enter model name here]")
        self.textGroup = QtGui.QLineEdit()
        self.textGroup.setText("[Enter group name here]")
        self.horizontalLayoutNames = QtGui.QHBoxLayout()
        self.horizontalLayoutNames.setSpacing(2)
        self.horizontalLayoutNames.setMargin(0)
        self.horizontalLayoutNames.setObjectName("horizontalLayoutNames")
        self.horizontalLayoutNames.addWidget(self.textName)
        self.horizontalLayoutNames.addWidget(self.textGroup)

        self.scene = ModelerScene()
        self.scene.setSceneRect(QtCore.QRectF(0, 0, 2000, 2000))

        self.canvasTabWidget = QtGui.QTabWidget()
        self.canvasTabWidget.setMinimumWidth(300)
        self.canvasTabWidget.setObjectName("canvasTabWidget")
        self.view = QtGui.QGraphicsView(self.scene)
        self.pythonText = QtGui.QTextEdit()
        self.canvasTabWidget.addTab(self.view, "Design" )
        self.canvasTabWidget.addTab(self.pythonText, "Python code" )

        self.canvasLayout = QtGui.QVBoxLayout()
        self.canvasLayout.setSpacing(2)
        self.canvasLayout.setMargin(0)
        self.canvasLayout.setObjectName("canvasLayout")
        self.canvasLayout.addLayout(self.horizontalLayoutNames)
        self.canvasLayout.addWidget(self.canvasTabWidget)

        #upper part, putting the two previous parts together
        #===================================================
        self.horizontalLayout = QtGui.QHBoxLayout()
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setMargin(0)
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.horizontalLayout.addWidget(self.tabWidget)
        self.horizontalLayout.addLayout(self.canvasLayout)

        #And the whole layout
        #==========================
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

        self.view.ensureVisible(0, 0, 10, 10)


    def saveModel(self):
        if str(self.textGroup.text()) == "[Enter group name here]" or str(self.textName.text()) == "[Enter model name here]":
            QMessageBox.warning(self, "Warning", "Please enter group and model names before saving")
            return
        self.alg.setPositions(self.scene.getParameterPositions(), self.scene.getAlgorithmPositions())
        self.alg.name = str(self.textName.text())
        self.alg.group = str(self.textGroup.text())
        if self.alg.descriptionFile!=None:
            filename = self.alg.descriptionFile
        else:
            filename = str(QtGui.QFileDialog.getSaveFileName(self, "Save Model", ModelerUtils.modelsFolder(), "SEXTANTE models (*.model)"))
            if not filename.endswith(".model"):
                filename += ".model"
            self.alg.descriptionFile = filename
        if filename:
            text = self.alg.serialize()
            fout = open(filename, "w")
            fout.write(text)
            fout.close()
            self.update = True

    def openModel(self):
        filename = QtGui.QFileDialog.getOpenFileName(self, "Open Model", ModelerUtils.modelsFolder(), "SEXTANTE models (*.model)")
        if filename:
            try:
                alg = ModelerAlgorithm()
                alg.openModel(filename)
                self.alg = alg;
                self.textGroup.setText(alg.group)
                self.textName.setText(alg.name)
                self.repaintModel()
                self.view.ensureVisible(self.scene.getLastAlgorithmItem())
            except WrongModelException, e:
                QMessageBox.critical(self, "Could not open model", "The selected model could not be loaded\nWrong line:" + e.msg)


    def repaintModel(self):
        self.scene = ModelerScene()
        self.scene.setSceneRect(QtCore.QRectF(0, 0, 1000, 1000))
        self.scene.paintModel(self.alg)
        self.view.setScene(self.scene)
        self.pythonText.setText(self.alg.getAsPythonCode())


    def addInput(self):
        item = self.inputsTree.currentItem()
        paramType = str(item.text(0))
        if paramType in ModelerParameterDefinitionDialog.paramTypes:
            dlg = ModelerParameterDefinitionDialog(self.alg, paramType)
            dlg.exec_()
            if dlg.param != None:
                self.alg.setPositions(self.scene.getParameterPositions(), self.scene.getAlgorithmPositions())
                self.alg.addParameter(dlg.param)
                self.repaintModel()
                self.view.ensureVisible(self.scene.getLastParameterItem())


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
            alg = ModelerUtils.getAlgorithm(item.alg.commandLineName())
            alg = copy.deepcopy(alg)
            dlg = ModelerParametersDialog(alg,self.alg)
            dlg.exec_()
            if dlg.params != None:
                self.alg.setPositions(self.scene.getParameterPositions(), self.scene.getAlgorithmPositions())
                self.alg.addAlgorithm(alg, dlg.params, dlg.values, dlg.outputs)
                self.repaintModel()
                self.view.ensureVisible(self.scene.getLastAlgorithmItem())

    def fillAlgorithmTree(self):
        self.algorithmTree.clear()
        text = str(self.searchBox.text())
        allAlgs = ModelerUtils.getAlgorithms()
        for providerName in allAlgs.keys():
            groups = {}
            provider = allAlgs[providerName]
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
                providerItem.setIcon(0, ProviderIcons.providerIcons[providerName])
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
        self.setIcon(0, alg.getIcon())


