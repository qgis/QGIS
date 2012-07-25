# -*- coding: utf-8 -*-

"""
/***************************************************************************
Name                 : DB Manager
Description          : Database manager plugin for QuantumGIS
Date                 : May 23, 2011
copyright            : (C) 2011 by Giuseppe Sucameli
email                : brush.tyler@gmail.com

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

def name():
	return "DB Manager"

def description():
	return "Manage your databases within QGis"

def version():
	return "0.1.20"

def qgisMinimumVersion():
	return "1.5.0"

def icon():
	return "icons/dbmanager.png"

def authorName():
	return "Giuseppe Sucameli"

def classFactory(iface):
	from .db_manager_plugin import DBManagerPlugin
	return DBManagerPlugin(iface)
