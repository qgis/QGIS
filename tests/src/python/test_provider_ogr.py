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
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
import shutil
import sys
import tempfile

from osgeo import gdal, ogr  # NOQA
from qgis.core import (QgsFeature, QgsFeatureRequest, QgsSettings,
                       QgsVectorDataProvider, QgsVectorLayer, QgsWkbTypes, QgsNetworkAccessManager)
from qgis.testing import start_app, unittest

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
        self.assertEqual(vl.dataProvider().subLayers()[0], '0:testMixOfPolygonCurvePolygon:4:CurvePolygon:')

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
        self.assertEqual(vl.dataProvider().subLayers()[0], '0:testMixOfLineStringCompoundCurve:5:CompoundCurve:')

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
            ("Triangle Z((0 0 1, 0 1 2, 1 1 3, 0 0 1))", QgsWkbTypes.TriangleZ, "TriangleZ ((0 0 1, 0 1 2, 1 1 3, 0 0 1))"),
            ("Triangle M((0 0 4, 0 1 5, 1 1 6, 0 0 4))", QgsWkbTypes.TriangleM, "TriangleM ((0 0 4, 0 1 5, 1 1 6, 0 0 4))"),
            ("Triangle ZM((0 0 0 1, 0 1 2 3, 1 1 4 5, 0 0 0 1))", QgsWkbTypes.TriangleZM, "TriangleZM ((0 0 0 1, 0 1 2 3, 1 1 4 5, 0 0 0 1))"),

            ("TIN (((0 0, 0 1, 1 1, 0 0)),((0 0, 1 0, 1 1, 0 0)))", QgsWkbTypes.MultiPolygon, "MultiPolygon (((0 0, 0 1, 1 1, 0 0)),((0 0, 1 0, 1 1, 0 0)))"),
            ("TIN Z(((0 0 0, 0 1 1, 1 1 1, 0 0 0)),((0 0 0, 1 0 0, 1 1 1, 0 0 0)))", QgsWkbTypes.MultiPolygonZ, "MultiPolygonZ (((0 0 0, 0 1 1, 1 1 1, 0 0 0)),((0 0 0, 1 0 0, 1 1 1, 0 0 0)))"),
            ("TIN M(((0 0 0, 0 1 2, 1 1 3, 0 0 0)),((0 0 0, 1 0 4, 1 1 3, 0 0 0)))", QgsWkbTypes.MultiPolygonM, "MultiPolygonM (((0 0 0, 0 1 2, 1 1 3, 0 0 0)),((0 0 0, 1 0 4, 1 1 3, 0 0 0)))"),
            ("TIN ZM(((0 0 0 0, 0 1 1 2, 1 1 1 3, 0 0 0 0)),((0 0 0 0, 1 0 0 4, 1 1 1 3, 0 0 0 0)))", QgsWkbTypes.MultiPolygonZM, "MultiPolygonZM (((0 0 0 0, 0 1 1 2, 1 1 1 3, 0 0 0 0)),((0 0 0 0, 1 0 0 4, 1 1 1 3, 0 0 0 0)))"),

            ("PolyhedralSurface (((0 0, 0 1, 1 1, 0 0)),((0 0, 1 0, 1 1, 0 0)))", QgsWkbTypes.MultiPolygon, "MultiPolygon (((0 0, 0 1, 1 1, 0 0)),((0 0, 1 0, 1 1, 0 0)))"),
            ("PolyhedralSurface Z(((0 0 0, 0 1 1, 1 1 1, 0 0 0)),((0 0 0, 1 0 0, 1 1 1, 0 0 0)))", QgsWkbTypes.MultiPolygonZ, "MultiPolygonZ (((0 0 0, 0 1 1, 1 1 1, 0 0 0)),((0 0 0, 1 0 0, 1 1 1, 0 0 0)))"),
            ("PolyhedralSurface M(((0 0 0, 0 1 2, 1 1 3, 0 0 0)),((0 0 0, 1 0 4, 1 1 3, 0 0 0)))", QgsWkbTypes.MultiPolygonM, "MultiPolygonM (((0 0 0, 0 1 2, 1 1 3, 0 0 0)),((0 0 0, 1 0 4, 1 1 3, 0 0 0)))"),
            ("PolyhedralSurface ZM(((0 0 0 0, 0 1 1 2, 1 1 1 3, 0 0 0 0)),((0 0 0 0, 1 0 0 4, 1 1 1 3, 0 0 0 0)))", QgsWkbTypes.MultiPolygonZM, "MultiPolygonZM (((0 0 0 0, 0 1 1 2, 1 1 1 3, 0 0 0 0)),((0 0 0 0, 1 0 0 4, 1 1 1 3, 0 0 0 0)))")
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


if __name__ == '__main__':
    unittest.main()
