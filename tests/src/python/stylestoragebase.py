# coding=utf-8
""""Base test for provider style DB storage

.. note:: This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

"""

__author__ = 'elpaso@itopen.it'
__date__ = '2022-11-07'
__copyright__ = 'Copyright 2022, ItOpen'

from qgis.testing import start_app, unittest
from qgis.PyQt.QtCore import QCoreApplication, QVariant
from qgis.PyQt.QtGui import QColor

from qgis.core import (
    QgsProviderRegistry,
    QgsSettings,
    QgsAbstractDatabaseProviderConnection,
    QgsField,
    QgsFields,
    QgsCoordinateReferenceSystem,
    QgsWkbTypes,
    QgsVectorLayer,
)

import time


class StyleStorageTestCaseBase(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""

        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("%s.com" % __name__)
        QCoreApplication.setApplicationName(__name__)
        QgsSettings().clear()
        start_app()


class StyleStorageTestBase():

    def layerUri(self, conn, schema_name, table_name):
        """Providers may override if they need more complex URI generation than
        what tableUri() offers"""

        return conn.tableUri(schema_name, table_name)

    def schemaName(self):
        """Providers may override (Oracle?)"""

        return 'test_styles_schema'

    def tableName(self):
        """Providers may override (Oracle?)"""

        return 'test_styles_table'

    def testMultipleStyles(self):

        md = QgsProviderRegistry.instance().providerMetadata(self.providerKey)

        conn = md.createConnection(self.uri, {})
        md.saveConnection(conn, 'qgis_test1')

        schema = None
        capabilities = conn.capabilities()

        if (capabilities & QgsAbstractDatabaseProviderConnection.CreateSchema
            and capabilities & QgsAbstractDatabaseProviderConnection.Schemas
                and capabilities & QgsAbstractDatabaseProviderConnection.DropSchema):

            schema = self.schemaName()
            # Start clean
            if schema in conn.schemas():
                conn.dropSchema(schema, True)

            # Create
            conn.createSchema(schema)
            schemas = conn.schemas()
            self.assertTrue(schema in schemas)

        elif (capabilities & QgsAbstractDatabaseProviderConnection.Schemas):
            schema = self.schemaName()

            try:
                conn.dropVectorTable(schema, self.tableName())
            except Exception:
                pass

            try:
                conn.createSchema(schema)
            except Exception:
                pass

            schemas = conn.schemas()
            self.assertTrue(schema in schemas)

        fields = QgsFields()
        fields.append(QgsField("string_t", QVariant.String))
        options = {}
        crs = QgsCoordinateReferenceSystem.fromEpsgId(4326)
        typ = QgsWkbTypes.Point

        # Create table
        conn.createVectorTable(schema, self.tableName(), fields, typ, crs, True, options)

        uri = self.layerUri(conn, schema, self.tableName())

        vl = QgsVectorLayer(uri, 'vl', self.providerKey)
        self.assertTrue(vl.isValid())
        renderer = vl.renderer()
        symbol = renderer.symbol().clone()
        symbol.setColor(QColor('#ff0000'))
        renderer.setSymbol(symbol)

        vl.saveStyleToDatabase('style1', 'style1', False, None)

        symbol = renderer.symbol().clone()
        symbol.setColor(QColor('#00ff00'))
        renderer.setSymbol(symbol)

        vl.saveStyleToDatabase('style2', 'style2', True, None)

        symbol = renderer.symbol().clone()
        symbol.setColor(QColor('#0000ff'))
        renderer.setSymbol(symbol)

        vl.saveStyleToDatabase('style3', 'style3', False, None)
        num, ids, names, desc, err = vl.listStylesInDatabase()

        self.assertTrue(set(['style2', 'style3', 'style1']).issubset(set(names)))

        del vl
        vl = QgsVectorLayer(uri, 'vl', self.providerKey)
        self.assertTrue(vl.isValid())
        renderer = vl.renderer()
        symbol = renderer.symbol()
        self.assertEqual(symbol.color().name(), '#00ff00')

        mgr = vl.styleManager()
        self.assertEqual(mgr.styles(), ['style2'])

        del vl
        options = QgsVectorLayer.LayerOptions()
        options.loadAllStoredStyles = True
        vl = QgsVectorLayer(uri, 'vl', self.providerKey, options)
        self.assertTrue(vl.isValid())
        renderer = vl.renderer()
        symbol = renderer.symbol()
        self.assertEqual(symbol.color().name(), '#00ff00')

        mgr = vl.styleManager()
        self.assertTrue(set(['style2', 'style3', 'style1']).issubset(set(names)))
