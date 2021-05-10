# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsQueryResultWidget.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Alessandro Pasotti'
__date__ = '2021-01'
__copyright__ = 'Copyright 2021, The QGIS Project'

import os

import qgis  # NOQA
from qgis.core import QgsProviderRegistry, QgsQueryResultModel
from qgis.gui import QgsQueryResultWidget
from qgis.PyQt.QtCore import QCoreApplication
from qgis.PyQt.QtWidgets import QDialog, QLabel, QListView, QVBoxLayout
from qgis.testing import start_app, unittest
from utilities import getTestFont

start_app()


class PyQgsQueryResultWidget(unittest.TestCase):

    NUM_RECORDS = 1000000

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
        conn.executeSql('DROP TABLE IF EXISTS qgis_test.random_big_data')
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
        except Exception:
            pass

    def test_widget(self):

        md = QgsProviderRegistry.instance().providerMetadata('postgres')
        conn = md.createConnection(self.uri, {})
        widget = QgsQueryResultWidget(None, conn)
        widget.setQuery("SELECT * FROM qgis_test.random_big_data")
        d = QDialog()
        l = QVBoxLayout(d)
        d.setLayout(l)
        l.addWidget(widget)
        d.exec_()

    def test_widget_invalid(self):
        """Test it does not crash"""

        QgsQueryResultWidget(None, None)


if __name__ == '__main__':
    unittest.main()
