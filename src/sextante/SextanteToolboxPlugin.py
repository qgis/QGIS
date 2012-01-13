from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
# Initialize Qt resources from file resources.py
import resources
# Import the code for the dialog


import os, sys
import inspect
from sextante.core.Sextante import Sextante
from sextante.gui.SextanteToolbox import SextanteToolbox
from sextante.core.QGisLayers import QGisLayers

cmd_folder = os.path.split(inspect.getfile( inspect.currentframe() ))[0]
if cmd_folder not in sys.path:
    sys.path.insert(0, cmd_folder)

class SextanteToolboxPlugin:

    def __init__(self, iface):
        self.iface = iface
        QGisLayers.setInterface(iface)
        Sextante.initialize()

    def initGui(self):
        self.toolbox = SextanteToolbox(self.iface)
        self.toolbox.setVisible(False)
        self.toolboxAction = QAction(QIcon(":/plugins/sextante/toolbox.png"), \
            "SEXTANTE Toolbox", self.iface.mainWindow())
        QObject.connect(self.toolboxAction, SIGNAL("triggered()"), self.openToolbox)
        self.iface.addPluginToMenu("&SEXTANTE", self.toolboxAction)

        self.modelerAction = QAction(QIcon(":/plugins/sextante/modeler.png"), \
            "SEXTANTE Modeler", self.iface.mainWindow())
        QObject.connect(self.modelerAction, SIGNAL("triggered()"), self.openModeler)
        self.iface.addPluginToMenu("&SEXTANTE", self.modelerAction)

    def unload(self):
        # Remove the plugin menu items
        self.iface.removePluginMenu("&SEXTANTE",self.toolboxAction)
        self.iface.removePluginMenu("&SEXTANTE",self.modelerAction)

    def openToolbox(self):

        #dlg = SextanteToolbox(self.iface)
        self.toolbox.setVisible(True)
        #dlg.exec_()

    def openModeler(self):
        pass
