# -*- coding: utf-8 -*-
"""
Copyright (C) 2007-2008 Matthew Perry
Copyright (C) 2008 Borys Jurgiel

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
    if QGIS_VER: # new plugin API
      self.mainWindow = self.iface.mainWindow
    else: # old plugin API
      self.mainWindow = self.iface.getMainWindow

  # ----------------------------------------- #
  def initGui(self):
    """ create action that will start plugin window and then add it to menu """
    self.action = QAction(QIcon(":/plugins/installer/plugin_installer.png"), QCoreApplication.translate("QgsPluginInstaller","Fetch Python Plugins..."), self.mainWindow())
    self.action.setWhatsThis(QCoreApplication.translate("QgsPluginInstaller","Install more plugins from remote repositories"))
    self.action.setStatusTip(QCoreApplication.translate("QgsPluginInstaller","Install more plugins from remote repositories"))
    if QGIS_VER: # new plugin API
      nextAction = self.iface.actionManagePlugins()
      self.iface.pluginMenu().insertAction(nextAction,self.action)
    else: # old plugin API
      nextAction = self.mainWindow().menuBar().actions()[4].menu().actions()[1]
      self.mainWindow().menuBar().actions()[4].menu().insertAction(nextAction,self.action)
    QObject.connect(self.action, SIGNAL("activated()"), self.run)
    self.statusLabel = None

    repositories.load()
    plugins.clear()

    if repositories.checkingOnStart() and repositories.allEnabled():
      self.statusLabel = QLabel(QCoreApplication.translate("QgsPluginInstaller","Looking for new plugins..."), self.mainWindow().statusBar())
      self.mainWindow().statusBar().insertPermanentWidget(0,self.statusLabel)
      QObject.connect(self.statusLabel, SIGNAL("linkActivated (QString)"), self.run)
      QObject.connect(repositories, SIGNAL("checkingDone()"), self.checkingDone)
      for key in repositories.allEnabled():
        repositories.requestFetching(key)
    else:
      for key in repositories.allEnabled():
        repositories.setRepositoryData(key,"state",3)


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
    self.mainWindow().menuBar().actions()[4].menu().removeAction(self.action)
    if self.statusLabel:
      self.mainWindow().statusBar().removeWidget(self.statusLabel)


  # ----------------------------------------- #
  def run(self, * params):
    """ create and show a configuration dialog """
    QApplication.setOverrideCursor(Qt.WaitCursor)
    if self.statusLabel:
      self.mainWindow().statusBar().removeWidget(self.statusLabel)
      self.statusLabel = None

    for key in repositories.all():
      if repositories.all()[key]["state"] == 3: # if state = 3 (error), try to fetch once again
        repositories.requestFetching(key)

    if repositories.fetchingInProgress():
      self.fetchDlg = QgsPluginInstallerFetchingDialog(self.mainWindow())
      self.fetchDlg.exec_()
      del self.fetchDlg
      for key in repositories.all():
        repositories.killConnection(key)

    QApplication.restoreOverrideCursor()

    # display an error message for every unavailable reposioty, except the case if all repositories are unavailable!
    if repositories.allUnavailable() and repositories.allUnavailable() != repositories.allEnabled():
      for key in repositories.allUnavailable():
        QMessageBox.warning(self.mainWindow(), QCoreApplication.translate("QgsPluginInstaller","QGIS Python Plugin Installer"), QCoreApplication.translate("QgsPluginInstaller","Error reading repository:") + u' %s\n%s' % (key,repositories.all()[key]["error"]))

    plugins.getAllInstalled()

    flags = Qt.WindowTitleHint | Qt.WindowSystemMenuHint | Qt.WindowMaximizeButtonHint 
    self.guiDlg = QgsPluginInstallerDialog(self.mainWindow(),flags)
    self.guiDlg.show()
