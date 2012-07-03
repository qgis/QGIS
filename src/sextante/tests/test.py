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
from qgis.core import *
from qgis_interface import QgisInterface
from PyQt4.QtGui import QWidget
from utilities_test import getQgisTestApp
#from gui.is_plugin import ISPlugin
from sextante.SextantePlugin import SextantePlugin
from sextante.core.Sextante import Sextante
from sextante.parameters.ParameterRaster import ParameterRaster
from sextante.parameters.ParameterVector import ParameterVector
from sextante.parameters.ParameterNumber import ParameterNumber
from sextante.outputs.OutputRaster import OutputRaster
from sextante.outputs.OutputVector import OutputVector
QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()

class DataProviderStub:
    def __init__(self, uri):
        self.dataSourceUri = lambda: uri

class SextantePluginTest(unittest.TestCase):
    """Test suite for Sextante QGis plugin"""
    def gen_test_parameters(self, alg):
        for p in alg.parameters:
            if isinstance(p, ParameterRaster):
                l = QgsRasterLayer('.data/raster', "test raster")
                l.dataProvider = lambda: DataProviderStub('data/raster')
                yield l
            elif isinstance(p, ParameterVector):
                l = QgsVectorLayer('.data/vector', "test vector")
                l.dataProvider = lambda: DataProviderStub('data/vector')
                yield l
            elif isinstance(p, ParameterNumber):
                yield p.max
            else:
                yield
        i = 0;
        for o in alg.outputs:
            if o.hidden:
                continue;
            i = i + 1
            if isinstance(o, OutputRaster):
                yield 'output%i.tif' % i
            elif isinstance(o, OutputVector):
                yield 'output%i.shp' % i
            else:
                yield
        
    def test_0createplugin(self):
        """Initialize plugin"""
        self.sextanteplugin = SextantePlugin(IFACE)
        self.assertIsNotNone(self.sextanteplugin)

    def test_1sextante_alglist(self):
        """Test alglist"""
        self.sextanteplugin = SextantePlugin(IFACE)
        self.providerToAlgs = Sextante.algs
        self.assertTrue(self.providerToAlgs, "Alg list")

    def test_runalg(self):
        self.sextanteplugin = SextantePlugin(IFACE)
        self.providerToAlgs = Sextante.algs
        for provider, algs in self.providerToAlgs.items():
            if not algs.items():
                print "WARINING: %s seems to provide no algs!" % provider
                continue
            algId, alg = algs.items()[-1]
            args = list(self.gen_test_parameters(alg))
            print "Alg: ", algId
            print alg.parameters, ' => ', args
            result = Sextante.runalg(algId, *args)
            self.assertIsNotNone(result, "Running directly %s" % algId)
            print algId, " ok."

if __name__ == '__main__':
    unittest.main()
