# -*- coding: utf-8 -*-

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
__copyright__ = \
    'Copyright 2012, Australia Indonesia Facility for Disaster Reduction'

import os
import sys
import time
import itertools
import unittest

from PyQt4.QtGui import QWidget
from qgis.gui import QgsMapCanvas
from qgis.core import *

import processing
from processing.ProcessingPlugin import ProcessingPlugin
from processing.core.Processing import Processing
from processing.core.ProcessingLog import ProcessingLog
from processing.core.ProcessingConfig import ProcessingConfig
from processing.gui.AlgorithmDialog import AlgorithmDialog
from processing.modeler.ModelerAlgorithm import ModelerAlgorithm
from processing.modeler.Providers import Providers
from processing.parameters.ParameterRaster import ParameterRaster
from processing.parameters.ParameterVector import ParameterVector
from processing.parameters.ParameterNumber import ParameterNumber
from processing.parameters.ParameterString import ParameterString
from processing.parameters.ParameterBoolean import ParameterBoolean
from processing.core.outputs import OutputRaster
from processing.core.outputs import OutputVector

from qgis_interface import QgisInterface
from utilities_test import getQgisTestApp

# Add parent directory to path to make test aware of other modules

pardir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
sys.path.append(pardir)

sys.path.append( qgis.utils.sys_plugin_path )

(QGISAPP, CANVAS, IFACE, PARENT) = getQgisTestApp()


class bcolors:

    INFO = '\033[94m'
    WARNING = '\033[91m'
    ENDC = '\033[0m'


class ProcessingPluginTest(unittest.TestCase):
    """Test suite for Processing QGis plugin."""

    def test_createplugin(self):
        """Initialize plugin."""

        self.processingplugin = ProcessingPlugin(IFACE)
        self.assertIsNotNone(self.processingplugin)

    def test_processing_alglist(self):
        """Test alglist"""

        self.processingplugin = ProcessingPlugin(IFACE)
        self.providerToAlgs = Processing.algs
        self.assertTrue(self.providerToAlgs, 'Alg list')


class ProcessingProviderTestCase(unittest.TestCase):

    def __init__(
        self,
        algId,
        alg,
        threaded,
        dialog='none',
        ):
        self.algId = algId
        self.alg = alg
        self.threaded = threaded
        self.msg = 'ALG %s (%s %s)' % (self.algId, {True: 'threaded',
                                       False: 'unthreaded'}[threaded], dialog)
        unittest.TestCase.__init__(self, 'runalg_%s' % dialog)

    def gen_test_parameters(self, alg, doSet=False):
        b = False
        for p in alg.parameters:
            if isinstance(p, ParameterRaster):
                l = IFACE.testRaster
                if doSet:
                    p.setValue(l)
                yield l
            elif isinstance(p, ParameterVector):
                l = IFACE.testVector
                if doSet:
                    p.setValue(l)
                yield l
            elif isinstance(p, ParameterNumber):
                n = 42
                if p.max:
                    n = p.max
                elif p.min:
                    n = p.min
                if doSet:
                    p.setValue(n)
                yield n
            elif isinstance(p, ParameterString):
                s = 'Test string'
                if doSet:
                    p.setValue(s)
                yield s
            elif isinstance(p, ParameterBoolean):
                b = not b
                if doSet:
                    p.setValue(b)
                yield b
            else:
                if doSet:
                    p.setValue(None)
                yield None
        i = 0
        for o in alg.outputs:
            if o.hidden:
                continue
            i = i + 1
            outbasename = self.msg.replace('/', '-')
            if isinstance(o, OutputRaster):
                fn = 'outputs/%s - %i.tif' % (outbasename, i)
                if doSet:
                    o.setValue(fn)
                yield fn
            elif isinstance(o, OutputVector):
                fn = 'outputs/%s - %i.shp' % (outbasename, i)
                if doSet:
                    o.setValue(fn)
                yield fn
            else:
                if doSet:
                    o.setValue(None)
                yield None

    def setUp(self):
        print
        print bcolors.INFO, self.msg, bcolors.ENDC,
        print 'Parameters: ', self.alg.parameters,
        print 'Outputs: ', [out for out in self.alg.outputs
                            if not out.hidden],
        self.args = list(self.gen_test_parameters(self.alg, True))
        print ' => ', self.args, bcolors.WARNING,

    def runalg_none(self):
        result = processing.runalg(self.alg, *self.args)
        print bcolors.ENDC
        self.assertIsNotNone(result, self.msg)
        if not result:
            return
        for p in result.values():
            if isinstance(p, str):
                self.assertTrue(os.path.exists(p), 'Output %s exists' % p)

    def runalg_parameters(self):
        dlg = self.alg.getCustomParametersDialog()
        if not dlg:
            dlg = AlgorithmDialog(self.alg)

        # Hack to handle that hacky code...
        dlg.setParamValues = lambda : True
        dlg.show()
        dlg.accept()
        while not dlg.executed:
            time.sleep(.5)

    def tearDown(self):
        print bcolors.ENDC


def algSuite(dialog='none', threaded=True, unthreaded=True,
             provider=None, algName=None):
    s = unittest.TestSuite()
    for (provider, algs) in Processing.algs.items():
        if not algs.items():
            print bcolors.WARNING, 'WARNING: %s seems to provide no algs!' \
                % provider,
            print bcolors.ENDC
            continue
        (algId, alg) = algs.items()[-1]
        if threaded:
            s.addTest(ProcessingProviderTestCase(algId, alg, True, dialog))
        if unthreaded:
            s.addTest(ProcessingProviderTestCase(algId, alg, False, dialog))
    return s


def modelSuite(modelFile, dialog='none', threaded=True, unthreaded=True):
    s = unittest.TestSuite()
    model = ModelerAlgorithm()
    model.openModel(modelFile)
    if model.provider is None:
        # Might happen if model is opened from modeler dialog
        model.provider = Providers.providers['model']
    if threaded:
        s.addTest(ProcessingProviderTestCase(modelFile, model, True, dialog))
    if unthreaded:
        s.addTest(ProcessingProviderTestCase(modelFile, model, False, dialog))
    return s


if __name__ == '__main__':
    import argparse

    parser = argparse.ArgumentParser(description='Processing test suite.')
    parser.add_argument('-l', action='store_true',
            help='Test processing loading only. Ignore further arguments.')
    parser.add_argument('-m', dest='model', help='Test a particular model.',
                        default=None)
    parser.add_argument('-d', dest='dialog', help='Test a particular dialog.',
                        default='none')
    parser.add_argument('-t', dest='tOnly', action='store_true',
                        help='Enable threaded execution only.')
    parser.add_argument('-u', dest='uOnly', action='store_true',
                        help='Enable unthreaded execution only.')
    parser.add_argument('-r', dest='raster',
                        help='Use specified raster as input.',
                        default='data/raster')
    parser.add_argument('-v', dest='vector',
                        help='Use specified vectro as input.',
                        default='data/vector')

    args = parser.parse_args()

    threaded = not args.uOnly or args.tOnly
    unthreaded = not args.tOnly or args.uOnly

    try:
        loadSuite = \
            unittest.TestLoader().loadTestsFromTestCase(ProcessingPluginTest)
        unittest.TextTestRunner(verbosity=2).run(loadSuite)
        if args.l:
            exit(0)
        if not os.path.exists(args.raster) or not os.path.exists(args.vector):
            print 'No data under %s or %s. Run with -h argument for help' \
                % (args.raster, args.vector)
            exit(1)
        if args.model:
            unittest.TextTestRunner(verbosity=2).run(modelSuite(args.model
                    or 'data/model', args.dialog, threaded, unthreaded))
            exit(0)
        unittest.TextTestRunner(verbosity=2).run(algSuite(args.dialog,
                threaded, unthreaded, args.dialog))
    except KeyboardInterrupt:
        print bcolors.ENDC, 'Test interrupted.'
