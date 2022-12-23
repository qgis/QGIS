# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsOwsConnection

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
from qgis.core import (QgsOwsConnection,
                       QgsDataSourceUri,
                       QgsSettings)
from qgis.PyQt.QtCore import QCoreApplication


class TestQgsOwsConnection(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("QGIS_TestPyQgsColorScheme.com")
        QCoreApplication.setApplicationName("QGIS_TestPyQgsColorScheme")
        QgsSettings().clear()
        start_app()

        # setup some fake connections
        settings = QgsSettings()
        key = 'qgis/connections-wms/test/'
        settings.setValue(key + 'url', 'aaa.bbb.com')
        settings.setValue(key + 'referer', 'my_ref')
        settings.setValue(key + 'ignoreGetMapURI', True)
        settings.setValue(key + 'ignoreGetFeatureInfoURI', True)
        settings.setValue(key + 'smoothPixmapTransform', True)
        settings.setValue(key + 'dpiMode', 4)
        settings.setValue(key + 'ignoreAxisOrientation', True)
        settings.setValue(key + 'invertAxisOrientation', True)

        key = 'qgis/connections-wfs/test/'
        settings.setValue(key + 'url', 'ccc.ddd.com')
        settings.setValue(key + 'version', '1.1.0')
        settings.setValue(key + 'maxnumfeatures', '47')
        settings.setValue(key + 'ignoreAxisOrientation', True)
        settings.setValue(key + 'invertAxisOrientation', True)

    def testWmsConnection(self):
        c = QgsOwsConnection('WMS', 'test')
        uri = c.uri()

        self.assertEqual(uri.param('url'), 'aaa.bbb.com')
        self.assertEqual(uri.httpHeader('referer'), 'my_ref')
        self.assertEqual(uri.param('IgnoreGetMapUrl'), '1')
        self.assertEqual(uri.param('IgnoreGetFeatureInfoUrl'), '1')
        self.assertEqual(uri.param('SmoothPixmapTransform'), '1')
        self.assertEqual(uri.param('dpiMode'), '4')
        self.assertEqual(uri.param('IgnoreAxisOrientation'), '1')
        self.assertEqual(uri.param('InvertAxisOrientation'), '1')

    def testWmsSettings(self):
        uri = QgsDataSourceUri()
        QgsOwsConnection.addWmsWcsConnectionSettings(uri, 'qgis/connections-wms/test/')

        self.assertEqual(uri.httpHeader('referer'), 'my_ref')
        self.assertEqual(uri.param('IgnoreGetMapUrl'), '1')
        self.assertEqual(uri.param('IgnoreGetFeatureInfoUrl'), '1')
        self.assertEqual(uri.param('SmoothPixmapTransform'), '1')
        self.assertEqual(uri.param('dpiMode'), '4')
        self.assertEqual(uri.param('IgnoreAxisOrientation'), '1')
        self.assertEqual(uri.param('InvertAxisOrientation'), '1')

    def testWfsConnection(self):
        c = QgsOwsConnection('WFS', 'test')
        uri = c.uri()

        self.assertEqual(uri.param('url'), 'ccc.ddd.com')
        self.assertEqual(uri.param('version'), '1.1.0')
        self.assertEqual(uri.param('maxNumFeatures'), '47')
        self.assertEqual(uri.param('IgnoreAxisOrientation'), '1')
        self.assertEqual(uri.param('InvertAxisOrientation'), '1')

    def testWfsSettings(self):
        uri = QgsDataSourceUri()
        QgsOwsConnection.addWfsConnectionSettings(uri, 'qgis/connections-wfs/test/')

        self.assertEqual(uri.param('version'), '1.1.0')
        self.assertEqual(uri.param('maxNumFeatures'), '47')
        self.assertEqual(uri.param('IgnoreAxisOrientation'), '1')
        self.assertEqual(uri.param('InvertAxisOrientation'), '1')


if __name__ == "__main__":
    unittest.main()
