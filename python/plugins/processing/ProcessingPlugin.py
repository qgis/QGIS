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
from builtins import object

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import shutil
import inspect
import os
import sys

from qgis.core import (QgsApplication,
                       QgsProcessingUtils,
                       QgsProcessingModelAlgorithm,
                       QgsDataItemProvider,
                       QgsDataProvider,
                       QgsDataItem,
                       QgsMimeDataUtils)
from qgis.gui import (QgsOptionsWidgetFactory,
                      QgsCustomDropHandler)
from qgis.PyQt.QtCore import Qt, QCoreApplication, QDir, QFileInfo
from qgis.PyQt.QtWidgets import QMenu, QAction
from qgis.PyQt.QtGui import QIcon

from processing.core.Processing import Processing
from processing.gui.AlgorithmDialog import AlgorithmDialog
from processing.gui.ProcessingToolbox import ProcessingToolbox
from processing.gui.HistoryDialog import HistoryDialog
from processing.gui.ConfigDialog import ConfigOptionsPage
from processing.gui.ResultsDock import ResultsDock
from processing.gui.AlgorithmLocatorFilter import AlgorithmLocatorFilter
from processing.modeler.ModelerDialog import ModelerDialog
from processing.tools.system import tempHelpFolder
from processing.gui.menus import removeMenus, initializeMenus, createMenus
from processing.core.ProcessingResults import resultsList

cmd_folder = os.path.split(inspect.getfile(inspect.currentframe()))[0]
if cmd_folder not in sys.path:
    sys.path.insert(0, cmd_folder)


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
        self.runAlg(file)

    @staticmethod
    def runAlg(file):
        alg = QgsProcessingModelAlgorithm()
        if not alg.fromFile(file):
            return False

        alg.setProvider(QgsApplication.processingRegistry().providerById('model'))
        dlg = AlgorithmDialog(alg)
        dlg.setAttribute(Qt.WA_DeleteOnClose)
        dlg.show()
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
        dlg = ModelerDialog()
        dlg.loadModel(self.path())
        dlg.show()

    def actions(self):
        run_model_action = QAction(QCoreApplication.translate('ProcessingPlugin', '&Run Model…'), self)
        run_model_action.triggered.connect(self.runModel)
        edit_model_action = QAction(QCoreApplication.translate('ProcessingPlugin', '&Edit Model…'), self)
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


class ProcessingPlugin(object):

    def __init__(self, iface):
        self.iface = iface
        self.options_factory = ProcessingOptionsFactory()
        self.options_factory.setTitle(self.tr('Processing'))
        iface.registerOptionsWidgetFactory(self.options_factory)
        self.drop_handler = ProcessingDropHandler()
        iface.registerCustomDropHandler(self.drop_handler)
        self.item_provider = ProcessingDataItemProvider()
        QgsApplication.dataItemProviderRegistry().addProvider(self.item_provider)
        self.locator_filter = AlgorithmLocatorFilter()
        iface.registerLocatorFilter(self.locator_filter)
        Processing.initialize()

    def initGui(self):
        self.toolbox = ProcessingToolbox()
        self.iface.addDockWidget(Qt.RightDockWidgetArea, self.toolbox)
        self.toolbox.hide()

        self.resultsDock = ResultsDock()
        self.iface.addDockWidget(Qt.RightDockWidgetArea, self.resultsDock)
        self.resultsDock.hide()

        resultsList.resultAdded.connect(self.resultsDock.fillTree)

        self.menu = QMenu(self.iface.mainWindow().menuBar())
        self.menu.setObjectName('processing')
        self.menu.setTitle(self.tr('Pro&cessing'))

        self.toolboxAction = self.toolbox.toggleViewAction()
        self.toolboxAction.setObjectName('toolboxAction')
        self.toolboxAction.setIcon(
            QgsApplication.getThemeIcon("/processingAlgorithm.svg"))
        self.toolboxAction.setText(self.tr('&Toolbox'))
        self.iface.registerMainWindowAction(self.toolboxAction, 'Ctrl+Alt+T')
        self.menu.addAction(self.toolboxAction)

        self.modelerAction = QAction(
            QgsApplication.getThemeIcon("/processingModel.svg"),
            self.tr('Graphical &Modeler...'), self.iface.mainWindow())
        self.modelerAction.setObjectName('modelerAction')
        self.modelerAction.triggered.connect(self.openModeler)
        self.iface.registerMainWindowAction(self.modelerAction, 'Ctrl+Alt+M')
        self.menu.addAction(self.modelerAction)

        self.historyAction = QAction(
            QIcon(os.path.join(cmd_folder, 'images', 'history.svg')),
            self.tr('&History...'), self.iface.mainWindow())
        self.historyAction.setObjectName('historyAction')
        self.historyAction.triggered.connect(self.openHistory)
        self.iface.registerMainWindowAction(self.historyAction, 'Ctrl+Alt+H')
        self.menu.addAction(self.historyAction)

        self.resultsAction = self.resultsDock.toggleViewAction()
        self.resultsAction.setObjectName('resultsAction')
        self.resultsAction.setIcon(
            QgsApplication.getThemeIcon("/processingResult.svg"))
        self.resultsAction.setText(self.tr('&Results Viewer'))
        self.iface.registerMainWindowAction(self.resultsAction, 'Ctrl+Alt+R')
        self.menu.addAction(self.resultsAction)

        menuBar = self.iface.mainWindow().menuBar()
        menuBar.insertMenu(
            self.iface.firstRightStandardMenu().menuAction(), self.menu)

        self.menu.addSeparator()

        initializeMenus()
        createMenus()

    def unload(self):
        self.toolbox.setVisible(False)
        self.iface.removeDockWidget(self.toolbox)

        self.resultsDock.setVisible(False)
        self.iface.removeDockWidget(self.resultsDock)

        self.menu.deleteLater()

        # delete temporary output files
        folder = QgsProcessingUtils.tempFolder()
        if QDir(folder).exists():
            shutil.rmtree(folder, True)

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
        self.iface.unregisterCustomDropHandler(self.drop_handler)
        QgsApplication.dataItemProviderRegistry().removeProvider(self.item_provider)

        removeMenus()
        Processing.deinitialize()

    def openToolbox(self):
        if self.toolbox.isVisible():
            self.toolbox.hide()
        else:
            self.toolbox.show()

    def openModeler(self):
        dlg = ModelerDialog()
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

    def tr(self, message):
        return QCoreApplication.translate('ProcessingPlugin', message)
