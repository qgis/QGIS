# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsGeoNodeConnection

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '12.09.2017'
__copyright__ = 'Copyright 2017, The QGIS Project'

import qgis  # NOQA

from qgis.testing import unittest, start_app
from qgis.core import (QgsGeoNodeConnectionUtils,
                       QgsGeoNodeConnection,
                       QgsDataSourceUri,
                       QgsSettings)
from qgis.PyQt.QtCore import QCoreApplication


class TestQgsGeoNodeConnection(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("QGIS_TestPyQgsColorScheme.com")
        QCoreApplication.setApplicationName("QGIS_TestPyQgsColorScheme")
        QgsSettings().clear()
        start_app()

        # setup a fake connection
        settings = QgsSettings()
        key = QgsGeoNodeConnectionUtils.pathGeoNodeConnection() + '/test/'

        settings.setValue(key + 'wms/referer', 'my_ref')
        settings.setValue(key + 'wms/ignoreGetMapURI', True)
        settings.setValue(key + 'wms/ignoreGetFeatureInfoURI', True)
        settings.setValue(key + 'wms/smoothPixmapTransform', True)
        settings.setValue(key + 'wms/dpiMode', 4)
        settings.setValue(key + 'wms/ignoreAxisOrientation', True)
        settings.setValue(key + 'wms/invertAxisOrientation', True)

        settings.setValue(key + 'wfs/version', '1.1.0')
        settings.setValue(key + 'wfs/maxnumfeatures', '47')
        settings.setValue(key + 'wfs/ignoreAxisOrientation', True)
        settings.setValue(key + 'wfs/invertAxisOrientation', True)

    def testWmsConnection(self):
        """
        Test adding GeoNode WMS related connection settings to a uri
        """
        c = QgsGeoNodeConnection('test')

        uri = c.uri()
        c.addWmsConnectionSettings(uri)

        self.assertEqual(uri.httpHeader('referer'), 'my_ref')
        self.assertEqual(uri.param('IgnoreGetMapUrl'), '1')
        self.assertEqual(uri.param('IgnoreGetFeatureInfoUrl'), '1')
        self.assertEqual(uri.param('SmoothPixmapTransform'), '1')
        self.assertEqual(uri.param('dpiMode'), '4')
        self.assertEqual(uri.param('IgnoreAxisOrientation'), '1')
        self.assertEqual(uri.param('InvertAxisOrientation'), '1')

    def testWfsConnection(self):
        """
        Test adding GeoNode WFS related connection settings to a uri
        """
        c = QgsGeoNodeConnection('test')

        uri = c.uri()
        c.addWfsConnectionSettings(uri)

        self.assertEqual(uri.param('version'), '1.1.0')
        self.assertEqual(uri.param('maxNumFeatures'), '47')
        self.assertEqual(uri.param('IgnoreAxisOrientation'), '1')
        self.assertEqual(uri.param('InvertAxisOrientation'), '1')


if __name__ == "__main__":
    unittest.main()
