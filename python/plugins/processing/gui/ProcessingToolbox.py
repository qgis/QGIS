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
from qgis.PyQt.QtCore import Qt, QCoreApplication
from qgis.PyQt.QtWidgets import QToolButton, QMenu, QAction
from qgis.utils import iface
from qgis.core import (QgsWkbTypes,
                       QgsMapLayer,
                       QgsApplication,
                       QgsProcessingAlgorithm)
from qgis.gui import (QgsGui,
                      QgsDockWidget,
                      QgsProcessingToolboxProxyModel)

from processing.gui.Postprocessing import handleAlgorithmResults
from processing.core.ProcessingConfig import ProcessingConfig
from processing.gui.MessageDialog import MessageDialog
from processing.gui.AlgorithmDialog import AlgorithmDialog
from processing.gui.BatchAlgorithmDialog import BatchAlgorithmDialog
from processing.gui.EditRenderingStylesDialog import EditRenderingStylesDialog
from processing.gui.MessageBarProgress import MessageBarProgress
from processing.gui.AlgorithmExecutor import execute
from processing.gui.ProviderActions import (ProviderActions,
                                            ProviderContextMenuActions)
from processing.tools import dataobjects
from processing.gui.AlgorithmExecutor import execute_in_place

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
        self.in_place_mode = False
        self.setupUi(self)
        self.setAllowedAreas(Qt.LeftDockWidgetArea | Qt.RightDockWidgetArea)
        self.processingToolbar.setIconSize(iface.iconSize(True))

        self.algorithmTree.setRegistry(QgsApplication.processingRegistry(),
                                       QgsGui.instance().processingRecentAlgorithmLog())
        filters = QgsProcessingToolboxProxyModel.Filters(QgsProcessingToolboxProxyModel.FilterToolbox)
        if ProcessingConfig.getSetting(ProcessingConfig.SHOW_ALGORITHMS_KNOWN_ISSUES):
            filters |= QgsProcessingToolboxProxyModel.FilterShowKnownIssues
        self.algorithmTree.setFilters(filters)

        self.searchBox.setShowSearchIcon(True)

        self.searchBox.textChanged.connect(self.set_filter_string)
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

        iface.currentLayerChanged.connect(self.layer_changed)

    def set_filter_string(self, string):
        filters = self.algorithmTree.filters()
        if ProcessingConfig.getSetting(ProcessingConfig.SHOW_ALGORITHMS_KNOWN_ISSUES):
            filters |= QgsProcessingToolboxProxyModel.FilterShowKnownIssues
        else:
            filters &= ~QgsProcessingToolboxProxyModel.FilterShowKnownIssues
        self.algorithmTree.setFilters(filters)
        self.algorithmTree.setFilterString(string)

    def set_in_place_edit_mode(self, enabled):
        filters = QgsProcessingToolboxProxyModel.Filters(QgsProcessingToolboxProxyModel.FilterToolbox)
        if ProcessingConfig.getSetting(ProcessingConfig.SHOW_ALGORITHMS_KNOWN_ISSUES):
            filters |= QgsProcessingToolboxProxyModel.FilterShowKnownIssues

        if enabled:
            self.algorithmTree.setFilters(filters | QgsProcessingToolboxProxyModel.FilterInPlace)
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
        if provider is not None:
            self.addProviderActions(provider)

    def removeProvider(self, provider_id):
        button = self.findChild(QToolButton, 'provideraction-' + provider_id)
        if button:
            self.processingToolbar.removeChild(button)

    def showPopupMenu(self, point):
        index = self.algorithmTree.indexAt(point)
        popupmenu = QMenu()
        alg = self.algorithmTree.algorithmForIndex(index)
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
                if action.is_separator:
                    popupmenu.addSeparator()
                elif action.isEnabled():
                    contextMenuAction = QAction(action.name,
                                                popupmenu)
                    contextMenuAction.setIcon(action.icon())
                    contextMenuAction.triggered.connect(action.execute)
                    popupmenu.addAction(contextMenuAction)

            popupmenu.exec_(self.algorithmTree.mapToGlobal(point))

    def editRenderingStyles(self):
        alg = self.algorithmTree.selectedAlgorithm().create() if self.algorithmTree.selectedAlgorithm() is not None else None
        if alg is not None:
            dlg = EditRenderingStylesDialog(alg)
            dlg.exec_()

    def activateCurrent(self):
        self.executeAlgorithm()

    def executeAlgorithmAsBatchProcess(self):
        alg = self.algorithmTree.selectedAlgorithm().create() if self.algorithmTree.selectedAlgorithm() is not None else None
        if alg is not None:
            dlg = BatchAlgorithmDialog(alg, iface.mainWindow())
            dlg.setAttribute(Qt.WA_DeleteOnClose)
            dlg.show()
            dlg.exec_()

    def executeAlgorithm(self):
        alg = self.algorithmTree.selectedAlgorithm().create() if self.algorithmTree.selectedAlgorithm() is not None else None
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

            if self.in_place_mode and not [d for d in alg.parameterDefinitions() if d.name() not in ('INPUT', 'OUTPUT')]:
                parameters = {}
                feedback = MessageBarProgress(algname=alg.displayName())
                ok, results = execute_in_place(alg, parameters, feedback=feedback)
                if ok:
                    iface.messageBar().pushSuccess('', self.tr('{} complete').format(alg.displayName()))
                feedback.close()
                # MessageBarProgress handles errors
                return

            if alg.countVisibleParameters() > 0:
                dlg = alg.createCustomParametersWidget(self)

                if not dlg:
                    dlg = AlgorithmDialog(alg, self.in_place_mode, iface.mainWindow())
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
                feedback = MessageBarProgress(algname=alg.displayName())
                context = dataobjects.createContext(feedback)
                parameters = {}
                ret, results = execute(alg, parameters, context, feedback)
                handleAlgorithmResults(alg, context, feedback)
                feedback.close()
