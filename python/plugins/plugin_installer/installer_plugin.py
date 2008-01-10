from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from dialog import InstallerPluginGui
import resources

class InstallerPlugin:

  # ----------------------------------------- #
  def __init__(self, iface):
    # save reference to the QGIS interface
    self.iface = iface

  # ----------------------------------------- #
  def initGui(self):
    # create action that will start plugin configuration
    self.action = QAction(QIcon(":/plugins/installer/icon.xpm"), "Install plugins", self.iface.getMainWindow())
    self.action.setWhatsThis("Plugin Installer")
    QObject.connect(self.action, SIGNAL("activated()"), self.run)

    # add toolbar button and menu item
    self.iface.addToolBarIcon(self.action)
    self.iface.addPluginMenu("&Plugin Installer", self.action)

  # ----------------------------------------- #
  def unload(self):
    # remove the plugin menu item and icon
    self.iface.removePluginMenu("&Plugin Installer",self.action)
    self.iface.removeToolBarIcon(self.action)

  # ----------------------------------------- #
  def run(self):
    # create and show a configuration dialog 
    flags = Qt.WindowTitleHint | Qt.WindowSystemMenuHint | Qt.WindowMaximizeButtonHint 
    self.gui = InstallerPluginGui(self.iface.getMainWindow(),flags)
    self.gui.show()
