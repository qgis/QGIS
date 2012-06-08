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
from sextante.modeler.ModelerDialog import ModelerDialog
from sextante.gui.ResultsDialog import ResultsDialog
from sextante.about.AboutDialog import AboutDialog
import subprocess
    
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
        self.menu.setTitle("Analysis")

        icon = QIcon(os.path.dirname(__file__) + "/images/toolbox.png")
        self.toolboxAction = QAction(icon, \
            "&SEXTANTE Toolbox", self.iface.mainWindow())
        QObject.connect(self.toolboxAction, SIGNAL("triggered()"), self.openToolbox)
        self.menu.addAction(self.toolboxAction)

        icon = QIcon(os.path.dirname(__file__) + "/images/model.png")
        self.modelerAction = QAction(icon, \
            "&SEXTANTE Modeler", self.iface.mainWindow())
        QObject.connect(self.modelerAction, SIGNAL("triggered()"), self.openModeler)
        self.menu.addAction(self.modelerAction)

        icon = QIcon(os.path.dirname(__file__) + "/images/history.gif")
        self.historyAction = QAction(icon, \
            "&SEXTANTE History and log", self.iface.mainWindow())
        QObject.connect(self.historyAction, SIGNAL("triggered()"), self.openHistory)
        self.menu.addAction(self.historyAction)

        icon = QIcon(os.path.dirname(__file__) + "/images/config.png")
        self.configAction = QAction(icon, \
            "&SEXTANTE options and configuration", self.iface.mainWindow())
        QObject.connect(self.configAction, SIGNAL("triggered()"), self.openConfig)
        self.menu.addAction(self.configAction)

        icon = QIcon(os.path.dirname(__file__) + "/images/results.png")
        self.resultsAction = QAction(icon, \
            "&SEXTANTE results viewer", self.iface.mainWindow())
        QObject.connect(self.resultsAction, SIGNAL("triggered()"), self.openResults)
        self.menu.addAction(self.resultsAction)

        icon = QIcon(os.path.dirname(__file__) + "/images/help.png")
        self.helpAction = QAction(icon, \
            "&SEXTANTE help", self.iface.mainWindow())
        QObject.connect(self.helpAction, SIGNAL("triggered()"), self.openHelp)
        self.menu.addAction(self.helpAction)

        icon = QIcon(os.path.dirname(__file__) + "/images/info.png")
        self.aboutAction = QAction(icon, \
            "&About SEXTANTE", self.iface.mainWindow())
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
        dlg = AboutDialog()
        dlg.exec_()

    def openHelp(self):
        filename = os.path.dirname(__file__) + "/help/index.html"
        if os.name == "nt":
            os.startfile(filename)
        elif sys.platform == "darwin":
            subprocess.Popen(('open', filename))
        else:
            subprocess.call(('xdg-open', filename))


