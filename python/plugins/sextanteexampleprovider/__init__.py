# -*- coding: utf-8 -*-

"""
***************************************************************************
    __init__.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Tim Sutton
    Email                : tim dot linfiniti at com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Tim Sutton'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Tim Sutton'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from sextanteexampleprovider.SextanteExampleProviderPlugin import SextanteExampleProviderPlugin
def name():
    return "SEXTANTE example provider"
def description():
    return "An example plugin that adds algorithms to SEXTANTE. Mainly created to guide developers in the process of creating plugins that add new capabilities to SEXTANTE"
def version():
    return "Version 1.0"
def icon():
    return "icon.png"
def qgisMinimumVersion():
    return "1.0"
def classFactory(iface):
    return SextanteExampleProviderPlugin()
