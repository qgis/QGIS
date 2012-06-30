"""
InaSAFE Disaster risk assessment tool developed by AusAid -
**QGIS plugin test suite.**

Contact : ole.moller.nielsen@gmail.com

.. note:: This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

"""

__author__ = 'tim@linfiniti.com'
__version__ = '0.3.0'
__date__ = '10/01/2011'
__copyright__ = ('Copyright 2012, Australia Indonesia Facility for '
                 'Disaster Reduction')

import unittest
import sys
import os

# Add parent directory to path to make test aware of other modules
pardir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
sys.path.append(pardir)

sys.path.append('/usr/share/qgis/python/plugins')

from qgis.gui import QgsMapCanvas
from qgis_interface import QgisInterface
from PyQt4.QtGui import QWidget
from utilities_test import getQgisTestApp
#from gui.is_plugin import ISPlugin
from sextante import SextantePlugin
from sextante.core.Sextante import Sextante
QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()


class SextantePluginTest(unittest.TestCase):
    """Test suite for Sextante QGis plugin"""

    def test_createplugin(self):
        """Initialize plugin"""
        sextanteplugin = SextantePlugin(IFACE)
        assert sextanteplugin != None,  "Unable to create plugin"

    def test_sextante_alglist(self):
        """Test alglist"""
        sextanteplugin = SextantePlugin(IFACE)
        algs = Sextante.algs.values()
        for provider in Sextante.algs.values():
            for algo in provider.values():
                print algo.commandLineName()
        assert algs, "Algo list is empty"

if __name__ == '__main__':
    unittest.main()
