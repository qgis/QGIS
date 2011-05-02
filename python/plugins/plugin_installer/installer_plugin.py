# -*- coding: utf-8 -*-
"""
Copyright (C) 2007-2008 Matthew Perry
Copyright (C) 2008-2010 Borys Jurgiel

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
from installer_gui import *
from installer_data import *
import resources_rc


class InstallerPlugin():
  # ----------------------------------------- #
  def __init__(self, iface):
    self.iface = iface
    setIface(self.iface) #pass self.iface to installer_data module (needed for plugin loading & testing)
    self.mainWindow = self.iface.mainWindow
    self.guiDlg = None


  # ----------------------------------------- #
  def setCurrentTheme(self, theThemeName):
    """ Set icons to the current theme """
    self.action.setIcon(self.getThemeIcon("plugin_installer.png"))


  # ----------------------------------------- #
  def getThemeIcon(self, theName):
    """ get the icon from the best available theme """
    myCurThemePath = QgsApplication.activeThemePath() + "/plugins/" + theName;
    myDefThemePath = QgsApplication.defaultThemePath() + "/plugins/" + theName;
    myQrcPath = ":/plugins/installer/" + theName;
    if QFile.exists(myCurThemePath):
      return QIcon(myCurThemePath)
    elif QFile.exists(myDefThemePath):
      return QIcon(myDefThemePath)
    elif QFile.exists(myQrcPath):
      return QIcon(myQrcPath)
    else:
      return QIcon()


  # ----------------------------------------- #
  def initGui(self):
    """ create action that will start plugin window and then add it to menu """
    self.action = QAction(self.getThemeIcon("plugin_installer.png"), QCoreApplication.translate("QgsPluginInstaller","Fetch Python Plugins..."), self.mainWindow())
    self.action.setWhatsThis(QCoreApplication.translate("QgsPluginInstaller","Install more plugins from remote repositories"))
    self.action.setStatusTip(QCoreApplication.translate("QgsPluginInstaller","Install more plugins from remote repositories"))
    nextAction = self.iface.actionManagePlugins()
    self.iface.pluginMenu().insertAction(nextAction,self.action)
    QObject.connect(self.action, SIGNAL("triggered()"), self.run)
    QObject.connect(self.iface, SIGNAL("currentThemeChanged ( QString )"), self.setCurrentTheme)
    self.statusLabel = None

    repositories.load()
    plugins.getAllInstalled()

    if repositories.checkingOnStart() and repositories.timeForChecking() and repositories.allEnabled():
      self.statusLabel = QLabel(QCoreApplication.translate("QgsPluginInstaller","Looking for new plugins..."), self.mainWindow().statusBar())
      self.mainWindow().statusBar().insertPermanentWidget(0,self.statusLabel)
      QObject.connect(self.statusLabel, SIGNAL("linkActivated (QString)"), self.preRun)
      QObject.connect(repositories, SIGNAL("checkingDone()"), self.checkingDone)
      for key in repositories.allEnabled():
        repositories.requestFetching(key)
    else:
      for key in repositories.allEnabled():
        repositories.setRepositoryData(key,"state",3)
    for i in plugins.obsoletePlugins:
      if i == "plugin_installer":
        # uninstall the installer itself
        QMessageBox.warning(self.mainWindow(), QCoreApplication.translate("QgsPluginInstaller","QGIS Plugin Installer update"), QCoreApplication.translate("QgsPluginInstaller","The Plugin Installer has been updated. Please restart QGIS prior to using it"))
        removeDir( QFileInfo(QgsApplication.qgisUserDbFilePath()).path() + "/python/plugins/" + plugins.localCache[i]["localdir"] )
        return  
      else:
        QMessageBox.warning(self.mainWindow(), QCoreApplication.translate("QgsPluginInstaller","QGIS Plugin Conflict:")+" "+plugins.localCache[i]["name"], "<b>"+ plugins.localCache[i]["name"] + "</b><br/><br/>" + QCoreApplication.translate("QgsPluginInstaller","The Plugin Installer has detected an obsolete plugin which masks a newer version shipped with this QGIS version. This is likely due to files associated with a previous installation of QGIS. Please use the Plugin Installer to remove that older plugin in order to unmask the newer version shipped with this copy of QGIS."))


  # ----------------------------------------- #
  def checkingDone(self):
    """ display the notify label if any updates or news available """
    if not self.statusLabel:
      return
    # look for news in the repositories
    plugins.markNews()
    status = ""
    # first check for news
    for key in plugins.all():
      if plugins.all()[key]["status"] == "new":
        status = QCoreApplication.translate("QgsPluginInstaller","There is a new plugin available")
    # then check for updates (and eventually overwrite status)
    for key in plugins.all():
      if plugins.all()[key]["status"] == "upgradeable":
        status = QCoreApplication.translate("QgsPluginInstaller","There is a plugin update available")
    # finally set the notify label
    if status:
      self.statusLabel.setText(u' <a href="#">%s</a>  ' % status)
    else:
      self.mainWindow().statusBar().removeWidget(self.statusLabel)
      self.statusLabel = None


  # ----------------------------------------- #
  def unload(self):
    """ remove the menu item and notify label """
    # kill pending http requests
    for key in repositories.all():
      repositories.killConnection(key)
    self.iface.pluginMenu().removeAction(self.action)
    if self.statusLabel:
      self.mainWindow().statusBar().removeWidget(self.statusLabel)
    if self.guiDlg:
      self.guiDlg.close()


  # ----------------------------------------- #
  def preRun(self, * params):
    """ stupid method to get rid of the string value """
    self.run()


  # ----------------------------------------- #
  def run(self, parent = None):
    """ create and show a configuration dialog """
    QApplication.setOverrideCursor(Qt.WaitCursor)
    if self.statusLabel:
      self.mainWindow().statusBar().removeWidget(self.statusLabel)
      self.statusLabel = None

    if not parent:
      parent = self.mainWindow()
  
    for key in repositories.all():
      if repositories.all()[key]["state"] == 3: # if state = 3 (error), try to fetch once again
        repositories.requestFetching(key)

    if repositories.fetchingInProgress():
      self.fetchDlg = QgsPluginInstallerFetchingDialog(parent)
      self.fetchDlg.exec_()
      del self.fetchDlg
      for key in repositories.all():
        repositories.killConnection(key)

    QApplication.restoreOverrideCursor()

    # display error messages for every unavailable reposioty, unless Shift pressed nor all repositories are unavailable
    keepQuiet = QgsApplication.keyboardModifiers() == Qt.KeyboardModifiers(Qt.ShiftModifier)
    if repositories.allUnavailable() and repositories.allUnavailable() != repositories.allEnabled():
      for key in repositories.allUnavailable():
        if not keepQuiet:
          QMessageBox.warning(parent, QCoreApplication.translate("QgsPluginInstaller","QGIS Python Plugin Installer"), QCoreApplication.translate("QgsPluginInstaller","Error reading repository:") + QString(' %s\n%s' % (key,repositories.all()[key]["error"])))
        if QgsApplication.keyboardModifiers() == Qt.KeyboardModifiers(Qt.ShiftModifier):
          keepQuiet = True

    flags = Qt.WindowTitleHint | Qt.WindowSystemMenuHint | Qt.WindowMaximizeButtonHint 
    self.guiDlg = QgsPluginInstallerDialog(parent,flags)
    self.guiDlg.show()


  # ----------------------------------------- #
  def newlyInstalledPlugins(self):
    """ return the list of newly installed plugins for further loading """
    return history.toList("A")


  # ----------------------------------------- #
  def newlyUninstalledPlugins(self):
    """ return the list of newly uninstalled plugins for further unloading """
    return history.toList("D")


  # ----------------------------------------- #
  def newlyReinstalledPlugins(self):
    """ return the list of newly reinstalled plugins for further reloading """
    return history.toList("R")


  # ----------------------------------------- #
  def resetNewlyProcessedPlugins(self):
    """ clear the dict of newly processed plugins """
    history.clear()