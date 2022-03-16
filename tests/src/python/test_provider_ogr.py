# -*- coding: utf-8 -*-
"""QGIS Unit tests for the non-shapefile, non-tabfile datasources handled by OGR provider.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Even Rouault'
__date__ = '2016-04-11'
__copyright__ = 'Copyright 2016, Even Rouault'

import hashlib
import os
import shutil
import sys
import tempfile
from datetime import datetime

from osgeo import gdal, ogr  # NOQA
from qgis.PyQt.QtCore import QVariant, QByteArray, QTemporaryDir
from qgis.PyQt.QtXml import QDomDocument

from qgis.core import (
    NULL,
    QgsAuthMethodConfig,
    QgsApplication,
    QgsCoordinateTransformContext,
    QgsEditorWidgetSetup,
    QgsProject,
    QgsField,
    QgsFields,
    QgsGeometry,
    QgsRectangle,
    QgsProviderRegistry,
    QgsFeature,
    QgsFeatureRequest,
    QgsSettings,
    QgsDataProvider,
    QgsVectorDataProvider,
    QgsVectorLayer,
    QgsVectorFileWriter,
    QgsVectorLayerExporter,
    QgsWkbTypes,
    QgsNetworkAccessManager,
    QgsLayerMetadata,
    QgsNotSupportedException,
    QgsMapLayerType,
    QgsProviderSublayerDetails,
    Qgis,
    QgsDirectoryItem
)

from qgis.gui import (
    QgsGui
)
from qgis.testing import start_app, unittest
from qgis.utils import spatialite_connect

import mockedwebserver
from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


def GDAL_COMPUTE_VERSION(maj, min, rev):
    return ((maj) * 1000000 + (min) * 10000 + (rev) * 100)


# Note - doesn't implement ProviderTestCase as most OGR provider is tested by the shapefile provider test


def count_opened_filedescriptors(filename_to_test):
    count = -1
    if sys.platform.startswith('linux'):
        count = 0
        open_files_dirname = '/proc/%d/fd' % os.getpid()
        filenames = os.listdir(open_files_dirname)
        for filename in filenames:
            full_filename = open_files_dirname + '/' + filename
            if os.path.exists(full_filename):
                link = os.readlink(full_filename)
                if os.path.basename(link) == os.path.basename(filename_to_test):
                    count += 1
    return count


class PyQgsOGRProvider(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        # Create test layer
        cls.basetestpath = tempfile.mkdtemp()
        cls.datasource = os.path.join(cls.basetestpath, 'test.csv')
        with open(cls.datasource, 'wt') as f:
            f.write('id,WKT\n')
            f.write('1,POINT(2 49)\n')

        cls.dirs_to_cleanup = [cls.basetestpath]

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        for dirname in cls.dirs_to_cleanup:
            shutil.rmtree(dirname, True)

    def testUpdateMode(self):

        vl = QgsVectorLayer('{}|layerid=0'.format(self.datasource), 'test', 'ogr')
        self.assertTrue(vl.isValid())
        caps = vl.dataProvider().capabilities()
        self.assertTrue(caps & QgsVectorDataProvider.AddFeatures)

        self.assertEqual(vl.dataProvider().property("_debug_open_mode"), "read-write")

        # No-op
        self.assertTrue(vl.dataProvider().enterUpdateMode())
        self.assertEqual(vl.dataProvider().property("_debug_open_mode"), "read-write")

        # No-op
        self.assertTrue(vl.dataProvider().leaveUpdateMode())
        self.assertEqual(vl.dataProvider().property("_debug_open_mode"), "read-write")

    def testGeometryTypeKnownAtSecondFeature(self):

        datasource = os.path.join(self.basetestpath, 'testGeometryTypeKnownAtSecondFeature.csv')
        with open(datasource, 'wt') as f:
            f.write('id,WKT\n')
            f.write('1,\n')
            f.write('2,POINT(2 49)\n')

        vl = QgsVectorLayer('{}|layerid=0'.format(datasource), 'test', 'ogr')
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.Point)

    def testMixOfPolygonCurvePolygon(self):

        datasource = os.path.join(self.basetestpath, 'testMixOfPolygonCurvePolygon.csv')
        with open(datasource, 'wt') as f:
            f.write('id,WKT\n')
            f.write('1,"POLYGON((0 0,0 1,1 1,0 0))"\n')
            f.write('2,"CURVEPOLYGON((0 0,0 1,1 1,0 0))"\n')
            f.write('3,"MULTIPOLYGON(((0 0,0 1,1 1,0 0)))"\n')
            f.write('4,"MULTISURFACE(((0 0,0 1,1 1,0 0)))"\n')

        vl = QgsVectorLayer('{}|layerid=0'.format(datasource), 'test', 'ogr')
        self.assertTrue(vl.isValid())
        self.assertEqual(len(vl.dataProvider().subLayers()), 1)
        self.assertEqual(vl.dataProvider().subLayers()[0], QgsDataProvider.SUBLAYER_SEPARATOR.join(
            ['0', 'testMixOfPolygonCurvePolygon', '4', 'CurvePolygon', '', '']))

    def testMixOfLineStringCompoundCurve(self):

        datasource = os.path.join(self.basetestpath, 'testMixOfLineStringCompoundCurve.csv')
        with open(datasource, 'wt') as f:
            f.write('id,WKT\n')
            f.write('1,"LINESTRING(0 0,0 1)"\n')
            f.write('2,"COMPOUNDCURVE((0 0,0 1))"\n')
            f.write('3,"MULTILINESTRING((0 0,0 1))"\n')
            f.write('4,"MULTICURVE((0 0,0 1))"\n')
            f.write('5,"CIRCULARSTRING(0 0,1 1,2 0)"\n')

        vl = QgsVectorLayer('{}|layerid=0'.format(datasource), 'test', 'ogr')
        self.assertTrue(vl.isValid())
        self.assertEqual(len(vl.dataProvider().subLayers()), 1)
        self.assertEqual(vl.dataProvider().subLayers()[0], QgsDataProvider.SUBLAYER_SEPARATOR.join(
            ['0', 'testMixOfLineStringCompoundCurve', '5', 'CompoundCurve', '', '']))

    def testGpxElevation(self):
        # GPX without elevation data
        datasource = os.path.join(TEST_DATA_DIR, 'noelev.gpx')
        vl = QgsVectorLayer('{}|layername=routes'.format(datasource), 'test', 'ogr')
        self.assertTrue(vl.isValid())
        f = next(vl.getFeatures())
        self.assertEqual(f.geometry().wkbType(), QgsWkbTypes.LineString)

        # GPX with elevation data
        datasource = os.path.join(TEST_DATA_DIR, 'elev.gpx')
        vl = QgsVectorLayer('{}|layername=routes'.format(datasource), 'test', 'ogr')
        self.assertTrue(vl.isValid())
        f = next(vl.getFeatures())
        self.assertEqual(f.geometry().wkbType(), QgsWkbTypes.LineString25D)
        self.assertEqual(f.geometry().constGet().pointN(0).z(), 1)
        self.assertEqual(f.geometry().constGet().pointN(1).z(), 2)
        self.assertEqual(f.geometry().constGet().pointN(2).z(), 3)

    def testNoDanglingFileDescriptorAfterCloseVariant1(self):
        ''' Test that when closing the provider all file handles are released '''

        datasource = os.path.join(self.basetestpath, 'testNoDanglingFileDescriptorAfterCloseVariant1.csv')
        with open(datasource, 'wt') as f:
            f.write('id,WKT\n')
            f.write('1,\n')
            f.write('2,POINT(2 49)\n')

        vl = QgsVectorLayer('{}|layerid=0'.format(datasource), 'test', 'ogr')
        self.assertTrue(vl.isValid())
        # The iterator will take one extra connection
        myiter = vl.getFeatures()
        # Consume one feature but the iterator is still opened
        f = next(myiter)
        self.assertTrue(f.isValid())

        if sys.platform.startswith('linux'):
            self.assertEqual(count_opened_filedescriptors(datasource), 2)

        # Should release one file descriptor
        del vl

        # Non portable, but Windows testing is done with trying to unlink
        if sys.platform.startswith('linux'):
            self.assertEqual(count_opened_filedescriptors(datasource), 1)

        f = next(myiter)
        self.assertTrue(f.isValid())

        # Should release one file descriptor
        del myiter

        # Non portable, but Windows testing is done with trying to unlink
        if sys.platform.startswith('linux'):
            self.assertEqual(count_opened_filedescriptors(datasource), 0)

        # Check that deletion works well (can only fail on Windows)
        os.unlink(datasource)
        self.assertFalse(os.path.exists(datasource))

    def testNoDanglingFileDescriptorAfterCloseVariant2(self):
        ''' Test that when closing the provider all file handles are released '''

        datasource = os.path.join(self.basetestpath, 'testNoDanglingFileDescriptorAfterCloseVariant2.csv')
        with open(datasource, 'wt') as f:
            f.write('id,WKT\n')
            f.write('1,\n')
            f.write('2,POINT(2 49)\n')

        vl = QgsVectorLayer('{}|layerid=0'.format(datasource), 'test', 'ogr')
        self.assertTrue(vl.isValid())
        # Consume all features.
        myiter = vl.getFeatures()
        for feature in myiter:
            pass
        # The iterator is closed, but the corresponding connection still not closed
        if sys.platform.startswith('linux'):
            self.assertEqual(count_opened_filedescriptors(datasource), 2)

        # Should release one file descriptor
        del vl

        # Non portable, but Windows testing is done with trying to unlink
        if sys.platform.startswith('linux'):
            self.assertEqual(count_opened_filedescriptors(datasource), 0)

        # Check that deletion works well (can only fail on Windows)
        os.unlink(datasource)
        self.assertFalse(os.path.exists(datasource))

    def testGeometryCollection(self):
        ''' Test that we can at least retrieves attribute of features with geometry collection '''

        datasource = os.path.join(self.basetestpath, 'testGeometryCollection.csv')
        with open(datasource, 'wt') as f:
            f.write('id,WKT\n')
            f.write('1,POINT Z(2 49 0)\n')
            f.write('2,GEOMETRYCOLLECTION Z (POINT Z (2 49 0))\n')

        vl = QgsVectorLayer('{}|layerid=0|geometrytype=GeometryCollection'.format(datasource), 'test', 'ogr')
        self.assertTrue(vl.isValid())
        self.assertTrue(vl.featureCount(), 1)
        values = [f['id'] for f in vl.getFeatures()]
        self.assertEqual(values, ['2'])
        del vl

        os.unlink(datasource)
        self.assertFalse(os.path.exists(datasource))

    def testGdb(self):
        """ Test opening a GDB database layer"""
        gdb_path = os.path.join(unitTestDataPath(), 'test_gdb.gdb')
        for i in range(3):
            l = QgsVectorLayer(gdb_path + '|layerid=' + str(i), 'test', 'ogr')
            self.assertTrue(l.isValid())

    def testGdbFilter(self):
        """ Test opening a GDB database layer with filter"""
        gdb_path = os.path.join(unitTestDataPath(), 'test_gdb.gdb')
        l = QgsVectorLayer(gdb_path + '|layerid=1|subset="text" = \'shape 2\'', 'test', 'ogr')
        self.assertTrue(l.isValid())
        it = l.getFeatures()
        f = QgsFeature()
        while it.nextFeature(f):
            self.assertTrue(f.attribute("text") == "shape 2")

    def testTriangleTINPolyhedralSurface(self):
        """ Test support for Triangles (mapped to Polygons) """
        testsets = (
            ("Triangle((0 0, 0 1, 1 1, 0 0))", QgsWkbTypes.Triangle, "Triangle ((0 0, 0 1, 1 1, 0 0))"),
            ("Triangle Z((0 0 1, 0 1 2, 1 1 3, 0 0 1))", QgsWkbTypes.TriangleZ,
             "TriangleZ ((0 0 1, 0 1 2, 1 1 3, 0 0 1))"),
            ("Triangle M((0 0 4, 0 1 5, 1 1 6, 0 0 4))", QgsWkbTypes.TriangleM,
             "TriangleM ((0 0 4, 0 1 5, 1 1 6, 0 0 4))"),
            ("Triangle ZM((0 0 0 1, 0 1 2 3, 1 1 4 5, 0 0 0 1))", QgsWkbTypes.TriangleZM,
             "TriangleZM ((0 0 0 1, 0 1 2 3, 1 1 4 5, 0 0 0 1))"),

            ("TIN (((0 0, 0 1, 1 1, 0 0)),((0 0, 1 0, 1 1, 0 0)))", QgsWkbTypes.MultiPolygon,
             "MultiPolygon (((0 0, 0 1, 1 1, 0 0)),((0 0, 1 0, 1 1, 0 0)))"),
            ("TIN Z(((0 0 0, 0 1 1, 1 1 1, 0 0 0)),((0 0 0, 1 0 0, 1 1 1, 0 0 0)))", QgsWkbTypes.MultiPolygonZ,
             "MultiPolygonZ (((0 0 0, 0 1 1, 1 1 1, 0 0 0)),((0 0 0, 1 0 0, 1 1 1, 0 0 0)))"),
            ("TIN M(((0 0 0, 0 1 2, 1 1 3, 0 0 0)),((0 0 0, 1 0 4, 1 1 3, 0 0 0)))", QgsWkbTypes.MultiPolygonM,
             "MultiPolygonM (((0 0 0, 0 1 2, 1 1 3, 0 0 0)),((0 0 0, 1 0 4, 1 1 3, 0 0 0)))"),
            ("TIN ZM(((0 0 0 0, 0 1 1 2, 1 1 1 3, 0 0 0 0)),((0 0 0 0, 1 0 0 4, 1 1 1 3, 0 0 0 0)))",
             QgsWkbTypes.MultiPolygonZM,
             "MultiPolygonZM (((0 0 0 0, 0 1 1 2, 1 1 1 3, 0 0 0 0)),((0 0 0 0, 1 0 0 4, 1 1 1 3, 0 0 0 0)))"),

            ("PolyhedralSurface (((0 0, 0 1, 1 1, 0 0)),((0 0, 1 0, 1 1, 0 0)))", QgsWkbTypes.MultiPolygon,
             "MultiPolygon (((0 0, 0 1, 1 1, 0 0)),((0 0, 1 0, 1 1, 0 0)))"),
            ("PolyhedralSurface Z(((0 0 0, 0 1 1, 1 1 1, 0 0 0)),((0 0 0, 1 0 0, 1 1 1, 0 0 0)))",
             QgsWkbTypes.MultiPolygonZ,
             "MultiPolygonZ (((0 0 0, 0 1 1, 1 1 1, 0 0 0)),((0 0 0, 1 0 0, 1 1 1, 0 0 0)))"),
            ("PolyhedralSurface M(((0 0 0, 0 1 2, 1 1 3, 0 0 0)),((0 0 0, 1 0 4, 1 1 3, 0 0 0)))",
             QgsWkbTypes.MultiPolygonM,
             "MultiPolygonM (((0 0 0, 0 1 2, 1 1 3, 0 0 0)),((0 0 0, 1 0 4, 1 1 3, 0 0 0)))"),
            ("PolyhedralSurface ZM(((0 0 0 0, 0 1 1 2, 1 1 1 3, 0 0 0 0)),((0 0 0 0, 1 0 0 4, 1 1 1 3, 0 0 0 0)))",
             QgsWkbTypes.MultiPolygonZM,
             "MultiPolygonZM (((0 0 0 0, 0 1 1 2, 1 1 1 3, 0 0 0 0)),((0 0 0 0, 1 0 0 4, 1 1 1 3, 0 0 0 0)))")
        )
        for row in testsets:
            datasource = os.path.join(self.basetestpath, 'test.csv')
            with open(datasource, 'wt') as f:
                f.write('id,WKT\n')
                f.write('1,"%s"' % row[0])

            vl = QgsVectorLayer(datasource, 'test', 'ogr')
            self.assertTrue(vl.isValid())
            self.assertEqual(vl.wkbType(), row[1])

            f = QgsFeature()
            self.assertTrue(vl.getFeatures(QgsFeatureRequest(1)).nextFeature(f))
            self.assertTrue(f.geometry())
            self.assertEqual(f.geometry().constGet().asWkt(), row[2])

    """PolyhedralSurface, Tin => mapped to MultiPolygon
          Triangle => mapped to Polygon
      """

    def testSetupProxy(self):
        """Test proxy setup"""
        settings = QgsSettings()
        settings.setValue("proxy/proxyEnabled", True)
        settings.setValue("proxy/proxyPort", '1234')
        settings.setValue("proxy/proxyHost", 'myproxyhostname.com')
        settings.setValue("proxy/proxyUser", 'username')
        settings.setValue("proxy/proxyPassword", 'password')
        settings.setValue("proxy/proxyExcludedUrls", "http://www.myhost.com|http://www.myotherhost.com")
        QgsNetworkAccessManager.instance().setupDefaultProxyAndCache()
        vl = QgsVectorLayer(TEST_DATA_DIR + '/' + 'lines.shp', 'proxy_test', 'ogr')
        self.assertTrue(vl.isValid())
        self.assertEqual(gdal.GetConfigOption("GDAL_HTTP_PROXY"), "myproxyhostname.com:1234")
        self.assertEqual(gdal.GetConfigOption("GDAL_HTTP_PROXYUSERPWD"), "username:password")

        settings.setValue("proxy/proxyEnabled", True)
        settings.remove("proxy/proxyPort")
        settings.setValue("proxy/proxyHost", 'myproxyhostname.com')
        settings.setValue("proxy/proxyUser", 'username')
        settings.remove("proxy/proxyPassword")
        settings.setValue("proxy/proxyExcludedUrls", "http://www.myhost.com|http://www.myotherhost.com")
        QgsNetworkAccessManager.instance().setupDefaultProxyAndCache()
        vl = QgsVectorLayer(TEST_DATA_DIR + '/' + 'lines.shp', 'proxy_test', 'ogr')
        self.assertTrue(vl.isValid())
        self.assertEqual(gdal.GetConfigOption("GDAL_HTTP_PROXY"), "myproxyhostname.com")
        self.assertEqual(gdal.GetConfigOption("GDAL_HTTP_PROXYUSERPWD"), "username")

    def testEditGeoJsonRemoveField(self):
        """ Test bugfix of https://github.com/qgis/QGIS/issues/26484 (deleting an existing field)"""

        datasource = os.path.join(self.basetestpath, 'testEditGeoJsonRemoveField.json')
        with open(datasource, 'wt') as f:
            f.write("""{
"type": "FeatureCollection",
"features": [
{ "type": "Feature", "properties": { "x": 1, "y": 2, "z": 3, "w": 4 }, "geometry": { "type": "Point", "coordinates": [ 0, 0 ] } } ] }""")

        vl = QgsVectorLayer(datasource, 'test', 'ogr')
        self.assertTrue(vl.isValid())
        self.assertTrue(vl.startEditing())
        self.assertTrue(vl.deleteAttribute(1))
        self.assertTrue(vl.commitChanges())
        self.assertEqual(len(vl.dataProvider().fields()), 4 - 1)

        f = QgsFeature()
        self.assertTrue(vl.getFeatures(QgsFeatureRequest()).nextFeature(f))
        self.assertEqual(f['x'], 1)
        self.assertEqual(f['z'], 3)
        self.assertEqual(f['w'], 4)

    def testEditGeoJsonAddField(self):
        """ Test bugfix of https://github.com/qgis/QGIS/issues/26484 (adding a new field)"""

        datasource = os.path.join(self.basetestpath, 'testEditGeoJsonAddField.json')
        with open(datasource, 'wt') as f:
            f.write("""{
"type": "FeatureCollection",
"features": [
{ "type": "Feature", "properties": { "x": 1 }, "geometry": { "type": "Point", "coordinates": [ 0, 0 ] } } ] }""")

        vl = QgsVectorLayer(datasource, 'test', 'ogr')
        self.assertTrue(vl.isValid())
        self.assertTrue(vl.startEditing())
        self.assertTrue(vl.addAttribute(QgsField('strfield', QVariant.String)))
        self.assertTrue(vl.commitChanges())
        self.assertEqual(len(vl.dataProvider().fields()), 1 + 1)

        f = QgsFeature()
        self.assertTrue(vl.getFeatures(QgsFeatureRequest()).nextFeature(f))
        self.assertIsNone(f['strfield'])

        # Completely reload file
        vl = QgsVectorLayer(datasource, 'test', 'ogr')
        # As we didn't set any value to the new field, it is not written at
        # all in the GeoJSON file, so it has disappeared
        self.assertEqual(len(vl.fields()), 1)

    def testEditGeoJsonAddFieldAndThenAddFeatures(self):
        """ Test bugfix of https://github.com/qgis/QGIS/issues/26484 (adding a new field)"""

        datasource = os.path.join(self.basetestpath, 'testEditGeoJsonAddField.json')
        with open(datasource, 'wt') as f:
            f.write("""{
"type": "FeatureCollection",
"features": [
{ "type": "Feature", "properties": { "x": 1 }, "geometry": { "type": "Point", "coordinates": [ 0, 0 ] } } ] }""")

        vl = QgsVectorLayer(datasource, 'test', 'ogr')
        self.assertTrue(vl.isValid())
        self.assertTrue(vl.startEditing())
        self.assertTrue(vl.addAttribute(QgsField('strfield', QVariant.String)))
        self.assertTrue(vl.commitChanges())
        self.assertEqual(len(vl.dataProvider().fields()), 1 + 1)
        self.assertEqual([f.name() for f in vl.dataProvider().fields()], ['x', 'strfield'])

        f = QgsFeature()
        self.assertTrue(vl.getFeatures(QgsFeatureRequest()).nextFeature(f))
        self.assertIsNone(f['strfield'])
        self.assertEqual([field.name() for field in f.fields()], ['x', 'strfield'])

        self.assertTrue(vl.startEditing())
        vl.changeAttributeValue(f.id(), 1, 'x')
        self.assertTrue(vl.commitChanges())
        f = QgsFeature()
        self.assertTrue(vl.getFeatures(QgsFeatureRequest()).nextFeature(f))
        self.assertEqual(f['strfield'], 'x')
        self.assertEqual([field.name() for field in f.fields()], ['x', 'strfield'])

        # Completely reload file
        vl = QgsVectorLayer(datasource, 'test', 'ogr')
        self.assertEqual(len(vl.fields()), 2)

    def testDataItems(self):
        dataitem = QgsDirectoryItem(None, 'name', unitTestDataPath())
        children = dataitem.createChildren()

        # Single layer
        item = [i for i in children if i.path().endswith('lines.shp')][0]
        self.assertTrue(item.uri().endswith('lines.shp'))

        # Multiple layer
        item = [i for i in children if i.path().endswith('multilayer.kml')][0]
        children = item.createChildren()
        self.assertEqual(len(children), 2)
        self.assertIn('multilayer.kml|layername=Layer1', children[0].uri())
        self.assertIn('multilayer.kml|layername=Layer2', children[1].uri())

        # Multiple layer (geopackage)
        tmpfile = os.path.join(self.basetestpath, 'testDataItems.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('Layer1', geom_type=ogr.wkbPoint)
        lyr = ds.CreateLayer('Layer2', geom_type=ogr.wkbPoint)
        ds = None

        dataitem = QgsDirectoryItem(None, 'name', self.basetestpath)
        children = dataitem.createChildren()
        item = [i for i in children if i.path().endswith('testDataItems.gpkg')][0]

        children = item.createChildren()
        self.assertEqual(len(children), 2)
        self.assertIn('testDataItems.gpkg|layername=Layer1', children[0].uri())
        self.assertIn('testDataItems.gpkg|layername=Layer2', children[1].uri())

    def testDataItemsRaster(self):
        dataitem = QgsDirectoryItem(None, 'name', unitTestDataPath())
        dir_children = dataitem.createChildren()

        # Multiple layer (geopackage)
        item = [i for i in dir_children if i.path().endswith('two_raster_layers.gpkg')][0]
        children = item.createChildren()

        self.assertEqual(len(children), 2)
        self.assertIn('GPKG:' + unitTestDataPath() + '/two_raster_layers.gpkg:layer01', children[0].uri())
        self.assertIn('GPKG:' + unitTestDataPath() + '/two_raster_layers.gpkg:layer02', children[1].uri())

    def testOSM(self):
        """ Test that opening several layers of the same OSM datasource works properly """

        datasource = os.path.join(TEST_DATA_DIR, 'test.osm')
        vl_points = QgsVectorLayer(datasource + "|layername=points", 'test', 'ogr')
        vl_multipolygons = QgsVectorLayer(datasource + "|layername=multipolygons", 'test', 'ogr')

        f = QgsFeature()

        # When sharing the same dataset handle, the spatial filter of test
        # points layer would apply to the other layers
        iter_points = vl_points.getFeatures(QgsFeatureRequest().setFilterRect(QgsRectangle(-200, -200, -200, -200)))
        self.assertFalse(iter_points.nextFeature(f))

        iter_multipolygons = vl_multipolygons.getFeatures(QgsFeatureRequest())
        self.assertTrue(iter_multipolygons.nextFeature(f))
        self.assertTrue(iter_multipolygons.nextFeature(f))
        self.assertTrue(iter_multipolygons.nextFeature(f))
        self.assertFalse(iter_multipolygons.nextFeature(f))

        # Re-start an iterator (tests #20098)
        iter_multipolygons = vl_multipolygons.getFeatures(QgsFeatureRequest())
        self.assertTrue(iter_multipolygons.nextFeature(f))

        # Test filter by id (#20308)
        f = next(vl_multipolygons.getFeatures(QgsFeatureRequest().setFilterFid(8)))
        self.assertTrue(f.isValid())
        self.assertEqual(f.id(), 8)

        f = next(vl_multipolygons.getFeatures(QgsFeatureRequest().setFilterFid(1)))
        self.assertTrue(f.isValid())
        self.assertEqual(f.id(), 1)

        f = next(vl_multipolygons.getFeatures(QgsFeatureRequest().setFilterFid(5)))
        self.assertTrue(f.isValid())
        self.assertEqual(f.id(), 5)

        # 6 doesn't exist
        it = vl_multipolygons.getFeatures(QgsFeatureRequest().setFilterFids([1, 5, 6, 8]))
        f = next(it)
        self.assertTrue(f.isValid())
        self.assertEqual(f.id(), 1)
        f = next(it)
        self.assertTrue(f.isValid())
        self.assertEqual(f.id(), 5)
        f = next(it)
        self.assertTrue(f.isValid())
        self.assertEqual(f.id(), 8)
        del it

    def testBinaryField(self):
        source = os.path.join(TEST_DATA_DIR, 'attachments.gdb')
        vl = QgsVectorLayer(source + "|layername=points__ATTACH")
        self.assertTrue(vl.isValid())

        fields = vl.fields()
        data_field = fields[fields.lookupField('DATA')]
        self.assertEqual(data_field.type(), QVariant.ByteArray)
        self.assertEqual(data_field.typeName(), 'Binary')

        features = {f['ATTACHMENTID']: f for f in vl.getFeatures()}
        self.assertEqual(len(features), 2)
        self.assertIsInstance(features[1]['DATA'], QByteArray)
        self.assertEqual(hashlib.md5(features[1]['DATA'].data()).hexdigest(), 'ef3dbc530cc39a545832a6c82aac57b6')
        self.assertIsInstance(features[2]['DATA'], QByteArray)
        self.assertEqual(hashlib.md5(features[2]['DATA'].data()).hexdigest(), '4b952b80e4288ca5111be2f6dd5d6809')

    def testGmlStringListField(self):
        source = os.path.join(TEST_DATA_DIR, 'stringlist.gml')
        vl = QgsVectorLayer(source)
        self.assertTrue(vl.isValid())

        fields = vl.fields()
        descriptive_group_field = fields[fields.lookupField('descriptiveGroup')]
        self.assertEqual(descriptive_group_field.type(), QVariant.StringList)
        self.assertEqual(descriptive_group_field.typeName(), 'StringList')
        self.assertEqual(descriptive_group_field.subType(), QVariant.String)

        feature = vl.getFeature(1000002717654)
        self.assertEqual(feature['descriptiveGroup'], ['Building'])
        self.assertEqual(feature['reasonForChange'], ['Reclassified', 'Attributes'])

        tmpfile = os.path.join(self.basetestpath, 'newstringlistfield.gml')
        ds = ogr.GetDriverByName('GML').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbPoint)
        lyr.CreateField(ogr.FieldDefn('strfield', ogr.OFTString))
        lyr.CreateField(ogr.FieldDefn('intfield', ogr.OFTInteger))
        lyr.CreateField(ogr.FieldDefn('strlistfield', ogr.OFTStringList))
        ds = None

        vl = QgsVectorLayer(tmpfile)
        self.assertTrue(vl.isValid())

        dp = vl.dataProvider()
        fields = dp.fields()
        list_field = fields[fields.lookupField('strlistfield')]
        self.assertEqual(list_field.type(), QVariant.StringList)
        self.assertEqual(list_field.typeName(), 'StringList')
        self.assertEqual(list_field.subType(), QVariant.String)

    def testStringListField(self):
        tmpfile = os.path.join(self.basetestpath, 'newstringlistfield.geojson')
        ds = ogr.GetDriverByName('GeoJSON').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbPoint)
        lyr.CreateField(ogr.FieldDefn('strfield', ogr.OFTString))
        lyr.CreateField(ogr.FieldDefn('intfield', ogr.OFTInteger))
        lyr.CreateField(ogr.FieldDefn('stringlistfield', ogr.OFTStringList))

        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetGeometry(ogr.CreateGeometryFromWkt('POINT (1 1)'))
        f.SetField('strfield', 'one')
        f.SetField('intfield', 1)
        f.SetFieldStringList(2, ['a', 'b', 'c'])
        lyr.CreateFeature(f)

        lyr = None
        ds = None

        vl = QgsVectorLayer(tmpfile)
        self.assertTrue(vl.isValid())

        dp = vl.dataProvider()
        fields = dp.fields()
        list_field = fields[fields.lookupField('stringlistfield')]
        self.assertEqual(list_field.type(), QVariant.StringList)
        self.assertEqual(list_field.typeName(), 'StringList')
        self.assertEqual(list_field.subType(), QVariant.String)

        f = next(vl.getFeatures())
        self.assertEqual(f.attributes(), ['one', 1, ['a', 'b', 'c']])

        # add features
        f = QgsFeature()
        f.setAttributes(['two', 2, ['z', 'y', 'x']])
        self.assertTrue(vl.dataProvider().addFeature(f))
        f.setAttributes(['three', 3, NULL])
        self.assertTrue(vl.dataProvider().addFeature(f))
        f.setAttributes(['four', 4, []])
        self.assertTrue(vl.dataProvider().addFeature(f))

        vl = None

        vl = QgsVectorLayer(tmpfile)
        self.assertTrue(vl.isValid())

        self.assertEqual([f.attributes() for f in vl.getFeatures()],
                         [['one', 1, ['a', 'b', 'c']],
                          ['two', 2, ['z', 'y', 'x']],
                          ['three', 3, NULL],
                          ['four', 4, NULL]])

        # change attribute values
        f1_id = [f.id() for f in vl.getFeatures() if f.attributes()[1] == 1][0]
        f3_id = [f.id() for f in vl.getFeatures() if f.attributes()[1] == 3][0]
        self.assertTrue(vl.dataProvider().changeAttributeValues({f1_id: {2: NULL}, f3_id: {2: ['m', 'n', 'o']}}))

        vl = QgsVectorLayer(tmpfile)
        self.assertTrue(vl.isValid())

        self.assertEqual([f.attributes() for f in vl.getFeatures()],
                         [['one', 1, NULL],
                          ['two', 2, ['z', 'y', 'x']],
                          ['three', 3, ['m', 'n', 'o']],
                          ['four', 4, NULL]])

        # add attribute
        self.assertTrue(
            vl.dataProvider().addAttributes([QgsField('new_list', type=QVariant.StringList, subType=QVariant.String)]))
        f1_id = [f.id() for f in vl.getFeatures() if f.attributes()[1] == 1][0]
        f3_id = [f.id() for f in vl.getFeatures() if f.attributes()[1] == 3][0]
        self.assertTrue(vl.dataProvider().changeAttributeValues({f1_id: {3: ['111', '222']}, f3_id: {3: ['121', '122', '123']}}))

        vl = QgsVectorLayer(tmpfile)
        self.assertTrue(vl.isValid())

        self.assertEqual([f.attributes() for f in vl.getFeatures()],
                         [['one', 1, NULL, ['111', '222']],
                          ['two', 2, ['z', 'y', 'x'], NULL],
                          ['three', 3, ['m', 'n', 'o'], ['121', '122', '123']],
                          ['four', 4, NULL, NULL]])

    def testIntListField(self):
        tmpfile = os.path.join(self.basetestpath, 'newintlistfield.geojson')
        ds = ogr.GetDriverByName('GeoJSON').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbPoint)
        lyr.CreateField(ogr.FieldDefn('strfield', ogr.OFTString))
        lyr.CreateField(ogr.FieldDefn('intfield', ogr.OFTInteger))
        lyr.CreateField(ogr.FieldDefn('intlistfield', ogr.OFTIntegerList))

        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetGeometry(ogr.CreateGeometryFromWkt('POINT (1 1)'))
        f.SetField('strfield', 'one')
        f.SetField('intfield', 1)
        f.SetFieldIntegerList(2, [1, 2, 3, 4])
        lyr.CreateFeature(f)

        lyr = None
        ds = None

        vl = QgsVectorLayer(tmpfile)
        self.assertTrue(vl.isValid())

        dp = vl.dataProvider()
        fields = dp.fields()
        list_field = fields[fields.lookupField('intlistfield')]
        self.assertEqual(list_field.type(), QVariant.List)
        self.assertEqual(list_field.typeName(), 'IntegerList')
        self.assertEqual(list_field.subType(), QVariant.Int)

        f = next(vl.getFeatures())
        self.assertEqual(f.attributes(), ['one', 1, [1, 2, 3, 4]])

        # add features
        f = QgsFeature()
        f.setAttributes(['two', 2, [11, 12, 13, 14]])
        self.assertTrue(vl.dataProvider().addFeature(f))
        f.setAttributes(['three', 3, NULL])
        self.assertTrue(vl.dataProvider().addFeature(f))
        f.setAttributes(['four', 4, []])
        self.assertTrue(vl.dataProvider().addFeature(f))

        vl = None

        vl = QgsVectorLayer(tmpfile)
        self.assertTrue(vl.isValid())

        self.assertEqual([f.attributes() for f in vl.getFeatures()],
                         [['one', 1, [1, 2, 3, 4]],
                          ['two', 2, [11, 12, 13, 14]],
                          ['three', 3, NULL],
                          ['four', 4, NULL]])

        # change attribute values
        f1_id = [f.id() for f in vl.getFeatures() if f.attributes()[1] == 1][0]
        f3_id = [f.id() for f in vl.getFeatures() if f.attributes()[1] == 3][0]
        self.assertTrue(vl.dataProvider().changeAttributeValues({f1_id: {2: NULL}, f3_id: {2: [21, 22, 23]}}))

        vl = QgsVectorLayer(tmpfile)
        self.assertTrue(vl.isValid())

        self.assertEqual([f.attributes() for f in vl.getFeatures()],
                         [['one', 1, NULL],
                          ['two', 2, [11, 12, 13, 14]],
                          ['three', 3, [21, 22, 23]],
                          ['four', 4, NULL]])

        # add attribute
        self.assertTrue(
            vl.dataProvider().addAttributes([QgsField('new_list', type=QVariant.List, subType=QVariant.Int)]))
        f1_id = [f.id() for f in vl.getFeatures() if f.attributes()[1] == 1][0]
        f3_id = [f.id() for f in vl.getFeatures() if f.attributes()[1] == 3][0]
        self.assertTrue(vl.dataProvider().changeAttributeValues({f1_id: {3: [111, 222]}, f3_id: {3: [121, 122, 123]}}))

        vl = QgsVectorLayer(tmpfile)
        self.assertTrue(vl.isValid())

        self.assertEqual([f.attributes() for f in vl.getFeatures()],
                         [['one', 1, NULL, [111, 222]],
                          ['two', 2, [11, 12, 13, 14], NULL],
                          ['three', 3, [21, 22, 23], [121, 122, 123]],
                          ['four', 4, NULL, NULL]])

    def testDoubleListField(self):
        tmpfile = os.path.join(self.basetestpath, 'newdoublelistfield.geojson')
        ds = ogr.GetDriverByName('GeoJSON').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbPoint)
        lyr.CreateField(ogr.FieldDefn('strfield', ogr.OFTString))
        lyr.CreateField(ogr.FieldDefn('intfield', ogr.OFTInteger))
        lyr.CreateField(ogr.FieldDefn('doublelistfield', ogr.OFTRealList))

        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetGeometry(ogr.CreateGeometryFromWkt('POINT (1 1)'))
        f.SetField('strfield', 'one')
        f.SetField('intfield', 1)
        f.SetFieldDoubleList(2, [1.1, 2.2, 3.3, 4.4])
        lyr.CreateFeature(f)

        lyr = None
        ds = None

        vl = QgsVectorLayer(tmpfile)
        self.assertTrue(vl.isValid())

        dp = vl.dataProvider()
        fields = dp.fields()
        list_field = fields[fields.lookupField('doublelistfield')]
        self.assertEqual(list_field.type(), QVariant.List)
        self.assertEqual(list_field.typeName(), 'RealList')
        self.assertEqual(list_field.subType(), QVariant.Double)

        f = next(vl.getFeatures())
        self.assertEqual(f.attributes(), ['one', 1, [1.1, 2.2, 3.3, 4.4]])

        # add features
        f = QgsFeature()
        f.setAttributes(['two', 2, [11.1, 12.2, 13.3, 14.4]])
        self.assertTrue(vl.dataProvider().addFeature(f))
        f.setAttributes(['three', 3, NULL])
        self.assertTrue(vl.dataProvider().addFeature(f))
        f.setAttributes(['four', 4, []])
        self.assertTrue(vl.dataProvider().addFeature(f))

        vl = None

        vl = QgsVectorLayer(tmpfile)
        self.assertTrue(vl.isValid())

        self.assertEqual([f.attributes() for f in vl.getFeatures()],
                         [['one', 1, [1.1, 2.2, 3.3, 4.4]],
                          ['two', 2, [11.1, 12.2, 13.3, 14.4]],
                          ['three', 3, NULL],
                          ['four', 4, NULL]])

        # change attribute values
        f1_id = [f.id() for f in vl.getFeatures() if f.attributes()[1] == 1][0]
        f3_id = [f.id() for f in vl.getFeatures() if f.attributes()[1] == 3][0]
        self.assertTrue(vl.dataProvider().changeAttributeValues({f1_id: {2: NULL}, f3_id: {2: [21.1, 22.2, 23.3]}}))

        vl = QgsVectorLayer(tmpfile)
        self.assertTrue(vl.isValid())

        self.assertEqual([f.attributes() for f in vl.getFeatures()],
                         [['one', 1, NULL],
                          ['two', 2, [11.1, 12.2, 13.3, 14.4]],
                          ['three', 3, [21.1, 22.2, 23.3]],
                          ['four', 4, NULL]])

        # add attribute
        self.assertTrue(
            vl.dataProvider().addAttributes([QgsField('new_list', type=QVariant.List, subType=QVariant.Double)]))
        f1_id = [f.id() for f in vl.getFeatures() if f.attributes()[1] == 1][0]
        f3_id = [f.id() for f in vl.getFeatures() if f.attributes()[1] == 3][0]
        self.assertTrue(
            vl.dataProvider().changeAttributeValues({f1_id: {3: [111.1, 222.2]}, f3_id: {3: [121.1, 122.2, 123.3]}}))

        vl = QgsVectorLayer(tmpfile)
        self.assertTrue(vl.isValid())

        self.assertEqual([f.attributes() for f in vl.getFeatures()],
                         [['one', 1, NULL, [111.1, 222.2]],
                          ['two', 2, [11.1, 12.2, 13.3, 14.4], NULL],
                          ['three', 3, [21.1, 22.2, 23.3], [121.1, 122.2, 123.3]],
                          ['four', 4, NULL, NULL]])

    def testInteger64ListField(self):
        tmpfile = os.path.join(self.basetestpath, 'newlonglonglistfield.geojson')
        ds = ogr.GetDriverByName('GeoJSON').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbPoint)
        lyr.CreateField(ogr.FieldDefn('strfield', ogr.OFTString))
        lyr.CreateField(ogr.FieldDefn('intfield', ogr.OFTInteger))
        lyr.CreateField(ogr.FieldDefn('longlonglistfield', ogr.OFTInteger64List))

        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetGeometry(ogr.CreateGeometryFromWkt('POINT (1 1)'))
        f.SetField('strfield', 'one')
        f.SetField('intfield', 1)
        f.SetFieldDoubleList(2, [1234567890123, 1234567890124, 1234567890125, 1234567890126])
        lyr.CreateFeature(f)

        lyr = None
        ds = None

        vl = QgsVectorLayer(tmpfile)
        self.assertTrue(vl.isValid())

        dp = vl.dataProvider()
        fields = dp.fields()
        list_field = fields[fields.lookupField('longlonglistfield')]
        self.assertEqual(list_field.type(), QVariant.List)
        self.assertEqual(list_field.typeName(), 'Integer64List')
        self.assertEqual(list_field.subType(), QVariant.LongLong)

        f = next(vl.getFeatures())
        self.assertEqual(f.attributes(), ['one', 1, [1234567890123, 1234567890124, 1234567890125, 1234567890126]])

        # add features
        f = QgsFeature()
        f.setAttributes(['two', 2, [2234567890123, 2234567890124, 2234567890125, 2234567890126]])
        self.assertTrue(vl.dataProvider().addFeature(f))
        f.setAttributes(['three', 3, NULL])
        self.assertTrue(vl.dataProvider().addFeature(f))
        f.setAttributes(['four', 4, []])
        self.assertTrue(vl.dataProvider().addFeature(f))

        vl = None

        vl = QgsVectorLayer(tmpfile)
        self.assertTrue(vl.isValid())

        self.assertEqual([f.attributes() for f in vl.getFeatures()],
                         [['one', 1, [1234567890123, 1234567890124, 1234567890125, 1234567890126]],
                          ['two', 2, [2234567890123, 2234567890124, 2234567890125, 2234567890126]],
                          ['three', 3, NULL],
                          ['four', 4, NULL]])

        # change attribute values
        f1_id = [f.id() for f in vl.getFeatures() if f.attributes()[1] == 1][0]
        f3_id = [f.id() for f in vl.getFeatures() if f.attributes()[1] == 3][0]
        self.assertTrue(vl.dataProvider().changeAttributeValues(
            {f1_id: {2: NULL}, f3_id: {2: [3234567890123, 3234567890124, 3234567890125, 3234567890126]}}))

        vl = QgsVectorLayer(tmpfile)
        self.assertTrue(vl.isValid())

        self.assertEqual([f.attributes() for f in vl.getFeatures()],
                         [['one', 1, NULL],
                          ['two', 2, [2234567890123, 2234567890124, 2234567890125, 2234567890126]],
                          ['three', 3, [3234567890123, 3234567890124, 3234567890125, 3234567890126]],
                          ['four', 4, NULL]])

        # add attribute
        self.assertTrue(
            vl.dataProvider().addAttributes([QgsField('new_list', type=QVariant.List, subType=QVariant.LongLong)]))
        f1_id = [f.id() for f in vl.getFeatures() if f.attributes()[1] == 1][0]
        f3_id = [f.id() for f in vl.getFeatures() if f.attributes()[1] == 3][0]
        self.assertTrue(vl.dataProvider().changeAttributeValues(
            {f1_id: {3: [4234567890123, 4234567890124]}, f3_id: {3: [5234567890123, 5234567890124, 5234567890125]}}))

        vl = QgsVectorLayer(tmpfile)
        self.assertTrue(vl.isValid())

        self.assertEqual([f.attributes() for f in vl.getFeatures()],
                         [['one', 1, NULL, [4234567890123, 4234567890124]],
                          ['two', 2, [2234567890123, 2234567890124, 2234567890125, 2234567890126], NULL],
                          ['three', 3, [3234567890123, 3234567890124, 3234567890125, 3234567890126],
                           [5234567890123, 5234567890124, 5234567890125]],
                          ['four', 4, NULL, NULL]])

    def testBlobCreation(self):
        """
        Test creating binary blob field in existing table
        """
        tmpfile = os.path.join(self.basetestpath, 'newbinaryfield.sqlite')
        ds = ogr.GetDriverByName('SQLite').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbPoint, options=['FID=fid'])
        lyr.CreateField(ogr.FieldDefn('strfield', ogr.OFTString))
        lyr.CreateField(ogr.FieldDefn('intfield', ogr.OFTInteger))
        f = None
        ds = None

        vl = QgsVectorLayer(tmpfile)
        self.assertTrue(vl.isValid())

        dp = vl.dataProvider()
        f = QgsFeature(dp.fields())
        f.setAttributes([1, 'str', 100])
        self.assertTrue(dp.addFeature(f))

        # add binary field
        self.assertTrue(dp.addAttributes([QgsField('binfield', QVariant.ByteArray)]))

        fields = dp.fields()
        bin1_field = fields[fields.lookupField('binfield')]
        self.assertEqual(bin1_field.type(), QVariant.ByteArray)
        self.assertEqual(bin1_field.typeName(), 'Binary')

        f = QgsFeature(fields)
        bin_1 = b'xxx'
        bin_val1 = QByteArray(bin_1)
        f.setAttributes([2, 'str2', 200, bin_val1])
        self.assertTrue(dp.addFeature(f))

        f2 = [f for f in dp.getFeatures()][1]
        self.assertEqual(f2.attributes(), [2, 'str2', 200, QByteArray(bin_1)])

    def testBoolFieldEvaluation(self):
        datasource = os.path.join(unitTestDataPath(), 'bool_geojson.json')
        vl = QgsVectorLayer(datasource, 'test', 'ogr')
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.fields().at(0).name(), 'bool')
        self.assertEqual(vl.fields().at(0).type(), QVariant.Bool)
        self.assertEqual([f[0] for f in vl.getFeatures()], [True, False, NULL])
        self.assertEqual([f[0].__class__.__name__ for f in vl.getFeatures()], ['bool', 'bool', 'QVariant'])

    def testReloadDataAndFeatureCount(self):

        filename = '/vsimem/test.json'
        gdal.FileFromMemBuffer(filename, """{
"type": "FeatureCollection",
"features": [
{ "type": "Feature", "properties": null, "geometry": { "type": "Point", "coordinates": [2, 49] } },
{ "type": "Feature", "properties": null, "geometry": { "type": "Point", "coordinates": [3, 50] } }
]
}""")
        vl = QgsVectorLayer(filename, 'test', 'ogr')
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.featureCount(), 2)
        gdal.FileFromMemBuffer(filename, """{
"type": "FeatureCollection",
"features": [
{ "type": "Feature", "properties": null, "geometry": { "type": "Point", "coordinates": [2, 49] } }
]
}""")
        vl.reload()
        self.assertEqual(vl.featureCount(), 1)
        gdal.Unlink(filename)

    def testSpatialiteDefaultValues(self):
        """Test whether in spatialite table with default values like CURRENT_TIMESTAMP or
        (datetime('now','localtime')) they are respected. See GH #33383"""

        # Create the test table

        dbname = os.path.join(tempfile.gettempdir(), "test.sqlite")
        if os.path.exists(dbname):
            os.remove(dbname)
        con = spatialite_connect(dbname, isolation_level=None)
        cur = con.cursor()
        cur.execute("BEGIN")
        sql = "SELECT InitSpatialMetadata()"
        cur.execute(sql)

        # simple table with primary key
        sql = """
        CREATE TABLE test_table_default_values (
            id integer primary key autoincrement,
            comment TEXT,
            created_at_01 text DEFAULT (datetime('now','localtime')),
            created_at_02 text DEFAULT CURRENT_TIMESTAMP,
            anumber INTEGER DEFAULT 123,
            atext TEXT default 'My default'
        )
        """
        cur.execute(sql)
        cur.execute("COMMIT")
        con.close()

        vl = QgsVectorLayer(dbname + '|layername=test_table_default_values', 'test_table_default_values', 'ogr')
        self.assertTrue(vl.isValid())

        # Save it for the test
        now = datetime.now()

        # Test default values
        dp = vl.dataProvider()
        # FIXME: should it be None?
        self.assertTrue(dp.defaultValue(0).isNull())
        self.assertIsNone(dp.defaultValue(1))
        # FIXME: This fails because there is no backend-side evaluation in this provider
        # self.assertTrue(dp.defaultValue(2).startswith(now.strftime('%Y-%m-%d')))
        self.assertTrue(dp.defaultValue(3).startswith(now.strftime('%Y-%m-%d')))
        self.assertEqual(dp.defaultValue(4), 123)
        self.assertEqual(dp.defaultValue(5), 'My default')

        self.assertEqual(dp.defaultValueClause(0), 'Autogenerate')
        self.assertEqual(dp.defaultValueClause(1), '')
        self.assertEqual(dp.defaultValueClause(2), "datetime('now','localtime')")
        self.assertEqual(dp.defaultValueClause(3), "CURRENT_TIMESTAMP")
        # FIXME: ogr provider simply returns values when asked for clauses
        # self.assertEqual(dp.defaultValueClause(4), '')
        # self.assertEqual(dp.defaultValueClause(5), '')

        feature = QgsFeature(vl.fields())
        for idx in range(vl.fields().count()):
            default = vl.dataProvider().defaultValue(idx)
            if not default:
                feature.setAttribute(idx, 'A comment')
            else:
                feature.setAttribute(idx, default)

        self.assertTrue(vl.dataProvider().addFeature(feature))
        del (vl)

        # Verify
        vl2 = QgsVectorLayer(dbname + '|layername=test_table_default_values', 'test_table_default_values', 'ogr')
        self.assertTrue(vl2.isValid())
        feature = next(vl2.getFeatures())
        self.assertEqual(feature.attribute(1), 'A comment')
        self.assertTrue(feature.attribute(2).startswith(now.strftime('%Y-%m-%d')))
        self.assertTrue(feature.attribute(3).startswith(now.strftime('%Y-%m-%d')))
        self.assertEqual(feature.attribute(4), 123)
        self.assertEqual(feature.attribute(5), 'My default')

    def testMixOfFilterExpressionAndSubsetStringWhenFilterExpressionCompilationFails(self):
        datasource = os.path.join(unitTestDataPath(), 'filter_test.shp')
        vl = QgsVectorLayer(datasource, 'test', 'ogr')
        self.assertTrue(vl.isValid())

        self.assertCountEqual([f.attributes() for f in vl.getFeatures()], [['circle', '1'],
                                                                           ['circle', '2'],
                                                                           ['rectangle', '1'],
                                                                           ['rectangle', '2']])

        # note - request uses wrong type for match (string vs int). This is OK for QGIS expressions,
        # but will be rejected after we try to compile the expression for OGR to use.
        request = QgsFeatureRequest().setFilterExpression('"color" = 1')
        self.assertCountEqual([f.attributes() for f in vl.getFeatures(request)], [['circle', '1'],
                                                                                  ['rectangle', '1']])
        request = QgsFeatureRequest().setFilterExpression('"color" = 1')
        self.assertCountEqual([f.attributes() for f in vl.getFeatures(request)], [['circle', '1'],
                                                                                  ['rectangle', '1']])

        vl.setSubsetString("\"shape\" = 'rectangle'")
        self.assertCountEqual([f.attributes() for f in vl.getFeatures()], [['rectangle', '1'],
                                                                           ['rectangle', '2']])

        self.assertCountEqual([f.attributes() for f in vl.getFeatures(request)], [['rectangle', '1']])

    @unittest.skipIf(int(gdal.VersionInfo('VERSION_NUM')) < GDAL_COMPUTE_VERSION(3, 2, 0), "GDAL 3.2 required")
    def testFieldAliases(self):
        """
        Test that field aliases are taken from OGR where available (requires GDAL 3.2 or later)
        """
        datasource = os.path.join(unitTestDataPath(), 'field_alias.gdb')
        vl = QgsVectorLayer(datasource, 'test', 'ogr')
        self.assertTrue(vl.isValid())

        fields = vl.fields()

        # proprietary FileGDB driver doesn't have the raster column
        if 'raster' not in set(f.name() for f in fields):
            expected_fieldnames = ['OBJECTID', 'text', 'short_int', 'long_int', 'float', 'double', 'date', 'blob',
                                   'guid', 'SHAPE_Length', 'SHAPE_Area']
            expected_alias = ['', 'My Text Field', 'My Short Int Field', 'My Long Int Field', 'My Float Field',
                              'My Double Field', 'My Date Field', 'My Blob Field', 'My GUID field', '', '']
            expected_alias_map = {'OBJECTID': '', 'SHAPE_Area': '', 'SHAPE_Length': '', 'blob': 'My Blob Field',
                                  'date': 'My Date Field', 'double': 'My Double Field', 'float': 'My Float Field',
                                  'guid': 'My GUID field', 'long_int': 'My Long Int Field',
                                  'short_int': 'My Short Int Field', 'text': 'My Text Field'}
        else:
            expected_fieldnames = ['OBJECTID', 'text', 'short_int', 'long_int', 'float', 'double', 'date', 'blob',
                                   'guid', 'raster', 'SHAPE_Length', 'SHAPE_Area']
            expected_alias = ['', 'My Text Field', 'My Short Int Field', 'My Long Int Field', 'My Float Field',
                              'My Double Field', 'My Date Field', 'My Blob Field', 'My GUID field', 'My Raster Field',
                              '', '']
            expected_alias_map = {'OBJECTID': '', 'SHAPE_Area': '', 'SHAPE_Length': '', 'blob': 'My Blob Field',
                                  'date': 'My Date Field', 'double': 'My Double Field', 'float': 'My Float Field',
                                  'guid': 'My GUID field', 'long_int': 'My Long Int Field', 'raster': 'My Raster Field',
                                  'short_int': 'My Short Int Field', 'text': 'My Text Field'}

        self.assertEqual([f.name() for f in fields], expected_fieldnames)
        self.assertEqual([f.alias() for f in fields], expected_alias)

        self.assertEqual(vl.attributeAliases(), expected_alias_map)

    @unittest.skipIf(int(gdal.VersionInfo('VERSION_NUM')) < GDAL_COMPUTE_VERSION(3, 3, 0), "GDAL 3.3 required")
    def testFieldDomainNames(self):
        """
        Test that field domain names are taken from OGR where available (requires GDAL 3.3 or later)
        """
        datasource = os.path.join(unitTestDataPath(), 'domains.gpkg')
        vl = QgsVectorLayer(datasource, 'test', 'ogr')
        self.assertTrue(vl.isValid())

        fields = vl.fields()
        self.assertEqual(fields.field('with_range_domain_int').constraints().domainName(), 'range_domain_int')
        self.assertEqual(fields.field('with_glob_domain').constraints().domainName(), 'glob_domain')

        datasource = os.path.join(unitTestDataPath(), 'gps_timestamp.gpkg')
        vl = QgsVectorLayer(datasource, 'test', 'ogr')
        self.assertTrue(vl.isValid())
        fields = vl.fields()
        self.assertFalse(fields.field('stringf').constraints().domainName())

    def testGdbLayerMetadata(self):
        """
        Test that we translate GDB metadata to QGIS layer metadata on loading a GDB source
        """
        datasource = os.path.join(unitTestDataPath(), 'gdb_metadata.gdb')
        vl = QgsVectorLayer(datasource, 'test', 'ogr')
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.metadata().identifier(), 'Test')
        self.assertEqual(vl.metadata().title(), 'Title')
        self.assertEqual(vl.metadata().type(), 'dataset')
        self.assertEqual(vl.metadata().language(), 'ENG')
        self.assertIn('This is the abstract', vl.metadata().abstract())
        self.assertEqual(vl.metadata().keywords(), {'Search keys': ['Tags']})
        self.assertEqual(vl.metadata().rights(), ['This is the credits'])
        self.assertEqual(vl.metadata().constraints()[0].type, 'Limitations of use')
        self.assertEqual(vl.metadata().constraints()[0].constraint, 'This is the use limitation')
        self.assertEqual(vl.metadata().extent().spatialExtents()[0].bounds.xMinimum(), 1)
        self.assertEqual(vl.metadata().extent().spatialExtents()[0].bounds.xMaximum(), 2)
        self.assertEqual(vl.metadata().extent().spatialExtents()[0].bounds.yMinimum(), 3)
        self.assertEqual(vl.metadata().extent().spatialExtents()[0].bounds.yMaximum(), 4)

    def testShpLayerMetadata(self):
        """
        Test that we translate .shp.xml metadata to QGIS layer metadata on loading a shp file (if present)
        """
        datasource = os.path.join(unitTestDataPath(), 'france_parts.shp')
        vl = QgsVectorLayer(datasource, 'test', 'ogr')
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.metadata().identifier(), 'QLD_STRUCTURAL_FRAMEWORK_OUTLINE')
        self.assertEqual(vl.metadata().title(), 'QLD_STRUCTURAL_FRAMEWORK_OUTLINE')
        self.assertEqual(vl.metadata().type(), 'dataset')
        self.assertEqual(vl.metadata().language(), 'EN')

    def testOpenOptions(self):

        filename = os.path.join(tempfile.gettempdir(), "testOpenOptions.gpkg")
        if os.path.exists(filename):
            os.remove(filename)
        ds = ogr.GetDriverByName("GPKG").CreateDataSource(filename)
        lyr = ds.CreateLayer("points", geom_type=ogr.wkbPoint)
        ds.ExecuteSQL("CREATE TABLE foo(id INTEGER PRIMARY KEY, name TEXT)")
        ds.ExecuteSQL("INSERT INTO foo VALUES(1, 'bar');")
        ds = None

        vl = QgsVectorLayer(filename + "|option:LIST_ALL_TABLES=NO", 'test', 'ogr')
        self.assertTrue(vl.isValid())
        self.assertEqual(len(vl.dataProvider().subLayers()), 1)
        del vl

        vl = QgsVectorLayer(filename + "|option:LIST_ALL_TABLES=YES", 'test', 'ogr')
        self.assertTrue(vl.isValid())
        self.assertEqual(len(vl.dataProvider().subLayers()), 2)
        del vl

        vl = QgsVectorLayer(filename + "|layername=foo|option:LIST_ALL_TABLES=YES", 'test', 'ogr')
        self.assertTrue(vl.isValid())
        self.assertEqual(len([f for f in vl.getFeatures()]), 1)
        del vl

        os.remove(filename)

    def testTransactionGroupExpressionFields(self):
        """Test issue GH #39230, this is not really specific to GPKG"""

        project = QgsProject()
        project.setTransactionMode(Qgis.TransactionMode.AutomaticGroups)
        tmpfile = os.path.join(
            self.basetestpath, 'tempGeoPackageTransactionExpressionFields.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbPoint)
        lyr.CreateField(ogr.FieldDefn('str_field', ogr.OFTString))

        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetGeometry(ogr.CreateGeometryFromWkt('POINT (1 1)'))
        f.SetField('str_field', 'one')
        lyr.CreateFeature(f)

        del lyr
        del ds

        vl = QgsVectorLayer(tmpfile + '|layername=test', 'test', 'ogr')
        f = QgsField('expression_field', QVariant.Int)
        idx = vl.addExpressionField('123', f)
        self.assertEqual(vl.fields().fieldOrigin(idx), QgsFields.OriginExpression)

        project.addMapLayers([vl])

        feature = next(vl.getFeatures())
        feature.setAttributes([None, 'two', 123])

        self.assertTrue(vl.startEditing())
        self.assertTrue(vl.addFeature(feature))
        self.assertFalse(vl.dataProvider().hasErrors())

    @unittest.skipIf(int(gdal.VersionInfo('VERSION_NUM')) < GDAL_COMPUTE_VERSION(3, 2, 0), "GDAL 3.2 required")
    @unittest.skipIf(ogr.GetDriverByName('OAPIF') is None, "OAPIF driver not available")
    def testHTTPRequestsOverrider(self):
        """
        Test that GDAL curl network requests are redirected through QGIS networking
        """
        with mockedwebserver.install_http_server() as port:
            handler = mockedwebserver.SequentialHandler()

            # Check failed network requests
            # Check that the driver requested Accept header is well propagated
            handler.add('GET', '/collections/foo', 404, expected_headers={'Accept': 'application/json'})
            with mockedwebserver.install_http_handler(handler):
                QgsVectorLayer("OAPIF:http://127.0.0.1:%d/collections/foo" % port, 'test', 'ogr')
                # Error coming from Qt network stack, not GDAL/CURL one
                assert 'server replied: Not Found' in gdal.GetLastErrorMsg()

            # Test a nominal case
            handler = mockedwebserver.SequentialHandler()
            handler.add('GET', '/collections/foo', 200, {'Content-Type': 'application/json'}, '{ "id": "foo" }')
            handler.add('GET', '/collections/foo/items?limit=10', 200, {'Content-Type': 'application/geo+json'},
                        '{ "type": "FeatureCollection", "features": [] }')
            handler.add('GET', '/collections/foo/items?limit=10', 200, {'Content-Type': 'application/geo+json'},
                        '{ "type": "FeatureCollection", "features": [] }')
            if int(gdal.VersionInfo('VERSION_NUM')) < GDAL_COMPUTE_VERSION(3, 3, 0):
                handler.add('GET', '/collections/foo/items?limit=10', 200, {'Content-Type': 'application/geo+json'},
                            '{ "type": "FeatureCollection", "features": [] }')
            with mockedwebserver.install_http_handler(handler):
                vl = QgsVectorLayer("OAPIF:http://127.0.0.1:%d/collections/foo" % port, 'test', 'ogr')
                assert vl.isValid()

            # More complicated test using an anthentication configuration
            authm = QgsApplication.authManager()
            self.assertTrue(authm.setMasterPassword('masterpassword', True))
            config = QgsAuthMethodConfig()
            config.setName('Basic')
            config.setMethod('Basic')
            config.setConfig('username', 'username')
            config.setConfig('password', 'password')
            self.assertTrue(authm.storeAuthenticationConfig(config, True))

            handler = mockedwebserver.SequentialHandler()
            # Check that the authcfg gets expanded during the network request !
            handler.add('GET', '/collections/foo', 404, expected_headers={
                'Authorization': 'Basic dXNlcm5hbWU6cGFzc3dvcmQ='})
            with mockedwebserver.install_http_handler(handler):
                QgsVectorLayer("OAPIF:http://127.0.0.1:%d/collections/foo authcfg='%s'" % (port, config.id()), 'test',
                               'ogr')

    def testShapefilesWithNoAttributes(self):
        """Test issue GH #38834"""

        ml = QgsVectorLayer('Point?crs=epsg:4326', 'test', 'memory')
        self.assertTrue(ml.isValid())

        d = QTemporaryDir()
        options = QgsVectorFileWriter.SaveVectorOptions()
        options.driverName = 'ESRI Shapefile'
        options.layerName = 'writetest'
        err, _ = QgsVectorFileWriter.writeAsVectorFormatV2(ml, os.path.join(d.path(), 'writetest.shp'),
                                                           QgsCoordinateTransformContext(), options)
        self.assertEqual(err, QgsVectorFileWriter.NoError)
        self.assertTrue(os.path.isfile(os.path.join(d.path(), 'writetest.shp')))

        vl = QgsVectorLayer(os.path.join(d.path(), 'writetest.shp'))
        self.assertTrue(bool(vl.dataProvider().capabilities() & QgsVectorDataProvider.AddFeatures))

        # Let's try if we can really add features
        feature = QgsFeature(vl.fields())
        geom = QgsGeometry.fromWkt('POINT(9 45)')
        feature.setGeometry(geom)
        self.assertTrue(vl.startEditing())
        self.assertTrue(vl.addFeatures([feature]))
        self.assertTrue(vl.commitChanges())
        del (vl)

        vl = QgsVectorLayer(os.path.join(d.path(), 'writetest.shp'))
        self.assertEqual(vl.featureCount(), 1)

    def testFidDoubleSaveAsGeopackage(self):
        """Test issue GH #25795"""

        ml = QgsVectorLayer('Point?crs=epsg:4326&field=fid:double(20,0)', 'test', 'memory')
        self.assertTrue(ml.isValid())
        self.assertEqual(ml.fields()[0].type(), QVariant.Double)

        d = QTemporaryDir()
        options = QgsVectorFileWriter.SaveVectorOptions()
        options.driverName = 'GPKG'
        options.layerName = 'fid_double_test'
        err, _ = QgsVectorFileWriter.writeAsVectorFormatV2(ml, os.path.join(d.path(), 'fid_double_test.gpkg'),
                                                           QgsCoordinateTransformContext(), options)
        self.assertEqual(err, QgsVectorFileWriter.NoError)
        self.assertTrue(os.path.isfile(os.path.join(d.path(), 'fid_double_test.gpkg')))

        vl = QgsVectorLayer(os.path.join(d.path(), 'fid_double_test.gpkg'))
        self.assertEqual(vl.fields()[0].type(), QVariant.LongLong)

    def testNonGeopackageSaveMetadata(self):
        """
        Save layer metadata for a file-based format which doesn't have native metadata support.
        In this case we should resort to a sidecar file instead.
        """
        ml = QgsVectorLayer('Point?crs=epsg:4326&field=pk:integer&field=cnt:int8', 'test', 'memory')
        self.assertTrue(ml.isValid())

        d = QTemporaryDir()
        options = QgsVectorFileWriter.SaveVectorOptions()
        options.driverName = 'ESRI Shapefile'
        options.layerName = 'metadatatest'
        err, _ = QgsVectorFileWriter.writeAsVectorFormatV2(ml, os.path.join(d.path(), 'metadatatest.shp'),
                                                           QgsCoordinateTransformContext(), options)
        self.assertEqual(err, QgsVectorFileWriter.NoError)
        self.assertTrue(os.path.isfile(os.path.join(d.path(), 'metadatatest.shp')))

        uri = d.path() + '/metadatatest.shp'

        # now save some metadata
        metadata = QgsLayerMetadata()
        metadata.setAbstract('my abstract')
        metadata.setIdentifier('my identifier')
        metadata.setLicenses(['l1', 'l2'])
        ok, err = QgsProviderRegistry.instance().saveLayerMetadata('ogr', uri, metadata)
        self.assertTrue(ok)

        self.assertTrue(os.path.exists(os.path.join(d.path(), 'metadatatest.qmd')))
        with open(os.path.join(d.path(), 'metadatatest.qmd'), 'rt') as f:
            metadata_xml = ''.join(f.readlines())

        metadata2 = QgsLayerMetadata()
        doc = QDomDocument()
        doc.setContent(metadata_xml)
        self.assertTrue(metadata2.readMetadataXml(doc.documentElement()))
        self.assertEqual(metadata2.abstract(), 'my abstract')
        self.assertEqual(metadata2.identifier(), 'my identifier')
        self.assertEqual(metadata2.licenses(), ['l1', 'l2'])

        # try updating existing metadata -- file should be overwritten
        metadata2.setAbstract('my abstract 2')
        metadata2.setIdentifier('my identifier 2')
        metadata2.setHistory(['h1', 'h2'])
        ok, err = QgsProviderRegistry.instance().saveLayerMetadata('ogr', uri, metadata2)
        self.assertTrue(ok)

        with open(os.path.join(d.path(), 'metadatatest.qmd'), 'rt') as f:
            metadata_xml = ''.join(f.readlines())

        metadata3 = QgsLayerMetadata()
        doc = QDomDocument()
        doc.setContent(metadata_xml)
        self.assertTrue(metadata3.readMetadataXml(doc.documentElement()))
        self.assertEqual(metadata3.abstract(), 'my abstract 2')
        self.assertEqual(metadata3.identifier(), 'my identifier 2')
        self.assertEqual(metadata3.licenses(), ['l1', 'l2'])
        self.assertEqual(metadata3.history(), ['h1', 'h2'])

    def testSaveMetadataUnsupported(self):
        """
        Test saving metadata to an unsupported URI
        """
        metadata = QgsLayerMetadata()
        # this should raise a QgsNotSupportedException, as we don't support writing metadata to a WFS uri
        with self.assertRaises(QgsNotSupportedException):
            QgsProviderRegistry.instance().saveLayerMetadata('ogr', 'WFS:http://www2.dmsolutions.ca/cgi-bin/mswfs_gmap', metadata)

    def testSaveDefaultMetadataUnsupported(self):
        """
        Test saving default metadata to an unsupported layer
        """
        layer = QgsVectorLayer('WFS:http://www2.dmsolutions.ca/cgi-bin/mswfs_gmap', 'test')
        # now save some metadata
        metadata = QgsLayerMetadata()
        metadata.setAbstract('my abstract')
        metadata.setIdentifier('my identifier')
        metadata.setLicenses(['l1', 'l2'])
        layer.setMetadata(metadata)
        # save as default
        msg, res = layer.saveDefaultMetadata()
        self.assertFalse(res)
        self.assertEqual(msg, 'Storing metadata for the specified uri is not supported')

    def testEmbeddedSymbolsKml(self):
        """
        Test retrieving embedded symbols from a KML file
        """
        layer = QgsVectorLayer(os.path.join(TEST_DATA_DIR, 'embedded_symbols', 'samples.kml') + '|layername=Paths',
                               'Lines')
        self.assertTrue(layer.isValid())

        # symbols should not be fetched by default
        self.assertFalse(any(f.embeddedSymbol() for f in layer.getFeatures()))

        symbols = [f.embeddedSymbol().clone() if f.embeddedSymbol() else None for f in
                   layer.getFeatures(QgsFeatureRequest().setFlags(QgsFeatureRequest.EmbeddedSymbols))]
        self.assertCountEqual([s.color().name() for s in symbols if s is not None],
                              ['#ff00ff', '#ffff00', '#000000', '#ff0000'])
        self.assertCountEqual([s.color().alpha() for s in symbols if s is not None], [127, 135, 255, 127])
        self.assertEqual(len([s for s in symbols if s is None]), 2)

    def testDecodeEncodeUriVsizip(self):
        """Test decodeUri/encodeUri for /vsizip/ prefixed URIs"""

        uri = '/vsizip//my/file.zip/shapefile.shp'
        parts = QgsProviderRegistry.instance().decodeUri('ogr', uri)
        self.assertEqual(parts, {'path': '/my/file.zip', 'layerName': None, 'layerId': None, 'vsiPrefix': '/vsizip/',
                                 'vsiSuffix': '/shapefile.shp'})
        encodedUri = QgsProviderRegistry.instance().encodeUri('ogr', parts)
        self.assertEqual(encodedUri, uri)

        uri = '/my/file.zip'
        parts = QgsProviderRegistry.instance().decodeUri('ogr', uri)
        self.assertEqual(parts, {'path': '/my/file.zip', 'layerName': None, 'layerId': None})
        encodedUri = QgsProviderRegistry.instance().encodeUri('ogr', parts)
        self.assertEqual(encodedUri, uri)

        uri = '/vsizip//my/file.zip|layername=shapefile'
        parts = QgsProviderRegistry.instance().decodeUri('ogr', uri)
        self.assertEqual(parts,
                         {'path': '/my/file.zip', 'layerName': 'shapefile', 'layerId': None, 'vsiPrefix': '/vsizip/'})
        encodedUri = QgsProviderRegistry.instance().encodeUri('ogr', parts)
        self.assertEqual(encodedUri, uri)

        uri = '/vsizip//my/file.zip|layername=shapefile|subset="field"=\'value\''
        parts = QgsProviderRegistry.instance().decodeUri('ogr', uri)
        self.assertEqual(parts, {'path': '/my/file.zip', 'layerName': 'shapefile', 'layerId': None,
                                 'subset': '"field"=\'value\'', 'vsiPrefix': '/vsizip/'})
        encodedUri = QgsProviderRegistry.instance().encodeUri('ogr', parts)
        self.assertEqual(encodedUri, uri)

    @unittest.skipIf(int(gdal.VersionInfo('VERSION_NUM')) < GDAL_COMPUTE_VERSION(3, 3, 0), "GDAL 3.3 required")
    def testFieldDomains(self):
        """
        Test that field domains are translated from OGR where available (requires GDAL 3.3 or later)
        """
        datasource = os.path.join(unitTestDataPath(), 'domains.gpkg')
        vl = QgsVectorLayer(datasource, 'test', 'ogr')
        self.assertTrue(vl.isValid())

        fields = vl.fields()

        range_int_field = fields[fields.lookupField('with_range_domain_int')]
        range_int_setup = range_int_field.editorWidgetSetup()
        self.assertEqual(range_int_setup.type(), 'Range')
        self.assertTrue(range_int_setup.config()['AllowNull'])
        self.assertEqual(range_int_setup.config()['Max'], 2)
        self.assertEqual(range_int_setup.config()['Min'], 1)
        self.assertEqual(range_int_setup.config()['Precision'], 0)
        self.assertEqual(range_int_setup.config()['Step'], 1)
        self.assertEqual(range_int_setup.config()['Style'], 'SpinBox')
        # make sure editor widget config from provider has been copied to layer!
        self.assertEqual(vl.editorWidgetSetup(fields.lookupField('with_range_domain_int')).type(), 'Range')

        range_int64_field = fields[fields.lookupField('with_range_domain_int64')]
        range_int64_setup = range_int64_field.editorWidgetSetup()
        self.assertEqual(range_int64_setup.type(), 'Range')
        self.assertTrue(range_int64_setup.config()['AllowNull'])
        self.assertEqual(range_int64_setup.config()['Max'], 1234567890123)
        self.assertEqual(range_int64_setup.config()['Min'], -1234567890123)
        self.assertEqual(range_int64_setup.config()['Precision'], 0)
        self.assertEqual(range_int64_setup.config()['Step'], 1)
        self.assertEqual(range_int64_setup.config()['Style'], 'SpinBox')
        self.assertEqual(vl.editorWidgetSetup(fields.lookupField('with_range_domain_int64')).type(), 'Range')

        range_real_field = fields[fields.lookupField('with_range_domain_real')]
        range_real_setup = range_real_field.editorWidgetSetup()
        self.assertEqual(range_real_setup.type(), 'Range')
        self.assertTrue(range_real_setup.config()['AllowNull'])
        self.assertEqual(range_real_setup.config()['Max'], 2.5)
        self.assertEqual(range_real_setup.config()['Min'], 1.5)
        self.assertEqual(range_real_setup.config()['Precision'], 0)
        self.assertEqual(range_real_setup.config()['Step'], 1)
        self.assertEqual(range_real_setup.config()['Style'], 'SpinBox')
        self.assertEqual(vl.editorWidgetSetup(fields.lookupField('with_range_domain_real')).type(), 'Range')

        enum_field = fields[fields.lookupField('with_enum_domain')]
        enum_setup = enum_field.editorWidgetSetup()
        self.assertEqual(enum_setup.type(), 'ValueMap')
        self.assertTrue(enum_setup.config()['map'], [{'one': '1'}, {'2': '2'}])
        self.assertEqual(vl.editorWidgetSetup(fields.lookupField('with_enum_domain')).type(), 'ValueMap')

    def test_provider_editorWidgets(self):
        if len(QgsGui.editorWidgetRegistry().factories()) == 0:
            QgsGui.editorWidgetRegistry().initEditors()

        editor_widget_type = 'Color'
        factory = QgsGui.instance().editorWidgetRegistry().factory(editor_widget_type)
        assert factory.name() == editor_widget_type

        # 1. create a vector
        uri = "point?crs=epsg:4326&field=id:integer"
        layer = QgsVectorLayer(uri, "Scratch point layer", "memory")

        path = '/vsimem/test.gpkg'
        result, msg = QgsVectorLayerExporter.exportLayer(layer, path, 'ogr', layer.crs())
        self.assertTrue(result == Qgis.VectorExportResult.Success, msg=msg)
        layer = QgsVectorLayer(path)
        self.assertTrue(layer.isValid())
        self.assertTrue(layer.providerType() == 'ogr')

        field1 = QgsField(name='field1', type=QVariant.String)
        field2 = QgsField(name='field2', type=QVariant.String)
        setup1 = QgsEditorWidgetSetup(editor_widget_type, {})
        setup2 = QgsEditorWidgetSetup(editor_widget_type, {})

        # 2. Add field, set editor widget after commitChanges()
        assert layer.startEditing()
        layer.addAttribute(field1)
        assert layer.commitChanges(stopEditing=False)
        i = layer.fields().lookupField(field1.name())
        layer.setEditorWidgetSetup(i, setup1)

        # 3. Add field, set editor widget before commitChanges()
        field2.setEditorWidgetSetup(setup2)
        layer.addAttribute(field2)
        i = layer.fields().lookupField(field2.name())

        # this is a workaround:
        # layer.setEditorWidgetSetup(i, field2.editorWidgetSetup())
        self.assertEqual(layer.editorWidgetSetup(i).type(), editor_widget_type)
        self.assertTrue(layer.commitChanges())

        # editor widget should not change by commitChanges
        self.assertEqual(layer.editorWidgetSetup(i).type(),
                         editor_widget_type,
                         msg='QgsVectorLayer::commitChanged() changed QgsEditorWidgetSetup' +
                             f'\nDriver: {layer.dataProvider().name()}')

    def test_provider_sublayer_details(self):
        """
        Test retrieving sublayer details from data provider metadata
        """
        metadata = QgsProviderRegistry.instance().providerMetadata('ogr')

        # invalid uri
        res = metadata.querySublayers('')
        self.assertFalse(res)

        # not a vector
        res = metadata.querySublayers(os.path.join(TEST_DATA_DIR, 'landsat.tif'))
        self.assertFalse(res)

        # single layer vector
        res = metadata.querySublayers(os.path.join(TEST_DATA_DIR, 'lines.shp'))
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layerNumber(), 0)
        self.assertEqual(res[0].name(), "lines")
        self.assertEqual(res[0].description(), '')
        self.assertEqual(res[0].uri(), TEST_DATA_DIR + "/lines.shp")
        self.assertEqual(res[0].providerKey(), "ogr")
        self.assertEqual(res[0].type(), QgsMapLayerType.VectorLayer)
        self.assertEqual(res[0].wkbType(), QgsWkbTypes.LineString)
        self.assertEqual(res[0].geometryColumnName(), '')
        self.assertEqual(res[0].driverName(), 'ESRI Shapefile')

        # zip file layer vector
        res = metadata.querySublayers(os.path.join(TEST_DATA_DIR, 'zip', 'points2.zip'))
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layerNumber(), 0)
        self.assertEqual(res[0].name(), "points.shp")
        self.assertEqual(res[0].description(), '')
        self.assertEqual(res[0].uri(), '/vsizip/' + TEST_DATA_DIR + "/zip/points2.zip/points.shp|layername=points")
        self.assertEqual(res[0].providerKey(), "ogr")
        self.assertEqual(res[0].type(), QgsMapLayerType.VectorLayer)
        self.assertEqual(res[0].wkbType(), QgsWkbTypes.Point)
        self.assertEqual(res[0].geometryColumnName(), '')
        self.assertEqual(res[0].driverName(), 'ESRI Shapefile')
        options = QgsProviderSublayerDetails.LayerOptions(QgsCoordinateTransformContext())
        vl = res[0].toLayer(options)
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.Point)

        # zip file layer vector, explicit file in zip
        res = metadata.querySublayers('/vsizip/' + TEST_DATA_DIR + '/zip/points2.zip/points.shp')
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layerNumber(), 0)
        self.assertEqual(res[0].name(), "points.shp")
        self.assertEqual(res[0].description(), '')
        self.assertEqual(res[0].uri(), '/vsizip/' + TEST_DATA_DIR + "/zip/points2.zip/points.shp|layername=points")
        self.assertEqual(res[0].providerKey(), "ogr")
        self.assertEqual(res[0].type(), QgsMapLayerType.VectorLayer)
        self.assertEqual(res[0].wkbType(), QgsWkbTypes.Point)
        self.assertEqual(res[0].geometryColumnName(), '')
        self.assertEqual(res[0].driverName(), 'ESRI Shapefile')
        options = QgsProviderSublayerDetails.LayerOptions(QgsCoordinateTransformContext())
        vl = res[0].toLayer(options)
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.Point)

        # zip file layer vector, explicit file in zip which is NOT a OGR supported source
        res = metadata.querySublayers('/vsizip/' + TEST_DATA_DIR + '/zip/points2.zip/points.qml')
        self.assertEqual(len(res), 0)

        # multi-layer archive
        res = metadata.querySublayers(os.path.join(TEST_DATA_DIR, 'zip', 'testtar.tgz'))
        self.assertEqual(len(res), 2)
        self.assertEqual(res[0].layerNumber(), 0)
        self.assertEqual(res[0].name(), "folder/points.geojson")
        self.assertEqual(res[0].description(), '')
        self.assertEqual(res[0].uri(), '/vsitar/' + TEST_DATA_DIR + "/zip/testtar.tgz/folder/points.geojson|layername=points")
        self.assertEqual(res[0].providerKey(), "ogr")
        self.assertEqual(res[0].type(), QgsMapLayerType.VectorLayer)
        self.assertEqual(res[0].wkbType(), QgsWkbTypes.Point)
        self.assertEqual(res[0].geometryColumnName(), '')
        self.assertEqual(res[0].driverName(), 'GeoJSON')
        options = QgsProviderSublayerDetails.LayerOptions(QgsCoordinateTransformContext())
        vl = res[0].toLayer(options)
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.Point)
        self.assertEqual(res[1].layerNumber(), 0)
        self.assertEqual(res[1].name(), "points.shp")
        self.assertEqual(res[1].description(), '')
        self.assertEqual(res[1].uri(), '/vsitar/' + TEST_DATA_DIR + "/zip/testtar.tgz/points.shp|layername=points")
        self.assertEqual(res[1].providerKey(), "ogr")
        self.assertEqual(res[1].type(), QgsMapLayerType.VectorLayer)
        self.assertEqual(res[1].wkbType(), QgsWkbTypes.Point)
        self.assertEqual(res[1].geometryColumnName(), '')
        self.assertEqual(res[1].driverName(), 'ESRI Shapefile')
        options = QgsProviderSublayerDetails.LayerOptions(QgsCoordinateTransformContext())
        vl = res[1].toLayer(options)
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.Point)

        # multi-layer archive, but with specific suffix specified
        res = metadata.querySublayers('/vsitar/' + os.path.join(TEST_DATA_DIR, 'zip', 'testtar.tgz') + '/folder/points.geojson')
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layerNumber(), 0)
        self.assertEqual(res[0].name(), "folder/points.geojson")
        self.assertEqual(res[0].description(), '')
        self.assertEqual(res[0].uri(), '/vsitar/' + TEST_DATA_DIR + "/zip/testtar.tgz/folder/points.geojson|layername=points")
        self.assertEqual(res[0].providerKey(), "ogr")
        self.assertEqual(res[0].type(), QgsMapLayerType.VectorLayer)
        self.assertEqual(res[0].wkbType(), QgsWkbTypes.Point)
        self.assertEqual(res[0].geometryColumnName(), '')
        self.assertEqual(res[0].driverName(), 'GeoJSON')
        options = QgsProviderSublayerDetails.LayerOptions(QgsCoordinateTransformContext())
        vl = res[0].toLayer(options)
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.Point)
        self.assertEqual(vl.dataProvider().storageType(), 'GeoJSON')

        res = metadata.querySublayers('/vsitar/' + os.path.join(TEST_DATA_DIR, 'zip', 'testtar.tgz') + '/points.shp')
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layerNumber(), 0)
        self.assertEqual(res[0].name(), "points.shp")
        self.assertEqual(res[0].description(), '')
        self.assertEqual(res[0].uri(), '/vsitar/' + TEST_DATA_DIR + "/zip/testtar.tgz/points.shp|layername=points")
        self.assertEqual(res[0].providerKey(), "ogr")
        self.assertEqual(res[0].type(), QgsMapLayerType.VectorLayer)
        self.assertEqual(res[0].wkbType(), QgsWkbTypes.Point)
        self.assertEqual(res[0].geometryColumnName(), '')
        self.assertEqual(res[0].driverName(), 'ESRI Shapefile')
        options = QgsProviderSublayerDetails.LayerOptions(QgsCoordinateTransformContext())
        vl = res[0].toLayer(options)
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.Point)
        self.assertEqual(vl.dataProvider().storageType(), 'ESRI Shapefile')

        # archive, with suffix, and layername
        res = metadata.querySublayers('/vsitar/' + os.path.join(TEST_DATA_DIR, 'zip', 'testtar.tgz') + '/points.shp|layername=points')
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layerNumber(), 0)
        self.assertEqual(res[0].name(), "points.shp")
        self.assertEqual(res[0].description(), '')
        self.assertEqual(res[0].uri(), '/vsitar/' + TEST_DATA_DIR + "/zip/testtar.tgz/points.shp|layername=points")
        self.assertEqual(res[0].providerKey(), "ogr")
        self.assertEqual(res[0].type(), QgsMapLayerType.VectorLayer)
        self.assertEqual(res[0].wkbType(), QgsWkbTypes.Point)
        self.assertEqual(res[0].geometryColumnName(), '')
        self.assertEqual(res[0].driverName(), 'ESRI Shapefile')
        options = QgsProviderSublayerDetails.LayerOptions(QgsCoordinateTransformContext())
        vl = res[0].toLayer(options)
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.Point)
        self.assertEqual(vl.dataProvider().storageType(), 'ESRI Shapefile')

        # geometry collection sublayers -- requires a scan to resolve geometry type
        res = metadata.querySublayers(os.path.join(TEST_DATA_DIR, 'multipatch.shp'))
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layerNumber(), 0)
        self.assertEqual(res[0].name(), "multipatch")
        self.assertEqual(res[0].description(), '')
        self.assertEqual(res[0].uri(), TEST_DATA_DIR + "/multipatch.shp")
        self.assertEqual(res[0].providerKey(), "ogr")
        self.assertEqual(res[0].type(), QgsMapLayerType.VectorLayer)
        self.assertEqual(res[0].wkbType(), QgsWkbTypes.Unknown)
        self.assertEqual(res[0].geometryColumnName(), '')
        self.assertEqual(res[0].driverName(), 'ESRI Shapefile')

        # retry with retrieving geometry types
        res = metadata.querySublayers(os.path.join(TEST_DATA_DIR, 'multipatch.shp'), Qgis.SublayerQueryFlag.ResolveGeometryType)
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layerNumber(), 0)
        self.assertEqual(res[0].name(), "multipatch")
        self.assertEqual(res[0].description(), '')
        self.assertEqual(res[0].uri(), TEST_DATA_DIR + "/multipatch.shp|geometrytype=Polygon25D")
        self.assertEqual(res[0].providerKey(), "ogr")
        self.assertEqual(res[0].type(), QgsMapLayerType.VectorLayer)
        self.assertEqual(res[0].wkbType(), QgsWkbTypes.PolygonZ)
        self.assertEqual(res[0].geometryColumnName(), '')
        self.assertEqual(res[0].driverName(), 'ESRI Shapefile')

        # check a feature
        vl = res[0].toLayer(options)
        self.assertTrue(vl.isValid())
        feature = next(vl.getFeatures())
        self.assertEqual(feature.geometry().wkbType(), QgsWkbTypes.MultiPolygonZ)
        self.assertEqual(feature.geometry().asWkt(), 'MultiPolygonZ (((0 0 0, 0 1 0, 1 1 0, 0 0 0)),((0 0 0, 1 1 0, 1 0 0,'
                                                     ' 0 0 0)),((0 0 0, 0 -1 0, 1 -1 0, 0 0 0)),((0 0 0, 1 -1 0, 1 0 0, 0 0 0)))')

        # single layer geopackage -- sublayers MUST have the layerName set on the uri,
        # in case more layers are added in future to the gpkg
        res = metadata.querySublayers(os.path.join(TEST_DATA_DIR, 'curved_polys.gpkg'))
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layerNumber(), 0)
        self.assertEqual(res[0].name(), "polys")
        self.assertEqual(res[0].description(), '')
        self.assertEqual(res[0].uri(), TEST_DATA_DIR + "/curved_polys.gpkg|layername=polys")
        self.assertEqual(res[0].providerKey(), "ogr")
        self.assertEqual(res[0].type(), QgsMapLayerType.VectorLayer)
        self.assertEqual(res[0].wkbType(), QgsWkbTypes.CurvePolygon)
        self.assertEqual(res[0].geometryColumnName(), 'geometry')
        self.assertEqual(res[0].driverName(), 'GPKG')

        # make sure result is valid to load layer from
        vl = res[0].toLayer(options)
        self.assertTrue(vl.isValid())

        # geopackage with two vector layers
        res = metadata.querySublayers(os.path.join(TEST_DATA_DIR, "mixed_layers.gpkg"))
        self.assertEqual(len(res), 2)
        self.assertEqual(res[0].layerNumber(), 0)
        self.assertEqual(res[0].name(), "points")
        self.assertEqual(res[0].description(), "")
        self.assertEqual(res[0].uri(), "{}/mixed_layers.gpkg|layername=points".format(TEST_DATA_DIR))
        self.assertEqual(res[0].providerKey(), "ogr")
        self.assertEqual(res[0].type(), QgsMapLayerType.VectorLayer)
        self.assertEqual(res[0].featureCount(), Qgis.FeatureCountState.Uncounted)
        self.assertEqual(res[0].wkbType(), QgsWkbTypes.Point)
        self.assertEqual(res[0].geometryColumnName(), 'geometry')
        self.assertEqual(res[0].driverName(), 'GPKG')
        vl = res[0].toLayer(options)
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.Point)

        self.assertEqual(res[1].layerNumber(), 1)
        self.assertEqual(res[1].name(), "lines")
        self.assertEqual(res[1].description(), "")
        self.assertEqual(res[1].uri(), "{}/mixed_layers.gpkg|layername=lines".format(TEST_DATA_DIR))
        self.assertEqual(res[1].providerKey(), "ogr")
        self.assertEqual(res[1].type(), QgsMapLayerType.VectorLayer)
        self.assertEqual(res[1].featureCount(), Qgis.FeatureCountState.Uncounted)
        self.assertEqual(res[1].wkbType(), QgsWkbTypes.MultiLineString)
        self.assertEqual(res[1].geometryColumnName(), 'geom')
        self.assertEqual(res[1].driverName(), 'GPKG')
        vl = res[1].toLayer(options)
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.MultiLineString)

        # request feature count
        res = metadata.querySublayers(os.path.join(TEST_DATA_DIR, "mixed_layers.gpkg"), Qgis.SublayerQueryFlag.CountFeatures)
        self.assertEqual(len(res), 2)
        self.assertEqual(res[0].name(), "points")
        self.assertEqual(res[0].featureCount(), 0)
        self.assertEqual(res[0].geometryColumnName(), 'geometry')
        self.assertEqual(res[1].name(), "lines")
        self.assertEqual(res[1].featureCount(), 6)
        self.assertEqual(res[1].geometryColumnName(), 'geom')
        self.assertEqual(res[1].driverName(), 'GPKG')

        # geopackage with two layers, but specific layer is requested in uri
        res = metadata.querySublayers(os.path.join(TEST_DATA_DIR, "mixed_layers.gpkg") + '|layerid=0')
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layerNumber(), 0)
        self.assertEqual(res[0].name(), "points")
        self.assertEqual(res[0].description(), "")
        self.assertEqual(res[0].uri(), "{}/mixed_layers.gpkg|layername=points".format(TEST_DATA_DIR))
        self.assertEqual(res[0].providerKey(), "ogr")
        self.assertEqual(res[0].type(), QgsMapLayerType.VectorLayer)
        self.assertEqual(res[0].featureCount(), Qgis.FeatureCountState.Uncounted)
        self.assertEqual(res[0].wkbType(), QgsWkbTypes.Point)
        self.assertEqual(res[0].geometryColumnName(), 'geometry')
        self.assertEqual(res[0].driverName(), 'GPKG')
        vl = res[0].toLayer(options)
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.Point)

        res = metadata.querySublayers(os.path.join(TEST_DATA_DIR, "mixed_layers.gpkg") + '|layerid=1')
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layerNumber(), 1)
        self.assertEqual(res[0].name(), "lines")
        self.assertEqual(res[0].description(), "")
        self.assertEqual(res[0].uri(), "{}/mixed_layers.gpkg|layername=lines".format(TEST_DATA_DIR))
        self.assertEqual(res[0].providerKey(), "ogr")
        self.assertEqual(res[0].type(), QgsMapLayerType.VectorLayer)
        self.assertEqual(res[0].featureCount(), Qgis.FeatureCountState.Uncounted)
        self.assertEqual(res[0].wkbType(), QgsWkbTypes.MultiLineString)
        self.assertEqual(res[0].geometryColumnName(), 'geom')
        self.assertEqual(res[0].driverName(), 'GPKG')
        vl = res[0].toLayer(options)
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.MultiLineString)

        res = metadata.querySublayers(os.path.join(TEST_DATA_DIR, "mixed_layers.gpkg") + '|layername=points')
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layerNumber(), 0)
        self.assertEqual(res[0].name(), "points")
        self.assertEqual(res[0].description(), "")
        self.assertEqual(res[0].uri(), "{}/mixed_layers.gpkg|layername=points".format(TEST_DATA_DIR))
        self.assertEqual(res[0].providerKey(), "ogr")
        self.assertEqual(res[0].type(), QgsMapLayerType.VectorLayer)
        self.assertEqual(res[0].featureCount(), Qgis.FeatureCountState.Uncounted)
        self.assertEqual(res[0].wkbType(), QgsWkbTypes.Point)
        self.assertEqual(res[0].geometryColumnName(), 'geometry')
        self.assertEqual(res[0].driverName(), 'GPKG')
        vl = res[0].toLayer(options)
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.Point)

        res = metadata.querySublayers(os.path.join(TEST_DATA_DIR, "mixed_layers.gpkg") + '|layername=lines')
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layerNumber(), 1)
        self.assertEqual(res[0].name(), "lines")
        self.assertEqual(res[0].description(), "")
        self.assertEqual(res[0].uri(), "{}/mixed_layers.gpkg|layername=lines".format(TEST_DATA_DIR))
        self.assertEqual(res[0].providerKey(), "ogr")
        self.assertEqual(res[0].type(), QgsMapLayerType.VectorLayer)
        self.assertEqual(res[0].featureCount(), Qgis.FeatureCountState.Uncounted)
        self.assertEqual(res[0].wkbType(), QgsWkbTypes.MultiLineString)
        self.assertEqual(res[0].geometryColumnName(), 'geom')
        self.assertEqual(res[0].driverName(), 'GPKG')
        vl = res[0].toLayer(options)
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.MultiLineString)

        # layer with mixed geometry types - without resolving geometry types
        res = metadata.querySublayers(os.path.join(TEST_DATA_DIR, "mixed_types.TAB"))
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layerNumber(), 0)
        self.assertEqual(res[0].name(), "mixed_types")
        self.assertEqual(res[0].description(), "")
        self.assertEqual(res[0].uri(), "{}/mixed_types.TAB".format(TEST_DATA_DIR))
        self.assertEqual(res[0].providerKey(), "ogr")
        self.assertEqual(res[0].type(), QgsMapLayerType.VectorLayer)
        self.assertEqual(res[0].featureCount(), Qgis.FeatureCountState.Uncounted)
        self.assertEqual(res[0].wkbType(), QgsWkbTypes.Unknown)
        self.assertEqual(res[0].geometryColumnName(), '')
        self.assertEqual(res[0].driverName(), 'MapInfo File')
        vl = res[0].toLayer(options)
        self.assertTrue(vl.isValid())

        # layer with mixed geometry types - without resolving geometry types, but with feature count
        res = metadata.querySublayers(os.path.join(TEST_DATA_DIR, "mixed_types.TAB"), Qgis.SublayerQueryFlag.CountFeatures)
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layerNumber(), 0)
        self.assertEqual(res[0].name(), "mixed_types")
        self.assertEqual(res[0].description(), "")
        self.assertEqual(res[0].uri(), "{}/mixed_types.TAB".format(TEST_DATA_DIR))
        self.assertEqual(res[0].providerKey(), "ogr")
        self.assertEqual(res[0].type(), QgsMapLayerType.VectorLayer)
        self.assertEqual(res[0].featureCount(), 13)
        self.assertEqual(res[0].wkbType(), QgsWkbTypes.Unknown)
        self.assertEqual(res[0].geometryColumnName(), '')
        self.assertEqual(res[0].driverName(), 'MapInfo File')

        # layer with mixed geometry types - resolve geometry type (for OGR provider this implies also that we count features!)
        res = metadata.querySublayers(os.path.join(TEST_DATA_DIR, "mixed_types.TAB"), Qgis.SublayerQueryFlag.ResolveGeometryType)
        self.assertEqual(len(res), 3)
        self.assertEqual(res[0].layerNumber(), 0)
        self.assertEqual(res[0].name(), "mixed_types")
        self.assertEqual(res[0].description(), "")
        self.assertEqual(res[0].uri(), "{}/mixed_types.TAB|geometrytype=Point".format(TEST_DATA_DIR))
        self.assertEqual(res[0].providerKey(), "ogr")
        self.assertEqual(res[0].type(), QgsMapLayerType.VectorLayer)
        self.assertEqual(res[0].featureCount(), 4)
        self.assertEqual(res[0].wkbType(), QgsWkbTypes.Point)
        self.assertEqual(res[0].geometryColumnName(), '')
        self.assertEqual(res[0].driverName(), 'MapInfo File')
        vl = res[0].toLayer(options)
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.Point)

        self.assertEqual(res[1].layerNumber(), 0)
        self.assertEqual(res[1].name(), "mixed_types")
        self.assertEqual(res[1].description(), "")
        self.assertEqual(res[1].uri(), "{}/mixed_types.TAB|geometrytype=LineString".format(TEST_DATA_DIR))
        self.assertEqual(res[1].providerKey(), "ogr")
        self.assertEqual(res[1].type(), QgsMapLayerType.VectorLayer)
        self.assertEqual(res[1].featureCount(), 4)
        self.assertEqual(res[1].wkbType(), QgsWkbTypes.LineString)
        self.assertEqual(res[1].geometryColumnName(), '')
        self.assertEqual(res[1].driverName(), 'MapInfo File')
        vl = res[1].toLayer(options)
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.LineString)

        self.assertEqual(res[2].layerNumber(), 0)
        self.assertEqual(res[2].name(), "mixed_types")
        self.assertEqual(res[2].description(), "")
        self.assertEqual(res[2].uri(), "{}/mixed_types.TAB|geometrytype=Polygon".format(TEST_DATA_DIR))
        self.assertEqual(res[2].providerKey(), "ogr")
        self.assertEqual(res[2].type(), QgsMapLayerType.VectorLayer)
        self.assertEqual(res[2].featureCount(), 3)
        self.assertEqual(res[2].wkbType(), QgsWkbTypes.Polygon)
        self.assertEqual(res[2].geometryColumnName(), '')
        self.assertEqual(res[2].driverName(), 'MapInfo File')
        vl = res[2].toLayer(options)
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.Polygon)

        # a layer which reports unknown geometry type and requires a full table scan to resolve, but which only
        # contains a single type of geometry
        res = metadata.querySublayers(os.path.join(TEST_DATA_DIR, "mapinfo", "fill_styles.TAB"),
                                      Qgis.SublayerQueryFlag.ResolveGeometryType)
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layerNumber(), 0)
        self.assertEqual(res[0].name(), "fill_styles")
        self.assertEqual(res[0].description(), "")
        self.assertEqual(res[0].uri(), "{}/mapinfo/fill_styles.TAB|geometrytype=Polygon".format(TEST_DATA_DIR))
        self.assertEqual(res[0].providerKey(), "ogr")
        self.assertEqual(res[0].type(), QgsMapLayerType.VectorLayer)
        self.assertEqual(res[0].featureCount(), 49)
        self.assertEqual(res[0].wkbType(), QgsWkbTypes.Polygon)
        self.assertEqual(res[0].geometryColumnName(), '')
        self.assertEqual(res[0].driverName(), 'MapInfo File')
        vl = res[0].toLayer(options)
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.Polygon)

        # same, but don't resolve geometry types
        res = metadata.querySublayers(os.path.join(TEST_DATA_DIR, "mapinfo", "fill_styles.TAB"))
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layerNumber(), 0)
        self.assertEqual(res[0].name(), "fill_styles")
        self.assertEqual(res[0].description(), "")
        self.assertEqual(res[0].uri(), "{}/mapinfo/fill_styles.TAB".format(TEST_DATA_DIR))
        self.assertEqual(res[0].providerKey(), "ogr")
        self.assertEqual(res[0].type(), QgsMapLayerType.VectorLayer)
        self.assertEqual(res[0].featureCount(), Qgis.FeatureCountState.Uncounted)
        self.assertEqual(res[0].wkbType(), QgsWkbTypes.Unknown)
        self.assertEqual(res[0].geometryColumnName(), '')
        self.assertEqual(res[0].driverName(), 'MapInfo File')
        vl = res[0].toLayer(options)
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.Polygon)

        # mixed types source, but with a URI which specifies a particular type. Only this type should be returned
        res = metadata.querySublayers(os.path.join(TEST_DATA_DIR, "mixed_types.TAB|geometrytype=Point"),
                                      Qgis.SublayerQueryFlag.ResolveGeometryType)
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layerNumber(), 0)
        self.assertEqual(res[0].name(), "mixed_types")
        self.assertEqual(res[0].description(), "")
        self.assertEqual(res[0].uri(), "{}/mixed_types.TAB|geometrytype=Point".format(TEST_DATA_DIR))
        self.assertEqual(res[0].providerKey(), "ogr")
        self.assertEqual(res[0].type(), QgsMapLayerType.VectorLayer)
        self.assertEqual(res[0].featureCount(), 4)
        self.assertEqual(res[0].wkbType(), QgsWkbTypes.Point)
        self.assertEqual(res[0].geometryColumnName(), '')
        self.assertEqual(res[0].driverName(), 'MapInfo File')
        vl = res[0].toLayer(options)
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.Point)

        res = metadata.querySublayers(os.path.join(TEST_DATA_DIR, "mixed_types.TAB|geometrytype=LineString"),
                                      Qgis.SublayerQueryFlag.ResolveGeometryType)
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layerNumber(), 0)
        self.assertEqual(res[0].name(), "mixed_types")
        self.assertEqual(res[0].description(), "")
        self.assertEqual(res[0].uri(), "{}/mixed_types.TAB|geometrytype=LineString".format(TEST_DATA_DIR))
        self.assertEqual(res[0].providerKey(), "ogr")
        self.assertEqual(res[0].type(), QgsMapLayerType.VectorLayer)
        self.assertEqual(res[0].featureCount(), 4)
        self.assertEqual(res[0].wkbType(), QgsWkbTypes.LineString)
        self.assertEqual(res[0].geometryColumnName(), '')
        self.assertEqual(res[0].driverName(), 'MapInfo File')
        vl = res[0].toLayer(options)
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.LineString)

        res = metadata.querySublayers(os.path.join(TEST_DATA_DIR, "mixed_types.TAB|geometrytype=Polygon"),
                                      Qgis.SublayerQueryFlag.ResolveGeometryType)
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layerNumber(), 0)
        self.assertEqual(res[0].name(), "mixed_types")
        self.assertEqual(res[0].description(), "")
        self.assertEqual(res[0].uri(), "{}/mixed_types.TAB|geometrytype=Polygon".format(TEST_DATA_DIR))
        self.assertEqual(res[0].providerKey(), "ogr")
        self.assertEqual(res[0].type(), QgsMapLayerType.VectorLayer)
        self.assertEqual(res[0].featureCount(), 3)
        self.assertEqual(res[0].wkbType(), QgsWkbTypes.Polygon)
        self.assertEqual(res[0].geometryColumnName(), '')
        self.assertEqual(res[0].driverName(), 'MapInfo File')
        vl = res[0].toLayer(options)
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.Polygon)

        # same as above, but without ResolveGeometryType flag
        res = metadata.querySublayers(os.path.join(TEST_DATA_DIR, "mixed_types.TAB|geometrytype=Point"))
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layerNumber(), 0)
        self.assertEqual(res[0].name(), "mixed_types")
        self.assertEqual(res[0].description(), "")
        self.assertEqual(res[0].uri(), "{}/mixed_types.TAB|geometrytype=Point".format(TEST_DATA_DIR))
        self.assertEqual(res[0].providerKey(), "ogr")
        self.assertEqual(res[0].type(), QgsMapLayerType.VectorLayer)
        self.assertEqual(res[0].featureCount(), Qgis.FeatureCountState.Uncounted)
        self.assertEqual(res[0].wkbType(), QgsWkbTypes.Point)
        self.assertEqual(res[0].geometryColumnName(), '')
        self.assertEqual(res[0].driverName(), 'MapInfo File')
        vl = res[0].toLayer(options)
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.Point)

        res = metadata.querySublayers(os.path.join(TEST_DATA_DIR, "mixed_types.TAB|geometrytype=LineString"))
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layerNumber(), 0)
        self.assertEqual(res[0].name(), "mixed_types")
        self.assertEqual(res[0].description(), "")
        self.assertEqual(res[0].uri(), "{}/mixed_types.TAB|geometrytype=LineString".format(TEST_DATA_DIR))
        self.assertEqual(res[0].providerKey(), "ogr")
        self.assertEqual(res[0].type(), QgsMapLayerType.VectorLayer)
        self.assertEqual(res[0].featureCount(), Qgis.FeatureCountState.Uncounted)
        self.assertEqual(res[0].wkbType(), QgsWkbTypes.LineString)
        self.assertEqual(res[0].geometryColumnName(), '')
        self.assertEqual(res[0].driverName(), 'MapInfo File')
        vl = res[0].toLayer(options)
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.LineString)

        res = metadata.querySublayers(os.path.join(TEST_DATA_DIR, "mixed_types.TAB|geometrytype=Polygon"))
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layerNumber(), 0)
        self.assertEqual(res[0].name(), "mixed_types")
        self.assertEqual(res[0].description(), "")
        self.assertEqual(res[0].uri(), "{}/mixed_types.TAB|geometrytype=Polygon".format(TEST_DATA_DIR))
        self.assertEqual(res[0].providerKey(), "ogr")
        self.assertEqual(res[0].type(), QgsMapLayerType.VectorLayer)
        self.assertEqual(res[0].featureCount(), Qgis.FeatureCountState.Uncounted)
        self.assertEqual(res[0].wkbType(), QgsWkbTypes.Polygon)
        self.assertEqual(res[0].geometryColumnName(), '')
        self.assertEqual(res[0].driverName(), 'MapInfo File')
        vl = res[0].toLayer(options)
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.Polygon)

        # spatialite
        res = metadata.querySublayers(os.path.join(TEST_DATA_DIR, "provider/spatialite.db"))
        self.assertCountEqual([{'name': r.name(),
                                'description': r.description(),
                                'uri': r.uri(),
                                'providerKey': r.providerKey(),
                                'wkbType': r.wkbType(),
                                'driverName': r.driverName(),
                                'geomColName': r.geometryColumnName()} for r in res],
                              [{'name': 'somedata',
                                'description': '',
                                'uri': '{}/provider/spatialite.db|layername=somedata'.format(TEST_DATA_DIR),
                                'providerKey': 'ogr',
                                'wkbType': 1,
                                'driverName': 'SQLite',
                                'geomColName': 'geom'},
                               {'name': 'somepolydata',
                                'description': '',
                                'uri': '{}/provider/spatialite.db|layername=somepolydata'.format(TEST_DATA_DIR),
                                'providerKey': 'ogr',
                                'wkbType': 6,
                                'driverName': 'SQLite',
                                'geomColName': 'geom'},
                               {'name': 'some data',
                                'description': '',
                                'uri': '{}/provider/spatialite.db|layername=some data'.format(TEST_DATA_DIR),
                                'providerKey': 'ogr',
                                'wkbType': 1,
                                'driverName': 'SQLite',
                                'geomColName': 'geom'},
                               {'name': 'validator_project_test',
                                'description': '',
                                'uri': '{}/provider/spatialite.db|layername=validator_project_test'.format(TEST_DATA_DIR),
                                'providerKey': 'ogr',
                                'wkbType': 1,
                                'driverName': 'SQLite',
                                'geomColName': 'geom'},
                               {'name': 'data_licenses',
                                'description': '',
                                'uri': '{}/provider/spatialite.db|layername=data_licenses'.format(TEST_DATA_DIR),
                                'providerKey': 'ogr',
                                'wkbType': 100,
                                'driverName': 'SQLite',
                                'geomColName': ''},
                               {'name': 'some view',
                                'description': '',
                                'uri': '{}/provider/spatialite.db|layername=some view'.format(TEST_DATA_DIR),
                                'providerKey': 'ogr',
                                'wkbType': 100,
                                'driverName': 'SQLite',
                                'geomColName': ''}])

        # sqlite
        res = metadata.querySublayers(
            os.path.join(TEST_DATA_DIR, "valuerelation_widget_wrapper_test.spatialite.sqlite"))
        self.assertCountEqual([{'name': r.name(),
                                'systemTable': bool(r.flags() & Qgis.SublayerFlag.SystemTable)} for r in res],
                              [{'name': 'authors', 'systemTable': False},
                               {'name': 'json', 'systemTable': False}])

        # retrieve system tables
        res = metadata.querySublayers(
            os.path.join(TEST_DATA_DIR, "valuerelation_widget_wrapper_test.spatialite.sqlite"),
            Qgis.SublayerQueryFlag.IncludeSystemTables)
        self.assertCountEqual([{'name': r.name(),
                                'systemTable': bool(r.flags() & Qgis.SublayerFlag.SystemTable)} for r in res],
                              [{'name': 'ElementaryGeometries', 'systemTable': True},
                               {'name': 'SpatialIndex', 'systemTable': True},
                               {'name': 'authors', 'systemTable': False},
                               {'name': 'geom_cols_ref_sys', 'systemTable': True},
                               {'name': 'geometry_columns', 'systemTable': True},
                               {'name': 'geometry_columns_auth', 'systemTable': True},
                               {'name': 'geometry_columns_field_infos', 'systemTable': True},
                               {'name': 'geometry_columns_statistics', 'systemTable': True},
                               {'name': 'geometry_columns_time', 'systemTable': True},
                               {'name': 'json', 'systemTable': False},
                               {'name': 'spatial_ref_sys', 'systemTable': True},
                               {'name': 'spatial_ref_sys_all', 'systemTable': True},
                               {'name': 'spatial_ref_sys_aux', 'systemTable': True},
                               {'name': 'spatialite_history', 'systemTable': True},
                               {'name': 'sql_statements_log', 'systemTable': True},
                               {'name': 'sqlite_sequence', 'systemTable': True},
                               {'name': 'vector_layers', 'systemTable': True},
                               {'name': 'vector_layers_auth', 'systemTable': True},
                               {'name': 'vector_layers_field_infos', 'systemTable': True},
                               {'name': 'vector_layers_statistics', 'systemTable': True},
                               {'name': 'views_geometry_columns', 'systemTable': True},
                               {'name': 'views_geometry_columns_auth', 'systemTable': True},
                               {'name': 'views_geometry_columns_field_infos', 'systemTable': True},
                               {'name': 'views_geometry_columns_statistics', 'systemTable': True},
                               {'name': 'virts_geometry_columns', 'systemTable': True},
                               {'name': 'virts_geometry_columns_auth', 'systemTable': True},
                               {'name': 'virts_geometry_columns_field_infos', 'systemTable': True},
                               {'name': 'virts_geometry_columns_statistics', 'systemTable': True}])

        # metadata.xml file next to tdenv?.adf file -- this is a subcomponent of an ESRI tin layer, should not be exposed
        res = metadata.querySublayers(
            os.path.join(TEST_DATA_DIR, 'esri_tin', 'metadata.xml'))
        self.assertFalse(res)

        # ESRI Arcinfo file
        res = metadata.querySublayers(os.path.join(TEST_DATA_DIR, 'esri_coverage', 'testpolyavc'))
        self.assertEqual(len(res), 4)
        self.assertEqual(res[0].layerNumber(), 0)
        self.assertEqual(res[0].name(), "ARC")
        self.assertEqual(res[0].description(), "")
        self.assertEqual(res[0].uri(), '{}|layername=ARC'.format(os.path.join(TEST_DATA_DIR, 'esri_coverage', 'testpolyavc')))
        self.assertEqual(res[0].providerKey(), "ogr")
        self.assertEqual(res[0].type(), QgsMapLayerType.VectorLayer)
        self.assertFalse(res[0].skippedContainerScan())

    @unittest.skipIf(int(gdal.VersionInfo('VERSION_NUM')) < GDAL_COMPUTE_VERSION(3, 4, 0), "GDAL 3.4 required")
    def test_provider_sublayer_details_hierarchy(self):
        """
        Test retrieving sublayer details from a datasource with a hierarchy of layers
        """
        metadata = QgsProviderRegistry.instance().providerMetadata('ogr')

        res = metadata.querySublayers(os.path.join(TEST_DATA_DIR, 'featuredataset.gdb'))
        self.assertEqual(len(res), 4)
        self.assertEqual(res[0].name(), 'fd1_lyr1')
        self.assertEqual(res[0].path(), ['fd1'])
        self.assertEqual(res[1].name(), 'fd1_lyr2')
        self.assertEqual(res[1].path(), ['fd1'])
        self.assertEqual(res[2].name(), 'standalone')
        self.assertEqual(res[2].path(), [])
        self.assertEqual(res[3].name(), 'fd2_lyr')
        self.assertEqual(res[3].path(), ['fd2'])

    def test_provider_sublayer_details_fast_scan(self):
        """
        Test retrieving sublayer details from data provider metadata, using fast scan
        """
        metadata = QgsProviderRegistry.instance().providerMetadata('ogr')

        # invalid uri
        res = metadata.querySublayers('', Qgis.SublayerQueryFlag.FastScan)
        self.assertFalse(res)

        # not a vector
        res = metadata.querySublayers(os.path.join(TEST_DATA_DIR, 'landsat.tif'), Qgis.SublayerQueryFlag.FastScan)
        self.assertFalse(res)

        # single layer vector
        res = metadata.querySublayers(os.path.join(TEST_DATA_DIR, 'lines.shp'), Qgis.SublayerQueryFlag.FastScan)
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layerNumber(), 0)
        self.assertEqual(res[0].name(), "lines")
        self.assertEqual(res[0].description(), '')
        self.assertEqual(res[0].uri(), TEST_DATA_DIR + "/lines.shp")
        self.assertEqual(res[0].providerKey(), "ogr")
        self.assertEqual(res[0].type(), QgsMapLayerType.VectorLayer)
        self.assertFalse(res[0].skippedContainerScan())

        # geometry collection sublayers -- requires a scan to resolve geometry type
        res = metadata.querySublayers(os.path.join(TEST_DATA_DIR, 'multipatch.shp'), Qgis.SublayerQueryFlag.FastScan)
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layerNumber(), 0)
        self.assertEqual(res[0].name(), "multipatch")
        self.assertEqual(res[0].description(), '')
        self.assertEqual(res[0].uri(), TEST_DATA_DIR + "/multipatch.shp")
        self.assertEqual(res[0].providerKey(), "ogr")
        self.assertEqual(res[0].type(), QgsMapLayerType.VectorLayer)
        self.assertEqual(res[0].wkbType(), QgsWkbTypes.Unknown)
        self.assertEqual(res[0].geometryColumnName(), '')
        self.assertFalse(res[0].skippedContainerScan())

        # single layer geopackage -- sublayers MUST have the layerName set on the uri,
        # in case more layers are added in future to the gpkg
        res = metadata.querySublayers(os.path.join(TEST_DATA_DIR, 'curved_polys.gpkg'), Qgis.SublayerQueryFlag.FastScan)
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layerNumber(), 0)
        self.assertEqual(res[0].name(), "curved_polys")
        self.assertEqual(res[0].description(), '')
        self.assertEqual(res[0].uri(), TEST_DATA_DIR + "/curved_polys.gpkg")
        self.assertEqual(res[0].providerKey(), "ogr")
        self.assertEqual(res[0].type(), QgsMapLayerType.VectorLayer)
        self.assertTrue(res[0].skippedContainerScan())

        # geopackage with two vector layers
        res = metadata.querySublayers(os.path.join(TEST_DATA_DIR, "mixed_layers.gpkg"), Qgis.SublayerQueryFlag.FastScan)
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layerNumber(), 0)
        self.assertEqual(res[0].name(), "mixed_layers")
        self.assertEqual(res[0].description(), "")
        self.assertEqual(res[0].uri(), "{}/mixed_layers.gpkg".format(TEST_DATA_DIR))
        self.assertEqual(res[0].providerKey(), "ogr")
        self.assertEqual(res[0].type(), QgsMapLayerType.VectorLayer)
        self.assertTrue(res[0].skippedContainerScan())

        # layer with mixed geometry types - without resolving geometry types
        res = metadata.querySublayers(os.path.join(TEST_DATA_DIR, "mixed_types.TAB"), Qgis.SublayerQueryFlag.FastScan)
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layerNumber(), 0)
        self.assertEqual(res[0].name(), "mixed_types")
        self.assertEqual(res[0].description(), "")
        self.assertEqual(res[0].uri(), "{}/mixed_types.TAB".format(TEST_DATA_DIR))
        self.assertEqual(res[0].providerKey(), "ogr")
        self.assertEqual(res[0].type(), QgsMapLayerType.VectorLayer)
        self.assertFalse(res[0].skippedContainerScan())

        # spatialite
        res = metadata.querySublayers(os.path.join(TEST_DATA_DIR, "provider/spatialite.db"), Qgis.SublayerQueryFlag.FastScan)
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layerNumber(), 0)
        self.assertEqual(res[0].name(), "spatialite")
        self.assertEqual(res[0].description(), "")
        self.assertEqual(res[0].uri(), "{}/provider/spatialite.db".format(TEST_DATA_DIR))
        self.assertEqual(res[0].providerKey(), "ogr")
        self.assertEqual(res[0].type(), QgsMapLayerType.VectorLayer)
        self.assertTrue(res[0].skippedContainerScan())

        # fast scan, but for trivial type -- fast scan flag will be ignored
        res = metadata.querySublayers(os.path.join(TEST_DATA_DIR, "spreadsheet.ods"), Qgis.SublayerQueryFlag.FastScan)
        self.assertEqual(len(res), 2)
        self.assertEqual(res[0].layerNumber(), 0)
        self.assertEqual(res[0].name(), "Sheet1")
        self.assertEqual(res[0].description(), "")
        self.assertEqual(res[0].uri(), "{}/spreadsheet.ods|layername=Sheet1".format(TEST_DATA_DIR))
        self.assertEqual(res[0].providerKey(), "ogr")
        self.assertEqual(res[0].driverName(), "ODS")
        self.assertEqual(res[0].type(), QgsMapLayerType.VectorLayer)
        self.assertFalse(res[0].skippedContainerScan())
        self.assertEqual(res[1].layerNumber(), 1)
        self.assertEqual(res[1].name(), "Sheet2")
        self.assertEqual(res[1].description(), "")
        self.assertEqual(res[1].uri(), "{}/spreadsheet.ods|layername=Sheet2".format(TEST_DATA_DIR))
        self.assertEqual(res[1].providerKey(), "ogr")
        self.assertEqual(res[1].driverName(), "ODS")
        self.assertEqual(res[1].type(), QgsMapLayerType.VectorLayer)
        self.assertFalse(res[1].skippedContainerScan())

        # vector vrt
        res = metadata.querySublayers(os.path.join(TEST_DATA_DIR, "vector_vrt.vrt"), Qgis.SublayerQueryFlag.FastScan)
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layerNumber(), 0)
        self.assertEqual(res[0].name(), "vector_vrt")
        self.assertEqual(res[0].description(), "")
        self.assertEqual(res[0].uri(), os.path.join(TEST_DATA_DIR, "vector_vrt.vrt"))
        self.assertEqual(res[0].providerKey(), "ogr")
        self.assertEqual(res[0].type(), QgsMapLayerType.VectorLayer)
        self.assertTrue(res[0].skippedContainerScan())

        # raster vrt
        res = metadata.querySublayers(os.path.join(TEST_DATA_DIR, "/raster/hub13263.vrt"), Qgis.SublayerQueryFlag.FastScan)
        self.assertEqual(len(res), 0)

        # metadata.xml file next to tdenv?.adf file -- this is a subcomponent of an ESRI tin layer, should not be exposed
        res = metadata.querySublayers(
            os.path.join(TEST_DATA_DIR, 'esri_tin', 'metadata.xml'), Qgis.SublayerQueryFlag.FastScan)
        self.assertFalse(res)

        # ESRI Arcinfo file
        res = metadata.querySublayers(
            os.path.join(TEST_DATA_DIR, 'esri_coverage', 'testpolyavc'), Qgis.SublayerQueryFlag.FastScan)
        self.assertEqual(len(res), 4)
        self.assertEqual(res[0].layerNumber(), 0)
        self.assertEqual(res[0].name(), "ARC")
        self.assertEqual(res[0].description(), "")
        self.assertEqual(res[0].uri(), '{}|layername=ARC'.format(os.path.join(TEST_DATA_DIR, 'esri_coverage', 'testpolyavc')))
        self.assertEqual(res[0].providerKey(), "ogr")
        self.assertEqual(res[0].type(), QgsMapLayerType.VectorLayer)
        self.assertFalse(res[0].skippedContainerScan())

        # zip file layer vector, explicit file in zip
        res = metadata.querySublayers('/vsizip/' + TEST_DATA_DIR + '/zip/points2.zip/points.shp', Qgis.SublayerQueryFlag.FastScan)
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layerNumber(), 0)
        self.assertEqual(res[0].name(), "points")
        self.assertEqual(res[0].description(), '')
        self.assertEqual(res[0].uri(), '/vsizip/' + TEST_DATA_DIR + "/zip/points2.zip/points.shp")
        self.assertEqual(res[0].providerKey(), "ogr")
        self.assertEqual(res[0].type(), QgsMapLayerType.VectorLayer)
        self.assertEqual(res[0].wkbType(), QgsWkbTypes.Unknown)
        self.assertEqual(res[0].geometryColumnName(), '')
        options = QgsProviderSublayerDetails.LayerOptions(QgsCoordinateTransformContext())
        vl = res[0].toLayer(options)
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.Point)

        # zip file layer vector, explicit file in zip which is NOT a OGR supported source
        res = metadata.querySublayers('/vsizip/' + TEST_DATA_DIR + '/zip/points2.zip/points.qml', Qgis.SublayerQueryFlag.FastScan)
        self.assertEqual(len(res), 0)

    def test_provider_sidecar_files_for_uri(self):
        """
        Test retrieving sidecar files for uris
        """
        metadata = QgsProviderRegistry.instance().providerMetadata('ogr')

        self.assertEqual(metadata.sidecarFilesForUri(''), [])
        self.assertEqual(metadata.sidecarFilesForUri('/home/me/not special.doc'), [])
        self.assertEqual(metadata.sidecarFilesForUri('/home/me/special.shp'),
                         ['/home/me/special.shx', '/home/me/special.dbf', '/home/me/special.sbn',
                          '/home/me/special.sbx', '/home/me/special.prj', '/home/me/special.idm',
                          '/home/me/special.ind', '/home/me/special.qix', '/home/me/special.cpg',
                          '/home/me/special.qpj', '/home/me/special.shp.xml'])
        self.assertEqual(metadata.sidecarFilesForUri('/home/me/special.tab'),
                         ['/home/me/special.dat', '/home/me/special.id', '/home/me/special.map', '/home/me/special.ind',
                          '/home/me/special.tda', '/home/me/special.tin', '/home/me/special.tma',
                          '/home/me/special.lda', '/home/me/special.lin', '/home/me/special.lma'])
        self.assertEqual(metadata.sidecarFilesForUri('/home/me/special.mif'),
                         ['/home/me/special.mid'])
        self.assertEqual(metadata.sidecarFilesForUri('/home/me/special.gml'),
                         ['/home/me/special.gfs', '/home/me/special.xsd'])
        self.assertEqual(metadata.sidecarFilesForUri('/home/me/special.csv'), ['/home/me/special.csvt'])

    def testGeoJsonFieldOrder(self):
        """Test issue GH #45139"""

        d = QTemporaryDir()
        json_path = os.path.join(d.path(), 'test.geojson')
        with open(json_path, 'w+') as f:
            f.write("""
            {
                "type": "FeatureCollection",
                "features": [
                    {
                        "type": "Feature",
                        "geometry": {
                            "type": "Point",
                            "coordinates": [11.1215698,46.0677293]
                        },
                        "properties": {
                            "A": "A",
                        }
                    },
                    {
                        "type": "Feature",
                        "geometry": {
                            "type": "Point",
                            "coordinates": [11.1214686,46.0677385]
                        },
                        "properties": {
                            "A": "A",
                            "B": "B",
                        }
                    }
                ]
            }
            """)

        vl = QgsVectorLayer(json_path, 'json')
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.featureCount(), 2)
        self.assertEqual(vl.fields().names(), ['A', 'B'])

        # Append a field
        self.assertTrue(vl.startEditing())
        self.assertTrue(vl.addAttribute(QgsField('C', QVariant.String)))

        for f in vl.getFeatures():
            vl.changeAttributeValue(f.id(), 2, 'C')

        self.assertEqual(vl.fields().names(), ['A', 'B', 'C'])

        features = [f for f in vl.getFeatures()]

        self.assertEqual(features[0].attribute('B'), NULL)
        self.assertEqual(features[0].attribute('C'), 'C')
        self.assertEqual(features[1].attribute('B'), 'B')
        self.assertEqual(features[1].attribute('C'), 'C')

        self.assertTrue(vl.commitChanges())

        # This has been fixed in GDAL >= 3.4
        if int(gdal.VersionInfo('VERSION_NUM')) >= GDAL_COMPUTE_VERSION(3, 4, 0):
            self.assertEqual(vl.fields().names(), ['A', 'B', 'C'])
        else:
            self.assertEqual(vl.fields().names(), ['A', 'C', 'B'])

        features = [f for f in vl.getFeatures()]

        self.assertEqual(features[0].attribute('B'), NULL)
        self.assertEqual(features[0].attribute('C'), 'C')
        self.assertEqual(features[1].attribute('B'), 'B')
        self.assertEqual(features[1].attribute('C'), 'C')

    def test_provider_feature_iterator_options(self):
        """Test issue GH #45534"""

        datasource = os.path.join(self.basetestpath, 'testProviderFeatureIteratorOptions.csv')
        with open(datasource, 'wt') as f:
            f.write('id,Longitude,Latitude\n')
            f.write('1,1.0,1.0\n')
            f.write('2,2.0,2.0\n')

        vl = QgsVectorLayer('{}|option:X_POSSIBLE_NAMES=Longitude|option:Y_POSSIBLE_NAMES=Latitude'.format(datasource), 'test', 'ogr')
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.Point)

        f = vl.getFeature(1)
        self.assertEqual(f.geometry().asWkt(), 'Point (1 1)')
        f = vl.getFeature(2)
        self.assertEqual(f.geometry().asWkt(), 'Point (2 2)')

    def test_provider_dxf_3d(self):
        """Test issue GH #45938"""

        metadata = QgsProviderRegistry.instance().providerMetadata('ogr')
        layers = metadata.querySublayers(os.path.join(TEST_DATA_DIR, 'points_lines_3d.dxf'),
                                         Qgis.SublayerQueryFlag.ResolveGeometryType)

        options = QgsProviderSublayerDetails.LayerOptions(QgsCoordinateTransformContext())

        for ld in layers:
            if ld.wkbType() == QgsWkbTypes.PointZ:
                point_layer = ld.toLayer(options)
            if ld.wkbType() == QgsWkbTypes.LineStringZ:
                polyline_layer = ld.toLayer(options)

        self.assertTrue(point_layer.isValid())
        self.assertEqual(point_layer.featureCount(), 11)
        feature = next(point_layer.getFeatures())
        self.assertTrue(feature.isValid())
        self.assertEqual(feature.geometry().wkbType(), QgsWkbTypes.Point25D)
        self.assertEqual(feature.geometry().asWkt(),
                         'PointZ (635660.10747100005391985 1768912.79759799991734326 3.36980799999999991)')

        self.assertTrue(polyline_layer.isValid())
        self.assertEqual(polyline_layer.featureCount(), 2)
        feature = next(polyline_layer.getFeatures())
        self.assertTrue(feature.isValid())
        self.assertEqual(feature.geometry().wkbType(), QgsWkbTypes.LineString25D)
        self.assertEqual(feature.geometry().vertexAt(1).asWkt(),
                         'PointZ (635660.11699699994642287 1768910.93880999996326864 3.33884099999999995)')


if __name__ == '__main__':
    unittest.main()
