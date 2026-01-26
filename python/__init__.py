"""
***************************************************************************
    __init__.py
    ---------------------
    Date                 : January 2007
    Copyright            : (C) 2007 by Martin Dobias
    Email                : wonder dot sk at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Martin Dobias"
__date__ = "January 2007"
__copyright__ = "(C) 2007, Martin Dobias"

import os
import sys


def setupenv():
    """
    Set the environment for Windows based on the .vars files from the
    OSGeo4W package format.
    """
    # If the prefix path is already set then we don't do any more path setup.
    if os.getenv("QGIS_PREFIX_PATH"):
        return

    # Setup the paths based on the .vars file.
    from pathlib import PurePath

    path_split = PurePath(os.path.dirname(os.path.realpath(__file__))).parts

    try:
        appname = os.environ["QGIS_ENVNAME"]
    except KeyError:
        appname = path_split[-3]

    envfile = list(path_split[:-4])
    envfile.append("bin")
    envfile.append(f"{appname}-bin.env")
    envfile = os.path.join(*envfile)

    if not os.path.exists(envfile):
        return

    with open(envfile) as f:
        for line in f:
            line = line.rstrip("\n")
            if line.startswith("#") or not line:
                continue
            try:
                env_key, env_value = line.split("=", maxsplit=1)
                os.environ[env_key] = env_value
            except ValueError:
                pass


if os.name == "nt":
    # On Windows we need to setup the paths before we can import
    # any of the QGIS modules or else it will error.
    setupenv()

    if sys.version_info[0] > 3 or (
        sys.version_info[0] == 3 and sys.version_info[1] >= 9
    ):
        for p in os.getenv("PATH").split(";"):
            if os.path.exists(p):
                os.add_dll_directory(p)

from qgis.PyQt import QtCore

# monkey patching custom widgets in case we are running on a local install
# this should fix import errors such as "ModuleNotFoundError: No module named qgsfilewidget"
# ("from qgsfilewidget import QgsFileWidget")
# In a complete install, this is normally avoided and rather imports "qgis.gui"
# (thanks to uic/widget-plugins/qgis_customwidgets.py)
try:
    import qgis.gui

    widget_list = dir(qgis.gui)
    # remove widgets that are not allowed as custom widgets (they need to be manually promoted)
    skip_list = ["QgsScrollArea"]
    for widget in widget_list:
        if widget.startswith("Qgs") and widget not in skip_list:
            sys.modules[widget.lower()] = qgis.gui
except ImportError:
    # gui might not be built
    pass
