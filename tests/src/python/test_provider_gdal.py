# -*- coding: utf-8 -*-
"""Generic Unit tests for the GDAL provider.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '2018-30-10'
__copyright__ = 'Copyright 2018, Nyall Dawson'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from qgis.core import (QgsProviderRegistry,
                       QgsDataProvider)
from qgis.testing import start_app, unittest

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class PyQgsGdalProvider(unittest.TestCase):

    def testCapabilities(self):
        self.assertTrue(QgsProviderRegistry.instance().providerCapabilities("gdal") & QgsDataProvider.File)
        self.assertTrue(QgsProviderRegistry.instance().providerCapabilities("gdal") & QgsDataProvider.Dir)
        self.assertTrue(QgsProviderRegistry.instance().providerCapabilities("gdal") & QgsDataProvider.Net)


if __name__ == '__main__':
    unittest.main()
