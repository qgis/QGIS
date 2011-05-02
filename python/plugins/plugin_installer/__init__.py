# -*- coding: utf-8 -*-
"""
Copyright (C) 2007-2008 Matthew Perry
Copyright (C) 2008-2010 Borys Jurgiel
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
  return "Plugin Installer"

def version():
  return "Version 1.2.1"

def description():
  return "Downloads and installs QGIS python plugins"

def qgisMinimumVersion():
  return "1.0"

def icon():
  import resources_rc
  return ":/plugins/installer/plugin_installer.png"

def authorName():
  return "Matthew Perry, Borys Jurgiel"

def homepage():
  return "http://bwj.aster.net.pl/qgis/"

def classFactory(iface):
  from installer_plugin import InstallerPlugin
  return InstallerPlugin(iface)
