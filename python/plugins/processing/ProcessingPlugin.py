# -*- coding: utf-8 -*-

"""
***************************************************************************
    ProcessingPlugin.py
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

import shutil
import os
import sys
from functools import partial

from qgis.core import (QgsApplication,
                       QgsProcessingUtils,
                       QgsProcessingModelAlgorithm,
                       QgsProcessingAlgorithm,
                       QgsDataItemProvider,
                       QgsDataProvider,
                       QgsDataItem,
                       QgsMapLayerType,
                       QgsMimeDataUtils,
                       QgsSettings)
from qgis.gui import (QgsGui,
                      QgsOptionsWidgetFactory,
                      QgsCustomDropHandler)
from qgis.PyQt.QtCore import QObject, Qt, QItemSelectionModel, QCoreApplication, QDir, QFileInfo, pyqtSlot
from qgis.PyQt.QtWidgets import QWidget, QMenu, QAction
from qgis.PyQt.QtGui import QIcon, QKeySequence
from qgis.utils import iface

from processing.core.Processing import Processing
from processing.gui.ProcessingToolbox import ProcessingToolbox
from processing.gui.HistoryDialog import HistoryDialog
from processing.gui.ConfigDialog import ConfigOptionsPage
from processing.gui.ResultsDock import ResultsDock
from processing.gui.MessageDialog import MessageDialog
from processing.gui.MessageBarProgress import MessageBarProgress
from processing.gui.AlgorithmLocatorFilter import (AlgorithmLocatorFilter,
                                                   InPlaceAlgorithmLocatorFilter)
from processing.gui.Postprocessing import handleAlgorithmResults
from processing.gui.AlgorithmExecutor import execute, execute_in_place
from processing.gui.AlgorithmDialog import AlgorithmDialog
from processing.gui.BatchAlgorithmDialog import BatchAlgorithmDialog
from processing.modeler.ModelerDialog import ModelerDialog
from processing.tools.system import tempHelpFolder
from processing.tools import dataobjects
from processing.gui.menus import removeMenus, initializeMenus, createMenus, createButtons, removeButtons
from processing.core.ProcessingResults import resultsList

pluginPath = os.path.dirname(__file__)


class ProcessingOptionsFactory(QgsOptionsWidgetFactory):

    def __init__(self):
        super(QgsOptionsWidgetFactory, self).__init__()

    def icon(self):
        return QgsApplication.getThemeIcon('/processingAlgorithm.svg')

    def createWidget(self, parent):
        return ConfigOptionsPage(parent)


class ProcessingDropHandler(QgsCustomDropHandler):

    def handleFileDrop(self, file):
        if not file.lower().endswith('.model3'):
            return False
        return self.runAlg(file)

    @staticmethod
    def runAlg(file):
        alg = QgsProcessingModelAlgorithm()
        if not alg.fromFile(file):
            return False

        alg.setProvider(QgsApplication.processingRegistry().providerById('model'))
        dlg = AlgorithmDialog(alg, parent=iface.mainWindow())
        dlg.show()
        # do NOT remove!!!! if you do then sip forgets the python subclass of AlgorithmDialog and you get a broken
        # dialog
        dlg.exec_()
        return True

    def customUriProviderKey(self):
        return 'processing'

    def handleCustomUriDrop(self, uri):
        path = uri.uri
        self.runAlg(path)


class ProcessingModelItem(QgsDataItem):

    def __init__(self, parent, name, path):
        super(ProcessingModelItem, self).__init__(QgsDataItem.Custom, parent, name, path)
        self.setState(QgsDataItem.Populated)  # no children
        self.setIconName(":/images/themes/default/processingModel.svg")
        self.setToolTip(QDir.toNativeSeparators(path))

    def hasDragEnabled(self):
        return True

    def handleDoubleClick(self):
        self.runModel()
        return True

    def mimeUri(self):
        u = QgsMimeDataUtils.Uri()
        u.layerType = "custom"
        u.providerKey = "processing"
        u.name = self.name()
        u.uri = self.path()
        return u

    def runModel(self):
        ProcessingDropHandler.runAlg(self.path())

    def editModel(self):
        dlg = ModelerDialog.create()
        dlg.loadModel(self.path())
        dlg.show()

    def actions(self, parent):
        run_model_action = QAction(QCoreApplication.translate('ProcessingPlugin', '&Run Model…'), parent)
        run_model_action.triggered.connect(self.runModel)
        edit_model_action = QAction(QCoreApplication.translate('ProcessingPlugin', '&Edit Model…'), parent)
        edit_model_action.triggered.connect(self.editModel)
        return [run_model_action, edit_model_action]


class ProcessingDataItemProvider(QgsDataItemProvider):

    def __init__(self):
        super(ProcessingDataItemProvider, self).__init__()

    def name(self):
        return 'processing'

    def capabilities(self):
        return QgsDataProvider.File

    def createDataItem(self, path, parentItem):
        file_info = QFileInfo(path)

        if file_info.suffix().lower() == 'model3':
            alg = QgsProcessingModelAlgorithm()
            if alg.fromFile(path):
                return ProcessingModelItem(parentItem, alg.name(), path)
        return None


class ProcessingPlugin(QObject):

    def __init__(self, iface):
        super().__init__()
        self.iface = iface
        self.options_factory = None
        self.drop_handler = None
        self.item_provider = None
        self.locator_filter = None
        self.edit_features_locator_filter = None
        self.initialized = False
        self.initProcessing()

    def initProcessing(self):
        if not self.initialized:
            self.initialized = True
            Processing.initialize()

    def initGui(self):
        # port old log, ONCE ONLY!
        settings = QgsSettings()
        if not settings.value("/Processing/hasPortedOldLog", False, bool):
            processing_history_provider = QgsGui.historyProviderRegistry().providerById('processing')
            if processing_history_provider:
                processing_history_provider.portOldLog()
                settings.setValue("/Processing/hasPortedOldLog", True)

        self.options_factory = ProcessingOptionsFactory()
        self.options_factory.setTitle(self.tr('Processing'))
        iface.registerOptionsWidgetFactory(self.options_factory)
        self.drop_handler = ProcessingDropHandler()
        iface.registerCustomDropHandler(self.drop_handler)
        self.item_provider = ProcessingDataItemProvider()
        QgsApplication.dataItemProviderRegistry().addProvider(self.item_provider)
        self.locator_filter = AlgorithmLocatorFilter()
        iface.registerLocatorFilter(self.locator_filter)
        # Invalidate the locator filter for in-place when active layer changes
        iface.currentLayerChanged.connect(lambda _: self.iface.invalidateLocatorResults())
        self.edit_features_locator_filter = InPlaceAlgorithmLocatorFilter()
        iface.registerLocatorFilter(self.edit_features_locator_filter)

        self.toolbox = ProcessingToolbox()
        self.iface.addDockWidget(Qt.RightDockWidgetArea, self.toolbox)
        self.toolbox.hide()
        self.toolbox.visibilityChanged.connect(self.toolboxVisibilityChanged)

        self.toolbox.executeWithGui.connect(self.executeAlgorithm)

        self.resultsDock = ResultsDock()
        self.iface.addDockWidget(Qt.RightDockWidgetArea, self.resultsDock)
        self.resultsDock.hide()

        self.menu = QMenu(self.iface.mainWindow().menuBar())
        self.menu.setObjectName('processing')
        self.menu.setTitle(self.tr('Pro&cessing'))

        self.toolboxAction = QAction(self.tr('&Toolbox'), self.iface.mainWindow())
        self.toolboxAction.setCheckable(True)
        self.toolboxAction.setObjectName('toolboxAction')
        self.toolboxAction.setIcon(
            QgsApplication.getThemeIcon("/processingAlgorithm.svg"))
        self.iface.registerMainWindowAction(self.toolboxAction,
                                            QKeySequence('Ctrl+Alt+T').toString(QKeySequence.NativeText))
        self.toolboxAction.toggled.connect(self.openToolbox)
        self.iface.attributesToolBar().insertAction(self.iface.actionOpenStatisticalSummary(), self.toolboxAction)
        self.menu.addAction(self.toolboxAction)

        self.modelerAction = QAction(
            QgsApplication.getThemeIcon("/processingModel.svg"),
            QCoreApplication.translate('ProcessingPlugin', '&Graphical Modeler…'), self.iface.mainWindow())
        self.modelerAction.setObjectName('modelerAction')
        self.modelerAction.triggered.connect(self.openModeler)
        self.iface.registerMainWindowAction(self.modelerAction,
                                            QKeySequence('Ctrl+Alt+G').toString(QKeySequence.NativeText))
        self.menu.addAction(self.modelerAction)

        self.historyAction = QAction(
            QgsApplication.getThemeIcon("/mIconHistory.svg"),
            QCoreApplication.translate('ProcessingPlugin', '&History…'), self.iface.mainWindow())
        self.historyAction.setObjectName('historyAction')
        self.historyAction.triggered.connect(self.openHistory)
        self.iface.registerMainWindowAction(self.historyAction,
                                            QKeySequence('Ctrl+Alt+H').toString(QKeySequence.NativeText))
        self.menu.addAction(self.historyAction)
        self.toolbox.processingToolbar.addAction(self.historyAction)

        self.resultsAction = QAction(
            QgsApplication.getThemeIcon("/processingResult.svg"),
            self.tr('&Results Viewer'), self.iface.mainWindow())
        self.resultsAction.setObjectName('resultsViewer')
        self.resultsAction.setCheckable(True)
        self.iface.registerMainWindowAction(self.resultsAction,
                                            QKeySequence('Ctrl+Alt+R').toString(QKeySequence.NativeText))

        self.menu.addAction(self.resultsAction)
        self.toolbox.processingToolbar.addAction(self.resultsAction)
        self.resultsDock.visibilityChanged.connect(self.resultsAction.setChecked)
        self.resultsAction.toggled.connect(self.resultsDock.setUserVisible)

        self.toolbox.processingToolbar.addSeparator()

        self.editInPlaceAction = QAction(
            QgsApplication.getThemeIcon("/mActionProcessSelected.svg"),
            self.tr('Edit Features In-Place'), self.iface.mainWindow())
        self.editInPlaceAction.setObjectName('editInPlaceFeatures')
        self.editInPlaceAction.setCheckable(True)
        self.editInPlaceAction.toggled.connect(self.editSelected)
        self.menu.addAction(self.editInPlaceAction)
        self.toolbox.processingToolbar.addAction(self.editInPlaceAction)

        self.toolbox.processingToolbar.addSeparator()

        self.optionsAction = QAction(
            QgsApplication.getThemeIcon("/mActionOptions.svg"),
            self.tr('Options'), self.iface.mainWindow())
        self.optionsAction.setObjectName('optionsAction')
        self.optionsAction.triggered.connect(self.openProcessingOptions)
        self.toolbox.processingToolbar.addAction(self.optionsAction)

        menuBar = self.iface.mainWindow().menuBar()
        menuBar.insertMenu(
            self.iface.firstRightStandardMenu().menuAction(), self.menu)

        self.menu.addSeparator()

        initializeMenus()
        createMenus()
        createButtons()

        # In-place editing button state sync
        self.iface.currentLayerChanged.connect(self.sync_in_place_button_state)
        self.iface.mapCanvas().selectionChanged.connect(self.sync_in_place_button_state)
        self.iface.actionToggleEditing().triggered.connect(partial(self.sync_in_place_button_state, None))
        self.sync_in_place_button_state()

        # Sync project models
        self.projectModelsMenu = None
        self.projectMenuAction = None
        self.projectMenuSeparator = None

        self.projectProvider = QgsApplication.instance().processingRegistry().providerById("project")
        self.projectProvider.algorithmsLoaded.connect(self.updateProjectModelMenu)

    def updateProjectModelMenu(self):
        """Add projects models to menu"""

        if self.projectMenuAction is None:
            self.projectModelsMenu = QMenu(self.tr("Models"))
            self.projectMenuAction = self.iface.projectMenu().insertMenu(self.iface.projectMenu().children()[-1], self.projectModelsMenu)
            self.projectMenuAction.setParent(self.projectModelsMenu)
            self.iface.projectMenu().insertSeparator(self.projectMenuAction)

        self.projectModelsMenu.clear()

        for model in self.projectProvider.algorithms():
            modelSubMenu = self.projectModelsMenu.addMenu(model.name())
            modelSubMenu.setParent(self.projectModelsMenu)
            action = QAction(self.tr("Execute…"), modelSubMenu)
            action.triggered.connect(partial(self.executeAlgorithm, model.id(), self.projectModelsMenu, self.toolbox.in_place_mode))
            modelSubMenu.addAction(action)
            if model.flags() & QgsProcessingAlgorithm.FlagSupportsBatch:
                action = QAction(self.tr("Execute as Batch Process…"), modelSubMenu)
                modelSubMenu.addAction(action)
                action.triggered.connect(partial(self.executeAlgorithm, model.id(), self.projectModelsMenu, self.toolbox.in_place_mode, True))

    @pyqtSlot(str, QWidget, bool, bool)
    def executeAlgorithm(self, alg_id, parent, in_place=False, as_batch=False):
        """Executes a project model with GUI interaction if needed.

        :param alg_id: algorithm id
        :type alg_id: string
        :param parent: parent widget
        :type parent: QWidget
        :param in_place: in place flag, defaults to False
        :type in_place: bool, optional
        :param as_batch: execute as batch flag, defaults to False
        :type as_batch: bool, optional
        """

        config = {}
        if in_place:
            config['IN_PLACE'] = True

        alg = QgsApplication.instance().processingRegistry().createAlgorithmById(alg_id, config)

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

            if as_batch:
                dlg = BatchAlgorithmDialog(alg, iface.mainWindow())
                dlg.show()
                dlg.exec_()
            else:
                in_place_input_parameter_name = 'INPUT'
                if hasattr(alg, 'inputParameterName'):
                    in_place_input_parameter_name = alg.inputParameterName()

                if in_place and not [d for d in alg.parameterDefinitions() if d.name() not in (in_place_input_parameter_name, 'OUTPUT')]:
                    parameters = {}
                    feedback = MessageBarProgress(algname=alg.displayName())
                    ok, results = execute_in_place(alg, parameters, feedback=feedback)
                    if ok:
                        iface.messageBar().pushSuccess('', self.tr('{algname} completed. %n feature(s) processed.', n=results['__count']).format(algname=alg.displayName()))
                    feedback.close()
                    # MessageBarProgress handles errors
                    return

                if alg.countVisibleParameters() > 0:
                    dlg = alg.createCustomParametersWidget(parent)

                    if not dlg:
                        dlg = AlgorithmDialog(alg, in_place, iface.mainWindow())
                    canvas = iface.mapCanvas()
                    prevMapTool = canvas.mapTool()
                    dlg.show()
                    dlg.exec_()
                    if canvas.mapTool() != prevMapTool:
                        try:
                            canvas.mapTool().reset()
                        except Exception:
                            pass
                        canvas.setMapTool(prevMapTool)
                else:
                    feedback = MessageBarProgress(algname=alg.displayName())
                    context = dataobjects.createContext(feedback)
                    parameters = {}
                    ret, results = execute(alg, parameters, context, feedback)
                    handleAlgorithmResults(alg, context, feedback)
                    feedback.close()

    def sync_in_place_button_state(self, layer=None):
        """Synchronise the button state with layer state"""

        if layer is None:
            layer = self.iface.activeLayer()

        old_enabled_state = self.editInPlaceAction.isEnabled()

        new_enabled_state = layer is not None and layer.type() == QgsMapLayerType.VectorLayer
        self.editInPlaceAction.setEnabled(new_enabled_state)

        if new_enabled_state != old_enabled_state:
            self.toolbox.set_in_place_edit_mode(new_enabled_state and self.editInPlaceAction.isChecked())

    def openProcessingOptions(self):
        self.iface.showOptionsDialog(self.iface.mainWindow(), currentPage='processingOptions')

    def unload(self):
        self.toolbox.setVisible(False)
        self.iface.removeDockWidget(self.toolbox)
        self.iface.attributesToolBar().removeAction(self.toolboxAction)

        self.resultsDock.setVisible(False)
        self.iface.removeDockWidget(self.resultsDock)

        self.toolbox.deleteLater()
        self.menu.deleteLater()

        # also delete temporary help files
        folder = tempHelpFolder()
        if QDir(folder).exists():
            shutil.rmtree(folder, True)

        self.iface.unregisterMainWindowAction(self.toolboxAction)
        self.iface.unregisterMainWindowAction(self.modelerAction)
        self.iface.unregisterMainWindowAction(self.historyAction)
        self.iface.unregisterMainWindowAction(self.resultsAction)

        self.iface.unregisterOptionsWidgetFactory(self.options_factory)
        self.iface.deregisterLocatorFilter(self.locator_filter)
        self.iface.deregisterLocatorFilter(self.edit_features_locator_filter)
        self.iface.unregisterCustomDropHandler(self.drop_handler)
        QgsApplication.dataItemProviderRegistry().removeProvider(self.item_provider)

        removeButtons()
        removeMenus()

        if self.projectMenuAction is not None:
            self.iface.projectMenu().removeAction(self.projectMenuAction)
            self.projectMenuAction = None
        if self.projectMenuSeparator is not None:
            self.iface.projectMenu().removeAction(self.projectMenuSeparator)
            self.projectMenuSeparator = None

        Processing.deinitialize()

    def openToolbox(self, show):
        self.toolbox.setUserVisible(show)

    def toolboxVisibilityChanged(self, visible):
        self.toolboxAction.setChecked(visible)

    def openModeler(self):
        dlg = ModelerDialog.create()
        dlg.update_model.connect(self.updateModel)
        dlg.show()

    def updateModel(self):
        model_provider = QgsApplication.processingRegistry().providerById('model')
        model_provider.refreshAlgorithms()

    def openResults(self):
        if self.resultsDock.isVisible():
            self.resultsDock.hide()
        else:
            self.resultsDock.show()

    def openHistory(self):
        dlg = HistoryDialog()
        dlg.exec_()

    def tr(self, message, disambiguation=None, n=-1):
        return QCoreApplication.translate('ProcessingPlugin', message, disambiguation=disambiguation, n=n)

    def editSelected(self, enabled):
        self.toolbox.set_in_place_edit_mode(enabled)
