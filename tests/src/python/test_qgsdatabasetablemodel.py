"""QGIS Unit tests for QgsDatabaseTableModel

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
__author__ = 'Nyall Dawson'
__date__ = '07/03/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os

from qgis.PyQt.QtCore import QCoreApplication, QModelIndex, Qt, QVariant
from qgis.core import (
    QgsCoordinateReferenceSystem,
    QgsDatabaseTableModel,
    QgsField,
    QgsFields,
    QgsProviderRegistry,
    QgsWkbTypes,
)
from qgis.testing import start_app, unittest


class TestPyQgsDatabaseTableModel(unittest.TestCase):
    # Provider test cases must define the string URI for the test
    uri = ''
    # Provider test cases must define the provider name (e.g. "postgres" or "ogr")
    providerKey = 'postgres'

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

    def testModel(self):
        conn = QgsProviderRegistry.instance().providerMetadata('postgres').createConnection(self.uri, {})
        self.assertTrue(conn)
        model = QgsDatabaseTableModel(conn)
        self.assertGreaterEqual(model.rowCount(), 3)
        old_count = model.rowCount()
        self.assertEqual(model.columnCount(), 1)
        tables = [model.data(model.index(r, 0, QModelIndex()), Qt.DisplayRole) for r in range(model.rowCount())]
        self.assertIn('qgis_test.someData', tables)
        self.assertIn('qgis_test.some_poly_data', tables)
        self.assertIn('information_schema.attributes', tables)
        self.assertEqual(model.data(model.index(tables.index('qgis_test.someData'), 0, QModelIndex()),
                                    QgsDatabaseTableModel.RoleTableName), 'someData')
        self.assertEqual(model.data(model.index(tables.index('qgis_test.someData'), 0, QModelIndex()),
                                    QgsDatabaseTableModel.RoleSchema), 'qgis_test')
        self.assertEqual(model.data(model.index(tables.index('qgis_test.someData'), 0, QModelIndex()),
                                    QgsDatabaseTableModel.RoleComment), 'QGIS Test Table')
        self.assertEqual(model.data(model.index(tables.index('qgis_test.someData'), 0, QModelIndex()),
                                    QgsDatabaseTableModel.RoleCrs), QgsCoordinateReferenceSystem('EPSG:4326'))
        self.assertEqual(model.data(model.index(tables.index('qgis_test.someData'), 0, QModelIndex()),
                                    QgsDatabaseTableModel.RoleCustomInfo), {})
        self.assertEqual(model.data(model.index(tables.index('qgis_test.someData'), 0, QModelIndex()),
                                    QgsDatabaseTableModel.RoleTableFlags), 4)
        self.assertEqual(model.data(model.index(tables.index('qgis_test.someData'), 0, QModelIndex()),
                                    QgsDatabaseTableModel.RoleWkbType), QgsWkbTypes.Point)
        self.assertEqual(model.data(model.index(tables.index('qgis_test.some_poly_data'), 0, QModelIndex()),
                                    QgsDatabaseTableModel.RoleWkbType), QgsWkbTypes.Polygon)
        self.assertIsNone(model.data(model.index(model.rowCount(), 0, QModelIndex()), Qt.DisplayRole))

        model.refresh()
        self.assertEqual(model.rowCount(), old_count)

        fields = QgsFields()
        fields.append(QgsField('test', QVariant.String))
        conn.createVectorTable('qgis_test', 'myNewTable', fields, QgsWkbTypes.Point, QgsCoordinateReferenceSystem('EPSG:3857'), False, {})
        self.assertEqual(model.rowCount(), old_count)
        model.refresh()
        self.assertEqual(model.rowCount(), old_count + 1)
        tables = [model.data(model.index(r, 0, QModelIndex()), Qt.DisplayRole) for r in range(model.rowCount())]
        self.assertIn('qgis_test.someData', tables)
        self.assertIn('qgis_test.some_poly_data', tables)
        self.assertIn('information_schema.attributes', tables)
        self.assertIn('qgis_test.myNewTable', tables)

        conn.createVectorTable('qgis_test', 'myNewTable2', fields, QgsWkbTypes.Point, QgsCoordinateReferenceSystem('EPSG:3857'), False, {})
        conn.createVectorTable('qgis_test', 'myNewTable3', fields, QgsWkbTypes.Point, QgsCoordinateReferenceSystem('EPSG:3857'), False, {})
        model.refresh()
        self.assertEqual(model.rowCount(), old_count + 3)
        tables = [model.data(model.index(r, 0, QModelIndex()), Qt.DisplayRole) for r in range(model.rowCount())]
        self.assertIn('qgis_test.someData', tables)
        self.assertIn('qgis_test.some_poly_data', tables)
        self.assertIn('information_schema.attributes', tables)
        self.assertIn('qgis_test.myNewTable', tables)
        self.assertIn('qgis_test.myNewTable2', tables)
        self.assertIn('qgis_test.myNewTable3', tables)

        conn.createVectorTable('qgis_test', 'myNewTable4', fields, QgsWkbTypes.Point, QgsCoordinateReferenceSystem('EPSG:3857'), False, {})
        conn.dropVectorTable('qgis_test', 'myNewTable2')
        conn.dropVectorTable('qgis_test', 'myNewTable')
        model.refresh()
        self.assertEqual(model.rowCount(), old_count + 2)
        tables = [model.data(model.index(r, 0, QModelIndex()), Qt.DisplayRole) for r in range(model.rowCount())]
        self.assertIn('qgis_test.someData', tables)
        self.assertIn('qgis_test.some_poly_data', tables)
        self.assertIn('information_schema.attributes', tables)
        self.assertNotIn('qgis_test.myNewTable', tables)
        self.assertNotIn('qgis_test.myNewTable2', tables)
        self.assertIn('qgis_test.myNewTable3', tables)
        self.assertIn('qgis_test.myNewTable4', tables)

        conn.dropVectorTable('qgis_test', 'myNewTable3')
        conn.dropVectorTable('qgis_test', 'myNewTable4')
        model.refresh()
        self.assertEqual(model.rowCount(), old_count)
        tables = [model.data(model.index(r, 0, QModelIndex()), Qt.DisplayRole) for r in range(model.rowCount())]
        self.assertIn('qgis_test.someData', tables)
        self.assertIn('qgis_test.some_poly_data', tables)
        self.assertIn('information_schema.attributes', tables)
        self.assertNotIn('qgis_test.myNewTable', tables)
        self.assertNotIn('qgis_test.myNewTable2', tables)
        self.assertNotIn('qgis_test.myNewTable3', tables)
        self.assertNotIn('qgis_test.myNewTable4', tables)

    def testModelSpecificSchema(self):
        conn = QgsProviderRegistry.instance().providerMetadata('postgres').createConnection(self.uri, {})
        self.assertTrue(conn)
        model = QgsDatabaseTableModel(conn, 'qgis_test')
        self.assertGreaterEqual(model.rowCount(), 3)
        old_count = model.rowCount()
        self.assertEqual(model.columnCount(), 1)
        tables = [model.data(model.index(r, 0, QModelIndex()), Qt.DisplayRole) for r in range(model.rowCount())]
        self.assertIn('someData', tables)
        self.assertIn('some_poly_data', tables)
        self.assertNotIn('attributes', tables)
        self.assertEqual(model.data(model.index(tables.index('someData'), 0, QModelIndex()),
                                    QgsDatabaseTableModel.RoleTableName), 'someData')
        self.assertEqual(model.data(model.index(tables.index('someData'), 0, QModelIndex()),
                                    QgsDatabaseTableModel.RoleSchema), 'qgis_test')
        self.assertEqual(model.data(model.index(tables.index('someData'), 0, QModelIndex()),
                                    QgsDatabaseTableModel.RoleComment), 'QGIS Test Table')
        self.assertEqual(model.data(model.index(tables.index('someData'), 0, QModelIndex()),
                                    QgsDatabaseTableModel.RoleCrs), QgsCoordinateReferenceSystem('EPSG:4326'))
        self.assertEqual(model.data(model.index(tables.index('someData'), 0, QModelIndex()),
                                    QgsDatabaseTableModel.RoleCustomInfo), {})
        self.assertEqual(model.data(model.index(tables.index('someData'), 0, QModelIndex()),
                                    QgsDatabaseTableModel.RoleTableFlags), 4)
        self.assertEqual(model.data(model.index(tables.index('someData'), 0, QModelIndex()),
                                    QgsDatabaseTableModel.RoleWkbType), QgsWkbTypes.Point)
        self.assertEqual(model.data(model.index(tables.index('some_poly_data'), 0, QModelIndex()),
                                    QgsDatabaseTableModel.RoleWkbType), QgsWkbTypes.Polygon)
        self.assertIsNone(model.data(model.index(model.rowCount(), 0, QModelIndex()), Qt.DisplayRole))

    def test_model_allow_empty(self):
        """Test model with empty entry"""
        conn = QgsProviderRegistry.instance().providerMetadata('postgres').createConnection(self.uri, {})
        self.assertTrue(conn)
        model = QgsDatabaseTableModel(conn)
        self.assertGreaterEqual(model.rowCount(), 3)
        old_count = model.rowCount()

        model.setAllowEmptyTable(True)
        self.assertTrue(model.allowEmptyTable())
        self.assertEqual(model.rowCount(), old_count + 1)
        self.assertEqual(model.columnCount(), 1)
        tables = [model.data(model.index(r, 0, QModelIndex()), Qt.DisplayRole) for r in range(model.rowCount())]
        self.assertIn('qgis_test.someData', tables)
        self.assertIn('qgis_test.some_poly_data', tables)
        self.assertIn('information_schema.attributes', tables)

        self.assertFalse(model.data(model.index(0, 0, QModelIndex()), Qt.DisplayRole))
        self.assertTrue(model.data(model.index(0, 0, QModelIndex()), QgsDatabaseTableModel.RoleEmpty))
        self.assertFalse(model.data(model.index(tables.index('qgis_test.someData'), 0, QModelIndex()), QgsDatabaseTableModel.RoleEmpty))

        self.assertEqual(model.data(model.index(tables.index('qgis_test.someData'), 0, QModelIndex()),
                                    QgsDatabaseTableModel.RoleTableName), 'someData')
        self.assertEqual(model.data(model.index(tables.index('qgis_test.someData'), 0, QModelIndex()),
                                    QgsDatabaseTableModel.RoleSchema), 'qgis_test')
        self.assertEqual(model.data(model.index(tables.index('qgis_test.someData'), 0, QModelIndex()),
                                    QgsDatabaseTableModel.RoleComment), 'QGIS Test Table')
        self.assertEqual(model.data(model.index(tables.index('qgis_test.someData'), 0, QModelIndex()),
                                    QgsDatabaseTableModel.RoleCrs), QgsCoordinateReferenceSystem('EPSG:4326'))
        self.assertEqual(model.data(model.index(tables.index('qgis_test.someData'), 0, QModelIndex()),
                                    QgsDatabaseTableModel.RoleCustomInfo), {})
        self.assertEqual(model.data(model.index(tables.index('qgis_test.someData'), 0, QModelIndex()),
                                    QgsDatabaseTableModel.RoleTableFlags), 4)
        self.assertEqual(model.data(model.index(tables.index('qgis_test.someData'), 0, QModelIndex()),
                                    QgsDatabaseTableModel.RoleWkbType), QgsWkbTypes.Point)
        self.assertEqual(model.data(model.index(tables.index('qgis_test.some_poly_data'), 0, QModelIndex()),
                                    QgsDatabaseTableModel.RoleWkbType), QgsWkbTypes.Polygon)
        self.assertIsNone(model.data(model.index(model.rowCount(), 0, QModelIndex()), Qt.DisplayRole))

        model.refresh()
        self.assertEqual(model.rowCount(), old_count + 1)

        fields = QgsFields()
        fields.append(QgsField('test', QVariant.String))
        conn.createVectorTable('qgis_test', 'myNewTable', fields, QgsWkbTypes.Point, QgsCoordinateReferenceSystem('EPSG:3857'), False, {})
        self.assertEqual(model.rowCount(), old_count + 1)

        model.refresh()
        self.assertEqual(model.rowCount(), old_count + 2)
        self.assertFalse(model.data(model.index(0, 0, QModelIndex()), Qt.DisplayRole))
        self.assertTrue(model.data(model.index(0, 0, QModelIndex()), QgsDatabaseTableModel.RoleEmpty))
        self.assertFalse(model.data(model.index(tables.index('qgis_test.someData'), 0, QModelIndex()), QgsDatabaseTableModel.RoleEmpty))

        tables = [model.data(model.index(r, 0, QModelIndex()), Qt.DisplayRole) for r in range(model.rowCount())]
        self.assertIn('qgis_test.someData', tables)
        self.assertIn('qgis_test.some_poly_data', tables)
        self.assertIn('information_schema.attributes', tables)
        self.assertIn('qgis_test.myNewTable', tables)

        model.setAllowEmptyTable(False)
        self.assertEqual(model.rowCount(), old_count + 1)
        self.assertTrue(model.data(model.index(0, 0, QModelIndex()), Qt.DisplayRole))
        self.assertFalse(model.data(model.index(0, 0, QModelIndex()), QgsDatabaseTableModel.RoleEmpty))
        model.setAllowEmptyTable(True)
        self.assertEqual(model.rowCount(), old_count + 2)
        self.assertFalse(model.data(model.index(0, 0, QModelIndex()), Qt.DisplayRole))
        self.assertTrue(model.data(model.index(0, 0, QModelIndex()), QgsDatabaseTableModel.RoleEmpty))

        conn.createVectorTable('qgis_test', 'myNewTable2', fields, QgsWkbTypes.Point, QgsCoordinateReferenceSystem('EPSG:3857'), False, {})
        conn.createVectorTable('qgis_test', 'myNewTable3', fields, QgsWkbTypes.Point, QgsCoordinateReferenceSystem('EPSG:3857'), False, {})
        model.refresh()
        self.assertEqual(model.rowCount(), old_count + 4)
        self.assertFalse(model.data(model.index(0, 0, QModelIndex()), Qt.DisplayRole))
        self.assertTrue(model.data(model.index(0, 0, QModelIndex()), QgsDatabaseTableModel.RoleEmpty))
        tables = [model.data(model.index(r, 0, QModelIndex()), Qt.DisplayRole) for r in range(model.rowCount())]
        self.assertIn('qgis_test.someData', tables)
        self.assertIn('qgis_test.some_poly_data', tables)
        self.assertIn('information_schema.attributes', tables)
        self.assertIn('qgis_test.myNewTable', tables)
        self.assertIn('qgis_test.myNewTable2', tables)
        self.assertIn('qgis_test.myNewTable3', tables)
        self.assertFalse(model.data(model.index(tables.index('qgis_test.someData'), 0, QModelIndex()), QgsDatabaseTableModel.RoleEmpty))

        conn.createVectorTable('qgis_test', 'myNewTable4', fields, QgsWkbTypes.Point, QgsCoordinateReferenceSystem('EPSG:3857'), False, {})
        conn.dropVectorTable('qgis_test', 'myNewTable2')
        conn.dropVectorTable('qgis_test', 'myNewTable')
        model.refresh()
        self.assertEqual(model.rowCount(), old_count + 3)
        self.assertFalse(model.data(model.index(0, 0, QModelIndex()), Qt.DisplayRole))
        self.assertTrue(model.data(model.index(0, 0, QModelIndex()), QgsDatabaseTableModel.RoleEmpty))

        tables = [model.data(model.index(r, 0, QModelIndex()), Qt.DisplayRole) for r in range(model.rowCount())]
        self.assertIn('qgis_test.someData', tables)
        self.assertIn('qgis_test.some_poly_data', tables)
        self.assertIn('information_schema.attributes', tables)
        self.assertNotIn('qgis_test.myNewTable', tables)
        self.assertNotIn('qgis_test.myNewTable2', tables)
        self.assertIn('qgis_test.myNewTable3', tables)
        self.assertIn('qgis_test.myNewTable4', tables)
        self.assertFalse(model.data(model.index(tables.index('qgis_test.someData'), 0, QModelIndex()), QgsDatabaseTableModel.RoleEmpty))

        conn.dropVectorTable('qgis_test', 'myNewTable3')
        conn.dropVectorTable('qgis_test', 'myNewTable4')
        model.refresh()
        self.assertEqual(model.rowCount(), old_count + 1)
        tables = [model.data(model.index(r, 0, QModelIndex()), Qt.DisplayRole) for r in range(model.rowCount())]
        self.assertIn('qgis_test.someData', tables)
        self.assertIn('qgis_test.some_poly_data', tables)
        self.assertIn('information_schema.attributes', tables)
        self.assertNotIn('qgis_test.myNewTable', tables)
        self.assertNotIn('qgis_test.myNewTable2', tables)
        self.assertNotIn('qgis_test.myNewTable3', tables)
        self.assertNotIn('qgis_test.myNewTable4', tables)
        self.assertFalse(model.data(model.index(0, 0, QModelIndex()), Qt.DisplayRole))
        self.assertTrue(model.data(model.index(0, 0, QModelIndex()), QgsDatabaseTableModel.RoleEmpty))
        self.assertFalse(model.data(model.index(tables.index('qgis_test.someData'), 0, QModelIndex()), QgsDatabaseTableModel.RoleEmpty))

        model.setAllowEmptyTable(False)
        self.assertEqual(model.rowCount(), old_count)
        self.assertTrue(model.data(model.index(0, 0, QModelIndex()), Qt.DisplayRole))
        self.assertFalse(model.data(model.index(0, 0, QModelIndex()), QgsDatabaseTableModel.RoleEmpty))


if __name__ == '__main__':
    unittest.main()
