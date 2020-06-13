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

import os
import shutil
import sys
import tempfile
import hashlib
from datetime import datetime

from osgeo import gdal, ogr  # NOQA
from qgis.PyQt.QtCore import QVariant, QByteArray
from qgis.core import (NULL,
                       QgsApplication,
                       QgsRectangle,
                       QgsProviderRegistry,
                       QgsFeature, QgsFeatureRequest, QgsField, QgsSettings, QgsDataProvider,
                       QgsVectorDataProvider, QgsVectorLayer, QgsWkbTypes, QgsNetworkAccessManager)
from qgis.testing import start_app, unittest

from utilities import unitTestDataPath
from qgis.utils import spatialite_connect

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

    def testCapabilities(self):
        self.assertTrue(QgsProviderRegistry.instance().providerCapabilities("ogr") & QgsDataProvider.File)
        self.assertTrue(QgsProviderRegistry.instance().providerCapabilities("ogr") & QgsDataProvider.Dir)

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

        registry = QgsApplication.dataItemProviderRegistry()
        ogrprovider = next(provider for provider in registry.providers() if provider.name() == 'OGR')

        # Single layer
        item = ogrprovider.createDataItem(os.path.join(TEST_DATA_DIR, 'lines.shp'), None)
        self.assertTrue(item.uri().endswith('lines.shp'))

        # Multiple layer
        item = ogrprovider.createDataItem(os.path.join(TEST_DATA_DIR, 'multilayer.kml'), None)
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
        item = ogrprovider.createDataItem(tmpfile, None)
        children = item.createChildren()
        self.assertEqual(len(children), 2)
        self.assertIn('testDataItems.gpkg|layername=Layer1', children[0].uri())
        self.assertIn('testDataItems.gpkg|layername=Layer2', children[1].uri())

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

    @unittest.skip(int(gdal.VersionInfo('VERSION_NUM')) < GDAL_COMPUTE_VERSION(2, 4, 0))
    def testStringListField(self):
        source = os.path.join(TEST_DATA_DIR, 'stringlist.gml')
        vl = QgsVectorLayer(source)
        self.assertTrue(vl.isValid())

        fields = vl.fields()
        descriptive_group_field = fields[fields.lookupField('descriptiveGroup')]
        self.assertEqual(descriptive_group_field.type(), QVariant.List)
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
        self.assertEqual(list_field.type(), QVariant.List)
        self.assertEqual(list_field.typeName(), 'StringList')
        self.assertEqual(list_field.subType(), QVariant.String)

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


if __name__ == '__main__':
    unittest.main()
