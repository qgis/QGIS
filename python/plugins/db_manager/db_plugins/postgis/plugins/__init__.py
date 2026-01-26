"""
/***************************************************************************
Name                 : DB Manager
Description          : Database manager plugin for QGIS
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

import os
from importlib import import_module

current_dir = os.path.dirname(__file__)


def load(db, mainwindow):
    for name in os.listdir(current_dir):
        if not os.path.isdir(os.path.join(current_dir, name)):
            continue
        if name in ("__pycache__"):
            continue
        try:
            plugin_module = import_module(".".join((__package__, name)))
        except ImportError:
            continue
        plugin_module.load(db, mainwindow)
