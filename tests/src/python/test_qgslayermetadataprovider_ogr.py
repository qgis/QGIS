# coding=utf-8
""""Test for ogr layer metadata provider

.. note:: This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

"""

__author__ = 'elpaso@itopen.it'
__date__ = '2022-08-19'
__copyright__ = 'Copyright 2022, ItOpen'

import os
import shutil

from qgis.core import (
    QgsVectorLayer,
    QgsProviderRegistry,
)

from qgis.PyQt.QtCore import QTemporaryDir
from qgis.testing import unittest
from qgslayermetadataprovidertestbase import LayerMetadataProviderTestBase, TEST_DATA_DIR


class TestPostgresLayerMetadataProvider(unittest.TestCase, LayerMetadataProviderTestBase):

    def getMetadataProviderId(self) -> str:

        return 'ogr'

    def getLayer(self) -> QgsVectorLayer:

        return QgsVectorLayer('{}|layername=geopackage'.format(self.getConnectionUri()), "someData", 'ogr')

    def getConnectionUri(self) -> str:

        return self.conn

    def setUp(self):

        super().setUp()
        self.temp_dir = QTemporaryDir()
        self.temp_path = self.temp_dir.path()
        srcpath = os.path.join(TEST_DATA_DIR, 'provider')
        shutil.copy(os.path.join(srcpath, 'geopackage.gpkg'), self.temp_path)
        self.conn = os.path.join(self.temp_path, 'geopackage.gpkg')
        md = QgsProviderRegistry.instance().providerMetadata('ogr')
        conn = md.createConnection(self.getConnectionUri(), {})
        conn.store('OGR Metadata Enabled Connection')


if __name__ == '__main__':
    unittest.main()
