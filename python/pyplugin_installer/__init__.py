# -*- coding: utf-8 -*-

"""
***************************************************************************
    __init__.py
    ---------------------
    Date                 : May 2013
    Copyright            : (C) 2013 by Borys Jurgiel
    Email                : info at borysjurgiel dot pl

    This module is based on former plugin_installer plugin:
      Copyright (C) 2007-2008 Matthew Perry
      Copyright (C) 2008-2013 Borys Jurgiel

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
"""

__author__ = 'Borys Jurgiel'
__date__ = 'May 2013'
__copyright__ = '(C) 2013, Borys Jurgiel'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'


# import functions for easier access
import installer
from installer import initPluginInstaller


def instance():
    if not installer.pluginInstaller:
        installer.initPluginInstaller()
    return installer.pluginInstaller
