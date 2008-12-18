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
from PyQt4.QtXml import QDomDocument
from PyQt4.QtNetwork import *
from qgis.core import *
from unzip import unzip
from version_compare import compareVersions, normalizeVersion


"""
Data structure:
mRepositories = dict of dicts: {repoName : {"url" QString,
                                            "enabled" bool,
                                            "valid" bool,
                                            "QPHttp" QPHttp,
                                            "Relay" Relay, # Relay object for transmitting signals from QPHttp with adding the repoName information
                                            "xmlData" QDomDocument,
                                            "state" int,   (0 - disabled, 1-loading, 2-loaded ok, 3-error (to be retried), 4-rejected)
                                            "error" QString}}
mPlugins = dict of dicts {id : {"name" QString,
                                "version_avail" QString,
                                "version_inst" QString,
                                "desc_repo" QString,
                                "desc_local" QString,
                                "author" QString,
                                "status" QString,      ("not installed", "installed", "upgradeable", "orphan", "new", "newer")
                                "error" QString,       ("", "broken", "incompatible", "dependent")
                                "error_details" QString,
                                "homepage" QString,
                                "url" QString,
                                "filename" QString,
                                "repository" QString,
                                "localdir" QString,
                                "read-only" boolean}}
"""


try:
  QGIS_VER = QGis.qgisVersion
  if QGIS_VER[0] == "1":
    QGIS_MAJOR_VER = 1
  else:
    QGIS_MAJOR_VER = 0
except:
  QGIS_VER = QGis.QGIS_VERSION
  QGIS_MAJOR_VER = 1



def setIface(qgisIface):
  global iface
  iface = qgisIface



reposGroup = "/Qgis/plugin-repos"
settingsGroup = "/Qgis/plugin-installer"
seenPluginGroup = "/Qgis/plugin-seen"


# knownRepos: (name, url for QGIS 0.x, url for QGIS 1.x, possible depreciated url, another possible depreciated url)
knownRepos = [("Official QGIS Repository","http://spatialserver.net/cgi-bin/pyqgis_plugin.rb","http://pyqgis.org/repo/official","",""),
              ("Carson Farmer's Repository","http://www.ftools.ca/cfarmerQgisRepo.xml","http://www.ftools.ca/cfarmerQgisRepo.xml", "http://www.geog.uvic.ca/spar/carson/cfarmerQgisRepo.xml","http://www.ftools.ca/cfarmerQgisRepo_0.xx.xml"),
              ("Borys Jurgiel's Repository","http://bwj.aster.net.pl/qgis-oldapi/plugins.xml","http://bwj.aster.net.pl/qgis/plugins.xml","",""),
              ("Faunalia Repository","http://faunalia.it/qgis/plugins.xml","http://faunalia.it/qgis/plugins.xml","http://faunalia.it/qgis/1.x/plugins.xml","")]




# --- class QPHttp  ----------------------------------------------------------------------- #
# --- It's a temporary workaround for broken proxy handling in Qt ------------------------- #
class QPHttp(QHttp):
  def __init__(self,*args):
    QHttp.__init__(self,*args)
    settings = QSettings()
    settings.beginGroup("proxy")
    if settings.value("/proxyEnabled").toBool():
      self.proxy=QNetworkProxy()
      proxyType = settings.value( "/proxyType", QVariant(0)).toString()
      if proxyType in ["1","Socks5Proxy"]: self.proxy.setType(QNetworkProxy.Socks5Proxy)
      elif proxyType in ["2","NoProxy"]: self.proxy.setType(QNetworkProxy.NoProxy)
      elif proxyType in ["3","HttpProxy"]: self.proxy.setType(QNetworkProxy.HttpProxy)
      elif proxyType in ["4","HttpCachingProxy"] and QT_VERSION >= 0X040400: self.proxy.setType(QNetworkProxy.HttpCachingProxy)
      elif proxyType in ["5","FtpCachingProxy"] and QT_VERSION >= 0X040400: self.proxy.setType(QNetworkProxy.FtpCachingProxy)
      else: self.proxy.setType(QNetworkProxy.DefaultProxy)
      self.proxy.setHostName(settings.value("/proxyHost").toString())
      self.proxy.setPort(settings.value("/proxyPort").toUInt()[0])
      self.proxy.setUser(settings.value("/proxyUser").toString())
      self.proxy.setPassword(settings.value("/proxyPassword").toString())
      self.setProxy(self.proxy)
    settings.endGroup()
    return None
# --- /class QPHttp  ---------------------------------------------------------------------- #





# --- class Relay  ----------------------------------------------------------------------- #
class Relay(QObject):
  """ Relay object for transmitting signals from QPHttp with adding the repoName information """
  # ----------------------------------------- #
  def __init__(self, key):
    QObject.__init__(self)
    self.key = key

  def stateChanged(self, state):
    self.emit(SIGNAL("anythingChanged(QString, int, int)"), self.key, state, 0)

  # ----------------------------------------- #
  def dataReadProgress(self, done, total):
    state = 4
    if total:
      progress = int(float(done)/float(total)*100)
    else:
      progress = 0
    self.emit(SIGNAL("anythingChanged(QString, int, int)"), self.key, state, progress)
# --- /class Relay  ---------------------------------------------------------------------- #





# --- class Repositories ----------------------------------------------------------------- #
class Repositories(QObject):
  """ A dict-like class for handling repositories data """
  # ----------------------------------------- #
  def __init__(self):
    QObject.__init__(self)
    self.mRepositories = {}
    self.httpId = {}   # {httpId : repoName}


  # ----------------------------------------- #
  def addKnownRepos(self):
    """ add known 3rd party repositories to QSettings """
    presentURLs = []
    for i in self.all().values():
      presentURLs += [QString(i["url"])]
    for i in knownRepos:
      if i[QGIS_MAJOR_VER+1] and presentURLs.count(i[QGIS_MAJOR_VER+1]) == 0:
        settings = QSettings()
        settings.beginGroup(reposGroup)
        repoName = QString(i[0])
        if self.all().has_key(repoName):
          repoName = repoName + "(2)"
        # add to settings
        settings.setValue(repoName+"/url", QVariant(i[QGIS_MAJOR_VER+1]))
        settings.setValue(repoName+"/enabled", QVariant(True))


  # ----------------------------------------- #
  def all(self):
    """ return dict of all repositories """
    return self.mRepositories


  # ----------------------------------------- #
  def allEnabled(self):
    """ return dict of all enabled and valid repositories """
    repos = {}
    for i in self.mRepositories:
      if self.mRepositories[i]["enabled"] and self.mRepositories[i]["valid"]:
        repos[i] = self.mRepositories[i]
    return repos


  # ----------------------------------------- #
  def allUnavailable(self):
    """ return dict of all unavailable repositories """
    repos = {}
    for i in self.mRepositories:
      if self.mRepositories[i]["enabled"] and self.mRepositories[i]["valid"] and self.mRepositories[i]["state"] == 3:
        repos[i] = self.mRepositories[i]
    return repos


  # ----------------------------------------- #
  def setRepositoryData(self,reposName, key, value):
    """ write data to the mRepositories dict """
    self.mRepositories[reposName][key] = value


  # ----------------------------------------- #
  def rename(self, oldName, newName):
    """ rename repository key """
    if oldName == newName:
      return
    self.mRepositories[newName] = self.mRepositories[oldName]
    del self.mRepositories[oldName]


  # ----------------------------------------- #
  def checkingOnStart(self):
    """ return true if checking for news and updates is enabled """
    settings = QSettings()
    return settings.value(settingsGroup+"/checkOnStart", QVariant(False)).toBool()


  # ----------------------------------------- #
  def setCheckingOnStart(self, state):
    """ set state of checking for news and updates """
    settings = QSettings()
    settings.setValue(settingsGroup+"/checkOnStart", QVariant(state))


  # ----------------------------------------- #
  def load(self):
    """ populate the mRepositories dict"""
    self.mRepositories = {}
    settings = QSettings()
    settings.beginGroup(reposGroup)
    # first, update the QSettings repositories if needed
    if len(settings.childGroups()) == 0: # add the default repository when there isn't any
      settings.setValue(knownRepos[0][0]+"/url", QVariant(knownRepos[0][QGIS_MAJOR_VER+1]))
    else: # else update invalid urls
      for key in settings.childGroups():
        url = settings.value(key+"/url", QVariant()).toString()
        allOk = True
        for repo in knownRepos:
          if repo[3] == url or repo[4] == url or (repo[QGIS_MAJOR_VER+1] != url and repo[int(not QGIS_MAJOR_VER)+1] == url):
            if repo[QGIS_MAJOR_VER+1]: #update the URL
              settings.setValue(key+"/url", QVariant(repo[QGIS_MAJOR_VER+1]))
              settings.setValue(key+"/valid", QVariant(True))
              allOk = False
            else: # mark as invalid
              settings.setValue(key+"/valid", QVariant(False))
              allOk = False
        if allOk: # marking as valid if no problem.
          settings.setValue(key+"/valid", QVariant(True))

    for key in settings.childGroups():
      self.mRepositories[key] = {}
      self.mRepositories[key]["url"] = settings.value(key+"/url", QVariant()).toString()
      self.mRepositories[key]["enabled"] = settings.value(key+"/enabled", QVariant(True)).toBool()
      self.mRepositories[key]["valid"] = settings.value(key+"/valid", QVariant(True)).toBool()
      self.mRepositories[key]["QPHttp"] = QPHttp()
      self.mRepositories[key]["Relay"] = Relay(key)
      self.mRepositories[key]["xmlData"] = QBuffer()
      self.mRepositories[key]["state"] = 0
      self.mRepositories[key]["error"] = QString()
    settings.endGroup()


  # ----------------------------------------- #
  def requestFetching(self,key):
    """ start fetching the repository given by key """
    self.mRepositories[key]["state"] = 1
    url = QUrl(self.mRepositories[key]["url"])
    path = QString(url.toPercentEncoding(url.path(), "!$&'()*+,;=:@/"))
    self.mRepositories[key]["QPHttp"] = QPHttp(url.host())
    self.connect(self.mRepositories[key]["QPHttp"], SIGNAL("requestFinished (int, bool)"), self.xmlDownloaded)
    self.connect(self.mRepositories[key]["QPHttp"], SIGNAL("stateChanged ( int )"), self.mRepositories[key]["Relay"].stateChanged)
    self.connect(self.mRepositories[key]["QPHttp"], SIGNAL("dataReadProgress ( int , int )"), self.mRepositories[key]["Relay"].dataReadProgress)
    self.connect(self.mRepositories[key]["Relay"], SIGNAL("anythingChanged(QString, int, int)"), self, SIGNAL("anythingChanged (QString, int, int)"))
    i = self.mRepositories[key]["QPHttp"].get(path, self.mRepositories[key]["xmlData"])
    self.httpId[i] = key


  # ----------------------------------------- #
  def fetchingInProgress(self):
    """ return true if fetching repositories is still in progress """
    for key in self.mRepositories:
      if self.mRepositories[key]["state"] == 1:
        return True
    return False


  # ----------------------------------------- #
  def killConnection(self, key):
    """ kill the fetching on demand """
    if self.mRepositories[key]["QPHttp"].state():
      self.mRepositories[key]["QPHttp"].abort()


  # ----------------------------------------- #
  def xmlDownloaded(self,nr,state):
    """ populate the plugins object with the fetched data """
    if not self.httpId.has_key(nr):
      return
    reposName = self.httpId[nr]
    if state:                             # fetching failed
      self.mRepositories[reposName]["state"] =  3
      self.mRepositories[reposName]["error"] = self.mRepositories[reposName]["QPHttp"].errorString()
    else:
      repoData = self.mRepositories[reposName]["xmlData"]
      reposXML = QDomDocument()
      reposXML.setContent(repoData.data())
      pluginNodes = reposXML.elementsByTagName("pyqgis_plugin")
      if pluginNodes.size():
        for i in range(pluginNodes.size()):
          fileName = QFileInfo(pluginNodes.item(i).firstChildElement("download_url").text().trimmed()).fileName()
          name = fileName.section(".", 0, 0)
          name = str(name)
          plugin = {}
          plugin[name] = {
            "name"          : pluginNodes.item(i).toElement().attribute("name"),
            "version_avail" : pluginNodes.item(i).toElement().attribute("version"),
            "desc_repo"     : pluginNodes.item(i).firstChildElement("description").text().trimmed(),
            "desc_local"    : "",
            "author"        : pluginNodes.item(i).firstChildElement("author_name").text().trimmed(),
            "homepage"      : pluginNodes.item(i).firstChildElement("homepage").text().trimmed(),
            "url"           : pluginNodes.item(i).firstChildElement("download_url").text().trimmed(),
            "filename"      : fileName,
            "status"        : "not installed",
            "error"         : "",
            "error_details" : "",
            "version_inst"  : "",
            "repository"    : reposName,
            "localdir"      : name,
            "read-only"     : False}
          qgisMinimumVersion = pluginNodes.item(i).firstChildElement("qgis_minimum_version").text().trimmed()
          if not qgisMinimumVersion: qgisMinimumVersion = "0"
          # please use the tag below only if really needed! (for example if plugin development is abandoned)
          qgisMaximumVersion = pluginNodes.item(i).firstChildElement("qgis_maximum_version").text().trimmed()
          if not qgisMaximumVersion: qgisMaximumVersion = "2"
          #if compatible, add the plugin to the list
          if compareVersions(QGIS_VER, qgisMinimumVersion) < 2 and compareVersions(qgisMaximumVersion, QGIS_VER) < 2:
            if QGIS_VER[0]=="0" or qgisMinimumVersion[0]=="1" or name=="plugin_installer":
              plugins.addPlugin(plugin)
        plugins.workarounds()
        self.mRepositories[reposName]["state"] = 2
      else:
        #print "Repository parsing error"
        self.mRepositories[reposName]["state"] = 3
        self.mRepositories[reposName]["error"] = QCoreApplication.translate("QgsPluginInstaller","Couldn't parse output from the repository")

    self.emit(SIGNAL("repositoryFetched(QString)"), reposName )

    # is the checking done?
    if not self.fetchingInProgress():
      plugins.getAllInstalled()
      self.emit(SIGNAL("checkingDone()"))
# --- /class Repositories ---------------------------------------------------------------- #





# --- class Plugins ---------------------------------------------------------------------- #
class Plugins(QObject):
  """ A dict-like class for handling plugins data """
  # ----------------------------------------- #
  def __init__(self):
    QObject.__init__(self)
    self.mPlugins = {}


  # ----------------------------------------- #
  def all(self):
    """ return all plugins """
    return self.mPlugins


  # ----------------------------------------- #
  def keyByUrl(self, name):
    """ return plugin key by given url """
    plugins = [i for i in self.mPlugins if self.mPlugins[i]["url"] == name]
    if plugins:
      return plugins[0]
    return None


  # ----------------------------------------- #
  def setPluginData(self, pluginName, key, value):
    """ write data to the mPlugins dict """
    self.mPlugins[pluginName][key] = value


  # ----------------------------------------- #
  def clear(self):
    """ clear the plugins dict"""
    self.mPlugins = {}


  # ----------------------------------------- #
  def addPlugin(self, plugins):
    """ add a plugin (first from given) to the mPlugins dict """
    key = plugins.keys()[0]
    plugin = plugins[key]
    plugin["version_avail"] = normalizeVersion(plugin["version_avail"])
    plugin["version_inst"] = normalizeVersion(plugin["version_inst"])
    if not self.mPlugins.has_key(key) or compareVersions(self.mPlugins[key]["version_avail"],plugin["version_avail"]) == 2:
      self.mPlugins[key] = plugin # add the plugin if not present yet or if is newer than existing one


  # ----------------------------------------- #
  def remove(self, key):
    """ remove given plugin from the mPlugins dict """
    del self.mPlugins[key]


  # ----------------------------------------- #
  def updatePlugin(self, key, readOnly):
    """ The mPlugins should contain available plugins first. Now, add installed one (add when not present, update if present) """
    if readOnly:
      path = QgsApplication.pkgDataPath()
    else:
      path = QgsApplication.qgisSettingsDirPath()
    path = QDir.cleanPath(path) + "/python/plugins/" + key
    if not QDir(path).exists():
      return
    nam   = ""
    ver   = ""
    desc  = ""
    auth  = ""
    homepage  = ""
    error = ""
    errorDetails = ""
    try:
      exec("import "+ key)
      try:
        exec("nam = %s.name()" % key)
      except:
        pass
      try:
        exec("ver = %s.version()" % key)
      except:
        pass
      try:
        exec("desc = %s.description()" % key)
      except:
        pass
      try:
        exec("auth = %s.authorName()" % key)
      except:
        pass
      try:
        exec("homepage = %s.homepage()" % key)
      except:
        pass
      try:
        exec("qgisMinimumVersion = %s.qgisMinimumVersion()" % key)
        if compareVersions(QGIS_VER, qgisMinimumVersion) == 2:
          error = "incompatible"
          errorDetails = qgisMinimumVersion
      except:
        pass
      try:
        exec ("%s.classFactory(iface)" % key)
      except Exception, error:
        error = error.message
    except Exception, error:
      error = error.message

    if not nam:
      nam = key
    if error[:16] == "No module named ":
      mona = error.replace("No module named ","")
      if mona != key:
        error = "dependent"
        errorDetails = mona
    if not error in ["", "dependent", "incompatible"]:
      errorDetails = error
      error = "broken"

    plugin = {
        "name"          : nam,
        "version_inst"  : normalizeVersion(ver),
        "version_avail" : "",
        "desc_local"    : desc,
        "desc_repo"     : "",
        "author"        : auth,
        "homepage"      : homepage,
        "url"           : path,
        "filename"      : "",
        "status"        : "",
        "error"         : error,
        "error_details" : errorDetails,
        "repository"    : "",
        "localdir"      : key,
        "read-only"     : readOnly}

    if not self.mPlugins.has_key(key):
      self.mPlugins[key] = plugin   # just add a new plugin
    else:
      self.mPlugins[key]["localdir"] = plugin["localdir"]
      self.mPlugins[key]["read-only"] = plugin["read-only"]
      self.mPlugins[key]["error"] = plugin["error"]
      self.mPlugins[key]["error_details"] = plugin["error_details"]
      if plugin["name"] and plugin["name"] != key:
        self.mPlugins[key]["name"] = plugin["name"] # local name has higher priority
      self.mPlugins[key]["version_inst"] = plugin["version_inst"]
      self.mPlugins[key]["desc_local"] = plugin["desc_local"]
    # set status
    #
    # installed   available   status
    # ---------------------------------------
    # none        any         "not installed" (will be later checked if is "new")
    # any         none        "orphan"
    # same        same        "installed"
    # less        greater     "upgradeable"
    # greater     less        "newer"
    if not self.mPlugins[key]["version_avail"]:
      self.mPlugins[key]["status"] = "orphan"
    elif self.mPlugins[key]["error"] in ["broken","dependent"]:
      self.mPlugins[key]["status"] = "installed"
    elif not self.mPlugins[key]["version_inst"]:
      self.mPlugins[key]["status"] = "not installed"
    elif compareVersions(self.mPlugins[key]["version_avail"],self.mPlugins[key]["version_inst"]) == 0:
      self.mPlugins[key]["status"] = "installed"
    elif compareVersions(self.mPlugins[key]["version_avail"],self.mPlugins[key]["version_inst"]) == 1:
      self.mPlugins[key]["status"] = "upgradeable"
    else:
      self.mPlugins[key]["status"] = "newer"


  # ----------------------------------------- #
  def getAllInstalled(self):
    """ update the mPlugins dict with alredy installed plugins """
    # first, try to add the read-only plugins...
    try:
      pluginDir = QDir.cleanPath(unicode(QgsApplication.pkgDataPath()) + "/python/plugins")
      pluginDir = QDir(pluginDir)
      pluginDir.setFilter(QDir.AllDirs)
      for key in pluginDir.entryList():
        key = unicode(key)
        if not key in [".",".."]:
          self.updatePlugin(key, True)
    except:
      # return QCoreApplication.translate("QgsPluginInstaller","Couldn't open the system plugin directory")
      pass # it's not necessary to stop due to this error
    # ...then try to add locally installed ones
    try:
      pluginDir = QDir.cleanPath(unicode(QgsApplication.qgisSettingsDirPath()) + "/python/plugins")
      pluginDir = QDir(pluginDir)
      pluginDir.setFilter(QDir.AllDirs)
    except:
      return QCoreApplication.translate("QgsPluginInstaller","Couldn't open the local plugin directory")
    for key in pluginDir.entryList():
      key = unicode(key)
      if not key in [".",".."]:
        self.updatePlugin(key, False)


  # ----------------------------------------- #
  def markNews(self):
    """ mark all new plugins as new """
    settings = QSettings()
    seenPlugins = settings.value(seenPluginGroup, QVariant(QStringList(self.mPlugins.keys()))).toStringList()
    if len(seenPlugins) > 0:
      for i in self.mPlugins.keys():
        if seenPlugins.count(QString(i)) == 0 and self.mPlugins[i]["status"] == "not installed":
          self.mPlugins[i]["status"] = "new"


  # ----------------------------------------- #
  def updateSeenPluginsList(self):
    """ update the list of all seen plugins """
    settings = QSettings()
    seenPlugins = settings.value(seenPluginGroup, QVariant(QStringList(self.mPlugins.keys()))).toStringList()
    for i in self.mPlugins.keys():
      if seenPlugins.count(QString(i)) == 0:
        seenPlugins += [i]
    settings.setValue(seenPluginGroup, QVariant(QStringList(seenPlugins)))


  # ----------------------------------------- #
  def isThereAnythingNew(self):
    """ return true if an upgradeable or new plugin detected """
    for i in self.mPlugins.values():
      if i["status"] in ["upgradeable","new"]:
        return True
    return False


  # ----------------------------------------- #
  def workarounds(self):
    """ workarounds for particular plugins with wrong metadata """
    if self.mPlugins.has_key("select") and self.mPlugins["select"]["version_avail"] == "0.1":
      self.mPlugins["select"]["version_avail"] = "0.2"

# --- /class Plugins --------------------------------------------------------------------- #





# public members:
repositories = Repositories()
plugins = Plugins()
