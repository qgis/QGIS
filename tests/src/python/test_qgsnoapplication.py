# -*- coding: utf-8 -*-
"""QGIS Unit tests for accessing QgsApplication members with a QgsApplication instance.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '1/02/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA
import sys
from qgis.PyQt.QtCore import QCoreApplication
from qgis.core import (QgsApplication)
from qgis.testing import unittest

"""
Really important!! This test is designed to ensure that the members
which usually belong to a QgsApplication instance are still usable
when no QgsApplication instance is available (eg when using
custom editor widgets in QtDesigner). In this case QgsApplication
falls back to static members, which this test is designed to check.

So don't add start_app here or anything else which creates a
QgsApplication instance!
"""

app = QCoreApplication(sys.argv)


class TestQgsNoApplication(unittest.TestCase):

    def testMembers(self):
        self.assertTrue(QgsApplication.actionScopeRegistry())
        # self.assertTrue(QgsApplication.annotationRegistry()) NOT AVAILABLE IN BINDINGS
        self.assertTrue(QgsApplication.colorSchemeRegistry())
        self.assertTrue(QgsApplication.fieldFormatterRegistry())
        self.assertTrue(QgsApplication.gpsConnectionRegistry())
        self.assertTrue(QgsApplication.messageLog())
        self.assertTrue(QgsApplication.paintEffectRegistry())
        self.assertTrue(QgsApplication.pluginLayerRegistry())
        self.assertTrue(QgsApplication.processingRegistry())
        self.assertTrue(QgsApplication.profiler())
        # self.assertTrue(QgsApplication.rasterRendererRegistry()) NOT AVAILABLE IN BINDINGS
        self.assertTrue(QgsApplication.rendererRegistry())
        self.assertTrue(QgsApplication.svgCache())
        self.assertTrue(QgsApplication.symbolLayerRegistry())
        self.assertTrue(QgsApplication.taskManager())

    def testNullRepresentation(self):
        nr = 'my_null_value'
        QgsApplication.setNullRepresentation(nr)
        self.assertEqual(QgsApplication.nullRepresentation(), nr)

    def testAuthManager(self):
        self.assertTrue(QgsApplication.authManager())

    def testDataItemProviderRegistry(self):
        self.assertTrue(QgsApplication.dataItemProviderRegistry())

    def testInit(self):
        """
        Test calling QgsApplication.initQgis() without QgsApplication instance
        """
        QgsApplication.initQgis()


if __name__ == '__main__':
    unittest.main()
