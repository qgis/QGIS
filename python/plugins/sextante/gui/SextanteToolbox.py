# -*- coding: utf-8 -*-

"""
***************************************************************************
    SextanteToolbox.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""
__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from PyQt4.QtCore import *
from PyQt4.QtGui import *

import webbrowser
from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.modeler.Providers import Providers
from sextante.gui.AlgorithmClassification import AlgorithmDecorator
from sextante.core.Sextante import Sextante
from sextante.core.SextanteLog import SextanteLog
from sextante.core.SextanteConfig import SextanteConfig
from sextante.core.QGisLayers import QGisLayers
from sextante.gui.ParametersDialog import ParametersDialog
from sextante.gui.BatchProcessingDialog import BatchProcessingDialog
from sextante.gui.EditRenderingStylesDialog import EditRenderingStylesDialog
from sextante.ui.ui_SextanteToolbox import Ui_SextanteToolbox

try:
    _fromUtf8 = QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class SextanteToolbox(QDockWidget, Ui_SextanteToolbox):
    def __init__(self, iface):
        QDockWidget.__init__(self, None)
        self.setupUi(self)
        self.setAllowedAreas(Qt.LeftDockWidgetArea | Qt.RightDockWidgetArea)

        self.iface=iface

        self.externalAppsButton.clicked.connect(self.configureProviders)
        self.searchBox.textChanged.connect(self.fillTree)
        self.algorithmTree.customContextMenuRequested.connect(self.showPopupMenu)
        self.algorithmTree.doubleClicked.connect(self.executeAlgorithm)

        if hasattr(self.searchBox, 'setPlaceholderText'):
            self.searchBox.setPlaceholderText(self.tr("Search..."))

        self.fillTree()

    def algsListHasChanged(self):
        self.fillTree()

    def updateTree(self):
        Sextante.updateAlgsList()

    def configureProviders(self):
        webbrowser.open("http://sextantegis.com")

    def showPopupMenu(self,point):
        item = self.algorithmTree.itemAt(point)
        if isinstance(item, TreeAlgorithmItem):
            alg = item.alg
            popupmenu = QMenu()
            executeAction = QAction(self.tr("Execute"), self.algorithmTree)
            executeAction.triggered.connect(self.executeAlgorithm)
            popupmenu.addAction(executeAction)
            if alg.canRunInBatchMode:
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
            alg = alg.getCopy()
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
                    self.addRecentAlgorithms(True)
        if isinstance(item, TreeActionItem):
            action = item.action
            action.setData(self)
            action.execute()

    def fillTree(self):
        useCategories = SextanteConfig.getSetting(SextanteConfig.USE_CATEGORIES)
        if useCategories:
            self.fillTreeUsingCategories()
        else:
            self.fillTreeUsingProviders()
        self.algorithmTree.sortItems(0, Qt.AscendingOrder)
        self.addRecentAlgorithms(False)

    def addRecentAlgorithms(self, updating):
        showRecent = SextanteConfig.getSetting(SextanteConfig.SHOW_RECENT_ALGORITHMS)
        if showRecent:
            recent = SextanteLog.getRecentAlgorithms()
            if len(recent) != 0:
                found = False
                if updating:
                    recentItem = self.algorithmTree.topLevelItem(0)
                    treeWidget = recentItem.treeWidget()
                    treeWidget.takeTopLevelItem(treeWidget.indexOfTopLevelItem(recentItem))
                    #self.algorithmTree.removeItemWidget(first, 0)

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

    def fillTreeUsingCategories(self):
        providersToExclude = ["model", "script"]
        self.algorithmTree.clear()
        text = unicode(self.searchBox.text())
        groups = {}
        for providerName in Sextante.algs.keys():
            provider = Sextante.algs[providerName]
            name = "ACTIVATE_" + providerName.upper().replace(" ", "_")
            if not SextanteConfig.getSetting(name):
                continue
            if providerName in providersToExclude or len(Providers.providers[providerName].actions) != 0:
                continue
            algs = provider.values()
            #add algorithms
            for alg in algs:
                if not alg.showInToolbox:
                    continue
                altgroup, altsubgroup, altname = AlgorithmDecorator.getGroupsAndName(alg)
                if altgroup is None:
                    continue
                if text =="" or text.lower() in altname.lower():
                    if altgroup not in groups:
                        groups[altgroup] = {}
                    group = groups[altgroup]
                    if altsubgroup not in group:
                        groups[altgroup][altsubgroup] = []
                    subgroup = groups[altgroup][altsubgroup]
                    subgroup.append(alg)

        if len(groups) > 0:
            mainItem = QTreeWidgetItem()
            mainItem.setText(0, "Geoalgorithms")
            mainItem.setIcon(0, GeoAlgorithm.getDefaultIcon())
            mainItem.setToolTip(0, mainItem.text(0))
            for groupname, group in groups.items():
                groupItem = QTreeWidgetItem()
                groupItem.setText(0, groupname)
                groupItem.setIcon(0, GeoAlgorithm.getDefaultIcon())
                groupItem.setToolTip(0, groupItem.text(0))
                mainItem.addChild(groupItem)
                for subgroupname, subgroup in group.items():
                    subgroupItem = QTreeWidgetItem()
                    subgroupItem.setText(0, subgroupname)
                    subgroupItem.setIcon(0, GeoAlgorithm.getDefaultIcon())
                    subgroupItem.setToolTip(0, subgroupItem.text(0))
                    groupItem.addChild(subgroupItem)
                    for alg in subgroup:
                        algItem = TreeAlgorithmItem(alg)
                        subgroupItem.addChild(algItem)

            self.algorithmTree.addTopLevelItem(mainItem)

        for providerName in Sextante.algs.keys():
            groups = {}
            provider = Sextante.algs[providerName]
            name = "ACTIVATE_" + providerName.upper().replace(" ", "_")
            if not SextanteConfig.getSetting(name):
                continue
            if providerName not in providersToExclude:
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
                        groupItem.setText(0, alg.group)
                        groupItem.setToolTip(0, alg.group)
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
                providerItem.setText(0, Sextante.getProviderFromName(providerName).getDescription())
                providerItem.setIcon(0, Sextante.getProviderFromName(providerName).getIcon())
                providerItem.setToolTip(0, providerItem.text(0))
                for groupItem in groups.values():
                    providerItem.addChild(groupItem)
                self.algorithmTree.addTopLevelItem(providerItem)

        if (text != ""):
            self.algorithmTree.expandAll()

    def fillTreeUsingProviders(self):
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
                        groupItem.setText(0, alg.group)
                        groupItem.setToolTip(0, alg.group)
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
                providerItem.setToolTip(0, providerItem.text(0))
                for groupItem in groups.values():
                    providerItem.addChild(groupItem)
                self.algorithmTree.addTopLevelItem(providerItem)
                providerItem.setExpanded(text!="")
                for groupItem in groups.values():
                    groupItem.setExpanded(text != "")



class TreeAlgorithmItem(QTreeWidgetItem):

    def __init__(self, alg):
        useCategories = SextanteConfig.getSetting(SextanteConfig.USE_CATEGORIES)
        QTreeWidgetItem.__init__(self)
        self.alg = alg
        icon = alg.getIcon()
        name = alg.name
        if useCategories:
            icon = GeoAlgorithm.getDefaultIcon()
            group, subgroup, name = AlgorithmDecorator.getGroupsAndName(alg)
        self.setIcon(0, icon)
        self.setToolTip(0, name)
        self.setText(0, name)

class TreeActionItem(QTreeWidgetItem):

    def __init__(self, action):
        QTreeWidgetItem.__init__(self)
        self.action = action
        self.setText(0, action.name)
        self.setIcon(0, action.getIcon())
