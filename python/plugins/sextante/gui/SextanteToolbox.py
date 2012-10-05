import os
import sys

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from sextante.core.Sextante import Sextante
from sextante.core.SextanteLog import SextanteLog
from sextante.core.SextanteConfig import SextanteConfig
from sextante.core.QGisLayers import QGisLayers

from sextante.gui.ParametersDialog import ParametersDialog
from sextante.gui.BatchProcessingDialog import BatchProcessingDialog
from sextante.gui.EditRenderingStylesDialog import EditRenderingStylesDialog

try:
    _fromUtf8 = QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s


class SextanteToolbox(QDockWidget):
    def __init__(self, iface):
        QDialog.__init__(self)
        self.iface=iface
        self.setupUi()

    def algsListHasChanged(self):
        self.fillTree()

    def updateTree(self):
        Sextante.updateAlgsList()

    def setupUi(self):
        self.setObjectName("SEXTANTE_Toolbox")
        self.setFloating(False)
        self.resize(400, 500)
        self.setWindowTitle(self.tr("SEXTANTE Toolbox"))
        self.contents = QWidget()
        self.verticalLayout = QVBoxLayout(self.contents)
        self.verticalLayout.setSpacing(2)
        self.verticalLayout.setMargin(0)
        self.externalAppsButton = QPushButton()
        self.externalAppsButton.setText(self.tr("Click here to configure\nadditional algorithm providers"))
        QObject.connect(self.externalAppsButton, SIGNAL("clicked()"), self.configureProviders)
        self.verticalLayout.addWidget(self.externalAppsButton)
        self.searchBox = QLineEdit(self.contents)
        self.searchBox.textChanged.connect(self.fillTree)
        self.verticalLayout.addWidget(self.searchBox)
        self.algorithmTree = QTreeWidget(self.contents)
        self.algorithmTree.setHeaderHidden(True)
        self.algorithmTree.setContextMenuPolicy(Qt.CustomContextMenu)
        self.fillTree()
        self.connect(self.algorithmTree, SIGNAL('customContextMenuRequested(QPoint)'),
                     self.showPopupMenu)
        self.verticalLayout.addWidget(self.algorithmTree)
        self.algorithmTree.doubleClicked.connect(self.executeAlgorithm)
        self.setWidget(self.contents)
        self.iface.addDockWidget(Qt.RightDockWidgetArea, self)
        QMetaObject.connectSlotsByName(self)

    def configureProviders(self):
        QDesktopServices.openUrl(QUrl(os.path.join(os.path.dirname(__file__), os.path.pardir) + "/help/3rdParty.html"))

    def showPopupMenu(self,point):
        item = self.algorithmTree.itemAt(point)
        if isinstance(item, TreeAlgorithmItem):
            alg = item.alg
            popupmenu = QMenu()
            executeAction = QAction(self.tr("Execute"), self.algorithmTree)
            executeAction.triggered.connect(self.executeAlgorithm)
            popupmenu.addAction(executeAction)
            executeBatchAction = QAction(self.tr("Execute as batch process"), self.algorithmTree)
            executeBatchAction.triggered.connect(self.executeAlgorithmAsBatchProcess)
            popupmenu.addAction(executeBatchAction)
            editRenderingStylesAction = QAction(self.tr("Edit rendering styles for outputs"), self.algorithmTree)
            editRenderingStylesAction.triggered.connect(self.editRenderingStyles)
            popupmenu.addAction(editRenderingStylesAction)
            actions = Sextante.contextMenuActions
            for action in actions:
                action.setData(alg,self)
                if action.isEnabled():
                    contextMenuAction = QAction(action.name, self.algorithmTree)
                    contextMenuAction.triggered.connect(action.execute)
                    popupmenu.addAction(contextMenuAction)

            popupmenu.exec_(self.algorithmTree.mapToGlobal(point))

    def editRenderingStyles(self):
        item = self.algorithmTree.currentItem()
        if isinstance(item, TreeAlgorithmItem):
            alg = Sextante.getAlgorithm(item.alg.commandLineName())
            dlg = EditRenderingStylesDialog(alg)
            dlg.exec_()

    def executeAlgorithmAsBatchProcess(self):
        item = self.algorithmTree.currentItem()
        if isinstance(item, TreeAlgorithmItem):
            alg = Sextante.getAlgorithm(item.alg.commandLineName())
            dlg = BatchProcessingDialog(alg)
            dlg.exec_()

    def executeAlgorithm(self):
        item = self.algorithmTree.currentItem()
        if isinstance(item, TreeAlgorithmItem):
            alg = Sextante.getAlgorithm(item.alg.commandLineName())
            message = alg.checkBeforeOpeningParametersDialog()
            if message:
                QMessageBox.warning(self, self.tr("Warning"), message)
                return
            alg = alg.getCopy()#copy.deepcopy(alg)
            dlg = alg.getCustomParametersDialog()
            if not dlg:
                dlg = ParametersDialog(alg)
            canvas = QGisLayers.iface.mapCanvas()
            prevMapTool = canvas.mapTool()
            dlg.show()
            dlg.exec_()
            if canvas.mapTool()!=prevMapTool:
                try:
                    canvas.mapTool().reset()
                except:
                    pass
                canvas.setMapTool(prevMapTool)
            if dlg.executed:
                showRecent = SextanteConfig.getSetting(SextanteConfig.SHOW_RECENT_ALGORITHMS)
                if showRecent:
                    self.fillTree()
        if isinstance(item, TreeActionItem):
            action = item.action
            action.setData(self)
            action.execute()

    def fillTree(self):
        self.algorithmTree.clear()
        text = unicode(self.searchBox.text())
        for providerName in Sextante.algs.keys():
            groups = {}
            provider = Sextante.algs[providerName]
            name = "ACTIVATE_" + providerName.upper().replace(" ", "_")
            if not SextanteConfig.getSetting(name):
                continue
            algs = provider.values()
            #add algorithms
            for alg in algs:
                if not alg.showInToolbox:
                    continue
                if text =="" or text.lower() in alg.name.lower():
                    if alg.group in groups:
                        groupItem = groups[alg.group]
                    else:
                        groupItem = QTreeWidgetItem()
                        groupItem.setText(0,alg.group)
                        groups[alg.group] = groupItem
                    algItem = TreeAlgorithmItem(alg)
                    groupItem.addChild(algItem)

            actions = Sextante.actions[providerName]
            for action in actions:
                if text =="" or text.lower() in action.name.lower():
                    if action.group in groups:
                        groupItem = groups[action.group]
                    else:
                        groupItem = QTreeWidgetItem()
                        groupItem.setText(0,action.group)
                        groups[action.group] = groupItem
                    algItem = TreeActionItem(action)
                    groupItem.addChild(algItem)


            if len(groups) > 0:
                providerItem = QTreeWidgetItem()
                providerItem.setText(0, Sextante.getProviderFromName(providerName).getDescription()
                                     + " [" + str(len(provider)) + " geoalgorithms]")
                providerItem.setIcon(0, Sextante.getProviderFromName(providerName).getIcon())
                for groupItem in groups.values():
                    providerItem.addChild(groupItem)
                self.algorithmTree.addTopLevelItem(providerItem)
                providerItem.setExpanded(text!="")
                for groupItem in groups.values():
                    if text != "":
                        groupItem.setExpanded(True)

        self.algorithmTree.sortItems(0, Qt.AscendingOrder)

        showRecent = SextanteConfig.getSetting(SextanteConfig.SHOW_RECENT_ALGORITHMS)
        if showRecent:
            recent = SextanteLog.getRecentAlgorithms()
            if len(recent) != 0:
                found = False
                recentItem = QTreeWidgetItem()
                recentItem.setText(0, self.tr("Recently used algorithms"))
                for algname in recent:
                    alg = Sextante.getAlgorithm(algname)
                    if alg is not None:
                        algItem = TreeAlgorithmItem(alg)
                        recentItem.addChild(algItem)
                        found = True
                if found:
                    self.algorithmTree.insertTopLevelItem(0, recentItem)
                    recentItem.setExpanded(True)

            self.algorithmTree.setWordWrap(True)

class TreeAlgorithmItem(QTreeWidgetItem):

    def __init__(self, alg):
        QTreeWidgetItem.__init__(self)
        self.alg = alg
        self.setText(0, alg.name)
        self.setIcon(0, alg.getIcon())
        self.setToolTip(0, alg.name)

class TreeActionItem(QTreeWidgetItem):

    def __init__(self, action):
        QTreeWidgetItem.__init__(self)
        self.action = action
        self.setText(0, action.name)
        self.setIcon(0, action.getIcon())
