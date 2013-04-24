# -*- coding: utf-8 -*-

"""
***************************************************************************
    SextantePlugin.py
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

import shutil
import os, sys
import inspect
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from sextante.core.Sextante import Sextante
from sextante.core.QGisLayers import QGisLayers
from sextante.core.SextanteUtils import SextanteUtils
from sextante.gui.SextanteToolbox import SextanteToolbox
from sextante.gui.HistoryDialog import HistoryDialog
from sextante.gui.ConfigDialog import ConfigDialog
from sextante.gui.ResultsDialog import ResultsDialog
from sextante.modeler.ModelerDialog import ModelerDialog
import sextante.resources_rc

cmd_folder = os.path.split(inspect.getfile( inspect.currentframe() ))[0]
if cmd_folder not in sys.path:
    sys.path.insert(0, cmd_folder)

class SextantePlugin:

    def __init__(self, iface):
        self.iface = iface
        QGisLayers.setInterface(iface)
        Sextante.initialize()
        Sextante.setInterface(iface)

    def initGui(self):
        self.toolbox = SextanteToolbox(self.iface)
        self.iface.addDockWidget(Qt.RightDockWidgetArea, self.toolbox)
        self.toolbox.hide()
        Sextante.addAlgListListener(self.toolbox)

        self.menu = QMenu(self.iface.mainWindow())
        self.menu.setTitle(QCoreApplication.translate("SEXTANTE", "Analysis"))

        self.toolboxAction = self.toolbox.toggleViewAction()
        self.toolboxAction.setIcon(QIcon(":/sextante/images/toolbox.png"))
        self.toolboxAction.setText(QCoreApplication.translate("SEXTANTE", "&SEXTANTE toolbox"))
        self.menu.addAction(self.toolboxAction)

        self.modelerAction = QAction(QIcon(":/sextante/images/model.png"),
            QCoreApplication.translate("SEXTANTE", "&SEXTANTE modeler"),
            self.iface.mainWindow())
        self.modelerAction.triggered.connect(self.openModeler)
        self.menu.addAction(self.modelerAction)

        self.historyAction = QAction(QIcon(":/sextante/images/history.gif"),
            QCoreApplication.translate("SEXTANTE", "&SEXTANTE history and log"),
            self.iface.mainWindow())
        self.historyAction.triggered.connect(self.openHistory)
        self.menu.addAction(self.historyAction)

        self.configAction = QAction(QIcon(":/sextante/images/config.png"),
            QCoreApplication.translate("SEXTANTE", "&SEXTANTE options and configuration"),
            self.iface.mainWindow())
        self.configAction.triggered.connect(self.openConfig)
        self.menu.addAction(self.configAction)

        self.resultsAction = QAction(QIcon(":/sextante/images/results.png"),
            QCoreApplication.translate("SEXTANTE", "&SEXTANTE results viewer"),
            self.iface.mainWindow())
        self.resultsAction.triggered.connect(self.openResults)
        self.menu.addAction(self.resultsAction)

        menuBar = self.iface.mainWindow().menuBar()
        menuBar.insertMenu(menuBar.actions()[-1], self.menu)

    def unload(self):
        self.toolbox.setVisible(False)
        self.menu.deleteLater()
        #delete temporary output files
        folder = SextanteUtils.tempFolder()
        if QDir(folder).exists():
            shutil.rmtree(folder, True)

    def openToolbox(self):
        if self.toolbox.isVisible():
            self.toolbox.hide()
        else:
            self.toolbox.show()

    def openModeler(self):
        dlg = ModelerDialog()
        dlg.exec_()
        if dlg.update:
            self.toolbox.updateTree()

    def openResults(self):
        dlg = ResultsDialog()
        dlg.exec_()

    def openHistory(self):
        dlg = HistoryDialog()
        dlg.exec_()

    def openConfig(self):
        dlg = ConfigDialog(self.toolbox)
        dlg.exec_()

