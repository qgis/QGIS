# -*- coding: utf-8 -*-
"""
Copyright (C) 2008 Matthew Perry
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
from qgis.core import QgsApplication, QgsContextHelp
import sys, time
from fetchingbase import Ui_QgsPluginInstallerFetchingDialog
from installingbase import Ui_QgsPluginInstallerInstallingDialog
from repositorybase import Ui_QgsPluginInstallerRepositoryDetailsDialog
from pluginerrorbase import Ui_QgsPluginInstallerPluginErrorDialog
from guibase import Ui_QgsPluginInstallerDialog
from installer_data import *



# --- common functions ------------------------------------------------------------------- #
def removeDir(path):
    result = QString()
    if not QFile(path).exists():
      result = QCoreApplication.translate("QgsPluginInstaller","Plugin directory doesn't exist: ") + path
    elif QFile(path).remove(): # if it's only link, just remove it without resolving.
      #print " Link removing successfull: %s" % path
      pass
    else:
      fltr = QDir.Dirs | QDir.Files | QDir.Hidden
      iterator = QDirIterator(path, fltr, QDirIterator.Subdirectories)
      while iterator.hasNext():
        item = iterator.next()
        if QFile(item).remove():
          #print " File removing successfull: %s" % item
          pass
      fltr = QDir.Dirs | QDir.Hidden
      iterator = QDirIterator(path, fltr, QDirIterator.Subdirectories)
      while iterator.hasNext():
        item = iterator.next()
        if QDir().rmpath(item):
          #print " Directory removing successfull: %s" % item
          pass
    if QFile(path).exists():
      result = QCoreApplication.translate("QgsPluginInstaller","Failed to remove directory")+" "+path+" \n"+QCoreApplication.translate("QgsPluginInstaller","Check permissions or remove it manually")
    # restore plugin directory if removed by QDir().rmpath()
    pluginDir = unicode(QFileInfo(QgsApplication.qgisUserDbFilePath()).path()+"/python/plugins")
    if not QDir(pluginDir).exists():
      QDir().mkpath(pluginDir)
    return result
# --- /common functions ------------------------------------------------------------------ #





# --- class QgsPluginInstallerFetchingDialog --------------------------------------------------------------- #
class QgsPluginInstallerFetchingDialog(QDialog, Ui_QgsPluginInstallerFetchingDialog):
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
    QObject.connect(repositories, SIGNAL("repositoryFetched(QString)"), self.repositoryFetched)
    QObject.connect(repositories, SIGNAL("anythingChanged(QString, int, int)"), self.displayState)


  # ----------------------------------------- #
  def displayState(self,key,state,state2=None):
    messages=[self.tr("Done"),self.tr("Resolving host name..."),self.tr("Connecting..."),self.tr("Host connected. Sending request..."),self.tr("Downloading data..."),self.tr("Idle"),self.tr("Closing connection..."),self.tr("Error")]
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
class QgsPluginInstallerRepositoryDialog(QDialog, Ui_QgsPluginInstallerRepositoryDetailsDialog):
  # ----------------------------------------- #
  def __init__(self, parent=None):
    QDialog.__init__(self, parent)
    self.setupUi(self)
    self.editURL.setText("http://")
    self.connect(self.editName, SIGNAL("textChanged(const QString &)"), self.textChanged)
    self.connect(self.editURL,  SIGNAL("textChanged(const QString &)"), self.textChanged)
    self.textChanged(None)

  # ----------------------------------------- #
  def textChanged(self, string):
    enable = (self.editName.text().count() > 0 and self.editURL.text().count() > 0)
    self.buttonBox.button(QDialogButtonBox.Ok).setEnabled(enable)
# --- /class QgsPluginInstallerRepositoryDialog ------------------------------------------------------------ #





# --- class QgsPluginInstallerInstallingDialog --------------------------------------------------------------- #
class QgsPluginInstallerInstallingDialog(QDialog, Ui_QgsPluginInstallerInstallingDialog):
  # ----------------------------------------- #
  def __init__(self, parent, plugin):
    QDialog.__init__(self, parent)
    self.setupUi(self)
    self.plugin = plugin
    self.mResult = QString()
    self.progressBar.setRange(0,0)
    self.progressBar.setFormat(QString("%p%"))
    self.labelName.setText(QString(plugin["name"]))
    self.connect(self.buttonBox, SIGNAL("clicked(QAbstractButton*)"), self.abort)

    url = QUrl(plugin["url"])
    path = QString(url.toPercentEncoding(url.path(), "!$&'()*+,;=:@/"))
    fileName = plugin["filename"]
    #print "Retrieving from %s" % path
    tmpDir = QDir.tempPath()
    tmpPath = QDir.cleanPath(tmpDir+"/"+fileName)
    self.file = QFile(tmpPath)
    self.http = QHttp(url.host())
    self.connect(self.http, SIGNAL("stateChanged ( int )"), self.stateChanged) 
    self.connect(self.http, SIGNAL("dataReadProgress ( int , int )"), self.readProgress)
    self.connect(self.http, SIGNAL("requestFinished (int, bool)"), self.requestFinished)
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
    pluginDir = unicode(QFileInfo(QgsApplication.qgisUserDbFilePath()).path()+"/python/plugins")
    tmpPath = unicode(self.file.fileName())

    # make sure that the parent directory exists
    if not QDir(pluginDir).exists():
      QDir().mkpath(pluginDir)
    #print "Extracting to plugin directory (%s)" % pluginDir
    try:
      un = unzip()
      un.extract(tmpPath, pluginDir) # test extract. If fails, then exception will be raised and no removing occurs
      #print "Removing old plugin files if exist"
      removeDir(QDir.cleanPath(pluginDir+"/"+self.plugin["localdir"])) # remove old plugin if exists
      un.extract(tmpPath, pluginDir) # final extract.
    except:
      self.mResult = self.tr("Failed to unzip file to ") + pluginDir + "\n" + self.tr("check permissions")
      self.reject()
      return

    try:
      #print "Cleaning: removing the zip file (%s)" % tmpPath
      QFile(tmpPath).remove()
    except:
      pass

    self.close()


  # ----------------------------------------- #
  def abort(self):
    self.http.abort()
    self.mResult = self.tr("Aborted by user")
    self.reject()
# --- /class QgsPluginInstallerPluginErrorDialog ------------------------------------------------------------- #





# --- class QgsPluginInstallerPluginErrorDialog -------------------------------------------------------------- #
class QgsPluginInstallerPluginErrorDialog(QDialog, Ui_QgsPluginInstallerPluginErrorDialog):
  # ----------------------------------------- #
  def __init__(self, parent, errorMessage):
    QDialog.__init__(self, parent)
    self.setupUi(self)
    if not errorMessage:
      errorMessage = self.tr("No error message received. Try to restart QGIS and ensure the plugin isn't installed under different name. If it is, contact the plugin author and submit this issue, please.")
    self.textBrowser.setText(errorMessage)
# --- /class QgsPluginInstallerPluginErrorDialog ------------------------------------------------------------- #





# --- class QgsPluginInstallerDialog ------------------------------------------------------------------------- #
class QgsPluginInstallerDialog(QDialog, Ui_QgsPluginInstallerDialog):
  # ----------------------------------------- #
  def __init__(self, parent, fl):
    QDialog.__init__(self, parent, fl)
    self.setupUi(self)
    self.reposGroup = "/Qgis/plugin-repos"

    self.connect(self.lineFilter, SIGNAL("textChanged (QString)"), self.filterChanged)
    self.connect(self.comboFilter1, SIGNAL("currentIndexChanged (int)"), self.filterChanged)
    self.connect(self.comboFilter2, SIGNAL("currentIndexChanged (int)"), self.filterChanged)
    # grab the click on the treePlugins
    self.connect(self.treePlugins, SIGNAL("itemSelectionChanged()"), self.treeClicked)
    # buttons
    self.connect(self.buttonInstall, SIGNAL("clicked()"), self.installPlugin)
    self.connect(self.buttonUninstall, SIGNAL("clicked()"), self.uninstallPlugin)
    self.buttonInstall.setEnabled(False)
    self.buttonUninstall.setEnabled(False)
    # repositories handling
    self.connect(self.treeRepositories, SIGNAL("doubleClicked(QModelIndex)"), self.editRepository)
    self.connect(self.buttonFetchRepositories, SIGNAL("clicked()"), self.addKnownRepositories)
    self.connect(self.buttonAddRep, SIGNAL("clicked()"), self.addRepository)
    self.connect(self.buttonEditRep, SIGNAL("clicked()"), self.editRepository)
    self.connect(self.buttonDeleteRep, SIGNAL("clicked()"), self.deleteRepository)
    # checkingOnStart checkbox
    self.connect(self.checkUpdates, SIGNAL("stateChanged (int)"), self.ChangeCheckingPolicy)
    if repositories.checkingOnStart():
      self.checkUpdates.setCheckState(Qt.Checked)
    else:
      self.checkUpdates.setCheckState(Qt.Unchecked)

    self.populateMostWidgets()


  # ----------------------------------------- #
  def getAllAvailablePlugins(self):
    """ repopulate the mPlugins dict """
    repositories.load()
    plugins.clear()
    for key in repositories.allEnabled():
      repositories.requestFetching(key)

    if repositories.fetchingInProgress():
      self.fetchDlg = QgsPluginInstallerFetchingDialog(self)
      self.fetchDlg.exec_()
      del self.fetchDlg
      for key in repositories.all():
        repositories.killConnection(key)

    # display error messages for every unavailable reposioty, except the case if all repositories are unavailable!
    if repositories.allUnavailable() and repositories.allUnavailable() != repositories.allEnabled():
      for key in repositories.allUnavailable():
        QMessageBox.warning(self, self.tr("QGIS Python Plugin Installer"), self.tr("Error reading repository: ") + key + "\n" + repositories.all()[key]["error"])


  # ----------------------------------------- #
  def populateMostWidgets(self):
    self.comboFilter1.clear()
    self.comboFilter1.addItem(self.tr("all repositories"))
    self.treeRepositories.clear()
    for key in repositories.all():
      a = QTreeWidgetItem(self.treeRepositories)
      a.setText(1,key)
      a.setText(2,repositories.all()[key]["url"])
      if repositories.all()[key]["enabled"] and repositories.all()[key]["valid"]:
        if repositories.all()[key]["state"] == 2:
          a.setText(0,self.tr("connected"))
          a.setIcon(0,QIcon(":/plugins/installer/repoConnected.png"))
          a.setToolTip(0,self.tr("This repository is connected"))
          self.comboFilter1.addItem(key)
        else:
          a.setText(0,self.tr("unavailable"))
          a.setIcon(0,QIcon(":/plugins/installer/repoUnavailable.png"))
          a.setToolTip(0,self.tr("This repository is enabled, but unavailable"))
          self.comboFilter1.addItem(key)
      else:
        a.setText(0,self.tr("disabled"))
        a.setIcon(0,QIcon(":/plugins/installer/repoDisabled.png"))
        if repositories.all()[key]["valid"]:
          a.setToolTip(0,self.tr("This repository is disabled"))
        else:
          a.setToolTip(0,self.tr("This repository is blocked due to incompatibility with your QGIS version"))
        a.setDisabled(True)
    for i in [0,1,2]:
      self.treeRepositories.resizeColumnToContents(i)
    self.comboFilter1.addItem(self.tr("orphans"))
    # filling the status filter comboBox
    self.comboFilter2.clear()
    self.comboFilter2.addItem(self.tr("any status"))
    self.comboFilter2.addItem(self.tr("not installed"))
    self.comboFilter2.addItem(self.tr("installed"))
    if plugins.isThereAnythingNew():
      self.comboFilter2.addItem(self.tr("upgradeable and news"))


  # ----------------------------------------- #
  def filterChanged(self,i):
    """ one of the filter widgets has been changed """
    self.populatePluginTree()


  # ----------------------------------------- #
  def filterCheck(self,plugin):
    """ the filter for the pluginsTree """
    if self.comboFilter1.currentIndex() != 0 and self.comboFilter1.currentText() != self.tr("orphans"):
      if self.comboFilter1.currentText() != plugin["repository"]:
        return False
    elif self.comboFilter1.currentText() == self.tr("orphans"):
      if plugin["status"] != "orphan":
        return False
    if self.comboFilter2.currentIndex() == 1 and not plugin["status"] in ["not installed","new"]:
      return False
    if self.comboFilter2.currentIndex() == 2 and not plugin["status"] in ["installed","upgradeable","newer","orphan","invalid"]:
      return False
    if self.comboFilter2.currentIndex() == 3 and not plugin["status"] in ["upgradeable","new"]:
      return False
    if self.lineFilter.text() == "":
      return True
    else:
      for i in ["name","version_inst","version_avail","desc_repo","desc_local","author","status","repository"]:
        item = str(plugin[i]).upper()
        if item != None:
          if item.find(self.lineFilter.text().toUpper()) > -1:
            return True
    return False


  # ----------------------------------------- #
  def populatePluginTree(self):
    """ fill up the pluginTree """
    descrip={"not installed" : self.tr("This plugin is not installed"),
            "installed" : self.tr("This plugin is installed"),
            "upgradeable" : self.tr("This plugin is installed, but there is an updated version available"),
            "orphan" : self.tr("This plugin is installed, but I can't find it in any enabled repository"),
            "new" : self.tr("This plugin is not installed and is seen first time"),
            "newer" : self.tr("This plugin is installed and is newer than its version available in a repository"),
            "invalid" : self.tr("This plugin seems to be invalid or have unfulfilled dependencies\nIt has been installed, but can't be loaded")}
    status ={"not installed" : self.tr("not installed"),
            "installed" : self.tr("installed"),
            "upgradeable" : self.tr("upgradeable"),
            "orphan" : self.tr("installed"),
            "new" : self.tr("new!"),
            "newer" : self.tr("installed"),
            "invalid" : self.tr("invalid")}
    order = ["invalid","upgradeable","new","not installed","installed","orphan","newer"]
    def addItem(p):
      if self.filterCheck(p):
        statusTip = descrip[p["status"]]
        if p["read-only"]:
          statusTip += "\n" + self.tr("Note that it's installed in the read-only location and you can't uninstall it")
        if p["status"] == "upgradeable":
          ver = p["version_inst"] + " -> " + p["version_avail"]
        elif p["status"] ==  "newer":
          ver = p["version_inst"] + " (" + p["version_avail"] + ")"
        elif p["status"] in ["not installed", "new", "invalid"]:
          ver = p["version_avail"]
        else:
          ver = p["version_inst"]
        if p["status"] in ["upgradeable","newer"]:
          vd = self.tr("installed version") + ": " + p["version_inst"] + "\n" + self.tr("available version") + ": " + p["version_avail"]
        elif p["status"] in ["not installed", "new"]:
          vd = self.tr("available version") + ": " + p["version_avail"]
        elif p["status"] == "installed":
          vd = self.tr("installed version") + ": " + p["version_inst"] + "\n" + self.tr("That's the newest available version.")
        elif p["status"] == "orphan":
          vd = self.tr("installed version") + ": " + p["version_inst"] + "\n" + self.tr("There is no version available for download.")
        else:
          vd = ""
        if p["status"] == "invalid":
          p["desc_local"] = self.tr("This plugin seems to be invalid or have unfulfilled dependencies")
          p["desc_repo"] = self.tr("This plugin seems to be invalid or have unfulfilled dependencies\nIt has been installed, but can't be loaded")
        if p["status"] == "orphan":
          repository = self.tr("only locally available")
        else:
          repository = p["repository"]
        if not p["desc_local"]:
          p["desc_local"] = p["desc_repo"]
        a = QTreeWidgetItem(self.treePlugins)
        a.setText(0,status[p["status"]])
        a.setToolTip(0,statusTip)
        a.setText(1,p["name"])
        a.setText(2,ver)
        a.setToolTip(2,vd)
        a.setText(3,p["desc_local"])
        a.setToolTip(3, p["desc_repo"])
        a.setText(4,p["author"])
        if p["homepage"]:
          a.setToolTip(4,p["homepage"])
        else:
          a.setToolTip(4,"")
        a.setText(5,repository)
        a.setToolTip(5,p["url"])
        # set fonts and colours
        for i in [0,1,2,3,4,5]:
          if p["status"] == "invalid":
            a.setForeground(i,QBrush(QColor(Qt.red)))
          if p["status"] in ["new","upgradeable","invalid"]:
            font = QFont()
            font.setWeight(QFont.Bold)
            a.setFont(i,font)
    # -------- #
    if not plugins.all():
      return
    self.treePlugins.clear()
    for i in order:
      for p in plugins.all().values():
        if p["status"] == i:
          addItem(p)
    # resize the columns
    for i in [0,1,2,3,4,5]:
      self.treePlugins.resizeColumnToContents(i)
    for i in [0,1,2,4,5]:
      if self.treePlugins.columnWidth(i) > 260:
        self.treePlugins.setColumnWidth(i, 260)
    if self.treePlugins.columnWidth(3) > 560:
      self.treePlugins.setColumnWidth(3, 560)
    # initially, keep order of inserting
    self.treePlugins.sortItems(100,Qt.AscendingOrder)


  # ----------------------------------------- #
  def treeClicked(self):
    """ the pluginsTree has been clicked """
    buttons={"not installed":(True,False,self.tr("Install plugin")),
            "installed":(True,True,self.tr("Reinstall plugin")),
            "upgradeable":(True,True,self.tr("Upgrade plugin",)),
            "orphan":(False,True,self.tr("Install/upgrade plugin")),
            "new":(True, False,self.tr("Install plugin")),
            "newer":(True,True,self.tr("Downgrade plugin")),
            "invalid":(False,True,self.tr("Reinstall plugin"))}
    self.buttonInstall.setEnabled(False)
    self.buttonInstall.setText(self.tr("Install/upgrade plugin"))
    self.buttonUninstall.setEnabled(False)
    item = self.treePlugins.currentItem()
    if not item:
      return
    key = plugins.keyByUrl(item.toolTip(5))
    if not key:
      return
    plugin = plugins.all()[key]
    if not plugin:
      return
    self.buttonInstall.setEnabled(buttons[plugin["status"]][0])
    self.buttonUninstall.setEnabled(buttons[plugin["status"]][1])
    self.buttonInstall.setText(buttons[plugin["status"]][2])
    if plugin["read-only"]:
      self.buttonUninstall.setEnabled(False)


  # ----------------------------------------- #
  def installPlugin(self):
    """ install currently selected plugin """
    if not self.treePlugins.currentItem():
      return
    key = plugins.keyByUrl(self.treePlugins.currentItem().toolTip(5))
    plugin = plugins.all()[key]
    if not plugin:
      return

    dlg = QgsPluginInstallerInstallingDialog(self,plugin)
    dlg.exec_()

    if dlg.result():
      infoString = (self.tr("Plugin installation failed"), dlg.result())
    else:
      try:
        exec ("sys.path_importer_cache.clear()")
        exec ("del sys.modules[%s]" % plugin["localdir"]) # remove old version if exist
      except:
        pass
      try:
        exec ("import %s" % plugin["localdir"])
        exec ("reload (%s)" % plugin["localdir"])
        if plugin["status"] == "not installed" or plugin["status"] == "new":
          infoString = (self.tr("Plugin installed successfully"), self.tr("Python plugin installed.\nYou have to enable it in the Plugin Manager."))
        else:
          infoString = (self.tr("Plugin installed successfully"),self.tr("Python plugin reinstalled.\nYou have to restart Quantum GIS to reload it."))
      except Exception, error:
        dlg = QgsPluginInstallerPluginErrorDialog(self,error.message)
        dlg.exec_()
        if dlg.result():
          pluginDir = unicode(QFileInfo(QgsApplication.qgisUserDbFilePath()).path()+"/python/plugins/"+ str(plugin["localdir"]))
          result = removeDir(pluginDir)
          if result:
            QMessageBox.warning(self, self.tr("Plugin uninstall failed"), result)
        plugins.updatePlugin(key, False)
        self.populateMostWidgets()
        self.populatePluginTree()
        return
      plugins.updatePlugin(key, False)
    self.populateMostWidgets()
    self.populatePluginTree()
    QMessageBox.information(self, infoString[0], infoString[1])


  # ----------------------------------------- #
  def uninstallPlugin(self):
    """ uninstall currently selected plugin """
    if not self.treePlugins.currentItem():
      return
    key = plugins.keyByUrl(self.treePlugins.currentItem().toolTip(5))
    plugin = plugins.all()[key]
    if not plugin:
      return

    warning = self.tr("Are you sure you want to uninstall the plugin ") + plugin["name"] + "?"
    if plugin["status"] == "orphan":
      warning += "\n\n"+self.tr("Warning: this plugin isn't available in any accessible repository!")
    if QMessageBox.warning(self, self.tr("QGIS Python Plugin Installer"), warning , QMessageBox.Yes, QMessageBox.No) == QMessageBox.No:
      return
    pluginDir = unicode(QFileInfo(QgsApplication.qgisUserDbFilePath()).path()+"/python/plugins/"+ str(plugin["localdir"]))
    #print "Uninstalling plugin", plugin["name"], pluginDir
    result = removeDir(pluginDir)
    if result:
      QApplication.restoreOverrideCursor()
      QMessageBox.warning(self, self.tr("Plugin uninstall failed"), result)
    else:
      try:
        exec ("del sys.modules[%s]" % plugin["localdir"])
      except:
        pass
      if plugin["status"] == "orphan":
        plugins.remove(key)
      else:
        plugins.setPluginData(key, "status", "not installed")
        plugins.setPluginData(key, "version_inst", "")
        plugins.setPluginData(key, "desc_local", "")
      self.populatePluginTree()
      QApplication.restoreOverrideCursor()
      QMessageBox.information(self, self.tr("QGIS Python Plugin Installer"), self.tr("Plugin uninstalled successfully"))


  # ----------------------------------------- #
  def ChangeCheckingPolicy(self,policy):
    if policy == Qt.Checked:
      repositories.setCheckingOnStart(True)
    else:
      repositories.setCheckingOnStart(False)


  # ----------------------------------------- #
  def addKnownRepositories(self):
    """ update list of known repositories - in the future it will be replaced with an online fetching """
    #print "add known repositories"
    message = "You are going to add some plugin repositories neither authorized nor supported by the QGIS team, however provided by folks associated with us.\n"
    message += "Plugin authors generally make efforts to make their works useful and safe, but we can't assume any responsibility for them. FEEL WARNED!"
    if QMessageBox.question(self, self.tr("QGIS Python Plugin Installer"), self.tr(message), QMessageBox.Ok, QMessageBox.Abort) == QMessageBox.Ok:
      repositories.addKnownRepos()
      # refresh lists and populate widgets
      QApplication.setOverrideCursor(Qt.WaitCursor)
      self.getAllAvailablePlugins()
      self.populateMostWidgets()
      self.populatePluginTree()
      QApplication.restoreOverrideCursor()


  # ----------------------------------------- #
  def addRepository(self):
    """ add repository button has been clicked """
    #print "add"
    dlg = QgsPluginInstallerRepositoryDialog(self)
    dlg.checkBoxEnabled.setCheckState(Qt.Checked)
    if not dlg.exec_():
      return
    for i in repositories.all().values():
      if dlg.editURL.text() == i["url"]:
        QMessageBox.warning(self, self.tr("QGIS Python Plugin Installer"), self.tr("Unable to add another repository with the same URL!"))
        return
    settings = QSettings()
    settings.beginGroup(self.reposGroup)
    reposName = dlg.editName.text()
    reposURL = dlg.editURL.text()
    if repositories.all().has_key(reposName):
      reposName = reposName + "(2)"
    #print "name: "+reposName
    #print "url: "+reposURL
    # add to settings
    settings.setValue(reposName+"/url", QVariant(reposURL))
    settings.setValue(reposName+"/enabled", QVariant(bool(dlg.checkBoxEnabled.checkState())))
    # refresh lists and populate widgets
    QApplication.setOverrideCursor(Qt.WaitCursor)
    self.getAllAvailablePlugins()
    self.populateMostWidgets()
    self.populatePluginTree()
    QApplication.restoreOverrideCursor()


  # ----------------------------------------- #
  def editRepository(self):
    """ edit repository button has been clicked """
    #print "edit"
    checkState={False:Qt.Unchecked,True:Qt.Checked}
    current = self.treeRepositories.currentItem()
    if current == None:
      return
    reposName = current.text(1)
    dlg = QgsPluginInstallerRepositoryDialog(self)
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
      if dlg.editURL.text() == i["url"] and dlg.editURL.text() != repositories.all()[reposName]["url"]:
        QMessageBox.warning(self, self.tr("QGIS Python Plugin Installer"), self.tr("Unable to add another repository with the same URL!"))
        return
    # delete old repo from QSettings and create new one
    settings = QSettings()
    settings.beginGroup(self.reposGroup)
    settings.remove(reposName)
    newName = dlg.editName.text()
    if repositories.all().has_key(newName) and newName != reposName:
      newName = newName + "(2)"
    settings.setValue(newName+"/url", QVariant(dlg.editURL.text()))
    settings.setValue(newName+"/enabled", QVariant(bool(dlg.checkBoxEnabled.checkState())))
    if dlg.editURL.text() == repositories.all()[reposName]["url"] and dlg.checkBoxEnabled.checkState() == checkState[repositories.all()[reposName]["enabled"]]:
      repositories.rename(reposName, newName)
      self.populateMostWidgets()
      return # nothing else to do if only repository name was changed
    # refresh lists and populate widgets
    QApplication.setOverrideCursor(Qt.WaitCursor)
    self.getAllAvailablePlugins()
    self.populateMostWidgets()
    self.populatePluginTree()
    QApplication.restoreOverrideCursor()


  # ----------------------------------------- #
  def deleteRepository(self):
    """ delete repository button has been clicked """
    #print "delete"
    current = self.treeRepositories.currentItem()
    if current == None:
      return
    warning = self.tr("Are you sure you want to remove the repository") + "\n" + current.text(1) + "?"
    if QMessageBox.warning(self, self.tr("QGIS Python Plugin Installer"), warning , QMessageBox.Yes, QMessageBox.No) == QMessageBox.No:
      return
    reposName = current.text(1)
    settings = QSettings()
    settings.beginGroup(self.reposGroup)
    # delete from settings
    settings.remove(reposName)
    # refresh lists and populate widgets
    QApplication.setOverrideCursor(Qt.WaitCursor)
    self.getAllAvailablePlugins()
    self.populateMostWidgets()
    self.populatePluginTree()
    QApplication.restoreOverrideCursor()


  # ----------------------------------------- #
  def reject(self):
    """ update the list of seen plugins before exit (both 'done' and 'x' buttons emit 'reject' signal) """
    plugins.updateSeenPluginsList()
    QDialog.reject(self)
# --- /class QgsPluginInstallerDialog ------------------------------------------------------------------------ #
