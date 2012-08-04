from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4 import QtCore, QtGui
from sextante.modeler.ModelerParameterDefinitionDialog import ModelerParameterDefinitionDialog
from sextante.modeler.ModelerAlgorithm import ModelerAlgorithm
from sextante.modeler.ModelerParametersDialog import ModelerParametersDialog
from sextante.modeler.ModelerUtils import ModelerUtils
from sextante.modeler.WrongModelException import WrongModelException
from sextante.modeler.ModelerScene import ModelerScene
from sextante.modeler.Providers import Providers
from sextante.gui.HelpEditionDialog import HelpEditionDialog
import pickle
from sextante.gui.ParametersDialog import ParametersDialog
from sextante.core.SextanteUtils import SextanteUtils
import codecs

class ModelerDialog(QtGui.QDialog):
    def __init__(self, alg=None):
        QtGui.QDialog.__init__(self)
        self.setupUi()
        self.setWindowFlags(self.windowFlags() | QtCore.Qt.WindowSystemMenuHint |
                            QtCore.Qt.WindowMinMaxButtonsHint)
        if alg is not None:
            self.alg = alg
            self.textGroup.setText(alg.group)
            self.textName.setText(alg.name)
            self.repaintModel()
            last = self.scene.getLastAlgorithmItem()
            if last is not None:
                self.view.ensureVisible(last)
            else:
                self.view.ensureVisible(0,0,10,10)
        else:
            self.alg = ModelerAlgorithm()
        self.alg.setModelerView(self)
        self.help = None
        self.update = False #indicates whether to update or not the toolbox after closing this dialog

    def setupUi(self):
        self.resize(1000, 600)
        self.setWindowTitle("SEXTANTE Modeler")
        self.tabWidget = QtGui.QTabWidget()
        self.tabWidget.setMaximumSize(QtCore.QSize(350, 10000))
        self.tabWidget.setMinimumWidth(300)

        #left hand side part
        #==================================
        self.inputsTree = QtGui.QTreeWidget()
        self.inputsTree.setHeaderHidden(True)
        self.fillInputsTree()
        self.inputsTree.doubleClicked.connect(self.addInput)
        self.tabWidget.addTab(self.inputsTree, "Inputs")

        self.verticalLayout = QtGui.QVBoxLayout()
        self.verticalLayout.setSpacing(2)
        self.verticalLayout.setMargin(0)
        self.searchBox = QtGui.QLineEdit()
        self.searchBox.textChanged.connect(self.fillAlgorithmTree)
        self.verticalLayout.addWidget(self.searchBox)
        self.algorithmTree = QtGui.QTreeWidget()
        self.algorithmTree.setHeaderHidden(True)
        self.fillAlgorithmTree()
        self.verticalLayout.addWidget(self.algorithmTree)
        self.algorithmTree.doubleClicked.connect(self.addAlgorithm)

        self.algorithmsTab = QtGui.QWidget()
        self.algorithmsTab.setLayout(self.verticalLayout)
        self.tabWidget.addTab(self.algorithmsTab, "Algorithms")

        #right hand side part
        #==================================
        self.textName = QtGui.QLineEdit()
        if hasattr(self.textName, 'setPlaceholderText'):
            self.textName.setPlaceholderText("[Enter model name here]")
        self.textGroup = QtGui.QLineEdit()
        if hasattr(self.textGroup, 'setPlaceholderText'):
            self.textGroup.setPlaceholderText("[Enter group name here]")
        self.horizontalLayoutNames = QtGui.QHBoxLayout()
        self.horizontalLayoutNames.setSpacing(2)
        self.horizontalLayoutNames.setMargin(0)
        self.horizontalLayoutNames.addWidget(self.textName)
        self.horizontalLayoutNames.addWidget(self.textGroup)

        self.scene = ModelerScene(self)
        self.scene.setSceneRect(QtCore.QRectF(0, 0, 4000, 4000))

        self.canvasTabWidget = QtGui.QTabWidget()
        self.canvasTabWidget.setMinimumWidth(300)
        self.view = QtGui.QGraphicsView(self.scene)

        #=======================================================================
        # self.canvasTabWidget.addTab(self.view, "Design")
        # self.pythonText = QtGui.QTextEdit()
        # self.createScriptButton = QtGui.QPushButton()
        # self.createScriptButton.setText("Create script from model code")
        # self.createScriptButton.clicked.connect(self.createScript)
        # self.verticalLayoutPython = QtGui.QVBoxLayout()
        # self.verticalLayoutPython.setSpacing(2)
        # self.verticalLayoutPython.setMargin(0)
        # self.verticalLayoutPython.addWidget(self.pythonText)
        # self.verticalLayoutPython.addWidget(self.createScriptButton)
        # self.pythonWidget = QtGui.QWidget()
        # self.pythonWidget.setLayout(self.verticalLayoutPython)
        # self.canvasTabWidget.addTab(self.pythonWidget, "Python code")
        #=======================================================================

        self.canvasLayout = QtGui.QVBoxLayout()
        self.canvasLayout.setSpacing(2)
        self.canvasLayout.setMargin(0)
        self.canvasLayout.addLayout(self.horizontalLayoutNames)
        self.canvasLayout.addWidget(self.view)#canvasTabWidget)

        #upper part, putting the two previous parts together
        #===================================================
        self.horizontalLayout = QtGui.QHBoxLayout()
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setMargin(0)
        self.horizontalLayout.addWidget(self.tabWidget)
        self.horizontalLayout.addLayout(self.canvasLayout)

        #And the whole layout
        #==========================

        self.buttonBox = QtGui.QDialogButtonBox()
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.editHelpButton = QtGui.QPushButton()
        self.editHelpButton.setText("Edit model help")
        self.buttonBox.addButton(self.editHelpButton, QtGui.QDialogButtonBox.ActionRole)
        self.runButton = QtGui.QPushButton()
        self.runButton.setText("Run")
        self.buttonBox.addButton(self.runButton, QtGui.QDialogButtonBox.ActionRole)
        self.openButton = QtGui.QPushButton()
        self.openButton.setText("Open")
        self.buttonBox.addButton(self.openButton, QtGui.QDialogButtonBox.ActionRole)
        self.saveButton = QtGui.QPushButton()
        self.saveButton.setText("Save")
        self.buttonBox.addButton(self.saveButton, QtGui.QDialogButtonBox.ActionRole)
        self.closeButton = QtGui.QPushButton()
        self.closeButton.setText("Close")
        self.buttonBox.addButton(self.closeButton, QtGui.QDialogButtonBox.ActionRole)
        QObject.connect(self.openButton, QtCore.SIGNAL("clicked()"), self.openModel)
        QObject.connect(self.saveButton, QtCore.SIGNAL("clicked()"), self.saveModel)
        QObject.connect(self.closeButton, QtCore.SIGNAL("clicked()"), self.closeWindow)
        QObject.connect(self.runButton, QtCore.SIGNAL("clicked()"), self.runModel)
        QObject.connect(self.editHelpButton, QtCore.SIGNAL("clicked()"), self.editHelp)

        self.globalLayout = QtGui.QVBoxLayout()
        self.globalLayout.setSpacing(2)
        self.globalLayout.setMargin(0)
        self.globalLayout.addLayout(self.horizontalLayout)
        self.globalLayout.addWidget(self.buttonBox)
        self.setLayout(self.globalLayout)
        QtCore.QMetaObject.connectSlotsByName(self)

        self.view.ensureVisible(0, 0, 10, 10)

    def closeWindow(self):
        self.close()

    def editHelp(self):
        dlg = HelpEditionDialog(self.alg)
        dlg.exec_()
        #We store the description string in case there were not saved because there was no
        #filename defined yet
        if self.alg.descriptionFile is None and dlg.descriptions:
            self.help = dlg.descriptions

    #===========================================================================
    # def createScript(self):
    #    if str(self.textGroup.text()).strip() == "":
    #        QMessageBox.warning(self, "Warning", "Please enter group name before saving")
    #        return
    #    filename = QtGui.QFileDialog.getSaveFileName(self, "Save Script", ScriptUtils.scriptsFolder(), "Python scripts (*.py)")
    #    if filename:
    #        fout = open(filename, "w")
    #        fout.write(str(self.textGroup.text()) + "=group")
    #        fout.write(self.alg.getAsPythonCode())
    #        fout.close()
    #        self.update = True
    #===========================================================================

    def runModel(self):
        ##TODO: enable alg cloning without saving to file
        if self.alg.descriptionFile is None:
            self.alg.descriptionFile = SextanteUtils.getTempFilename("model")
            text = self.alg.serialize()
            fout = open(self.alg.descriptionFile, "w")
            fout.write(text)
            fout.close()
            self.alg.provider = Providers.providers["model"]
            alg = self.alg.getCopy()
            dlg = ParametersDialog(alg)
            dlg.exec_()
            self.alg.descriptionFile = None
            alg.descriptionFile = None
        else:
            if self.alg.provider is None: # might happen if model is opened from modeler dialog
                self.alg.provider = Providers.providers["model"]
            alg = self.alg.getCopy()
            dlg = ParametersDialog(alg)
            dlg.exec_()

    def saveModel(self):
        if unicode(self.textGroup.text()).strip() == "" or unicode(self.textName.text()).strip() == "":
            QMessageBox.warning(self, "Warning", "Please enter group and model names before saving")
            return
        self.alg.setPositions(self.scene.getParameterPositions(), self.scene.getAlgorithmPositions())
        self.alg.name = unicode(self.textName.text())
        self.alg.group = unicode(self.textGroup.text())
        if self.alg.descriptionFile != None:
            filename = self.alg.descriptionFile
        else:
            filename = str(QtGui.QFileDialog.getSaveFileName(self, "Save Model", ModelerUtils.modelsFolder(), "SEXTANTE models (*.model)"))
            if filename:
                if not filename.endswith(".model"):
                    filename += ".model"
                self.alg.descriptionFile = filename
        if filename:
            text = self.alg.serialize()
            fout = codecs.open(filename, "w", encoding='utf-8')
            #fout = open(filename, "w")
            fout.write(text)
            fout.close()
            self.update = True
            #if help strings were defined before saving the model for the first time, we do it here
            if self.help:
                f = open(self.alg.descriptionFile + ".help", "wb")
                pickle.dump(self.help, f)
                f.close()
                self.help = None
            QtGui.QMessageBox.information(self, "Model saving", "Model was correctly saved.")

    def openModel(self):
        filename = unicode(QtGui.QFileDialog.getOpenFileName(self, "Open Model", ModelerUtils.modelsFolder(), "SEXTANTE models (*.model)"))
        if filename:
            try:
                alg = ModelerAlgorithm()
                alg.openModel(filename)
                self.alg = alg;
                self.alg.setModelerView(self)
                self.textGroup.setText(alg.group)
                self.textName.setText(alg.name)
                self.repaintModel()
                self.view.ensureVisible(self.scene.getLastAlgorithmItem())
            except WrongModelException, e:
                QMessageBox.critical(self, "Could not open model", "The selected model could not be loaded\nWrong line:" + e.msg)


    def repaintModel(self):
        self.scene = ModelerScene()
        self.scene.setSceneRect(QtCore.QRectF(0, 0, ModelerAlgorithm.CANVAS_SIZE, ModelerAlgorithm.CANVAS_SIZE))
        self.scene.paintModel(self.alg)
        self.view.setScene(self.scene)
        #self.pythonText.setText(self.alg.getAsPythonCode())


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
        parametersItem = QtGui.QTreeWidgetItem()
        parametersItem.setText(0, "Parameters")
        for paramType in ModelerParameterDefinitionDialog.paramTypes:
            paramItem = QtGui.QTreeWidgetItem()
            paramItem.setText(0, paramType)
            parametersItem.addChild(paramItem)
        self.inputsTree.addTopLevelItem(parametersItem)
        parametersItem.setExpanded(True)


    def addAlgorithm(self):
        item = self.algorithmTree.currentItem()
        if isinstance(item, TreeAlgorithmItem):
            alg = ModelerUtils.getAlgorithm(item.alg.commandLineName())
            alg = alg.getCopy()
            dlg = alg.getCustomModelerParametersDialog(self.alg)
            if not dlg:
                dlg = ModelerParametersDialog(alg, self.alg)
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
                if not alg.showInModeler:
                    continue
                if text == "" or text.lower() in alg.name.lower():
                    if alg.group in groups:
                        groupItem = groups[alg.group]
                    else:
                        groupItem = QtGui.QTreeWidgetItem()
                        groupItem.setText(0, alg.group)
                        groups[alg.group] = groupItem
                    algItem = TreeAlgorithmItem(alg)
                    groupItem.addChild(algItem)

            if len(groups) > 0:
                providerItem = QtGui.QTreeWidgetItem()
                providerItem.setText(0, Providers.providers[providerName].getDescription())
                providerItem.setIcon(0, Providers.providers[providerName].getIcon())
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


