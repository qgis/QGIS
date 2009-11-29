"""
/***************************************************************************
        MapServerExport  - A QGIS plugin to export a saved project file
                            to a MapServer map file
                             -------------------
    begin                : 2008-01-07
    copyright            : (C) 2008 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 This script initializes the plugin, making it known to QGIS.
"""

def name(): 
  return "MapServer Export" 
def description():
  return "Export a saved QGIS project file to a MapServer map file"
def version(): 
  return "Version 0.4.3" 
def qgisMinimumVersion(): 
  return "1.0"
def authorName():
  return "Gary E. Sherman"
def classFactory(iface): 
  # load MapServerExport class from file mapserverexport.py
  from mapserverexport import MapServerExport 
  return MapServerExport(iface) 
