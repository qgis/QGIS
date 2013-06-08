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

import sys
import time

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from qgis.core import QgsApplication, QgsContextHelp

from ui_qgsplugininstallerfetchingbase import Ui_QgsPluginInstallerFetchingDialogBase
from ui_qgsplugininstallerinstallingbase import Ui_QgsPluginInstallerInstallingDialogBase
from ui_qgsplugininstallerrepositorybase import Ui_QgsPluginInstallerRepositoryDetailsDialogBase
from ui_qgsplugininstallerpluginerrorbase import Ui_QgsPluginInstallerPluginErrorDialogBase

from installer_data import *
from unzip import unzip




# --- class QgsPluginInstallerFetchingDialog --------------------------------------------------------------- #
class QgsPluginInstallerFetchingDialog(QDialog, Ui_QgsPluginInstallerFetchingDialogBase):
  # ----------------------------------------- #
  def __init__(self, parent):
    QDialog.__init__(self, parent)
    self.setupUi(self)
    self.progressBar.setRange(0,len(repositories.allEnabled())*100)
    self.itemProgress = {}
    self.item = {}
    for key in repositories.allEnabled():
      self.item[key] = QTreeWidgetItem(self.treeWidget)
      self.item[key].setText(0,key)
      if repositories.all()[key]["state"] > 1:
        self.itemProgress[key] = 100
        self.displayState(key,0)
      else:
        self.itemProgress[key] = 0
        self.displayState(key,2)
    self.treeWidget.resizeColumnToContents(0)
    repositories.repositoryFetched.connect(self.repositoryFetched)
    repositories.anythingChanged.connect(self.displayState)


  # ----------------------------------------- #
  def displayState(self,key,state,state2=None):
    messages=[self.tr("Success"),self.tr("Resolving host name..."),self.tr("Connecting..."),self.tr("Host connected. Sending request..."),self.tr("Downloading data..."),self.tr("Idle"),self.tr("Closing connection..."),self.tr("Error")]
    message = messages[state]
    if state2:
      message += " (%s%%)" % state2
    self.item[key].setText(1,message)

    if state == 4 and state2:
      self.itemProgress[key] = state2
    totalProgress = sum(self.itemProgress.values())
    self.progressBar.setValue(totalProgress)


  # ----------------------------------------- #
  def repositoryFetched(self, repoName):
    self.itemProgress[repoName] = 100
    if repositories.all()[repoName]["state"] == 2:
      self.displayState(repoName,0)
    else:
      self.displayState(repoName,7)
    if not repositories.fetchingInProgress():
      self.close()
# --- /class QgsPluginInstallerFetchingDialog -------------------------------------------------------------- #





# --- class QgsPluginInstallerRepositoryDialog ------------------------------------------------------------- #
class QgsPluginInstallerRepositoryDialog(QDialog, Ui_QgsPluginInstallerRepositoryDetailsDialogBase):
  # ----------------------------------------- #
  def __init__(self, parent=None):
    QDialog.__init__(self, parent)
    self.setupUi(self)
    self.editURL.setText("http://")
    self.editName.textChanged.connect(self.textChanged)
    self.editURL.textChanged.connect(self.textChanged)
    self.textChanged(None)

  # ----------------------------------------- #
  def textChanged(self, string):
    enable = (len(self.editName.text()) > 0 and len(self.editURL.text()) > 0)
    self.buttonBox.button(QDialogButtonBox.Ok).setEnabled(enable)
# --- /class QgsPluginInstallerRepositoryDialog ------------------------------------------------------------ #





# --- class QgsPluginInstallerInstallingDialog --------------------------------------------------------------- #
class QgsPluginInstallerInstallingDialog(QDialog, Ui_QgsPluginInstallerInstallingDialogBase):
  # ----------------------------------------- #
  def __init__(self, parent, plugin):
    QDialog.__init__(self, parent)
    self.setupUi(self)
    self.plugin = plugin
    self.mResult = ""
    self.progressBar.setRange(0,0)
    self.progressBar.setFormat("%p%")
    self.labelName.setText(plugin["name"])
    self.buttonBox.clicked.connect(self.abort)

    url = QUrl(plugin["download_url"])
    path = unicode(url.toPercentEncoding(url.path(), "!$&'()*+,;=:/@"))
    fileName = plugin["filename"]
    tmpDir = QDir.tempPath()
    tmpPath = QDir.cleanPath(tmpDir+"/"+fileName)
    self.file = QFile(tmpPath)
    port = url.port()
    if port < 0:
      port = 80
    self.http = QPHttp(url.host(), port)
    self.http.stateChanged.connect(self.stateChanged)
    self.http.dataReadProgress.connect(self.readProgress)
    self.http.requestFinished.connect(self.requestFinished)
    self.httpGetId = self.http.get(path, self.file)


  # ----------------------------------------- #
  def result(self):
    return self.mResult


  # ----------------------------------------- #
  def stateChanged(self, state):
    messages=[self.tr("Installing..."),self.tr("Resolving host name..."),self.tr("Connecting..."),self.tr("Host connected. Sending request..."),self.tr("Downloading data..."),self.tr("Idle"),self.tr("Closing connection..."),self.tr("Error")]
    self.labelState.setText(messages[state])


  # ----------------------------------------- #
  def readProgress(self, done, total):
    self.progressBar.setMaximum(total)
    self.progressBar.setValue(done)


  # ----------------------------------------- #
  def requestFinished(self, requestId, state):
    if requestId != self.httpGetId:
      return
    self.buttonBox.setEnabled(False)
    if state:
      self.mResult = self.http.errorString()
      self.reject()
      return
    self.file.close()
    pluginDir = QFileInfo(QgsApplication.qgisUserDbFilePath()).path() + "/python/plugins"
    tmpPath = self.file.fileName()
    # make sure that the parent directory exists
    if not QDir(pluginDir).exists():
      QDir().mkpath(pluginDir)
    # if the target directory already exists as a link, remove the link without resolving:
    QFile(pluginDir+unicode(QDir.separator())+self.plugin["id"]).remove()
    try:
      unzip(unicode(tmpPath), unicode(pluginDir)) # test extract. If fails, then exception will be raised and no removing occurs
      # removing old plugin files if exist
      removeDir(QDir.cleanPath(pluginDir+"/"+self.plugin["id"])) # remove old plugin if exists
      unzip(unicode(tmpPath), unicode(pluginDir)) # final extract.
    except:
      self.mResult = self.tr("Failed to unzip the plugin package. Probably it's broken or missing from the repository. You may also want to make sure that you have write permission to the plugin directory:") + "\n" + pluginDir
      self.reject()
      return
    try:
      # cleaning: removing the temporary zip file
      QFile(tmpPath).remove()
    except:
      pass
    self.close()


  # ----------------------------------------- #
  def abort(self):
    self.http.abort()
    self.mResult = self.tr("Aborted by user")
    self.reject()
# --- /class QgsPluginInstallerInstallingDialog ------------------------------------------------------------- #





# --- class QgsPluginInstallerPluginErrorDialog -------------------------------------------------------------- #
class QgsPluginInstallerPluginErrorDialog(QDialog, Ui_QgsPluginInstallerPluginErrorDialogBase):
  # ----------------------------------------- #
  def __init__(self, parent, errorMessage):
    QDialog.__init__(self, parent)
    self.setupUi(self)
    if not errorMessage:
      errorMessage = self.tr("no error message received")
    self.textBrowser.setText(errorMessage)
# --- /class QgsPluginInstallerPluginErrorDialog ------------------------------------------------------------- #


