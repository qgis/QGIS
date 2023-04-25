# -*- coding: utf-8 -*-
"""QGIS Unit tests for Redshift QgsDatabaseSchemaModel

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
__author__ = 'Marcel Bezdrighin'
__date__ = '18/02/2021'
__copyright__ = '(C) 2021 Amazon Inc. or its affiliates'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
from qgis.core import (
    QgsDatabaseSchemaModel,
    QgsProviderRegistry,
)
from qgis.PyQt.QtCore import (
    QCoreApplication,
    QModelIndex,
    Qt
)
from qgis.testing import unittest, start_app


class TestPyQgsDatabaseSchemaModelRedshift(unittest.TestCase):

    # Provider test cases must define the string URI for the test
    uri = ''
    # Provider test cases must define the provider name (e.g. "postgres" or "ogr")
    providerKey = 'redshift'

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""

        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain(cls.__name__)
        QCoreApplication.setApplicationName(cls.__name__)
        start_app()
        cls.postgres_conn = os.environ['QGIS_RSTEST_DB']
        cls.uri = cls.postgres_conn + ' sslmode=disable'

    def testModel(self):
        conn = QgsProviderRegistry.instance().providerMetadata('redshift').createConnection(self.uri, {})
        self.assertTrue(conn)
        model = QgsDatabaseSchemaModel(conn)
        self.assertGreaterEqual(model.rowCount(), 3)
        old_count = model.rowCount()
        self.assertEqual(model.columnCount(), 1)
        schemas = [model.data(model.index(r, 0, QModelIndex()), Qt.DisplayRole) for r in range(model.rowCount())]
        self.assertIn('public', schemas)
        self.assertIn('camelcaseschema', schemas)  # marcel: compared to Postgres, Redshift doesn't have case sensitive identifiers, so we make sure that the identifiers are lower cased even though at creation time they were not provided like so
        self.assertIn('qgis_test', schemas)
        self.assertEqual(model.data(model.index(schemas.index('qgis_test'), 0, QModelIndex()), Qt.ToolTipRole), 'qgis_test')
        self.assertIsNone(model.data(model.index(model.rowCount(), 0, QModelIndex()), Qt.DisplayRole))

        model.refresh()
        self.assertEqual(model.rowCount(), old_count)

        conn.createSchema('myNewSchema')
        self.assertEqual(model.rowCount(), old_count)
        model.refresh()
        self.assertEqual(model.rowCount(), old_count + 1)
        schemas = [model.data(model.index(r, 0, QModelIndex()), Qt.DisplayRole) for r in range(model.rowCount())]
        self.assertIn('public', schemas)
        self.assertIn('camelcaseschema', schemas)
        self.assertIn('qgis_test', schemas)
        self.assertIn('mynewschema', schemas)

        conn.createSchema('myNewSchEmA2')
        conn.createSchema('mynewSCHEMA3')
        model.refresh()
        self.assertEqual(model.rowCount(), old_count + 3)
        schemas = [model.data(model.index(r, 0, QModelIndex()), Qt.DisplayRole) for r in range(model.rowCount())]
        self.assertIn('public', schemas)
        self.assertIn('camelcaseschema', schemas)
        self.assertIn('qgis_test', schemas)
        self.assertIn('mynewschema', schemas)
        self.assertIn('mynewschema2', schemas)
        self.assertIn('mynewschema3', schemas)

        conn.createSchema('myNEWSCHEMA4')
        conn.dropSchema('myNEWschema2')
        conn.dropSchema('mynewsCHema')
        model.refresh()
        self.assertEqual(model.rowCount(), old_count + 2)
        schemas = [model.data(model.index(r, 0, QModelIndex()), Qt.DisplayRole) for r in range(model.rowCount())]
        self.assertIn('public', schemas)
        self.assertIn('camelcaseschema', schemas)
        self.assertIn('qgis_test', schemas)
        self.assertNotIn('mynewschema', schemas)
        self.assertNotIn('mynewschema2', schemas)
        self.assertIn('mynewschema3', schemas)
        self.assertIn('mynewschema4', schemas)

        conn.dropSchema('mynewschema3')
        conn.dropSchema('mynewschema4')
        model.refresh()
        self.assertEqual(model.rowCount(), old_count)
        schemas = [model.data(model.index(r, 0, QModelIndex()), Qt.DisplayRole) for r in range(model.rowCount())]
        self.assertIn('public', schemas)
        self.assertIn('camelcaseschema', schemas)
        self.assertIn('qgis_test', schemas)
        self.assertNotIn('mynewschema2', schemas)
        self.assertNotIn('mynewschema4', schemas)

    def test_model_allow_empty(self):
        """Test model with empty entry"""
        conn = QgsProviderRegistry.instance().providerMetadata('redshift').createConnection(self.uri, {})
        self.assertTrue(conn)
        model = QgsDatabaseSchemaModel(conn)
        self.assertGreaterEqual(model.rowCount(), 3)
        old_count = model.rowCount()
        model.setAllowEmptySchema(True)
        self.assertEqual(model.rowCount(), old_count + 1)

        schemas = [model.data(model.index(r, 0, QModelIndex()), Qt.DisplayRole) for r in range(model.rowCount())]
        self.assertIn('public', schemas)
        self.assertIn('camelcaseschema', schemas)
        self.assertIn('qgis_test', schemas)
        self.assertFalse(model.data(model.index(0, 0, QModelIndex()), Qt.DisplayRole))
        self.assertTrue(model.data(model.index(0, 0, QModelIndex()), QgsDatabaseSchemaModel.RoleEmpty))
        self.assertFalse(model.data(model.index(schemas.index('qgis_test'), 0, QModelIndex()), QgsDatabaseSchemaModel.RoleEmpty))
        self.assertIsNone(model.data(model.index(model.rowCount(), 0, QModelIndex()), Qt.DisplayRole))

        model.refresh()
        self.assertEqual(model.rowCount(), old_count + 1)

        conn.createSchema('myNewSchema')
        self.assertEqual(model.rowCount(), old_count + 1)
        model.refresh()
        self.assertEqual(model.rowCount(), old_count + 2)
        schemas = [model.data(model.index(r, 0, QModelIndex()), Qt.DisplayRole) for r in range(model.rowCount())]
        self.assertIn('public', schemas)
        self.assertIn('camelcaseschema', schemas)
        self.assertIn('qgis_test', schemas)
        self.assertIn('mynewschema', schemas)
        self.assertFalse(model.data(model.index(0, 0, QModelIndex()), Qt.DisplayRole))
        self.assertTrue(model.data(model.index(0, 0, QModelIndex()), QgsDatabaseSchemaModel.RoleEmpty))
        self.assertFalse(model.data(model.index(schemas.index('qgis_test'), 0, QModelIndex()), QgsDatabaseSchemaModel.RoleEmpty))

        model.setAllowEmptySchema(False)
        self.assertEqual(model.rowCount(), old_count + 1)
        self.assertTrue(model.data(model.index(0, 0, QModelIndex()), Qt.DisplayRole))
        self.assertFalse(model.data(model.index(0, 0, QModelIndex()), QgsDatabaseSchemaModel.RoleEmpty))
        model.setAllowEmptySchema(True)
        self.assertEqual(model.rowCount(), old_count + 2)
        self.assertFalse(model.data(model.index(0, 0, QModelIndex()), Qt.DisplayRole))
        self.assertTrue(model.data(model.index(0, 0, QModelIndex()), QgsDatabaseSchemaModel.RoleEmpty))
        self.assertFalse(model.data(model.index(schemas.index('qgis_test'), 0, QModelIndex()), QgsDatabaseSchemaModel.RoleEmpty))

        conn.createSchema('myNewSchema2')
        conn.createSchema('myNewSchEMA3')
        model.refresh()
        self.assertEqual(model.rowCount(), old_count + 4)
        schemas = [model.data(model.index(r, 0, QModelIndex()), Qt.DisplayRole) for r in range(model.rowCount())]
        self.assertIn('public', schemas)
        self.assertIn('camelcaseschema', schemas)
        self.assertIn('qgis_test', schemas)
        self.assertIn('mynewschema', schemas)
        self.assertIn('mynewschema2', schemas)
        self.assertIn('mynewschema3', schemas)
        self.assertFalse(model.data(model.index(0, 0, QModelIndex()), Qt.DisplayRole))
        self.assertTrue(model.data(model.index(0, 0, QModelIndex()), QgsDatabaseSchemaModel.RoleEmpty))
        self.assertFalse(model.data(model.index(schemas.index('qgis_test'), 0, QModelIndex()), QgsDatabaseSchemaModel.RoleEmpty))

        conn.createSchema('myNewSchema4')
        conn.dropSchema('myNewSCHEma2')
        conn.dropSchema('myNewSchema')
        model.refresh()
        self.assertEqual(model.rowCount(), old_count + 3)
        schemas = [model.data(model.index(r, 0, QModelIndex()), Qt.DisplayRole) for r in range(model.rowCount())]
        self.assertIn('public', schemas)
        self.assertIn('camelcaseschema', schemas)
        self.assertIn('qgis_test', schemas)
        self.assertNotIn('mynewschema', schemas)
        self.assertNotIn('mynewschema2', schemas)
        self.assertIn('mynewschema3', schemas)
        self.assertIn('mynewschema4', schemas)
        self.assertFalse(model.data(model.index(0, 0, QModelIndex()), Qt.DisplayRole))
        self.assertTrue(model.data(model.index(0, 0, QModelIndex()), QgsDatabaseSchemaModel.RoleEmpty))
        self.assertFalse(model.data(model.index(schemas.index('qgis_test'), 0, QModelIndex()), QgsDatabaseSchemaModel.RoleEmpty))

        conn.dropSchema('mynewschema3')
        conn.dropSchema('mynewschema4')
        model.refresh()
        self.assertEqual(model.rowCount(), old_count + 1)
        schemas = [model.data(model.index(r, 0, QModelIndex()), Qt.DisplayRole) for r in range(model.rowCount())]
        self.assertIn('public', schemas)
        self.assertIn('camelcaseschema', schemas)
        self.assertIn('qgis_test', schemas)
        self.assertNotIn('mynewschema3', schemas)
        self.assertNotIn('mynewschema4', schemas)
        self.assertFalse(model.data(model.index(0, 0, QModelIndex()), Qt.DisplayRole))
        self.assertTrue(model.data(model.index(0, 0, QModelIndex()), QgsDatabaseSchemaModel.RoleEmpty))
        self.assertFalse(model.data(model.index(schemas.index('qgis_test'), 0, QModelIndex()), QgsDatabaseSchemaModel.RoleEmpty))

        model.setAllowEmptySchema(False)
        self.assertEqual(model.rowCount(), old_count)
        self.assertTrue(model.data(model.index(0, 0, QModelIndex()), Qt.DisplayRole))
        self.assertFalse(model.data(model.index(0, 0, QModelIndex()), QgsDatabaseSchemaModel.RoleEmpty))


if __name__ == '__main__':
    unittest.main()
