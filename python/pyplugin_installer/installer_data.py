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
from PyQt4.QtXml import QDomDocument
from PyQt4.QtNetwork import *
import sys
import os
import codecs
import ConfigParser
import qgis.utils
from qgis.core import *
from qgis.utils import iface
from version_compare import compareVersions, normalizeVersion, isCompatible

"""
Data structure:
mRepositories = dict of dicts: {repoName : {"url" unicode,
                                            "enabled" bool,
                                            "valid" bool,
                                            "Relay" Relay, # Relay object for transmitting signals from QPHttp with adding the repoName information
                                            "Request" QNetworkRequest,
                                            "xmlData" QNetworkReply,
                                            "state" int,   (0 - disabled, 1-loading, 2-loaded ok, 3-error (to be retried), 4-rejected)
                                            "error" unicode}}


mPlugins = dict of dicts {id : {
    "id" unicode                                # module name
    "name" unicode,                             # human readable plugin name
    "description" unicode,                      # short description of the plugin purpose only
    "about" unicode,                            # longer description: how does it work, where does it install, how to run it?
    "category" unicode,                         # will be removed?
    "tags" unicode,                             # comma separated, spaces allowed
    "changelog" unicode,                        # may be multiline
    "author_name" unicode,                      # author name
    "author_email" unicode,                     # author email
    "homepage" unicode,                         # url to the plugin homepage
    "tracker" unicode,                          # url to a tracker site
    "code_repository" unicode,                  # url to the source code repository
    "version_installed" unicode,                # installed instance version
    "library" unicode,                          # absolute path to the installed library / Python module
    "icon" unicode,                             # path to the first:(INSTALLED | AVAILABLE) icon
    "pythonic" const bool=True                  # True if Python plugin
    "readonly" boolean,                         # True if core plugin
    "installed" boolean,                        # True if installed
    "available" boolean,                        # True if available in repositories
    "status" unicode,                           # ( not installed | new ) | ( installed | upgradeable | orphan | newer )
    "error" unicode,                            # NULL | broken | incompatible | dependent
    "error_details" unicode,                    # error description
    "experimental" boolean,                     # true if experimental, false if stable
    "version_available" unicode,                # available version
    "zip_repository" unicode,                   # the remote repository id
    "download_url" unicode,                     # url for downloading the plugin
    "filename" unicode,                         # the zip file name to be unzipped after downloaded
    "downloads" unicode,                        # number of dowloads
    "average_vote" unicode,                     # average vote
    "rating_votes" unicode,                     # number of votes
}}
"""



translatableAttributes = ["name", "description", "about", "tags"]

reposGroup = "/Qgis/plugin-repos"
settingsGroup = "/Qgis/plugin-installer"
seenPluginGroup = "/Qgis/plugin-seen"


# Repositories: (name, url, possible depreciated url)
officialRepo = ( QCoreApplication.translate("QgsPluginInstaller", "QGIS Official Plugin Repository"), "http://plugins.qgis.org/plugins/plugins.xml","http://plugins.qgis.org/plugins")
depreciatedRepos = [
    ("Old QGIS Official Repository",   "http://pyqgis.org/repo/official"),
    ("Old QGIS Contributed Repository","http://pyqgis.org/repo/contributed"),
    ("Aaron Racicot's Repository",     "http://qgisplugins.z-pulley.com"),
    ("Barry Rowlingson's Repository",  "http://www.maths.lancs.ac.uk/~rowlings/Qgis/Plugins/plugins.xml"),
    ("Bob Bruce's Repository",         "http://www.mappinggeek.ca/QGISPythonPlugins/Bobs-QGIS-plugins.xml"),
    ("Borys Jurgiel's Repository",     "http://bwj.aster.net.pl/qgis/plugins.xml"),
    ("Carson Farmer's Repository",     "http://www.ftools.ca/cfarmerQgisRepo.xml"),
    ("CatAIS Repository",              "http://www.catais.org/qgis/plugins.xml"),
    ("Faunalia Repository",            "http://www.faunalia.it/qgis/plugins.xml"),
    ("GIS-Lab Repository",             "http://gis-lab.info/programs/qgis/qgis-repo.xml"),
    ("Kappasys Repository",            "http://www.kappasys.org/qgis/plugins.xml"),
    ("Martin Dobias' Sandbox",         "http://mapserver.sk/~wonder/qgis/plugins-sandbox.xml"),
    ("Marco Hugentobler's Repository", "http://karlinapp.ethz.ch/python_plugins/python_plugins.xml"),
    ("Sourcepole Repository",          "http://build.sourcepole.ch/qgis/plugins.xml"),
    ("Volkan Kepoglu's Repository",    "http://ggit.metu.edu.tr/~volkan/plugins.xml")
]



################################################################################
################################################################################
### TEMPORARY WORKAROUND UNTIL VERSION NUMBER IS GLOBALY SWITCHED TO 2.0 #######
################################################################################
class QGis:                                   ##################################
    QGIS_VERSION_INT = 20000                  ##################################
    QGIS_VERSION = '2.0.0-Master'             ##################################
################################################################################
################################################################################


# --- common functions ------------------------------------------------------------------- #
def removeDir(path):
    result = ""
    if not QFile(path).exists():
      result = QCoreApplication.translate("QgsPluginInstaller","Nothing to remove! Plugin directory doesn't exist:")+"\n"+path
    elif QFile(path).remove(): # if it is only link, just remove it without resolving.
      pass
    else:
      fltr = QDir.Dirs | QDir.Files | QDir.Hidden
      iterator = QDirIterator(path, fltr, QDirIterator.Subdirectories)
      while iterator.hasNext():
        item = iterator.next()
        if QFile(item).remove():
          pass
      fltr = QDir.Dirs | QDir.Hidden
      iterator = QDirIterator(path, fltr, QDirIterator.Subdirectories)
      while iterator.hasNext():
        item = iterator.next()
        if QDir().rmpath(item):
          pass
    if QFile(path).exists():
      result = QCoreApplication.translate("QgsPluginInstaller","Failed to remove the directory:")+"\n"+path+"\n"+QCoreApplication.translate("QgsPluginInstaller","Check permissions or remove it manually")
    # restore plugin directory if removed by QDir().rmpath()
    pluginDir = QFileInfo(QgsApplication.qgisUserDbFilePath()).path() + "/python/plugins"
    if not QDir(pluginDir).exists():
      QDir().mkpath(pluginDir)
    return result
# --- /common functions ------------------------------------------------------------------ #





# --- class Relay  ----------------------------------------------------------------------- #
class Relay(QObject):
  """ Relay object for transmitting signals from QPHttp with adding the repoName information """
  # ----------------------------------------- #
  anythingChanged = pyqtSignal( unicode, int, int )

  def __init__(self, key):
    QObject.__init__(self)
    self.key = key

  def stateChanged(self, state):
    self.anythingChanged.emit( self.key, state, 0 )

  # ----------------------------------------- #
  def dataReadProgress(self, done, total):
    state = 4
    if total > 0:
      progress = int(float(done)/float(total)*100)
    else:
      progress = 0
    self.anythingChanged.emit( self.key, state, progress )

# --- /class Relay  ---------------------------------------------------------------------- #





# --- class Repositories ----------------------------------------------------------------- #
class Repositories(QObject):
  """ A dict-like class for handling repositories data """
  # ----------------------------------------- #

  anythingChanged = pyqtSignal( unicode, int, int )
  repositoryFetched = pyqtSignal( unicode )
  checkingDone = pyqtSignal()

  def __init__(self):
    QObject.__init__(self)
    self.mRepositories = {}
    self.httpId = {}   # {httpId : repoName}
    self.mInspectionFilter = None


  # ----------------------------------------- #
  def all(self):
    """ return dict of all repositories """
    return self.mRepositories


  # ----------------------------------------- #
  def allEnabled(self):
    """ return dict of all enabled and valid repositories """
    if self.mInspectionFilter:
      return { self.mInspectionFilter: self.mRepositories[self.mInspectionFilter] }

    repos = {}
    for i in self.mRepositories:
      if self.mRepositories[i]["enabled"] and self.mRepositories[i]["valid"]:
        repos[i] = self.mRepositories[i]
    return repos


  # ----------------------------------------- #
  def allUnavailable(self):
    """ return dict of all unavailable repositories """
    repos = {}

    if self.mInspectionFilter:
      # return the inspected repo if unavailable, otherwise empty dict
      if self.mRepositories[self.mInspectionFilter]["state"] == 3:
        repos [self.mInspectionFilter] = self.mRepositories[self.mInspectionFilter]
      return repos

    for i in self.mRepositories:
      if self.mRepositories[i]["enabled"] and self.mRepositories[i]["valid"] and self.mRepositories[i]["state"] == 3:
        repos[i] = self.mRepositories[i]
    return repos


  # ----------------------------------------- #
  def urlParams(self):
    """ return GET parameters to be added to every request """
    v=str(QGis.QGIS_VERSION_INT)
    return "?qgis=%d.%d" % ( int(v[0]), int(v[1:3]) )


  # ----------------------------------------- #
  def setRepositoryData(self, reposName, key, value):
    """ write data to the mRepositories dict """
    self.mRepositories[reposName][key] = value


  # ----------------------------------------- #
  def remove(self, reposName):
    """ remove given item from the mRepositories dict """
    del self.mRepositories[reposName]


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
    return settings.value(settingsGroup+"/checkOnStart", False, type=bool)


  # ----------------------------------------- #
  def setCheckingOnStart(self, state):
    """ set state of checking for news and updates """
    settings = QSettings()
    settings.setValue(settingsGroup+"/checkOnStart", state)


  # ----------------------------------------- #
  def checkingOnStartInterval(self):
    """ return checking for news and updates interval """
    settings = QSettings()
    try:
      # QSettings may contain non-int value...
      i = settings.value(settingsGroup+"/checkOnStartInterval", 1, type=int)
    except:
      # fallback do 1 day by default
      i = 1
    if i < 0: i = 1
    # allowed values: 0,1,3,7,14,30 days
    interval = 0
    for j in [1,3,7,14,30]:
      if i >= j:
        interval = j
    return interval


  # ----------------------------------------- #
  def setCheckingOnStartInterval(self, interval):
    """ set checking for news and updates interval """
    settings = QSettings()
    settings.setValue(settingsGroup+"/checkOnStartInterval", interval)


  # ----------------------------------------- #
  def saveCheckingOnStartLastDate(self):
    """ set today's date as the day of last checking  """
    settings = QSettings()
    settings.setValue(settingsGroup+"/checkOnStartLastDate", QDate.currentDate())


  # ----------------------------------------- #
  def timeForChecking(self):
    """ determine whether it's the time for checking for news and updates now """
    if self.checkingOnStartInterval() == 0:
      return True
    settings = QSettings()
    try:
      # QSettings may contain ivalid value...
      interval = settings.value(settingsGroup+"/checkOnStartLastDate",type=QDate).daysTo(QDate.currentDate())
    except:
      interval = 0
    if interval >= self.checkingOnStartInterval():
      return True
    else:
      return False


  # ----------------------------------------- #
  def load(self):
    """ populate the mRepositories dict"""
    self.mRepositories = {}
    settings = QSettings()
    settings.beginGroup(reposGroup)
    # first, update repositories in QSettings if needed
    officialRepoPresent = False
    for key in settings.childGroups():
      url = settings.value(key+"/url", "", type=unicode)
      if url == officialRepo[1]:
        officialRepoPresent = True
      if url == officialRepo[2]:
        settings.setValue(key+"/url", officialRepo[1]) # correct a depreciated url
        officialRepoPresent = True
    if not officialRepoPresent:
      settings.setValue(officialRepo[0]+"/url", officialRepo[1])

    for key in settings.childGroups():
      self.mRepositories[key] = {}
      self.mRepositories[key]["url"] = settings.value(key+"/url", "", type=unicode)
      self.mRepositories[key]["enabled"] = settings.value(key+"/enabled", True, type=bool)
      self.mRepositories[key]["valid"] = settings.value(key+"/valid", True, type=bool)
      self.mRepositories[key]["Relay"] = Relay(key)
      self.mRepositories[key]["xmlData"] = None
      self.mRepositories[key]["state"] = 0
      self.mRepositories[key]["error"] = ""
    settings.endGroup()


  # ----------------------------------------- #
  def requestFetching(self,key):
    """ start fetching the repository given by key """
    self.mRepositories[key]["state"] = 1
    url = QUrl(self.mRepositories[key]["url"] + self.urlParams() )
    #v=str(QGis.QGIS_VERSION_INT)
    #url.addQueryItem('qgis', '.'.join([str(int(s)) for s in [v[0], v[1:3]]]) ) # don't include the bugfix version!

    self.mRepositories[key]["QRequest"] = QNetworkRequest(url)
    self.mRepositories[key]["QRequest"].setAttribute( QNetworkRequest.User, key)
    self.mRepositories[key]["xmlData"] = QgsNetworkAccessManager.instance().get( self.mRepositories[key]["QRequest"] )
    self.mRepositories[key]["xmlData"].setProperty( 'reposName', key)
    self.mRepositories[key]["xmlData"].downloadProgress.connect( self.mRepositories[key]["Relay"].dataReadProgress )
    self.mRepositories[key]["xmlData"].finished.connect( self.xmlDownloaded )


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
    if self.mRepositories[key]["state"]==1 and self.mRepositories[key]["xmlData"] and self.mRepositories[key]["xmlData"].isRunning():
      self.mRepositories[key]["xmlData"].finished.disconnect()
      self.mRepositories[key]["xmlData"].abort()


  # ----------------------------------------- #
  def xmlDownloaded(self):
    """ populate the plugins object with the fetched data """
    reply = self.sender()
    reposName = reply.property( 'reposName' )
    if reply.error() != QNetworkReply.NoError:                             # fetching failed
      self.mRepositories[reposName]["state"] = 3
      self.mRepositories[reposName]["error"] = reply.errorString()
      if reply.error() == QNetworkReply.OperationCanceledError:
        self.mRepositories[reposName]["error"] += "\n\n" + QCoreApplication.translate("QgsPluginInstaller", "If you haven't cancelled the download manually, it was most likely caused by a timeout. In this case consider increasing the connection timeout value in QGIS options window.")
    else:
      reposXML = QDomDocument()
      reposXML.setContent(reply.readAll())
      pluginNodes = reposXML.elementsByTagName("pyqgis_plugin")
      if pluginNodes.size():
        for i in range(pluginNodes.size()):
          fileName = pluginNodes.item(i).firstChildElement("file_name").text().strip()
          if not fileName:
              fileName = QFileInfo(pluginNodes.item(i).firstChildElement("download_url").text().strip().split("?")[0]).fileName()
          name = fileName.partition(".")[0]
          experimental = False
          if pluginNodes.item(i).firstChildElement("experimental").text().strip().upper() in ["TRUE","YES"]:
            experimental = True
          icon = pluginNodes.item(i).firstChildElement("icon").text().strip()
          if icon and not icon.startswith("http"):
            icon = "http://%s/%s" % ( QUrl(self.mRepositories[reposName]["url"]).host() , icon )

          plugin = {
            "id"            : name,
            "name"          : pluginNodes.item(i).toElement().attribute("name"),
            "version_available" : pluginNodes.item(i).toElement().attribute("version"),
            "description"   : pluginNodes.item(i).firstChildElement("description").text().strip(),
            "about"         : pluginNodes.item(i).firstChildElement("about").text().strip(),
            "author_name"   : pluginNodes.item(i).firstChildElement("author_name").text().strip(),
            "homepage"      : pluginNodes.item(i).firstChildElement("homepage").text().strip(),
            "download_url"  : pluginNodes.item(i).firstChildElement("download_url").text().strip(),
            "category"      : pluginNodes.item(i).firstChildElement("category").text().strip(),
            "tags"          : pluginNodes.item(i).firstChildElement("tags").text().strip(),
            "changelog"     : pluginNodes.item(i).firstChildElement("changelog").text().strip(),
            "author_email"  : pluginNodes.item(i).firstChildElement("author_email").text().strip(),
            "tracker"       : pluginNodes.item(i).firstChildElement("tracker").text().strip(),
            "code_repository"  : pluginNodes.item(i).firstChildElement("repository").text().strip(),
            "downloads"     : pluginNodes.item(i).firstChildElement("downloads").text().strip(),
            "average_vote"  : pluginNodes.item(i).firstChildElement("average_vote").text().strip(),
            "rating_votes"  : pluginNodes.item(i).firstChildElement("rating_votes").text().strip(),
            "icon"          : icon,
            "experimental"  : experimental,
            "filename"      : fileName,
            "installed"     : False,
            "available"     : True,
            "status"        : "not installed",
            "error"         : "",
            "error_details" : "",
            "version_installed" : "",
            "zip_repository"    : reposName,
            "library"      : "",
            "readonly"     : False
          }
          qgisMinimumVersion = pluginNodes.item(i).firstChildElement("qgis_minimum_version").text().strip()
          if not qgisMinimumVersion: qgisMinimumVersion = "2"
          qgisMaximumVersion = pluginNodes.item(i).firstChildElement("qgis_maximum_version").text().strip()
          if not qgisMaximumVersion: qgisMaximumVersion = qgisMinimumVersion[0] + ".99"
          #if compatible, add the plugin to the list
          if not pluginNodes.item(i).firstChildElement("disabled").text().strip().upper() in ["TRUE","YES"]:
            if isCompatible(QGis.QGIS_VERSION, qgisMinimumVersion, qgisMaximumVersion):
              #add the plugin to the cache
              plugins.addFromRepository(plugin)
        self.mRepositories[reposName]["state"] = 2
      else:
        # no plugin metadata found
        self.mRepositories[reposName]["state"] = 3
        if reply.attribute(QNetworkRequest.HttpStatusCodeAttribute) == 200:
          self.mRepositories[reposName]["error"] = QCoreApplication.translate("QgsPluginInstaller", "Server response is 200 OK, but doesn't contain plugin metatada. This is most likely caused by a proxy or a wrong repository URL. You can configure proxy settings in QGIS options.")
        else:
          self.mRepositories[reposName]["error"] = QCoreApplication.translate("QgsPluginInstaller", "Status code:") + " %d %s" % (
                    reply.attribute(QNetworkRequest.HttpStatusCodeAttribute),
                    reply.attribute(QNetworkRequest.HttpReasonPhraseAttribute))

    self.repositoryFetched.emit( reposName )

    # is the checking done?
    if not self.fetchingInProgress():
      self.checkingDone.emit()

    reply.deleteLater()


  # ----------------------------------------- #
  def inspectionFilter(self):
    """ return inspection filter (only one repository to be fetched) """
    return self.mInspectionFilter


  # ----------------------------------------- #
  def setInspectionFilter(self, key = None):
    """ temporarily disable all repositories but this for inspection """
    self.mInspectionFilter = key

# --- /class Repositories ---------------------------------------------------------------- #





# --- class Plugins ---------------------------------------------------------------------- #
class Plugins(QObject):
  """ A dict-like class for handling plugins data """
  # ----------------------------------------- #
  def __init__(self):
    QObject.__init__(self)
    self.mPlugins = {}   # the dict of plugins (dicts)
    self.repoCache = {}  # the dict of lists of plugins (dicts)
    self.localCache = {} # the dict of plugins (dicts)
    self.obsoletePlugins = [] # the list of outdated 'user' plugins masking newer 'system' ones


  # ----------------------------------------- #
  def all(self):
    """ return all plugins """
    return self.mPlugins


  # ----------------------------------------- #
  def allUpgradeable(self):
    """ return all upgradeable plugins """
    result = {}
    for i in self.mPlugins:
      if self.mPlugins[i]["status"] == "upgradeable":
        result[i] = self.mPlugins[i]
    return result


  # ----------------------------------------- #
  def keyByUrl(self, name):
    """ return plugin key by given url """
    plugins = [i for i in self.mPlugins if self.mPlugins[i]["download_url"] == name]
    if plugins:
      return plugins[0]
    return None


  # ----------------------------------------- #
  def clearRepoCache(self):
    """ clears the repo cache before re-fetching repositories """
    self.repoCache = {}


  # ----------------------------------------- #
  def addFromRepository(self, plugin):
    """ add given plugin to the repoCache """
    repo = plugin["zip_repository"]
    try:
      self.repoCache[repo] += [plugin]
    except:
      self.repoCache[repo] = [plugin]


  # ----------------------------------------- #
  def removeInstalledPlugin(self, key):
    """ remove given plugin from the localCache """
    if self.localCache.has_key(key):
      del self.localCache[key]


  # ----------------------------------------- #
  def removeRepository(self, repo):
    """ remove whole repository from the repoCache """
    if self.repoCache.has_key(repo):
      del self.repoCache[repo]


  # ----------------------------------------- #
  def getInstalledPlugin(self, key, readOnly, testLoad=True):
    """ get the metadata of an installed plugin """
    def metadataParser(fct):
        """ plugin metadata parser reimplemented from qgis.utils
            for better control on wchich module is examined
            in case there is an installed plugin masking a core one """
        metadataFile = os.path.join(path, 'metadata.txt')
        if not os.path.exists(metadataFile):
          return "" # plugin has no metadata.txt file
        cp = ConfigParser.ConfigParser()
        try:
          cp.readfp(codecs.open(metadataFile, "r", "utf8"))
          return cp.get('general', fct)
        except:
          return ""

    def pluginMetadata(fct):
        """ calls metadataParser for current l10n.
            If failed, fallbacks to the standard metadata """
        locale = QLocale.system().name()
        if locale and fct in translatableAttributes:
          value = metadataParser( "%s[%s]" % (fct, locale ) )
          if value: return value
          value = metadataParser( "%s[%s]" % (fct, locale.split("_")[0] ) )
          if value: return value
        return metadataParser( fct )

    if readOnly:
      path = QDir.cleanPath( QgsApplication.pkgDataPath() ) + "/python/plugins/" + key
    else:
      path = QDir.cleanPath( QgsApplication.qgisSettingsDirPath() ) + "/python/plugins/" + key

    if not QDir(path).exists():
      return

    plugin = dict()
    error = ""
    errorDetails = ""

    version = normalizeVersion( pluginMetadata("version") )
    if version:
      qgisMinimumVersion = pluginMetadata("qgisMinimumVersion").strip()
      if not qgisMinimumVersion: qgisMinimumVersion = "0"
      qgisMaximumVersion = pluginMetadata("qgisMaximumVersion").strip()
      if not qgisMaximumVersion: qgisMaximumVersion = qgisMinimumVersion[0] + ".99"
      #if compatible, add the plugin to the list
      if not isCompatible(QGis.QGIS_VERSION, qgisMinimumVersion, qgisMaximumVersion):
        error = "incompatible"
        errorDetails = "%s - %s" % (qgisMinimumVersion, qgisMaximumVersion)
      elif testLoad:
        # only testLoad if compatible version
        try:
          exec "import %s" % key in globals(), locals()
          exec "reload (%s)" % key in globals(), locals()
          exec "%s.classFactory(iface)" % key in globals(), locals()
        except Exception, error:
          error = unicode(error.args[0])
    else:
        # seems there is no metadata.txt file. Maybe it's an old plugin for QGIS 1.x.
        version = "-1"
        error = "incompatible"
        errorDetails = "1.x"

    if error[:16] == "No module named ":
      mona = error.replace("No module named ","")
      if mona != key:
        error = "dependent"
        errorDetails = mona
    if not error in ["", "dependent", "incompatible"]:
      errorDetails = error
      error = "broken"

    icon = pluginMetadata("icon")
    if QFileInfo( icon ).isRelative():
      icon = path + "/" + icon;

    plugin = {
        "id"                : key,
        "name"              : pluginMetadata("name") or key,
        "description"       : pluginMetadata("description"),
        "about"             : pluginMetadata("about"),
        "icon"              : icon,
        "category"          : pluginMetadata("category"),
        "tags"              : pluginMetadata("tags"),
        "changelog"         : pluginMetadata("changelog"),
        "author_name"       : pluginMetadata("author_name") or pluginMetadata("author"),
        "author_email"      : pluginMetadata("email"),
        "homepage"          : pluginMetadata("homepage"),
        "tracker"           : pluginMetadata("tracker"),
        "code_repository"   : pluginMetadata("repository"),
        "version_installed" : version,
        "library"           : path,
        "pythonic"          : True,
        "experimental"      : pluginMetadata("experimental").strip().upper() in ["TRUE","YES"],
        "version_available" : "",
        "zip_repository"    : "",
        "download_url"      : path,      # warning: local path as url!
        "filename"          : "",
        "downloads"         : "",
        "average_vote"      : "",
        "rating_votes"      : "",
        "available"         : False,     # Will be overwritten, if any available version found.
        "installed"         : True,
        "status"            : "orphan",  # Will be overwritten, if any available version found.
        "error"             : error,
        "error_details"     : errorDetails,
        "readonly"          : readOnly }
    return plugin


  # ----------------------------------------- #
  def getAllInstalled(self, testLoad=True):
    """ Build the localCache """
    self.localCache = {}
    # first, try to add the readonly plugins...
    pluginsPath = unicode(QDir.convertSeparators(QDir.cleanPath(QgsApplication.pkgDataPath() + "/python/plugins")))
    #  temporarily add the system path as the first element to force loading the readonly plugins, even if masked by user ones.
    sys.path = [pluginsPath] + sys.path
    try:
      pluginDir = QDir(pluginsPath)
      pluginDir.setFilter(QDir.AllDirs)
      for key in pluginDir.entryList():
        key = unicode(key)
        if not key in [".",".."]:
          # only test those not yet loaded. Others proved they're o.k.
          self.localCache[key] = self.getInstalledPlugin(key, readOnly=True, testLoad=testLoad and not qgis.utils.plugins.has_key(key))
    except:
      # return QCoreApplication.translate("QgsPluginInstaller","Couldn't open the system plugin directory")
      pass # it's not necessary to stop due to this error
    # remove the temporarily added path
    sys.path.remove(pluginsPath)
    # ...then try to add locally installed ones
    try:
      pluginDir = QDir.convertSeparators(QDir.cleanPath(QgsApplication.qgisSettingsDirPath() + "/python/plugins"))
      pluginDir = QDir(pluginDir)
      pluginDir.setFilter(QDir.AllDirs)
    except:
      return QCoreApplication.translate("QgsPluginInstaller","Couldn't open the local plugin directory")
    for key in pluginDir.entryList():
      key = unicode(key)
      if not key in [".",".."]:
        # only test those not yet loaded. Others proved they're o.k.
        plugin = self.getInstalledPlugin(key, readOnly=False, testLoad=testLoad and not qgis.utils.plugins.has_key(key))
        if key in self.localCache.keys() and compareVersions(self.localCache[key]["version_installed"],plugin["version_installed"]) == 1:
          # An obsolete plugin in the "user" location is masking a newer one in the "system" location!
          self.obsoletePlugins += [key]
        self.localCache[key] = plugin


  # ----------------------------------------- #
  def rebuild(self):
    """ build or rebuild the mPlugins from the caches """
    self.mPlugins = {}
    for i in self.localCache.keys():
      self.mPlugins[i] = self.localCache[i].copy()
    settings = QSettings()
    allowExperimental = settings.value(settingsGroup+"/allowExperimental", False, type=bool)
    for i in self.repoCache.values():
      for j in i:
        plugin=j.copy() # do not update repoCache elements!
        key = plugin["id"]
        # check if the plugin is allowed and if there isn't any better one added already.
        if (allowExperimental or not plugin["experimental"]) \
        and not (self.mPlugins.has_key(key) and self.mPlugins[key]["version_available"] and compareVersions(self.mPlugins[key]["version_available"], plugin["version_available"]) < 2):
          # The mPlugins dict contains now locally installed plugins.
          # Now, add the available one if not present yet or update it if present already.
          if not self.mPlugins.has_key(key):
            self.mPlugins[key] = plugin   # just add a new plugin
          else:
            # update local plugin with remote metadata
            # description, about, icon: only use remote data if local one not available. Prefer local version because of i18n.
            # NOTE: don't prefer local name to not desynchronize names if repository doesn't support i18n.
            # Also prefer local icon to avoid downloading.
            for attrib in translatableAttributes + ["icon"]:
              if attrib != "name":
                if not self.mPlugins[key][attrib] and plugin[attrib]:
                    self.mPlugins[key][attrib] = plugin[attrib]
            # other remote metadata is preffered:
            for attrib in ["name", "description", "about", "category", "tags", "changelog", "author_name", "author_email", "homepage",
                           "tracker", "code_repository", "experimental", "version_available", "zip_repository",
                           "download_url", "filename", "downloads", "average_vote", "rating_votes"]:
              if ( not attrib in translatableAttributes ) or ( attrib == "name" ): # include name!
                if plugin[attrib]:
                    self.mPlugins[key][attrib] = plugin[attrib]
          # set status
          #
          # installed   available   status
          # ---------------------------------------
          # none        any         "not installed" (will be later checked if is "new")
          # any         none        "orphan"
          # same        same        "installed"
          # less        greater     "upgradeable"
          # greater     less        "newer"
          if not self.mPlugins[key]["version_available"]:
            self.mPlugins[key]["status"] = "orphan"
          elif not self.mPlugins[key]["version_installed"]:
            self.mPlugins[key]["status"] = "not installed"
          elif self.mPlugins[key]["version_installed"] in ["?", "-1"]:
            self.mPlugins[key]["status"] = "installed"
          elif compareVersions(self.mPlugins[key]["version_available"],self.mPlugins[key]["version_installed"]) == 0:
            self.mPlugins[key]["status"] = "installed"
          elif compareVersions(self.mPlugins[key]["version_available"],self.mPlugins[key]["version_installed"]) == 1:
            self.mPlugins[key]["status"] = "upgradeable"
          else:
            self.mPlugins[key]["status"] = "newer"
          # debug: test if the status match the "installed" tag:
          if self.mPlugins[key]["status"] in ["not installed"] and self.mPlugins[key]["installed"]:
              raise Exception("Error: plugin status is ambiguous (1)")
          if self.mPlugins[key]["status"] in ["installed","orphan","upgradeable","newer"] and not self.mPlugins[key]["installed"]:
              raise Exception("Error: plugin status is ambiguous (2)")
    self.markNews()


  # ----------------------------------------- #
  def markNews(self):
    """ mark all new plugins as new """
    settings = QSettings()
    seenPlugins = settings.value(seenPluginGroup, self.mPlugins.keys(), type=unicode)
    if len(seenPlugins) > 0:
      for i in self.mPlugins.keys():
        if seenPlugins.count(i) == 0 and self.mPlugins[i]["status"] == "not installed":
          self.mPlugins[i]["status"] = "new"


  # ----------------------------------------- #
  def updateSeenPluginsList(self):
    """ update the list of all seen plugins """
    settings = QSettings()
    seenPlugins = settings.value(seenPluginGroup, self.mPlugins.keys(), type=unicode)
    for i in self.mPlugins.keys():
      if seenPlugins.count(i) == 0:
        seenPlugins += [i]
    settings.setValue(seenPluginGroup, seenPlugins)


  # ----------------------------------------- #
  def isThereAnythingNew(self):
    """ return true if an upgradeable or new plugin detected """
    for i in self.mPlugins.values():
      if i["status"] in ["upgradeable","new"]:
        return True
    return False


# --- /class Plugins --------------------------------------------------------------------- #


# public instances:
repositories = Repositories()
plugins = Plugins()
