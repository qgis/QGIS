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

import itertools
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
from sextante.parameters.ParameterString import ParameterString
from sextante.parameters.ParameterBoolean import ParameterBoolean
from sextante.outputs.OutputRaster import OutputRaster
from sextante.outputs.OutputVector import OutputVector
from sextante.core.SextanteConfig import SextanteConfig
QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()

class DataProviderStub:
    def __init__(self, uri):
        self.dataSourceUri = lambda: uri

class SextantePluginTest(unittest.TestCase):
    """Test suite for Sextante QGis plugin"""
        
    def test_createplugin(self):
        """Initialize plugin"""
        self.sextanteplugin = SextantePlugin(IFACE)
        self.assertIsNotNone(self.sextanteplugin)

    def test_sextante_alglist(self):
        """Test alglist"""
        self.sextanteplugin = SextantePlugin(IFACE)
        self.providerToAlgs = Sextante.algs
        self.assertTrue(self.providerToAlgs, "Alg list")

class SextanteProviderTestCase(unittest.TestCase):
    def __init__(self, algId, alg, threaded):
        self.algId = algId
        self.alg = alg
        self.threaded = threaded
        self.msg = "ALG %s (%s)" % (self.algId, { True: "threaded" , False : "unthreaded"}[threaded])
        unittest.TestCase.__init__(self, "test_runalg")

    def gen_test_parameters(self, alg):
        b = False
        for p in alg.parameters:
            if isinstance(p, ParameterRaster):
                l = QgsRasterLayer('data/raster', "test raster")
                l.dataProvider = lambda: DataProviderStub('data/raster')
                yield l
            elif isinstance(p, ParameterVector):
                l = QgsVectorLayer('data/vector', "test vector")
                l.dataProvider = lambda: DataProviderStub('data/vector')
                yield l
            elif isinstance(p, ParameterNumber):
                if p.max:
                    yield p.max
                elif p.min:
                    yield p.min
                yield 42
            elif isinstance(p, ParameterString):
                yield str()
            elif isinstance(p, ParameterBoolean):
                b = not b
                yield b
            else:
                yield
        i = 0;
        for o in alg.outputs:
            if o.hidden:
                continue;
            i = i + 1
            outbasename = self.msg.replace('/', '-')
            if isinstance(o, OutputRaster):
                yield 'outputs/%s - %i.tif' % (outbasename, i)
            elif isinstance(o, OutputVector):
                yield 'outputs/%s - %i.shp' % (outbasename, i)
            else:
                yield
        
    def test_runalg(self):
        SextanteConfig.setSettingValue(SextanteConfig.USE_THREADS, self.threaded)
        args = list(self.gen_test_parameters(self.alg))
        print 
        print self.msg, "Parameters: ", self.alg.parameters, ' => ', args
        result = Sextante.runalg(self.algId, *args)
        self.assertIsNotNone(result, self.msg)
        if not result:
            return
        for p in result.values():
            if isinstance(p, str):
                self.assertTrue(os.path.exists(p), "Output %s exists" % p)

def algSuite():
    s = unittest.TestSuite()
    for provider, algs in Sextante.algs.items():
        if not algs.items():
            print "WARNING: %s seems to provide no algs!" % provider
            continue
        algId, alg = algs.items()[-1]        
        s.addTest(SextanteProviderTestCase(algId, alg, True))
        s.addTest(SextanteProviderTestCase(algId, alg, False))
    return s

if __name__ == '__main__':
    if not os.path.exists("data/raster") or not os.path.exists("data/vector"):
        print "Please install test data under ./data/raster and ./data/vector."
        exit(1)
    loadSuite = unittest.TestLoader().loadTestsFromTestCase(SextantePluginTest)
    unittest.TextTestRunner(verbosity=2).run(loadSuite)
    unittest.TextTestRunner(verbosity=2).run(algSuite())

