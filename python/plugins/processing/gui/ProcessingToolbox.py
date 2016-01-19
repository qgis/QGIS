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

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from PyQt4 import uic
from PyQt4.QtCore import Qt, QSettings, QCoreApplication, SIGNAL
from PyQt4.QtGui import QMenu, QAction, QTreeWidgetItem, QLabel, QMessageBox
from qgis.utils import iface

from processing.core.Processing import Processing
from processing.core.ProcessingLog import ProcessingLog
from processing.core.ProcessingConfig import ProcessingConfig
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.gui.MessageDialog import MessageDialog
from processing.gui import AlgorithmClassification
from processing.gui.AlgorithmDialog import AlgorithmDialog
from processing.gui.BatchAlgorithmDialog import BatchAlgorithmDialog
from processing.gui.EditRenderingStylesDialog import EditRenderingStylesDialog
from processing.gui.ConfigDialog import ConfigDialog


pluginPath = os.path.split(os.path.dirname(__file__))[0]
WIDGET, BASE = uic.loadUiType(
    os.path.join(pluginPath, 'ui', 'ProcessingToolbox.ui'))


class ProcessingToolbox(BASE, WIDGET):

    updateAlgList = True

    def __init__(self):
        super(ProcessingToolbox, self).__init__(None)
        self.setupUi(self)
        self.setAllowedAreas(Qt.LeftDockWidgetArea | Qt.RightDockWidgetArea)

        self.searchBox.textChanged.connect(self.textChanged)
        self.algorithmTree.customContextMenuRequested.connect(
            self.showPopupMenu)
        self.algorithmTree.doubleClicked.connect(self.executeAlgorithm)
        self.txtDisabled.setVisible(False)
        self.txtTip.setVisible(self.disabledProviders())
        self.txtDisabled.linkActivated.connect(self.showDisabled)

        def openSettings():
            dlg = ConfigDialog(self)
            dlg.exec_()
            self.txtTip.setVisible(self.disabledProviders())
        self.txtTip.linkActivated.connect(openSettings)
        if hasattr(self.searchBox, 'setPlaceholderText'):
            self.searchBox.setPlaceholderText(self.tr('Search...'))

        self.fillTree()

    def showDisabled(self):
        self.txtDisabled.setVisible(False)
        for providerName in self.disabledWithMatchingAlgs:
            self.disabledProviderItems[providerName].setHidden(False)
        self.algorithmTree.expandAll()

    def disabledProviders(self):
        for providerName in Processing.algs.keys():
            name = 'ACTIVATE_' + providerName.upper().replace(' ', '_')
            if not ProcessingConfig.getSetting(name):
                return True
        return False

    def textChanged(self):
        text = self.searchBox.text().strip(' ').lower()
        for item in self.disabledProviderItems.values():
            item.setHidden(True)
        self._filterItem(self.algorithmTree.invisibleRootItem(), text)
        if text:
            self.algorithmTree.expandAll()
            self.disabledWithMatchingAlgs = []
            for providerName, provider in Processing.algs.iteritems():
                name = 'ACTIVATE_' + providerName.upper().replace(' ', '_')
                if not ProcessingConfig.getSetting(name):
                    for alg in provider.values():
                        if text in alg.name:
                            self.disabledWithMatchingAlgs.append(providerName)
                            break
            self.txtDisabled.setVisible(bool(self.disabledWithMatchingAlgs))
        else:
            self.algorithmTree.collapseAll()
            self.algorithmTree.invisibleRootItem().child(0).setExpanded(True)
            self.txtDisabled.setVisible(False)

    def _filterItem(self, item, text):
        if (item.childCount() > 0):
            show = False
            for i in xrange(item.childCount()):
                child = item.child(i)
                showChild = self._filterItem(child, text)
                show = (showChild or show) and not item in self.disabledProviderItems.values()
            item.setHidden(not show)
            return show
        elif isinstance(item, (TreeAlgorithmItem, TreeActionItem)):
            hide = bool(text) and (text not in item.text(0).lower())
            if isinstance(item, TreeAlgorithmItem):
                hide = hide and (text not in item.alg.commandLineName())
                if item.alg.shortHelp() is not None:
                    hide = hide and (text not in item.alg.shortHelp())
            item.setHidden(hide)
            return not hide
        else:
            item.setHidden(True)
            return False

    def activateProvider(self, providerName):
        name = 'ACTIVATE_' + providerName.upper().replace(' ', '_')
        ProcessingConfig.setSettingValue(name, True)
        self.fillTree()
        self.textChanged()
        self.showDisabled()
        provider = Processing.getProviderFromName(providerName)
        if not provider.canBeActivated():
            QMessageBox.warning(self, "Activate provider",
                                "The provider has been activated, but it might need additional configuration.")

    def algsListHasChanged(self):
        if self.updateAlgList:
            self.fillTree()
            self.textChanged()

    def updateProvider(self, providerName, updateAlgsList=True):
        if updateAlgsList:
            self.updateAlgList = False
            Processing.updateAlgsList()
            self.updateAlgList = True
        for i in xrange(self.algorithmTree.invisibleRootItem().childCount()):
            child = self.algorithmTree.invisibleRootItem().child(i)
            if isinstance(child, TreeProviderItem):
                if child.providerName == providerName:
                    child.refresh()
                    # sort categories and items in categories
                    child.sortChildren(0, Qt.AscendingOrder)
                    for i in xrange(child.childCount()):
                        child.child(i).sortChildren(0, Qt.AscendingOrder)
                    break
        self.addRecentAlgorithms(True)

    def showPopupMenu(self, point):
        item = self.algorithmTree.itemAt(point)
        if isinstance(item, TreeAlgorithmItem):
            alg = item.alg
            popupmenu = QMenu()
            executeAction = QAction(self.tr('Execute'), self.algorithmTree)
            executeAction.triggered.connect(self.executeAlgorithm)
            popupmenu.addAction(executeAction)
            if alg.canRunInBatchMode and not alg.allowOnlyOpenedLayers:
                executeBatchAction = QAction(
                    self.tr('Execute as batch process'),
                    self.algorithmTree)
                executeBatchAction.triggered.connect(
                    self.executeAlgorithmAsBatchProcess)
                popupmenu.addAction(executeBatchAction)
            popupmenu.addSeparator()
            editRenderingStylesAction = QAction(
                self.tr('Edit rendering styles for outputs'),
                self.algorithmTree)
            editRenderingStylesAction.triggered.connect(
                self.editRenderingStyles)
            popupmenu.addAction(editRenderingStylesAction)
            actions = Processing.contextMenuActions
            if len(actions) > 0:
                popupmenu.addSeparator()
            for action in actions:
                action.setData(alg, self)
                if action.isEnabled():
                    contextMenuAction = QAction(action.name,
                                                self.algorithmTree)
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
            alg = alg.getCopy()
            dlg = BatchAlgorithmDialog(alg)
            dlg.show()
            dlg.exec_()

    def executeAlgorithm(self):
        item = self.algorithmTree.currentItem()
        if isinstance(item, TreeAlgorithmItem):
            alg = Processing.getAlgorithm(item.alg.commandLineName())
            message = alg.checkBeforeOpeningParametersDialog()
            if message:
                dlg = MessageDialog()
                dlg.setTitle(self.tr('Error executing algorithm'))
                dlg.setMessage(
                    self.tr('<h3>This algorithm cannot '
                            'be run :-( </h3>\n%s') % message)
                dlg.exec_()
                return
            alg = alg.getCopy()
            dlg = alg.getCustomParametersDialog()
            if not dlg:
                dlg = AlgorithmDialog(alg)
            canvas = iface.mapCanvas()
            prevMapTool = canvas.mapTool()
            dlg.show()
            dlg.exec_()
            if canvas.mapTool() != prevMapTool:
                try:
                    canvas.mapTool().reset()
                except:
                    pass
                canvas.setMapTool(prevMapTool)
            if dlg.executed:
                showRecent = ProcessingConfig.getSetting(
                    ProcessingConfig.SHOW_RECENT_ALGORITHMS)
                if showRecent:
                    self.addRecentAlgorithms(True)
        if isinstance(item, TreeActionItem):
            action = item.action
            action.setData(self)
            action.execute()

    def fillTree(self):
        self.fillTreeUsingProviders()
        self.addRecentAlgorithms(False)

    def addRecentAlgorithms(self, updating):
        showRecent = ProcessingConfig.getSetting(
            ProcessingConfig.SHOW_RECENT_ALGORITHMS)
        if showRecent:
            recent = ProcessingLog.getRecentAlgorithms()
            if len(recent) != 0:
                found = False
                if updating:
                    recentItem = self.algorithmTree.topLevelItem(0)
                    treeWidget = recentItem.treeWidget()
                    treeWidget.takeTopLevelItem(
                        treeWidget.indexOfTopLevelItem(recentItem))

                recentItem = QTreeWidgetItem()
                recentItem.setText(0, self.tr('Recently used algorithms'))
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

    def fillTreeUsingProviders(self):
        self.algorithmTree.clear()
        self.disabledProviderItems = {}
        disabled = []
        for providerName in Processing.algs.keys():
            name = 'ACTIVATE_' + providerName.upper().replace(' ', '_')
            if ProcessingConfig.getSetting(name):
                providerItem = TreeProviderItem(providerName, self.algorithmTree, self)
                providerItem.setHidden(providerItem.childCount() == 0)
            else:
                disabled.append(providerName)
        self.algorithmTree.sortItems(0, Qt.AscendingOrder)
        for providerName in sorted(disabled):
            providerItem = TreeProviderItem(providerName, self.algorithmTree, self)
            providerItem.setHidden(True)
            self.disabledProviderItems[providerName] = providerItem


class TreeAlgorithmItem(QTreeWidgetItem):

    def __init__(self, alg):
        QTreeWidgetItem.__init__(self)
        self.alg = alg
        icon = alg.getIcon()
        name = AlgorithmClassification.getDisplayName(alg)
        self.setIcon(0, icon)
        self.setToolTip(0, name)
        self.setText(0, name)


class TreeActionItem(QTreeWidgetItem):

    def __init__(self, action):
        QTreeWidgetItem.__init__(self)
        self.action = action
        self.setText(0, action.name)
        self.setIcon(0, action.getIcon())


class TreeProviderItem(QTreeWidgetItem):

    def __init__(self, providerName, tree, toolbox):
        QTreeWidgetItem.__init__(self, tree)
        self.tree = tree
        self.toolbox = toolbox
        self.providerName = providerName
        self.provider = Processing.getProviderFromName(providerName)
        self.setIcon(0, self.provider.getIcon())
        self.populate()

    def refresh(self):
        self.takeChildren()
        self.populate()

    def populate(self):
        groups = {}
        count = 0
        provider = Processing.algs[self.providerName]
        algs = provider.values()

        name = 'ACTIVATE_' + self.providerName.upper().replace(' ', '_')
        active = ProcessingConfig.getSetting(name)

        # Add algorithms
        for alg in algs:
            if not alg.showInToolbox:
                continue
            if alg.group in groups:
                groupItem = groups[alg.group]
            else:
                groupItem = QTreeWidgetItem()
                name = alg.i18n_group or alg.group
                if not active:
                    groupItem.setForeground(0, Qt.darkGray)
                groupItem.setText(0, name)
                groupItem.setToolTip(0, name)
                groups[alg.group] = groupItem
            algItem = TreeAlgorithmItem(alg)
            if not active:
                algItem.setForeground(0, Qt.darkGray)
            groupItem.addChild(algItem)
            count += 1

        actions = Processing.actions[self.providerName]
        for action in actions:
            if action.group in groups:
                groupItem = groups[action.group]
            else:
                groupItem = QTreeWidgetItem()
                groupItem.setText(0, action.group)
                groups[action.group] = groupItem
            algItem = TreeActionItem(action)
            groupItem.addChild(algItem)

        text = self.provider.getDescription()

        if not active:
            def activateProvider():
                self.toolbox.activateProvider(self.providerName)
            label = QLabel(text + "&nbsp;&nbsp;&nbsp;&nbsp;<a href='%s'>Activate</a>")
            label.setStyleSheet("QLabel {background-color: white; color: grey;}")
            label.linkActivated.connect(activateProvider)
            self.tree.setItemWidget(self, 0, label)

        else:
            text += QCoreApplication.translate("TreeProviderItem", " [{0} geoalgorithms]").format(count)
        self.setText(0, text)
        self.setToolTip(0, self.text(0))
        for groupItem in groups.values():
            self.addChild(groupItem)
