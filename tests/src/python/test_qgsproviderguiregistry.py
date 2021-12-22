# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsProviderGuiRegistry.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Mathieu Pellerin'
__date__ = '23/11/2021'
__copyright__ = 'Copyright 2021, The QGIS Project'

import sys
import qgis  # NOQA

from qgis.gui import (
    QgsGui,
    QgsProviderGuiRegistry
)
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

# Convenience instances in case you may need them
# to find the srs.db
start_app()


class TestQgsProviderGuiRegistry(unittest.TestCase):

    def testProviderList(self):
        """
        Test provider list
        """
        providers = QgsGui.providerGuiRegistry().providerList()
        self.assertIn('ogr', providers)
        self.assertIn('gdal', providers)
        self.assertIn('wms', providers)
        self.assertIn('wms', providers)
        self.assertIn('wcs', providers)
        self.assertIn('delimitedtext', providers)
        self.assertIn('arcgisfeatureserver', providers)
        if 'WITH_SPATIALITE=TRUE' in sys.argv:
            self.assertIn('spatialite', providers)
            self.assertIn('WFS', providers)
            self.assertIn('virtual', providers)


if __name__ == '__main__':
    unittest.main(argv=['WITH_SPATIALITE'], exit=False)
