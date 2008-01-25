"""
Copyright (C) 2008 Matthew Perry
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
"""
from PyQt4.QtCore import *
from PyQt4.QtGui import *

from qgis.core import QgsApplication, QgsContextHelp

from gui import Ui_Dialog
import resources

from repository_dialog import RepositoryDialog


class InstallerPluginGui(QDialog, Ui_Dialog):
  def __init__(self, parent, fl):
    QDialog.__init__(self, parent, fl)
    
    self.default_repository_name = "Official QGIS repository"
    self.default_repository = "http://spatialserver.net/cgi-bin/pyqgis_plugin.rb"
    
    self.setupUi(self)
    
    self.connect(self.buttonBrowse, SIGNAL("clicked()"), self.getAvailablePlugins)
    self.connect(self.pbnOK, SIGNAL("clicked()"), self.installPlugin)

    # grab the click on the treelist
    self.connect(self.treePlugins, SIGNAL("itemClicked(QTreeWidgetItem *,int)"), self.treeClicked)

    # repositories handling
    self.connect(self.buttonAddRep, SIGNAL("clicked()"), self.addRepository)
    self.connect(self.buttonEditRep, SIGNAL("clicked()"), self.editRepository)
    self.connect(self.buttonDeleteRep, SIGNAL("clicked()"), self.deleteRepository)
    
    self.populateRepositories()
    
  
  def on_pbnCancel_clicked(self):
    self.close()
    
  
  def getAvailablePlugins(self):
    print "getting list of plugins"
    repository = self.getRepository()
    if not repository:
      return
    repository_url = str(repository[1])
    
    from qgis_plugins import retrieve_list
    QApplication.setOverrideCursor(Qt.WaitCursor)

    try:
      pluginlist = retrieve_list(repository_url)
    except IOError:
      QApplication.restoreOverrideCursor()
      QMessageBox.warning(self, "Error", "Couldn't connect to the repository.")
      return
    except Exception:
      QApplication.restoreOverrideCursor()
      QMessageBox.warning(self, "Error", "Couldn't parse output from repository.")
      return
    
    #output = "QGIS python plugins avialable from \n%s\n" % self.repository
    #for p in pluginlist:
    #    output += "\n%s ( version %s )" % (p["name"], p["version"])
    #    output += "\n\t%s by %s" % (p["desc"],p["author"])
    #self.gui.txtAvailable.setText(output)   
    self.treePlugins.clear()
    for p in pluginlist:
      a = QTreeWidgetItem(self.treePlugins)
      a.setText(0,p["name"])
      a.setText(1,p["version"])
      a.setText(2,p["desc"])
      a.setText(3,p["author"])
      a.setToolTip(2, p["desc"])
    
    QApplication.restoreOverrideCursor()

    # resize the columns
    # plugin name
    self.treePlugins.resizeColumnToContents(0);
    # version
    self.treePlugins.resizeColumnToContents(1);
    # author/contributor
    self.treePlugins.resizeColumnToContents(3);
    # description
    self.treePlugins.setColumnWidth(2, 560);

  
  def installPlugin(self):
    """ installs currently selected plugin """
    plugin = self.linePlugin.text()
    repository = self.getRepository()
    if not repository:
      return
    repository_url = str(repository[1])
    plugindir = str(QgsApplication.qgisSettingsDirPath()) + "/python/plugins"
    
    QApplication.setOverrideCursor(Qt.WaitCursor)
    from qgis_plugins import retrieve_list, install_plugin
    print "install_plugin",plugin,plugindir,repository_url
    result = install_plugin(plugin, plugindir, repository_url)
    QApplication.restoreOverrideCursor()
    
    if result[0]:
        QMessageBox.information(self, "Plugin installed successfully", result[1])
    else:
        QMessageBox.warning(self, "Plugin installation failed", result[1])

  
  def treeClicked(self, item, col):
    self.linePlugin.setText(item.text(0))
    
    
  def getRepository(self):
    """ returns Name and URL of the current repository as a tuple or None if no repository is selected """
    if self.comboRepositories.currentIndex() == -1:
      return None
    
    settings = QSettings()
    reposGroup = "/Qgis/plugin-repos"
    reposName = self.comboRepositories.currentText()
    reposURL = settings.value(reposGroup+"/"+reposName+"/url", QVariant()).toString()
    return (reposName, reposURL)


  def populateRepositories(self):
    """ populate repository combo box from the settings """
    self.comboRepositories.clear()
    
    settings = QSettings()
    reposGroup = "/Qgis/plugin-repos"
    settings.beginGroup(reposGroup)
    
    # add the default repository when there isn't any...
    if len(settings.childGroups()) == 0:
      settings.setValue(self.default_repository_name+"/url", QVariant(self.default_repository))
    
    for key in settings.childGroups():
      self.comboRepositories.addItem(key)
      
    settings.endGroup()
    
    
  def addRepository(self):
    """ add repository button has been clicked """
    print "add"
    dlg = RepositoryDialog(self)
    if not dlg.exec_():
      return
    
    settings = QSettings()
    reposGroup = "/Qgis/plugin-repos"
    settings.beginGroup(reposGroup)
    
    reposName = dlg.editName.text()
    reposURL = dlg.editURL.text()
    print "name: "+reposName
    print "url: "+reposURL
    
    # add to settings
    settings.setValue(reposName+"/url", QVariant(reposURL))
    
    # add to combobox
    self.comboRepositories.addItem(reposName)
    
    
  def editRepository(self):
    """ edit repository button has been clicked """
    print "edit"
    
    current = self.comboRepositories.currentIndex()
    if current == -1:
      return
    
    (reposName, reposURL) = self.getRepository()
    
    dlg = RepositoryDialog(self)
    dlg.editName.setText(reposName)
    dlg.editURL.setText(reposURL)
    if not dlg.exec_():
      return
    
    settings = QSettings()
    reposGroup = "/Qgis/plugin-repos"
    settings.beginGroup(reposGroup)
    
    # first delete old setting
    settings.remove(reposName)
    
    # and create new one
    settings.setValue(dlg.editName.text()+"/url", QVariant(dlg.editURL.text()))
    
    # update the name if it has been changed
    self.comboRepositories.setItemText(current, dlg.editName.text())
    
    
  def deleteRepository(self):
    """ delete repository button has been clicked """
    print "delete"
    
    current = self.comboRepositories.currentIndex()
    if current == -1:
      return
    
    settings = QSettings()
    reposGroup = "/Qgis/plugin-repos"
    settings.beginGroup(reposGroup)

    # delete from settings
    reposName = self.comboRepositories.currentText()
    settings.remove(reposName)
    
    # delete from combo box
    self.comboRepositories.removeItem(current)
