# -*- coding: utf-8 -*-
"""QGIS Unit tests for the OGR/GPKG provider.

From build dir, run:
ctest -R PyQgsOGRProviderGpkg -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Even Rouault'
__date__ = '2016-04-21'
__copyright__ = 'Copyright 2016, Even Rouault'

import os
import re
import shutil
import sys
import tempfile
import time

import qgis  # NOQA
from osgeo import gdal, ogr
from providertestbase import ProviderTestCase
from qgis.core import (Qgis,
                       QgsFeature,
                       QgsCoordinateReferenceSystem,
                       QgsFeatureRequest,
                       QgsFeatureSink,
                       QgsFields,
                       QgsField,
                       QgsFieldConstraints,
                       QgsGeometry,
                       QgsProviderRegistry,
                       QgsRectangle,
                       QgsSettings,
                       QgsVectorLayer,
                       QgsVectorLayerExporter,
                       QgsPointXY,
                       QgsProject,
                       QgsWkbTypes,
                       QgsDataProvider,
                       QgsVectorDataProvider,
                       QgsLayerMetadata,
                       NULL)
from qgis.PyQt.QtCore import QCoreApplication, QVariant, QDate, QTime, QDateTime, Qt, QTemporaryDir, QFileInfo
from qgis.PyQt.QtXml import QDomDocument
from qgis.testing import start_app, unittest
from qgis.utils import spatialite_connect
from utilities import unitTestDataPath

from sqlite3 import OperationalError

TEST_DATA_DIR = unitTestDataPath()


def GDAL_COMPUTE_VERSION(maj, min, rev):
    return ((maj) * 1000000 + (min) * 10000 + (rev) * 100)

#########################################################################
# Standard conformance tests for a provider
#########################################################################


class TestPyQgsOGRProviderGpkgConformance(unittest.TestCase, ProviderTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        # Create test layer
        cls.basetestpath = tempfile.mkdtemp()
        cls.repackfilepath = tempfile.mkdtemp()

        srcpath = os.path.join(TEST_DATA_DIR, 'provider')
        shutil.copy(os.path.join(srcpath, 'geopackage.gpkg'), cls.basetestpath)
        shutil.copy(os.path.join(srcpath, 'geopackage_poly.gpkg'),
                    cls.basetestpath)
        cls.basetestfile = os.path.join(cls.basetestpath, 'geopackage.gpkg')
        cls.basetestpolyfile = os.path.join(
            cls.basetestpath, 'geopackage_poly.gpkg')
        cls.vl = QgsVectorLayer(
            cls.basetestfile + '|layername=geopackage', 'test', 'ogr')
        assert(cls.vl.isValid())
        cls.source = cls.vl.dataProvider()
        cls.vl_poly = QgsVectorLayer(cls.basetestpolyfile, 'test', 'ogr')
        assert (cls.vl_poly.isValid())
        cls.poly_provider = cls.vl_poly.dataProvider()

        cls.dirs_to_cleanup = [cls.basetestpath, cls.repackfilepath]

        # Create the other layer for constraints check
        cls.check_constraint = QgsVectorLayer(
            cls.basetestfile + '|layername=check_constraint', 'check_constraint', 'ogr')
        cls.check_constraint_editing_started = False

        # Create the other layer for unique and not null constraints check
        cls.unique_not_null_constraints = QgsVectorLayer(
            cls.basetestfile + '|layername=unique_not_null_constraints', 'unique_not_null_constraints', 'ogr')
        assert cls.unique_not_null_constraints.isValid()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        del(cls.vl)
        del(cls.vl_poly)
        del(cls.check_constraint)
        del(cls.unique_not_null_constraints)
        for dirname in cls.dirs_to_cleanup:
            shutil.rmtree(dirname, True)

    def getSource(self):
        tmpdir = tempfile.mkdtemp()
        self.dirs_to_cleanup.append(tmpdir)
        srcpath = os.path.join(TEST_DATA_DIR, 'provider')
        shutil.copy(os.path.join(srcpath, 'geopackage.gpkg'), tmpdir)
        datasource = os.path.join(tmpdir, 'geopackage.gpkg')

        vl = QgsVectorLayer(datasource, 'test', 'ogr')

        return vl

    def getEditableLayerWithCheckConstraint(self):
        """Returns the layer for attribute change CHECK constraint violation"""
        if not self.check_constraint_editing_started:
            self.assertFalse(self.check_constraint_editing_started)
            self.check_constraint_editing_started = True
            self.check_constraint.startEditing()
        return self.check_constraint

    def stopEditableLayerWithCheckConstraint(self):
        self.assertTrue(self.check_constraint_editing_started)
        self.check_constraint_editing_started = False
        self.check_constraint.commitChanges()

    def getEditableLayerWithUniqueNotNullConstraints(self):
        """Returns the layer for UNIQUE and NOT NULL constraints detection"""

        return self.unique_not_null_constraints

    def enableCompiler(self):
        QgsSettings().setValue('/qgis/compileExpressions', True)
        return True

    def disableCompiler(self):
        QgsSettings().setValue('/qgis/compileExpressions', False)

    def treat_time_as_string(self):
        return True

    def uncompiledFilters(self):
        return set(['cnt = 10 ^ 2',
                    '"name" ~ \'[OP]ra[gne]+\'',
                    'sqrt(pk) >= 2',
                    'radians(cnt) < 2',
                    'degrees(pk) <= 200',
                    'cos(pk) < 0',
                    'sin(pk) < 0',
                    'tan(pk) < 0',
                    'acos(-1) < pk',
                    'asin(1) < pk',
                    'atan(3.14) < pk',
                    'atan2(3.14, pk) < 1',
                    'exp(pk) < 10',
                    'ln(pk) <= 1',
                    'log(3, pk) <= 1',
                    'log10(pk) < 0.5',
                    'floor(3.14) <= pk',
                    'ceil(3.14) <= pk',
                    'pk < pi()',
                    'floor(cnt / 66.67) <= 2',
                    'ceil(cnt / 66.67) <= 2',
                    'pk < pi() / 2',
                    'x($geometry) < -70',
                    'y($geometry) > 70',
                    'xmin($geometry) < -70',
                    'ymin($geometry) > 70',
                    'xmax($geometry) < -70',
                    'ymax($geometry) > 70',
                    'disjoint($geometry,geom_from_wkt( \'Polygon ((-72.2 66.1, -65.2 66.1, -65.2 72.0, -72.2 72.0, -72.2 66.1))\'))',
                    'intersects($geometry,geom_from_wkt( \'Polygon ((-72.2 66.1, -65.2 66.1, -65.2 72.0, -72.2 72.0, -72.2 66.1))\'))',
                    'contains(geom_from_wkt( \'Polygon ((-72.2 66.1, -65.2 66.1, -65.2 72.0, -72.2 72.0, -72.2 66.1))\'),$geometry)',
                    'distance($geometry,geom_from_wkt( \'Point (-70 70)\')) > 7',
                    'intersects($geometry,geom_from_gml( \'<gml:Polygon srsName="EPSG:4326"><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>-72.2,66.1 -65.2,66.1 -65.2,72.0 -72.2,72.0 -72.2,66.1</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon>\'))',
                    'x($geometry) < -70',
                    'y($geometry) > 79',
                    'xmin($geometry) < -70',
                    'ymin($geometry) < 76',
                    'xmax($geometry) > -68',
                    'ymax($geometry) > 80',
                    'area($geometry) > 10',
                    'perimeter($geometry) < 12',
                    'relate($geometry,geom_from_wkt( \'Polygon ((-68.2 82.1, -66.95 82.1, -66.95 79.05, -68.2 79.05, -68.2 82.1))\')) = \'FF2FF1212\'',
                    'relate($geometry,geom_from_wkt( \'Polygon ((-68.2 82.1, -66.95 82.1, -66.95 79.05, -68.2 79.05, -68.2 82.1))\'), \'****F****\')',
                    'crosses($geometry,geom_from_wkt( \'Linestring (-68.2 82.1, -66.95 82.1, -66.95 79.05)\'))',
                    'overlaps($geometry,geom_from_wkt( \'Polygon ((-68.2 82.1, -66.95 82.1, -66.95 79.05, -68.2 79.05, -68.2 82.1))\'))',
                    'within($geometry,geom_from_wkt( \'Polygon ((-75.1 76.1, -75.1 81.6, -68.8 81.6, -68.8 76.1, -75.1 76.1))\'))',
                    'overlaps(translate($geometry,-1,-1),geom_from_wkt( \'Polygon ((-75.1 76.1, -75.1 81.6, -68.8 81.6, -68.8 76.1, -75.1 76.1))\'))',
                    'overlaps(buffer($geometry,1),geom_from_wkt( \'Polygon ((-75.1 76.1, -75.1 81.6, -68.8 81.6, -68.8 76.1, -75.1 76.1))\'))',
                    'intersects(centroid($geometry),geom_from_wkt( \'Polygon ((-74.4 78.2, -74.4 79.1, -66.8 79.1, -66.8 78.2, -74.4 78.2))\'))',
                    'intersects(point_on_surface($geometry),geom_from_wkt( \'Polygon ((-74.4 78.2, -74.4 79.1, -66.8 79.1, -66.8 78.2, -74.4 78.2))\'))',
                    '"dt" = to_datetime(\'000www14ww13ww12www4ww5ww2020\',\'zzzwwwsswwmmwwhhwwwdwwMwwyyyy\')',
                    'to_time("time") >= make_time(12, 14, 14)',
                    'to_time("time") = to_time(\'000www14ww13ww12www\',\'zzzwwwsswwmmwwhhwww\')',
                    '"date" = to_date(\'www4ww5ww2020\',\'wwwdwwMwwyyyy\')'
                    ])

    def partiallyCompiledFilters(self):
        return set(['"name" NOT LIKE \'Ap%\'',
                    'name LIKE \'Apple\'',
                    'name LIKE \'aPple\'',
                    'name LIKE \'Ap_le\'',
                    'name LIKE \'Ap\\_le\''
                    ])


class ErrorReceiver():

    def __init__(self):
        self.msg = None

    def receiveError(self, msg):
        self.msg = msg


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

#########################################################################
# Other tests specific to GPKG handling in OGR provider
#########################################################################


class TestPyQgsOGRProviderGpkg(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""

        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("TestPyQgsOGRProviderGpkg.com")
        QCoreApplication.setApplicationName("TestPyQgsOGRProviderGpkg")
        QgsSettings().clear()
        start_app()

        # Create test layer
        cls.basetestpath = tempfile.mkdtemp()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        shutil.rmtree(cls.basetestpath, True)

        QgsSettings().clear()

    def testDecodeUri(self):

        filename = '/home/to/path/my_file.gpkg'

        registry = QgsProviderRegistry.instance()
        uri = filename
        components = registry.decodeUri('ogr', uri)
        self.assertEqual(components["path"], filename)

        uri = '{}|layername=test'.format(filename)
        components = registry.decodeUri('ogr', uri)
        self.assertEqual(components["path"], filename)
        self.assertEqual(components["layerName"], 'test')

        uri = '{}|layerName=test'.format(filename)
        components = registry.decodeUri('ogr', uri)
        self.assertEqual(components["path"], filename)
        self.assertEqual(components["layerName"], 'test')

        uri = '{}|layerid=0'.format(filename)
        components = registry.decodeUri('ogr', uri)
        self.assertEqual(components["path"], filename)
        self.assertEqual(components["layerId"], 0)

        uri = '{}|layerId=0'.format(filename)
        components = registry.decodeUri('ogr', uri)
        self.assertEqual(components["path"], filename)
        self.assertEqual(components["layerId"], 0)

        uri = '{}|geometryType=POINT'.format(filename)
        components = registry.decodeUri('ogr', uri)
        self.assertEqual(components["path"], filename)
        self.assertEqual(components["geometryType"], 'POINT')

    def testEncodeUri(self):

        filename = '/home/to/path/my_file.gpkg'
        registry = QgsProviderRegistry.instance()

        parts = {"path": filename}
        uri = registry.encodeUri('ogr', parts)
        self.assertEqual(uri, filename)

        # layerName only
        parts["layerName"] = "test"
        uri = registry.encodeUri('ogr', parts)
        self.assertEqual(uri, '{}|layername=test'.format(filename))
        del parts["layerName"]

        # layerId only
        parts["layerId"] = "0"
        uri = registry.encodeUri('ogr', parts)
        self.assertEqual(uri, '{}|layerid=0'.format(filename))

        # Both layerName and layerId: layerName takes precedence
        parts["layerName"] = "test"
        uri = registry.encodeUri('ogr', parts)
        self.assertEqual(uri, '{}|layername=test'.format(filename))

    def testSingleToMultiPolygonPromotion(self):

        tmpfile = os.path.join(self.basetestpath, 'testSingleToMultiPolygonPromotion.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        ds.CreateLayer('test', geom_type=ogr.wkbMultiPolygon)
        ds = None

        vl = QgsVectorLayer('{}|layerid=0'.format(tmpfile), 'test', 'ogr')
        f = QgsFeature()
        f.setGeometry(QgsGeometry.fromWkt('POLYGON ((0 0,0 1,1 1,0 0))'))
        vl.dataProvider().addFeatures([f])
        got = [feat for feat in vl.getFeatures()][0]
        got_geom = got.geometry()
        reference = QgsGeometry.fromWkt('MultiPolygon (((0 0, 0 1, 1 1, 0 0)))')
        # The geometries must be binarily identical
        self.assertEqual(got_geom.asWkb(), reference.asWkb(),
                         'Expected {}, got {}'.format(reference.asWkt(), got_geom.asWkt()))

    def testCurveGeometryType(self):

        tmpfile = os.path.join(self.basetestpath, 'testCurveGeometryType.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        ds.CreateLayer('test', geom_type=ogr.wkbCurvePolygon)
        ds = None

        vl = QgsVectorLayer('{}'.format(tmpfile), 'test', 'ogr')
        self.assertEqual(1, vl.dataProvider().subLayerCount())
        self.assertEqual(vl.dataProvider().subLayers(),
                         [QgsDataProvider.SUBLAYER_SEPARATOR.join(['0', 'test', '0', 'CurvePolygon', 'geom', ''])])
        f = QgsFeature()
        f.setGeometry(QgsGeometry.fromWkt('POLYGON ((0 0,0 1,1 1,0 0))'))
        vl.dataProvider().addFeatures([f])
        got = [feat for feat in vl.getFeatures()][0]
        got_geom = got.geometry()
        reference = QgsGeometry.fromWkt('CurvePolygon ((0 0, 0 1, 1 1, 0 0))')
        # The geometries must be binarily identical
        self.assertEqual(got_geom.asWkb(), reference.asWkb(),
                         'Expected {}, got {}'.format(reference.asWkt(), got_geom.asWkt()))

    def internalTestBug15351(self, orderClosing):

        tmpfile = os.path.join(self.basetestpath, 'testBug15351.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbPoint)
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetGeometry(ogr.CreateGeometryFromWkt('POINT(0 0)'))
        lyr.CreateFeature(f)
        f = None
        ds = None

        vl = QgsVectorLayer(u'{}'.format(tmpfile), u'test', u'ogr')
        self.assertTrue(vl.startEditing())
        self.assertTrue(vl.changeGeometry(1, QgsGeometry.fromWkt('Point (3 50)')))

        # Iterate over features (will open a new OGR connection), but do not
        # close the iterator for now
        it = vl.getFeatures()
        f = QgsFeature()
        it.nextFeature(f)

        if orderClosing == 'closeIter_commit_closeProvider':
            it = None

        # Commit changes
        cbk = ErrorReceiver()
        vl.dataProvider().raiseError.connect(cbk.receiveError)
        self.assertTrue(vl.commitChanges())
        self.assertIsNone(cbk.msg)

        # Close layer and iterator in different orders
        if orderClosing == 'closeIter_commit_closeProvider':
            vl = None
        elif orderClosing == 'commit_closeProvider_closeIter':
            vl = None
            it = None
        else:
            assert orderClosing == 'commit_closeIter_closeProvider'
            it = None
            vl = None

        # Test that we succeeded restoring default journal mode, and we
        # are not let in WAL mode.
        ds = ogr.Open(tmpfile)
        lyr = ds.ExecuteSQL('PRAGMA journal_mode')
        f = lyr.GetNextFeature()
        res = f.GetField(0)
        ds.ReleaseResultSet(lyr)
        ds = None
        self.assertEqual(res, 'delete')

    # We need GDAL 2.0 to issue PRAGMA journal_mode
    # Note: for that case, we don't strictly need turning on WAL
    def testBug15351_closeIter_commit_closeProvider(self):
        self.internalTestBug15351('closeIter_commit_closeProvider')

    # We need GDAL 2.0 to issue PRAGMA journal_mode
    def testBug15351_commit_closeProvider_closeIter(self):
        self.internalTestBug15351('commit_closeProvider_closeIter')

    # We need GDAL 2.0 to issue PRAGMA journal_mode
    def testBug15351_commit_closeIter_closeProvider(self):
        self.internalTestBug15351('commit_closeIter_closeProvider')

    def testGeopackageExtentUpdate(self):
        ''' test https://github.com/qgis/QGIS/issues/23209 '''
        tmpfile = os.path.join(self.basetestpath, 'testGeopackageExtentUpdate.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbPoint)
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetGeometry(ogr.CreateGeometryFromWkt('POINT(0 0)'))
        lyr.CreateFeature(f)
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetGeometry(ogr.CreateGeometryFromWkt('POINT(1 1)'))
        lyr.CreateFeature(f)
        f = None
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetGeometry(ogr.CreateGeometryFromWkt('POINT(1 0.5)'))
        lyr.CreateFeature(f)
        f = None
        gdal.ErrorReset()
        ds.ExecuteSQL('RECOMPUTE EXTENT ON test')
        has_error = gdal.GetLastErrorMsg() != ''
        ds = None
        if has_error:
            print('Too old GDAL trunk version. Please update')
            return

        vl = QgsVectorLayer(u'{}'.format(tmpfile), u'test', u'ogr')

        # Test moving a geometry that touches the bbox
        self.assertTrue(vl.startEditing())
        self.assertTrue(vl.changeGeometry(1, QgsGeometry.fromWkt('Point (0.5 0)')))
        self.assertTrue(vl.commitChanges())
        reference = QgsGeometry.fromRect(QgsRectangle(0.5, 0.0, 1.0, 1.0))
        provider_extent = QgsGeometry.fromRect(vl.extent())
        self.assertTrue(QgsGeometry.compare(provider_extent.asPolygon()[0], reference.asPolygon()[0], 0.00001),
                        provider_extent.asPolygon()[0])

        # Test deleting a geometry that touches the bbox
        self.assertTrue(vl.startEditing())
        self.assertTrue(vl.deleteFeature(2))
        self.assertTrue(vl.commitChanges())
        reference = QgsGeometry.fromRect(QgsRectangle(0.5, 0.0, 1.0, 0.5))
        provider_extent = QgsGeometry.fromRect(vl.extent())
        self.assertTrue(QgsGeometry.compare(provider_extent.asPolygon()[0], reference.asPolygon()[0], 0.00001),
                        provider_extent.asPolygon()[0])

    def testSelectSubsetString(self):

        tmpfile = os.path.join(self.basetestpath, 'testSelectSubsetString.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbMultiPolygon)
        lyr.CreateField(ogr.FieldDefn('foo', ogr.OFTString))
        f = ogr.Feature(lyr.GetLayerDefn())
        f['foo'] = 'bar'
        lyr.CreateFeature(f)
        f = None
        f = ogr.Feature(lyr.GetLayerDefn())
        f['foo'] = 'baz'
        lyr.CreateFeature(f)
        f = None
        ds = None

        vl = QgsVectorLayer('{}|layerid=0'.format(tmpfile), 'test', 'ogr')
        vl.setSubsetString("SELECT fid, foo FROM test WHERE foo = 'baz'")
        got = [feat for feat in vl.getFeatures()]
        self.assertEqual(len(got), 1)
        del vl

        testdata_path = unitTestDataPath('provider')
        shutil.copy(os.path.join(testdata_path, 'bug_19826.gpkg'), tmpfile)
        vl = QgsVectorLayer('{}|layerid=0'.format(tmpfile), 'test', 'ogr')
        vl.setSubsetString("name = 'two'")
        got = [feat for feat in vl.getFeatures()]
        self.assertEqual(len(got), 1)

        attributes = got[0].attributes()
        self.assertEqual(got[0].id(), 2)
        self.assertEqual(attributes[0], 2)
        self.assertEqual(attributes[1], 'two')
        self.assertNotEqual(attributes[2], None)

        # Request by FeatureId on a subset layer
        got = [feat for feat in vl.getFeatures(QgsFeatureRequest(2))]
        self.assertEqual(len(got), 1)
        attributes = got[0].attributes()
        self.assertEqual(got[0].id(), 2)
        self.assertEqual(attributes[0], 2)
        self.assertEqual(attributes[1], 'two')
        self.assertNotEqual(attributes[2], None)

        request = QgsFeatureRequest(2).setSubsetOfAttributes([0])
        got = [feat for feat in vl.getFeatures(request)]
        self.assertEqual(len(got), 1)
        attributes = got[0].attributes()
        self.assertEqual(got[0].id(), 2)
        self.assertEqual(attributes[0], 2)
        self.assertEqual(attributes[1], None)
        self.assertEqual(attributes[2], None)

        # Request by FeatureId on a subset layer. The name = 'two' filter
        # only returns FID 2, so requesting on FID 1 should return nothing
        # but this is broken now.
        got = [feat for feat in vl.getFeatures(QgsFeatureRequest(1))]
        self.assertEqual(len(got), 1)  # this is the current behavior, broken

    def testEditSubsetString(self):

        tmpfile = os.path.join(self.basetestpath, 'testEditSubsetString.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbMultiPolygon)
        lyr.CreateField(ogr.FieldDefn('foo', ogr.OFTString))
        f = ogr.Feature(lyr.GetLayerDefn())
        f['foo'] = 'bar'
        lyr.CreateFeature(f)
        f = None
        f = ogr.Feature(lyr.GetLayerDefn())
        f['foo'] = 'baz'
        lyr.CreateFeature(f)
        f = None
        ds = None

        vl = QgsVectorLayer('{}|layerid=0'.format(tmpfile), 'test', 'ogr')
        self.assertEqual(vl.dataProvider().featureCount(), 2)

        # Test adding features
        vl.setSubsetString("foo = 'baz'")
        self.assertTrue(vl.startEditing())
        feature = QgsFeature(vl.fields())
        feature['foo'] = 'abc'
        vl.addFeature(feature)
        vl.commitChanges()
        vl.setSubsetString(None)
        self.assertEqual(vl.dataProvider().featureCount(), 3)

        # Test deleting a feature
        vl.setSubsetString("foo = 'baz'")
        self.assertTrue(vl.startEditing())
        vl.deleteFeature(1)
        vl.commitChanges()
        vl.setSubsetString(None)
        self.assertEqual(vl.dataProvider().featureCount(), 2)

        # Test editing a feature
        vl.setSubsetString("foo = 'baz'")
        self.assertTrue(vl.startEditing())
        vl.changeAttributeValue(2, 1, 'xx')
        vl.commitChanges()
        vl.setSubsetString(None)
        self.assertEqual(set((feat['foo'] for feat in vl.getFeatures())), set(['xx', 'abc']))

    def testStyle(self):

        # First test with invalid URI
        vl = QgsVectorLayer('/idont/exist.gpkg', 'test', 'ogr')

        self.assertFalse(vl.dataProvider().isSaveAndLoadStyleToDatabaseSupported())

        res, err = QgsProviderRegistry.instance().styleExists('ogr', '/idont/exist.gpkg', '')
        self.assertFalse(res)
        self.assertTrue(err)
        res, err = QgsProviderRegistry.instance().styleExists('ogr', '/idont/exist.gpkg', 'a style')
        self.assertFalse(res)
        self.assertTrue(err)

        related_count, idlist, namelist, desclist, errmsg = vl.listStylesInDatabase()
        self.assertEqual(related_count, -1)
        self.assertEqual(idlist, [])
        self.assertEqual(namelist, [])
        self.assertEqual(desclist, [])
        self.assertTrue(errmsg)

        qml, errmsg = vl.getStyleFromDatabase("1")
        self.assertFalse(qml)
        self.assertTrue(errmsg)

        qml, success = vl.loadNamedStyle('/idont/exist.gpkg')
        self.assertFalse(success)

        errorMsg = vl.saveStyleToDatabase("name", "description", False, "")
        self.assertTrue(errorMsg)

        # Now with valid URI
        tmpfile = os.path.join(self.basetestpath, 'testStyle.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbMultiPolygon)
        lyr.CreateField(ogr.FieldDefn('foo', ogr.OFTString))
        f = ogr.Feature(lyr.GetLayerDefn())
        f['foo'] = 'bar'
        lyr.CreateFeature(f)
        f = None
        lyr = ds.CreateLayer('test2', geom_type=ogr.wkbMultiPolygon)
        lyr.CreateField(ogr.FieldDefn('foo', ogr.OFTString))
        f = ogr.Feature(lyr.GetLayerDefn())
        f['foo'] = 'bar'
        lyr.CreateFeature(f)
        f = None
        ds = None

        vl = QgsVectorLayer('{}|layername=test'.format(tmpfile), 'test', 'ogr')
        self.assertTrue(vl.isValid())

        vl2 = QgsVectorLayer('{}|layername=test2'.format(tmpfile), 'test2', 'ogr')
        self.assertTrue(vl2.isValid())

        self.assertTrue(vl.dataProvider().isSaveAndLoadStyleToDatabaseSupported())

        # style tables don't exist yet
        res, err = QgsProviderRegistry.instance().styleExists('ogr', vl.source(), '')
        self.assertFalse(res)
        self.assertFalse(err)
        res, err = QgsProviderRegistry.instance().styleExists('ogr', vl2.source(), 'a style')
        self.assertFalse(res)
        self.assertFalse(err)

        related_count, idlist, namelist, desclist, errmsg = vl.listStylesInDatabase()
        self.assertEqual(related_count, 0)
        self.assertEqual(idlist, [])
        self.assertEqual(namelist, [])
        self.assertEqual(desclist, [])
        self.assertTrue(errmsg)

        qml, errmsg = vl.getStyleFromDatabase("not_existing")
        self.assertFalse(qml)
        self.assertTrue(errmsg)

        qml, success = vl.loadNamedStyle('{}|layerid=0'.format(tmpfile))
        self.assertFalse(success)

        errorMsg = vl.saveStyleToDatabase("name", "description", False, "")
        self.assertFalse(errorMsg)

        res, err = QgsProviderRegistry.instance().styleExists('ogr', vl.source(), '')
        self.assertFalse(res)
        self.assertFalse(err)
        res, err = QgsProviderRegistry.instance().styleExists('ogr', vl.source(), 'a style')
        self.assertFalse(res)
        self.assertFalse(err)
        res, err = QgsProviderRegistry.instance().styleExists('ogr', vl.source(), 'name')
        self.assertTrue(res)
        self.assertFalse(err)

        qml, errmsg = vl.getStyleFromDatabase("not_existing")
        self.assertFalse(qml)
        self.assertTrue(errmsg)

        related_count, idlist, namelist, desclist, errmsg = vl.listStylesInDatabase()
        self.assertEqual(related_count, 1)
        self.assertFalse(errmsg)
        self.assertEqual(idlist, ['1'])
        self.assertEqual(namelist, ['name'])
        self.assertEqual(desclist, ['description'])

        qml, errmsg = vl.getStyleFromDatabase("100")
        self.assertFalse(qml)
        self.assertTrue(errmsg)

        qml, errmsg = vl.getStyleFromDatabase("1")
        self.assertTrue(qml.startswith('<!DOCTYPE qgis'), qml)
        self.assertFalse(errmsg)

        # Try overwriting an existing style
        errorMsg = vl.saveStyleToDatabase("name", "description_bis", False, "")
        self.assertFalse(errorMsg)

        res, err = QgsProviderRegistry.instance().styleExists('ogr', vl.source(), 'name')
        self.assertTrue(res)
        self.assertFalse(err)

        related_count, idlist, namelist, desclist, errmsg = vl.listStylesInDatabase()
        self.assertEqual(related_count, 1)
        self.assertFalse(errmsg)
        self.assertEqual(idlist, ['1'])
        self.assertEqual(namelist, ['name'])
        self.assertEqual(desclist, ['description_bis'])

        errorMsg = vl2.saveStyleToDatabase("name_test2", "description_test2", True, "")
        self.assertFalse(errorMsg)

        res, err = QgsProviderRegistry.instance().styleExists('ogr', vl2.source(), 'name_test2')
        self.assertTrue(res)
        self.assertFalse(err)

        errorMsg = vl.saveStyleToDatabase("name2", "description2", True, "")
        self.assertFalse(errorMsg)

        res, err = QgsProviderRegistry.instance().styleExists('ogr', vl.source(), 'name2')
        self.assertTrue(res)
        self.assertFalse(err)

        errorMsg = vl.saveStyleToDatabase("name3", "description3", True, "")
        self.assertFalse(errorMsg)

        res, err = QgsProviderRegistry.instance().styleExists('ogr', vl.source(), 'name3')
        self.assertTrue(res)
        self.assertFalse(err)

        related_count, idlist, namelist, desclist, errmsg = vl.listStylesInDatabase()
        self.assertEqual(related_count, 3)
        self.assertFalse(errmsg)
        self.assertEqual(idlist, ['1', '3', '4', '2'])
        self.assertEqual(namelist, ['name', 'name2', 'name3', 'name_test2'])
        self.assertEqual(desclist, ['description_bis', 'description2', 'description3', 'description_test2'])

        # Check that layers_style table is not list in subLayers()
        vl = QgsVectorLayer(tmpfile, 'test', 'ogr')
        sublayers = vl.dataProvider().subLayers()
        self.assertEqual(2, vl.dataProvider().subLayerCount())
        self.assertEqual(len(sublayers), 2, sublayers)

    @staticmethod
    def _getJournalMode(filename):
        ds = ogr.Open(filename)
        lyr = ds.ExecuteSQL('PRAGMA journal_mode')
        f = lyr.GetNextFeature()
        res = f.GetField(0)
        ds.ReleaseResultSet(lyr)
        ds = None
        return res

    def testDisablewalForSqlite3(self):
        ''' Test disabling walForSqlite3 setting '''
        QgsSettings().setValue("/qgis/walForSqlite3", False)

        tmpfile = os.path.join(self.basetestpath, 'testDisablewalForSqlite3.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbPoint)
        lyr.CreateField(ogr.FieldDefn('attr0', ogr.OFTInteger))
        lyr.CreateField(ogr.FieldDefn('attr1', ogr.OFTInteger))
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetGeometry(ogr.CreateGeometryFromWkt('POINT(0 0)'))
        lyr.CreateFeature(f)
        f = None
        ds = None

        vl = QgsVectorLayer(u'{}'.format(tmpfile), u'test', u'ogr')

        # Test that we are using default delete mode and not WAL
        self.assertEqual(TestPyQgsOGRProviderGpkg._getJournalMode(tmpfile), 'delete')

        self.assertTrue(vl.startEditing())
        feature = next(vl.getFeatures())
        self.assertTrue(vl.changeAttributeValue(feature.id(), 1, 1001))

        # Commit changes
        cbk = ErrorReceiver()
        vl.dataProvider().raiseError.connect(cbk.receiveError)
        self.assertTrue(vl.commitChanges())
        self.assertIsNone(cbk.msg)
        vl = None

        QgsSettings().setValue("/qgis/walForSqlite3", None)

    @unittest.skipIf(int(gdal.VersionInfo('VERSION_NUM')) < GDAL_COMPUTE_VERSION(3, 4, 2), "GDAL 3.4.2 required")
    def testNolock(self):
        """ Test that with GDAL >= 3.4.2 opening a GPKG file doesn't turn on WAL journal_mode """

        srcpath = os.path.join(TEST_DATA_DIR, 'provider')
        srcfile = os.path.join(srcpath, 'geopackage.gpkg')

        last_modified = QFileInfo(srcfile).lastModified()
        vl = QgsVectorLayer(u'{}'.format(srcfile) + "|layername=geopackage", u'test', u'ogr')
        self.assertEqual(TestPyQgsOGRProviderGpkg._getJournalMode(srcfile), 'delete')
        del vl
        self.assertEqual(last_modified, QFileInfo(srcfile).lastModified())

        shutil.copy(os.path.join(srcpath, 'geopackage.gpkg'), self.basetestpath)
        tmpfile = os.path.join(self.basetestpath, 'geopackage.gpkg')

        vl = QgsVectorLayer(u'{}'.format(tmpfile) + "|layername=geopackage", u'test', u'ogr')

        self.assertEqual(TestPyQgsOGRProviderGpkg._getJournalMode(tmpfile), 'delete')

        vl.startEditing()
        self.assertEqual(TestPyQgsOGRProviderGpkg._getJournalMode(tmpfile), 'wal')
        vl.commitChanges()

        self.assertEqual(TestPyQgsOGRProviderGpkg._getJournalMode(tmpfile), 'delete')

    def testSimulatedDBManagerImport(self):
        uri = 'point?field=f1:int'
        uri += '&field=f2:double(6,4)'
        uri += '&field=f3:string(20)'
        mem_lyr = QgsVectorLayer(uri, "x", "memory")
        self.assertTrue(mem_lyr.isValid())
        f = QgsFeature(mem_lyr.fields())
        f['f1'] = 1
        f['f2'] = 123.456
        f['f3'] = '12345678.90123456789'
        f2 = QgsFeature(mem_lyr.fields())
        f2['f1'] = 2
        mem_lyr.dataProvider().addFeatures([f, f2])

        # Test creating new DB
        tmpfile = os.path.join(self.basetestpath, 'testSimulatedDBManagerImport.gpkg')
        options = {}
        options['driverName'] = 'GPKG'
        err = QgsVectorLayerExporter.exportLayer(mem_lyr, tmpfile, "ogr", mem_lyr.crs(), False, options)
        self.assertEqual(err[0], QgsVectorLayerExporter.NoError,
                         'unexpected import error {0}'.format(err))
        lyr = QgsVectorLayer(tmpfile, "y", "ogr")
        self.assertTrue(lyr.isValid())
        features = lyr.getFeatures()
        f = next(features)
        self.assertEqual(f['f1'], 1)
        self.assertEqual(f['f2'], 123.456)
        self.assertEqual(f['f3'], '12345678.90123456789')
        f = next(features)
        self.assertEqual(f['f1'], 2)
        features = None
        del lyr

        # Test updating existing DB, by adding a new layer
        mem_lyr = QgsVectorLayer(uri, "x", "memory")
        self.assertTrue(mem_lyr.isValid())
        f = QgsFeature(mem_lyr.fields())
        f['f1'] = 1
        f['f2'] = 2
        mem_lyr.dataProvider().addFeatures([f])

        options = {}
        options['update'] = True
        options['driverName'] = 'GPKG'
        options['layerName'] = 'my_out_table'
        err = QgsVectorLayerExporter.exportLayer(mem_lyr, tmpfile, "ogr", mem_lyr.crs(), False, options)
        self.assertEqual(err[0], QgsVectorLayerExporter.NoError,
                         'unexpected import error {0}'.format(err))
        lyr = QgsVectorLayer(tmpfile + "|layername=my_out_table", "y", "ogr")
        self.assertTrue(lyr.isValid())
        features = lyr.getFeatures()
        f = next(features)
        self.assertEqual(f['f1'], 1)
        self.assertEqual(f['f2'], 2)
        features = None
        del lyr

        # Test overwriting without overwrite option
        err = QgsVectorLayerExporter.exportLayer(mem_lyr, tmpfile, "ogr", mem_lyr.crs(), False, options)
        self.assertEqual(err[0], QgsVectorLayerExporter.ErrCreateDataSource)

        # Test overwriting, without specifying a layer name
        mem_lyr = QgsVectorLayer(uri, "x", "memory")
        self.assertTrue(mem_lyr.isValid())
        f = QgsFeature(mem_lyr.fields())
        f['f1'] = 3
        f['f2'] = 4
        mem_lyr.dataProvider().addFeatures([f])

        options = {}
        options['driverName'] = 'GPKG'
        options['overwrite'] = True
        err = QgsVectorLayerExporter.exportLayer(mem_lyr, tmpfile, "ogr", mem_lyr.crs(), False, options)
        self.assertEqual(err[0], QgsVectorLayerExporter.NoError,
                         'unexpected import error {0}'.format(err))
        lyr = QgsVectorLayer(tmpfile, "y", "ogr")
        self.assertTrue(lyr.isValid())
        features = lyr.getFeatures()
        f = next(features)
        self.assertEqual(f['f1'], 3)
        self.assertEqual(f['f2'], 4)
        features = None

    def testExportLayerToExistingDatabase(self):
        fields = QgsFields()
        fields.append(QgsField('f1', QVariant.Int))
        tmpfile = os.path.join(self.basetestpath, 'testCreateNewGeopackage.gpkg')
        options = {}
        options['update'] = True
        options['driverName'] = 'GPKG'
        options['layerName'] = 'table1'
        exporter = QgsVectorLayerExporter(tmpfile, "ogr", fields, QgsWkbTypes.Polygon,
                                          QgsCoordinateReferenceSystem('EPSG:3111'), False, options)
        self.assertFalse(exporter.errorCode(),
                         'unexpected export error {}: {}'.format(exporter.errorCode(), exporter.errorMessage()))
        del exporter
        options['layerName'] = 'table2'
        exporter = QgsVectorLayerExporter(tmpfile, "ogr", fields, QgsWkbTypes.Point, QgsCoordinateReferenceSystem('EPSG:3113'),
                                          False, options)
        self.assertFalse(exporter.errorCode(),
                         'unexpected export error {} : {}'.format(exporter.errorCode(), exporter.errorMessage()))
        del exporter
        # make sure layers exist
        lyr = QgsVectorLayer('{}|layername=table1'.format(tmpfile), "lyr1", "ogr")
        self.assertTrue(lyr.isValid())
        self.assertEqual(lyr.crs().authid(), 'EPSG:3111')
        self.assertEqual(lyr.wkbType(), QgsWkbTypes.Polygon)
        lyr2 = QgsVectorLayer('{}|layername=table2'.format(tmpfile), "lyr2", "ogr")
        self.assertTrue(lyr2.isValid())
        self.assertEqual(lyr2.crs().authid(), 'EPSG:3113')
        self.assertEqual(lyr2.wkbType(), QgsWkbTypes.Point)

    def testGeopackageTwoLayerEdition(self):
        ''' test https://github.com/qgis/QGIS/issues/24933 '''
        tmpfile = os.path.join(self.basetestpath, 'testGeopackageTwoLayerEdition.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('layer1', geom_type=ogr.wkbPoint)
        lyr.CreateField(ogr.FieldDefn('attr', ogr.OFTInteger))
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetGeometry(ogr.CreateGeometryFromWkt('POINT(0 0)'))
        lyr.CreateFeature(f)
        f = None
        lyr = ds.CreateLayer('layer2', geom_type=ogr.wkbPoint)
        lyr.CreateField(ogr.FieldDefn('attr', ogr.OFTInteger))
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetGeometry(ogr.CreateGeometryFromWkt('POINT(1 1)'))
        lyr.CreateFeature(f)
        f = None
        ds = None

        vl1 = QgsVectorLayer(u'{}'.format(tmpfile) + "|layername=layer1", u'layer1', u'ogr')
        vl2 = QgsVectorLayer(u'{}'.format(tmpfile) + "|layername=layer2", u'layer2', u'ogr')

        # Edit vl1, vl2 multiple times
        self.assertTrue(vl1.startEditing())
        self.assertTrue(vl2.startEditing())
        self.assertTrue(vl1.changeGeometry(1, QgsGeometry.fromWkt('Point (2 2)')))
        self.assertTrue(vl2.changeGeometry(1, QgsGeometry.fromWkt('Point (3 3)')))
        self.assertTrue(vl1.commitChanges())
        self.assertTrue(vl2.commitChanges())

        self.assertTrue(vl1.startEditing())
        self.assertTrue(vl2.startEditing())
        self.assertTrue(vl1.changeAttributeValue(1, 1, 100))
        self.assertTrue(vl2.changeAttributeValue(1, 1, 101))
        self.assertTrue(vl1.commitChanges())
        self.assertTrue(vl2.commitChanges())

        self.assertTrue(vl1.startEditing())
        self.assertTrue(vl2.startEditing())
        self.assertTrue(vl1.changeGeometry(1, QgsGeometry.fromWkt('Point (4 4)')))
        self.assertTrue(vl2.changeGeometry(1, QgsGeometry.fromWkt('Point (5 5)')))
        self.assertTrue(vl1.commitChanges())
        self.assertTrue(vl2.commitChanges())

        vl1 = None
        vl2 = None

        # Check everything is as expected after re-opening
        vl1 = QgsVectorLayer(u'{}'.format(tmpfile) + "|layername=layer1", u'layer1', u'ogr')
        vl2 = QgsVectorLayer(u'{}'.format(tmpfile) + "|layername=layer2", u'layer2', u'ogr')

        got = [feat for feat in vl1.getFeatures()][0]
        got_geom = got.geometry()
        self.assertEqual(got['attr'], 100)
        reference = QgsGeometry.fromWkt('Point (4 4)')
        self.assertEqual(got_geom.asWkb(), reference.asWkb(),
                         'Expected {}, got {}'.format(reference.asWkt(), got_geom.asWkt()))

        got = [feat for feat in vl2.getFeatures()][0]
        got_geom = got.geometry()
        self.assertEqual(got['attr'], 101)
        reference = QgsGeometry.fromWkt('Point (5 5)')
        self.assertEqual(got_geom.asWkb(), reference.asWkb(),
                         'Expected {}, got {}'.format(reference.asWkt(), got_geom.asWkt()))

    def testReplaceLayerWhileOpen(self):
        ''' Replace an existing geopackage layer whilst it's open in the project'''
        tmpfile = os.path.join(self.basetestpath, 'testGeopackageReplaceOpenLayer.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('layer1', geom_type=ogr.wkbPoint)
        lyr.CreateField(ogr.FieldDefn('attr', ogr.OFTInteger))
        lyr.CreateField(ogr.FieldDefn('attr2', ogr.OFTInteger))
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetGeometry(ogr.CreateGeometryFromWkt('POINT(0 0)'))
        lyr.CreateFeature(f)
        f = None

        vl1 = QgsVectorLayer(u'{}'.format(tmpfile) + "|layername=layer1", u'layer1', u'ogr')
        p = QgsProject()
        p.addMapLayer(vl1)
        request = QgsFeatureRequest().setSubsetOfAttributes([0])
        features = [f for f in vl1.getFeatures(request)]
        self.assertEqual(len(features), 1)

        # now, overwrite the layer with a different geometry type and fields
        ds.DeleteLayer('layer1')
        lyr = ds.CreateLayer('layer1', geom_type=ogr.wkbLineString)
        lyr.CreateField(ogr.FieldDefn('attr', ogr.OFTString))
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetGeometry(ogr.CreateGeometryFromWkt('LineString(0 0, 1 1)'))
        lyr.CreateFeature(f)
        f = None
        vl2 = QgsVectorLayer(u'{}'.format(tmpfile) + "|layername=layer1", u'layer2', u'ogr')
        p.addMapLayer(vl2)

        features = [f for f in vl1.getFeatures(request)]
        self.assertEqual(len(features), 1)

    def testPkAttributeIndexes(self):
        ''' Test the primary key index '''
        tmpfile = os.path.join(self.basetestpath, 'testPkAttributeIndexes.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        ds.CreateLayer('test', geom_type=ogr.wkbPoint,
                       options=['COLUMN_TYPES=foo=int8,bar=string', 'GEOMETRY_NAME=the_geom', 'FID=customfid'])
        ds = None
        vl = QgsVectorLayer('{}|layerid=0'.format(tmpfile), 'test', 'ogr')
        pks = vl.primaryKeyAttributes()
        fields = vl.fields()
        pkfield = fields.at(pks[0])
        self.assertEqual(len(pks), 1)
        self.assertEqual(pks[0], 0)
        self.assertEqual(pkfield.name(), 'customfid')
        self.assertTrue(pkfield.constraints().constraints() & QgsFieldConstraints.ConstraintUnique)

    def testSublayerWithComplexLayerName(self):
        ''' Test reading a gpkg with a sublayer name containing : '''
        tmpfile = os.path.join(self.basetestpath, 'testGeopackageComplexLayerName.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('layer1:', geom_type=ogr.wkbPoint, options=['GEOMETRY_NAME=geom:'])
        lyr.CreateField(ogr.FieldDefn('attr', ogr.OFTInteger))
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetGeometry(ogr.CreateGeometryFromWkt('POINT(0 0)'))
        lyr.CreateFeature(f)
        f = None

        vl = QgsVectorLayer(u'{}'.format(tmpfile), u'layer', u'ogr')
        self.assertEqual(1, vl.dataProvider().subLayerCount())
        self.assertEqual(vl.dataProvider().subLayers(),
                         [QgsDataProvider.SUBLAYER_SEPARATOR.join(['0', 'layer1:', '1', 'Point', 'geom:', ''])])

    def testGeopackageManyLayers(self):
        ''' test opening more than 64 layers without running out of Spatialite connections '''

        tmpfile = os.path.join(self.basetestpath, 'testGeopackageManyLayers.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        for i in range(70):
            lyr = ds.CreateLayer('layer%d' % i, geom_type=ogr.wkbPoint)
            f = ogr.Feature(lyr.GetLayerDefn())
            f.SetGeometry(ogr.CreateGeometryFromWkt('POINT(%d 0)' % i))
            lyr.CreateFeature(f)
            f = None
        ds = None

        vl_tab = []
        for i in range(70):
            layername = 'layer%d' % i
            vl = QgsVectorLayer(u'{}'.format(tmpfile) + "|layername=" + layername, layername, u'ogr')
            self.assertTrue(vl.isValid())
            vl_tab += [vl]

        count = count_opened_filedescriptors(tmpfile)
        if count > 0:
            # We should have just 1 but for obscure reasons
            # uniqueFields() (sometimes?) leaves one behind
            self.assertIn(count, (1, 2))

        for i in range(70):
            got = [feat for feat in vl.getFeatures()]
            self.assertTrue(len(got) == 1)

        # We shouldn't have more than 2 file handles opened:
        # one shared by the QgsOgrProvider object
        # one shared by the feature iterators
        count = count_opened_filedescriptors(tmpfile)
        if count > 0:
            self.assertEqual(count, 2)

        # Re-open an already opened layers. We should get a new handle
        layername = 'layer%d' % 0
        vl_extra0 = QgsVectorLayer(u'{}'.format(tmpfile) + "|layername=" + layername, layername, u'ogr')
        self.assertTrue(vl_extra0.isValid())
        countNew = count_opened_filedescriptors(tmpfile)
        if countNew > 0:
            self.assertLessEqual(countNew, 4)  # for some reason we get 4 and not 3

        layername = 'layer%d' % 1
        vl_extra1 = QgsVectorLayer(u'{}'.format(tmpfile) + "|layername=" + layername, layername, u'ogr')
        self.assertTrue(vl_extra1.isValid())
        countNew2 = count_opened_filedescriptors(tmpfile)
        self.assertEqual(countNew2, countNew)

    def testGeopackageRefreshIfTableListUpdated(self):
        ''' test that creating/deleting a layer is reflected when opening a new layer '''

        tmpfile = os.path.join(self.basetestpath, 'testGeopackageRefreshIfTableListUpdated.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        ds.CreateLayer('test', geom_type=ogr.wkbPoint)
        ds = None

        vl = QgsVectorLayer(u'{}'.format(tmpfile) + "|layername=" + "test", 'test', u'ogr')

        self.assertTrue(vl.extent().isNull())

        time.sleep(1)  # so timestamp gets updated
        ds = ogr.Open(tmpfile, update=1)
        ds.CreateLayer('test2', geom_type=ogr.wkbPoint)
        ds = None

        vl2 = QgsVectorLayer(u'{}'.format(tmpfile), 'test', u'ogr')
        self.assertEqual(2, vl2.dataProvider().subLayerCount())
        vl2.subLayers()
        self.assertEqual(vl2.dataProvider().subLayers(),
                         [QgsDataProvider.SUBLAYER_SEPARATOR.join(['0', 'test', '0', 'Point', 'geom', '']),
                          QgsDataProvider.SUBLAYER_SEPARATOR.join(['1', 'test2', '0', 'Point', 'geom', ''])])

    def testGeopackageLargeFID(self):

        tmpfile = os.path.join(self.basetestpath, 'testGeopackageLargeFID.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbPoint)
        lyr.CreateField(ogr.FieldDefn('str_field', ogr.OFTString))
        ds = None

        vl = QgsVectorLayer(u'{}'.format(tmpfile) + "|layername=" + "test", 'test', u'ogr')
        f = QgsFeature()
        f.setAttributes([1234567890123, None])
        f2 = QgsFeature()
        f2.setAttributes([1234567890124, None])
        self.assertTrue(vl.startEditing())
        self.assertTrue(vl.dataProvider().addFeatures([f, f2]))
        self.assertTrue(vl.commitChanges())

        got = [feat for feat in vl.getFeatures(QgsFeatureRequest(1234567890123))][0]
        self.assertEqual(got['fid'], 1234567890123)

        self.assertTrue(vl.startEditing())
        self.assertTrue(vl.changeGeometry(1234567890123, QgsGeometry.fromWkt('Point (3 50)')))
        self.assertTrue(vl.changeAttributeValue(1234567890123, 1, 'foo'))
        self.assertTrue(vl.commitChanges())

        got = [feat for feat in vl.getFeatures(QgsFeatureRequest(1234567890123))][0]
        self.assertEqual(got['str_field'], 'foo')
        got_geom = got.geometry()
        self.assertIsNotNone(got_geom)

        # We don't change the FID, so OK
        self.assertTrue(vl.startEditing())
        self.assertTrue(vl.dataProvider().changeAttributeValues({1234567890123: {0: 1234567890123, 1: 'bar'},
                                                                 1234567890124: {0: 1234567890124, 1: 'bar2'}}))
        self.assertTrue(vl.commitChanges())

        got = [feat for feat in vl.getFeatures(QgsFeatureRequest(1234567890123))][0]
        self.assertEqual(got['str_field'], 'bar')

        got = [feat for feat in vl.getFeatures(QgsFeatureRequest(1234567890124))][0]
        self.assertEqual(got['str_field'], 'bar2')

        # We try to change the FID, not allowed
        # also check that all changes where reverted
        self.assertTrue(vl.startEditing())
        self.assertFalse(vl.dataProvider().changeAttributeValues({1234567890123: {0: 1, 1: 'baz'},
                                                                  1234567890124: {1: 'baz2'}}))
        self.assertTrue(vl.commitChanges())

        got = [feat for feat in vl.getFeatures(QgsFeatureRequest(1234567890123))][0]
        self.assertEqual(got['str_field'], 'bar')

        got = [feat for feat in vl.getFeatures(QgsFeatureRequest(1234567890124))][0]
        self.assertEqual(got['str_field'], 'bar2')

        self.assertTrue(vl.startEditing())
        self.assertTrue(vl.deleteFeature(1234567890123))
        self.assertTrue(vl.commitChanges())

    def test_AddFeatureNullFid(self):
        """Test gpkg feature with NULL fid can be added"""
        tmpfile = os.path.join(self.basetestpath, 'testGeopackageSplitFeatures.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbPolygon)
        lyr.CreateField(ogr.FieldDefn('str_field', ogr.OFTString))
        ds = None

        layer = QgsVectorLayer(u'{}'.format(tmpfile) + "|layername=" + "test", 'test', u'ogr')

        # Check that pk field has unique constraint
        fields = layer.fields()
        pkfield = fields.at(0)
        self.assertTrue(pkfield.constraints().constraints() & QgsFieldConstraints.ConstraintUnique)

        # Test add feature with default Fid (NULL)
        layer.startEditing()
        f = QgsFeature()
        feat = QgsFeature(layer.fields())
        feat.setGeometry(QgsGeometry.fromWkt('Polygon ((0 0, 0 1, 1 1, 1 0, 0 0))'))
        feat.setAttribute(1, 'test_value')
        layer.addFeature(feat)
        self.assertTrue(layer.commitChanges())
        self.assertEqual(layer.featureCount(), 1)

    def test_SplitFeature(self):
        """Test gpkg feature can be split"""
        tmpfile = os.path.join(self.basetestpath, 'testGeopackageSplitFeatures.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbPolygon)
        lyr.CreateField(ogr.FieldDefn('str_field', ogr.OFTString))
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetGeometry(ogr.CreateGeometryFromWkt('POLYGON ((0 0,0 1,1 1,1 0,0 0))'))
        lyr.CreateFeature(f)
        f = None
        ds = None

        # Split features
        layer = QgsVectorLayer(u'{}'.format(tmpfile) + "|layername=" + "test", 'test', u'ogr')
        self.assertTrue(layer.isValid())
        self.assertTrue(layer.isSpatial())
        self.assertEqual([f for f in layer.getFeatures()][0].geometry().asWkt(), 'Polygon ((0 0, 0 1, 1 1, 1 0, 0 0))')
        layer.startEditing()
        self.assertEqual(layer.splitFeatures([QgsPointXY(0.5, 0), QgsPointXY(0.5, 1)], 0), 0)
        self.assertTrue(layer.commitChanges())
        self.assertEqual(layer.featureCount(), 2)

        layer = QgsVectorLayer(u'{}'.format(tmpfile) + "|layername=" + "test", 'test', u'ogr')
        self.assertEqual(layer.featureCount(), 2)
        g, g2 = [f.geometry() for f in layer.getFeatures()]
        g.normalize()
        g2.normalize()
        self.assertCountEqual([geom.asWkt() for geom in [g, g2]], ['Polygon ((0 0, 0 1, 0.5 1, 0.5 0, 0 0))',
                                                                   'Polygon ((0.5 0, 0.5 1, 1 1, 1 0, 0.5 0))'])

    def test_SplitFeatureErrorIncompatibleGeometryType(self):
        """Test we behave correctly when split feature is not possible due to incompatible geometry type"""
        tmpfile = os.path.join(self.basetestpath, 'test_SplitFeatureErrorIncompatibleGeometryType.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbPoint)
        f = ogr.Feature(lyr.GetLayerDefn())
        # For the purpose of this test, we insert a Polygon in a Point layer
        # which is normally not allowed
        f.SetGeometry(ogr.CreateGeometryFromWkt('POLYGON ((0 0,0 1,1 1,1 0,0 0))'))
        gdal.PushErrorHandler('CPLQuietErrorHandler')
        self.assertEqual(lyr.CreateFeature(f), ogr.OGRERR_NONE)
        gdal.PopErrorHandler()
        f = None
        ds = None

        # Split features
        layer = QgsVectorLayer(u'{}'.format(tmpfile) + "|layername=" + "test", 'test', u'ogr')
        self.assertTrue(layer.isValid())
        self.assertTrue(layer.isSpatial())
        self.assertEqual([f for f in layer.getFeatures()][0].geometry().asWkt(), 'Polygon ((0 0, 0 1, 1 1, 1 0, 0 0))')
        layer.startEditing()
        self.assertEqual(layer.splitFeatures([QgsPointXY(0.5, 0), QgsPointXY(0.5, 1)], 0), 0)
        self.assertFalse(layer.commitChanges())

        layer = QgsVectorLayer(u'{}'.format(tmpfile) + "|layername=" + "test", 'test', u'ogr')
        self.assertEqual(layer.featureCount(), 1)
        g = [f.geometry() for f in layer.getFeatures()][0]
        self.assertEqual(g.asWkt(), 'Polygon ((0 0, 0 1, 1 1, 1 0, 0 0))')

    def test_SplitFeatureErrorIncompatibleGeometryType2(self):
        """Test we behave correctly when split a single-part multipolygon of a polygon layer (https://github.com/qgis/QGIS/issues/41283)"""
        # This is really a non-nominal case. Failing properly would also be understandable.
        tmpfile = os.path.join(self.basetestpath, 'test_SplitFeatureErrorIncompatibleGeometryType2.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbPolygon)
        f = ogr.Feature(lyr.GetLayerDefn())
        # For the purpose of this test, we insert a MultiPolygon in a Polygon layer
        # which is normally not allowed
        f.SetGeometry(ogr.CreateGeometryFromWkt('MULTIPOLYGON (((0 0,0 1,1 1,1 0,0 0)))'))
        gdal.PushErrorHandler('CPLQuietErrorHandler')
        self.assertEqual(lyr.CreateFeature(f), ogr.OGRERR_NONE)
        gdal.PopErrorHandler()
        f = None
        ds = None

        # Split features
        layer = QgsVectorLayer(u'{}'.format(tmpfile) + "|layername=" + "test", 'test', u'ogr')
        self.assertTrue(layer.isValid())
        self.assertTrue(layer.isSpatial())
        self.assertEqual([f for f in layer.getFeatures()][0].geometry().asWkt(), 'MultiPolygon (((0 0, 0 1, 1 1, 1 0, 0 0)))')
        layer.startEditing()
        self.assertEqual(layer.splitFeatures([QgsPointXY(0.5, 0), QgsPointXY(0.5, 1)], 0), 0)
        self.assertTrue(layer.commitChanges())
        self.assertEqual(layer.featureCount(), 2)

        layer = QgsVectorLayer(u'{}'.format(tmpfile) + "|layername=" + "test", 'test', u'ogr')
        self.assertEqual(layer.featureCount(), 2)
        g, g2 = [f.geometry() for f in layer.getFeatures()]
        g.normalize()
        g2.normalize()
        self.assertCountEqual([geom.asWkt() for geom in [g, g2]], ['Polygon ((0 0, 0 1, 0.5 1, 0.5 0, 0 0))',
                                                                   'Polygon ((0.5 0, 0.5 1, 1 1, 1 0, 0.5 0))'])

    def testCreateAttributeIndex(self):
        tmpfile = os.path.join(self.basetestpath, 'testGeopackageAttributeIndex.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbPolygon)
        lyr.CreateField(ogr.FieldDefn('str_field', ogr.OFTString))
        lyr.CreateField(ogr.FieldDefn('str_field2', ogr.OFTString))
        f = None
        ds = None

        vl = QgsVectorLayer(u'{}'.format(tmpfile) + "|layername=" + "test", 'test', u'ogr')
        self.assertTrue(vl.isValid())
        self.assertTrue(vl.dataProvider().capabilities() & QgsVectorDataProvider.CreateAttributeIndex)
        self.assertFalse(vl.dataProvider().createAttributeIndex(-1))
        self.assertFalse(vl.dataProvider().createAttributeIndex(100))

        # should not be allowed - there's already a index on the primary key
        self.assertFalse(vl.dataProvider().createAttributeIndex(0))

        self.assertTrue(vl.dataProvider().createAttributeIndex(1))

        con = spatialite_connect(tmpfile, isolation_level=None)
        cur = con.cursor()
        rs = cur.execute("SELECT * FROM sqlite_master WHERE type='index' AND tbl_name='test'")
        res = [row for row in rs]
        self.assertEqual(len(res), 1)
        index_name = res[0][1]
        rs = cur.execute("PRAGMA index_info({})".format(index_name))
        res = [row for row in rs]
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0][2], 'str_field')

        # second index
        self.assertTrue(vl.dataProvider().createAttributeIndex(2))
        rs = cur.execute("SELECT * FROM sqlite_master WHERE type='index' AND tbl_name='test'")
        res = [row for row in rs]
        self.assertEqual(len(res), 2)
        indexed_columns = []
        for row in res:
            index_name = row[1]
            rs = cur.execute("PRAGMA index_info({})".format(index_name))
            res = [row for row in rs]
            self.assertEqual(len(res), 1)
            indexed_columns.append(res[0][2])

        self.assertCountEqual(indexed_columns, ['str_field', 'str_field2'])
        con.close()

    def testCreateSpatialIndex(self):
        tmpfile = os.path.join(self.basetestpath, 'testGeopackageSpatialIndex.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbPolygon, options=['SPATIAL_INDEX=NO'])
        lyr.CreateField(ogr.FieldDefn('str_field', ogr.OFTString))
        lyr.CreateField(ogr.FieldDefn('str_field2', ogr.OFTString))
        f = None
        ds = None

        vl = QgsVectorLayer(u'{}'.format(tmpfile) + "|layername=" + "test", 'test', u'ogr')
        self.assertTrue(vl.isValid())
        self.assertTrue(vl.dataProvider().capabilities() & QgsVectorDataProvider.CreateSpatialIndex)
        self.assertTrue(vl.dataProvider().createSpatialIndex())

    def testSubSetStringEditable_bug17795_but_with_modified_behavior(self):
        """Test that a layer is editable after setting a subset"""

        tmpfile = os.path.join(self.basetestpath, 'testSubSetStringEditable_bug17795.gpkg')
        shutil.copy(TEST_DATA_DIR + '/' + 'provider/bug_17795.gpkg', tmpfile)

        isEditable = QgsVectorDataProvider.ChangeAttributeValues
        testPath = tmpfile + '|layername=bug_17795'

        vl = QgsVectorLayer(testPath, 'subset_test', 'ogr')
        self.assertTrue(vl.isValid())
        self.assertTrue(vl.dataProvider().capabilities() & isEditable)

        vl = QgsVectorLayer(testPath, 'subset_test', 'ogr')
        vl.setSubsetString('')
        self.assertTrue(vl.isValid())
        self.assertTrue(vl.dataProvider().capabilities() & isEditable)

        vl = QgsVectorLayer(testPath, 'subset_test', 'ogr')
        vl.setSubsetString('"category" = \'one\'')
        self.assertTrue(vl.isValid())
        self.assertTrue(vl.dataProvider().capabilities() & isEditable)

        vl.setSubsetString('')
        self.assertTrue(vl.dataProvider().capabilities() & isEditable)

    def testSubsetStringExtent_bug17863(self):
        """Check that the extent is correct when applied in the ctor and when
        modified after a subset string is set """

        def _lessdigits(s):
            return re.sub(r'(\d+\.\d{3})\d+', r'\1', s)

        tmpfile = os.path.join(self.basetestpath, 'testSubsetStringExtent_bug17863.gpkg')
        shutil.copy(TEST_DATA_DIR + '/' + 'provider/bug_17795.gpkg', tmpfile)

        testPath = tmpfile + '|layername=bug_17795'
        subSetString = '"name" = \'int\''
        subSet = '|subset=%s' % subSetString

        # unfiltered
        vl = QgsVectorLayer(testPath, 'test', 'ogr')
        self.assertTrue(vl.isValid())
        unfiltered_extent = _lessdigits(vl.extent().toString())
        del (vl)

        # filter after construction ...
        subSet_vl2 = QgsVectorLayer(testPath, 'test', 'ogr')
        self.assertEqual(_lessdigits(subSet_vl2.extent().toString()), unfiltered_extent)
        # ... apply filter now!
        subSet_vl2.setSubsetString(subSetString)
        self.assertEqual(subSet_vl2.subsetString(), subSetString)
        self.assertNotEqual(_lessdigits(subSet_vl2.extent().toString()), unfiltered_extent)
        filtered_extent = _lessdigits(subSet_vl2.extent().toString())
        del (subSet_vl2)

        # filtered in constructor
        subSet_vl = QgsVectorLayer(testPath + subSet, 'subset_test', 'ogr')
        self.assertEqual(subSet_vl.subsetString(), subSetString)
        self.assertTrue(subSet_vl.isValid())

        # This was failing in bug 17863
        self.assertEqual(_lessdigits(subSet_vl.extent().toString()), filtered_extent)
        self.assertNotEqual(_lessdigits(subSet_vl.extent().toString()), unfiltered_extent)

    def testRequestWithoutGeometryOnLayerMixedGeometry(self):
        """ Test bugfix for https://github.com/qgis/QGIS/issues/26907 """

        # Issue is more a generic one of the OGR provider, but easy to trigger with GPKG

        tmpfile = os.path.join(self.basetestpath, 'testRequestWithoutGeometryOnLayerMixedGeometry.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbUnknown, options=['SPATIAL_INDEX=NO'])
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetGeometry(ogr.CreateGeometryFromWkt('POINT(0 1)'))
        lyr.CreateFeature(f)
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetGeometry(ogr.CreateGeometryFromWkt('LINESTRING(0 0,1 0)'))
        lyr.CreateFeature(f)
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetGeometry(ogr.CreateGeometryFromWkt('LINESTRING(0 0,1 0)'))
        lyr.CreateFeature(f)
        f = None
        ds = None

        vl = QgsVectorLayer(u'{}'.format(tmpfile) + "|geometrytype=Point|layername=" + "test", 'test', u'ogr')
        self.assertTrue(vl.isValid())
        request = QgsFeatureRequest().setFlags(QgsFeatureRequest.NoGeometry)
        features = [f for f in vl.getFeatures(request)]
        self.assertEqual(len(features), 1)

    def testAddingTwoIntFieldsWithWidth(self):
        """ Test buggfix for https://github.com/qgis/QGIS/issues/26840 """

        tmpfile = os.path.join(self.basetestpath, 'testRequestWithoutGeometryOnLayerMixedGeometry.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbPoint, options=['SPATIAL_INDEX=NO'])
        lyr.CreateField(ogr.FieldDefn('a', ogr.OFTInteger))
        ds = None

        vl = QgsVectorLayer(u'{}'.format(tmpfile) + "|layername=" + "test", 'test', u'ogr')
        self.assertTrue(vl.isValid())

        vl.startEditing()
        self.assertTrue(vl.addAttribute(QgsField("b", QVariant.Int, "integer", 10)))
        self.assertTrue(vl.commitChanges())

        vl.startEditing()
        self.assertTrue(vl.addAttribute(QgsField("c", QVariant.Int, "integer", 10)))
        self.assertTrue(vl.commitChanges())

    def testApproxFeatureCountAndExtent(self):
        """ Test perf improvement for for https://github.com/qgis/QGIS/issues/26292 """

        tmpfile = os.path.join(self.basetestpath, 'testApproxFeatureCountAndExtent.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbPoint)
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetGeometry(ogr.CreateGeometryFromWkt('POINT(0 1)'))
        lyr.CreateFeature(f)
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetGeometry(ogr.CreateGeometryFromWkt('POINT(2 3)'))
        lyr.CreateFeature(f)
        fid = f.GetFID()
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetGeometry(ogr.CreateGeometryFromWkt('POINT(4 5)'))
        lyr.CreateFeature(f)
        lyr.DeleteFeature(fid)
        ds = None
        ds = ogr.Open(tmpfile, update=1)
        ds.ExecuteSQL('DROP TABLE gpkg_ogr_contents')
        ds = None

        os.environ['QGIS_GPKG_FC_THRESHOLD'] = '1'
        vl = QgsVectorLayer(u'{}'.format(tmpfile) + "|layername=" + "test", 'test', u'ogr')
        self.assertTrue(vl.isValid())
        fc = vl.featureCount()
        del os.environ['QGIS_GPKG_FC_THRESHOLD']
        self.assertEqual(fc, 3)  # didn't notice the hole

        reference = QgsGeometry.fromRect(QgsRectangle(0, 1, 4, 5))
        provider_extent = QgsGeometry.fromRect(vl.extent())
        self.assertTrue(QgsGeometry.compare(provider_extent.asPolygon()[0], reference.asPolygon()[0], 0.00001),
                        provider_extent.asPolygon()[0])

    def testRegenerateFid(self):
        """ Test regenerating feature ids """

        fields = QgsFields()
        fields.append(QgsField('fid', QVariant.Int))
        fields.append(QgsField('f1', QVariant.Int))
        tmpfile = os.path.join(self.basetestpath, 'testRegenerateFid.gpkg')
        options = {}
        options['update'] = True
        options['driverName'] = 'GPKG'
        options['layerName'] = 'table1'
        exporter = QgsVectorLayerExporter(tmpfile, "ogr", fields, QgsWkbTypes.Polygon,
                                          QgsCoordinateReferenceSystem('EPSG:3111'), False, options,
                                          QgsFeatureSink.RegeneratePrimaryKey)
        self.assertFalse(exporter.errorCode(),
                         'unexpected export error {}: {}'.format(exporter.errorCode(), exporter.errorMessage()))

        feat = QgsFeature(fields)

        feat['fid'] = 0
        feat['f1'] = 10
        exporter.addFeature(feat)

        feat['fid'] = 0
        feat['f1'] = 20
        exporter.addFeature(feat)

        feat['fid'] = 1
        feat['f1'] = 30
        exporter.addFeature(feat)

        feat['fid'] = 1
        feat['f1'] = 40
        exporter.addFeature(feat)

        del exporter
        # make sure layers exist
        lyr = QgsVectorLayer('{}|layername=table1'.format(tmpfile), "lyr1", "ogr")
        self.assertTrue(lyr.isValid())
        self.assertEqual(lyr.crs().authid(), 'EPSG:3111')
        self.assertEqual(lyr.wkbType(), QgsWkbTypes.Polygon)

        values = set([f['f1'] for f in lyr.getFeatures()])
        self.assertEqual(values, set([10, 20, 30, 40]))

        fids = set([f['fid'] for f in lyr.getFeatures()])
        self.assertEqual(len(fids), 4)

    def testExportWithoutFids(self):
        """ Test export with a feature without fid, regression GH #32927

        This test case is related to testRegenerateFid
        """

        fields = QgsFields()
        fields.append(QgsField('one', QVariant.Int))
        fields.append(QgsField('two', QVariant.Int))
        tmpfile = os.path.join(self.basetestpath, 'testExportWithoutFids.gpkg')
        options = {}
        options['update'] = True
        options['driverName'] = 'GPKG'
        options['layerName'] = 'output'
        exporter = QgsVectorLayerExporter(tmpfile, "ogr", fields, QgsWkbTypes.Point, QgsCoordinateReferenceSystem('EPSG:4326'),
                                          False, options, QgsFeatureSink.RegeneratePrimaryKey)
        self.assertFalse(exporter.errorCode(),
                         'unexpected export error {}: {}'.format(exporter.errorCode(), exporter.errorMessage()))

        feat = QgsFeature(fields)

        feat['one'] = 100
        feat['two'] = 200
        feat.setGeometry(QgsGeometry.fromWkt('point(4 45)'))
        exporter.addFeature(feat)

        del exporter
        # make sure layers exist
        lyr = QgsVectorLayer('{}|layername=output'.format(tmpfile), "lyr1", "ogr")
        self.assertTrue(lyr.isValid())
        self.assertEqual(lyr.crs().authid(), 'EPSG:4326')
        self.assertEqual(lyr.wkbType(), QgsWkbTypes.Point)
        feat_out = next(lyr.getFeatures())
        self.assertEqual(feat_out.attribute('two'), 200)
        self.assertEqual(feat_out.attribute('one'), 100)

    def testTransaction(self):

        tmpfile = os.path.join(self.basetestpath, 'testTransaction.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('lyr1', geom_type=ogr.wkbPoint)
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetGeometry(ogr.CreateGeometryFromWkt('POINT(0 1)'))
        lyr.CreateFeature(f)
        lyr = ds.CreateLayer('lyr2', geom_type=ogr.wkbPoint)
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetGeometry(ogr.CreateGeometryFromWkt('POINT(2 3)'))
        lyr.CreateFeature(f)
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetGeometry(ogr.CreateGeometryFromWkt('POINT(4 5)'))
        lyr.CreateFeature(f)
        ds = None

        vl1 = QgsVectorLayer(u'{}'.format(tmpfile) + "|layername=" + "lyr1", 'test', u'ogr')
        self.assertTrue(vl1.isValid())
        vl2 = QgsVectorLayer(u'{}'.format(tmpfile) + "|layername=" + "lyr2", 'test', u'ogr')
        self.assertTrue(vl2.isValid())

        # prepare a project with transactions enabled
        p = QgsProject()
        p.setTransactionMode(Qgis.TransactionMode.AutomaticGroups)
        p.addMapLayers([vl1, vl2])

        self.assertTrue(vl1.startEditing())
        self.assertIsNotNone(vl1.dataProvider().transaction())
        self.assertTrue(vl1.deleteFeature(1))

        # An iterator opened on the layer should see the feature deleted
        self.assertEqual(len([f for f in vl1.getFeatures(QgsFeatureRequest())]), 0)

        # But not if opened from another connection
        vl1_external = QgsVectorLayer(u'{}'.format(tmpfile) + "|layername=" + "lyr1", 'test', u'ogr')
        self.assertTrue(vl1_external.isValid())
        self.assertEqual(len([f for f in vl1_external.getFeatures(QgsFeatureRequest())]), 1)
        del vl1_external

        self.assertTrue(vl1.commitChanges())

        # Should still get zero features on vl1
        self.assertEqual(len([f for f in vl1.getFeatures(QgsFeatureRequest())]), 0)
        self.assertEqual(len([f for f in vl2.getFeatures(QgsFeatureRequest())]), 2)

        # Test undo/redo
        self.assertTrue(vl2.startEditing())
        self.assertIsNotNone(vl2.dataProvider().transaction())
        self.assertTrue(vl2.editBuffer().deleteFeature(1))
        self.assertEqual(len([f for f in vl2.getFeatures(QgsFeatureRequest())]), 1)
        self.assertTrue(vl2.editBuffer().deleteFeature(2))
        self.assertEqual(len([f for f in vl2.getFeatures(QgsFeatureRequest())]), 0)
        vl2.undoStack().undo()
        self.assertEqual(len([f for f in vl2.getFeatures(QgsFeatureRequest())]), 1)
        vl2.undoStack().undo()
        self.assertEqual(len([f for f in vl2.getFeatures(QgsFeatureRequest())]), 2)
        vl2.undoStack().redo()
        self.assertEqual(len([f for f in vl2.getFeatures(QgsFeatureRequest())]), 1)
        self.assertTrue(vl2.commitChanges())

        self.assertEqual(len([f for f in vl2.getFeatures(QgsFeatureRequest())]), 1)
        del vl1
        del vl2

        vl2_external = QgsVectorLayer(u'{}'.format(tmpfile) + "|layername=" + "lyr2", 'test', u'ogr')
        self.assertTrue(vl2_external.isValid())
        self.assertEqual(len([f for f in vl2_external.getFeatures(QgsFeatureRequest())]), 1)
        del vl2_external

    def testJson(self):
        tmpfile = os.path.join(self.basetestpath, 'test_json.gpkg')
        testdata_path = unitTestDataPath('provider')
        shutil.copy(os.path.join(unitTestDataPath('provider'), 'test_json.gpkg'), tmpfile)

        vl = QgsVectorLayer('{}|layerid=0'.format(tmpfile), 'foo', 'ogr')
        self.assertTrue(vl.isValid())

        fields = vl.dataProvider().fields()
        self.assertEqual(fields.at(fields.indexFromName('json_content')).type(), QVariant.Map)

        fi = vl.getFeatures(QgsFeatureRequest())
        f = QgsFeature()

        # test reading dict value from attribute
        while fi.nextFeature(f):
            if f['fid'] == 1:
                self.assertIsInstance(f['json_content'], dict)
                self.assertEqual(f['json_content'], {'foo': 'bar'})
                # test changing dict value in attribute
                f['json_content'] = {'foo': 'baz'}
                self.assertEqual(f['json_content'], {'foo': 'baz'})
                # test changint dict to list
                f['json_content'] = ['eins', 'zwei', 'drei']
                self.assertEqual(f['json_content'], ['eins', 'zwei', 'drei'])
                # test changing list value in attribute
                f['json_content'] = ['eins', 'zwei', 'drei', 4]
                self.assertEqual(f['json_content'], ['eins', 'zwei', 'drei', 4])
                # test changing to complex json structure
                f['json_content'] = {'name': 'Lily', 'age': '0',
                                     'cars': {'car1': ['fiat tipo', 'fiat punto', 'davoser schlitten'],
                                              'car2': 'bobbycar', 'car3': 'tesla'}}
                self.assertEqual(f['json_content'], {'name': 'Lily', 'age': '0',
                                                     'cars': {'car1': ['fiat tipo', 'fiat punto', 'davoser schlitten'],
                                                              'car2': 'bobbycar', 'car3': 'tesla'}})

        # test adding attribute
        vl.startEditing()
        self.assertTrue(
            vl.addAttribute(QgsField('json_content2', QVariant.Map, "JSON", 60, 0, 'no comment', QVariant.String)))
        self.assertTrue(vl.commitChanges())

        vl.startEditing()
        self.assertTrue(
            vl.addAttribute(QgsField('json_content3', QVariant.Map, "JSON", 60, 0, 'no comment', QVariant.String)))
        self.assertTrue(vl.commitChanges())

        # test setting values to new attributes
        while fi.nextFeature(f):
            if f['fid'] == 2:
                f['json_content'] = {'uno': 'foo'}
                f['json_content2'] = ['uno', 'due', 'tre']
                f['json_content3'] = {'uno': ['uno', 'due', 'tre']}
                self.assertEqual(f['json_content'], {'foo': 'baz'})
                self.assertEqual(f['json_content2'], ['uno', 'due', 'tre'])
                self.assertEqual(f['json_content3'], {'uno': ['uno', 'due', 'tre']})

        # test deleting attribute
        vl.startEditing()
        self.assertTrue(vl.deleteAttribute(vl.fields().indexFromName('json_content3')))
        self.assertTrue(vl.commitChanges())

        # test if index of existent field is not -1 and the one of the deleted is -1
        self.assertNotEqual(vl.fields().indexFromName('json_content2'), -1)
        self.assertEqual(vl.fields().indexFromName('json_content3'), -1)

    def test_quote_identifier(self):
        """Regression #21100"""

        tmpfile = os.path.join(self.basetestpath, 'bug_21100-wierd_field_names.gpkg')  # spellok
        shutil.copy(os.path.join(unitTestDataPath(''), 'bug_21100-wierd_field_names.gpkg'), tmpfile)  # spellok
        vl = QgsVectorLayer('{}|layerid=0'.format(tmpfile), 'foo', 'ogr')
        self.assertTrue(vl.isValid())
        for i in range(1, len(vl.fields())):
            self.assertEqual(vl.uniqueValues(i), {'a', 'b', 'c'})

    def testGeopackageLayerMetadata(self):
        """
        Geopackage layer description and identifier should be read into layer metadata automatically
        """
        tmpfile = os.path.join(self.basetestpath, 'testGeopackageLayerMetadata.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('layer1', geom_type=ogr.wkbPoint)
        lyr.SetMetadataItem('DESCRIPTION', "my desc")
        lyr.SetMetadataItem('IDENTIFIER', "my title")  # see geopackage specs -- "'identifier' is analogous to 'title'"
        lyr.CreateField(ogr.FieldDefn('attr', ogr.OFTInteger))
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetGeometry(ogr.CreateGeometryFromWkt('POINT(0 0)'))
        lyr.CreateFeature(f)
        f = None
        vl1 = QgsVectorLayer(u'{}'.format(tmpfile) + "|layername=" + "layer1", 'test', u'ogr')
        self.assertTrue(vl1.isValid())
        self.assertEqual(vl1.metadata().title(), 'my title')
        self.assertEqual(vl1.metadata().abstract(), 'my desc')

    def testGeopackageSaveMetadata(self):
        tmpfile = os.path.join(self.basetestpath, 'testGeopackageSaveMetadata.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbPolygon)
        lyr.CreateField(ogr.FieldDefn('str_field', ogr.OFTString))
        lyr.CreateField(ogr.FieldDefn('str_field2', ogr.OFTString))
        f = None
        ds = None

        con = spatialite_connect(tmpfile, isolation_level=None)
        cur = con.cursor()
        try:
            rs = cur.execute("SELECT * FROM gpkg_metadata_reference WHERE table_name='test'")
            res = [row for row in rs]
            self.assertEqual(len(res), 0)
            con.close()
        except OperationalError:
            # geopackage_metadata_reference table doesn't exist, that's ok
            pass

        # now save some metadata
        metadata = QgsLayerMetadata()
        metadata.setAbstract('my abstract')
        metadata.setIdentifier('my identifier')
        metadata.setLicenses(['l1', 'l2'])
        ok, err = QgsProviderRegistry.instance().saveLayerMetadata('ogr', QgsProviderRegistry.instance().encodeUri('ogr', {'path': tmpfile, 'layerName': 'test'}), metadata)
        self.assertTrue(ok)

        con = spatialite_connect(tmpfile, isolation_level=None)
        cur = con.cursor()

        # check that main gpkg_contents metadata columns have been updated
        rs = cur.execute("SELECT identifier, description FROM gpkg_contents WHERE table_name='test'")
        rows = [r for r in rs]
        self.assertCountEqual(rows[0], ['my identifier', 'my abstract'])

        rs = cur.execute("SELECT md_file_id FROM gpkg_metadata_reference WHERE table_name='test'")
        file_ids = [row[0] for row in rs]
        self.assertTrue(file_ids)

        rs = cur.execute("SELECT id, md_scope, mime_type, metadata FROM gpkg_metadata WHERE md_standard_uri='http://mrcc.com/qgis.dtd'")
        res = [row for row in rs]
        # id must match md_file_id from gpkg_metadata_reference
        self.assertIn(res[0][0], file_ids)
        self.assertEqual(res[0][1], 'dataset')
        self.assertEqual(res[0][2], 'text/xml')
        metadata_xml = res[0][3]
        con.close()

        metadata2 = QgsLayerMetadata()
        doc = QDomDocument()
        doc.setContent(metadata_xml)
        self.assertTrue(metadata2.readMetadataXml(doc.documentElement()))
        self.assertEqual(metadata2.abstract(), 'my abstract')
        self.assertEqual(metadata2.identifier(), 'my identifier')
        self.assertEqual(metadata2.licenses(), ['l1', 'l2'])

        # try updating existing metadata -- current row must be updated, not a new row added
        metadata2.setAbstract('my abstract 2')
        metadata2.setIdentifier('my identifier 2')
        metadata2.setHistory(['h1', 'h2'])
        ok, err = QgsProviderRegistry.instance().saveLayerMetadata('ogr', QgsProviderRegistry.instance().encodeUri('ogr', {'path': tmpfile, 'layerName': 'test'}), metadata2)
        self.assertTrue(ok)

        con = spatialite_connect(tmpfile, isolation_level=None)
        cur = con.cursor()

        # check that main gpkg_contents metadata columns have been updated
        rs = cur.execute("SELECT identifier, description FROM gpkg_contents WHERE table_name='test'")
        rows = [r for r in rs]
        self.assertEqual(len(rows), 1)
        self.assertCountEqual(rows[0], ['my identifier 2', 'my abstract 2'])

        rs = cur.execute("SELECT md_file_id FROM gpkg_metadata_reference WHERE table_name='test'")
        rows = [r for r in rs]
        file_ids = [row[0] for row in rows]
        self.assertTrue(file_ids)

        rs = cur.execute("SELECT id, md_scope, mime_type, metadata FROM gpkg_metadata WHERE md_standard_uri='http://mrcc.com/qgis.dtd'")
        res = [row for row in rs]
        self.assertEqual(len(res), 1)
        # id must match md_file_id from gpkg_metadata_reference
        self.assertIn(res[0][0], file_ids)
        self.assertEqual(res[0][1], 'dataset')
        self.assertEqual(res[0][2], 'text/xml')
        metadata_xml = res[0][3]
        con.close()

        metadata3 = QgsLayerMetadata()
        doc = QDomDocument()
        doc.setContent(metadata_xml)
        self.assertTrue(metadata3.readMetadataXml(doc.documentElement()))
        self.assertEqual(metadata3.abstract(), 'my abstract 2')
        self.assertEqual(metadata3.identifier(), 'my identifier 2')
        self.assertEqual(metadata3.licenses(), ['l1', 'l2'])
        self.assertEqual(metadata3.history(), ['h1', 'h2'])

    def testGeopackageRestoreMetadata(self):
        """
        Test that metadata saved to gpkg_metadata is automatically restored on layer load
        """
        tmpfile = os.path.join(self.basetestpath, 'testGeopackageRestoreMetadata.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbPolygon)
        lyr.CreateField(ogr.FieldDefn('str_field', ogr.OFTString))
        lyr.CreateField(ogr.FieldDefn('str_field2', ogr.OFTString))
        f = None
        ds = None

        # now save some metadata
        metadata = QgsLayerMetadata()
        metadata.setAbstract('my abstract')
        metadata.setIdentifier('my identifier')
        metadata.setLicenses(['l1', 'l2'])
        ok, err = QgsProviderRegistry.instance().saveLayerMetadata('ogr', QgsProviderRegistry.instance().encodeUri('ogr', {'path': tmpfile, 'layerName': 'test'}), metadata)
        self.assertTrue(ok)

        vl = QgsVectorLayer(tmpfile, 'test')
        self.assertTrue(vl.isValid())
        metadata2 = vl.metadata()
        self.assertEqual(metadata2.abstract(), 'my abstract')
        self.assertEqual(metadata2.identifier(), 'my identifier')
        self.assertEqual(metadata2.licenses(), ['l1', 'l2'])

    def testGeopackageSaveDefaultMetadata(self):
        """
        Test saving layer metadata as default to a gpkg file
        """
        tmpfile = os.path.join(self.basetestpath, 'testGeopackageSaveMetadataDefault.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbPolygon)
        lyr.CreateField(ogr.FieldDefn('str_field', ogr.OFTString))
        lyr.CreateField(ogr.FieldDefn('str_field2', ogr.OFTString))
        f = None
        ds = None

        uri = QgsProviderRegistry.instance().encodeUri('ogr', {'path': tmpfile, 'layerName': 'test'})
        layer = QgsVectorLayer(uri, 'test')
        self.assertTrue(layer.isValid())
        # now save some metadata
        metadata = QgsLayerMetadata()
        metadata.setAbstract('my abstract')
        metadata.setIdentifier('my identifier')
        metadata.setLicenses(['l1', 'l2'])
        layer.setMetadata(metadata)
        # save as default
        msg, res = layer.saveDefaultMetadata()
        self.assertTrue(res)

        # QMD sidecar should NOT exist -- metadata should be written to gpkg_metadata table
        self.assertFalse(os.path.exists(os.path.join(self.basetestpath, 'testGeopackageSaveMetadataDefault.qmd')))

        con = spatialite_connect(tmpfile, isolation_level=None)
        cur = con.cursor()

        # check that main gpkg_contents metadata columns have been updated
        rs = cur.execute("SELECT identifier, description FROM gpkg_contents WHERE table_name='test'")
        rows = [r for r in rs]
        self.assertEqual(len(rows), 1)
        self.assertCountEqual(rows[0], ['my identifier', 'my abstract'])

        rs = cur.execute("SELECT md_file_id FROM gpkg_metadata_reference WHERE table_name='test'")
        rows = [r for r in rs]
        file_ids = [row[0] for row in rows]
        self.assertTrue(file_ids)

        rs = cur.execute("SELECT id, md_scope, mime_type, metadata FROM gpkg_metadata WHERE md_standard_uri='http://mrcc.com/qgis.dtd'")
        res = [row for row in rs]
        self.assertEqual(len(res), 1)
        # id must match md_file_id from gpkg_metadata_reference
        self.assertIn(res[0][0], file_ids)
        self.assertEqual(res[0][1], 'dataset')
        self.assertEqual(res[0][2], 'text/xml')
        con.close()

        # reload layer and check that metadata was restored
        layer2 = QgsVectorLayer(uri, 'test')
        self.assertTrue(layer2.isValid())
        self.assertEqual(layer2.metadata().abstract(), 'my abstract')
        self.assertEqual(layer2.metadata().identifier(), 'my identifier')
        self.assertEqual(layer2.metadata().licenses(), ['l1', 'l2'])

    def testUniqueValuesOnFidColumn(self):
        """Test regression #21311 OGR provider returns an empty set for GPKG uniqueValues"""

        tmpfile = os.path.join(self.basetestpath, 'testGeopackageUniqueValuesOnFidColumn.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbPolygon)
        lyr.CreateField(ogr.FieldDefn('str_field', ogr.OFTString))
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetGeometry(ogr.CreateGeometryFromWkt('POLYGON ((0 0,0 1,1 1,1 0,0 0))'))
        f.SetField('str_field', 'one')
        lyr.CreateFeature(f)
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetGeometry(ogr.CreateGeometryFromWkt('POLYGON ((0 0,0 2,2 2,2 0,0 0))'))
        f.SetField('str_field', 'two')
        lyr.CreateFeature(f)
        f = None
        ds = None
        vl1 = QgsVectorLayer('{}'.format(tmpfile) + "|layername=" + "test", 'test', 'ogr')
        self.assertTrue(vl1.isValid())
        self.assertEqual(vl1.uniqueValues(0), {1, 2})
        self.assertEqual(vl1.uniqueValues(1), {'one', 'two'})

    def testForeignKeyViolation(self):
        """Test that we can open a dataset with a foreign key violation"""

        tmpfile = os.path.join(self.basetestpath, 'testForeignKeyViolation.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbPoint)
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetGeometry(ogr.CreateGeometryFromWkt('POINT(0 1)'))
        lyr.CreateFeature(f)
        ds.ExecuteSQL("PRAGMA foreign_keys = OFF")
        ds.ExecuteSQL("CREATE TABLE foo(id INTEGER)")
        ds.ExecuteSQL(
            "CREATE TABLE bar(fkey INTEGER, CONSTRAINT fkey_constraint FOREIGN KEY (fkey) REFERENCES foo(id))")
        ds.ExecuteSQL("INSERT INTO bar VALUES (1)")
        ds = None
        vl = QgsVectorLayer('{}'.format(tmpfile) + "|layername=" + "test", 'test', 'ogr')
        self.assertTrue(vl.isValid())
        fids = set([f['fid'] for f in vl.getFeatures()])
        self.assertEqual(len(fids), 1)

    def testForeignKeyViolationAfterOpening(self):
        """Test that foreign keys are enforced"""

        tmpfile = os.path.join(self.basetestpath, 'testForeignKeyViolationAfterOpening.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbPoint)
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetGeometry(ogr.CreateGeometryFromWkt('POINT(0 1)'))
        lyr.CreateFeature(f)
        ds.ExecuteSQL(
            "CREATE TABLE bar(fid INTEGER PRIMARY KEY, fkey INTEGER, CONSTRAINT fkey_constraint FOREIGN KEY (fkey) REFERENCES test(fid))")
        ds = None
        vl = QgsVectorLayer('{}'.format(tmpfile) + "|layername=bar", 'test', 'ogr')
        self.assertTrue(vl.isValid())

        # OK
        f = QgsFeature()
        f.setAttributes([None, 1])
        self.assertTrue(vl.dataProvider().addFeature(f))

        # violates foreign key
        f = QgsFeature()
        f.setAttributes([None, 10])
        self.assertFalse(vl.dataProvider().addFeature(f))

    def testExportMultiFromShp(self):
        """Test if a Point is imported as single geom and MultiPoint as multi"""

        single_tmpfile = os.path.join(self.basetestpath, 'testExportMultiFromShp_point.shp')
        ds = ogr.GetDriverByName('ESRI Shapefile').CreateDataSource(single_tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbPoint)
        lyr.CreateField(ogr.FieldDefn('str_field', ogr.OFTString))
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetGeometry(ogr.CreateGeometryFromWkt('POINT (0 0)'))
        f.SetField('str_field', 'one')
        lyr.CreateFeature(f)
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetGeometry(ogr.CreateGeometryFromWkt('POINT (1 1)'))
        f.SetField('str_field', 'two')
        lyr.CreateFeature(f)
        f = None
        ds = None

        multi_tmpfile = os.path.join(self.basetestpath, 'testExportMultiFromShp_multipoint.shp')
        ds = ogr.GetDriverByName('ESRI Shapefile').CreateDataSource(multi_tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbMultiPoint)
        lyr.CreateField(ogr.FieldDefn('str_field', ogr.OFTString))
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetGeometry(ogr.CreateGeometryFromWkt('MULTIPOINT ((0 0))'))
        f.SetField('str_field', 'one')
        lyr.CreateFeature(f)
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetGeometry(ogr.CreateGeometryFromWkt('MULTIPOINT ((1 1), (2 2))'))
        f.SetField('str_field', 'two')
        lyr.CreateFeature(f)
        f = None
        ds = None

        tmpfile = os.path.join(self.basetestpath, 'testExportMultiFromShpMulti.gpkg')
        options = {}
        options['driverName'] = 'GPKG'
        lyr = QgsVectorLayer(multi_tmpfile, 'y', 'ogr')
        self.assertTrue(lyr.isValid())
        self.assertEqual(lyr.featureCount(), 2)
        err, _ = QgsVectorLayerExporter.exportLayer(lyr, tmpfile, "ogr", lyr.crs(), False, options)
        self.assertEqual(err, 0)
        lyr = QgsVectorLayer(tmpfile, "y", "ogr")
        self.assertTrue(lyr.isValid())
        self.assertEqual(lyr.wkbType(), QgsWkbTypes.MultiPoint)
        features = lyr.getFeatures()
        f = next(features)
        self.assertEqual(f.geometry().asWkt().upper(), 'MULTIPOINT ((0 0))')
        f = next(features)
        self.assertEqual(f.geometry().asWkt().upper(), 'MULTIPOINT ((1 1),(2 2))')

        tmpfile = os.path.join(self.basetestpath, 'testExportMultiFromShpSingle.gpkg')
        options = {}
        options['driverName'] = 'GPKG'
        lyr = QgsVectorLayer(single_tmpfile, 'y', 'ogr')
        self.assertTrue(lyr.isValid())
        self.assertEqual(lyr.featureCount(), 2)
        err, _ = QgsVectorLayerExporter.exportLayer(lyr, tmpfile, "ogr", lyr.crs(), False, options)
        self.assertEqual(err, 0)
        lyr = QgsVectorLayer(tmpfile, "y", "ogr")
        self.assertTrue(lyr.isValid())
        self.assertEqual(lyr.wkbType(), QgsWkbTypes.Point)
        features = lyr.getFeatures()
        f = next(features)
        self.assertEqual(f.geometry().asWkt().upper(), 'POINT (0 0)')
        f = next(features)
        self.assertEqual(f.geometry().asWkt().upper(), 'POINT (1 1)')

    def testMinMaxDateField(self):
        """
        Test that provider min/max calls work with date fields
        :return:
        """
        tmpfile = os.path.join(self.basetestpath, 'test_min_max_date_field.gpkg')
        shutil.copy(TEST_DATA_DIR + '/' + 'qgis_server/test_project_api_timefilters.gpkg', tmpfile)

        vl = QgsVectorLayer(tmpfile, 'subset_test', 'ogr')
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.fields().at(2).type(), QVariant.Date)
        self.assertEqual(vl.fields().at(3).type(), QVariant.DateTime)
        self.assertEqual(vl.dataProvider().minimumValue(2), QDate(2010, 1, 1))
        self.assertEqual(vl.dataProvider().maximumValue(2), QDate(2019, 1, 1))
        self.assertEqual(vl.dataProvider().minimumValue(3), QDateTime(2010, 1, 1, 1, 1, 1, 0))
        self.assertEqual(vl.dataProvider().maximumValue(3), QDateTime(2022, 1, 1, 1, 1, 1, 0))
        self.assertEqual(vl.dataProvider().uniqueValues(2),
                         {QDate(2017, 1, 1), NULL, QDate(2018, 1, 1), QDate(2019, 1, 1), QDate(2010, 1, 1)})
        self.assertEqual(vl.dataProvider().uniqueValues(3),
                         {QDateTime(2022, 1, 1, 1, 1, 1), NULL, QDateTime(2019, 1, 1, 1, 1, 1),
                          QDateTime(2021, 1, 1, 1, 1, 1), QDateTime(2010, 1, 1, 1, 1, 1)})

    def testExporterWithFIDColumn(self):
        """Test issue GH #34333, a memory layer with FID is not exported correctly to GPKG"""

        vl = QgsVectorLayer(
            'Point?crs=epsg:4326&field=FID:integer(0)&field=name:string(20)',
            'test',
            'memory')

        self.assertTrue(vl.isValid(), 'Provider not initialized')

        ft = QgsFeature(vl.fields())
        ft.setAttributes([123, 'text1'])
        ft.setGeometry(QgsGeometry.fromWkt('Point(2 49)'))
        myResult, myFeatures = vl.dataProvider().addFeatures([ft])
        self.assertTrue(myResult)
        self.assertTrue(myFeatures)

        dest_file_name = tempfile.mktemp('.gpkg')
        err = QgsVectorLayerExporter.exportLayer(vl, dest_file_name, "ogr", vl.crs(), False)
        self.assertEqual(err[0], QgsVectorLayerExporter.NoError,
                         'unexpected import error {0}'.format(err))

        # Open result and check
        created_layer = QgsVectorLayer(dest_file_name, 'test', 'ogr')
        self.assertTrue(created_layer.isValid())
        f = next(created_layer.getFeatures())
        self.assertEqual(f.geometry().asWkt(), 'Point (2 49)')
        self.assertEqual(f.attributes(), [123, 'text1'])
        self.assertEqual(f.id(), 123)

    def testTransactionGroup(self):
        """Test issue GH #36525"""

        project = QgsProject()
        project.setTransactionMode(Qgis.TransactionMode.AutomaticGroups)
        tmpfile1 = os.path.join(self.basetestpath, 'tempGeoPackageTransactionGroup1.gpkg')
        tmpfile2 = os.path.join(self.basetestpath, 'tempGeoPackageTransactionGroup2.gpkg')
        for tmpfile in (tmpfile1, tmpfile2):
            ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
            for i in range(2):
                lyr = ds.CreateLayer('test%s' % i, geom_type=ogr.wkbPoint)
                lyr.CreateField(ogr.FieldDefn('str_field', ogr.OFTString))
                f = ogr.Feature(lyr.GetLayerDefn())
                f.SetGeometry(ogr.CreateGeometryFromWkt('POINT (1 1)'))
                f.SetField('str_field', 'one')
                lyr.CreateFeature(f)

        vl1_1 = QgsVectorLayer(tmpfile1, 'test1_1', 'ogr')
        self.assertTrue(vl1_1.isValid())
        vl1_2 = QgsVectorLayer(tmpfile1, 'test1_2', 'ogr')
        self.assertTrue(vl1_2.isValid())
        vl2_1 = QgsVectorLayer(tmpfile2, 'test2_1', 'ogr')
        self.assertTrue(vl2_1.isValid())
        vl2_2 = QgsVectorLayer(tmpfile2, 'test2_2', 'ogr')
        self.assertTrue(vl2_2.isValid())
        project.addMapLayers([vl1_1, vl1_2, vl2_1, vl2_2])

        self.assertTrue(vl1_1.startEditing())
        self.assertTrue(vl1_2.isEditable())
        self.assertFalse(vl2_1.isEditable())
        self.assertFalse(vl2_2.isEditable())

        self.assertTrue(vl1_1.rollBack())
        self.assertFalse(vl1_1.isEditable())
        self.assertFalse(vl1_2.isEditable())

        self.assertTrue(vl2_1.startEditing())
        self.assertTrue(vl2_2.isEditable())
        self.assertFalse(vl1_1.isEditable())
        self.assertFalse(vl1_2.isEditable())

    def testTransactionGroupIterator(self):
        """Test issue GH #39178: the bug is that this test hangs
        forever in an endless loop"""

        project = QgsProject()
        project.setTransactionMode(Qgis.TransactionMode.AutomaticGroups)
        tmpfile = os.path.join(
            self.basetestpath, 'tempGeoPackageTransactionGroupIterator.gpkg')
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
        project.addMapLayers([vl])

        self.assertTrue(vl.startEditing())

        for f in vl.getFeatures():
            self.assertTrue(vl.changeAttributeValue(1, 1, 'new value'))

        # Test that QGIS sees the new changes
        self.assertEqual(next(vl.getFeatures()).attribute(1), 'new value')

    def testTransactionGroupCrash(self):
        """Test issue GH #39265 segfault"""

        project = QgsProject()
        project.setTransactionMode(Qgis.TransactionMode.AutomaticGroups)
        tmpfile = os.path.join(
            self.basetestpath, 'tempGeoPackageTransactionCrash.gpkg')
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

        project.addMapLayers([vl])

        feature = next(vl.getFeatures())
        feature.setAttributes([None, 'two'])

        self.assertTrue(vl.startEditing())
        self.assertTrue(vl.addFeature(feature))

        # Save without leaving editing
        self.assertTrue(vl.commitChanges(False))

        # Now add another one
        feature.setAttributes([None, 'three'])
        self.assertTrue(vl.addFeature(feature))

    def _testVectorLayerExporterDeferredSpatialIndex(self, layerOptions, expectSpatialIndex):
        """ Internal method """

        tmpfile = os.path.join(
            self.basetestpath, 'testVectorLayerExporterDeferredSpatialIndex.gpkg')
        gdal.Unlink(tmpfile)
        options = {}
        options['driverName'] = 'GPKG'
        options['layerName'] = 'table1'
        if layerOptions:
            options['layerOptions'] = layerOptions
        exporter = QgsVectorLayerExporter(tmpfile, "ogr", QgsFields(), QgsWkbTypes.Polygon,
                                          QgsCoordinateReferenceSystem('EPSG:3111'), False, options)
        self.assertFalse(exporter.errorCode(),
                         'unexpected export error {}: {}'.format(exporter.errorCode(), exporter.errorMessage()))

        # Check that at that point the rtree is *not* created
        ds = ogr.Open(tmpfile)
        sql_lyr = ds.ExecuteSQL("SELECT 1 FROM sqlite_master WHERE type = 'table' AND name = 'gpkg_extensions'")
        assert sql_lyr.GetNextFeature() is None
        ds.ReleaseResultSet(sql_lyr)
        del ds

        del exporter

        ds = gdal.OpenEx(tmpfile, gdal.OF_VECTOR)
        if expectSpatialIndex:
            # Check that at that point the rtree is created
            sql_lyr = ds.ExecuteSQL("SELECT 1 FROM sqlite_master WHERE type = 'table' AND name = 'gpkg_extensions'")
            assert sql_lyr.GetNextFeature() is not None
            ds.ReleaseResultSet(sql_lyr)
            sql_lyr = ds.ExecuteSQL("SELECT 1 FROM gpkg_extensions WHERE table_name = 'table1' AND extension_name = 'gpkg_rtree_index'")
            assert sql_lyr.GetNextFeature() is not None
            ds.ReleaseResultSet(sql_lyr)
        else:
            # Check that at that point the rtree is *still not* created
            sql_lyr = ds.ExecuteSQL("SELECT 1 FROM sqlite_master WHERE type = 'table' AND name = 'gpkg_extensions'")
            assert sql_lyr.GetNextFeature() is None
            ds.ReleaseResultSet(sql_lyr)
        return ds

    def testVectorLayerExporterDeferredSpatialIndexNoLayerOptions(self):
        """ Check that a deferred spatial index is created when no layer creation options is provided """

        ds = self._testVectorLayerExporterDeferredSpatialIndex(None, True)
        filename = ds.GetDescription()
        del ds
        gdal.Unlink(filename)

    def testVectorLayerExporterDeferredSpatialIndexLayerOptions(self):
        """ Check that a deferred spatial index is created when other layer creations options is provided """

        ds = self._testVectorLayerExporterDeferredSpatialIndex(['GEOMETRY_NAME=my_geom'], True)
        lyr = ds.GetLayer(0)
        self.assertEqual(lyr.GetGeometryColumn(), 'my_geom')
        filename = ds.GetDescription()
        del ds
        gdal.Unlink(filename)

    def testVectorLayerExporterDeferredSpatialIndexExplicitSpatialIndexAsked(self):
        """ Check that a deferred spatial index is created when explicit asked """

        ds = self._testVectorLayerExporterDeferredSpatialIndex(['SPATIAL_INDEX=YES'], True)
        filename = ds.GetDescription()
        del ds
        gdal.Unlink(filename)

    def testVectorLayerExporterDeferredSpatialIndexSpatialIndexDisallowed(self):
        """ Check that a deferred spatial index is NOT created when explicit disallowed """

        ds = self._testVectorLayerExporterDeferredSpatialIndex(['SPATIAL_INDEX=NO'], False)
        filename = ds.GetDescription()
        del ds
        gdal.Unlink(filename)

    def testRollback(self):
        """ Test that a failed operation is properly rolled back """
        tmpfile = os.path.join(self.basetestpath, 'testRollback.gpkg')

        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbPoint, options=['SPATIAL_INDEX=NO'])
        # Ugly hack to be able to create a column with unique constraint with GDAL < 3.2
        ds.ExecuteSQL('CREATE TABLE test2 ("fid" INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "geom" POINT, v INTEGER, v_unique INTEGER UNIQUE)')
        ds.ExecuteSQL("UPDATE gpkg_contents SET table_name = 'test2'")
        ds.ExecuteSQL("UPDATE gpkg_geometry_columns SET table_name = 'test2'")
        ds.ExecuteSQL('INSERT INTO test2 (fid, geom, v, v_unique) VALUES (1, NULL, -1, 123)')
        ds = None

        vl = QgsVectorLayer(u'{}'.format(tmpfile), 'test', u'ogr')
        self.assertTrue(vl.isValid())

        features = [f for f in vl.getFeatures()]
        self.assertEqual(len(features), 1)
        self.assertEqual(features[0].attributes(), [1, -1, 123])

        f = QgsFeature()
        # violates unique constraint
        f.setAttributes([None, -2, 123])
        f2 = QgsFeature()
        f2.setAttributes([None, -3, 124])
        ret, _ = vl.dataProvider().addFeatures([f, f2])
        self.assertFalse(ret)

        f = QgsFeature()
        f.setAttributes([None, -4, 125])
        ret, _ = vl.dataProvider().addFeatures([f])
        self.assertTrue(ret)

        features = [f for f in vl.getFeatures()]
        self.assertEqual(len(features), 2)
        self.assertEqual(features[0].attributes(), [1, -1, 123])
        self.assertEqual(features[1].attributes(), [2, -4, 125])

    def testFixWrongMetadataReferenceColumnNameUpdate(self):
        """ Test that we (or GDAL) fixes wrong gpkg_metadata_reference_column_name_update trigger """
        tmpfile = os.path.join(self.basetestpath, 'testFixWrongMetadataReferenceColumnNameUpdate.gpkg')

        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        ds.CreateLayer('test', geom_type=ogr.wkbPoint)
        ds.SetMetadata('FOO', 'BAR')
        ds = None

        ds = ogr.Open(tmpfile, update=1)
        gdal.PushErrorHandler()
        ds.ExecuteSQL('DROP TRIGGER gpkg_metadata_reference_column_name_update')
        gdal.PopErrorHandler()
        # inject wrong trigger on purpose
        wrong_trigger = "CREATE TRIGGER 'gpkg_metadata_reference_column_name_update' " + \
                        "BEFORE UPDATE OF column_name ON 'gpkg_metadata_reference' " + \
                        "FOR EACH ROW BEGIN " + \
                        "SELECT RAISE(ABORT, 'update on table gpkg_metadata_reference " + \
                        "violates constraint: column name must be NULL when reference_scope " + \
                        "is \"geopackage\", \"table\" or \"row\"') " + \
                        "WHERE (NEW.reference_scope IN ('geopackage','table','row') " + \
                        "AND NEW.column_nameIS NOT NULL); END;"
        ds.ExecuteSQL(wrong_trigger)
        ds = None

        vl = QgsVectorLayer(u'{}'.format(tmpfile), 'test', u'ogr')
        self.assertTrue(vl.isValid())
        del vl

        # Check trigger afterwards
        ds = ogr.Open(tmpfile)
        sql_lyr = ds.ExecuteSQL(
            "SELECT sql FROM sqlite_master WHERE type = 'trigger' " +
            "AND name = 'gpkg_metadata_reference_column_name_update'")
        f = sql_lyr.GetNextFeature()
        sql = f['sql']
        ds.ReleaseResultSet(sql_lyr)
        ds = None

        gdal.Unlink(tmpfile)
        self.assertNotIn('column_nameIS', sql)

    def testRejectedGeometryUpdate(self):
        """Test that we correctly behave when a geometry update fails"""
        tmpfile = os.path.join(self.basetestpath, 'testRejectedGeometryUpdate.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbUnknown)
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetGeometry(ogr.CreateGeometryFromWkt('POLYGON ((0 0,0 1,1 1,1 0,0 0))'))
        lyr.CreateFeature(f)
        ds.ExecuteSQL("CREATE TRIGGER rejectGeometryUpdate BEFORE UPDATE OF geom ON test FOR EACH ROW BEGIN SELECT RAISE(ABORT, 'update forbidden'); END;")
        f = None
        ds = None

        vl = QgsVectorLayer(u'{}'.format(tmpfile) + "|layername=" + "test", 'test', u'ogr')
        self.assertTrue(vl.isValid())

        self.assertTrue(vl.startEditing())
        self.assertTrue(vl.changeGeometry(1, QgsGeometry.fromWkt('Point (0 0)')))
        self.assertFalse(vl.commitChanges())

        vl = QgsVectorLayer(u'{}'.format(tmpfile) + "|layername=" + "test", 'test', u'ogr')
        self.assertTrue(vl.isValid())

        g = [f.geometry() for f in vl.getFeatures()][0]
        self.assertEqual(g.asWkt(), 'Polygon ((0 0, 0 1, 1 1, 1 0, 0 0))')

    def testSubsetComments(self):
        """Test issue GH #45754"""

        tmp_dir = QTemporaryDir()
        tmpfile = os.path.join(tmp_dir.path(), 'testSubsetComments.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('my--test', geom_type=ogr.wkbPoint)
        lyr.CreateField(ogr.FieldDefn('text_field', ogr.OFTString))
        lyr.CreateField(ogr.FieldDefn('my--thing\'s', ogr.OFTString))
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetGeometry(ogr.CreateGeometryFromWkt('POINT(0 0)'))
        f['text_field'] = 'one'
        f['my--thing\'s'] = 'one'
        lyr.CreateFeature(f)
        f = ogr.Feature(lyr.GetLayerDefn())
        f['text_field'] = 'two'
        f['my--thing\'s'] = 'my "things -- all'
        f.SetGeometry(ogr.CreateGeometryFromWkt('POINT(1 1)'))
        lyr.CreateFeature(f)
        del (lyr)

        def _test(subset_string):
            vl1 = QgsVectorLayer(f'{tmpfile}|subset={subset_string}'.format(tmpfile, subset_string), 'test', 'ogr')
            self.assertTrue(vl1.isValid())
            self.assertEqual(vl1.featureCount(), 1)

        _test('-- comment\n SELECT * --comment\nFROM "my--test" WHERE text_field=\'one\'')
        _test('\n SELECT * --comment\nFROM "my--test" WHERE\n-- comment \ntext_field=\'one\'')
        _test(' SELECT * --comment\nFROM "my--test" WHERE\ntext_field=\'one\' AND \ntext_field != \'--embedded comment\'')
        _test('SELECT * FROM "my--test" WHERE text_field=\'one\' AND text_field != \' \\\'--embedded comment\'')
        _test('select "my--thing\'s" from "my--test" where "my--thing\'s" = \'my "things -- all\'')

    def testIsSqlQuery(self):
        """Test that isQuery returns what it should in case of simple filters"""

        tmp_dir = QTemporaryDir()
        tmpfile = os.path.join(tmp_dir.path(), 'testQueryLayers.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbPoint)
        lyr.CreateField(ogr.FieldDefn('text_field', ogr.OFTString))
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetGeometry(ogr.CreateGeometryFromWkt('POINT(0 0)'))
        f['text_field'] = 'one'
        lyr.CreateFeature(f)
        f = ogr.Feature(lyr.GetLayerDefn())
        f['text_field'] = 'two'
        f.SetGeometry(ogr.CreateGeometryFromWkt('POINT(1 1)'))
        lyr.CreateFeature(f)
        del (lyr)

        vl1 = QgsVectorLayer(f'{tmpfile}|subset=SELECT * FROM test WHERE "text_field"=\'one\''.format(tmpfile), 'test', 'ogr')
        self.assertTrue(vl1.isValid())
        self.assertTrue(vl1.isSqlQuery())
        self.assertEqual(vl1.featureCount(), 1)

        # Test flags
        self.assertTrue(vl1.vectorLayerTypeFlags() & Qgis.VectorLayerTypeFlag.SqlQuery)

        vl2 = QgsVectorLayer(f'{tmpfile}|subset="text_field"=\'one\''.format(tmpfile), 'test', 'ogr')
        self.assertTrue(vl2.isValid())
        self.assertFalse(vl2.isSqlQuery())
        self.assertEqual(vl2.featureCount(), 1)

        # Test flags
        self.assertFalse(vl2.vectorLayerTypeFlags() & Qgis.VectorLayerTypeFlag.SqlQuery)


if __name__ == '__main__':
    unittest.main()
