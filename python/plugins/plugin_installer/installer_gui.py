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
from qgis.core import QgsApplication, QgsContextHelp
import sys, time
from qgsplugininstallerfetchingbase import Ui_QgsPluginInstallerFetchingDialogBase
from qgsplugininstallerinstallingbase import Ui_QgsPluginInstallerInstallingDialogBase
from qgsplugininstallerrepositorybase import Ui_QgsPluginInstallerRepositoryDetailsDialogBase
from qgsplugininstallerpluginerrorbase import Ui_QgsPluginInstallerPluginErrorDialogBase
from qgsplugininstallerbase import Ui_QgsPluginInstallerDialogBase
from installer_data import *



# --- common functions ------------------------------------------------------------------- #
def removeDir(path):
    result = QString()
    if not QFile(path).exists():
      result = QCoreApplication.translate("QgsPluginInstaller","Nothing to remove! Plugin directory doesn't exist:")+"\n"+path
    elif QFile(path).remove(): # if it is only link, just remove it without resolving.
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
      result = QCoreApplication.translate("QgsPluginInstaller","Failed to remove the directory:")+"\n"+path+"\n"+QCoreApplication.translate("QgsPluginInstaller","Check permissions or remove it manually")
    # restore plugin directory if removed by QDir().rmpath()
    pluginDir = unicode(QFileInfo(QgsApplication.qgisUserDbFilePath()).path()+"/python/plugins")
    if not QDir(pluginDir).exists():
      QDir().mkpath(pluginDir)
    return result
# --- /common functions ------------------------------------------------------------------ #





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
    QObject.connect(repositories, SIGNAL("repositoryFetched(QString)"), self.repositoryFetched)
    QObject.connect(repositories, SIGNAL("anythingChanged(QString, int, int)"), self.displayState)


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
    self.connect(self.editName, SIGNAL("textChanged(const QString &)"), self.textChanged)
    self.connect(self.editURL,  SIGNAL("textChanged(const QString &)"), self.textChanged)
    self.textChanged(None)

  # ----------------------------------------- #
  def textChanged(self, string):
    enable = (self.editName.text().count() > 0 and self.editURL.text().count() > 0)
    self.buttonBox.button(QDialogButtonBox.Ok).setEnabled(enable)
# --- /class QgsPluginInstallerRepositoryDialog ------------------------------------------------------------ #





# --- class QgsPluginInstallerInstallingDialog --------------------------------------------------------------- #
class QgsPluginInstallerInstallingDialog(QDialog, Ui_QgsPluginInstallerInstallingDialogBase):
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
    path = QString(url.toPercentEncoding(url.path(), "!$&'()*+,;=:/@"))
    fileName = plugin["filename"]
    #print "Retrieving from %s" % path
    tmpDir = QDir.tempPath()
    tmpPath = QDir.cleanPath(tmpDir+"/"+fileName)
    self.file = QFile(tmpPath)
    self.http = QPHttp(url.host())
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

    # if the target directory already exists as a link, remove the link without resolving:
    QFile(pluginDir+QString(QDir.separator())+self.plugin["localdir"]).remove()

    #print "Extracting to plugin directory (%s)" % pluginDir
    try:
      un = unzip()
      un.extract(tmpPath, pluginDir) # test extract. If fails, then exception will be raised and no removing occurs
      #print "Removing old plugin files if exist"
      removeDir(QDir.cleanPath(pluginDir+"/"+self.plugin["localdir"])) # remove old plugin if exists
      un.extract(tmpPath, pluginDir) # final extract.
    except:
      self.mResult = self.tr("Failed to unzip the plugin package. Probably it's broken or missing from the repository. You may also want to make sure that you have write permission to the plugin directory:") + "\n" + pluginDir
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





# --- class QgsPluginInstallerDialog ------------------------------------------------------------------------- #
class QgsPluginInstallerDialog(QDialog, Ui_QgsPluginInstallerDialogBase):
  # ----------------------------------------- #
  def __init__(self, parent, fl):
    QDialog.__init__(self, parent, fl)
    self.setupUi(self)
    self.reposGroup = "/Qgis/plugin-repos"

    self.connect(self.lineFilter, SIGNAL("textChanged (QString)"), self.filterChanged)
    self.connect(self.comboFilter1, SIGNAL("currentIndexChanged (int)"), self.filterChanged)
    self.connect(self.comboFilter2, SIGNAL("currentIndexChanged (int)"), self.filterChanged)
    # grab clicks on trees
    self.connect(self.treePlugins, SIGNAL("itemSelectionChanged()"), self.pluginTreeClicked)
    self.connect(self.treeRepositories, SIGNAL("itemSelectionChanged()"), self.repositoryTreeClicked)
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
    self.buttonEditRep.setEnabled(False)
    self.buttonDeleteRep.setEnabled(False)

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
        QMessageBox.warning(self, self.tr("QGIS Python Plugin Installer"), self.tr("Error reading repository:") + " " + key + "\n" + repositories.all()[key]["error"])

    plugins.getAllInstalled()


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
          a.setToolTip(0,self.tr("This repository is blocked due to incompatibility with your Quantum GIS version"))
        a.setDisabled(True)
    for i in [0,1,2]:
      self.treeRepositories.resizeColumnToContents(i)
    self.comboFilter1.addItem(self.tr("orphans"))
    # filling the status filter comboBox
    self.comboFilter2.clear()
    self.comboFilter2.addItem(self.tr("any status"))
    self.comboFilter2.addItem(self.tr("not installed", "plural"))
    self.comboFilter2.addItem(self.tr("installed", "plural"))
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
    if self.comboFilter2.currentIndex() == 2 and not plugin["status"] in ["installed","upgradeable","newer","orphan"]:
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
    statusTips={"not installed" : self.tr("This plugin is not installed"),
            "installed" : self.tr("This plugin is installed"),
            "upgradeable" : self.tr("This plugin is installed, but there is an updated version available"),
            "orphan" : self.tr("This plugin is installed, but I can't find it in any enabled repository"),
            "new" : self.tr("This plugin is not installed and is seen for the first time"),
            "newer" : self.tr("This plugin is installed and is newer than its version available in a repository"),
            "incompatible" : self.tr("This plugin is incompatible with your Quantum GIS version and probably won't work."),
            "dependent" : self.tr("The required Python module is not installed.\nFor more information, please visit its homepage."),
            "broken" : self.tr("This plugin seems to be broken.\nIt has been installed but can't be loaded.\nHere is the error message:")}
    statuses ={"not installed" : self.tr("not installed", "singular"),
            "installed" : self.tr("installed", "singular"),
            "upgradeable" : self.tr("upgradeable", "singular"),
            "orphan" : self.tr("installed", "singular"),
            "new" : self.tr("new!", "singular"),
            "newer" : self.tr("installed", "singular"),
            "incompatible" : self.tr("invalid", "singular"),
            "dependent" : self.tr("invalid", "singular"),
            "broken" : self.tr("invalid", "singular")}
    orderInvalid = ["incompatible","broken","dependent"]
    orderValid = ["upgradeable","new","not installed","installed","orphan","newer"]
    def addItem(p):
      if self.filterCheck(p):
        statusTip = statusTips[p["status"]]
        if p["read-only"]:
          statusTip = statusTip + "\n" + self.tr("Note that it's an uninstallable core plugin")
        installedVersion = p["version_inst"]
        if not installedVersion:
          installedVersion = "?"
        availableVersion = p["version_avail"]
        if not availableVersion:
          availableVersion = "?"
        if p["status"] == "upgradeable":
          ver = installedVersion + " -> " + availableVersion
        elif p["status"] ==  "newer":
          ver = installedVersion + " (" + availableVersion + ")"
        elif p["status"] in ["not installed", "new"]:
          ver = availableVersion
        else:
          ver = installedVersion
        if p["status"] in ["upgradeable","newer"]:
          verTip = self.tr("installed version") + ": " + installedVersion + "\n" + self.tr("available version") + ": " + availableVersion
        elif p["status"] in ["not installed", "new"]:
          verTip = self.tr("available version") + ": " + availableVersion
        elif p["status"] == "installed":
          verTip = self.tr("installed version") + ": " + installedVersion + "\n" + self.tr("That's the newest available version")
        elif p["status"] == "orphan":
          verTip = self.tr("installed version") + ": " + installedVersion + "\n" + self.tr("There is no version available for download")
        else:
          verTip = ""
        if p["error"] == "broken":
          desc = self.tr("This plugin is broken")
          descTip = statusTips[p["error"]] + "\n" + p["error_details"]
          statusTip = descTip
        elif p["error"] == "incompatible":
          desc = self.tr("This plugin requires a newer version of Quantum GIS") + " (" + self.tr("at least")+ " " + p["error_details"] + ")"
          descTip = statusTips[p["error"]]
          statusTip = descTip
        elif p["error"] == "dependent":
          desc = self.tr("This plugin requires a missing module") + " (" + p["error_details"] + ")"
          descTip = statusTips[p["error"]]
          statusTip = descTip
        else:
          desc = p["desc_local"]
          descTip = p["desc_repo"]
        if not desc:
          desc = descTip
        if not p["repository"]:
          repository = self.tr("only locally available")
        else:
          repository = p["repository"]
        a = QTreeWidgetItem(self.treePlugins)
        if p["error"]:
          a.setText(0,statuses[p["error"]])
        else:
          a.setText(0,statuses[p["status"]])
        a.setToolTip(0,statusTip)
        a.setText(1,p["name"])
        a.setText(2,ver)
        a.setToolTip(2,verTip)
        a.setText(3,desc)
        a.setToolTip(3,descTip)
        a.setText(4,p["author"])
        if p["homepage"]:
          a.setToolTip(4,p["homepage"])
        else:
          a.setToolTip(4,"")
        a.setText(5,repository)
        a.setToolTip(5,p["url"])
        # set fonts and colours
        for i in [0,1,2,3,4,5]:
          if p["error"]:
            a.setForeground(i,QBrush(QColor(Qt.red)))
          if p["status"] in ["new","upgradeable"] or p["error"]:
            font = QFont()
            font.setWeight(QFont.Bold)
            a.setFont(i,font)
    # -------- #
    if not plugins.all():
      return
    self.treePlugins.clear()
    for i in orderInvalid:
      for p in plugins.all().values():
        if p["error"] == i:
          addItem(p)
    for i in orderValid:
      for p in plugins.all().values():
        if p["status"] == i and not p["error"]:
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
  def pluginTreeClicked(self):
    """ the pluginsTree has been clicked """
    buttons={"not installed":(True,False,self.tr("Install plugin")),
            "installed":(True,True,self.tr("Reinstall plugin")),
            "upgradeable":(True,True,self.tr("Upgrade plugin")),
            "orphan":(False,True,self.tr("Install/upgrade plugin")),
            "new":(True,False,self.tr("Install plugin")),
            "newer":(True,True,self.tr("Downgrade plugin"))}
    self.buttonInstall.setEnabled(False)
    self.buttonInstall.setText(self.tr("Install/upgrade plugin"))
    self.buttonUninstall.setEnabled(False)
    if not self.treePlugins.selectedItems():
      return
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
    infoString = ('','')
    key = plugins.keyByUrl(self.treePlugins.currentItem().toolTip(5))
    plugin = plugins.all()[key]
    previousStatus = plugin["status"]
    if not plugin:
      return
    if plugin["status"] == "newer" and not plugin["error"]: # ask for confirmation if user downgrades an usable plugin
      if QMessageBox.warning(self, self.tr("QGIS Python Plugin Installer"), self.tr("Are you sure you want to downgrade the plugin to the latest available version? The installed one is newer!"), QMessageBox.Yes, QMessageBox.No) == QMessageBox.No:
        return
    dlg = QgsPluginInstallerInstallingDialog(self,plugin)
    dlg.exec_()

    if dlg.result():
      infoString = (self.tr("Plugin installation failed"), dlg.result())
    elif not QDir(QDir.cleanPath(QgsApplication.qgisSettingsDirPath() + "/python/plugins/" + key)).exists():
      infoString = (self.tr("Plugin has disappeared"), self.tr("The plugin seems to have been installed but I don't know where. Probably the plugin package contained a wrong named directory.\nPlease search the list of installed plugins. I'm nearly sure you'll find the plugin there, but I just can't determine which of them it is. It also means that I won't be able to determine if this plugin is installed and inform you about available updates. However the plugin may work. Please contact the plugin author and submit this issue."))
      QApplication.setOverrideCursor(Qt.WaitCursor)
      self.getAllAvailablePlugins()
      QApplication.restoreOverrideCursor()
    else:
      try:
        exec ("sys.path_importer_cache.clear()")
        exec ("import %s" % plugin["localdir"])
        exec ("reload (%s)" % plugin["localdir"])
      except:
        pass
      plugins.updatePlugin(key, False)
      plugin = plugins.all()[key]
      if not plugin["error"]:
        if previousStatus in ["not installed", "new"]:
          infoString = (self.tr("Plugin installed successfully"),
          self.tr("Python plugin installed.\nYou have to enable it in the Plugin Manager."))
        else:
          infoString = (self.tr("Plugin reinstalled successfully"),
          self.tr("Python plugin reinstalled.\nYou have to restart Quantum GIS to reload it."))
      else:
        if plugin["error"] == "incompatible":
          message = self.tr("The plugin is designed for a newer version of Quantum GIS. The minimum required version is:")
          message += " <b>" + plugin["error_details"] + "</b>"
        elif plugin["error"] == "dependent":
          message = self.tr("The plugin depends on some components missing on this system. You need to install the following Python module in order to enable it:")
          message += "<b> " + plugin["error_details"] + "</b>"
        else:
          message = self.tr("The plugin is broken. Python said:")
          message += "<br><b>" + plugin["error_details"] + "</b>"
        dlg = QgsPluginInstallerPluginErrorDialog(self,message)
        dlg.exec_()
        if dlg.result():
          # revert installation
          plugins.setPluginData(key, "status", "not installed")
          plugins.setPluginData(key, "version_inst", "")
          plugins.setPluginData(key, "desc_local", "")
          plugins.setPluginData(key, "error", "")
          plugins.setPluginData(key, "error_details", "")
          pluginDir = unicode(QFileInfo(QgsApplication.qgisUserDbFilePath()).path()+"/python/plugins/"+ str(plugin["localdir"]))
          removeDir(pluginDir)
          if QDir(pluginDir).exists():
            infoString = (self.tr("Plugin uninstall failed"), result)
            try:
              exec ("sys.path_importer_cache.clear()")
              exec ("import %s" % plugin["localdir"])
              exec ("reload (%s)" % plugin["localdir"])
            except:
              pass
            plugins.updatePlugin(key, False)
          else:
            try:
              exec ("del sys.modules[%s]" % plugin["localdir"])
            except:
              pass
            if not plugin["repository"]:
              plugins.remove(key)
    self.populatePluginTree()
    if infoString[0]:
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
    warning = self.tr("Are you sure you want to uninstall the following plugin?") + "\n" + plugin["name"]
    if plugin["status"] == "orphan" and not plugin["error"]:
      warning += "\n\n"+self.tr("Warning: this plugin isn't available in any accessible repository!")
    if QMessageBox.warning(self, self.tr("QGIS Python Plugin Installer"), warning , QMessageBox.Yes, QMessageBox.No) == QMessageBox.No:
      return
    pluginDir = unicode(QFileInfo(QgsApplication.qgisUserDbFilePath()).path()+"/python/plugins/"+ str(plugin["localdir"]))
    #print "Uninstalling plugin", plugin["name"], pluginDir
    result = removeDir(pluginDir)
    if result:
      QMessageBox.warning(self, self.tr("Plugin uninstall failed"), result)
    else:
      try:
        exec ("del sys.modules[%s]" % plugin["localdir"])
      except:
        pass
      if not plugin["repository"]:
        plugins.remove(key)
      else:
        plugins.setPluginData(key, "status", "not installed")
        plugins.setPluginData(key, "version_inst", "")
        plugins.setPluginData(key, "desc_local", "")
        plugins.setPluginData(key, "error", "")
        plugins.setPluginData(key, "error_details", "")
      self.populatePluginTree()
      QMessageBox.information(self, self.tr("QGIS Python Plugin Installer"), self.tr("Plugin uninstalled successfully"))


  # ----------------------------------------- #
  def repositoryTreeClicked(self):
    """ the repositoryTree has been clicked """
    if self.treeRepositories.selectedItems():
      self.buttonEditRep.setEnabled(True)
      self.buttonDeleteRep.setEnabled(True)
    else:
      self.buttonEditRep.setEnabled(False)
      self.buttonDeleteRep.setEnabled(False)


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
    message = self.tr("You are going to add some plugin repositories neither authorized nor supported by the Quantum GIS team, however provided by folks associated with us. Plugin authors generally make efforts to make their works useful and safe, but we can't assume any responsibility for them. FEEL WARNED!")
    if QMessageBox.question(self, self.tr("QGIS Python Plugin Installer"), message, QMessageBox.Ok, QMessageBox.Abort) == QMessageBox.Ok:
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
      dlg.labelInfo.setText(self.tr("This repository is blocked due to incompatibility with your Quantum GIS version"))
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
    warning = self.tr("Are you sure you want to remove the following repository?") + "\n" + current.text(1)
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
