from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *


import os, sys
import inspect
from sextante.core.Sextante import Sextante
from sextante.gui.SextanteToolbox import SextanteToolbox
from sextante.core.QGisLayers import QGisLayers
from sextante.gui.HistoryDialog import HistoryDialog

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

        menuBar = self.iface.mainWindow().menuBar()
        menuBar.insertMenu(menuBar.actions()[-1], self.menu)

    def unload(self):
        self.toolbox.setVisible(False)
        self.menu.deleteLater()

    def openToolbox(self):
        self.toolbox.setVisible(True)

    def openModeler(self):
        pass

    def openHistory(self):
        dlg = HistoryDialog()
        dlg.exec_()

