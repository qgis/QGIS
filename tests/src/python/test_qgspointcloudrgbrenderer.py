# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsPointCloudRgbRenderer

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '09/11/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import qgis  # NOQA

from qgis.core import (
    QgsProviderRegistry,
    QgsPointCloudLayer,
    QgsPointCloudRgbRenderer
)

from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

start_app()


class TestQgsPointCloudRgbRenderer(unittest.TestCase):

    @unittest.skipIf('ept' not in QgsProviderRegistry.instance().providerList(), 'EPT provider not available')
    def testSetLayer(self):
        layer = QgsPointCloudLayer(unitTestDataPath() + '/point_clouds/ept/rgb/ept.json', 'test', 'ept')
        self.assertTrue(layer.isValid())

        # test that a point cloud with RGB attributes is automatically assigned the RGB renderer by default
        self.assertIsInstance(layer.renderer(), QgsPointCloudRgbRenderer)


if __name__ == '__main__':
    unittest.main()
