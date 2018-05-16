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

import operator
import os

from qgis.PyQt import uic
from qgis.PyQt.QtCore import Qt, QCoreApplication
from qgis.PyQt.QtWidgets import QToolButton, QMenu, QAction, QTreeWidgetItem, QLabel, QMessageBox
from qgis.utils import iface
from qgis.core import (QgsApplication,
                       QgsProcessingAlgorithm)
from qgis.gui import QgsDockWidget

from processing.gui.Postprocessing import handleAlgorithmResults
from processing.core.ProcessingLog import ProcessingLog
from processing.core.ProcessingConfig import ProcessingConfig, settingsWatcher
from processing.gui.MessageDialog import MessageDialog
from processing.gui.AlgorithmDialog import AlgorithmDialog
from processing.gui.BatchAlgorithmDialog import BatchAlgorithmDialog
from processing.gui.EditRenderingStylesDialog import EditRenderingStylesDialog
from processing.gui.MessageBarProgress import MessageBarProgress
from processing.gui.AlgorithmExecutor import execute
from processing.gui.ProviderActions import (ProviderActions,
                                            ProviderContextMenuActions)
from processing.tools import dataobjects

pluginPath = os.path.split(os.path.dirname(__file__))[0]
WIDGET, BASE = uic.loadUiType(
    os.path.join(pluginPath, 'ui', 'ProcessingToolbox.ui'))


class ProcessingToolbox(QgsDockWidget, WIDGET):
    ALG_ITEM = 'ALG_ITEM'
    PROVIDER_ITEM = 'PROVIDER_ITEM'
    GROUP_ITEM = 'GROUP_ITEM'

    NAME_ROLE = Qt.UserRole
    TAG_ROLE = Qt.UserRole + 1
    TYPE_ROLE = Qt.UserRole + 2

    def __init__(self):
        super(ProcessingToolbox, self).__init__(None)
        self.tipWasClosed = False
        self.setupUi(self)
        self.setAllowedAreas(Qt.LeftDockWidgetArea | Qt.RightDockWidgetArea)
        self.processingToolbar.setIconSize(iface.iconSize(True))

        self.searchBox.textChanged.connect(self.textChanged)
        self.searchBox.returnPressed.connect(self.activateCurrent)
        self.algorithmTree.customContextMenuRequested.connect(
            self.showPopupMenu)
        self.algorithmTree.doubleClicked.connect(self.executeAlgorithm)
        self.txtDisabled.setVisible(False)
        self.txtTip.setVisible(self.disabledProviders())
        self.txtDisabled.linkActivated.connect(self.showDisabled)

        def openSettings(url):
            if url == "close":
                self.txtTip.setVisible(False)
                self.tipWasClosed = True
            else:
                iface.showOptionsDialog(iface.mainWindow(), 'processingOptions')
                self.txtTip.setVisible(self.disabledProviders())

        self.txtTip.linkActivated.connect(openSettings)
        if hasattr(self.searchBox, 'setPlaceholderText'):
            self.searchBox.setPlaceholderText(QCoreApplication.translate('ProcessingToolbox', 'Search…'))

        self.fillTree()

        # connect to existing providers
        for p in QgsApplication.processingRegistry().providers():
            p.algorithmsLoaded.connect(self.updateProvider)
            if p.isActive():
                self.addProviderActions(p)

        QgsApplication.processingRegistry().providerRemoved.connect(self.removeProvider)
        QgsApplication.processingRegistry().providerAdded.connect(self.addProvider)
        settingsWatcher.settingsChanged.connect(self.fillTree)

    def showDisabled(self):
        self.txtDisabled.setVisible(False)
        for provider_id in self.disabledWithMatchingAlgs:
            self.disabledProviderItems[provider_id].setHidden(False)
        self.algorithmTree.expandAll()

    def disabledProviders(self):
        showTip = ProcessingConfig.getSetting(ProcessingConfig.SHOW_PROVIDERS_TOOLTIP)
        if not showTip or self.tipWasClosed:
            return False

        for provider in QgsApplication.processingRegistry().providers():
            if not provider.isActive():
                return True

        return False

    def textChanged(self):
        text = self.searchBox.text().strip(' ').lower()
        for item in list(self.disabledProviderItems.values()):
            item.setHidden(True)
        self._filterItem(self.algorithmTree.invisibleRootItem(), [t for t in text.split(' ') if t])
        if text:
            self.algorithmTree.expandAll()
            self.disabledWithMatchingAlgs = []
            for provider in QgsApplication.processingRegistry().providers():
                if not provider.isActive():
                    for alg in provider.algorithms():
                        if text in alg.name():
                            self.disabledWithMatchingAlgs.append(provider.id())
                            break
            showTip = ProcessingConfig.getSetting(ProcessingConfig.SHOW_PROVIDERS_TOOLTIP)
            if showTip:
                self.txtDisabled.setVisible(bool(self.disabledWithMatchingAlgs))

            if self.algorithmTree.currentItem() is None or self.algorithmTree.currentItem().isHidden():
                # if previously selected item was hidden, auto select the first visible algorithm
                first_visible = self._findFirstVisibleAlgorithm(self.algorithmTree.invisibleRootItem())
                if first_visible is not None:
                    self.algorithmTree.setCurrentItem(first_visible)
        else:
            self.algorithmTree.collapseAll()
            self.algorithmTree.invisibleRootItem().child(0).setExpanded(True)
            self.txtDisabled.setVisible(False)

    def _filterItem(self, item, text):
        if (item.childCount() > 0):
            show = False
            for i in range(item.childCount()):
                child = item.child(i)
                showChild = self._filterItem(child, text)
                show = (showChild or show) and item not in list(self.disabledProviderItems.values())
            item.setHidden(not show)
            return show
        elif isinstance(item, TreeAlgorithmItem):
            # hide if every part of text is not contained somewhere in either the item text or item user role
            item_text = [item.text(0).lower(), item.data(0, ProcessingToolbox.NAME_ROLE).lower()]
            item_text.append(item.alg.id())
            item_text.extend(item.data(0, ProcessingToolbox.TAG_ROLE))

            hide = bool(text) and not all(
                any(part in t for t in item_text)
                for part in text)

            item.setHidden(hide)
            return not hide
        else:
            item.setHidden(True)
            return False

    def _findFirstVisibleAlgorithm(self, item):
        """
        Returns the first visible algorithm in the tree widget
        """
        if item is None:
            return None
        if item.childCount() > 0:
            for i in range(item.childCount()):
                child = item.child(i)
                first_visible = self._findFirstVisibleAlgorithm(child)
                if first_visible is not None:
                    return first_visible
            return None
        elif isinstance(item, TreeAlgorithmItem):
            if not item.isHidden():
                return item
            else:
                return None
        else:
            return None

    def addProviderActions(self, provider):
        if provider.id() in ProviderActions.actions:
            toolbarButton = QToolButton()
            toolbarButton.setObjectName('provideraction_' + provider.id())
            toolbarButton.setIcon(provider.icon())
            toolbarButton.setToolTip(provider.name())
            toolbarButton.setPopupMode(QToolButton.InstantPopup)

            actions = ProviderActions.actions[provider.id()]
            menu = QMenu(provider.name(), self)
            for action in actions:
                action.setData(self)
                act = QAction(action.name, menu)
                act.triggered.connect(action.execute)
                menu.addAction(act)
            toolbarButton.setMenu(menu)
            self.processingToolbar.addWidget(toolbarButton)

    def activateProvider(self, id):
        provider = QgsApplication.processingRegistry().providerById(id)
        if not provider.canBeActivated():
            QMessageBox.warning(self, self.tr("Activate provider"),
                                self.tr("The provider has been activated, but it might need additional configuration."))
            return

        try:
            # not part of the base class - only some providers have a setActive member
            provider.setActive(True)
            self.addProviderActions(provider)
            self.fillTree()
            self.textChanged()
            self.showDisabled()
        except:
            QMessageBox.warning(self, self.tr("Activate provider"),
                                self.tr("The provider could not be activated."))

    def updateProvider(self):
        provider = self.sender()
        item = self._providerItem(provider.id())
        if item is not None:
            item.refresh()
            item.sortChildren(0, Qt.AscendingOrder)
            for i in range(item.childCount()):
                item.child(i).sortChildren(0, Qt.AscendingOrder)
            self.addRecentAlgorithms(True)

    def removeProvider(self, provider_id):
        item = self._providerItem(provider_id)
        if item is not None:
            self.algorithmTree.invisibleRootItem().removeChild(item)
        button = self.findChild(QToolButton, 'provideraction-' + provider_id)
        if button:
            self.processingToolbar.removeChild(button)

    def _providerItem(self, provider_id):
        for i in range(self.algorithmTree.invisibleRootItem().childCount()):
            child = self.algorithmTree.invisibleRootItem().child(i)
            if isinstance(child, TreeProviderItem):
                if child.provider.id() == provider_id:
                    return child

    def showPopupMenu(self, point):
        item = self.algorithmTree.itemAt(point)
        popupmenu = QMenu()
        if isinstance(item, TreeAlgorithmItem):
            alg = item.alg
            executeAction = QAction(QCoreApplication.translate('ProcessingToolbox', 'Execute…'), self.algorithmTree)
            executeAction.triggered.connect(self.executeAlgorithm)
            popupmenu.addAction(executeAction)
            if alg.flags() & QgsProcessingAlgorithm.FlagSupportsBatch:
                executeBatchAction = QAction(
                    QCoreApplication.translate('ProcessingToolbox', 'Execute as Batch Process…'),
                    self.algorithmTree)
                executeBatchAction.triggered.connect(
                    self.executeAlgorithmAsBatchProcess)
                popupmenu.addAction(executeBatchAction)
            popupmenu.addSeparator()
            editRenderingStylesAction = QAction(
                QCoreApplication.translate('ProcessingToolbox', 'Edit Rendering Styles for Outputs…'),
                self.algorithmTree)
            editRenderingStylesAction.triggered.connect(
                self.editRenderingStyles)
            popupmenu.addAction(editRenderingStylesAction)
            actions = ProviderContextMenuActions.actions
            if len(actions) > 0:
                popupmenu.addSeparator()
            for action in actions:
                action.setData(item.alg, self)
                if action.isEnabled():
                    contextMenuAction = QAction(action.name,
                                                self.algorithmTree)
                    contextMenuAction.triggered.connect(action.execute)
                    popupmenu.addAction(contextMenuAction)

            popupmenu.exec_(self.algorithmTree.mapToGlobal(point))

    def editRenderingStyles(self):
        item = self.algorithmTree.currentItem()
        if isinstance(item, TreeAlgorithmItem):
            alg = QgsApplication.processingRegistry().createAlgorithmById(item.alg.id())
            dlg = EditRenderingStylesDialog(alg)
            dlg.exec_()

    def activateCurrent(self):
        self.executeAlgorithm()

    def executeAlgorithmAsBatchProcess(self):
        item = self.algorithmTree.currentItem()
        if isinstance(item, TreeAlgorithmItem):
            alg = QgsApplication.processingRegistry().createAlgorithmById(item.alg.id())
            if alg:
                dlg = BatchAlgorithmDialog(alg)
                dlg.show()
                dlg.exec_()

    def executeAlgorithm(self):
        item = self.algorithmTree.currentItem()
        if isinstance(item, TreeAlgorithmItem):
            alg = QgsApplication.processingRegistry().createAlgorithmById(item.alg.id())
            if not alg:
                return

            ok, message = alg.canExecute()
            if not ok:
                dlg = MessageDialog()
                dlg.setTitle(self.tr('Error executing algorithm'))
                dlg.setMessage(
                    self.tr('<h3>This algorithm cannot '
                            'be run :-( </h3>\n{0}').format(message))
                dlg.exec_()
                return

            if alg.countVisibleParameters() > 0:
                dlg = alg.createCustomParametersWidget(self)

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
                if dlg.wasExecuted():
                    showRecent = ProcessingConfig.getSetting(
                        ProcessingConfig.SHOW_RECENT_ALGORITHMS)
                    if showRecent:
                        self.addRecentAlgorithms(True)
            else:
                feedback = MessageBarProgress()
                context = dataobjects.createContext(feedback)
                parameters = {}
                ret, results = execute(alg, parameters, context, feedback)
                handleAlgorithmResults(alg, context, feedback)
                feedback.close()

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
                    if recentItem.text(0) == self.tr('Recently used'):
                        treeWidget = recentItem.treeWidget()
                        treeWidget.takeTopLevelItem(
                            treeWidget.indexOfTopLevelItem(recentItem))

                recentItem = QTreeWidgetItem()
                recentItem.setText(0, self.tr('Recently used'))
                for algname in recent:
                    alg = QgsApplication.processingRegistry().createAlgorithmById(algname)
                    if alg is not None:
                        algItem = TreeAlgorithmItem(alg)
                        recentItem.addChild(algItem)
                        found = True
                if found:
                    self.algorithmTree.insertTopLevelItem(0, recentItem)
                    recentItem.setExpanded(True)

            self.algorithmTree.setWordWrap(True)

    def addProvider(self, provider_id):
        provider = QgsApplication.processingRegistry().providerById(provider_id)
        providerItem = TreeProviderItem(provider, self.algorithmTree, self)

        for i in range(self.algorithmTree.invisibleRootItem().childCount()):
            child = self.algorithmTree.invisibleRootItem().child(i)
            if isinstance(child, TreeProviderItem):
                if child.text(0) > providerItem.text(0):
                    break

        self.algorithmTree.insertTopLevelItem(i, providerItem)
        if not provider.isActive():
            providerItem.setHidden(True)
            self.disabledProviderItems[provider.id()] = providerItem

        provider.algorithmsLoaded.connect(self.updateProvider)

    def fillTreeUsingProviders(self):
        self.algorithmTree.clear()
        self.disabledProviderItems = {}

        # TODO - replace with proper model for toolbox!

        # first add qgis/native providers, since they create top level groups
        for provider in QgsApplication.processingRegistry().providers():
            if provider.id() in ('qgis', 'native', '3d'):
                self.addAlgorithmsFromProvider(provider, self.algorithmTree.invisibleRootItem())
            else:
                continue
        self.algorithmTree.sortItems(0, Qt.AscendingOrder)

        for provider in QgsApplication.processingRegistry().providers():
            if provider.id() in ('qgis', 'native', '3d'):
                # already added
                continue
            else:
                providerItem = TreeProviderItem(provider, self.algorithmTree, self)

                # insert non-native providers at end of tree, alphabetically
                for i in range(self.algorithmTree.invisibleRootItem().childCount()):
                    child = self.algorithmTree.invisibleRootItem().child(i)
                    if isinstance(child, TreeProviderItem):
                        if child.text(0) > providerItem.text(0):
                            break

                self.algorithmTree.insertTopLevelItem(i + 1, providerItem)
                if not provider.isActive():
                    providerItem.setHidden(True)
                    self.disabledProviderItems[provider.id()] = providerItem

    def addAlgorithmsFromProvider(self, provider, parent):
        groups = {}
        count = 0
        algs = provider.algorithms()
        active = provider.isActive()

        # Add algorithms
        for alg in algs:
            if alg.flags() & QgsProcessingAlgorithm.FlagHideFromToolbox:
                continue
            groupItem = None
            if alg.group() in groups:
                groupItem = groups[alg.group()]
            else:
                # check if group already exists
                for i in range(parent.childCount()):
                    if parent.child(i).text(0) == alg.group():
                        groupItem = parent.child(i)
                        groups[alg.group()] = groupItem
                        break

                if not groupItem:
                    groupItem = TreeGroupItem(alg.group())
                    if not active:
                        groupItem.setInactive()
                    if provider.id() in ('qgis', 'native', '3d'):
                        groupItem.setIcon(0, provider.icon())
                    groups[alg.group()] = groupItem
            algItem = TreeAlgorithmItem(alg)
            if not active:
                algItem.setForeground(0, Qt.darkGray)
            groupItem.addChild(algItem)
            count += 1

        text = provider.name()

        if not provider.id() in ('qgis', 'native', '3d'):
            if not active:
                def activateProvider():
                    self.activateProvider(provider.id())

                label = QLabel(text + "&nbsp;&nbsp;&nbsp;&nbsp;<a href='%s'>Activate</a>")
                label.setStyleSheet("QLabel {background-color: white; color: grey;}")
                label.linkActivated.connect(activateProvider)
                self.algorithmTree.setItemWidget(parent, 0, label)
            else:
                parent.setText(0, text)

        for group, groupItem in sorted(groups.items(), key=operator.itemgetter(1)):
            parent.addChild(groupItem)

        if not provider.id() in ('qgis', 'native', '3d'):
            parent.setHidden(parent.childCount() == 0)


class TreeAlgorithmItem(QTreeWidgetItem):

    def __init__(self, alg):
        QTreeWidgetItem.__init__(self)
        self.alg = alg
        icon = alg.icon()
        nameEn = alg.name()
        name = alg.displayName()
        name = name if name != '' else nameEn
        self.setIcon(0, icon)
        self.setToolTip(0, self.formatAlgorithmTooltip(alg))
        self.setText(0, name)
        self.setData(0, ProcessingToolbox.NAME_ROLE, nameEn)
        self.setData(0, ProcessingToolbox.TAG_ROLE, alg.tags())
        self.setData(0, ProcessingToolbox.TYPE_ROLE, ProcessingToolbox.ALG_ITEM)

    def formatAlgorithmTooltip(self, alg):
        return '<p><b>{}</b></p>{}<p>{}</p>'.format(
            alg.displayName(),
            '<p>{}</p>'.format(alg.shortDescription()) if alg.shortDescription() else '',
            QCoreApplication.translate('Toolbox', 'Algorithm ID: ‘{}’').format('<i>{}</i>'.format(alg.id()))
        )


class TreeGroupItem(QTreeWidgetItem):

    def __init__(self, name):
        QTreeWidgetItem.__init__(self)
        self.setToolTip(0, name)
        self.setText(0, name)
        self.setData(0, ProcessingToolbox.NAME_ROLE, name)
        self.setData(0, ProcessingToolbox.TYPE_ROLE, ProcessingToolbox.GROUP_ITEM)

    def setInactive(self):
        self.setForeground(0, Qt.darkGray)


class TreeProviderItem(QTreeWidgetItem):

    def __init__(self, provider, tree, toolbox):
        QTreeWidgetItem.__init__(self, None)
        self.tree = tree
        self.toolbox = toolbox
        self.provider = provider
        self.setIcon(0, self.provider.icon())
        self.setData(0, ProcessingToolbox.TYPE_ROLE, ProcessingToolbox.PROVIDER_ITEM)
        self.setToolTip(0, self.provider.longName())
        self.populate()

    def refresh(self):
        self.takeChildren()
        self.populate()

    def populate(self):
        self.toolbox.addAlgorithmsFromProvider(self.provider, self)
