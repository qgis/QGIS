# -*- coding: utf-8 -*-
"""QGIS Unit tests for the DBManager GPKG plugin

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Even Rouault'
__date__ = '2016-10-17'
__copyright__ = 'Copyright 2016, Even Rouault'

import qgis  # NOQA

import os
import tempfile
import shutil
from osgeo import gdal, ogr, osr

from qgis.core import QgsDataSourceUri, QgsSettings
from qgis.PyQt.QtCore import QCoreApplication
from qgis.testing import start_app, unittest

from plugins.db_manager.db_plugins import supportedDbTypes, createDbPlugin
from plugins.db_manager.db_plugins.plugin import TableField

from utilities import unitTestDataPath


def GDAL_COMPUTE_VERSION(maj, min, rev):
    return ((maj) * 1000000 + (min) * 10000 + (rev) * 100)


class TestPyQgsDBManagerGpkg(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""

        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("TestPyQgsDBManagerGpkg.com")
        QCoreApplication.setApplicationName("TestPyQgsDBManagerGpkg")
        QgsSettings().clear()
        start_app()

        cls.basetestpath = tempfile.mkdtemp()

        cls.test_gpkg = os.path.join(cls.basetestpath, 'TestPyQgsDBManagerGpkg.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(cls.test_gpkg)
        lyr = ds.CreateLayer('testLayer', geom_type=ogr.wkbLineString, options=['SPATIAL_INDEX=NO'])
        cls.supportsAlterFieldDefn = lyr.TestCapability(ogr.OLCAlterFieldDefn) == 1
        lyr.CreateField(ogr.FieldDefn('text_field', ogr.OFTString))
        f = ogr.Feature(lyr.GetLayerDefn())
        f['text_field'] = 'foo'
        f.SetGeometry(ogr.CreateGeometryFromWkt('LINESTRING(1 2,3 4)'))
        lyr.CreateFeature(f)
        f = None
        ds = None

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""

        QgsSettings().clear()
        shutil.rmtree(cls.basetestpath, True)

    def testSupportedDbTypes(self):
        self.assertIn('gpkg', supportedDbTypes())

    def testCreateDbPlugin(self):
        plugin = createDbPlugin('gpkg')
        self.assertIsNotNone(plugin)

    def testConnect(self):
        connection_name = 'testConnect'
        plugin = createDbPlugin('gpkg')

        uri = QgsDataSourceUri()
        uri.setDatabase(self.test_gpkg)
        self.assertTrue(plugin.addConnection(connection_name, uri))

        connections = plugin.connections()
        self.assertEqual(len(connections), 1)

        connection = createDbPlugin('gpkg', connection_name + '_does_not_exist')
        connection_succeeded = False
        try:
            connection.connect()
            connection_succeeded = True
        except:
            pass
        self.assertFalse(connection_succeeded, 'exception should have been raised')

        connection = connections[0]
        connection.connect()

        connection.reconnect()

        connection.remove()

        self.assertEqual(len(plugin.connections()), 0)

        connection = createDbPlugin('gpkg', connection_name)
        connection_succeeded = False
        try:
            connection.connect()
            connection_succeeded = True
        except:
            pass
        self.assertFalse(connection_succeeded, 'exception should have been raised')

    def testListLayer(self):
        connection_name = 'testListLayer'
        plugin = createDbPlugin('gpkg')
        uri = QgsDataSourceUri()
        uri.setDatabase(self.test_gpkg)
        self.assertTrue(plugin.addConnection(connection_name, uri))

        connection = createDbPlugin('gpkg', connection_name)
        connection.connect()

        db = connection.database()
        self.assertIsNotNone(db)

        tables = db.tables()
        self.assertEqual(len(tables), 1)
        table = tables[0]
        self.assertEqual(table.name, 'testLayer')
        info = table.info()
        expected_html = """<div class="section"><h2>General info</h2><div><table><tr><td>Relation type:&nbsp;</td><td>Table&nbsp;</td></tr><tr><td>Rows:&nbsp;</td><td>1&nbsp;</td></tr></table></div></div><div class="section"><h2>GeoPackage</h2><div><table><tr><td>Column:&nbsp;</td><td>geom&nbsp;</td></tr><tr><td>Geometry:&nbsp;</td><td>LINESTRING&nbsp;</td></tr><tr><td>Dimension:&nbsp;</td><td>XY&nbsp;</td></tr><tr><td>Spatial ref:&nbsp;</td><td>Undefined (-1)&nbsp;</td></tr><tr><td>Extent:&nbsp;</td><td>1.00000, 2.00000 - 3.00000, 4.00000&nbsp;</td></tr></table><p><warning> No spatial index defined (<a href="action:spatialindex/create">create it</a>)</p></div></div><div class="section"><h2>Fields</h2><div><table class="header"><tr><th>#&nbsp;</th><th>Name&nbsp;</th><th>Type&nbsp;</th><th>Null&nbsp;</th><th>Default&nbsp;</th></tr><tr><td>0&nbsp;</td><td class="underline">fid&nbsp;</td><td>INTEGER&nbsp;</td><td>Y&nbsp;</td><td>&nbsp;</td></tr><tr><td>1&nbsp;</td><td>geom&nbsp;</td><td>LINESTRING&nbsp;</td><td>Y&nbsp;</td><td>&nbsp;</td></tr><tr><td>2&nbsp;</td><td>text_field&nbsp;</td><td>TEXT&nbsp;</td><td>Y&nbsp;</td><td>&nbsp;</td></tr></table></div></div>"""

        # GDAL 2.2.0
        expected_html_2 = """<div class="section"><h2>General info</h2><div><table><tr><td>Relation type:&nbsp;</td><td>Table&nbsp;</td></tr><tr><td>Rows:&nbsp;</td><td>1&nbsp;</td></tr></table></div></div><div class="section"><h2>GeoPackage</h2><div><table><tr><td>Column:&nbsp;</td><td>geom&nbsp;</td></tr><tr><td>Geometry:&nbsp;</td><td>LINESTRING&nbsp;</td></tr><tr><td>Dimension:&nbsp;</td><td>XY&nbsp;</td></tr><tr><td>Spatial ref:&nbsp;</td><td>Undefined (-1)&nbsp;</td></tr><tr><td>Extent:&nbsp;</td><td>1.00000, 2.00000 - 3.00000, 4.00000&nbsp;</td></tr></table><p><warning> No spatial index defined (<a href="action:spatialindex/create">create it</a>)</p></div></div><div class="section"><h2>Fields</h2><div><table class="header"><tr><th>#&nbsp;</th><th>Name&nbsp;</th><th>Type&nbsp;</th><th>Null&nbsp;</th><th>Default&nbsp;</th></tr><tr><td>0&nbsp;</td><td class="underline">fid&nbsp;</td><td>INTEGER&nbsp;</td><td>N&nbsp;</td><td>&nbsp;</td></tr><tr><td>1&nbsp;</td><td>geom&nbsp;</td><td>LINESTRING&nbsp;</td><td>Y&nbsp;</td><td>&nbsp;</td></tr><tr><td>2&nbsp;</td><td>text_field&nbsp;</td><td>TEXT&nbsp;</td><td>Y&nbsp;</td><td>&nbsp;</td></tr></table></div></div><div class="section"><h2>Triggers</h2><div><table class="header"><tr><th>Name&nbsp;</th><th>Function&nbsp;</th></tr><tr><td>trigger_insert_feature_count_testLayer (<a href="action:trigger/trigger_insert_feature_count_testLayer/delete">delete</a>)&nbsp;</td><td>CREATE TRIGGER "trigger_insert_feature_count_testLayer" AFTER INSERT ON "testLayer" BEGIN UPDATE gpkg_ogr_contents SET feature_count = feature_count + 1 WHERE table_name = 'testLayer'; END&nbsp;</td></tr><tr><td>trigger_delete_feature_count_testLayer (<a href="action:trigger/trigger_delete_feature_count_testLayer/delete">delete</a>)&nbsp;</td><td>CREATE TRIGGER "trigger_delete_feature_count_testLayer" AFTER DELETE ON "testLayer" BEGIN UPDATE gpkg_ogr_contents SET feature_count = feature_count - 1 WHERE table_name = 'testLayer'; END&nbsp;</td></tr></table></div></div>"""

        # GDAL 2.3.0
        expected_html_3 = """<div class="section"><h2>General info</h2><div><table><tr><td>Relation type:&nbsp;</td><td>Table&nbsp;</td></tr><tr><td>Rows:&nbsp;</td><td>1&nbsp;</td></tr></table></div></div><div class="section"><h2>GeoPackage</h2><div><table><tr><td>Column:&nbsp;</td><td>geom&nbsp;</td></tr><tr><td>Geometry:&nbsp;</td><td>LINESTRING&nbsp;</td></tr><tr><td>Dimension:&nbsp;</td><td>XY&nbsp;</td></tr><tr><td>Spatial ref:&nbsp;</td><td>Undefined (-1)&nbsp;</td></tr><tr><td>Extent:&nbsp;</td><td>1.00000, 2.00000 - 3.00000, 4.00000&nbsp;</td></tr></table><p><warning> No spatial index defined (<a href="action:spatialindex/create">create it</a>)</p></div></div><div class="section"><h2>Fields</h2><div><table class="header"><tr><th>#&nbsp;</th><th>Name&nbsp;</th><th>Type&nbsp;</th><th>Null&nbsp;</th><th>Default&nbsp;</th></tr><tr><td>0&nbsp;</td><td class="underline">fid&nbsp;</td><td>INTEGER&nbsp;</td><td>N&nbsp;</td><td>&nbsp;</td></tr><tr><td>1&nbsp;</td><td>geom&nbsp;</td><td>LINESTRING&nbsp;</td><td>Y&nbsp;</td><td>&nbsp;</td></tr><tr><td>2&nbsp;</td><td>text_field&nbsp;</td><td>TEXT&nbsp;</td><td>Y&nbsp;</td><td>&nbsp;</td></tr></table></div></div><div class="section"><h2>Triggers</h2><div><table class="header"><tr><th>Name&nbsp;</th><th>Function&nbsp;</th></tr><tr><td>trigger_insert_feature_count_testLayer (<a href="action:trigger/trigger_insert_feature_count_testLayer/delete">delete</a>)&nbsp;</td><td>CREATE TRIGGER "trigger_insert_feature_count_testLayer" AFTER INSERT ON "testLayer" BEGIN UPDATE gpkg_ogr_contents SET feature_count = feature_count + 1 WHERE lower(table_name) = lower('testLayer'); END&nbsp;</td></tr><tr><td>trigger_delete_feature_count_testLayer (<a href="action:trigger/trigger_delete_feature_count_testLayer/delete">delete</a>)&nbsp;</td><td>CREATE TRIGGER "trigger_delete_feature_count_testLayer" AFTER DELETE ON "testLayer" BEGIN UPDATE gpkg_ogr_contents SET feature_count = feature_count - 1 WHERE lower(table_name) = lower('testLayer'); END&nbsp;</td></tr></table></div></div>"""

        self.assertIn(info.toHtml(), [expected_html, expected_html_2, expected_html_3])

        connection.remove()

    @unittest.skipIf(os.environ.get('TRAVIS', '') == 'true', 'Test flaky') # see https://travis-ci.org/qgis/QGIS/jobs/502556996
    def testCreateRenameDeleteTable(self):
        connection_name = 'testCreateRenameDeleteTable'
        plugin = createDbPlugin('gpkg')
        uri = QgsDataSourceUri()

        test_gpkg_new = os.path.join(self.basetestpath, 'testCreateRenameDeleteTable.gpkg')
        shutil.copy(self.test_gpkg, test_gpkg_new)

        uri.setDatabase(test_gpkg_new)
        self.assertTrue(plugin.addConnection(connection_name, uri))

        connection = createDbPlugin('gpkg', connection_name)
        connection.connect()

        db = connection.database()
        self.assertIsNotNone(db)

        tables = db.tables()
        self.assertEqual(len(tables), 1)
        table = tables[0]
        self.assertTrue(table.rename('newName'))
        self.assertEqual(table.name, 'newName')

        connection.reconnect()

        db = connection.database()
        tables = db.tables()
        self.assertEqual(len(tables), 1)
        table = tables[0]
        self.assertEqual(table.name, 'newName')

        fields = []
        geom = ['geometry', 'POINT', 4326, 3]
        field1 = TableField(table)
        field1.name = 'fid'
        field1.dataType = 'INTEGER'
        field1.notNull = True
        field1.primaryKey = True

        field2 = TableField(table)
        field2.name = 'str_field'
        field2.dataType = 'TEXT'
        field2.modifier = 20

        fields = [field1, field2]
        self.assertTrue(db.createVectorTable('newName2', fields, geom))

        tables = db.tables()
        self.assertEqual(len(tables), 2)
        new_table = tables[1]
        self.assertEqual(new_table.name, 'newName2')
        fields = new_table.fields()
        self.assertEqual(len(fields), 3)
        self.assertFalse(new_table.hasSpatialIndex())

        self.assertTrue(new_table.createSpatialIndex())
        self.assertTrue(new_table.hasSpatialIndex())

        # Test delete spatial index
        self.assertTrue(new_table.deleteSpatialIndex())
        self.assertFalse(new_table.hasSpatialIndex())

        self.assertTrue(new_table.delete())

        tables = db.tables()
        self.assertEqual(len(tables), 1)

        connection.remove()

    def testCreateRenameDeleteFields(self):

        if not self.supportsAlterFieldDefn:
            return

        connection_name = 'testCreateRenameDeleteFields'
        plugin = createDbPlugin('gpkg')
        uri = QgsDataSourceUri()

        test_gpkg_new = os.path.join(self.basetestpath, 'testCreateRenameDeleteFields.gpkg')
        shutil.copy(self.test_gpkg, test_gpkg_new)

        uri.setDatabase(test_gpkg_new)
        self.assertTrue(plugin.addConnection(connection_name, uri))

        connection = createDbPlugin('gpkg', connection_name)
        connection.connect()

        db = connection.database()
        self.assertIsNotNone(db)

        tables = db.tables()
        self.assertEqual(len(tables), 1)
        table = tables[0]

        field_before_count = len(table.fields())

        field = TableField(table)
        field.name = 'real_field'
        field.dataType = 'DOUBLE'
        self.assertTrue(table.addField(field))

        self.assertEqual(len(table.fields()), field_before_count + 1)

        self.assertTrue(field.update('real_field2', new_type_str='TEXT (30)', new_not_null=True, new_default_str='foo'))

        field = table.fields()[field_before_count]
        self.assertEqual(field.name, 'real_field2')
        self.assertEqual(field.dataType, 'TEXT(30)')
        self.assertEqual(field.notNull, 1)
        self.assertEqual(field.default, "'foo'")

        self.assertTrue(table.deleteField(field))

        self.assertEqual(len(table.fields()), field_before_count)

        connection.remove()

    def testTableDataModel(self):
        connection_name = 'testTableDataModel'
        plugin = createDbPlugin('gpkg')
        uri = QgsDataSourceUri()
        uri.setDatabase(self.test_gpkg)
        self.assertTrue(plugin.addConnection(connection_name, uri))

        connection = createDbPlugin('gpkg', connection_name)
        connection.connect()

        db = connection.database()
        self.assertIsNotNone(db)

        tables = db.tables()
        self.assertEqual(len(tables), 1)
        table = tables[0]
        self.assertEqual(table.name, 'testLayer')
        model = table.tableDataModel(None)
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.getData(0, 0), 1)  # fid
        self.assertEqual(model.getData(0, 1), 'LINESTRING (1 2,3 4)')
        self.assertEqual(model.getData(0, 2), 'foo')

        connection.remove()

    def testRaster(self):

        if int(gdal.VersionInfo('VERSION_NUM')) < GDAL_COMPUTE_VERSION(2, 0, 2):
            return

        connection_name = 'testRaster'
        plugin = createDbPlugin('gpkg')
        uri = QgsDataSourceUri()

        test_gpkg_new = os.path.join(self.basetestpath, 'testRaster.gpkg')
        shutil.copy(self.test_gpkg, test_gpkg_new)
        mem_ds = gdal.GetDriverByName('MEM').Create('', 20, 20)
        mem_ds.SetGeoTransform([2, 0.01, 0, 49, 0, -0.01])
        sr = osr.SpatialReference()
        sr.ImportFromEPSG(4326)
        mem_ds.SetProjection(sr.ExportToWkt())
        mem_ds.GetRasterBand(1).Fill(255)
        gdal.GetDriverByName('GPKG').CreateCopy(test_gpkg_new, mem_ds, options=['APPEND_SUBDATASET=YES', 'RASTER_TABLE=raster_table'])
        mem_ds = None

        uri.setDatabase(test_gpkg_new)
        self.assertTrue(plugin.addConnection(connection_name, uri))

        connection = createDbPlugin('gpkg', connection_name)
        connection.connect()

        db = connection.database()
        self.assertIsNotNone(db)

        tables = db.tables()
        self.assertEqual(len(tables), 2)
        table = None
        for i in range(2):
            if tables[i].name == 'raster_table':
                table = tables[i]
                break
        self.assertIsNotNone(table)
        info = table.info()
        expected_html = """<div class="section"><h2>General info</h2><div><table><tr><td>Relation type:&nbsp;</td><td>Table&nbsp;</td></tr><tr><td>Rows:&nbsp;</td><td>Unknown (<a href="action:rows/count">find out</a>)&nbsp;</td></tr></table></div></div><div class="section"><h2>GeoPackage</h2><div><table><tr><td>Column:&nbsp;</td><td>&nbsp;</td></tr><tr><td>Geometry:&nbsp;</td><td>RASTER&nbsp;</td></tr><tr><td>Spatial ref:&nbsp;</td><td>WGS 84 (4326)&nbsp;</td></tr><tr><td>Extent:&nbsp;</td><td>2.00000, 48.80000 - 2.20000, 49.00000&nbsp;</td></tr></table></div></div><div class="section"><h2>Fields</h2><div><table class="header"><tr><th>#&nbsp;</th><th>Name&nbsp;</th><th>Type&nbsp;</th><th>Null&nbsp;</th><th>Default&nbsp;</th></tr><tr><td>0&nbsp;</td><td class="underline">id&nbsp;</td><td>INTEGER&nbsp;</td><td>Y&nbsp;</td><td>&nbsp;</td></tr><tr><td>1&nbsp;</td><td>zoom_level&nbsp;</td><td>INTEGER&nbsp;</td><td>N&nbsp;</td><td>&nbsp;</td></tr><tr><td>2&nbsp;</td><td>tile_column&nbsp;</td><td>INTEGER&nbsp;</td><td>N&nbsp;</td><td>&nbsp;</td></tr><tr><td>3&nbsp;</td><td>tile_row&nbsp;</td><td>INTEGER&nbsp;</td><td>N&nbsp;</td><td>&nbsp;</td></tr><tr><td>4&nbsp;</td><td>tile_data&nbsp;</td><td>BLOB&nbsp;</td><td>N&nbsp;</td><td>&nbsp;</td></tr></table></div></div><div class="section"><h2>Indexes</h2><div><table class="header"><tr><th>Name&nbsp;</th><th>Column(s)&nbsp;</th></tr><tr><td>sqlite_autoindex_raster_table_1&nbsp;</td><td>zoom_level<br>tile_column<br>tile_row&nbsp;</td></tr></table></div></div>"""

        self.assertEqual(info.toHtml(), expected_html)

        connection.remove()

    def testTwoRaster(self):

        if int(gdal.VersionInfo('VERSION_NUM')) < GDAL_COMPUTE_VERSION(2, 0, 2):
            return

        connection_name = 'testTwoRaster'
        plugin = createDbPlugin('gpkg')
        uri = QgsDataSourceUri()

        test_gpkg_new = os.path.join(self.basetestpath, 'testTwoRaster.gpkg')
        shutil.copy(self.test_gpkg, test_gpkg_new)
        mem_ds = gdal.GetDriverByName('MEM').Create('', 20, 20)
        mem_ds.SetGeoTransform([2, 0.01, 0, 49, 0, -0.01])
        sr = osr.SpatialReference()
        sr.ImportFromEPSG(4326)
        mem_ds.SetProjection(sr.ExportToWkt())
        mem_ds.GetRasterBand(1).Fill(255)
        for i in range(2):
            gdal.GetDriverByName('GPKG').CreateCopy(test_gpkg_new, mem_ds, options=['APPEND_SUBDATASET=YES', 'RASTER_TABLE=raster_table%d' % (i + 1)])
        mem_ds = None

        uri.setDatabase(test_gpkg_new)
        self.assertTrue(plugin.addConnection(connection_name, uri))

        connection = createDbPlugin('gpkg', connection_name)
        connection.connect()

        db = connection.database()
        self.assertIsNotNone(db)

        tables = db.tables()
        self.assertEqual(len(tables), 3)
        table = None
        for i in range(2):
            if tables[i].name.startswith('raster_table'):
                table = tables[i]
                info = table.info()
                info.toHtml()

        connection.remove()

    def testNonSpatial(self):

        connection_name = 'testNonSpatial'
        plugin = createDbPlugin('gpkg')
        uri = QgsDataSourceUri()

        test_gpkg = os.path.join(self.basetestpath, 'testNonSpatial.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(test_gpkg)
        lyr = ds.CreateLayer('testNonSpatial', geom_type=ogr.wkbNone)
        lyr.CreateField(ogr.FieldDefn('text_field', ogr.OFTString))
        f = ogr.Feature(lyr.GetLayerDefn())
        f['text_field'] = 'foo'
        lyr.CreateFeature(f)
        f = None
        ds = None

        uri.setDatabase(test_gpkg)
        self.assertTrue(plugin.addConnection(connection_name, uri))

        connection = createDbPlugin('gpkg', connection_name)
        connection.connect()

        db = connection.database()
        self.assertIsNotNone(db)

        tables = db.tables()
        self.assertEqual(len(tables), 1)
        table = tables[0]
        self.assertEqual(table.name, 'testNonSpatial')
        info = table.info()
        expected_html = """<div class="section"><h2>General info</h2><div><table><tr><td>Relation type:&nbsp;</td><td>Table&nbsp;</td></tr><tr><td>Rows:&nbsp;</td><td>1&nbsp;</td></tr></table></div></div><div class="section"><h2>Fields</h2><div><table class="header"><tr><th>#&nbsp;</th><th>Name&nbsp;</th><th>Type&nbsp;</th><th>Null&nbsp;</th><th>Default&nbsp;</th></tr><tr><td>0&nbsp;</td><td class="underline">fid&nbsp;</td><td>INTEGER&nbsp;</td><td>Y&nbsp;</td><td>&nbsp;</td></tr><tr><td>1&nbsp;</td><td>text_field&nbsp;</td><td>TEXT&nbsp;</td><td>Y&nbsp;</td><td>&nbsp;</td></tr></table></div></div>"""

        # GDAL 2.2.0
        expected_html_2 = """<div class="section"><h2>General info</h2><div><table><tr><td>Relation type:&nbsp;</td><td>Table&nbsp;</td></tr><tr><td>Rows:&nbsp;</td><td>1&nbsp;</td></tr></table></div></div><div class="section"><h2>Fields</h2><div><table class="header"><tr><th>#&nbsp;</th><th>Name&nbsp;</th><th>Type&nbsp;</th><th>Null&nbsp;</th><th>Default&nbsp;</th></tr><tr><td>0&nbsp;</td><td class="underline">fid&nbsp;</td><td>INTEGER&nbsp;</td><td>N&nbsp;</td><td>&nbsp;</td></tr><tr><td>1&nbsp;</td><td>text_field&nbsp;</td><td>TEXT&nbsp;</td><td>Y&nbsp;</td><td>&nbsp;</td></tr></table></div></div><div class="section"><h2>Triggers</h2><div><table class="header"><tr><th>Name&nbsp;</th><th>Function&nbsp;</th></tr><tr><td>trigger_insert_feature_count_testNonSpatial (<a href="action:trigger/trigger_insert_feature_count_testNonSpatial/delete">delete</a>)&nbsp;</td><td>CREATE TRIGGER "trigger_insert_feature_count_testNonSpatial" AFTER INSERT ON "testNonSpatial" BEGIN UPDATE gpkg_ogr_contents SET feature_count = feature_count + 1 WHERE table_name = 'testNonSpatial'; END&nbsp;</td></tr><tr><td>trigger_delete_feature_count_testNonSpatial (<a href="action:trigger/trigger_delete_feature_count_testNonSpatial/delete">delete</a>)&nbsp;</td><td>CREATE TRIGGER "trigger_delete_feature_count_testNonSpatial" AFTER DELETE ON "testNonSpatial" BEGIN UPDATE gpkg_ogr_contents SET feature_count = feature_count - 1 WHERE table_name = 'testNonSpatial'; END&nbsp;</td></tr></table></div></div>"""

        # GDAL 2.3.0
        expected_html_3 = """<div class="section"><h2>General info</h2><div><table><tr><td>Relation type:&nbsp;</td><td>Table&nbsp;</td></tr><tr><td>Rows:&nbsp;</td><td>1&nbsp;</td></tr></table></div></div><div class="section"><h2>Fields</h2><div><table class="header"><tr><th>#&nbsp;</th><th>Name&nbsp;</th><th>Type&nbsp;</th><th>Null&nbsp;</th><th>Default&nbsp;</th></tr><tr><td>0&nbsp;</td><td class="underline">fid&nbsp;</td><td>INTEGER&nbsp;</td><td>N&nbsp;</td><td>&nbsp;</td></tr><tr><td>1&nbsp;</td><td>text_field&nbsp;</td><td>TEXT&nbsp;</td><td>Y&nbsp;</td><td>&nbsp;</td></tr></table></div></div><div class="section"><h2>Triggers</h2><div><table class="header"><tr><th>Name&nbsp;</th><th>Function&nbsp;</th></tr><tr><td>trigger_insert_feature_count_testNonSpatial (<a href="action:trigger/trigger_insert_feature_count_testNonSpatial/delete">delete</a>)&nbsp;</td><td>CREATE TRIGGER "trigger_insert_feature_count_testNonSpatial" AFTER INSERT ON "testNonSpatial" BEGIN UPDATE gpkg_ogr_contents SET feature_count = feature_count + 1 WHERE lower(table_name) = lower('testNonSpatial'); END&nbsp;</td></tr><tr><td>trigger_delete_feature_count_testNonSpatial (<a href="action:trigger/trigger_delete_feature_count_testNonSpatial/delete">delete</a>)&nbsp;</td><td>CREATE TRIGGER "trigger_delete_feature_count_testNonSpatial" AFTER DELETE ON "testNonSpatial" BEGIN UPDATE gpkg_ogr_contents SET feature_count = feature_count - 1 WHERE lower(table_name) = lower('testNonSpatial'); END&nbsp;</td></tr></table></div></div>"""
        self.assertIn(info.toHtml(), [expected_html, expected_html_2, expected_html_3], info.toHtml())

        connection.remove()

    def testAllGeometryTypes(self):

        connection_name = 'testAllGeometryTypes'
        plugin = createDbPlugin('gpkg')
        uri = QgsDataSourceUri()

        test_gpkg = os.path.join(self.basetestpath, 'testAllGeometryTypes.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(test_gpkg)
        ds.CreateLayer('testPoint', geom_type=ogr.wkbPoint)
        ds.CreateLayer('testLineString', geom_type=ogr.wkbLineString)
        ds.CreateLayer('testPolygon', geom_type=ogr.wkbPolygon)
        ds.CreateLayer('testMultiPoint', geom_type=ogr.wkbMultiPoint)
        ds.CreateLayer('testMultiLineString', geom_type=ogr.wkbMultiLineString)
        ds.CreateLayer('testMultiPolygon', geom_type=ogr.wkbMultiPolygon)
        ds.CreateLayer('testGeometryCollection', geom_type=ogr.wkbGeometryCollection)
        ds.CreateLayer('testCircularString', geom_type=ogr.wkbCircularString)
        ds.CreateLayer('testCompoundCurve', geom_type=ogr.wkbCompoundCurve)
        ds.CreateLayer('testCurvePolygon', geom_type=ogr.wkbCurvePolygon)
        ds.CreateLayer('testMultiCurve', geom_type=ogr.wkbMultiCurve)
        ds.CreateLayer('testMultiSurface', geom_type=ogr.wkbMultiSurface)
        ds = None

        uri.setDatabase(test_gpkg)
        self.assertTrue(plugin.addConnection(connection_name, uri))

        connection = createDbPlugin('gpkg', connection_name)
        connection.connect()

        db = connection.database()
        self.assertIsNotNone(db)

        # tables = db.tables()
        # for i in range(len(tables)):
        #     table = tables[i]
        #     info = table.info()

        connection.remove()

    def testAmphibiousMode(self,):
        connectionName = 'geopack1'
        plugin = createDbPlugin('gpkg')
        uri = QgsDataSourceUri()
        test_gpkg = os.path.join(os.path.join(unitTestDataPath(), 'provider'), 'test_json.gpkg')

        uri.setDatabase(test_gpkg)
        plugin.addConnection(connectionName, uri)
        connection = createDbPlugin('gpkg', connectionName)
        connection.connect()
        db = connection.database()
        res = db.connector._execute(None, "SELECT St_area({}) from foo".format(db.tables()[0].fields()[1].name))
        results = [row for row in res]
        self.assertEqual(results, [(215229.265625,), (247328.171875,), (261752.78125,), (547597.2109375,), (15775.7578125,), (101429.9765625,), (268597.625,), (1634833.390625,), (596610.3359375,), (5268.8125,)])
        connection.remove()


if __name__ == '__main__':
    unittest.main()
