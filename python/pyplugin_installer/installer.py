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

import os
import json
import zipfile
from functools import partial

from qgis.PyQt import sip
from qgis.PyQt.QtCore import Qt, QObject, QDir, QUrl, QFileInfo, QFile
from qgis.PyQt.QtWidgets import (
    QApplication,
    QDialog,
    QDialogButtonBox,
    QFrame,
    QMessageBox,
    QLabel,
    QVBoxLayout,
    QPushButton
)
from qgis.PyQt.QtNetwork import QNetworkRequest

from qgis.core import Qgis, QgsApplication, QgsMessageLog, QgsNetworkAccessManager, QgsSettings, QgsSettingsTree, QgsNetworkRequestParameters
from qgis.gui import QgsMessageBar, QgsPasswordLineEdit, QgsHelp
from qgis.utils import (
    iface,
    startPlugin,
    unloadPlugin,
    loadPlugin,
    OverrideCursor,
    updateAvailablePlugins,
    plugins_metadata_parser,
    isPluginLoaded,
    HOME_PLUGIN_PATH
)
from .installer_data import (repositories, plugins, officialRepo,
                             reposGroup, removeDir)
from .qgsplugininstallerinstallingdialog import QgsPluginInstallerInstallingDialog
from .qgsplugininstallerpluginerrordialog import QgsPluginInstallerPluginErrorDialog
from .qgsplugininstallerfetchingdialog import QgsPluginInstallerFetchingDialog
from .qgsplugininstallerrepositorydialog import QgsPluginInstallerRepositoryDialog
from .unzip import unzip
from .plugindependencies import find_dependencies
from .qgsplugindependenciesdialog import QgsPluginDependenciesDialog


# public instances:
pluginInstaller = None


def initPluginInstaller():
    global pluginInstaller
    pluginInstaller = QgsPluginInstaller()


# -------------------------------------------------------- #
class QgsPluginInstaller(QObject):

    """ The main class for managing the plugin installer stuff"""

    # ----------------------------------------- #
    def __init__(self):
        """ Initialize data objects, starts fetching if appropriate, and warn about/removes obsolete plugins """

        QObject.__init__(self)  # initialize QObject in order to to use self.tr()
        repositories.load()
        plugins.getAllInstalled()

        self.message_bar_widget = None

        if repositories.checkingOnStart() and repositories.timeForChecking() and repositories.allEnabled():
            # start fetching repositories
            repositories.checkingDone.connect(self.checkingDone)
            for key in repositories.allEnabled():
                repositories.requestFetching(key)
        else:
            # no fetching at start, so mark all enabled repositories as requesting to be fetched.
            for key in repositories.allEnabled():
                repositories.setRepositoryData(key, "state", 3)

        # look for obsolete plugins updates (the user-installed one is older than the core one)
        for key in plugins.obsoletePlugins:
            plugin = plugins.localCache[key]
            msg = QMessageBox()
            msg.setIcon(QMessageBox.Icon.Warning)
            msg.setWindowTitle(self.tr("QGIS Python Plugin Installer"))
            msg.addButton(self.tr("Uninstall (recommended)"), QMessageBox.ButtonRole.AcceptRole)
            msg.addButton(self.tr("I will uninstall it later"), QMessageBox.ButtonRole.RejectRole)
            msg.setText("%s <b>%s</b><br/><br/>%s" % (self.tr("Obsolete plugin:"), plugin["name"], self.tr("QGIS has detected an obsolete plugin that masks its more recent version shipped with this copy of QGIS. This is likely due to files associated with a previous installation of QGIS. Do you want to remove the old plugin right now and unmask the more recent version?")))
            msg.exec()
            if not msg.result():
                settings = QgsSettings()
                plugin_is_active = settings.value("/PythonPlugins/" + key, False, type=bool)

                # uninstall the update, update utils and reload if enabled
                self.uninstallPlugin(key, quiet=True)
                updateAvailablePlugins()
                if plugin_is_active:
                    settings.setValue("/PythonPlugins/watchDog/" + key, True)
                    loadPlugin(key)
                    startPlugin(key)
                    settings.remove("/PythonPlugins/watchDog/" + key)

    # ----------------------------------------- #
    def fetchAvailablePlugins(self, reloadMode):
        """ Fetch plugins from all enabled repositories."""
        """  reloadMode = true:  Fully refresh data from QgsSettings to mRepositories  """
        """  reloadMode = false: Fetch unready repositories only """
        if reloadMode:
            repositories.load()
            plugins.clearRepoCache()
            plugins.getAllInstalled()

        for key in repositories.allEnabled():
            if reloadMode or repositories.all()[key]["state"] == 3:  # if state = 3 (error or not fetched yet), try to fetch once again
                repositories.requestFetching(key, force_reload=reloadMode)

        if repositories.fetchingInProgress():
            fetchDlg = QgsPluginInstallerFetchingDialog(iface.mainWindow())
            fetchDlg.exec()
            del fetchDlg
            for key in repositories.all():
                repositories.killConnection(key)

        # display error messages for every unavailable repository, unless Shift pressed nor all repositories are unavailable
        keepQuiet = QgsApplication.keyboardModifiers() == Qt.KeyboardModifiers(Qt.KeyboardModifier.ShiftModifier)
        if repositories.allUnavailable() and repositories.allUnavailable() != repositories.allEnabled():
            for key in repositories.allUnavailable():
                if not keepQuiet:
                    QMessageBox.warning(iface.mainWindow(), self.tr("QGIS Python Plugin Installer"), self.tr("Error reading repository:") + " " + key + "\n\n" + repositories.all()[key]["error"])
                if QgsApplication.keyboardModifiers() == Qt.KeyboardModifiers(Qt.KeyboardModifier.ShiftModifier):
                    keepQuiet = True
        # finally, rebuild plugins from the caches
        plugins.rebuild()

    # ----------------------------------------- #
    def checkingDone(self):
        """ Remove the "Looking for new plugins..." label and display a notification instead if any updates available """

        # rebuild plugins cache
        plugins.rebuild()
        # look for news in the repositories
        plugins.markNews()
        # then check for updates (and eventually overwrite status)
        updatable_plugin_names = []
        for _, properties in plugins.all().items():
            if properties["status"] == "upgradeable":
                updatable_plugin_names.append(properties["name"])

        if not updatable_plugin_names:
            return

        if len(updatable_plugin_names) >= 2:
            status = self.tr("Multiple plugin updates are available")
        else:
            status = self.tr("An update to the {} plugin is available").format(updatable_plugin_names[0])

        QgsMessageLog.logMessage(
            "Plugin update(s) available : {}".format(','.join(updatable_plugin_names)), self.tr("Plugins"))

        bar = iface.messageBar()
        self.message_bar_widget = bar.createMessage('', status)
        update_button = QPushButton(self.tr("Install Updates…"))
        tab_index = 3  # PLUGMAN_TAB_UPGRADEABLE
        update_button.pressed.connect(partial(self.showPluginManagerWhenReady, tab_index))
        self.message_bar_widget.layout().addWidget(update_button)
        bar.pushWidget(self.message_bar_widget, Qgis.MessageLevel.Info)

    # ----------------------------------------- #
    def exportRepositoriesToManager(self):
        """ Update manager's repository tree widget with current data """
        iface.pluginManagerInterface().clearRepositoryList()
        for key in repositories.all():
            url = repositories.all()[key]["url"] + repositories.urlParams()
            if repositories.inspectionFilter():
                enabled = (key == repositories.inspectionFilter())
            else:
                enabled = repositories.all()[key]["enabled"]
            iface.pluginManagerInterface().addToRepositoryList({
                "name": key,
                "url": url,
                "enabled": enabled and "true" or "false",
                "valid": repositories.all()[key]["valid"] and "true" or "false",
                "state": str(repositories.all()[key]["state"]),
                "error": repositories.all()[key]["error"],
                "inspection_filter": repositories.inspectionFilter() and "true" or "false"
            })

    # ----------------------------------------- #
    def exportPluginsToManager(self):
        """ Insert plugins metadata to QgsMetadataRegistry """
        iface.pluginManagerInterface().clearPythonPluginMetadata()
        for key in plugins.all():
            plugin = plugins.all()[key]
            iface.pluginManagerInterface().addPluginMetadata({
                "id": key,
                "plugin_id": plugin["plugin_id"] or "",
                "name": plugin["name"],
                "description": plugin["description"],
                "about": plugin["about"],
                "category": plugin["category"],
                "tags": plugin["tags"],
                "changelog": plugin["changelog"],
                "author_name": plugin["author_name"],
                "author_email": plugin["author_email"],
                "homepage": plugin["homepage"],
                "tracker": plugin["tracker"],
                "code_repository": plugin["code_repository"],
                "version_installed": plugin["version_installed"],
                "library": plugin["library"],
                "icon": plugin["icon"],
                "readonly": plugin["readonly"] and "true" or "false",
                "installed": plugin["installed"] and "true" or "false",
                "available": plugin["available"] and "true" or "false",
                "status": plugin["status"],
                "status_exp": plugin["status_exp"],
                "error": plugin["error"],
                "error_details": plugin["error_details"],
                "create_date": plugin["create_date"],
                "update_date": plugin["update_date"],
                "create_date_stable": plugin["create_date_stable"],
                "update_date_stable": plugin["update_date_stable"],
                "create_date_experimental": plugin["create_date_experimental"],
                "update_date_experimental": plugin["update_date_experimental"],
                "experimental": plugin["experimental"] and "true" or "false",
                "deprecated": plugin["deprecated"] and "true" or "false",
                "trusted": plugin["trusted"] and "true" or "false",
                "version_available": plugin["version_available"],
                "version_available_stable": plugin["version_available_stable"] or "",
                "version_available_experimental": plugin["version_available_experimental"] or "",
                "zip_repository": plugin["zip_repository"],
                "download_url": plugin["download_url"],
                "download_url_stable": plugin["download_url_stable"],
                "download_url_experimental": plugin["download_url_experimental"],
                "filename": plugin["filename"],
                "downloads": plugin["downloads"],
                "average_vote": plugin["average_vote"],
                "rating_votes": plugin["rating_votes"],
                "plugin_dependencies": plugin.get("plugin_dependencies", None),
                "pythonic": "true"
            })
        iface.pluginManagerInterface().reloadModel()

    # ----------------------------------------- #
    def reloadAndExportData(self):
        """ Reload All repositories and export data to the Plugin Manager """
        self.fetchAvailablePlugins(reloadMode=True)
        self.exportRepositoriesToManager()
        self.exportPluginsToManager()

    # ----------------------------------------- #
    def showPluginManagerWhenReady(self, * params):
        """ Open the plugin manager window. If fetching is still in progress, it shows the progress window first """
        """ Optionally pass the index of tab to be opened in params """
        if self.message_bar_widget:
            if not sip.isdeleted(self.message_bar_widget):
                iface.messageBar().popWidget(self.message_bar_widget)
            self.message_bar_widget = None

        self.fetchAvailablePlugins(reloadMode=False)
        self.exportRepositoriesToManager()
        self.exportPluginsToManager()

        # finally, show the plugin manager window
        tabIndex = -1
        if len(params) == 1:
            indx = str(params[0])
            if indx.isdigit() and int(indx) > -1 and int(indx) < 7:
                tabIndex = int(indx)
        iface.pluginManagerInterface().showPluginManager(tabIndex)

    # ----------------------------------------- #
    def onManagerClose(self):
        """ Call this method when closing manager window - it resets last-use-dependent values. """
        plugins.updateSeenPluginsList()
        repositories.saveCheckingOnStartLastDate()

    # ----------------------------------------- #
    def exportSettingsGroup(self):
        """ Return QgsSettings settingsGroup value """
        # todo QGIS 4 remove
        return "plugin-manager"

    # ----------------------------------------- #
    def upgradeAllUpgradeable(self):
        """ Reinstall all upgradeable plugins """
        for key in plugins.allUpgradeable():
            self.installPlugin(key, quiet=True)

    # ----------------------------------------- #
    def installPlugin(self, key, quiet=False, stable=True):
        """ Install given plugin """
        error = False
        status_key = 'status' if stable else 'status_exp'
        infoString = ('', '')
        plugin = plugins.all()[key]
        previousStatus = plugin[status_key]
        if not plugin:
            return
        if plugin[status_key] == "newer" and not plugin["error"]:  # ask for confirmation if user downgrades an usable plugin
            if QMessageBox.warning(iface.mainWindow(), self.tr("QGIS Python Plugin Installer"), self.tr("Are you sure you want to downgrade the plugin to the latest available version? The installed one is newer!"), QMessageBox.StandardButton.Yes, QMessageBox.StandardButton.No) == QMessageBox.StandardButton.No:
                return

        # if plugin is active, unload it before update, see https://github.com/qgis/QGIS/issues/54968
        pluginWasLoaded = isPluginLoaded(plugin["id"])
        if pluginWasLoaded:
            unloadPlugin(plugin["id"])

        dlg = QgsPluginInstallerInstallingDialog(iface.mainWindow(), plugin, stable=stable)
        dlg.exec()

        plugin_path = HOME_PLUGIN_PATH + "/" + key
        if dlg.result():
            error = True
            infoString = (self.tr("Plugin installation failed"), dlg.result())
            # download failed or aborted. If plugin was active before the update, let's try to load it back
            if pluginWasLoaded and loadPlugin(plugin["id"]):
                startPlugin(plugin["id"])
        elif not QDir(plugin_path).exists():
            error = True
            infoString = (
                self.tr("Plugin has disappeared"),
                self.tr(
                    "The plugin seems to have been installed but it's not possible to know where. The directory \"{}\" "
                    "has not been found. Probably the plugin package contained a wrong named directory.\nPlease search "
                    "the list of installed plugins. You should find the plugin there, but it's not possible to "
                    "determine which of them it is and it's also not possible to inform you about available updates. "
                    "Please contact the plugin author and submit this issue.").format(plugin_path))
            with OverrideCursor(Qt.CursorShape.WaitCursor):
                plugins.getAllInstalled()
                plugins.rebuild()
                self.exportPluginsToManager()
        else:
            QApplication.setOverrideCursor(Qt.CursorShape.WaitCursor)
            # update the list of plugins in plugin handling routines
            updateAvailablePlugins()
            self.processDependencies(plugin["id"])
            # try to load the plugin
            loadPlugin(plugin["id"])
            plugins.getAllInstalled()
            plugins.rebuild()
            plugin = plugins.all()[key]
            if not plugin["error"]:
                if previousStatus in ["not installed", "new"]:
                    infoString = (self.tr("Plugin installed successfully"), "")
                    if startPlugin(plugin["id"]):
                        settings = QgsSettings()
                        settings.setValue("/PythonPlugins/" + plugin["id"], True)
                else:
                    infoString = (self.tr("Plugin reinstalled successfully"), "")
                    if pluginWasLoaded:
                        loadPlugin(plugin["id"])
                        startPlugin(plugin["id"])
                    else:
                        unloadPlugin(key)  # Just for a case. Will exit quietly if really not loaded
                        loadPlugin(key)
                if quiet:
                    infoString = (None, None)
                QApplication.restoreOverrideCursor()
            else:
                QApplication.restoreOverrideCursor()
                if plugin["error"] == "incompatible":
                    message = self.tr("The plugin is not compatible with this version of QGIS. It's designed for QGIS versions:")
                    message += " <b>" + plugin["error_details"] + "</b>"
                elif plugin["error"] == "dependent":
                    message = self.tr("The plugin depends on some components missing on your system. You need to install the following Python module in order to enable it:")
                    message += "<b> " + plugin["error_details"] + "</b>"
                else:
                    message = self.tr("The plugin is broken. Python said:")
                    message += "<br><b>" + plugin["error_details"] + "</b>"
                dlg = QgsPluginInstallerPluginErrorDialog(iface.mainWindow(), message)
                dlg.exec()
                if dlg.result():
                    # revert installation
                    pluginDir = HOME_PLUGIN_PATH + "/" + plugin["id"]
                    result = removeDir(pluginDir)
                    if QDir(pluginDir).exists():
                        error = True
                        infoString = (self.tr("Plugin uninstall failed"), result)
                        try:
                            exec("sys.path_importer_cache.clear()")
                            exec("import %s" % plugin["id"])
                            exec("reload (%s)" % plugin["id"])
                        except:
                            pass
                    else:
                        try:
                            exec("del sys.modules[%s]" % plugin["id"])
                        except:
                            pass
                    plugins.getAllInstalled()
                    plugins.rebuild()

            self.exportPluginsToManager()

        if infoString[0]:
            level = error and Qgis.MessageLevel.Critical or Qgis.MessageLevel.Info
            msg = "<b>%s</b>" % infoString[0]
            if infoString[1]:
                msg += "<b>:</b> %s" % infoString[1]
            iface.pluginManagerInterface().pushMessage(msg, level)

    # ----------------------------------------- #
    def uninstallPlugin(self, key, quiet=False):
        """ Uninstall given plugin """
        if key in plugins.all():
            plugin = plugins.all()[key]
        else:
            plugin = plugins.localCache[key]
        if not plugin:
            return
        if not quiet:
            warning = self.tr("Are you sure you want to uninstall the following plugin?") + "\n(" + plugin["name"] + ")"
            if plugin["status"] == "orphan" and plugin["status_exp"] == "orphan" and not plugin["error"]:
                warning += "\n\n" + self.tr("Warning: this plugin isn't available in any accessible repository!")
            if QMessageBox.warning(iface.mainWindow(), self.tr("QGIS Python Plugin Installer"), warning, QMessageBox.StandardButton.Yes, QMessageBox.StandardButton.No) == QMessageBox.StandardButton.No:
                return
        # unload the plugin
        QApplication.setOverrideCursor(Qt.CursorShape.WaitCursor)
        try:
            unloadPlugin(key)
        except:
            pass
        pluginDir = HOME_PLUGIN_PATH + "/" + plugin["id"]
        result = removeDir(pluginDir)
        if result:
            QApplication.restoreOverrideCursor()
            msg = "<b>%s:</b>%s" % (self.tr("Plugin uninstall failed"), result)
            iface.pluginManagerInterface().pushMessage(msg, Qgis.MessageLevel.Critical)
        else:
            # safe remove
            try:
                unloadPlugin(plugin["id"])
            except:
                pass
            try:
                exec("plugins[%s].unload()" % plugin["id"])
                exec("del plugins[%s]" % plugin["id"])
            except:
                pass
            try:
                exec("del sys.modules[%s]" % plugin["id"])
            except:
                pass
            try:
                exec("del plugins_metadata_parser[%s]" % plugin["id"])
            except:
                pass

            plugins.getAllInstalled()
            plugins.rebuild()
            self.exportPluginsToManager()
            QApplication.restoreOverrideCursor()
            iface.pluginManagerInterface().pushMessage(self.tr("Plugin uninstalled successfully"), Qgis.MessageLevel.Success)

            settings = QgsSettings()
            settings.remove("/PythonPlugins/" + key)

    # ----------------------------------------- #
    def addRepository(self):
        """ add new repository connection """
        dlg = QgsPluginInstallerRepositoryDialog(iface.mainWindow())
        dlg.editParams.setText(repositories.urlParams())
        dlg.checkBoxEnabled.setCheckState(Qt.CheckState.Checked)
        if not dlg.exec():
            return
        for i in list(repositories.all().values()):
            if dlg.editURL.text().strip() == i["url"]:
                iface.pluginManagerInterface().pushMessage(self.tr("Unable to add another repository with the same URL!"), Qgis.MessageLevel.Warning)
                return
        settings = QgsSettings()
        settings.beginGroup(reposGroup)
        reposName = dlg.editName.text()
        reposURL = dlg.editURL.text().strip()
        if reposName in repositories.all():
            reposName = reposName + "(2)"
        # add to settings
        settings.setValue(reposName + "/url", reposURL)
        settings.setValue(reposName + "/authcfg", dlg.editAuthCfg.text().strip())
        settings.setValue(reposName + "/enabled", bool(dlg.checkBoxEnabled.checkState()))
        # refresh lists and populate widgets
        plugins.removeRepository(reposName)
        self.reloadAndExportData()

    # ----------------------------------------- #
    def editRepository(self, reposName):
        """ edit repository connection """
        if not reposName:
            return
        checkState = {False: Qt.CheckState.Unchecked, True: Qt.CheckState.Checked}
        dlg = QgsPluginInstallerRepositoryDialog(iface.mainWindow())
        dlg.editName.setText(reposName)
        dlg.editURL.setText(repositories.all()[reposName]["url"])
        dlg.editAuthCfg.setText(repositories.all()[reposName]["authcfg"])
        dlg.editParams.setText(repositories.urlParams())
        dlg.checkBoxEnabled.setCheckState(checkState[repositories.all()[reposName]["enabled"]])
        if repositories.all()[reposName]["valid"]:
            dlg.checkBoxEnabled.setEnabled(True)
            dlg.labelInfo.setText("")
        else:
            dlg.checkBoxEnabled.setEnabled(False)
            dlg.labelInfo.setText(self.tr("This repository is blocked due to incompatibility with your QGIS version"))
            dlg.labelInfo.setFrameShape(QFrame.Shape.Box)
        if not dlg.exec():
            return  # nothing to do if canceled
        for i in list(repositories.all().values()):
            if dlg.editURL.text().strip() == i["url"] and dlg.editURL.text().strip() != repositories.all()[reposName]["url"]:
                iface.pluginManagerInterface().pushMessage(self.tr("Unable to add another repository with the same URL!"), Qgis.MessageLevel.Warning)
                return
        # delete old repo from QgsSettings and create new one
        settings = QgsSettings()
        settings.beginGroup(reposGroup)
        settings.remove(reposName)
        newName = dlg.editName.text()
        if newName in repositories.all() and newName != reposName:
            newName = newName + "(2)"
        settings.setValue(newName + "/url", dlg.editURL.text().strip())
        settings.setValue(newName + "/authcfg", dlg.editAuthCfg.text().strip())
        settings.setValue(newName + "/enabled", bool(dlg.checkBoxEnabled.checkState()))
        if dlg.editAuthCfg.text().strip() != repositories.all()[reposName]["authcfg"]:
            repositories.all()[reposName]["authcfg"] = dlg.editAuthCfg.text().strip()
        if dlg.editURL.text().strip() == repositories.all()[reposName]["url"] and dlg.checkBoxEnabled.checkState() == checkState[repositories.all()[reposName]["enabled"]]:
            repositories.rename(reposName, newName)
            self.exportRepositoriesToManager()
            return  # nothing else to do if only repository name was changed
        plugins.removeRepository(reposName)
        self.reloadAndExportData()

    # ----------------------------------------- #
    def deleteRepository(self, reposName: str):
        """ delete repository connection """
        if not reposName:
            return
        settings = QgsSettings()
        settings.beginGroup(reposGroup)
        if settings.value(reposName + "/url", "", type=str) == officialRepo[1]:
            iface.pluginManagerInterface().pushMessage(self.tr("You can't remove the official QGIS Plugin Repository. You can disable it if needed."), Qgis.MessageLevel.Warning)
            return
        warning = self.tr("Are you sure you want to remove the following repository?") + "\n" + reposName
        if QMessageBox.warning(iface.mainWindow(), self.tr("QGIS Python Plugin Installer"), warning, QMessageBox.StandardButton.Yes, QMessageBox.StandardButton.No) == QMessageBox.StandardButton.No:
            return
        # delete from the settings, refresh data and repopulate all the widgets
        settings.remove(reposName)
        repositories.remove(reposName)
        plugins.removeRepository(reposName)
        self.reloadAndExportData()

    # ----------------------------------------- #
    def setRepositoryInspectionFilter(self, reposName=None):
        """ temporarily block another repositories to fetch only one for inspection """
        repositories.setInspectionFilter(reposName)
        self.reloadAndExportData()

    # ----------------------------------------- #
    def sendVote(self, plugin_id, vote):
        """ send vote via the RPC """

        if not plugin_id or not vote:
            return False
        url = "https://plugins.qgis.org/plugins/RPC2/"
        params = {"id": "djangorpc", "method": "plugin.vote", "params": [str(plugin_id), str(vote)]}
        req = QNetworkRequest(QUrl(url))
        req.setAttribute(QNetworkRequest.Attribute(QgsNetworkRequestParameters.RequestAttributes.AttributeInitiatorClass), "QgsPluginInstaller")
        req.setAttribute(QNetworkRequest.Attribute(QgsNetworkRequestParameters.RequestAttributes.AttributeInitiatorRequestId), "sendVote")
        req.setRawHeader(b"Content-Type", b"application/json")
        reply = QgsNetworkAccessManager.instance().blockingPost(req, bytes(json.dumps(params), "utf-8"))
        if reply.attribute(QNetworkRequest.Attribute.HttpStatusCodeAttribute) == 200:
            return True
        else:
            return False

    def installFromZipFile(self, filePath):
        if not os.path.isfile(filePath):
            return

        QgsSettingsTree.node("plugin-manager").childSetting("last-zip-directory").setValue(QFileInfo(filePath).absoluteDir().absolutePath())

        pluginName = None
        with zipfile.ZipFile(filePath, 'r') as zf:
            # search for metadata.txt. In case of multiple files, we can assume that
            # the shortest path relates <pluginname>/metadata.txt
            metadatafiles = sorted(f for f in zf.namelist() if f.endswith('metadata.txt'))
            if len(metadatafiles) > 0:
                pluginName = os.path.split(metadatafiles[0])[0]

        pluginFileName = os.path.splitext(os.path.basename(filePath))[0]

        if not pluginName:
            msg_box = QMessageBox()
            msg_box.setIcon(QMessageBox.Icon.Warning)
            msg_box.setWindowTitle(self.tr("QGIS Python Install from ZIP Plugin Installer"))
            msg_box.setText(self.tr("The Zip file is not a valid QGIS python plugin. No root folder was found inside."))
            msg_box.setStandardButtons(QMessageBox.StandardButton.Ok)
            more_info_btn = msg_box.addButton(self.tr("More Information"), QMessageBox.ButtonRole.HelpRole)
            msg_box.exec()
            if msg_box.clickedButton() == more_info_btn:
                QgsHelp.openHelp("plugins/plugins.html#the-install-from-zip-tab")
            return

        pluginsDirectory = HOME_PLUGIN_PATH
        if not QDir(pluginsDirectory).exists():
            QDir().mkpath(pluginsDirectory)

        pluginDirectory = QDir.cleanPath(os.path.join(pluginsDirectory, pluginName))

        # if plugin is active, unload it before update, see https://github.com/qgis/QGIS/issues/54968
        if isPluginLoaded(pluginName):
            unloadPlugin(pluginName)

        # If the target directory already exists as a link,
        # remove the link without resolving
        QFile(pluginDirectory).remove()

        password = None
        infoString = None
        success = False
        keepTrying = True

        while keepTrying:
            try:
                # Test extraction. If fails, then exception will be raised and no removing occurs
                unzip(filePath, pluginsDirectory, password)
                # Removing old plugin files if exist
                removeDir(pluginDirectory)
                # Extract new files
                unzip(filePath, pluginsDirectory, password)
                keepTrying = False
                success = True
            except Exception as e:
                success = False
                if 'password' in str(e):
                    infoString = self.tr('Aborted by user')
                    if 'Bad password' in str(e):
                        msg = self.tr('Wrong password. Please enter a correct password to the zip file.')
                    else:
                        msg = self.tr('The zip file is encrypted. Please enter password.')
                    # Display a password dialog with QgsPasswordLineEdit
                    dlg = QDialog()
                    dlg.setWindowTitle(self.tr('Enter password'))
                    buttonBox = QDialogButtonBox(QDialogButtonBox.StandardButton.Ok | QDialogButtonBox.StandardButton.Cancel, Qt.Orientation.Horizontal)
                    buttonBox.rejected.connect(dlg.reject)
                    buttonBox.accepted.connect(dlg.accept)
                    lePass = QgsPasswordLineEdit()
                    layout = QVBoxLayout()
                    layout.addWidget(QLabel(msg))
                    layout.addWidget(lePass)
                    layout.addWidget(buttonBox)
                    dlg.setLayout(layout)
                    keepTrying = dlg.exec()
                    password = lePass.text()
                else:
                    infoString = self.tr("Failed to unzip the plugin package\n{}.\nProbably it is broken".format(filePath))
                    keepTrying = False

        if success:
            with OverrideCursor(Qt.CursorShape.WaitCursor):
                updateAvailablePlugins()
                self.processDependencies(pluginName)
                loadPlugin(pluginName)
                plugins.getAllInstalled()
                plugins.rebuild()

                settings = QgsSettings()
                if settings.contains('/PythonPlugins/' + pluginName):  # Plugin was available?
                    unloadPlugin(pluginName)
                    loadPlugin(pluginName)
                    if settings.value('/PythonPlugins/' + pluginName, False, bool):  # Plugin was also active?
                        startPlugin(pluginName)
                else:
                    if startPlugin(pluginName):
                        settings.setValue('/PythonPlugins/' + pluginName, True)

            self.exportPluginsToManager()
            msg = "<b>%s</b>" % self.tr("Plugin installed successfully")
        else:
            msg = "<b>%s:</b> %s" % (self.tr("Plugin installation failed"), infoString)

        level = Qgis.MessageLevel.Success if success else Qgis.MessageLevel.Critical
        iface.pluginManagerInterface().pushMessage(msg, level)

    def processDependencies(self, plugin_id):
        """Processes plugin dependencies

        :param plugin_id: plugin id
        :type plugin_id: str
        """

        to_install, to_upgrade, not_found = find_dependencies(plugin_id)
        if to_install or to_upgrade or not_found:
            dlg = QgsPluginDependenciesDialog(plugin_id, to_install, to_upgrade, not_found)
            if dlg.exec() == QgsPluginDependenciesDialog.Accepted:
                actions = dlg.actions()
                for dependency_plugin_id, action_data in actions.items():
                    try:
                        self.installPlugin(dependency_plugin_id, stable=action_data['use_stable_version'])
                        if action_data['action'] == 'install':
                            iface.pluginManagerInterface().pushMessage(self.tr("Plugin dependency <b>%s</b> successfully installed") %
                                                                       dependency_plugin_id, Qgis.MessageLevel.Success)
                        else:
                            iface.pluginManagerInterface().pushMessage(self.tr("Plugin dependency <b>%s</b> successfully upgraded") %
                                                                       dependency_plugin_id, Qgis.MessageLevel.Success)
                    except Exception as ex:
                        if action_data['action'] == 'install':
                            iface.pluginManagerInterface().pushMessage(self.tr("Error installing plugin dependency <b>%s</b>: %s") %
                                                                       (dependency_plugin_id, ex), Qgis.MessageLevel.Warning)
                        else:
                            iface.pluginManagerInterface().pushMessage(self.tr("Error upgrading plugin dependency <b>%s</b>: %s") %
                                                                       (dependency_plugin_id, ex), Qgis.MessageLevel.Warning)
