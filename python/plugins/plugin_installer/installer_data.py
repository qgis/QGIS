# -*- coding: utf-8 -*-
"""
qgis_plugins.py
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
from PyQt4.QtNetwork import QHttp, QNetworkProxy
from qgis.core import *
from unzip import unzip



"""
Data structure:
mRepositories = dict of dicts: {repoName : {"url" string,
                                            "enabled" bool,
                                            "valid" bool,
                                            "QHttp" QHttp,
                                            "Relay" Relay, # Relay object for transmitting signals from QHttp with adding the repoName information
                                            "xmlData" QDomDocument,
                                            "state" int,   (0 - disabled, 1-loading, 2-loaded ok, 3-error (to be retried), 4-rejected)
                                            "error" QString}}
mPlugins = dict of dicts {id : {"name" string,
                                "version_avail" string,
                                "version_inst" string,
                                "desc_repo" string,
                                "desc_local" string,
                                "author" string,
                                "status" string,    ("not installed", "installed", "upgradeable", "orphan", "new", "newer", "invalid")
                                "homepage" string,
                                "url" string,
                                "filename" string,
                                "repository" string,
                                "localdir" string,
                                "read-only" boolean}}
"""


reposGroup = "/Qgis/plugin-repos"
settingsGroup = "/Qgis/plugin-installer"
seenPluginGroup = "/Qgis/plugin-seen"

# knownRepos: (name, url for QGIS 0.x, url for QGIS 1.x, possible depreciated url, another possible depreciated url)
knownRepos = [("Official QGIS Repository","http://spatialserver.net/cgi-bin/pyqgis_plugin.rb","http://spatialserver.net/cgi-bin/pyqgis_plugin.rb","",""),
              ("Carson Farmer's Repository","http://www.ftools.ca/cfarmerQgisRepo.xml","","http://www.geog.uvic.ca/spar/carson/cfarmerQgisRepo.xml",""),
              ("Barry Rowlingson's Repository","http://www.maths.lancs.ac.uk/~rowlings/Qgis/Plugins/plugins.xml","","",""),
              ("Borys Jurgiel's Repository","http://bwj.aster.net.pl/qgis-oldapi/plugins.xml","http://bwj.aster.net.pl/qgis/plugins.xml","",""),
              ("Faunalia Repository","http://faunalia.it/qgis/plugins.xml","","","")]



# --- class Relay  ----------------------------------------------------------------------- #
class Relay(QObject):
  """ Relay object for transmitting signals from QHttp with adding the repoName information """
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
  # ----------------------------------------- #
  def __init__(self):
    QObject.__init__(self)
    self.mRepositories = {}
    self.httpId = {}   # {httpId : repoName}
    self.QGISver = int(str(QGis.qgisVersion)[0])


  # ----------------------------------------- #
  def addKnownRepos(self):
    presentURLs = []
    for i in self.all().values():
      presentURLs += [str(i["url"])]
    for i in knownRepos:
      if i[self.QGISver+1] and presentURLs.count(i[self.QGISver+1]) == 0:
        settings = QSettings()
        settings.beginGroup(reposGroup)
        repoName = QString(i[0])
        if self.all().has_key(repoName):
          repoName = repoName + "(2)"
        # add to settings
        settings.setValue(repoName+"/url", QVariant(i[self.QGISver+1]))
        settings.setValue(repoName+"/enabled", QVariant(True))


  # ----------------------------------------- #
  def all(self):
    return self.mRepositories


  # ----------------------------------------- #
  def allEnabled(self):
    repos = {}
    for i in self.mRepositories:
      if self.mRepositories[i]["enabled"] and self.mRepositories[i]["valid"]:
        repos[i] = self.mRepositories[i]
    return repos


  # ----------------------------------------- #
  def allUnavailable(self):
    repos = {}
    for i in self.mRepositories:
      if self.mRepositories[i]["enabled"] and self.mRepositories[i]["valid"] and self.mRepositories[i]["state"] == 3:
        repos[i] = self.mRepositories[i]
    return repos


  # ----------------------------------------- #
  def setRepositoryData(self,reposName, key, value):
    self.mRepositories[reposName][key] = value


  # ----------------------------------------- #
  def rename(self,oldName, newName):
    if oldName == newName:
      return
    self.mRepositories[newName] = self.mRepositories[oldName]
    del self.mRepositories[oldName]


  # ----------------------------------------- #
  def checkingOnStart(self):
    settings = QSettings()
    return settings.value(settingsGroup+"/checkOnStart", QVariant(True)).toBool()


  # ----------------------------------------- #
  def setCheckingOnStart(self, state):
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
      settings.setValue(self.knownRepos[0][0]+"/url", QVariant(self.knownRepos[0][self.QGISver+1]))
    else: # else update invalid urls
      for key in settings.childGroups():
        url = settings.value(key+"/url", QVariant()).toString()
        allOk = True
        for repo in knownRepos:
          if repo[3] == url or repo[4] == url or (repo[self.QGISver+1] != url and repo[int(not self.QGISver)+1] == url):
            if repo[self.QGISver+1]: #update the URL
              settings.setValue(key+"/url", QVariant(repo[self.QGISver+1]))
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
      self.mRepositories[key]["QHttp"] = QHttp()
      self.mRepositories[key]["Relay"] = Relay(key)
      self.mRepositories[key]["xmlData"] = QBuffer()
      self.mRepositories[key]["state"] = 0
      self.mRepositories[key]["error"] = QString()
    settings.endGroup()


  # ----------------------------------------- #
  def requestFetching(self,key):
    self.mRepositories[key]["state"] = 1
    url = QUrl(self.mRepositories[key]["url"])
    path = QString(url.toPercentEncoding(url.path(), "!$&'()*+,;=:@/"))
    self.mRepositories[key]["QHttp"] = QHttp(url.host())
    self.connect(self.mRepositories[key]["QHttp"], SIGNAL("requestFinished (int, bool)"), self.xmlDownloaded)
    self.connect(self.mRepositories[key]["QHttp"], SIGNAL("stateChanged ( int )"), self.mRepositories[key]["Relay"].stateChanged)
    self.connect(self.mRepositories[key]["QHttp"], SIGNAL("dataReadProgress ( int , int )"), self.mRepositories[key]["Relay"].dataReadProgress)
    self.connect(self.mRepositories[key]["Relay"], SIGNAL("anythingChanged(QString, int, int)"), self, SIGNAL("anythingChanged (QString, int, int)"))
    i = self.mRepositories[key]["QHttp"].get(path, self.mRepositories[key]["xmlData"])
    self.httpId[i] = key


  # ----------------------------------------- #
  def fetchingInProgress(self):
    for key in self.mRepositories:
      if self.mRepositories[key]["state"] == 1:
        return True
    return False


  # ----------------------------------------- #
  def killConnection(self, key):
    if self.mRepositories[key]["QHttp"].state():
      self.mRepositories[key]["QHttp"].abort()


  # ----------------------------------------- #
  def xmlDownloaded(self,nr,state):
    if not self.httpId.has_key(nr):
      return
    reposName = self.httpId[nr]
    if state:                             # fetching failed
      self.mRepositories[reposName]["state"] =  3
      self.mRepositories[reposName]["error"] = self.mRepositories[reposName]["QHttp"].errorString()
      #print "Repository fetching failed! " , reposName , str(self.mRepositories[reposName]["error"])
    else:
      repoData = self.mRepositories[reposName]["xmlData"]
      reposXML = QDomDocument()
      reposXML.setContent(repoData.data())
      pluginNodes = reposXML.elementsByTagName("pyqgis_plugin")
      if pluginNodes.size():
        for i in range(pluginNodes.size()):
          name = QFileInfo(pluginNodes.item(i).firstChildElement("download_url").text().trimmed()).fileName()
          name = str(name[0:len(name)-4])
          plugin = {}
          plugin[name] = {
            "name"          : pluginNodes.item(i).toElement().attribute("name"),
            "version_avail" : pluginNodes.item(i).toElement().attribute("version"),
            "desc_repo"     : pluginNodes.item(i).firstChildElement("description").text().trimmed(),
            "desc_local"    : "",
            "author"        : pluginNodes.item(i).firstChildElement("author_name").text().trimmed(),
            "homepage"      : pluginNodes.item(i).firstChildElement("homepage").text().trimmed(),
            "url"           : pluginNodes.item(i).firstChildElement("download_url").text().trimmed(),
            "filename"      : pluginNodes.item(i).firstChildElement("file_name").text().trimmed(),
            "status"        : "not installed",
            "version_inst"  : "",
            "repository"    : reposName,
            "localdir"      : name,
            "read-only"     : False}
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
  # ----------------------------------------- #
  def __init__(self):
    QObject.__init__(self)
    self.mPlugins = {}


  # ----------------------------------------- #
  def all(self):
    return self.mPlugins


  # ----------------------------------------- #
  def keyByUrl(self, name):
    plugins = [i for i in self.mPlugins if self.mPlugins[i]["url"] == name]
    if plugins:
      return plugins[0]
    return None


  # ----------------------------------------- #
  def setPluginData(self, pluginName, key, value):
    self.mPlugins[pluginName][key] = value


  # ----------------------------------------- #
  def clear(self):
    """ clear the plugins dict"""
    self.mPlugins = {}


  # ----------------------------------------- #
  def normalizeVersion(self,ver):
    if not ver:
      return QString()
    if ver.toUpper().left(7) == "VERSION":
      ver.remove(0,7)
    elif ver.toUpper().left(4) == "VER.":
      ver.remove(0,4)
    if ver[0] == " ":
      ver.remove(0,1)
    return ver


  # ----------------------------------------- #
  def compareVersions(self,a,b):
    # -------- #
    def classify(s):
      if s in [".","-","_"," "]:
        return 0
      try:
        float(s)
        return 1
      except:
        return 2
    # -------- #
    def chop(s):
      s2 = [s[0]]
      for i in range(1,len(s)):
        if classify(s[i]) == 0:
          pass
        elif classify(s[i]) == classify(s[i-1]):
          s2[len(s2)-1] += s[i]
        else:
          s2 += [s[i]]
      return s2
    # -------- #
    def compare(s1,s2):
      # check if the matter is easy solvable:
      if s1 == s2:
        return 0
      # try to compare as numeric values:
      try:
        if s1[0] == '0' or s2[0] == '0':
          s1 = '0.' + s1
          s2 = '0.' + s2
        if float(s1) == float(s2):
          return 0
        elif float(s1) > float(s2):
          return 1
        else:
          return 2
      except:
        pass
      # set ALPHA < BETA < RC < FINAL < ANYTHING_ELSE
      if s1 == 'FINAL':
        s1 = 'Z' + s1
      elif not s1 in ['ALPHA','BETA','RC']:
        s1 = 'ZZ' + s1
      if s2 == 'FINAL':
        s2 = 'Z' + s2
      elif not s2 in ['ALPHA','BETA','RC']:
        s2 = 'ZZ' + s2
      # the real test:
      if s1 > s2:
        return 1
      else:
        return 2
    # -------- #
    if not a or not b:
      return 0
    a = unicode(a).upper()
    b = unicode(b).upper()
    if a == b:
      return 0

    v1 = chop(a)
    v2 = chop(b)
    l = len(v1)
    if l > len(v2):
      l = len(v2)

    for i in range(l):
      if compare(v1[i],v2[i]):
        return compare(v1[i],v2[i])

    if len(v1) > l:
      return compare(v1[l],u'')
    if len(v2) > l:
      return compare(u'',v2[l])
    # if everything else fails...
    if unicode(a) > unicode(b):
      return 1
    else:
      return 2


  # ----------------------------------------- #
  def addPlugin(self, plugins):
    key = plugins.keys()[0]
    plugin = plugins[key]
    plugin["version_avail"] = self.normalizeVersion(QString(plugin["version_avail"]))
    plugin["version_inst"] = self.normalizeVersion(QString(plugin["version_inst"]))
    if not self.mPlugins.has_key(key) or self.compareVersions(self.mPlugins[key]["version_avail"],plugin["version_avail"]) == 2:
      self.mPlugins[key] = plugin # add the plugin if not present yet or if is newer than existing one


  # ----------------------------------------- #
  def remove(self, key):
    del self.mPlugins[key]


  # ----------------------------------------- #
  def updatePlugin(self, key, readOnly):
    try:
      exec("import "+ key)
      try:
        exec("nam = %s.name()" % key)
      except:
        nam = ""
      try:
        exec("ver = %s.version()" % key)
      except:
        ver = ""
      try:
        exec("desc = %s.description()" % key)
      except:
        desc = ""
      try:
        exec("auth = %s.author_name()" % key)
      except:
        auth = ""
      try:
        exec("homepage = %s.homepage()" % key)
      except:
        homepage = ""
      stat = ""
    except:
      nam   = key
      stat  = "invalid"
      ver   = ""
      desc  = ""
      auth  = ""
      homepage  = ""
    if readOnly:
      path = QgsApplication.pkgDataPath()
    else:
      path = QgsApplication.qgisSettingsDirPath()
    path = QDir.cleanPath(unicode(path,"utf-8") + "python/plugins/" + key)
    normVer = self.normalizeVersion(QString(ver))
    plugin = {
        "name"          : nam,
        "version_inst"  : normVer,
        "version_avail" : "",
        "desc_local"    : desc,
        "desc_repo"     : "",
        "author"        : auth,
        "homepage"      : homepage,
        "url"           : path,
        "filename"      : "",
        "status"        : stat,
        "repository"    : "",
        "localdir"      : key,
        "read-only"     : readOnly}
    if not self.mPlugins.has_key(key):
      self.mPlugins[key] = plugin   # just add a new plugin
    else:
      self.mPlugins[key]["localdir"] = plugin["localdir"]
      self.mPlugins[key]["read-only"] = plugin["read-only"]
      if plugin["status"] == "invalid":
        self.mPlugins[key]["status"] = plugin["status"]
      else:
        self.mPlugins[key]["name"] = plugin["name"] # local name has higher priority, except invalid plugins
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
    # *marked as invalid*     "invalid"
    if self.mPlugins[key]["status"] == "invalid":
      pass
    elif not self.mPlugins[key]["version_inst"]:
      self.mPlugins[key]["status"] = "not installed"
    elif not self.mPlugins[key]["version_avail"]:
      self.mPlugins[key]["status"] = "orphan"
    elif self.compareVersions(self.mPlugins[key]["version_avail"],self.mPlugins[key]["version_inst"]) == 0:
      self.mPlugins[key]["status"] = "installed"
    elif self.compareVersions(self.mPlugins[key]["version_avail"],self.mPlugins[key]["version_inst"]) == 1:
      self.mPlugins[key]["status"] = "upgradeable"
    else:
      self.mPlugins[key]["status"] = "newer"


  # ----------------------------------------- #
  def getAllInstalled(self):
    """ update the mPlugins dict with alredy installed plugins """
    # first, try to add the read-only plugins...
    try:
      pluginDir = QDir.cleanPath(unicode(QgsApplication.pkgDataPath(),'utf-8') + "/python/plugins")
      pluginDir = QDir(pluginDir)
      pluginDir.setFilter(QDir.AllDirs)
    except:
      return QCoreApplication.translate("QgsPluginInstaller","Couldn't open the system plugin directory")
    for key in pluginDir.entryList():
      key = str(key)
      if not key in [".",".."]:
        self.updatePlugin(key, True)
    # ...then try to add locally installed ones
    try:
      pluginDir = QDir.cleanPath(unicode(QgsApplication.qgisSettingsDirPath(),'utf-8') + "/python/plugins")
      pluginDir = QDir(pluginDir)
      pluginDir.setFilter(QDir.AllDirs)
    except:
      return QCoreApplication.translate("QgsPluginInstaller","Couldn't open the local plugin directory")
    for key in pluginDir.entryList():
      key = str(key)
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
    for i in self.mPlugins.values():
      if i["status"] in ["upgradeable","new"]:
        return True
    return False


  # ----------------------------------------- #
  def workarounds(self):
    if self.mPlugins.has_key("postgps") and self.mPlugins["postgps"]["version_avail"] == "0.2":
      self.mPlugins["postgps"]["version_avail"] = "0.01"
    if self.mPlugins.has_key("select") and self.mPlugins["select"]["version_avail"] == "0.1":
      self.mPlugins["select"]["version_avail"] = "0.2"

# --- /class Plugins --------------------------------------------------------------------- #





# public members:
repositories = Repositories()
plugins = Plugins()
