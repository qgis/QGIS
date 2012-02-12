from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *


import os, sys
import inspect
from sextante.core.Sextante import Sextante
from sextante.gui.SextanteToolbox import SextanteToolbox
from sextante.core.QGisLayers import QGisLayers
from sextante.gui.HistoryDialog import HistoryDialog
from sextante.core.SextanteUtils import SextanteUtils
from sextante.gui.ConfigDialog import ConfigDialog
from sextante.gui.ModelerDialog import ModelerDialog

cmd_folder = os.path.split(inspect.getfile( inspect.currentframe() ))[0]
if cmd_folder not in sys.path:
    sys.path.insert(0, cmd_folder)

class SextantePlugin:

    def __init__(self, iface):
        self.iface = iface
        QGisLayers.setInterface(iface)
        Sextante.initialize()

    def initGui(self):
        self.toolbox = SextanteToolbox(self.iface)
        self.toolbox.setVisible(False)

        self.menu = QMenu()
        self.menu.setTitle("SEXTANTE")

        icon = QIcon(os.path.dirname(__file__) + "/toolbox.png")
        self.toolboxAction = QAction(icon, \
            "&SEXTANTE Toolbox", self.iface.mainWindow())
        QObject.connect(self.toolboxAction, SIGNAL("triggered()"), self.openToolbox)
        self.menu.addAction(self.toolboxAction)

        icon = QIcon(os.path.dirname(__file__) + "/model.png")
        self.modelerAction = QAction(icon, \
            "&SEXTANTE Modeler", self.iface.mainWindow())
        QObject.connect(self.modelerAction, SIGNAL("triggered()"), self.openModeler)
        self.menu.addAction(self.modelerAction)

        icon = QIcon(os.path.dirname(__file__) + "/history.gif")
        self.historyAction = QAction(icon, \
            "&SEXTANTE History and log", self.iface.mainWindow())
        QObject.connect(self.historyAction, SIGNAL("triggered()"), self.openHistory)
        self.menu.addAction(self.historyAction)

        icon = QIcon(os.path.dirname(__file__) + "/config.png")
        self.configAction = QAction(icon, \
            "&SEXTANTE options and configuration", self.iface.mainWindow())
        QObject.connect(self.configAction, SIGNAL("triggered()"), self.openConfig)
        self.menu.addAction(self.configAction)


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


    def openHistory(self):
        dlg = HistoryDialog()
        dlg.exec_()

    def openConfig(self):
        dlg = ConfigDialog(self.toolbox)
        dlg.exec_()


