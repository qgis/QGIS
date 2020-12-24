# -*- coding: utf-8 -*-
"""QGIS Unit tests for Postgres QgsQueryResultModel.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
__author__ = 'Alessandro Pasotti'
__date__ = '24/12/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
from qgis.core import (
    QgsProviderRegistry,
    QgsQueryResultModel,
    QgsAbstractDatabaseProviderConnection,
)
from qgis.testing import unittest, start_app
from qgis.PyQt.QtCore import QCoreApplication


class TestPyQgsQgsQueryResultModel(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""

        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain(cls.__name__)
        QCoreApplication.setApplicationName(cls.__name__)
        start_app()
        cls.postgres_conn = "service='qgis_test'"
        if 'QGIS_PGTEST_DB' in os.environ:
            cls.postgres_conn = os.environ['QGIS_PGTEST_DB']
        cls.uri = cls.postgres_conn + ' sslmode=disable'

    def test_model(self):
        """Test the model"""

        md = QgsProviderRegistry.instance().providerMetadata('postgres')
        conn = md.createConnection(self.uri, {})
        res = conn.execSql('SELECT generate_series(1, 1000)')
        model = QgsQueryResultModel(res)
        self.assertEqual(model.columnCount(model.index(-1)), 1)
        self.assertEqual(model.rowCount(model.index(-1)), 0)
        self.assertTrue(model.canFetchMore(model.index(-1)))

        model.fetchMore(model.index(-1))
        self.assertEqual(model.rowCount(model.index(-1)), 200)
        self.assertTrue(res.hasNextRow())

        # Fetch the rest
        for batch in range(2, 6):
            model.fetchMore(model.index(-1))
            self.assertEqual(model.rowCount(model.index(-1)), 200 * batch)

        self.assertEqual(model.rowCount(model.index(-1)), 1000)
        self.assertFalse(res.hasNextRow())
        self.assertFalse(model.canFetchMore(model.index(-1)))


if __name__ == '__main__':
    unittest.main()
