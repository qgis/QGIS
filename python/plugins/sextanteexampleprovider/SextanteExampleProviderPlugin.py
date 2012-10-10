# -*- coding: utf-8 -*-

"""
***************************************************************************
    SextanteExampleProviderPlugin.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Tim Sutton
    Email                : tim at linfiniti dot com
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

from qgis.core import *
import os, sys
import inspect
from sextante.core.Sextante import Sextante
from sextanteexampleprovider.ExampleAlgorithmProvider import ExampleAlgorithmProvider


cmd_folder = os.path.split(inspect.getfile( inspect.currentframe() ))[0]
if cmd_folder not in sys.path:
    sys.path.insert(0, cmd_folder)

class SextanteExampleProviderPlugin:

    def __init__(self):
        self.provider = ExampleAlgorithmProvider()
    def initGui(self):
        Sextante.addProvider(self.provider)

    def unload(self):
        Sextante.removeProvider(self.provider)

