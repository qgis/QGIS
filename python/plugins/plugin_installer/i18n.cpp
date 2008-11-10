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
  translate("QgsPluginInstaller","Plugin directory doesn't exist:")
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
  tr("Failed to unzip file to the following directory:")
  tr("Check permissions")

 // def abort
  tr("Aborted by user")
}


QgsPluginInstallerPluginErrorDialog::foo()
{
 // def __init__
  tr("No error message received. Try to restart Quantum GIS and ensure the plugin isn't installed under a different name. If it is, contact the plugin author and submit this issue, please.")
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
  tr("This plugin seems to be invalid or have unfulfilled dependencies\nIt has been installed, but can't be loaded")
  tr("not installed", "singular")
  tr("installed", "singular")
  tr("upgradeable", "singular")
  tr("installed", "singular")
  tr("new!", "singular")
  tr("installed", "singular")
  tr("invalid", "singular")
  tr("Note that it's an uninsatallable core plugin")
  tr("installed version")
  tr("available version")
  tr("available version")
  tr("installed version")
  tr("That's the newest available version")
  tr("installed version")
  tr("There is no version available for download")
  tr("This plugin seems to be invalid or have unfulfilled dependencies")
  tr("This plugin seems to be invalid or have unfulfilled dependencies\nIt has been installed, but can't be loaded")
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
  tr("Plugin installation failed")
  tr("Plugin installed successfully")
  tr("Python plugin installed.\nYou have to enable it in the Plugin Manager.")
  tr("Plugin installed successfully")
  tr("Python plugin reinstalled.\nYou have to restart Quantum GIS to reload it.")
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
