# -*- coding:utf-8 -*-
"""
/***************************************************************************
                            Plugin Installer module
                             -------------------
    Date                 : May 2013
    Copyright            : (C) 2013 by Borys Jurgiel
    Email                : info at borysjurgiel dot pl

    This module is based on former plugin_installer plugin:
      Copyright (C) 2007-2008 Matthew Perry
      Copyright (C) 2008-2013 Borys Jurgiel

 ***************************************************************************/

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
from qgis.core import *
from qgis.utils import iface
from installer_gui import QgsPluginInstallerInstallingDialog, QgsPluginInstallerPluginErrorDialog
from installer_gui import QgsPluginInstallerFetchingDialog, QgsPluginInstallerRepositoryDialog
from installer_gui import removeDir
from installer_data import *
from qgis.utils import startPlugin, unloadPlugin, loadPlugin, reloadPlugin, updateAvailablePlugins


# public instances:
pluginInstaller = None

def initPluginInstaller():
  global pluginInstaller
  pluginInstaller = QgsPluginInstaller()



# -------------------------------------------------------- #
class QgsPluginInstaller(QObject):
  """ The main class for managing the plugin installer stuff"""

  statusLabel = None

  # ----------------------------------------- #
  def __init__(self):
    """ Initialize data objects, starts fetching if appropriate, and warn about/removes obsolete plugins """

    QObject.__init__(self) # initialize QObject in order to to use self.tr()
    repositories.load()
    plugins.getAllInstalled()

    if repositories.checkingOnStart() and repositories.timeForChecking() and repositories.allEnabled():
      # start fetching repositories
      self.statusLabel = QLabel(self.tr("Looking for new plugins...") + " ", iface.mainWindow().statusBar())
      iface.mainWindow().statusBar().insertPermanentWidget(0,self.statusLabel)
      self.statusLabel.linkActivated.connect(self.showPluginManagerWhenReady)
      repositories.checkingDone.connect(self.checkingDone)
      for key in repositories.allEnabled():
        repositories.requestFetching(key)
    else:
      # no fetching at start, so mark all enabled repositories as requesting to be fetched.
      for key in repositories.allEnabled():
        repositories.setRepositoryData(key, "state", 3)

    # look for obsolete plugins (the user-installed one is newer than core one)
    for i in plugins.obsoletePlugins:
      if i == "plugin_installer":
        # uninstall the installer itself
        QMessageBox.warning(iface.mainWindow(), self.tr("QGIS Plugin Installer update"), self.tr("The Plugin Installer has been updated. Please restart QGIS prior to using it"))
        removeDir( QFileInfo(QgsApplication.qgisUserDbFilePath()).path() + "/python/plugins/" + plugins.localCache[i]["id"] )
        return
      else:
        # don't remove brutally other plugins, just inform
        QMessageBox.warning(iface.mainWindow(), self.tr("QGIS Plugin Conflict:")+" "+plugins.localCache[i]["name"], "<b>"+ plugins.localCache[i]["name"] + "</b><br/><br/>" + self.tr("The Plugin Installer has detected an obsolete plugin which masks a newer version shipped with this QGIS version. This is likely due to files associated with a previous installation of QGIS. Please use the Plugin Installer to remove that older plugin in order to unmask the newer version shipped with this copy of QGIS."))


  # ----------------------------------------- #
  def fetchAvailablePlugins(self, reloadMode):
    """ Fetch plugins from all enabled repositories."""
    """  reloadMode = true:  Fully refresh data from QSettings to mRepositories  """
    """  reloadMode = false: Fetch unready repositories only """
    QApplication.setOverrideCursor(Qt.WaitCursor)

    if reloadMode:
      repositories.load()
      plugins.getAllInstalled()

    for key in repositories.allEnabled():
      if reloadMode or repositories.all()[key]["state"] == 3: # if state = 3 (error or not fetched yet), try to fetch once again
        repositories.requestFetching(key)

    if repositories.fetchingInProgress():
      fetchDlg = QgsPluginInstallerFetchingDialog( iface.mainWindow() )
      fetchDlg.exec_()
      del fetchDlg
      for key in repositories.all():
        repositories.killConnection(key)

    QApplication.restoreOverrideCursor()

    # display error messages for every unavailable reposioty, unless Shift pressed nor all repositories are unavailable
    keepQuiet = QgsApplication.keyboardModifiers() == Qt.KeyboardModifiers(Qt.ShiftModifier)
    if repositories.allUnavailable() and repositories.allUnavailable() != repositories.allEnabled():
      for key in repositories.allUnavailable():
        if not keepQuiet:
          QMessageBox.warning(iface.mainWindow(), self.tr("QGIS Python Plugin Installer"), self.tr("Error reading repository:") + " " + key + "\n" + repositories.all()[key]["error"])
        if QgsApplication.keyboardModifiers() == Qt.KeyboardModifiers(Qt.ShiftModifier):
          keepQuiet = True
    # finally, rebuild plugins from the caches
    plugins.rebuild()


  # ----------------------------------------- #
  def checkingDone(self):
    """ Remove the "Looking for new plugins..." label and display a notification instead if any updates or news available """
    if not self.statusLabel:
      # only proceed if the label is present
      return
    # rebuild plugins cache
    plugins.rebuild()
    # look for news in the repositories
    plugins.markNews()
    status = ""
    # first check for news
    for key in plugins.all():
      if plugins.all()[key]["status"] == "new":
        status = self.tr("There is a new plugin available")
        tabIndex = 3 # tab 3 contains new plugins
    # then check for updates (and eventually overwrite status)
    for key in plugins.all():
      if plugins.all()[key]["status"] == "upgradeable":
        status = self.tr("There is a plugin update available")
        tabIndex = 2 # tab 2 contains upgradeable plugins
    # finally set the notify label
    if status:
      self.statusLabel.setText(u' <a href="#%d">%s</a>  ' % (tabIndex,status) )
    else:
      iface.mainWindow().statusBar().removeWidget(self.statusLabel)
      self.statusLabel = None


  # ----------------------------------------- #
  def exportRepositoriesToManager(self):
    """ Update manager's repository tree widget with current data """
    iface.pluginManagerInterface().clearRepositoryList()
    for key in repositories.all():
        url = repositories.all()[key]["url"]
        v=str(QGis.QGIS_VERSION_INT)
        url += "?qgis=%d.%d" % ( int(v[0]), int(v[1:3]) )
        repository = repositories.all()[key]
        iface.pluginManagerInterface().addToRepositoryList({
          "name" : key,
          "url"  : url,
          "enabled" : repositories.all()[key]["enabled"] and "true" or "false",
          "valid" : repositories.all()[key]["valid"] and "true" or "false",
          "state" : str(repositories.all()[key]["state"]),
          "error" : repositories.all()[key]["error"]
        })


  # ----------------------------------------- #
  def exportPluginsToManager(self):
    """ Insert plugins metadata to QgsMetadataRegistry """
    iface.pluginManagerInterface().clearPythonPluginMetadata()
    for key in plugins.all():
      plugin = plugins.all()[key]
      iface.pluginManagerInterface().addPluginMetadata({
        "id" : key,
        "name" : plugin["name"],
        "description" : plugin["description"],
        "category" : plugin["category"],
        "tags" : plugin["tags"],
        "changelog" : plugin["changelog"],
        "author_name" : plugin["author_name"],
        "author_email" : plugin["author_email"],
        "homepage" : plugin["homepage"],
        "tracker" : plugin["tracker"],
        "code_repository" : plugin["code_repository"],
        "version_installed" : plugin["version_installed"],
        "library" : plugin["library"],
        "icon" : plugin["icon"],
        "readonly" : plugin["readonly"] and "true" or "false",
        "installed": plugin["installed"] and "true" or "false",
        "available": plugin["available"] and "true" or "false",
        "status" : plugin["status"],
        "error" : plugin["error"],
        "error_details" : plugin["error_details"],
        "experimental" : plugin["experimental"] and "true" or "false",
        "version_available" : plugin["version_available"],
        "zip_repository" : plugin["zip_repository"],
        "download_url" : plugin["download_url"],
        "filename" : plugin["filename"],
        "downloads" : plugin["downloads"],
        "average_vote" : plugin["average_vote"],
        "rating_votes" : plugin["rating_votes"],
        "pythonic" : "true"
      })
    iface.pluginManagerInterface().reloadModel()


  # ----------------------------------------- #
  def reloadAndExportData(self):
    """ Reload All repositories and export data to the Plugin Manager """
    self.fetchAvailablePlugins(reloadMode = True)
    self.exportRepositoriesToManager()
    self.exportPluginsToManager()


  # ----------------------------------------- #
  def showPluginManagerWhenReady(self, * params):
    """ Open the plugin manager window. If fetching is still in progress, it shows the progress window first """
    """ Optionally pass the index of tab to be opened in params """
    if self.statusLabel:
      iface.mainWindow().statusBar().removeWidget(self.statusLabel)
      self.statusLabel = None

    self.fetchAvailablePlugins(reloadMode = False)
    self.exportRepositoriesToManager()
    self.exportPluginsToManager()

    # finally, show the plugin manager window
    tabIndex = -1
    if len( params ) == 1:
      indx = unicode(params[0])
      if indx.isdigit() and int(indx) > -1 and int(indx) < 7:
        tabIndex = indx
    iface.pluginManagerInterface().showPluginManager( tabIndex )


  # ----------------------------------------- #
  def onManagerClose(self):
    """ Call this method when closing manager window - it resets last-use-dependent values. """
    plugins.updateSeenPluginsList()
    repositories.saveCheckingOnStartLastDate()


  # ----------------------------------------- #
  def exportSettingsGroup(self):
    """ Return QSettings settingsGroup value """
    return settingsGroup


  # ----------------------------------------- #
  def upgradeAllUpgradeable(self):
    """ Reinstall all upgradeable plugins """
    for key in plugins.allUpgradeable():
      self.installPlugin(key, quiet=True)


  # ----------------------------------------- #
  def installPlugin(self, key, quiet=False):
    """ Install given plugin """
    infoString = ('','')
    plugin = plugins.all()[key]
    previousStatus = plugin["status"]
    if not plugin:
      return
    if plugin["status"] == "newer" and not plugin["error"]: # ask for confirmation if user downgrades an usable plugin
      if QMessageBox.warning(iface.mainWindow(), self.tr("QGIS Python Plugin Installer"), self.tr("Are you sure you want to downgrade the plugin to the latest available version? The installed one is newer!"), QMessageBox.Yes, QMessageBox.No) == QMessageBox.No:
        return

    dlg = QgsPluginInstallerInstallingDialog(iface.mainWindow(),plugin)
    dlg.exec_()

    if dlg.result():
      infoString = (self.tr("Plugin installation failed"), dlg.result())
    elif not QDir(QDir.cleanPath(QgsApplication.qgisSettingsDirPath() + "/python/plugins/" + key)).exists():
      infoString = (self.tr("Plugin has disappeared"), self.tr("The plugin seems to have been installed but I don't know where. Probably the plugin package contained a wrong named directory.\nPlease search the list of installed plugins. I'm nearly sure you'll find the plugin there, but I just can't determine which of them it is. It also means that I won't be able to determine if this plugin is installed and inform you about available updates. However the plugin may work. Please contact the plugin author and submit this issue."))
      QApplication.setOverrideCursor(Qt.WaitCursor)
      plugins.getAllInstalled()
      plugins.rebuild()
      self.exportPluginsToManager()
      QApplication.restoreOverrideCursor()
    else:
      QApplication.setOverrideCursor(Qt.WaitCursor)
      # update the list of plugins in plugin handling routines
      updateAvailablePlugins()
      # try to load the plugin
      loadPlugin(plugin["id"])
      plugins.getAllInstalled(testLoad=True)
      plugins.rebuild()
      self.exportPluginsToManager()
      plugin = plugins.all()[key]
      if not plugin["error"]:
        if previousStatus in ["not installed", "new"]:
          infoString = (self.tr("Plugin installed successfully"), self.tr("Plugin installed successfully"))
          settings = QSettings()
          settings.setValue("/PythonPlugins/"+plugin["id"], True)
          startPlugin(plugin["id"])
        else:
          settings = QSettings()
          if settings.value("/PythonPlugins/"+key, False, type=bool): # plugin will be reloaded on the fly only if currently loaded
            infoString = (self.tr("Plugin reinstalled successfully"), self.tr("Plugin reinstalled successfully"))
            reloadPlugin(key)
          else: infoString = (self.tr("Plugin reinstalled successfully"), self.tr("Python plugin reinstalled.\nYou need to restart QGIS in order to reload it."))
        if quiet:
          infoString = (None, None)
        QApplication.restoreOverrideCursor()
      else:
        QApplication.restoreOverrideCursor()
        if plugin["error"] == "incompatible":
          message = self.tr("The plugin is not compatible with this version of QGIS. It's designed for QGIS versions:")
          message += " <b>" + plugin["error_details"] + "</b>"
        elif plugin["error"] == "dependent":
          message = self.tr("The plugin depends on some components missing on your system. You need to install the following Python module in order to enable it:")
          message += "<b> " + plugin["error_details"] + "</b>"
        else:
          message = self.tr("The plugin is broken. Python said:")
          message += "<br><b>" + plugin["error_details"] + "</b>"
        dlg = QgsPluginInstallerPluginErrorDialog(iface.mainWindow(), message)
        dlg.exec_()
        if dlg.result():
          # revert installation
          plugins.getAllInstalled()
          plugins.rebuild()
          self.exportPluginsToManager()
          pluginDir = QFileInfo(QgsApplication.qgisUserDbFilePath()).path() + "/python/plugins/" + plugin["id"]
          removeDir(pluginDir)
          if QDir(pluginDir).exists():
            infoString = (self.tr("Plugin uninstall failed"), result)
            try:
              exec ("sys.path_importer_cache.clear()")
              exec ("import %s" % plugin["id"])
              exec ("reload (%s)" % plugin["id"])
            except:
              pass
          else:
            try:
              exec ("del sys.modules[%s]" % plugin["id"])
            except:
              pass
          plugins.getAllInstalled()
          plugins.rebuild()
          self.exportPluginsToManager()

    if infoString[0]:
      QMessageBox.information(iface.mainWindow(), infoString[0], infoString[1])


  # ----------------------------------------- #
  def uninstallPlugin(self,key):
    """ Uninstall given plugin """
    plugin = plugins.all()[key]
    if not plugin:
      return
    warning = self.tr("Are you sure you want to uninstall the following plugin?") + "\n(" + plugin["name"] + ")"
    if plugin["status"] == "orphan" and not plugin["error"]:
      warning += "\n\n"+self.tr("Warning: this plugin isn't available in any accessible repository!")
    if QMessageBox.warning(iface.mainWindow(), self.tr("QGIS Python Plugin Installer"), warning , QMessageBox.Yes, QMessageBox.No) == QMessageBox.No:
      return
    # unload the plugin if it's not plugin_installer itself (otherwise, do it after removing its directory):
    if key != "plugin_installer":
      try:
        unloadPlugin(key)
      except:
        pass
    pluginDir = QFileInfo(QgsApplication.qgisUserDbFilePath()).path() + "/python/plugins/" + plugin["id"]
    result = removeDir(pluginDir)
    if result:
      QMessageBox.warning(iface.mainWindow(), self.tr("Plugin uninstall failed"), result)
    else:
      # if the uninstalled plugin is the installer itself, reload it and quit
      if key == "plugin_installer":
        try:
          QMessageBox.information(iface.mainWindow(), self.tr("QGIS Python Plugin Installer"), self.tr("Plugin Installer update uninstalled. Plugin Installer will now close and revert to its primary version. You can find it in the Plugins menu and continue operation."))
          reloadPlugin(key)
          return
        except:
          pass
      # safe remove
      try:
        unloadPlugin(plugin["id"])
      except:
        pass
      try:
        exec ("plugins[%s].unload()" % plugin["id"])
        exec ("del plugins[%s]" % plugin["id"])
      except:
        pass
      try:
        exec ("del sys.modules[%s]" % plugin["id"])
      except:
        pass
      plugins.getAllInstalled()
      plugins.rebuild()
      self.exportPluginsToManager()
      QMessageBox.information(iface.mainWindow(), self.tr("Plugin uninstalled successfully"), self.tr("Plugin uninstalled successfully"))


  # ----------------------------------------- #
  def addRepository(self):
    """ add new repository connection """
    dlg = QgsPluginInstallerRepositoryDialog( iface.mainWindow() )
    dlg.checkBoxEnabled.setCheckState(Qt.Checked)
    if not dlg.exec_():
      return
    for i in repositories.all().values():
      if dlg.editURL.text().strip() == i["url"]:
        QMessageBox.warning(iface.mainWindow(), self.tr("QGIS Python Plugin Installer"), self.tr("Unable to add another repository with the same URL!"))
        return
    settings = QSettings()
    settings.beginGroup(reposGroup)
    reposName = dlg.editName.text()
    reposURL = dlg.editURL.text().strip()
    if repositories.all().has_key(reposName):
      reposName = reposName + "(2)"
    # add to settings
    settings.setValue(reposName+"/url", reposURL )
    settings.setValue(reposName+"/enabled", bool(dlg.checkBoxEnabled.checkState()))
    # refresh lists and populate widgets
    plugins.removeRepository(reposName)
    self.reloadAndExportData()


  # ----------------------------------------- #
  def editRepository(self, reposName):
    """ edit repository connection """
    if not reposName:
      return
    checkState={False:Qt.Unchecked,True:Qt.Checked}
    dlg = QgsPluginInstallerRepositoryDialog( iface.mainWindow() )
    dlg.editName.setText(reposName)
    dlg.editURL.setText(repositories.all()[reposName]["url"])
    dlg.checkBoxEnabled.setCheckState(checkState[repositories.all()[reposName]["enabled"]])
    if repositories.all()[reposName]["valid"]:
      dlg.checkBoxEnabled.setEnabled(True)
      dlg.labelInfo.setText("")
    else:
      dlg.checkBoxEnabled.setEnabled(False)
      dlg.labelInfo.setText(self.tr("This repository is blocked due to incompatibility with your QGIS version"))
      dlg.labelInfo.setFrameShape(QFrame.Box)
    if not dlg.exec_():
      return # nothing to do if cancelled
    for i in repositories.all().values():
      if dlg.editURL.text().strip() == i["url"] and dlg.editURL.text().strip() != repositories.all()[reposName]["url"]:
        QMessageBox.warning(iface.mainWindow(), self.tr("QGIS Python Plugin Installer"), self.tr("Unable to add another repository with the same URL!"))
        return
    # delete old repo from QSettings and create new one
    settings = QSettings()
    settings.beginGroup(reposGroup)
    settings.remove(reposName)
    newName = dlg.editName.text()
    if repositories.all().has_key(newName) and newName != reposName:
      newName = newName + "(2)"
    settings.setValue(newName+"/url", dlg.editURL.text().strip())
    settings.setValue(newName+"/enabled", bool(dlg.checkBoxEnabled.checkState()))
    if dlg.editURL.text().strip() == repositories.all()[reposName]["url"] and dlg.checkBoxEnabled.checkState() == checkState[repositories.all()[reposName]["enabled"]]:
      repositories.rename(reposName, newName)
      self.exportRepositoriesToManager()
      return # nothing else to do if only repository name was changed
    plugins.removeRepository(reposName)
    self.reloadAndExportData()


  # ----------------------------------------- #
  def deleteRepository(self, reposName):
    """ delete repository connection """
    if not reposName:
      return
    warning = self.tr("Are you sure you want to remove the following repository?") + "\n" + reposName
    if QMessageBox.warning(iface.mainWindow(), self.tr("QGIS Python Plugin Installer"), warning , QMessageBox.Yes, QMessageBox.No) == QMessageBox.No:
      return
    # delete from the settings, refresh data and repopulate all the widgets
    settings = QSettings()
    settings.beginGroup(reposGroup)
    settings.remove(reposName)
    repositories.remove(reposName)
    plugins.removeRepository(reposName)
    self.reloadAndExportData()
