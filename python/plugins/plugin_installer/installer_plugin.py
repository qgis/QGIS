from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from dialog import InstallerPluginGui
import resources
import os
import tempfile

class InstallerPlugin:

  # ----------------------------------------- #
  def __init__(self, iface):
    # save reference to the QGIS interface
    self.iface = iface
    self.repository = "http://spatialserver.net/cgi-bin/pyqgis_plugin.rb"

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
    gui = InstallerPluginGui(self.iface.getMainWindow(),flags)

    # connect to the submitAddresses signal which triggers the geocoding engine
    QObject.connect(gui, SIGNAL("retrieveList(QString )"), self.getAvailablePlugins)

    # connect to the submitOutput signal which sets the output text file
    QObject.connect(gui, SIGNAL("installPlugin(QString )"), self.installPlugin)

    # grab the click on the treelist
    QObject.connect(gui.treePlugins, SIGNAL("itemClicked(QTreeWidgetItem *,int)"), self.treeClicked)

    gui.show()
    self.gui = gui
   
 
  def getAvailablePlugins(self):
    print "getting list of plugins"
    from qgis_plugins import retrieve_list
    QApplication.setOverrideCursor(Qt.WaitCursor)
    pluginlist = retrieve_list(self.repository)
    output = "QGIS python plugins avialable from \n%s\n" % self.repository
    #for p in pluginlist:
    #    output += "\n%s ( version %s )" % (p["name"], p["version"])
    #    output += "\n\t%s by %s" % (p["desc"],p["author"])
    #self.gui.txtAvailable.setText(output)   
    for p in pluginlist:
    	a = QTreeWidgetItem(self.gui.treePlugins)
    	a.setText(0,p["name"])
    	a.setText(1,p["version"])
    	a.setText(2,p["desc"])
    	a.setText(3,p["author"])
    
    QApplication.restoreOverrideCursor()


    # resize the columns
    # plugin name
    self.gui.treePlugins.resizeColumnToContents(0);
    # version
    self.gui.treePlugins.resizeColumnToContents(1);
    # author/contributor
    self.gui.treePlugins.resizeColumnToContents(3);
    # description
    self.gui.treePlugins.setColumnWidth(2, 560);
    return

  def installPlugin(self, plugin):
    QApplication.setOverrideCursor(Qt.WaitCursor)
    from qgis_plugins import retrieve_list, install_plugin
    plugindir = str(QgsApplication.qgisSettingsDirPath()) + "/python/plugins"
    result = install_plugin(plugin, plugindir, self.repository)
    QApplication.restoreOverrideCursor()
    if result[0]:
        mb=QMessageBox(self.iface.getMainWindow())
        mb.information(mb, "Plugin installed successfully", result[1])
    else:
        mb=QMessageBox(self.iface.getMainWindow())
        mb.information(mb, "Plugin installation failed", result[1])
    return

  def treeClicked(self, item, col):
    self.gui.linePlugin.setText(item.text(0))
    return
