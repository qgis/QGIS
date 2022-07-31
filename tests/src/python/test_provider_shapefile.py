# -*- coding: utf-8 -*-
"""QGIS Unit tests for the OGR/Shapefile provider.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Matthias Kuhn'
__date__ = '2015-04-23'
__copyright__ = 'Copyright 2015, The QGIS Project'

import os
import re
import tempfile
import shutil
import glob
import osgeo.gdal
import osgeo.ogr
import sys

from osgeo import gdal
from qgis.core import (
    QgsApplication,
    QgsDataProvider,
    QgsSettings,
    QgsFeature,
    QgsField,
    QgsGeometry,
    QgsVectorLayer,
    QgsFeatureRequest,
    QgsProviderRegistry,
    QgsRectangle,
    QgsVectorDataProvider,
    QgsWkbTypes,
    QgsVectorLayerExporter,
    Qgis
)
from qgis.PyQt.QtCore import QVariant
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath
from providertestbase import ProviderTestCase

start_app()
TEST_DATA_DIR = unitTestDataPath()


def GDAL_COMPUTE_VERSION(maj, min, rev):
    return ((maj) * 1000000 + (min) * 10000 + (rev) * 100)


class ErrorReceiver():

    def __init__(self):
        self.msg = None

    def receiveError(self, msg):
        self.msg = msg


class TestPyQgsShapefileProvider(unittest.TestCase, ProviderTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        # Create test layer
        cls.basetestpath = tempfile.mkdtemp()
        cls.repackfilepath = tempfile.mkdtemp()

        srcpath = os.path.join(TEST_DATA_DIR, 'provider')
        for file in glob.glob(os.path.join(srcpath, 'shapefile.*')):
            shutil.copy(os.path.join(srcpath, file), cls.basetestpath)
            shutil.copy(os.path.join(srcpath, file), cls.repackfilepath)
        for file in glob.glob(os.path.join(srcpath, 'shapefile_poly.*')):
            shutil.copy(os.path.join(srcpath, file), cls.basetestpath)
        cls.basetestfile = os.path.join(cls.basetestpath, 'shapefile.shp')
        cls.repackfile = os.path.join(cls.repackfilepath, 'shapefile.shp')
        cls.basetestpolyfile = os.path.join(cls.basetestpath, 'shapefile_poly.shp')
        cls.vl = QgsVectorLayer('{}|layerid=0'.format(cls.basetestfile), 'test', 'ogr')
        assert cls.vl.isValid()
        cls.source = cls.vl.dataProvider()
        cls.vl_poly = QgsVectorLayer('{}|layerid=0'.format(cls.basetestpolyfile), 'test', 'ogr')
        assert cls.vl_poly.isValid()
        cls.poly_provider = cls.vl_poly.dataProvider()

        cls.dirs_to_cleanup = [cls.basetestpath, cls.repackfilepath]

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        del cls.vl
        del cls.vl_poly
        for dirname in cls.dirs_to_cleanup:
            shutil.rmtree(dirname, True)

    def treat_time_as_string(self):
        return True

    def treat_datetime_as_string(self):
        return True

    def getSource(self):
        tmpdir = tempfile.mkdtemp()
        self.dirs_to_cleanup.append(tmpdir)
        srcpath = os.path.join(TEST_DATA_DIR, 'provider')
        for file in glob.glob(os.path.join(srcpath, 'shapefile.*')):
            shutil.copy(os.path.join(srcpath, file), tmpdir)
        datasource = os.path.join(tmpdir, 'shapefile.shp')

        vl = QgsVectorLayer('{}|layerid=0'.format(datasource), 'test', 'ogr')
        return vl

    def getEditableLayer(self):
        return self.getSource()

    def enableCompiler(self):
        QgsSettings().setValue('/qgis/compileExpressions', True)
        return True

    def disableCompiler(self):
        QgsSettings().setValue('/qgis/compileExpressions', False)

    def uncompiledFilters(self):
        filters = set(['name ILIKE \'QGIS\'',
                       '"name" NOT LIKE \'Ap%\'',
                       '"name" NOT ILIKE \'QGIS\'',
                       '"name" NOT ILIKE \'pEAR\'',
                       'name <> \'Apple\'',
                       '"name" <> \'apple\'',
                       '(name = \'Apple\') is not null',
                       'name ILIKE \'aPple\'',
                       'name ILIKE \'%pp%\'',
                       'cnt = 1100 % 1000',
                       '"name" || \' \' || "name" = \'Orange Orange\'',
                       '"name" || \' \' || "cnt" = \'Orange 100\'',
                       '\'x\' || "name" IS NOT NULL',
                       '\'x\' || "name" IS NULL',
                       'cnt = 10 ^ 2',
                       '"name" ~ \'[OP]ra[gne]+\'',
                       'false and NULL',
                       'true and NULL',
                       'NULL and false',
                       'NULL and true',
                       'NULL and NULL',
                       'false or NULL',
                       'true or NULL',
                       'NULL or false',
                       'NULL or true',
                       'NULL or NULL',
                       'not name = \'Apple\'',
                       'not name = \'Apple\' or name = \'Apple\'',
                       'not name = \'Apple\' or not name = \'Apple\'',
                       'not name = \'Apple\' and pk = 4',
                       'not name = \'Apple\' and not pk = 4',
                       'num_char IN (2, 4, 5)',
                       '-cnt > 0',
                       '-cnt < 0',
                       '-cnt - 1 = -101',
                       '-(-cnt) = 100',
                       '-(cnt) = -(100)',
                       'sqrt(pk) >= 2',
                       'radians(cnt) < 2',
                       'degrees(pk) <= 200',
                       'abs(cnt) <= 200',
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
                       'round(3.14) <= pk',
                       'round(0.314,1) * 10 = pk',
                       'floor(3.14) <= pk',
                       'ceil(3.14) <= pk',
                       'pk < pi()',
                       'round(cnt / 66.67) <= 2',
                       'floor(cnt / 66.67) <= 2',
                       'ceil(cnt / 66.67) <= 2',
                       'pk < pi() / 2',
                       'pk = char(51)',
                       'pk = coalesce(NULL,3,4)',
                       'lower(name) = \'apple\'',
                       'upper(name) = \'APPLE\'',
                       'name = trim(\'   Apple   \')',
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
                       '"dt" <= format_date(make_datetime(2020, 5, 4, 12, 13, 14), \'yyyy-MM-dd hh:mm:ss\')',
                       '"dt" < format_date(make_date(2020, 5, 4), \'yyyy-MM-dd hh:mm:ss\')',
                       '"dt" = format_date(to_datetime(\'000www14ww13ww12www4ww5ww2020\',\'zzzwwwsswwmmwwhhwwwdwwMwwyyyy\'),\'yyyy-MM-dd hh:mm:ss\')',
                       """dt BETWEEN format_date(make_datetime(2020, 5, 3, 12, 13, 14),  'yyyy-MM-dd hh:mm:ss') AND format_date(make_datetime(2020, 5, 4, 12, 14, 14), 'yyyy-MM-dd hh:mm:ss')""",
                       """dt NOT BETWEEN format_date(make_datetime(2020, 5, 3, 12, 13, 14), 'yyyy-MM-dd hh:mm:ss') AND format_date(make_datetime(2020, 5, 4, 12, 14, 14), 'yyyy-MM-dd hh:mm:ss')""",
                       '"date" = to_date(\'www4ww5ww2020\',\'wwwdwwMwwyyyy\')',
                       'to_time("time") >= make_time(12, 14, 14)',
                       'to_time("time") = to_time(\'000www14ww13ww12www\',\'zzzwwwsswwmmwwhhwww\')',
                       'to_datetime("dt", \'yyyy-MM-dd hh:mm:ss\') + make_interval(days:=1) <= make_datetime(2020, 5, 4, 12, 13, 14)',
                       'to_datetime("dt", \'yyyy-MM-dd hh:mm:ss\') + make_interval(days:=0.01) <= make_datetime(2020, 5, 4, 12, 13, 14)',
                       'cnt BETWEEN -200 AND 200'  # NoUnaryMinus
                       ])
        return filters

    def partiallyCompiledFilters(self):
        return set(['name = \'Apple\'',
                    'name = \'apple\'',
                    '\"NaMe\" = \'Apple\'',
                    'name LIKE \'Apple\'',
                    'name LIKE \'aPple\'',
                    'name LIKE \'Ap_le\'',
                    'name LIKE \'Ap\\_le\'',
                    '"name"="name2"'])

    def testRepack(self):
        vl = QgsVectorLayer('{}|layerid=0'.format(self.repackfile), 'test', 'ogr')

        ids = [f.id() for f in vl.getFeatures(QgsFeatureRequest().setFilterExpression('pk=1'))]
        vl.selectByIds(ids)
        self.assertEqual(vl.selectedFeatureIds(), ids)
        self.assertEqual(vl.featureCount(), 5)
        self.assertTrue(vl.startEditing())
        self.assertTrue(vl.deleteFeature(3))
        self.assertTrue(vl.commitChanges())
        self.assertTrue(vl.selectedFeatureCount() == 0 or vl.selectedFeatures()[0]['pk'] == 1)

    def testUpdateMode(self):
        """ Test that on-the-fly re-opening in update/read-only mode works """

        tmpdir = tempfile.mkdtemp()
        self.dirs_to_cleanup.append(tmpdir)
        srcpath = os.path.join(TEST_DATA_DIR, 'provider')
        for file in glob.glob(os.path.join(srcpath, 'shapefile.*')):
            shutil.copy(os.path.join(srcpath, file), tmpdir)
        datasource = os.path.join(tmpdir, 'shapefile.shp')

        vl = QgsVectorLayer('{}|layerid=0'.format(datasource), 'test', 'ogr')
        caps = vl.dataProvider().capabilities()
        self.assertTrue(caps & QgsVectorDataProvider.AddFeatures)
        self.assertTrue(caps & QgsVectorDataProvider.DeleteFeatures)
        self.assertTrue(caps & QgsVectorDataProvider.ChangeAttributeValues)
        self.assertTrue(caps & QgsVectorDataProvider.AddAttributes)
        self.assertTrue(caps & QgsVectorDataProvider.DeleteAttributes)
        self.assertTrue(caps & QgsVectorDataProvider.CreateSpatialIndex)
        self.assertTrue(caps & QgsVectorDataProvider.SelectAtId)
        self.assertTrue(caps & QgsVectorDataProvider.ChangeGeometries)
        # self.assertTrue(caps & QgsVectorDataProvider.ChangeFeatures)

        # We should be really opened in read-only mode even if write capabilities are declared
        self.assertEqual(vl.dataProvider().property("_debug_open_mode"), "read-only")

        # Unbalanced call to leaveUpdateMode()
        self.assertFalse(vl.dataProvider().leaveUpdateMode())

        # Test that startEditing() / commitChanges() plays with enterUpdateMode() / leaveUpdateMode()
        self.assertTrue(vl.startEditing())
        self.assertEqual(vl.dataProvider().property("_debug_open_mode"), "read-write")
        self.assertTrue(vl.dataProvider().isValid())

        self.assertTrue(vl.commitChanges())
        self.assertEqual(vl.dataProvider().property("_debug_open_mode"), "read-only")
        self.assertTrue(vl.dataProvider().isValid())

        # Manual enterUpdateMode() / leaveUpdateMode() with 2 depths
        self.assertTrue(vl.dataProvider().enterUpdateMode())
        self.assertEqual(vl.dataProvider().property("_debug_open_mode"), "read-write")
        caps = vl.dataProvider().capabilities()
        self.assertTrue(caps & QgsVectorDataProvider.AddFeatures)

        f = QgsFeature()
        f.setAttributes([200])
        f.setGeometry(QgsGeometry.fromWkt('Point (2 49)'))
        (ret, feature_list) = vl.dataProvider().addFeatures([f])
        self.assertTrue(ret)
        fid = feature_list[0].id()

        features = [f_iter for f_iter in vl.getFeatures(QgsFeatureRequest().setFilterFid(fid))]
        values = [f_iter['pk'] for f_iter in features]
        self.assertEqual(values, [200])

        got_geom = [f_iter.geometry() for f_iter in features][0].constGet()
        self.assertEqual((got_geom.x(), got_geom.y()), (2.0, 49.0))

        self.assertTrue(vl.dataProvider().changeGeometryValues({fid: QgsGeometry.fromWkt('Point (3 50)')}))
        self.assertTrue(vl.dataProvider().changeAttributeValues({fid: {0: 100}}))

        features = [f_iter for f_iter in vl.getFeatures(QgsFeatureRequest().setFilterFid(fid))]
        values = [f_iter['pk'] for f_iter in features]

        got_geom = [f_iter.geometry() for f_iter in features][0].constGet()
        self.assertEqual((got_geom.x(), got_geom.y()), (3.0, 50.0))

        self.assertTrue(vl.dataProvider().deleteFeatures([fid]))

        # Check that it has really disappeared
        osgeo.gdal.PushErrorHandler('CPLQuietErrorHandler')
        features = [f_iter for f_iter in vl.getFeatures(QgsFeatureRequest().setFilterFid(fid))]
        osgeo.gdal.PopErrorHandler()
        self.assertEqual(features, [])

        self.assertTrue(vl.dataProvider().addAttributes([QgsField("new_field", QVariant.Int, "integer")]))
        self.assertTrue(vl.dataProvider().deleteAttributes([len(vl.dataProvider().fields()) - 1]))

        self.assertTrue(vl.startEditing())
        self.assertEqual(vl.dataProvider().property("_debug_open_mode"), "read-write")

        self.assertTrue(vl.commitChanges())
        self.assertEqual(vl.dataProvider().property("_debug_open_mode"), "read-write")

        self.assertTrue(vl.dataProvider().enterUpdateMode())
        self.assertEqual(vl.dataProvider().property("_debug_open_mode"), "read-write")

        self.assertTrue(vl.dataProvider().leaveUpdateMode())
        self.assertEqual(vl.dataProvider().property("_debug_open_mode"), "read-write")

        self.assertTrue(vl.dataProvider().leaveUpdateMode())
        self.assertEqual(vl.dataProvider().property("_debug_open_mode"), "read-only")

        # Test that update mode will be implicitly enabled if doing an action
        # that requires update mode
        (ret, _) = vl.dataProvider().addFeatures([QgsFeature()])
        self.assertTrue(ret)
        self.assertEqual(vl.dataProvider().property("_debug_open_mode"), "read-write")

    def testUpdateModeFailedReopening(self):
        ''' Test that methods on provider don't crash after a failed reopening '''

        # Windows doesn't like removing files opened by OGR, whatever
        # their open mode, so that makes it hard to test
        if sys.platform == 'win32':
            return

        tmpdir = tempfile.mkdtemp()
        self.dirs_to_cleanup.append(tmpdir)
        srcpath = os.path.join(TEST_DATA_DIR, 'provider')
        for file in glob.glob(os.path.join(srcpath, 'shapefile.*')):
            shutil.copy(os.path.join(srcpath, file), tmpdir)
        datasource = os.path.join(tmpdir, 'shapefile.shp')

        vl = QgsVectorLayer('{}|layerid=0'.format(datasource), 'test', 'ogr')

        os.unlink(datasource)

        self.assertFalse(vl.dataProvider().enterUpdateMode())
        self.assertFalse(vl.dataProvider().enterUpdateMode())
        self.assertEqual(vl.dataProvider().property("_debug_open_mode"), "invalid")

        self.assertFalse(vl.dataProvider().isValid())
        self.assertEqual(len([f for f in vl.dataProvider().getFeatures()]), 0)
        self.assertEqual(len(vl.dataProvider().subLayers()), 0)
        self.assertFalse(vl.dataProvider().setSubsetString('TRUE'))
        (ret, _) = vl.dataProvider().addFeatures([QgsFeature()])
        self.assertFalse(ret)
        self.assertFalse(vl.dataProvider().deleteFeatures([1]))
        self.assertFalse(vl.dataProvider().addAttributes([QgsField()]))
        self.assertFalse(vl.dataProvider().deleteAttributes([1]))
        self.assertFalse(vl.dataProvider().changeGeometryValues({0: QgsGeometry.fromWkt('Point (3 50)')}))
        self.assertFalse(vl.dataProvider().changeAttributeValues({0: {0: 0}}))
        self.assertFalse(vl.dataProvider().createSpatialIndex())
        self.assertFalse(vl.dataProvider().createAttributeIndex(0))

    def testreloadData(self):
        ''' Test reloadData() '''

        tmpdir = tempfile.mkdtemp()
        self.dirs_to_cleanup.append(tmpdir)
        srcpath = os.path.join(TEST_DATA_DIR, 'provider')
        for file in glob.glob(os.path.join(srcpath, 'shapefile.*')):
            shutil.copy(os.path.join(srcpath, file), tmpdir)
        datasource = os.path.join(tmpdir, 'shapefile.shp')

        vl1 = QgsVectorLayer('{}|layerid=0'.format(datasource), 'test', 'ogr')
        vl2 = QgsVectorLayer('{}|layerid=0'.format(datasource), 'test', 'ogr')
        self.assertTrue(vl1.startEditing())
        self.assertTrue(vl1.deleteAttributes([1]))
        self.assertTrue(vl1.commitChanges())
        self.assertEqual(len(vl1.fields()) + 1, len(vl2.fields()))
        # Reload
        vl2.reload()
        # And now check that fields are up-to-date
        self.assertEqual(len(vl1.fields()), len(vl2.fields()))

    def testRenameAttributes(self):
        ''' Test renameAttributes() '''

        tmpdir = tempfile.mkdtemp()
        self.dirs_to_cleanup.append(tmpdir)
        srcpath = os.path.join(TEST_DATA_DIR, 'provider')
        for file in glob.glob(os.path.join(srcpath, 'shapefile.*')):
            shutil.copy(os.path.join(srcpath, file), tmpdir)
        datasource = os.path.join(tmpdir, 'shapefile.shp')

        vl = QgsVectorLayer('{}|layerid=0'.format(datasource), 'test', 'ogr')
        provider = vl.dataProvider()

        # bad rename
        self.assertFalse(provider.renameAttributes({-1: 'not_a_field'}))
        self.assertFalse(provider.renameAttributes({100: 'not_a_field'}))
        # already exists
        self.assertFalse(provider.renameAttributes({2: 'cnt'}))

        # rename one field
        self.assertTrue(provider.renameAttributes({2: 'newname'}))
        self.assertEqual(provider.fields().at(2).name(), 'newname')
        vl.updateFields()
        fet = next(vl.getFeatures())
        self.assertEqual(fet.fields()[2].name(), 'newname')

        # rename two fields
        self.assertTrue(provider.renameAttributes({2: 'newname2', 3: 'another'}))
        self.assertEqual(provider.fields().at(2).name(), 'newname2')
        self.assertEqual(provider.fields().at(3).name(), 'another')
        vl.updateFields()
        fet = next(vl.getFeatures())
        self.assertEqual(fet.fields()[2].name(), 'newname2')
        self.assertEqual(fet.fields()[3].name(), 'another')

        # close file and reopen, then recheck to confirm that changes were saved to file
        del vl
        vl = None
        vl = QgsVectorLayer('{}|layerid=0'.format(datasource), 'test', 'ogr')
        provider = vl.dataProvider()
        self.assertEqual(provider.fields().at(2).name(), 'newname2')
        self.assertEqual(provider.fields().at(3).name(), 'another')
        fet = next(vl.getFeatures())
        self.assertEqual(fet.fields()[2].name(), 'newname2')
        self.assertEqual(fet.fields()[3].name(), 'another')

    def testDeleteGeometry(self):
        ''' Test changeGeometryValues() with a null geometry '''

        tmpdir = tempfile.mkdtemp()
        self.dirs_to_cleanup.append(tmpdir)
        srcpath = os.path.join(TEST_DATA_DIR, 'provider')
        for file in glob.glob(os.path.join(srcpath, 'shapefile.*')):
            shutil.copy(os.path.join(srcpath, file), tmpdir)
        datasource = os.path.join(tmpdir, 'shapefile.shp')

        vl = QgsVectorLayer('{}|layerid=0'.format(datasource), 'test', 'ogr')
        self.assertTrue(vl.dataProvider().changeGeometryValues({0: QgsGeometry()}))
        vl = None

        vl = QgsVectorLayer('{}|layerid=0'.format(datasource), 'test', 'ogr')
        fet = next(vl.getFeatures())
        self.assertFalse(fet.hasGeometry())

    def testDeleteShapes(self):
        ''' Test fix for #11007 '''

        tmpdir = tempfile.mkdtemp()
        self.dirs_to_cleanup.append(tmpdir)
        srcpath = os.path.join(TEST_DATA_DIR, 'provider')
        for file in glob.glob(os.path.join(srcpath, 'shapefile.*')):
            shutil.copy(os.path.join(srcpath, file), tmpdir)
        datasource = os.path.join(tmpdir, 'shapefile.shp')

        vl = QgsVectorLayer('{}|layerid=0'.format(datasource), 'test', 'ogr')
        feature_count = vl.featureCount()
        # Start an iterator that will open a new connection
        iterator = vl.getFeatures()
        next(iterator)

        # Delete a feature
        self.assertTrue(vl.startEditing())
        self.assertTrue(vl.deleteFeature(1))
        self.assertTrue(vl.commitChanges())

        # Test the content of the shapefile while it is still opened
        ds = osgeo.ogr.Open(datasource)
        # Test repacking has been done
        self.assertTrue(ds.GetLayer(0).GetFeatureCount() == feature_count - 1)
        ds = None

        # Delete another feature while in update mode
        self.assertTrue(2 == 2)
        vl.dataProvider().enterUpdateMode()
        vl.dataProvider().deleteFeatures([0])

        # Test that repacking has not been done (since in update mode)
        ds = osgeo.ogr.Open(datasource)
        self.assertTrue(ds.GetLayer(0).GetFeatureCount() == feature_count - 1)
        ds = None

        # Test that repacking was performed when leaving updateMode
        vl.dataProvider().leaveUpdateMode()
        ds = osgeo.ogr.Open(datasource)
        self.assertTrue(ds.GetLayer(0).GetFeatureCount() == feature_count - 2)
        ds = None

        vl = None

    def testDontRepackOnReload(self):
        ''' Test fix for #18421 '''

        tmpdir = tempfile.mkdtemp()
        self.dirs_to_cleanup.append(tmpdir)
        srcpath = os.path.join(TEST_DATA_DIR, 'provider')
        for file in glob.glob(os.path.join(srcpath, 'shapefile.*')):
            shutil.copy(os.path.join(srcpath, file), tmpdir)
        datasource = os.path.join(tmpdir, 'shapefile.shp')

        vl = QgsVectorLayer('{}|layerid=0'.format(datasource), 'test', 'ogr')
        feature_count = vl.featureCount()
        # Start an iterator that will open a new connection
        iterator = vl.getFeatures()
        next(iterator)

        # Delete another feature while in update mode
        vl.dataProvider().enterUpdateMode()
        vl.dataProvider().reloadData()
        vl.dataProvider().deleteFeatures([0])

        # Test that repacking has not been done (since in update mode)
        ds = osgeo.ogr.Open(datasource)
        self.assertTrue(ds.GetLayer(0).GetFeatureCount() == feature_count)
        ds = None

        vl = None

    def testRepackUnderFileLocks(self):
        ''' Test fix for #15570 and #15393 '''
        tmpdir = tempfile.mkdtemp()
        self.dirs_to_cleanup.append(tmpdir)
        srcpath = os.path.join(TEST_DATA_DIR, 'provider')
        for file in glob.glob(os.path.join(srcpath, 'shapefile.*')):
            shutil.copy(os.path.join(srcpath, file), tmpdir)
        datasource = os.path.join(tmpdir, 'shapefile.shp')

        vl = QgsVectorLayer('{}|layerid=0'.format(datasource), 'test', 'ogr')
        feature_count = vl.featureCount()

        # Keep a file descriptor opened on the .dbf, .shp and .shx
        f_shp = open(os.path.join(tmpdir, 'shapefile.shp'), 'rb')
        f_shx = open(os.path.join(tmpdir, 'shapefile.shx'), 'rb')
        f_dbf = open(os.path.join(tmpdir, 'shapefile.dbf'), 'rb')

        # Delete a feature
        self.assertTrue(vl.startEditing())
        self.assertTrue(vl.deleteFeature(1))

        # Commit changes and check no error is emitted
        cbk = ErrorReceiver()
        vl.dataProvider().raiseError.connect(cbk.receiveError)
        self.assertTrue(vl.commitChanges())
        self.assertIsNone(cbk.msg)

        vl = None

        del f_shp
        del f_shx
        del f_dbf

        # Test repacking has been done
        ds = osgeo.ogr.Open(datasource)
        self.assertTrue(ds.GetLayer(0).GetFeatureCount(), feature_count - 1)
        ds = None

    def testRepackAtFirstSave(self):
        ''' Test fix for #15407 '''
        tmpdir = tempfile.mkdtemp()
        self.dirs_to_cleanup.append(tmpdir)
        srcpath = os.path.join(TEST_DATA_DIR, 'provider')
        for file in glob.glob(os.path.join(srcpath, 'shapefile.*')):
            shutil.copy(os.path.join(srcpath, file), tmpdir)
        datasource = os.path.join(tmpdir, 'shapefile.shp')

        ds = osgeo.ogr.Open(datasource)
        lyr = ds.GetLayer(0)
        original_feature_count = lyr.GetFeatureCount()
        lyr.DeleteFeature(2)
        ds = None

        vl = QgsVectorLayer('{}|layerid=0'.format(datasource), 'test', 'ogr')

        self.assertTrue(vl.featureCount(), original_feature_count)

        # Edit a feature (attribute change only)
        self.assertTrue(vl.startEditing())
        self.assertTrue(vl.dataProvider().changeAttributeValues({0: {0: 100}}))

        # Commit changes and check no error is emitted
        cbk = ErrorReceiver()
        vl.dataProvider().raiseError.connect(cbk.receiveError)
        self.assertTrue(vl.commitChanges())
        self.assertIsNone(cbk.msg)

        self.assertTrue(vl.featureCount(), original_feature_count - 1)

        vl = None

        # Test repacking has been done
        ds = osgeo.ogr.Open(datasource)
        self.assertTrue(ds.GetLayer(0).GetFeatureCount(), original_feature_count - 1)
        ds = None

    def testOpenWithFilter(self):
        file_path = os.path.join(TEST_DATA_DIR, 'provider', 'shapefile.shp')
        uri = '{}|layerid=0|subset="name" = \'Apple\''.format(file_path)
        options = QgsDataProvider.ProviderOptions()
        # ensure that no longer required ogr SQL layers are correctly cleaned up
        # we need to run this twice for the incorrect cleanup asserts to trip,
        # since they are triggered only when fetching an existing layer from the ogr
        # connection pool
        for i in range(2):
            vl = QgsVectorLayer(uri)
            self.assertTrue(vl.isValid(), 'Layer not valid, iteration {}'.format(i + 1))
            self.assertEqual(vl.featureCount(), 1)
            f = next(vl.getFeatures())
            self.assertEqual(f['name'], 'Apple')
            # force close of data provider
            vl.setDataSource('', 'test', 'ogr', options)

    def testEncoding(self):
        file_path = os.path.join(TEST_DATA_DIR, 'shapefile', 'iso-8859-1.shp')
        vl = QgsVectorLayer(file_path)
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.dataProvider().encoding(), 'ISO-8859-1')
        self.assertEqual(next(vl.getFeatures())[1], 'äöü')

        file_path = os.path.join(TEST_DATA_DIR, 'shapefile', 'iso-8859-1_ldid.shp')
        vl = QgsVectorLayer(file_path)
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.dataProvider().encoding(), 'ISO-8859-1')
        self.assertEqual(next(vl.getFeatures())[1], 'äöü')

        file_path = os.path.join(TEST_DATA_DIR, 'shapefile', 'latin1.shp')
        vl = QgsVectorLayer(file_path)
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.dataProvider().encoding(), 'ISO-8859-1')
        self.assertEqual(next(vl.getFeatures())[1], 'äöü')

        file_path = os.path.join(TEST_DATA_DIR, 'shapefile', 'utf8.shp')
        vl = QgsVectorLayer(file_path)
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.dataProvider().encoding(), 'UTF-8')
        self.assertEqual(next(vl.getFeatures())[1], 'äöü')

        file_path = os.path.join(TEST_DATA_DIR, 'shapefile', 'windows-1252.shp')
        vl = QgsVectorLayer(file_path)
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.dataProvider().encoding(), 'windows-1252')
        self.assertEqual(next(vl.getFeatures())[1], 'äöü')

        file_path = os.path.join(TEST_DATA_DIR, 'shapefile', 'windows-1252_ldid.shp')
        vl = QgsVectorLayer(file_path)
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.dataProvider().encoding(), 'windows-1252')
        self.assertEqual(next(vl.getFeatures())[1], 'äöü')

        if int(gdal.VersionInfo('VERSION_NUM')) >= GDAL_COMPUTE_VERSION(3, 1, 0):
            # correct autodetection of vsizip based shapefiles depends on GDAL 3.1
            file_path = os.path.join(TEST_DATA_DIR, 'shapefile', 'windows-1252.zip')
            vl = QgsVectorLayer('/vsizip/{}'.format(file_path))
            self.assertTrue(vl.isValid())
            self.assertEqual(vl.dataProvider().encoding(), 'windows-1252')
            self.assertEqual(next(vl.getFeatures())[1], 'äöü')

        file_path = os.path.join(TEST_DATA_DIR, 'shapefile', 'system_encoding.shp')
        vl = QgsVectorLayer(file_path)
        self.assertTrue(vl.isValid())
        # no encoding hints, so it should default to UTF-8 (which is wrong for this particular file, but the correct guess to make first!)
        self.assertEqual(vl.dataProvider().encoding(), 'UTF-8')
        self.assertNotEqual(next(vl.getFeatures())[1], 'äöü')
        # set to correct encoding
        vl.dataProvider().setEncoding('ISO-8859-1')
        self.assertEqual(vl.dataProvider().encoding(), 'ISO-8859-1')
        self.assertEqual(next(vl.getFeatures())[1], 'äöü')

    def testCreateAttributeIndex(self):
        tmpdir = tempfile.mkdtemp()
        self.dirs_to_cleanup.append(tmpdir)
        srcpath = os.path.join(TEST_DATA_DIR, 'provider')
        for file in glob.glob(os.path.join(srcpath, 'shapefile.*')):
            shutil.copy(os.path.join(srcpath, file), tmpdir)
        datasource = os.path.join(tmpdir, 'shapefile.shp')

        vl = QgsVectorLayer('{}|layerid=0'.format(datasource), 'test', 'ogr')
        self.assertTrue(vl.isValid())
        self.assertTrue(vl.dataProvider().capabilities() & QgsVectorDataProvider.CreateAttributeIndex)
        self.assertFalse(vl.dataProvider().createAttributeIndex(-1))
        self.assertFalse(vl.dataProvider().createAttributeIndex(100))
        self.assertTrue(vl.dataProvider().createAttributeIndex(1))

    def testCreateSpatialIndex(self):
        tmpdir = tempfile.mkdtemp()
        self.dirs_to_cleanup.append(tmpdir)
        srcpath = os.path.join(TEST_DATA_DIR, 'provider')
        for file in glob.glob(os.path.join(srcpath, 'shapefile.*')):
            shutil.copy(os.path.join(srcpath, file), tmpdir)
        datasource = os.path.join(tmpdir, 'shapefile.shp')

        vl = QgsVectorLayer('{}|layerid=0'.format(datasource), 'test', 'ogr')
        self.assertTrue(vl.isValid())
        self.assertTrue(vl.dataProvider().capabilities() & QgsVectorDataProvider.CreateSpatialIndex)
        self.assertTrue(vl.dataProvider().createSpatialIndex())

    def testSubSetStringEditable_bug17795_but_with_modified_behavior(self):
        """Test that a layer is still editable after setting a subset"""

        testPath = TEST_DATA_DIR + '/' + 'lines.shp'
        isEditable = QgsVectorDataProvider.ChangeAttributeValues

        vl = QgsVectorLayer(testPath, 'subset_test', 'ogr')
        self.assertTrue(vl.isValid())
        self.assertTrue(vl.dataProvider().capabilities() & isEditable)

        vl = QgsVectorLayer(testPath, 'subset_test', 'ogr')
        vl.setSubsetString('')
        self.assertTrue(vl.isValid())
        self.assertTrue(vl.dataProvider().capabilities() & isEditable)

        vl = QgsVectorLayer(testPath, 'subset_test', 'ogr')
        vl.setSubsetString('"Name" = \'Arterial\'')
        self.assertTrue(vl.isValid())
        self.assertTrue(vl.dataProvider().capabilities() & isEditable)

        vl.setSubsetString('')
        self.assertTrue(vl.dataProvider().capabilities() & isEditable)

    def testSubsetStringExtent_bug17863(self):
        """Check that the extent is correct when applied in the ctor and when
        modified after a subset string is set """

        def _lessdigits(s):
            return re.sub(r'(\d+\.\d{3})\d+', r'\1', s)

        testPath = TEST_DATA_DIR + '/' + 'points.shp'
        subSetString = '"Class" = \'Biplane\''
        subSet = '|layerid=0|subset=%s' % subSetString

        # unfiltered
        vl = QgsVectorLayer(testPath, 'test', 'ogr')
        self.assertTrue(vl.isValid())
        unfiltered_extent = _lessdigits(vl.extent().toString())
        del vl

        # filter after construction ...
        subSet_vl2 = QgsVectorLayer(testPath, 'test', 'ogr')
        self.assertEqual(_lessdigits(subSet_vl2.extent().toString()), unfiltered_extent)
        # ... apply filter now!
        subSet_vl2.setSubsetString(subSetString)
        self.assertEqual(subSet_vl2.subsetString(), subSetString)
        self.assertNotEqual(_lessdigits(subSet_vl2.extent().toString()), unfiltered_extent)
        filtered_extent = _lessdigits(subSet_vl2.extent().toString())
        del subSet_vl2

        # filtered in constructor
        subSet_vl = QgsVectorLayer(testPath + subSet, 'subset_test', 'ogr')
        self.assertEqual(subSet_vl.subsetString(), subSetString)
        self.assertTrue(subSet_vl.isValid())

        # This was failing in bug 17863
        self.assertEqual(_lessdigits(subSet_vl.extent().toString()), filtered_extent)
        self.assertNotEqual(_lessdigits(subSet_vl.extent().toString()), unfiltered_extent)

    def testMalformedSubsetStrings(self):
        """Test that invalid where clauses always return false"""

        testPath = TEST_DATA_DIR + '/' + 'lines.shp'

        vl = QgsVectorLayer(testPath, 'subset_test', 'ogr')
        self.assertTrue(vl.isValid())
        self.assertTrue(vl.setSubsetString(''))
        self.assertTrue(vl.setSubsetString('"Name" = \'Arterial\''))
        self.assertTrue(vl.setSubsetString('select * from lines where "Name" = \'Arterial\''))
        self.assertFalse(vl.setSubsetString('this is invalid sql'))
        self.assertFalse(vl.setSubsetString('select * from lines where "NonExistentField" = \'someValue\''))
        self.assertFalse(vl.setSubsetString('select * from lines where "Name" = \'Arte...'))
        self.assertFalse(vl.setSubsetString('select * from lines where "Name" in (\'Arterial\', \'Highway\' '))
        self.assertFalse(vl.setSubsetString('select * from NonExistentTable'))
        self.assertFalse(vl.setSubsetString('select NonExistentField from lines'))
        self.assertFalse(vl.setSubsetString('"NonExistentField" = \'someValue\''))
        self.assertFalse(vl.setSubsetString('"Name" = \'Arte...'))
        self.assertFalse(vl.setSubsetString('"Name" in (\'Arterial\', \'Highway\' '))
        self.assertTrue(vl.setSubsetString(''))

    def testMultipatch(self):
        """Check that we can deal with multipatch shapefiles, returned natively by OGR as GeometryCollection of TIN"""

        testPath = TEST_DATA_DIR + '/' + 'multipatch.shp'
        vl = QgsVectorLayer(testPath, 'test', 'ogr')
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.MultiPolygonZ)
        f = next(vl.getFeatures())
        self.assertEqual(f.geometry().wkbType(), QgsWkbTypes.MultiPolygonZ)
        self.assertEqual(f.geometry().constGet().asWkt(),
                         'MultiPolygonZ (((0 0 0, 0 1 0, 1 1 0, 0 0 0)),((0 0 0, 1 1 0, 1 0 0, 0 0 0)),((0 0 0, 0 -1 0, 1 -1 0, 0 0 0)),((0 0 0, 1 -1 0, 1 0 0, 0 0 0)))')

    def testShzSupport(self):
        ''' Test support for single layer compressed shapefiles (.shz) '''

        if int(osgeo.gdal.VersionInfo('VERSION_NUM')) < GDAL_COMPUTE_VERSION(3, 1, 0):
            return

        tmpfile = os.path.join(self.basetestpath, 'testShzSupport.shz')
        ds = osgeo.ogr.GetDriverByName('ESRI Shapefile').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('testShzSupport', geom_type=osgeo.ogr.wkbPoint)
        lyr.CreateField(osgeo.ogr.FieldDefn('attr', osgeo.ogr.OFTInteger))
        f = osgeo.ogr.Feature(lyr.GetLayerDefn())
        f.SetField('attr', 1)
        f.SetGeometry(osgeo.ogr.CreateGeometryFromWkt('POINT(0 0)'))
        lyr.CreateFeature(f)
        f = None
        ds = None

        vl = QgsVectorLayer(tmpfile, 'test', 'ogr')
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.Point)
        f = next(vl.getFeatures())
        assert f['attr'] == 1
        self.assertEqual(f.geometry().constGet().asWkt(), 'Point (0 0)')

        self.assertTrue(vl.startEditing())
        self.assertTrue(vl.changeAttributeValue(f.id(), 0, -1))
        self.assertTrue(vl.commitChanges())

        f = next(vl.getFeatures())
        assert f['attr'] == -1

        # Check DataItem
        registry = QgsApplication.dataItemProviderRegistry()
        files_provider = next(provider for provider in registry.providers() if provider.name() == 'files')
        item = files_provider.createDataItem(tmpfile, None)
        self.assertTrue(item.uri().endswith('testShzSupport.shz'))

    def testShpZipSupport(self):
        ''' Test support for multi layer compressed shapefiles (.shp.zip) '''

        if int(osgeo.gdal.VersionInfo('VERSION_NUM')) < GDAL_COMPUTE_VERSION(3, 1, 0):
            return

        tmpfile = os.path.join(self.basetestpath, 'testShpZipSupport.shp.zip')
        ds = osgeo.ogr.GetDriverByName('ESRI Shapefile').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('layer1', geom_type=osgeo.ogr.wkbPoint)
        lyr.CreateField(osgeo.ogr.FieldDefn('attr', osgeo.ogr.OFTInteger))
        f = osgeo.ogr.Feature(lyr.GetLayerDefn())
        f.SetField('attr', 1)
        f.SetGeometry(osgeo.ogr.CreateGeometryFromWkt('POINT(0 0)'))
        lyr.CreateFeature(f)
        f = None
        lyr = ds.CreateLayer('layer2', geom_type=osgeo.ogr.wkbMultiLineString)
        lyr.CreateField(osgeo.ogr.FieldDefn('attr', osgeo.ogr.OFTInteger))
        f = osgeo.ogr.Feature(lyr.GetLayerDefn())
        f.SetField('attr', 2)
        f.SetGeometry(osgeo.ogr.CreateGeometryFromWkt('LINESTRING(0 0,1 1)'))
        lyr.CreateFeature(f)
        f = None
        ds = None

        vl1 = QgsVectorLayer(tmpfile + '|layername=layer1', 'test', 'ogr')
        vl2 = QgsVectorLayer(tmpfile + '|layername=layer2', 'test', 'ogr')
        self.assertTrue(vl1.isValid())
        self.assertTrue(vl2.isValid())
        self.assertEqual(vl1.wkbType(), QgsWkbTypes.Point)
        self.assertEqual(vl2.wkbType(), QgsWkbTypes.MultiLineString)
        f1 = next(vl1.getFeatures())
        f2 = next(vl2.getFeatures())
        assert f1['attr'] == 1
        self.assertEqual(f1.geometry().constGet().asWkt(), 'Point (0 0)')
        assert f2['attr'] == 2
        self.assertEqual(f2.geometry().constGet().asWkt(), 'MultiLineString ((0 0, 1 1))')

        self.assertTrue(vl1.startEditing())
        self.assertTrue(vl2.startEditing())
        self.assertTrue(vl1.changeAttributeValue(f1.id(), 0, -1))
        self.assertTrue(vl2.changeAttributeValue(f2.id(), 0, -2))
        self.assertTrue(vl1.commitChanges())
        self.assertTrue(vl2.commitChanges())

        f = next(vl1.getFeatures())
        assert f['attr'] == -1

        f = next(vl2.getFeatures())
        assert f['attr'] == -2

        # Check DataItem
        registry = QgsApplication.dataItemProviderRegistry()
        files_provider = next(provider for provider in registry.providers() if provider.name() == 'files')
        item = files_provider.createDataItem(tmpfile, None)
        children = item.createChildren()
        self.assertEqual(len(children), 2)
        uris = sorted([children[i].uri() for i in range(2)])
        self.assertIn('testShpZipSupport.shp.zip|layername=layer1', uris[0])
        self.assertIn('testShpZipSupport.shp.zip|layername=layer2', uris[1])

    def testWriteShapefileWithSingleConversion(self):
        """Check writing geometries from a POLYGON ESRI shapefile does not
        convert to multi when "forceSinglePartGeometryType" options is TRUE
        also checks failing cases.

        OGR provider always report MULTI for POLYGON and LINESTRING, but if we set
        the import option "forceSinglePartGeometryType" the writer must respect the
        actual single-part type if the features in the data provider are actually single
        and not multi.
        """

        ml = QgsVectorLayer(
            ('Polygon?crs=epsg:4326&field=id:int'),
            'test',
            'memory')

        provider = ml.dataProvider()
        ft = QgsFeature()
        ft.setGeometry(QgsGeometry.fromWkt('Polygon ((0 0, 0 1, 1 1, 1 0, 0 0))'))
        ft.setAttributes([1])
        res, features = provider.addFeatures([ft])

        dest_file_name = os.path.join(self.basetestpath, 'multipart.shp')
        write_result, error_message = QgsVectorLayerExporter.exportLayer(ml,
                                                                         dest_file_name,
                                                                         'ogr',
                                                                         ml.crs(),
                                                                         False,
                                                                         {"driverName": "ESRI Shapefile"}
                                                                         )
        self.assertEqual(write_result, QgsVectorLayerExporter.NoError, error_message)

        # Open the newly created layer
        shapefile_layer = QgsVectorLayer(dest_file_name)

        dest_singlepart_file_name = os.path.join(self.basetestpath, 'singlepart.gpkg')
        write_result, error_message = QgsVectorLayerExporter.exportLayer(shapefile_layer,
                                                                         dest_singlepart_file_name,
                                                                         'ogr',
                                                                         shapefile_layer.crs(),
                                                                         False,
                                                                         {
                                                                             "forceSinglePartGeometryType": True,
                                                                             "driverName": "GPKG",
                                                                         })
        self.assertEqual(write_result, QgsVectorLayerExporter.NoError, error_message)

        # Load result layer and check that it's NOT MULTI
        single_layer = QgsVectorLayer(dest_singlepart_file_name)
        self.assertTrue(single_layer.isValid())
        self.assertTrue(QgsWkbTypes.isSingleType(single_layer.wkbType()))

        # Now save the shapfile layer into a gpkg with no force options
        dest_multipart_file_name = os.path.join(self.basetestpath, 'multipart.gpkg')
        write_result, error_message = QgsVectorLayerExporter.exportLayer(shapefile_layer,
                                                                         dest_multipart_file_name,
                                                                         'ogr',
                                                                         shapefile_layer.crs(),
                                                                         False,
                                                                         {
                                                                             "forceSinglePartGeometryType": False,
                                                                             "driverName": "GPKG",
                                                                         })
        self.assertEqual(write_result, QgsVectorLayerExporter.NoError, error_message)
        # Load result layer and check that it's MULTI
        multi_layer = QgsVectorLayer(dest_multipart_file_name)
        self.assertTrue(multi_layer.isValid())
        self.assertTrue(QgsWkbTypes.isMultiType(multi_layer.wkbType()))

        # Failing case: add a real multi to the shapefile and try to force to single
        self.assertTrue(shapefile_layer.startEditing())
        ft = QgsFeature()
        ft.setGeometry(QgsGeometry.fromWkt('MultiPolygon (((0 0, 0 1, 1 1, 1 0, 0 0)), ((-10 -10,-10 -9,-9 -9,-10 -10)))'))
        ft.setAttributes([2])
        self.assertTrue(shapefile_layer.addFeatures([ft]))
        self.assertTrue(shapefile_layer.commitChanges())

        dest_multipart_failure_file_name = os.path.join(self.basetestpath, 'multipart_failure.gpkg')
        write_result, error_message = QgsVectorLayerExporter.exportLayer(shapefile_layer,
                                                                         dest_multipart_failure_file_name,
                                                                         'ogr',
                                                                         shapefile_layer.crs(),
                                                                         False,
                                                                         {
                                                                             "forceSinglePartGeometryType": True,
                                                                             "driverName": "GPKG",
                                                                         })
        self.assertTrue(QgsWkbTypes.isMultiType(multi_layer.wkbType()))
        self.assertEqual(write_result, QgsVectorLayerExporter.ErrFeatureWriteFailed, "Failed to transform a feature with ID '1' to single part. Writing stopped.")

    def testReadingLayerGeometryTypes(self):

        tests = [(osgeo.ogr.wkbPoint, 'Point (0 0)', QgsWkbTypes.Point, 'Point (0 0)'),
                 (osgeo.ogr.wkbPoint25D, 'Point Z (0 0 1)', QgsWkbTypes.PointZ, 'PointZ (0 0 1)'),
                 (osgeo.ogr.wkbPointM, 'Point M (0 0 1)', QgsWkbTypes.PointM, 'PointM (0 0 1)'),
                 (osgeo.ogr.wkbPointZM, 'Point ZM (0 0 1 2)', QgsWkbTypes.PointZM, 'PointZM (0 0 1 2)'),
                 (osgeo.ogr.wkbLineString, 'LineString (0 0, 1 1)', QgsWkbTypes.MultiLineString, 'MultiLineString ((0 0, 1 1))'),
                 (osgeo.ogr.wkbLineString25D, 'LineString Z (0 0 10, 1 1 10)', QgsWkbTypes.MultiLineStringZ, 'MultiLineStringZ ((0 0 10, 1 1 10))'),
                 (osgeo.ogr.wkbLineStringM, 'LineString M (0 0 10, 1 1 10)', QgsWkbTypes.MultiLineStringM, 'MultiLineStringM ((0 0 10, 1 1 10))'),
                 (osgeo.ogr.wkbLineStringZM, 'LineString ZM (0 0 10 20, 1 1 10 20)', QgsWkbTypes.MultiLineStringZM, 'MultiLineStringZM ((0 0 10 20, 1 1 10 20))'),
                 (osgeo.ogr.wkbPolygon, 'Polygon ((0 0,0 1,1 1,0 0))', QgsWkbTypes.MultiPolygon, 'MultiPolygon (((0 0, 0 1, 1 1, 0 0)))'),
                 (osgeo.ogr.wkbPolygon25D, 'Polygon Z ((0 0 10, 0 1 10, 1 1 10, 0 0 10))', QgsWkbTypes.MultiPolygonZ, 'MultiPolygonZ (((0 0 10, 0 1 10, 1 1 10, 0 0 10)))'),
                 (osgeo.ogr.wkbPolygonM, 'Polygon M ((0 0 10, 0 1 10, 1 1 10, 0 0 10))', QgsWkbTypes.MultiPolygonM, 'MultiPolygonM (((0 0 10, 0 1 10, 1 1 10, 0 0 10)))'),
                 (osgeo.ogr.wkbPolygonZM, 'Polygon ZM ((0 0 10 20, 0 1 10 20, 1 1 10 20, 0 0 10 20))', QgsWkbTypes.MultiPolygonZM, 'MultiPolygonZM (((0 0 10 20, 0 1 10 20, 1 1 10 20, 0 0 10 20)))'),
                 (osgeo.ogr.wkbMultiPoint, 'MultiPoint (0 0,1 1)', QgsWkbTypes.MultiPoint, 'MultiPoint ((0 0),(1 1))'),
                 (osgeo.ogr.wkbMultiPoint25D, 'MultiPoint Z ((0 0 10), (1 1 10))', QgsWkbTypes.MultiPointZ, 'MultiPointZ ((0 0 10),(1 1 10))'),
                 (osgeo.ogr.wkbMultiPointM, 'MultiPoint M ((0 0 10), (1 1 10))', QgsWkbTypes.MultiPointM, 'MultiPointM ((0 0 10),(1 1 10))'),
                 (osgeo.ogr.wkbMultiPointZM, 'MultiPoint ZM ((0 0 10 20), (1 1 10 20))', QgsWkbTypes.MultiPointZM, 'MultiPointZM ((0 0 10 20),(1 1 10 20))'),
                 (osgeo.ogr.wkbMultiLineString, 'MultiLineString ((0 0, 1 1))', QgsWkbTypes.MultiLineString, 'MultiLineString ((0 0, 1 1))'),
                 (osgeo.ogr.wkbMultiLineString25D, 'MultiLineString Z ((0 0 10, 1 1 10))', QgsWkbTypes.MultiLineStringZ, 'MultiLineStringZ ((0 0 10, 1 1 10))'),
                 (osgeo.ogr.wkbMultiLineStringM, 'MultiLineString M ((0 0 10, 1 1 10))', QgsWkbTypes.MultiLineStringM, 'MultiLineStringM ((0 0 10, 1 1 10))'),
                 (osgeo.ogr.wkbMultiLineStringZM, 'MultiLineString ZM ((0 0 10 20, 1 1 10 20))', QgsWkbTypes.MultiLineStringZM, 'MultiLineStringZM ((0 0 10 20, 1 1 10 20))'),
                 (osgeo.ogr.wkbMultiPolygon, 'MultiPolygon (((0 0,0 1,1 1,0 0)))', QgsWkbTypes.MultiPolygon, 'MultiPolygon (((0 0, 0 1, 1 1, 0 0)))'),
                 (osgeo.ogr.wkbMultiPolygon25D, 'MultiPolygon Z (((0 0 10, 0 1 10, 1 1 10, 0 0 10)))', QgsWkbTypes.MultiPolygonZ, 'MultiPolygonZ (((0 0 10, 0 1 10, 1 1 10, 0 0 10)))'),
                 (osgeo.ogr.wkbMultiPolygonM, 'MultiPolygon M (((0 0 10, 0 1 10, 1 1 10, 0 0 10)))', QgsWkbTypes.MultiPolygonM, 'MultiPolygonM (((0 0 10, 0 1 10, 1 1 10, 0 0 10)))'),
                 (osgeo.ogr.wkbMultiPolygonZM, 'MultiPolygon ZM (((0 0 10 20, 0 1 10 20, 1 1 10 20, 0 0 10 20)))', QgsWkbTypes.MultiPolygonZM, 'MultiPolygonZM (((0 0 10 20, 0 1 10 20, 1 1 10 20, 0 0 10 20)))'),
                 ]
        for ogr_type, wkt, qgis_type, expected_wkt in tests:

            filename = 'testPromoteToMulti'
            tmpfile = os.path.join(self.basetestpath, filename)
            ds = osgeo.ogr.GetDriverByName('ESRI Shapefile').CreateDataSource(tmpfile)
            lyr = ds.CreateLayer(filename, geom_type=ogr_type)
            f = osgeo.ogr.Feature(lyr.GetLayerDefn())
            f.SetGeometry(osgeo.ogr.CreateGeometryFromWkt(wkt))
            lyr.CreateFeature(f)
            ds = None

            vl = QgsVectorLayer(tmpfile, 'test', 'ogr')
            self.assertTrue(vl.isValid())
            self.assertEqual(vl.wkbType(), qgis_type)
            f = next(vl.getFeatures())
            self.assertEqual(f.geometry().constGet().asWkt(), expected_wkt)
            del vl

            osgeo.ogr.GetDriverByName('ESRI Shapefile').DeleteDataSource(tmpfile)

    def testEncoding(self):
        """ Test that CP852 shapefile is read/written correctly """

        tmpdir = tempfile.mkdtemp()
        self.dirs_to_cleanup.append(tmpdir)
        for file in glob.glob(os.path.join(TEST_DATA_DIR, 'test_852.*')):
            shutil.copy(os.path.join(TEST_DATA_DIR, file), tmpdir)
        datasource = os.path.join(tmpdir, 'test_852.shp')

        vl = QgsVectorLayer(datasource, 'test')
        self.assertTrue(vl.isValid())
        self.assertEqual([f.attributes() for f in vl.dataProvider().getFeatures()], [['abcŐ']])

        f = QgsFeature()
        f.setAttributes(['abcŐabcŐabcŐ'])
        self.assertTrue(vl.dataProvider().addFeature(f))

        # read it back in
        vl = QgsVectorLayer(datasource, 'test')
        self.assertTrue(vl.isValid())
        self.assertEqual([f.attributes() for f in vl.dataProvider().getFeatures()], [['abcŐ'], ['abcŐabcŐabcŐ']])

    def testSkipFeatureCountOnFeatureCount(self):
        """Test QgsDataProvider.SkipFeatureCount on featureCount()"""

        testPath = TEST_DATA_DIR + '/' + 'lines.shp'
        provider = QgsProviderRegistry.instance().createProvider('ogr', testPath, QgsDataProvider.ProviderOptions(), QgsDataProvider.SkipFeatureCount)
        self.assertTrue(provider.isValid())
        self.assertEqual(provider.featureCount(), QgsVectorDataProvider.UnknownCount)

    def testSkipFeatureCountOnSubLayers(self):
        """Test QgsDataProvider.SkipFeatureCount on subLayers()"""

        datasource = os.path.join(TEST_DATA_DIR, 'shapefile')
        provider = QgsProviderRegistry.instance().createProvider('ogr', datasource, QgsDataProvider.ProviderOptions(), QgsDataProvider.SkipFeatureCount)
        self.assertTrue(provider.isValid())
        sublayers = provider.subLayers()
        self.assertTrue(len(sublayers) > 1)
        self.assertEqual(int(sublayers[0].split(QgsDataProvider.sublayerSeparator())[2]), int(Qgis.FeatureCountState.Uncounted))

    def testLayersOnSameOGRLayerWithAndWithoutFilter(self):
        """Test fix for https://github.com/qgis/QGIS/issues/43361"""
        file_path = os.path.join(TEST_DATA_DIR, 'provider', 'shapefile.shp')
        uri = '{}|layerId=0|subset="name" = \'Apple\''.format(file_path)
        options = QgsDataProvider.ProviderOptions()
        vl1 = QgsVectorLayer(uri, 'vl1', 'ogr')
        vl2 = QgsVectorLayer(uri, 'vl2', 'ogr')
        vl3 = QgsVectorLayer('{}|layerId=0'.format(file_path), 'vl3', 'ogr')
        self.assertEqual(vl1.featureCount(), 1)
        vl1_extent = QgsGeometry.fromRect(vl1.extent())
        self.assertEqual(vl2.featureCount(), 1)
        vl2_extent = QgsGeometry.fromRect(vl2.extent())
        self.assertEqual(vl3.featureCount(), 5)
        vl3_extent = QgsGeometry.fromRect(vl3.extent())

        reference = QgsGeometry.fromRect(QgsRectangle(-68.2, 70.8, -68.2, 70.8))
        assert QgsGeometry.compare(vl1_extent.asPolygon()[0], reference.asPolygon()[0],
                                   0.00001), 'Expected {}, got {}'.format(reference.asWkt(), vl1_extent.asWkt())
        assert QgsGeometry.compare(vl2_extent.asPolygon()[0], reference.asPolygon()[0],
                                   0.00001), 'Expected {}, got {}'.format(reference.asWkt(), vl2_extent.asWkt())
        reference = QgsGeometry.fromRect(QgsRectangle(-71.123, 66.33, -65.32, 78.3))
        assert QgsGeometry.compare(vl3_extent.asPolygon()[0], reference.asPolygon()[0],
                                   0.00001), 'Expected {}, got {}'.format(reference.asWkt(), vl3_extent.asWkt())


if __name__ == '__main__':
    unittest.main()
