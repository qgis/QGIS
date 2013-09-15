# -*- coding: utf-8 -*-

"""
***************************************************************************
    ProcessingToolbox.py
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
from processing import interface

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from PyQt4.QtCore import *
from PyQt4.QtGui import *

import webbrowser
from processing.gui.MissingDependencyDialog import MissingDependencyDialog
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.modeler.Providers import Providers
from processing.gui.AlgorithmClassification import AlgorithmDecorator
from processing.core.Processing import Processing
from processing.core.ProcessingLog import ProcessingLog
from processing.core.ProcessingConfig import ProcessingConfig
from processing.tools import dataobjects
from processing.gui.ParametersDialog import ParametersDialog
from processing.gui.BatchProcessingDialog import BatchProcessingDialog
from processing.gui.EditRenderingStylesDialog import EditRenderingStylesDialog
from processing.ui.ui_ProcessingToolbox import Ui_ProcessingToolbox


class ProcessingToolbox(QDockWidget, Ui_ProcessingToolbox):

    USE_CATEGORIES = "/ProcessingQGIS/UseCategories"

    def __init__(self):
        QDockWidget.__init__(self, None)
        self.setupUi(self)
        self.setAllowedAreas(Qt.LeftDockWidgetArea | Qt.RightDockWidgetArea)

        self.modeComboBox.clear()
        self.modeComboBox.addItems(['Simplified interface', 'Advanced interface'])
        settings = QSettings()
        if not settings.contains(self.USE_CATEGORIES):
            settings.setValue(self.USE_CATEGORIES, True)
        useCategories = settings.value(self.USE_CATEGORIES, type = bool)
        if useCategories:
            self.modeComboBox.setCurrentIndex(0)
        else:
            self.modeComboBox.setCurrentIndex(1)
        self.modeComboBox.currentIndexChanged.connect(self.modeHasChanged)

        self.searchBox.textChanged.connect(self.fillTree)
        self.algorithmTree.customContextMenuRequested.connect(self.showPopupMenu)
        self.algorithmTree.doubleClicked.connect(self.executeAlgorithm)

        if hasattr(self.searchBox, 'setPlaceholderText'):
            self.searchBox.setPlaceholderText(self.tr("Search..."))

        self.fillTree()

    def modeHasChanged(self):
        idx = self.modeComboBox.currentIndex()
        settings = QSettings()
        if idx == 0: #simplified
            settings.setValue(self.USE_CATEGORIES, True)
        else:
            settings.setValue(self.USE_CATEGORIES, False)

        self.fillTree()

    def algsListHasChanged(self):
        self.fillTree()

    def updateTree(self):
        Processing.updateAlgsList()

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
            popupmenu.addSeparator()
            editRenderingStylesAction = QAction(self.tr("Edit rendering styles for outputs"), self.algorithmTree)
            editRenderingStylesAction.triggered.connect(self.editRenderingStyles)
            popupmenu.addAction(editRenderingStylesAction)
            actions = Processing.contextMenuActions
            if len(actions) > 0:
                popupmenu.addSeparator()
            for action in actions:
                action.setData(alg, self)
                if action.isEnabled():
                    contextMenuAction = QAction(action.name, self.algorithmTree)
                    contextMenuAction.triggered.connect(action.execute)
                    popupmenu.addAction(contextMenuAction)

            popupmenu.exec_(self.algorithmTree.mapToGlobal(point))

    def editRenderingStyles(self):
        item = self.algorithmTree.currentItem()
        if isinstance(item, TreeAlgorithmItem):
            alg = Processing.getAlgorithm(item.alg.commandLineName())
            dlg = EditRenderingStylesDialog(alg)
            dlg.exec_()

    def executeAlgorithmAsBatchProcess(self):
        item = self.algorithmTree.currentItem()
        if isinstance(item, TreeAlgorithmItem):
            alg = Processing.getAlgorithm(item.alg.commandLineName())
            dlg = BatchProcessingDialog(alg)
            dlg.exec_()

    def executeAlgorithm(self):
        item = self.algorithmTree.currentItem()
        if isinstance(item, TreeAlgorithmItem):
            alg = Processing.getAlgorithm(item.alg.commandLineName())
            message = alg.checkBeforeOpeningParametersDialog()
            if message:
                dlg = MissingDependencyDialog(message)
                dlg.exec_()
                return
            alg = alg.getCopy()
            dlg = alg.getCustomParametersDialog()
            if not dlg:
                dlg = ParametersDialog(alg)
            canvas = interface.iface.mapCanvas()
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
                showRecent = ProcessingConfig.getSetting(ProcessingConfig.SHOW_RECENT_ALGORITHMS)
                if showRecent:
                    self.addRecentAlgorithms(True)
        if isinstance(item, TreeActionItem):
            action = item.action
            action.setData(self)
            action.execute()

    def fillTree(self):
        settings = QSettings()
        useCategories = settings.value(self.USE_CATEGORIES, type = bool)
        if useCategories:
            self.fillTreeUsingCategories()
        else:
            self.fillTreeUsingProviders()
        self.algorithmTree.sortItems(0, Qt.AscendingOrder)
        self.addRecentAlgorithms(False)

    def addRecentAlgorithms(self, updating):
        showRecent = ProcessingConfig.getSetting(ProcessingConfig.SHOW_RECENT_ALGORITHMS)
        if showRecent:
            recent = ProcessingLog.getRecentAlgorithms()
            if len(recent) != 0:
                found = False
                if updating:
                    recentItem = self.algorithmTree.topLevelItem(0)
                    treeWidget = recentItem.treeWidget()
                    treeWidget.takeTopLevelItem(treeWidget.indexOfTopLevelItem(recentItem))

                recentItem = QTreeWidgetItem()
                recentItem.setText(0, self.tr("Recently used algorithms"))
                for algname in recent:
                    alg = Processing.getAlgorithm(algname)
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
        for providerName in Processing.algs.keys():
            provider = Processing.algs[providerName]
            name = "ACTIVATE_" + providerName.upper().replace(" ", "_")
            if not ProcessingConfig.getSetting(name):
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

        for providerName in Processing.algs.keys():
            groups = {}
            provider = Processing.algs[providerName]
            name = "ACTIVATE_" + providerName.upper().replace(" ", "_")
            if not ProcessingConfig.getSetting(name):
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

            actions = Processing.actions[providerName]
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
                providerItem.setText(0, Processing.getProviderFromName(providerName).getDescription())
                providerItem.setIcon(0, Processing.getProviderFromName(providerName).getIcon())
                providerItem.setToolTip(0, providerItem.text(0))
                for groupItem in groups.values():
                    providerItem.addChild(groupItem)
                self.algorithmTree.addTopLevelItem(providerItem)

        if (text != ""):
            self.algorithmTree.expandAll()

    def fillTreeUsingProviders(self):
        self.algorithmTree.clear()
        text = unicode(self.searchBox.text())
        for providerName in Processing.algs.keys():
            groups = {}
            provider = Processing.algs[providerName]
            name = "ACTIVATE_" + providerName.upper().replace(" ", "_")
            if not ProcessingConfig.getSetting(name):
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

            actions = Processing.actions[providerName]
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
                providerItem.setText(0, Processing.getProviderFromName(providerName).getDescription()
                                     + " [" + str(len(provider)) + " geoalgorithms]")
                providerItem.setIcon(0, Processing.getProviderFromName(providerName).getIcon())
                providerItem.setToolTip(0, providerItem.text(0))
                for groupItem in groups.values():
                    providerItem.addChild(groupItem)
                self.algorithmTree.addTopLevelItem(providerItem)
                providerItem.setExpanded(text!="")
                for groupItem in groups.values():
                    groupItem.setExpanded(text != "")

class TreeAlgorithmItem(QTreeWidgetItem):

    def __init__(self, alg):
        settings = QSettings()
        useCategories = settings.value(ProcessingToolbox.USE_CATEGORIES, type = bool)
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
