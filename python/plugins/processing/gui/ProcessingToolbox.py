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
import warnings

from qgis.PyQt import uic
from qgis.PyQt.QtCore import Qt, QCoreApplication, QModelIndex, QItemSelectionModel
from qgis.PyQt.QtWidgets import QToolButton, QMenu, QAction, QMessageBox
from qgis.utils import iface
from qgis.core import (QgsApplication,
                       QgsProcessingAlgorithm)
from qgis.gui import (QgsGui,
                      QgsDockWidget,
                      QgsProcessingToolboxProxyModel)

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

with warnings.catch_warnings():
    warnings.filterwarnings("ignore", category=DeprecationWarning)
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

        self.model = QgsProcessingToolboxProxyModel(self,
                                                    QgsApplication.processingRegistry(),
                                                    QgsGui.instance().processingRecentAlgorithmLog())
        self.model.setFilters(QgsProcessingToolboxProxyModel.FilterToolbox)
        self.algorithmTree.setModel(self.model)

        self.searchBox.setShowSearchIcon(True)

        self.searchBox.textChanged.connect(self.textChanged)
        self.searchBox.returnPressed.connect(self.activateCurrent)
        self.algorithmTree.customContextMenuRequested.connect(
            self.showPopupMenu)
        self.algorithmTree.doubleClicked.connect(self.executeAlgorithm)
        self.txtTip.setVisible(self.disabledProviders())

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

        # connect to existing providers
        for p in QgsApplication.processingRegistry().providers():
            if p.isActive():
                self.addProviderActions(p)

        QgsApplication.processingRegistry().providerRemoved.connect(self.addProvider)
        QgsApplication.processingRegistry().providerRemoved.connect(self.removeProvider)

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
        self.model.setFilterString(text)
        if text:
            self.algorithmTree.expandAll()
            if not self.algorithmTree.selectionModel().hasSelection():
                # if previously selected item was hidden, auto select the first visible algorithm
                first_visible_index = self._findFirstVisibleAlgorithm(QModelIndex())
                if first_visible_index is not None:
                    self.algorithmTree.selectionModel().setCurrentIndex(first_visible_index, QItemSelectionModel.ClearAndSelect)
        else:
            self.algorithmTree.collapseAll()

    def _findFirstVisibleAlgorithm(self, parent_index):
        """
        Returns the first visible algorithm in the tree widget
        """
        for r in range(self.model.rowCount(parent_index)):
            proxy_index = self.model.index(r, 0, parent_index)
            source_index = self.model.mapToSource(proxy_index)
            if self.model.toolboxModel().isAlgorithm(source_index):
                return proxy_index
            index = self._findFirstVisibleAlgorithm(proxy_index)
            if index is not None:
                return index
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

    def addProvider(self, provider_id):
        provider = QgsApplication.processingRegistry().providerById(provider_id)
        self.addProviderActions(provider)

    def removeProvider(self, provider_id):
        button = self.findChild(QToolButton, 'provideraction-' + provider_id)
        if button:
            self.processingToolbar.removeChild(button)

    def algorithm_for_index(self, index):
        source_index = self.model.mapToSource(index)
        if self.model.toolboxModel().isAlgorithm(source_index):
            return self.model.toolboxModel().algorithmForIndex(source_index)
        return None

    def selected_algorithm(self):
        if self.algorithmTree.selectionModel().hasSelection():
            index = self.algorithmTree.selectionModel().selectedIndexes()[0]
            return self.algorithm_for_index(index)
        return None

    def showPopupMenu(self, point):
        index = self.algorithmTree.indexAt(point)
        popupmenu = QMenu()
        alg = self.algorithm_for_index(index)
        if alg is not None:
            executeAction = QAction(QCoreApplication.translate('ProcessingToolbox', 'Execute…'), popupmenu)
            executeAction.triggered.connect(self.executeAlgorithm)
            popupmenu.addAction(executeAction)
            if alg.flags() & QgsProcessingAlgorithm.FlagSupportsBatch:
                executeBatchAction = QAction(
                    QCoreApplication.translate('ProcessingToolbox', 'Execute as Batch Process…'),
                    popupmenu)
                executeBatchAction.triggered.connect(
                    self.executeAlgorithmAsBatchProcess)
                popupmenu.addAction(executeBatchAction)
            popupmenu.addSeparator()
            editRenderingStylesAction = QAction(
                QCoreApplication.translate('ProcessingToolbox', 'Edit Rendering Styles for Outputs…'),
                popupmenu)
            editRenderingStylesAction.triggered.connect(
                self.editRenderingStyles)
            popupmenu.addAction(editRenderingStylesAction)
            actions = ProviderContextMenuActions.actions
            if len(actions) > 0:
                popupmenu.addSeparator()
            for action in actions:
                action.setData(alg, self)
                if action.isEnabled():
                    contextMenuAction = QAction(action.name,
                                                popupmenu)
                    contextMenuAction.triggered.connect(action.execute)
                    popupmenu.addAction(contextMenuAction)

            popupmenu.exec_(self.algorithmTree.mapToGlobal(point))

    def editRenderingStyles(self):
        alg = self.selected_algorithm()
        if alg is not None:
            dlg = EditRenderingStylesDialog(alg)
            dlg.exec_()

    def activateCurrent(self):
        self.executeAlgorithm()

    def executeAlgorithmAsBatchProcess(self):
        alg = self.selected_algorithm()
        if alg is not None:
            dlg = BatchAlgorithmDialog(alg)
            dlg.show()
            dlg.exec_()

    def executeAlgorithm(self):
        alg = self.selected_algorithm()
        if alg is not None:
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
            else:
                feedback = MessageBarProgress()
                context = dataobjects.createContext(feedback)
                parameters = {}
                ret, results = execute(alg, parameters, context, feedback)
                handleAlgorithmResults(alg, context, feedback)
                feedback.close()
