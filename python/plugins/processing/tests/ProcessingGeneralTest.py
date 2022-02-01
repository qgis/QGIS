# -*- coding: utf-8 -*-

"""
***************************************************************************
    QgisAlgorithmTests.py
    ---------------------
    Date                 : January 2019
    Copyright            : (C) 2019 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Nyall Dawson'
__date__ = 'January 2019'
__copyright__ = '(C) 2019, Nyall Dawson'

import nose2
import shutil
import gc

from qgis.core import (QgsApplication,
                       QgsProcessing,
                       QgsProcessingContext,
                       QgsVectorLayer,
                       QgsProject)
from qgis.PyQt import sip
from qgis.analysis import (QgsNativeAlgorithms)
from qgis.testing import start_app, unittest
import processing
from processing.tests.TestData import points


class TestProcessingGeneral(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        start_app()
        from processing.core.Processing import Processing
        Processing.initialize()
        cls.cleanup_paths = []
        cls.in_place_layers = {}
        cls.vector_layer_params = {}

    @classmethod
    def tearDownClass(cls):
        from processing.core.Processing import Processing
        Processing.deinitialize()
        for path in cls.cleanup_paths:
            shutil.rmtree(path)

    def testRun(self):
        context = QgsProcessingContext()

        # try running an alg using processing.run - ownership of result layer should be transferred back to the caller
        res = processing.run('qgis:buffer',
                             {'DISTANCE': 1, 'INPUT': points(), 'OUTPUT': QgsProcessing.TEMPORARY_OUTPUT},
                             context=context)
        self.assertIn('OUTPUT', res)
        # output should be the layer instance itself
        self.assertIsInstance(res['OUTPUT'], QgsVectorLayer)
        # Python should have ownership
        self.assertTrue(sip.ispyowned(res['OUTPUT']))
        del context
        gc.collect()
        self.assertFalse(sip.isdeleted(res['OUTPUT']))

        # now try using processing.run with is_child_algorithm = True. Ownership should remain with the context
        context = QgsProcessingContext()
        res = processing.run('qgis:buffer',
                             {'DISTANCE': 1, 'INPUT': points(), 'OUTPUT': QgsProcessing.TEMPORARY_OUTPUT},
                             context=context, is_child_algorithm=True)
        self.assertIn('OUTPUT', res)
        # output should be a layer string reference, NOT the layer itself
        self.assertIsInstance(res['OUTPUT'], str)
        layer = context.temporaryLayerStore().mapLayer(res['OUTPUT'])
        self.assertIsInstance(layer, QgsVectorLayer)
        # context should have ownership
        self.assertFalse(sip.ispyowned(layer))
        del context
        gc.collect()
        self.assertTrue(sip.isdeleted(layer))

    def testRunAndLoadResults(self):
        QgsProject.instance().removeAllMapLayers()
        context = QgsProcessingContext()

        # try running an alg using processing.runAndLoadResults - ownership of result layer should be transferred to
        # project, and layer should be present in project
        res = processing.runAndLoadResults('qgis:buffer',
                                           {'DISTANCE': 1, 'INPUT': points(), 'OUTPUT': QgsProcessing.TEMPORARY_OUTPUT},
                                           context=context)
        self.assertIn('OUTPUT', res)
        # output should be the layer path
        self.assertIsInstance(res['OUTPUT'], str)

        self.assertEqual(context.layersToLoadOnCompletion()[res['OUTPUT']].project, QgsProject.instance())
        layer = QgsProject.instance().mapLayer(res['OUTPUT'])
        self.assertIsInstance(layer, QgsVectorLayer)

        # Python should NOT have ownership
        self.assertFalse(sip.ispyowned(layer))

    def testProviders(self):
        """
        When run from a standalone script (like this test), ensure that the providers from separate plugins are available
        """
        providers = [p.id() for p in QgsApplication.processingRegistry().providers()]
        self.assertIn('qgis', providers)
        self.assertIn('native', providers)
        self.assertIn('gdal', providers)
        self.assertIn('project', providers)
        self.assertIn('script', providers)
        self.assertIn('model', providers)
        self.assertIn('grass7', providers)
        self.assertIn('saga', providers)
        self.assertIn('otb', providers)


if __name__ == '__main__':
    nose2.main()
