# coding=utf-8
""""Test for postgres layer metadata provider

.. note:: This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

"""

__author__ = 'elpaso@itopen.it'
__date__ = '2022-08-19'
__copyright__ = 'Copyright 2022, ItOpen'

import os

from qgis.core import (
    QgsRasterLayer,
    QgsProviderRegistry,
)

from qgis.PyQt.QtCore import QCoreApplication
from qgis.testing import unittest
from qgslayermetadataprovidertestbase import LayerMetadataProviderTestBase


class TestPostgresLayerMetadataProvider(unittest.TestCase, LayerMetadataProviderTestBase):

    def getMetadataProviderId(self):

        return 'postgres'

    def getLayer(self):

        return QgsRasterLayer('{} table="qgis_test"."Raster1" (Rast)'.format(self.getConnectionUri()), "someData", 'postgresraster')

    def getConnectionUri(self) -> str:

        dbconn = 'service=qgis_test'

        if 'QGIS_PGTEST_DB' in os.environ:
            dbconn = os.environ['QGIS_PGTEST_DB']

        return dbconn

    def clearMetadataTable(self):

        self.conn.execSql('DROP TABLE IF EXISTS qgis_test.qgis_layer_metadata')
        self.conn.execSql('DROP TABLE IF EXISTS public.qgis_layer_metadata')

    def setUp(self):

        super().setUp()

        dbconn = 'service=qgis_test'

        if 'QGIS_PGTEST_DB' in os.environ:
            dbconn = os.environ['QGIS_PGTEST_DB']

        md = QgsProviderRegistry.instance().providerMetadata('postgres')
        conn = md.createConnection(self.getConnectionUri(), {})
        conn.execSql('DROP TABLE IF EXISTS qgis_test.qgis_layer_metadata')
        conn.setConfiguration({'metadataInDatabase': True})
        conn.store('PG Metadata Enabled Connection')
        self.conn = conn
        self.clearMetadataTable()


if __name__ == '__main__':
    unittest.main()
