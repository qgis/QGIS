# -*- coding: utf-8 -*-
"""Contains tests for QgsMsSqlQueryBuilder

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '25/08/2022'
__copyright__ = 'Copyright 2022, The QGIS Project'

import os
import shutil
import tempfile

from qgis.core import (
    QgsProviderSqlQueryBuilder,
    QgsProviderRegistry
)
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

app = start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsMsSqlQueryBuilder(unittest.TestCase):

    def test_quoted_identifier(self):
        # we don't need a valid database to test this
        md = QgsProviderRegistry.instance().providerMetadata('mssql')
        conn = md.createConnection('', {})
        builder = conn.queryBuilder()
        self.assertEqual(builder.quoteIdentifier('a'), '[a]')
        self.assertEqual(builder.quoteIdentifier('a table'), '[a table]')
        self.assertEqual(builder.quoteIdentifier('a TABLE'), '[a TABLE]')
        self.assertEqual(builder.quoteIdentifier('a "TABLE"'), '[a "TABLE"]')

    def test_limit_query(self):
        # we don't need a valid database to test this
        md = QgsProviderRegistry.instance().providerMetadata('mssql')
        conn = md.createConnection('', {})
        builder = conn.queryBuilder()
        self.assertEqual(builder.createLimitQueryForTable('my_schema', 'my_table', 99), 'SELECT TOP 99 * FROM [my_schema].[my_table]')
        self.assertEqual(builder.createLimitQueryForTable(None, 'my_table', 99), 'SELECT TOP 99 * FROM [my_table]')


if __name__ == '__main__':
    unittest.main()
