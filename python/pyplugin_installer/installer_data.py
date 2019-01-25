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

from qgis.PyQt.QtCore import (pyqtSignal, QObject, QCoreApplication, QFile,
                              QDir, QDirIterator, QDate, QUrl, QFileInfo,
                              QLocale, QByteArray)
from qgis.PyQt.QtXml import QDomDocument
from qgis.PyQt.QtNetwork import QNetworkRequest, QNetworkReply
from qgis.core import Qgis, QgsSettings, QgsNetworkRequestParameters
import sys
import os
import codecs
import re
import configparser
import qgis.utils
from qgis.core import QgsNetworkAccessManager, QgsApplication
from qgis.utils import iface, plugin_paths
from .version_compare import pyQgisVersion, compareVersions, normalizeVersion, isCompatible


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
    "deprecated" boolean,                       # true if deprecated, false if actual
    "trusted" boolean,                          # true if trusted, false if not trusted
    "version_available" unicode,                # available version
    "zip_repository" unicode,                   # the remote repository id
    "download_url" unicode,                     # url for downloading the plugin
    "filename" unicode,                         # the zip file name to be unzipped after downloaded
    "downloads" unicode,                        # number of downloads
    "average_vote" unicode,                     # average vote
    "rating_votes" unicode,                     # number of votes
}}
"""


translatableAttributes = ["name", "description", "about", "tags"]

settingsGroup = "app/plugin_installer"
reposGroup = "app/plugin_repositories"

officialRepo = (QCoreApplication.translate("QgsPluginInstaller", "QGIS Official Plugin Repository"), "https://plugins.qgis.org/plugins/plugins.xml")


# --- common functions ------------------------------------------------------------------- #
def removeDir(path):
    result = ""
    if not QFile(path).exists():
        result = QCoreApplication.translate("QgsPluginInstaller", "Nothing to remove! Plugin directory doesn't exist:") + "\n" + path
    elif QFile(path).remove():  # if it is only link, just remove it without resolving.
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
        result = QCoreApplication.translate("QgsPluginInstaller", "Failed to remove the directory:") + "\n" + path + "\n" + QCoreApplication.translate("QgsPluginInstaller", "Check permissions or remove it manually")
    # restore plugin directory if removed by QDir().rmpath()
    pluginDir = qgis.utils.home_plugin_path
    if not QDir(pluginDir).exists():
        QDir().mkpath(pluginDir)
    return result
# --- /common functions ------------------------------------------------------------------ #


# --- class Relay  ----------------------------------------------------------------------- #
class Relay(QObject):

    """ Relay object for transmitting signals from QPHttp with adding the repoName information """
    # ----------------------------------------- #
    anythingChanged = pyqtSignal(str, int, int)

    def __init__(self, key):
        QObject.__init__(self)
        self.key = key

    def stateChanged(self, state):
        self.anythingChanged.emit(self.key, state, 0)

    # ----------------------------------------- #
    def dataReadProgress(self, done, total):
        state = 4
        if total > 0:
            progress = int(float(done) / float(total) * 100)
        else:
            progress = 0
        self.anythingChanged.emit(self.key, state, progress)

# --- /class Relay  ---------------------------------------------------------------------- #


# --- class Repositories ----------------------------------------------------------------- #
class Repositories(QObject):

    """ A dict-like class for handling repositories data """
    # ----------------------------------------- #

    anythingChanged = pyqtSignal(str, int, int)
    repositoryFetched = pyqtSignal(str)
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
            return {self.mInspectionFilter: self.mRepositories[self.mInspectionFilter]}

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
                repos[self.mInspectionFilter] = self.mRepositories[self.mInspectionFilter]
            return repos

        for i in self.mRepositories:
            if self.mRepositories[i]["enabled"] and self.mRepositories[i]["valid"] and self.mRepositories[i]["state"] == 3:
                repos[i] = self.mRepositories[i]
        return repos

    # ----------------------------------------- #
    def urlParams(self):
        """ return GET parameters to be added to every request """
        # Strip down the point release segment from the version string
        return "?qgis={}".format(re.sub(r'\.\d*$', '', pyQgisVersion()))

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
        settings = QgsSettings()
        return settings.value(settingsGroup + "/checkOnStart", False, type=bool)

    # ----------------------------------------- #
    def setCheckingOnStart(self, state):
        """ set state of checking for news and updates """
        settings = QgsSettings()
        settings.setValue(settingsGroup + "/checkOnStart", state)

    # ----------------------------------------- #
    def checkingOnStartInterval(self):
        """ return checking for news and updates interval """
        settings = QgsSettings()
        try:
            # QgsSettings may contain non-int value...
            i = settings.value(settingsGroup + "/checkOnStartInterval", 1, type=int)
        except:
            # fallback do 1 day by default
            i = 1
        if i < 0:
            i = 1
        # allowed values: 0,1,3,7,14,30 days
        interval = 0
        for j in [1, 3, 7, 14, 30]:
            if i >= j:
                interval = j
        return interval

    # ----------------------------------------- #
    def setCheckingOnStartInterval(self, interval):
        """ set checking for news and updates interval """
        settings = QgsSettings()
        settings.setValue(settingsGroup + "/checkOnStartInterval", interval)

    # ----------------------------------------- #
    def saveCheckingOnStartLastDate(self):
        """ set today's date as the day of last checking  """
        settings = QgsSettings()
        settings.setValue(settingsGroup + "/checkOnStartLastDate", QDate.currentDate())

    # ----------------------------------------- #
    def timeForChecking(self):
        """ determine whether it's the time for checking for news and updates now """
        if self.checkingOnStartInterval() == 0:
            return True
        settings = QgsSettings()
        try:
            # QgsSettings may contain ivalid value...
            interval = settings.value(settingsGroup + "/checkOnStartLastDate", type=QDate).daysTo(QDate.currentDate())
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
        settings = QgsSettings()
        settings.beginGroup(reposGroup)
        # first, update repositories in QgsSettings if needed
        officialRepoPresent = False
        for key in settings.childGroups():
            url = settings.value(key + "/url", "", type=str)
            if url == officialRepo[1]:
                officialRepoPresent = True
        if not officialRepoPresent:
            settings.setValue(officialRepo[0] + "/url", officialRepo[1])

        for key in settings.childGroups():
            self.mRepositories[key] = {}
            self.mRepositories[key]["url"] = settings.value(key + "/url", "", type=str)
            self.mRepositories[key]["authcfg"] = settings.value(key + "/authcfg", "", type=str)
            self.mRepositories[key]["enabled"] = settings.value(key + "/enabled", True, type=bool)
            self.mRepositories[key]["valid"] = settings.value(key + "/valid", True, type=bool)
            self.mRepositories[key]["Relay"] = Relay(key)
            self.mRepositories[key]["xmlData"] = None
            self.mRepositories[key]["state"] = 0
            self.mRepositories[key]["error"] = ""
        settings.endGroup()

    # ----------------------------------------- #
    def requestFetching(self, key, url=None, redirectionCounter=0):
        """ start fetching the repository given by key """
        self.mRepositories[key]["state"] = 1
        if not url:
            url = QUrl(self.mRepositories[key]["url"] + self.urlParams())
        # v=str(Qgis.QGIS_VERSION_INT)
        # url.addQueryItem('qgis', '.'.join([str(int(s)) for s in [v[0], v[1:3]]]) ) # don't include the bugfix version!

        self.mRepositories[key]["QRequest"] = QNetworkRequest(url)
        self.mRepositories[key]["QRequest"].setAttribute(QNetworkRequest.Attribute(QgsNetworkRequestParameters.AttributeInitiatorClass), "Relay")
        authcfg = self.mRepositories[key]["authcfg"]
        if authcfg and isinstance(authcfg, str):
            if not QgsApplication.authManager().updateNetworkRequest(
                    self.mRepositories[key]["QRequest"], authcfg.strip()):
                msg = QCoreApplication.translate(
                    "QgsPluginInstaller",
                    "Update of network request with authentication "
                    "credentials FAILED for configuration '{0}'").format(authcfg)
                iface.pluginManagerInterface().pushMessage(msg, Qgis.Warning)
                self.mRepositories[key]["QRequest"] = None
                return
        self.mRepositories[key]["QRequest"].setAttribute(QNetworkRequest.User, key)
        self.mRepositories[key]["xmlData"] = QgsNetworkAccessManager.instance().get(self.mRepositories[key]["QRequest"])
        self.mRepositories[key]["xmlData"].setProperty('reposName', key)
        self.mRepositories[key]["xmlData"].setProperty('redirectionCounter', redirectionCounter)
        self.mRepositories[key]["xmlData"].downloadProgress.connect(self.mRepositories[key]["Relay"].dataReadProgress)
        self.mRepositories[key]["xmlData"].finished.connect(self.xmlDownloaded)

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
        if self.mRepositories[key]["state"] == 1 and self.mRepositories[key]["xmlData"] and self.mRepositories[key]["xmlData"].isRunning():
            self.mRepositories[key]["xmlData"].finished.disconnect()
            self.mRepositories[key]["xmlData"].abort()

    # ----------------------------------------- #
    def xmlDownloaded(self):
        """ populate the plugins object with the fetched data """
        reply = self.sender()
        reposName = reply.property('reposName')
        if reply.error() != QNetworkReply.NoError:                             # fetching failed
            self.mRepositories[reposName]["state"] = 3
            self.mRepositories[reposName]["error"] = reply.errorString()
            if reply.error() == QNetworkReply.OperationCanceledError:
                self.mRepositories[reposName]["error"] += "\n\n" + QCoreApplication.translate("QgsPluginInstaller", "If you haven't canceled the download manually, it was most likely caused by a timeout. In this case consider increasing the connection timeout value in QGIS options window.")
        elif reply.attribute(QNetworkRequest.HttpStatusCodeAttribute) == 301:
            redirectionUrl = reply.attribute(QNetworkRequest.RedirectionTargetAttribute)
            if redirectionUrl.isRelative():
                redirectionUrl = reply.url().resolved(redirectionUrl)
            redirectionCounter = reply.property('redirectionCounter') + 1
            if redirectionCounter > 4:
                self.mRepositories[reposName]["state"] = 3
                self.mRepositories[reposName]["error"] = QCoreApplication.translate("QgsPluginInstaller", "Too many redirections")
            else:
                # Fire a new request and exit immediately in order to quietly destroy the old one
                self.requestFetching(reposName, redirectionUrl, redirectionCounter)
                reply.deleteLater()
                return
        else:
            reposXML = QDomDocument()
            content = reply.readAll()
            # Fix lonely ampersands in metadata
            a = QByteArray()
            a.append("& ")
            b = QByteArray()
            b.append("&amp; ")
            content = content.replace(a, b)
            reposXML.setContent(content)
            pluginNodes = reposXML.elementsByTagName("pyqgis_plugin")
            if pluginNodes.size():
                for i in range(pluginNodes.size()):
                    fileName = pluginNodes.item(i).firstChildElement("file_name").text().strip()
                    if not fileName:
                        fileName = QFileInfo(pluginNodes.item(i).firstChildElement("download_url").text().strip().split("?")[0]).fileName()
                    name = fileName.partition(".")[0]
                    experimental = False
                    if pluginNodes.item(i).firstChildElement("experimental").text().strip().upper() in ["TRUE", "YES"]:
                        experimental = True
                    deprecated = False
                    if pluginNodes.item(i).firstChildElement("deprecated").text().strip().upper() in ["TRUE", "YES"]:
                        deprecated = True
                    trusted = False
                    if pluginNodes.item(i).firstChildElement("trusted").text().strip().upper() in ["TRUE", "YES"]:
                        trusted = True
                    icon = pluginNodes.item(i).firstChildElement("icon").text().strip()
                    if icon and not icon.startswith("http"):
                        icon = "http://{}/{}".format(QUrl(self.mRepositories[reposName]["url"]).host(), icon)

                    if pluginNodes.item(i).toElement().hasAttribute("plugin_id"):
                        plugin_id = pluginNodes.item(i).toElement().attribute("plugin_id")
                    else:
                        plugin_id = None

                    plugin = {
                        "id": name,
                        "plugin_id": plugin_id,
                        "name": pluginNodes.item(i).toElement().attribute("name"),
                        "version_available": pluginNodes.item(i).toElement().attribute("version"),
                        "description": pluginNodes.item(i).firstChildElement("description").text().strip(),
                        "about": pluginNodes.item(i).firstChildElement("about").text().strip(),
                        "author_name": pluginNodes.item(i).firstChildElement("author_name").text().strip(),
                        "homepage": pluginNodes.item(i).firstChildElement("homepage").text().strip(),
                        "download_url": pluginNodes.item(i).firstChildElement("download_url").text().strip(),
                        "category": pluginNodes.item(i).firstChildElement("category").text().strip(),
                        "tags": pluginNodes.item(i).firstChildElement("tags").text().strip(),
                        "changelog": pluginNodes.item(i).firstChildElement("changelog").text().strip(),
                        "author_email": pluginNodes.item(i).firstChildElement("author_email").text().strip(),
                        "tracker": pluginNodes.item(i).firstChildElement("tracker").text().strip(),
                        "code_repository": pluginNodes.item(i).firstChildElement("repository").text().strip(),
                        "downloads": pluginNodes.item(i).firstChildElement("downloads").text().strip(),
                        "average_vote": pluginNodes.item(i).firstChildElement("average_vote").text().strip(),
                        "rating_votes": pluginNodes.item(i).firstChildElement("rating_votes").text().strip(),
                        "icon": icon,
                        "experimental": experimental,
                        "deprecated": deprecated,
                        "trusted": trusted,
                        "filename": fileName,
                        "installed": False,
                        "available": True,
                        "status": "not installed",
                        "error": "",
                        "error_details": "",
                        "version_installed": "",
                        "zip_repository": reposName,
                        "library": "",
                        "readonly": False
                    }
                    qgisMinimumVersion = pluginNodes.item(i).firstChildElement("qgis_minimum_version").text().strip()
                    if not qgisMinimumVersion:
                        qgisMinimumVersion = "2"
                    qgisMaximumVersion = pluginNodes.item(i).firstChildElement("qgis_maximum_version").text().strip()
                    if not qgisMaximumVersion:
                        qgisMaximumVersion = qgisMinimumVersion[0] + ".99"
                    # if compatible, add the plugin to the list
                    if not pluginNodes.item(i).firstChildElement("disabled").text().strip().upper() in ["TRUE", "YES"]:
                        if isCompatible(pyQgisVersion(), qgisMinimumVersion, qgisMaximumVersion):
                            # add the plugin to the cache
                            plugins.addFromRepository(plugin)
                self.mRepositories[reposName]["state"] = 2
            else:
                # no plugin metadata found
                self.mRepositories[reposName]["state"] = 3
                if reply.attribute(QNetworkRequest.HttpStatusCodeAttribute) == 200:
                    self.mRepositories[reposName]["error"] = QCoreApplication.translate("QgsPluginInstaller", "Server response is 200 OK, but doesn't contain plugin metatada. This is most likely caused by a proxy or a wrong repository URL. You can configure proxy settings in QGIS options.")
                else:
                    self.mRepositories[reposName]["error"] = QCoreApplication.translate("QgsPluginInstaller", "Status code:") + " {} {}".format(
                        reply.attribute(QNetworkRequest.HttpStatusCodeAttribute),
                        reply.attribute(QNetworkRequest.HttpReasonPhraseAttribute)
                    )

        self.repositoryFetched.emit(reposName)

        # is the checking done?
        if not self.fetchingInProgress():
            self.checkingDone.emit()

        reply.deleteLater()

    # ----------------------------------------- #
    def inspectionFilter(self):
        """ return inspection filter (only one repository to be fetched) """
        return self.mInspectionFilter

    # ----------------------------------------- #
    def setInspectionFilter(self, key=None):
        """ temporarily disable all repositories but this for inspection """
        self.mInspectionFilter = key

# --- /class Repositories ---------------------------------------------------------------- #


# --- class Plugins ---------------------------------------------------------------------- #
class Plugins(QObject):

    """ A dict-like class for handling plugins data """
    # ----------------------------------------- #

    def __init__(self):
        QObject.__init__(self)
        self.mPlugins = {}         # the dict of plugins (dicts)
        self.repoCache = {}        # the dict of lists of plugins (dicts)
        self.localCache = {}       # the dict of plugins (dicts)
        self.obsoletePlugins = []  # the list of outdated 'user' plugins masking newer 'system' ones

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
        if key in self.localCache:
            del self.localCache[key]

    # ----------------------------------------- #
    def removeRepository(self, repo):
        """ remove whole repository from the repoCache """
        if repo in self.repoCache:
            del self.repoCache[repo]

    # ----------------------------------------- #
    def getInstalledPlugin(self, key, path, readOnly):
        """ get the metadata of an installed plugin """
        def metadataParser(fct):
            """ plugin metadata parser reimplemented from qgis.utils
                for better control of which module is examined
                in case there is an installed plugin masking a core one """
            global errorDetails
            cp = configparser.ConfigParser()
            try:
                with codecs.open(metadataFile, "r", "utf8") as f:
                    cp.read_file(f)
                return cp.get('general', fct)
            except Exception as e:
                if not errorDetails:
                    errorDetails = e.args[0]  # set to the first problem
                return ""

        def pluginMetadata(fct):
            """ calls metadataParser for current l10n.
                If failed, fallbacks to the standard metadata """
            locale = QLocale.system().name()
            if locale and fct in translatableAttributes:
                value = metadataParser("{}[{}]".format(fct, locale))
                if value:
                    return value
                value = metadataParser("{}[{}]".format(fct, locale.split("_")[0]))
                if value:
                    return value
            return metadataParser(fct)

        if not QDir(path).exists():
            return

        global errorDetails  # to communicate with the metadataParser fn
        plugin = dict()
        error = ""
        errorDetails = ""
        version = None

        if not os.path.exists(os.path.join(path, '__init__.py')):
            error = "broken"
            errorDetails = QCoreApplication.translate("QgsPluginInstaller", "Missing __init__.py")

        metadataFile = os.path.join(path, 'metadata.txt')
        if os.path.exists(metadataFile):
            version = normalizeVersion(pluginMetadata("version"))

        if version:
            qgisMinimumVersion = pluginMetadata("qgisMinimumVersion").strip()
            if not qgisMinimumVersion:
                qgisMinimumVersion = "0"
            qgisMaximumVersion = pluginMetadata("qgisMaximumVersion").strip()
            if not qgisMaximumVersion:
                qgisMaximumVersion = qgisMinimumVersion[0] + ".99"
            # if compatible, add the plugin to the list
            if not isCompatible(pyQgisVersion(), qgisMinimumVersion, qgisMaximumVersion):
                error = "incompatible"
                errorDetails = "{} - {}".format(qgisMinimumVersion, qgisMaximumVersion)
        elif not os.path.exists(metadataFile):
            error = "broken"
            errorDetails = QCoreApplication.translate("QgsPluginInstaller", "Missing metadata file")
        else:
            error = "broken"
            e = errorDetails
            errorDetails = QCoreApplication.translate("QgsPluginInstaller", "Error reading metadata")
            if e:
                errorDetails += ": " + e

        if not version:
            version = "?"

        if error[:16] == "No module named ":
            mona = error.replace("No module named ", "")
            if mona != key:
                error = "dependent"
                errorDetails = mona

        icon = pluginMetadata("icon")
        if QFileInfo(icon).isRelative():
            icon = path + "/" + icon

        changelog = pluginMetadata("changelog")
        changelogFile = os.path.join(path, "CHANGELOG")
        if not changelog and QFile(changelogFile).exists():
            with open(changelogFile) as f:
                changelog = f.read()

        plugin = {
            "id": key,
            "plugin_id": None,
            "name": pluginMetadata("name") or key,
            "description": pluginMetadata("description"),
            "about": pluginMetadata("about"),
            "icon": icon,
            "category": pluginMetadata("category"),
            "tags": pluginMetadata("tags"),
            "changelog": changelog,
            "author_name": pluginMetadata("author_name") or pluginMetadata("author"),
            "author_email": pluginMetadata("email"),
            "homepage": pluginMetadata("homepage"),
            "tracker": pluginMetadata("tracker"),
            "code_repository": pluginMetadata("repository"),
            "version_installed": version,
            "library": path,
            "pythonic": True,
            "experimental": pluginMetadata("experimental").strip().upper() in ["TRUE", "YES"],
            "deprecated": pluginMetadata("deprecated").strip().upper() in ["TRUE", "YES"],
            "trusted": False,
            "version_available": "",
            "zip_repository": "",
            "download_url": path,      # warning: local path as url!
            "filename": "",
            "downloads": "",
            "average_vote": "",
            "rating_votes": "",
            "available": False,     # Will be overwritten, if any available version found.
            "installed": True,
            "status": "orphan",  # Will be overwritten, if any available version found.
            "error": error,
            "error_details": errorDetails,
            "readonly": readOnly}
        return plugin

    # ----------------------------------------- #
    def getAllInstalled(self):
        """ Build the localCache """
        self.localCache = {}

        # reversed list of the plugin paths: first system plugins -> then user plugins -> finally custom path(s)
        pluginPaths = list(plugin_paths)
        pluginPaths.reverse()

        for pluginsPath in pluginPaths:
            isTheSystemDir = (pluginPaths.index(pluginsPath) == 0)  # The current dir is the system plugins dir
            if isTheSystemDir:
                # temporarily add the system path as the first element to force loading the readonly plugins, even if masked by user ones.
                sys.path = [pluginsPath] + sys.path
            try:
                pluginDir = QDir(pluginsPath)
                pluginDir.setFilter(QDir.AllDirs)
                for key in pluginDir.entryList():
                    if key not in [".", ".."]:
                        path = QDir.toNativeSeparators(pluginsPath + "/" + key)
                        # readOnly = not QFileInfo(pluginsPath).isWritable() # On windows testing the writable status isn't reliable.
                        readOnly = isTheSystemDir                            # Assume only the system plugins are not writable.
                        # failedToLoad = settings.value("/PythonPlugins/watchDog/" + key) is not None
                        plugin = self.getInstalledPlugin(key, path=path, readOnly=readOnly)
                        if key in list(self.localCache.keys()) and compareVersions(self.localCache[key]["version_installed"], plugin["version_installed"]) == 1:
                            # An obsolete plugin in the "user" location is masking a newer one in the "system" location!
                            self.obsoletePlugins += [key]
                        self.localCache[key] = plugin
            except:
                # it's not necessary to stop if one of the dirs is inaccessible
                pass

            if isTheSystemDir:
                # remove the temporarily added path
                sys.path.remove(pluginsPath)

    # ----------------------------------------- #
    def rebuild(self):
        """ build or rebuild the mPlugins from the caches """
        self.mPlugins = {}
        for i in list(self.localCache.keys()):
            self.mPlugins[i] = self.localCache[i].copy()
        settings = QgsSettings()
        allowExperimental = settings.value(settingsGroup + "/allowExperimental", False, type=bool)
        allowDeprecated = settings.value(settingsGroup + "/allowDeprecated", False, type=bool)
        for i in list(self.repoCache.values()):
            for j in i:
                plugin = j.copy()  # do not update repoCache elements!
                key = plugin["id"]
                # check if the plugin is allowed and if there isn't any better one added already.
                if (allowExperimental or not plugin["experimental"]) \
                   and (allowDeprecated or not plugin["deprecated"]) \
                   and not (key in self.mPlugins and self.mPlugins[key]["version_available"] and compareVersions(self.mPlugins[key]["version_available"], plugin["version_available"]) < 2):
                    # The mPlugins dict contains now locally installed plugins.
                    # Now, add the available one if not present yet or update it if present already.
                    if key not in self.mPlugins:
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
                        # other remote metadata is preferred:
                        for attrib in ["name", "plugin_id", "description", "about", "category", "tags", "changelog", "author_name", "author_email", "homepage",
                                       "tracker", "code_repository", "experimental", "deprecated", "version_available", "zip_repository",
                                       "download_url", "filename", "downloads", "average_vote", "rating_votes", "trusted"]:
                            if attrib not in translatableAttributes or attrib == "name":  # include name!
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
                    elif compareVersions(self.mPlugins[key]["version_available"], self.mPlugins[key]["version_installed"]) == 0:
                        self.mPlugins[key]["status"] = "installed"
                    elif compareVersions(self.mPlugins[key]["version_available"], self.mPlugins[key]["version_installed"]) == 1:
                        self.mPlugins[key]["status"] = "upgradeable"
                    else:
                        self.mPlugins[key]["status"] = "newer"
                    # debug: test if the status match the "installed" tag:
                    if self.mPlugins[key]["status"] in ["not installed"] and self.mPlugins[key]["installed"]:
                        raise Exception("Error: plugin status is ambiguous (1)")
                    if self.mPlugins[key]["status"] in ["installed", "orphan", "upgradeable", "newer"] and not self.mPlugins[key]["installed"]:
                        raise Exception("Error: plugin status is ambiguous (2)")
        self.markNews()

    # ----------------------------------------- #
    def markNews(self):
        """ mark all new plugins as new """
        settings = QgsSettings()
        seenPlugins = settings.value(settingsGroup + '/seen_plugins', list(self.mPlugins.keys()), type=str)
        if len(seenPlugins) > 0:
            for i in list(self.mPlugins.keys()):
                if seenPlugins.count(i) == 0 and self.mPlugins[i]["status"] == "not installed":
                    self.mPlugins[i]["status"] = "new"

    # ----------------------------------------- #
    def updateSeenPluginsList(self):
        """ update the list of all seen plugins """
        settings = QgsSettings()
        seenPlugins = settings.value(settingsGroup + '/seen_plugins', list(self.mPlugins.keys()), type=str)
        for i in list(self.mPlugins.keys()):
            if seenPlugins.count(i) == 0:
                seenPlugins += [i]
        settings.setValue(settingsGroup + '/seen_plugins', seenPlugins)

    # ----------------------------------------- #
    def isThereAnythingNew(self):
        """ return true if an upgradeable or new plugin detected """
        for i in list(self.mPlugins.values()):
            if i["status"] in ["upgradeable", "new"]:
                return True
        return False


# --- /class Plugins --------------------------------------------------------------------- #


# public instances:
repositories = Repositories()
plugins = Plugins()
