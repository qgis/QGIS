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

from qgis.core import QgsApplication
from qgis.gui import QgsOptionsWidgetFactory
from qgis.PyQt.QtCore import Qt, QCoreApplication, QDir
from qgis.PyQt.QtWidgets import QMenu, QAction
from qgis.PyQt.QtGui import QIcon

from processing.core.Processing import Processing
from processing.gui.ProcessingToolbox import ProcessingToolbox
from processing.gui.HistoryDialog import HistoryDialog
from processing.gui.ConfigDialog import ConfigOptionsPage
from processing.gui.ResultsDock import ResultsDock
from processing.gui.CommanderWindow import CommanderWindow
from processing.modeler.ModelerDialog import ModelerDialog
from processing.tools.system import tempFolder
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


class ProcessingPlugin(object):

    def __init__(self, iface):
        self.iface = iface
        self.options_factory = ProcessingOptionsFactory()
        self.options_factory.setTitle(self.tr('Processing'))
        iface.registerOptionsWidgetFactory(self.options_factory)
        Processing.initialize()

    def initGui(self):
        self.commander = None
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

        self.commanderAction = QAction(
            QIcon(os.path.join(cmd_folder, 'images', 'commander.svg')),
            self.tr('&Commander'), self.iface.mainWindow())
        self.commanderAction.setObjectName('commanderAction')
        self.commanderAction.triggered.connect(self.openCommander)
        self.menu.addAction(self.commanderAction)
        self.iface.registerMainWindowAction(self.commanderAction,
                                            self.tr('Ctrl+Alt+D'))

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
        folder = tempFolder()
        if QDir(folder).exists():
            shutil.rmtree(folder, True)

        self.iface.unregisterMainWindowAction(self.toolboxAction)
        self.iface.unregisterMainWindowAction(self.modelerAction)
        self.iface.unregisterMainWindowAction(self.historyAction)
        self.iface.unregisterMainWindowAction(self.resultsAction)
        self.iface.unregisterMainWindowAction(self.commanderAction)

        self.iface.unregisterOptionsWidgetFactory(self.options_factory)

        removeMenus()

    def openCommander(self):
        if self.commander is None:
            self.commander = CommanderWindow(
                self.iface.mainWindow(),
                self.iface.mapCanvas())
        self.commander.prepareGui()
        self.commander.show()

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
