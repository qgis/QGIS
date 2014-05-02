# -*- coding: utf-8 -*-

"""
***************************************************************************
    customwidgets.py
    ---------------------
    Date                 : May 2014
    Copyright            : (C) 2014 by Denis Rouzaud
    Email                : denis.rouzaud@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

import sys
import qgis
from qgis import gui

"""
	This allow to redirect the custom widget include header file to qgis.gui
"""

def referenceCustomWidgets():
	sys.modules["qgscollapsiblegroupboxplugin"] = qgis.gui
	sys.modules["qgsfieldcomboboxplugin"] = qgis.gui
	sys.modules["qgsfieldexpressionwidgetplugin"] = qgis.gui
	sys.modules["qgsmaplayercomboboxplugin"] = qgis.gui
	sys.modules["qgsscalevisibilitywidgetplugin"] = qgis.gui
