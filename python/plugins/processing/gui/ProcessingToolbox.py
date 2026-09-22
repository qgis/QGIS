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

__author__ = "Victor Olaya"
__date__ = "August 2012"
__copyright__ = "(C) 2012, Victor Olaya"

import os
import warnings
from functools import partial

from qgis.core import Qgis, QgsApplication, QgsMapLayerType, QgsProcessingAlgorithm
from qgis.gui import (
    QgsDockWidget,
    QgsGui,
    QgsProcessingActionContext,
    QgsProcessingToolboxProxyModel,
)
from qgis.PyQt import uic
from qgis.PyQt.QtCore import QCoreApplication, Qt, pyqtSignal
from qgis.PyQt.QtWidgets import QAction, QMenu, QToolButton, QWidget
from qgis.utils import iface

from processing.core.ProcessingConfig import ProcessingConfig

pluginPath = os.path.split(os.path.dirname(__file__))[0]

with warnings.catch_warnings():
    warnings.filterwarnings("ignore", category=DeprecationWarning)
    WIDGET, BASE = uic.loadUiType(
        os.path.join(pluginPath, "ui", "ProcessingToolbox.ui")
    )


class ProcessingToolbox(QgsDockWidget, WIDGET):
    ALG_ITEM = "ALG_ITEM"
    PROVIDER_ITEM = "PROVIDER_ITEM"
    GROUP_ITEM = "GROUP_ITEM"

    NAME_ROLE = Qt.ItemDataRole.UserRole
    TAG_ROLE = Qt.ItemDataRole.UserRole + 1
    TYPE_ROLE = Qt.ItemDataRole.UserRole + 2

    # Trigger algorithm execution
    executeWithGui = pyqtSignal(str, QWidget, bool, bool)

    def __init__(self):
        super().__init__(None)
        self.tipWasClosed = False
        self.in_place_mode = False
        self.setupUi(self)
        self.setAllowedAreas(
            Qt.DockWidgetArea.LeftDockWidgetArea | Qt.DockWidgetArea.RightDockWidgetArea
        )
        self.processingToolbar.setIconSize(
            QgsGui.iconSize(Qgis.UserInterfaceIconType.DockedToolbar)
        )

        self.algorithmTree.setRegistry(
            QgsApplication.processingRegistry(),
            QgsGui.instance().processingRecentAlgorithmLog(),
            QgsGui.instance().processingFavoriteAlgorithmManager(),
        )
        filters = QgsProcessingToolboxProxyModel.Filters(
            QgsProcessingToolboxProxyModel.Filter.FilterToolbox
        )
        if ProcessingConfig.getSetting(ProcessingConfig.SHOW_ALGORITHMS_KNOWN_ISSUES):
            filters |= QgsProcessingToolboxProxyModel.Filter.FilterShowKnownIssues
        self.algorithmTree.setFilters(filters)

        self.searchBox.setShowSearchIcon(True)

        self.searchBox.textChanged.connect(self.set_filter_string)
        self.searchBox.returnPressed.connect(self.activateCurrent)
        self.algorithmTree.customContextMenuRequested.connect(self.showPopupMenu)
        self.algorithmTree.doubleClicked.connect(self.executeAlgorithm)
        self.txtTip.setVisible(self.disabledProviders())

        def openSettings(url):
            if url == "close":
                self.txtTip.setVisible(False)
                self.tipWasClosed = True
            else:
                iface.showOptionsDialog(iface.mainWindow(), "processingOptions")
                self.txtTip.setVisible(self.disabledProviders())

        self.txtTip.linkActivated.connect(openSettings)
        if hasattr(self.searchBox, "setPlaceholderText"):
            self.searchBox.setPlaceholderText(
                QCoreApplication.translate("ProcessingToolbox", "Search…")
            )

        # connect to existing providers
        for p in QgsApplication.processingRegistry().providers():
            if p.isActive():
                self.addProviderActions(p)

        QgsApplication.processingRegistry().providerRemoved.connect(self.addProvider)
        QgsApplication.processingRegistry().providerRemoved.connect(self.removeProvider)

        iface.currentLayerChanged.connect(self.layer_changed)

    def set_filter_string(self, string):
        filters = self.algorithmTree.filters()
        if ProcessingConfig.getSetting(ProcessingConfig.SHOW_ALGORITHMS_KNOWN_ISSUES):
            filters |= QgsProcessingToolboxProxyModel.Filter.FilterShowKnownIssues
        else:
            filters &= ~QgsProcessingToolboxProxyModel.Filter.FilterShowKnownIssues
        self.algorithmTree.setFilters(filters)
        self.algorithmTree.setFilterString(string)

    def set_in_place_edit_mode(self, enabled):
        filters = QgsProcessingToolboxProxyModel.Filters(
            QgsProcessingToolboxProxyModel.Filter.FilterToolbox
        )
        if ProcessingConfig.getSetting(ProcessingConfig.SHOW_ALGORITHMS_KNOWN_ISSUES):
            filters |= QgsProcessingToolboxProxyModel.Filter.FilterShowKnownIssues

        if enabled:
            self.algorithmTree.setFilters(
                filters | QgsProcessingToolboxProxyModel.Filter.FilterInPlace
            )
        else:
            self.algorithmTree.setFilters(filters)
        self.in_place_mode = enabled

    def layer_changed(self, layer):
        if layer is None or layer.type() != QgsMapLayerType.VectorLayer:
            return
        self.algorithmTree.setInPlaceLayer(layer)

    def disabledProviders(self):
        showTip = ProcessingConfig.getSetting(ProcessingConfig.SHOW_PROVIDERS_TOOLTIP)
        if not showTip or self.tipWasClosed:
            return False

        for provider in QgsApplication.processingRegistry().providers():
            if not provider.isActive() and provider.canBeActivated():
                return True

        return False

    def addProviderActions(self, provider):
        actions = QgsGui.processingGuiRegistry().toolboxActionsForProvider(
            provider.id()
        )

        if actions:
            toolbarButton = QToolButton()
            toolbarButton.setObjectName("provideraction_" + provider.id())
            toolbarButton.setIcon(provider.icon())
            toolbarButton.setToolTip(provider.name())
            toolbarButton.setPopupMode(QToolButton.ToolButtonPopupMode.InstantPopup)

            menu = QMenu(provider.name(), self)
            menu.setObjectName(provider.name() + "_menu")
            for action in actions:
                action.setData(self)
                act = QAction(action.actionName(), menu)
                act.setObjectName(action.actionName())
                act.triggered.connect(partial(self._trigger_action, action))
                menu.addAction(act)
            toolbarButton.setMenu(menu)
            self.processingToolbar.addWidget(toolbarButton)

    def _trigger_action(self, action):
        context = QgsProcessingActionContext()
        context.setParentWidget(self)
        action.trigger(context)

    def addProvider(self, provider_id):
        provider = QgsApplication.processingRegistry().providerById(provider_id)
        if provider is not None:
            self.addProviderActions(provider)

    def removeProvider(self, provider_id):
        button = self.findChild(QToolButton, "provideraction-" + provider_id)
        if button:
            self.processingToolbar.removeChild(button)

    def showPopupMenu(self, point):
        index = self.algorithmTree.indexAt(point)
        popupmenu = QMenu()
        alg = self.algorithmTree.algorithmForIndex(index)
        if alg is not None:
            executeAction = QAction(
                QCoreApplication.translate("ProcessingToolbox", "Execute…"), popupmenu
            )
            executeAction.triggered.connect(self.executeAlgorithm)
            popupmenu.addAction(executeAction)
            if alg.flags() & QgsProcessingAlgorithm.Flag.FlagSupportsBatch:
                executeBatchAction = QAction(
                    QCoreApplication.translate(
                        "ProcessingToolbox", "Execute as Batch Process…"
                    ),
                    popupmenu,
                )
                executeBatchAction.triggered.connect(
                    self.executeAlgorithmAsBatchProcess
                )
                popupmenu.addAction(executeBatchAction)
            popupmenu.addSeparator()

            actions = QgsGui.processingGuiRegistry().toolboxContextActions()
            if len(actions) > 0:
                popupmenu.addSeparator()
            for action in actions:
                action.setData(alg, self)
                provider_id = alg.provider().id() if alg.provider() else None
                if action.isSeparator():
                    popupmenu.addSeparator()
                elif action.isCompatibleWithAlgorithm(provider_id, alg.name()):
                    contextMenuAction = QAction(action.actionName(), popupmenu)
                    contextMenuAction.setIcon(action.icon())
                    contextMenuAction.triggered.connect(
                        partial(
                            self._trigger_context_action,
                            action,
                            provider_id,
                            alg.name(),
                        )
                    )
                    popupmenu.addAction(contextMenuAction)

            popupmenu.exec(self.algorithmTree.mapToGlobal(point))

    def _trigger_context_action(self, action, provider_id: str, algorithm_name: str):
        context = QgsProcessingActionContext()
        context.setParentWidget(self)
        context.setAlgorithmName(algorithm_name)
        context.setProviderId(provider_id)
        action.trigger(context)

    def activateCurrent(self):
        self.executeAlgorithm()

    def executeAlgorithmAsBatchProcess(self):
        alg = self.algorithmTree.selectedAlgorithm()
        if alg is not None:
            self.executeWithGui.emit(alg.id(), self, self.in_place_mode, True)

    def executeAlgorithm(self):
        alg = self.algorithmTree.selectedAlgorithm()
        if alg is not None:
            self.executeWithGui.emit(alg.id(), self, self.in_place_mode, False)
