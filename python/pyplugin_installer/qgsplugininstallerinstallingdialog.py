# -*- coding:utf-8 -*-
"""
/***************************************************************************
                           qgsplugininstallerinstallingdialog.py
                           Plugin Installer module
                             -------------------
    Date                 : June 2013
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

from qgis.core import QgsApplication

from ui_qgsplugininstallerinstallingbase import Ui_QgsPluginInstallerInstallingDialogBase
from installer_data import *
from unzip import unzip




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
