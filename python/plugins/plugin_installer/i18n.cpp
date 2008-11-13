/*
 This is NOT a proper c++ source code. This file is only designed to be catched
 by qmake and included to lupdate. It contains all translateable strings copied
 from the python files:
   installer_data.py
   installer_plugin.py
   installer_gui.py

 Please keep the python files and this file synchronized. I hope we'll find
 a more automated way to put PyQt strings to the qgis_*.ts files some day.
*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/




/*---------------------
file: installer_data.py
---------------------*/

 // Repositories::xmlDownloaded
 translate("QgsPluginInstaller","Couldn't parse output from the repository")

 // Plugins::getAllInstalled
 translate("QgsPluginInstaller","Couldn't open the system plugin directory")
 translate("QgsPluginInstaller","Couldn't open the local plugin directory")




/* ----------------------
file: installer_plugin.py
------------------------*/

// InstallerPlugin::initGui
  translate("QgsPluginInstaller","Fetch Python Plugins...")
  translate("QgsPluginInstaller","Install more plugins from remote repositories")
  translate("QgsPluginInstaller","Install more plugins from remote repositories")
  translate("QgsPluginInstaller","Looking for new plugins...")

// InstallerPlugin::checkingDone
  translate("QgsPluginInstaller","There is a new plugin available")
  translate("QgsPluginInstaller","There is a plugin update available")

// InstallerPlugin::run
  translate("QgsPluginInstaller","QGIS Python Plugin Installer")
  translate("QgsPluginInstaller","Error reading repository:")




/*--------------------
file: installer_gui.py
--------------------*/

// common functions
  translate("QgsPluginInstaller","Nothing to remove! Plugin directory doesn't exist:")
  translate("QgsPluginInstaller","Failed to remove the directory:")
  translate("QgsPluginInstaller","Check permissions or remove it manually")

QgsPluginInstallerFetchingDialog::foo()
{
 // def displayState
  tr("Success")
  tr("Resolving host name...")
  tr("Connecting...")
  tr("Host connected. Sending request...")
  tr("Downloading data...")
  tr("Idle")
  tr("Closing connection...")
  tr("Error")
}

QgsPluginInstallerInstallingDialog::foo()
{
 // def stateChanged
  tr("Installing...")
  tr("Resolving host name...")
  tr("Connecting...")
  tr("Host connected. Sending request...")
  tr("Downloading data...")
  tr("Idle")
  tr("Closing connection...")
  tr("Error")

 // def requestFinished
  tr("Failed to unzip the plugin package. Probably it's broken or missing from the repository. You may also want to make sure that you have write permission to the plugin directory:")

 // def abort
  tr("Aborted by user")
}


QgsPluginInstallerPluginErrorDialog::foo()
{
 // def __init__
  tr("no error message received")
}

QgsPluginInstallerDialog::foo()
{
 // def __init__

 // def getAllAvailablePlugins
  tr("QGIS Python Plugin Installer")
  tr("Error reading repository:")

 // def populateMostWidgets
  tr("all repositories")
  tr("connected")
  tr("This repository is connected")
  tr("unavailable")
  tr("This repository is enabled, but unavailable")
  tr("disabled")
  tr("This repository is disabled")
  tr("This repository is blocked due to incompatibility with your Quantum GIS version")
  tr("orphans")
  tr("any status")
  tr("not installed", "plural")
  tr("installed", "plural")
  tr("upgradeable and news")

 // def filterChanged

 // def filterCheck
  tr("orphans")
  tr("orphans")

 // def populatePluginTree
  tr("This plugin is not installed")
  tr("This plugin is installed")
  tr("This plugin is installed, but there is an updated version available")
  tr("This plugin is installed, but I can't find it in any enabled repository")
  tr("This plugin is not installed and is seen for the first time")
  tr("This plugin is installed and is newer than its version available in a repository")
  tr("This plugin is incompatible with your Quantum GIS version and probably won't work.")
  tr("The required Python module is not installed.\nFor more information, please visit its homepage.")
  tr("This plugin seems to be broken.\nIt has been installed but can't be loaded.\nHere is the error message:")
  tr("not installed", "singular")
  tr("installed", "singular")
  tr("upgradeable", "singular")
  tr("installed", "singular")
  tr("new!", "singular")
  tr("installed", "singular")
  tr("invalid", "singular")
  tr("Note that it's an uninstallable core plugin")
  tr("installed version")
  tr("available version")
  tr("available version")
  tr("installed version")
  tr("That's the newest available version")
  tr("installed version")
  tr("There is no version available for download")
  tr("This plugin is broken")
  tr("This plugin requires a newer version of Quantum GIS")
  tr("This plugin requires a missing module")
  tr("only locally available")

 // def treeClicked
  tr("Install plugin")
  tr("Reinstall plugin")
  tr("Upgrade plugin")
  tr("Install/upgrade plugin")
  tr("Install plugin")
  tr("Downgrade plugin")
  tr("Reinstall plugin")
  tr("Install/upgrade plugin")

 // def installPlugin
  tr("QGIS Python Plugin Installer")
  tr("Are you sure you want to downgrade the plugin to the latest available version? The installed one is newer!")
  tr("Plugin installation failed")
  tr("Plugin has disappeared")
  tr("The plugin seems to have been installed but I don't know where. Probably the plugin package contained a wrong named directory.\nPlease search the list of installed plugins. I'm nearly sure you'll find the plugin there, but I just can't determine which of them it is. It also means that I won't be able to determine if this plugin is installed and inform you about available updates. However the plugin may work. Please contact the plugin author and submit this issue.")
  tr("Plugin installed successfully")
  tr("Python plugin installed.\nYou have to enable it in the Plugin Manager.")
  tr("Plugin reinstalled successfully")
  tr("Python plugin reinstalled.\nYou have to restart Quantum GIS to reload it.")
  tr("The plugin is designed for a newer version of Quantum GIS. The minimum required version is:")
  tr("The plugin depends on some components missing on your system. You need to install the following Python module in order to enable it:")
  tr("The plugin is broken. Python said:")
  tr("Plugin uninstall failed")

 // def uninstallPlugin
  tr("Are you sure you want to uninstall the following plugin?")
  tr("Warning: this plugin isn't available in any accessible repository!")
  tr("QGIS Python Plugin Installer")
  tr("Plugin uninstall failed")
  tr("QGIS Python Plugin Installer")
  tr("Plugin uninstalled successfully")

 // def ChangeCheckingPolicy

 // def addKnownRepositories
  tr("You are going to add some plugin repositories neither authorized nor supported by the Quantum GIS team, however provided by folks associated with us. Plugin authors generally make efforts to make their works useful and safe, but we can't assume any responsibility for them. FEEL WARNED!")
  tr("QGIS Python Plugin Installer")

 // def addRepository
  tr("QGIS Python Plugin Installer")
  tr("Unable to add another repository with the same URL!")

 // def editRepository
  tr("This repository is blocked due to incompatibility with your Quantum GIS version")
  tr("QGIS Python Plugin Installer")
  tr("Unable to add another repository with the same URL!")

 // def deleteRepository
  tr("Are you sure you want to remove the following repository?")
  tr("QGIS Python Plugin Installer")

 // def reject

}
