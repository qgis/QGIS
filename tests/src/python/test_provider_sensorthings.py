"""QGIS Unit tests for the OGC SensorThings API provider.

From build dir, run: ctest -R PyQgsSensorThingsProvider -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '2023-11-08'

import hashlib
import tempfile
import unittest

from qgis.PyQt.QtCore import (
    QCoreApplication
)
from qgis.core import (
    QgsProviderRegistry,
    QgsRectangle,
    QgsSettings,
)
from qgis.testing import start_app, QgisTestCase


def sanitize(endpoint, x):
    if x.startswith('/query'):
        x = x[len('/query'):]
        endpoint = endpoint + '_query'

    if len(endpoint + x) > 150:
        ret = endpoint + hashlib.md5(x.encode()).hexdigest()
        # print('Before: ' + endpoint + x)
        # print('After:  ' + ret)
        return ret
    return endpoint + x.replace('?', '_').replace('&', '_').replace('<', '_').replace('>', '_').replace('"',
                                                                                                        '_').replace(
        "'", '_').replace(' ', '_').replace(':', '_').replace('/', '_').replace('\n', '_')


class TestPyQgsSensorThingsProvider(QgisTestCase):  # , ProviderTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super(TestPyQgsSensorThingsProvider, cls).setUpClass()

        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("TestPyQgsSensorThingsProvider.com")
        QCoreApplication.setApplicationName("TestPyQgsSensorThingsProvider")
        QgsSettings().clear()
        start_app()

        # On Windows we must make sure that any backslash in the path is
        # replaced by a forward slash so that QUrl can process it
        cls.basetestpath = tempfile.mkdtemp().replace('\\', '/')
        endpoint = cls.basetestpath + '/fake_qgis_http_endpoint'

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        QgsSettings().clear()
        # shutil.rmtree(cls.basetestpath, True)
        cls.vl = None  # so as to properly close the provider and remove any temporary file
        super().tearDownClass()

    def testDecodeUri(self):
        """
        Test decoding a SensorThings uri
        """
        uri = "url='https://sometest.com/api' authcfg='abc'"
        parts = QgsProviderRegistry.instance().decodeUri('sensorthings', uri)
        self.assertEqual(parts, {
            'url': 'https://sometest.com/api',
            'authcfg': 'abc'})

    def testEncodeUri(self):
        """
        Test encoding a SensorThings uri
        """
        parts = {'url': 'http://blah.com', 'authcfg': 'aaaaa'}
        uri = QgsProviderRegistry.instance().encodeUri('sensorthings', parts)
        self.assertEqual(uri, "authcfg=aaaaa url='http://blah.com'")


if __name__ == '__main__':
    unittest.main()
