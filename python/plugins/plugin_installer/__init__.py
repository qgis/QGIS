"""
Copyright (C) 2008 Matthew Perry
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
"""
def name():
  return "Plugin installer"

def description():
  return "Downloads and installs QGIS python plugins"

def version():
  return "Version 0.02"

def classFactory(iface):
  # load TestPlugin class from file testplugin.py
  from installer_plugin import InstallerPlugin
  return InstallerPlugin(iface)
