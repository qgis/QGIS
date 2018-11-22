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
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
import re
import tempfile
import shutil
import glob
import osgeo.gdal
import osgeo.ogr
import sys

from qgis.core import QgsSettings, QgsFeature, QgsField, QgsGeometry, QgsVectorLayer, QgsFeatureRequest, QgsVectorDataProvider
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
        assert(cls.vl.isValid())
        cls.source = cls.vl.dataProvider()
        cls.vl_poly = QgsVectorLayer('{}|layerid=0'.format(cls.basetestpolyfile), 'test', 'ogr')
        assert (cls.vl_poly.isValid())
        cls.poly_provider = cls.vl_poly.dataProvider()

        cls.dirs_to_cleanup = [cls.basetestpath, cls.repackfilepath]

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        for dirname in cls.dirs_to_cleanup:
            shutil.rmtree(dirname, True)

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
                       'intersects(point_on_surface($geometry),geom_from_wkt( \'Polygon ((-74.4 78.2, -74.4 79.1, -66.8 79.1, -66.8 78.2, -74.4 78.2))\'))'
                       ])
        if int(osgeo.gdal.VersionInfo()[:1]) < 2:
            filters.insert('not null')
        return filters

    def partiallyCompiledFilters(self):
        return set(['name = \'Apple\'',
                    'name = \'apple\'',
                    'name LIKE \'Apple\'',
                    'name LIKE \'aPple\'',
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

        # Test that update mode will be implictly enabled if doing an action
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

    def testRepackUnderFileLocks(self):
        ''' Test fix for #15570 and #15393 '''

        # This requires a GDAL fix done per https://trac.osgeo.org/gdal/ticket/6672
        # but on non-Windows version the test would succeed
        if int(osgeo.gdal.VersionInfo('VERSION_NUM')) < GDAL_COMPUTE_VERSION(2, 1, 2):
            return

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

        # This requires a GDAL fix done per https://trac.osgeo.org/gdal/ticket/6672
        # but on non-Windows version the test would succeed
        if int(osgeo.gdal.VersionInfo('VERSION_NUM')) < GDAL_COMPUTE_VERSION(2, 1, 2):
            return

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
            vl.setDataSource('', 'test', 'ogr')

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
        del(vl)

        # filter after construction ...
        subSet_vl2 = QgsVectorLayer(testPath, 'test', 'ogr')
        self.assertEqual(_lessdigits(subSet_vl2.extent().toString()), unfiltered_extent)
        # ... apply filter now!
        subSet_vl2.setSubsetString(subSetString)
        self.assertEqual(subSet_vl2.subsetString(), subSetString)
        self.assertNotEqual(_lessdigits(subSet_vl2.extent().toString()), unfiltered_extent)
        filtered_extent = _lessdigits(subSet_vl2.extent().toString())
        del(subSet_vl2)

        # filtered in constructor
        subSet_vl = QgsVectorLayer(testPath + subSet, 'subset_test', 'ogr')
        self.assertEqual(subSet_vl.subsetString(), subSetString)
        self.assertTrue(subSet_vl.isValid())

        # This was failing in bug 17863
        self.assertEqual(_lessdigits(subSet_vl.extent().toString()), filtered_extent)
        self.assertNotEqual(_lessdigits(subSet_vl.extent().toString()), unfiltered_extent)


if __name__ == '__main__':
    unittest.main()
