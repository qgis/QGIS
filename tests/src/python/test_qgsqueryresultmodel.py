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
from time import sleep
from qgis.core import (
    QgsProviderRegistry,
    QgsQueryResultModel,
    QgsAbstractDatabaseProviderConnection,
)
from qgis.testing import unittest, start_app
from qgis.PyQt.QtCore import QCoreApplication, QVariant, Qt, QTimer


class TestPyQgsQgsQueryResultModel(unittest.TestCase):

    NUM_RECORDS = 100050

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

        # Prepare data for threaded test
        cls._deleteBigData()
        md = QgsProviderRegistry.instance().providerMetadata('postgres')
        conn = md.createConnection(cls.uri, {})
        conn.executeSql('SELECT * INTO qgis_test.random_big_data FROM generate_series(1,%s) AS id, md5(random()::text) AS descr' % cls.NUM_RECORDS)

    @classmethod
    def tearDownClass(cls):

        cls._deleteBigData()

    @classmethod
    def _deleteBigData(cls):

        try:
            md = QgsProviderRegistry.instance().providerMetadata('postgres')
            conn = md.createConnection(cls.uri, {})
            conn.dropVectorTable('qgis_test', 'random_big_data')
        except:
            pass

    def test_model(self):
        """Test the model"""

        md = QgsProviderRegistry.instance().providerMetadata('postgres')
        conn = md.createConnection(self.uri, {})
        res = conn.execSql('SELECT generate_series(1, 1000)')
        model = QgsQueryResultModel(res)
        self.assertEqual(model.rowCount(model.index(-1)), 0)

        while model.rowCount(model.index(-1)) < 1000:
            QCoreApplication.processEvents()

        self.assertEqual(model.columnCount(model.index(-1)), 1)
        self.assertEqual(model.rowCount(model.index(-1)), 1000)
        self.assertEqual(model.data(model.index(999, 0), Qt.DisplayRole), 1000)

        # Test data
        for i in range(1000):
            self.assertEqual(model.data(model.index(i, 0), Qt.DisplayRole), i + 1)

        self.assertEqual(model.data(model.index(1000, 0), Qt.DisplayRole), QVariant())
        self.assertEqual(model.data(model.index(1, 1), Qt.DisplayRole), QVariant())

    def test_model_stop(self):
        """Test that when a model is deleted fetching query rows is also interrupted"""

        md = QgsProviderRegistry.instance().providerMetadata('postgres')
        conn = md.createConnection(self.uri, {})
        res = conn.execSql('SELECT * FROM qgis_test.random_big_data')

        self.model = QgsQueryResultModel(res)

        def model_deleter():
            del(self.model)

        self.running = True

        def loop_exiter():
            self.running = False

        QTimer.singleShot(0, model_deleter)
        QTimer.singleShot(1, loop_exiter)

        while self.running:
            QCoreApplication.processEvents()

        self.assertTrue(res.fetchedRowCount() > 0 and res.fetchedRowCount() < self.NUM_RECORDS)

    def test_model_concurrency(self):
        """Test concurrency: model is feching data from a separate thread"""

        # while we internally loop through features with nextRow()

        md = QgsProviderRegistry.instance().providerMetadata('postgres')
        conn = md.createConnection(self.uri, {})
        res = conn.execSql('SELECT * FROM qgis_test.random_big_data')
        model = QgsQueryResultModel(res)

        rowCount = 0
        while res.hasNextRow():
            rowCount += 1
            res.nextRow()
            QCoreApplication.processEvents()

        QCoreApplication.processEvents()

        self.assertEqual(rowCount, self.NUM_RECORDS)
        self.assertEqual(res.fetchedRowCount(), self.NUM_RECORDS)
        self.assertEqual(res.fetchedRowCount(), model.rowCount(model.index(-1)))
        self.assertEqual(res.fetchedRowCount(), len(res.rows()))

        rows = res.rows()
        for i in range(self.NUM_RECORDS):
            self.assertEqual(rows[i][0], i + 1)
            self.assertEqual(model.data(model.index(i, 0), Qt.DisplayRole), i + 1)


if __name__ == '__main__':
    unittest.main()
