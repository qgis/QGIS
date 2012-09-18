import os, sys
import inspect
import subprocess

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

from sextante.about.AboutDialog import AboutDialog

import resources_rc

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
        self.toolbox.setVisible(False)
        Sextante.addAlgListListener(self.toolbox)

        self.menu = QMenu(self.iface.mainWindow())
        self.menu.setTitle(QCoreApplication.translate("SEXTANTE", "Analysis"))

        self.toolboxAction = QAction(QIcon(":/sextante/images/toolbox.png"),
            QCoreApplication.translate("SEXTANTE", "&SEXTANTE Toolbox"),
            self.iface.mainWindow())
        QObject.connect(self.toolboxAction, SIGNAL("triggered()"), self.openToolbox)
        self.menu.addAction(self.toolboxAction)

        self.modelerAction = QAction(QIcon(":/sextante/images/model.png"),
            QCoreApplication.translate("SEXTANTE", "&SEXTANTE Modeler"),
            self.iface.mainWindow())
        QObject.connect(self.modelerAction, SIGNAL("triggered()"), self.openModeler)
        self.menu.addAction(self.modelerAction)

        self.historyAction = QAction(QIcon(":/sextante/images/history.gif"),
            QCoreApplication.translate("SEXTANTE", "&SEXTANTE History and log"),
            self.iface.mainWindow())
        QObject.connect(self.historyAction, SIGNAL("triggered()"), self.openHistory)
        self.menu.addAction(self.historyAction)

        self.configAction = QAction(QIcon(":/sextante/images/config.png"),
            QCoreApplication.translate("SEXTANTE", "&SEXTANTE options and configuration"),
            self.iface.mainWindow())
        QObject.connect(self.configAction, SIGNAL("triggered()"), self.openConfig)
        self.menu.addAction(self.configAction)

        self.resultsAction = QAction(QIcon(":/sextante/images/results.png"),
            QCoreApplication.translate("SEXTANTE", "&SEXTANTE results viewer"),
            self.iface.mainWindow())
        QObject.connect(self.resultsAction, SIGNAL("triggered()"), self.openResults)
        self.menu.addAction(self.resultsAction)

        self.helpAction = QAction(QIcon(":/sextante/images/help.png"),
            QCoreApplication.translate("SEXTANTE", "&SEXTANTE help"),
            self.iface.mainWindow())
        QObject.connect(self.helpAction, SIGNAL("triggered()"), self.openHelp)
        self.menu.addAction(self.helpAction)

        self.aboutAction = QAction(QIcon(":/sextante/images/info.png"),
            QCoreApplication.translate("SEXTANTE", "&About SEXTANTE"),
            self.iface.mainWindow())
        QObject.connect(self.aboutAction, SIGNAL("triggered()"), self.openAbout)
        self.menu.addAction(self.aboutAction)

        menuBar = self.iface.mainWindow().menuBar()
        menuBar.insertMenu(menuBar.actions()[-1], self.menu)

    def unload(self):
        self.toolbox.setVisible(False)
        self.menu.deleteLater()
        #delete temporary output files
        folder = SextanteUtils.tempFolder()
        for f in os.listdir(folder):
            path = os.path.join(folder,f)
            try:
                os.unlink(path)
            except:
                #leave files that could not be deleted
                pass

    def openToolbox(self):
        self.toolbox.setVisible(True)

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

    def openAbout(self):
        dlg = AboutDialog(self)
        dlg.exec_()

    def openHelp(self):
        filename = os.path.dirname(__file__) + "/help/index.html"
        if os.name == "nt":
            os.startfile(filename)
        elif sys.platform == "darwin":
            subprocess.Popen(('open', filename))
        else:
            subprocess.call(('xdg-open', filename))
