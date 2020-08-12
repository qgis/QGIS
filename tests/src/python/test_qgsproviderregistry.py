# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsProviderRegistry.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '16/03/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import qgis  # NOQA

from qgis.core import QgsProviderRegistry
from qgis.testing import start_app, unittest

# Convenience instances in case you may need them
# to find the srs.db
start_app()


class TestQgsProviderRegistry(unittest.TestCase):

    def testProviderList(self):
        """
        Test provider list
        """

        providers = QgsProviderRegistry.instance().providerList()
        self.assertIn('ogr', providers)
        self.assertIn('gdal', providers)

    def testProviderMetadata(self):
        """
        Test retrieving provider metadata
        """

        providers = QgsProviderRegistry.instance().providerList()
        for p in providers:
            self.assertTrue(QgsProviderRegistry.instance().providerMetadata(p))
            # should be case-insensitive
            self.assertTrue(QgsProviderRegistry.instance().providerMetadata(p.lower()))
            self.assertTrue(QgsProviderRegistry.instance().providerMetadata(p.upper()))

        self.assertIsNone(QgsProviderRegistry.instance().providerMetadata('asdasdasdasdasd'))

    def testCreateProvider(self):
        """
        Test creating provider instance
        """
        providers = QgsProviderRegistry.instance().providerList()
        for p in providers:
            if p == 'geonode' or p == 'vectortile':
                continue

            self.assertTrue(QgsProviderRegistry.instance().createProvider(p, ''))
            # should be case-insensitive
            self.assertTrue(QgsProviderRegistry.instance().createProvider(p.lower(), ''))
            self.assertTrue(QgsProviderRegistry.instance().createProvider(p.upper(), ''))

        self.assertIsNone(QgsProviderRegistry.instance().createProvider('asdasdasdasdasd', ''))


if __name__ == '__main__':
    unittest.main()
