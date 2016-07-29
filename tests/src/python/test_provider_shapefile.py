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
import tempfile
import shutil
import glob
import osgeo.gdal
import osgeo.ogr
import sys

from qgis.core import QgsFeature, QgsField, QgsGeometry, QgsVectorLayer, QgsFeatureRequest, QgsVectorDataProvider
from qgis.PyQt.QtCore import QSettings, QVariant
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath
from providertestbase import ProviderTestCase

start_app()
TEST_DATA_DIR = unitTestDataPath()


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
        cls.vl = QgsVectorLayer(u'{}|layerid=0'.format(cls.basetestfile), u'test', u'ogr')
        assert(cls.vl.isValid())
        cls.provider = cls.vl.dataProvider()
        cls.vl_poly = QgsVectorLayer(u'{}|layerid=0'.format(cls.basetestpolyfile), u'test', u'ogr')
        assert (cls.vl_poly.isValid())
        cls.poly_provider = cls.vl_poly.dataProvider()

        cls.dirs_to_cleanup = [cls.basetestpath, cls.repackfilepath]

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        for dirname in cls.dirs_to_cleanup:
            shutil.rmtree(dirname, True)

    def enableCompiler(self):
        QSettings().setValue(u'/qgis/compileExpressions', True)

    def disableCompiler(self):
        QSettings().setValue(u'/qgis/compileExpressions', False)

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
                       'intersects($geometry,geom_from_wkt( \'Polygon ((-72.2 66.1, -65.2 66.1, -65.2 72.0, -72.2 72.0, -72.2 66.1))\'))'])
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
        vl = QgsVectorLayer(u'{}|layerid=0'.format(self.repackfile), u'test', u'ogr')

        ids = [f.id() for f in vl.getFeatures(QgsFeatureRequest().setFilterExpression('pk=1'))]
        vl.setSelectedFeatures(ids)
        self.assertEqual(vl.selectedFeaturesIds(), ids)
        self.assertEqual(vl.pendingFeatureCount(), 5)
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

        vl = QgsVectorLayer(u'{}|layerid=0'.format(datasource), u'test', u'ogr')
        caps = vl.dataProvider().capabilities()
        self.assertTrue(caps & QgsVectorDataProvider.AddFeatures)
        self.assertTrue(caps & QgsVectorDataProvider.DeleteFeatures)
        self.assertTrue(caps & QgsVectorDataProvider.ChangeAttributeValues)
        self.assertTrue(caps & QgsVectorDataProvider.AddAttributes)
        self.assertTrue(caps & QgsVectorDataProvider.DeleteAttributes)
        self.assertTrue(caps & QgsVectorDataProvider.CreateSpatialIndex)
        self.assertTrue(caps & QgsVectorDataProvider.SelectAtId)
        self.assertTrue(caps & QgsVectorDataProvider.ChangeGeometries)
        #self.assertTrue(caps & QgsVectorDataProvider.ChangeFeatures)

        # We should be really opened in read-only mode even if write capabilities are declared
        self.assertEquals(vl.dataProvider().property("_debug_open_mode"), "read-only")

        # Unbalanced call to leaveUpdateMode()
        self.assertFalse(vl.dataProvider().leaveUpdateMode())

        # Test that startEditing() / commitChanges() plays with enterUpdateMode() / leaveUpdateMode()
        self.assertTrue(vl.startEditing())
        self.assertEquals(vl.dataProvider().property("_debug_open_mode"), "read-write")
        self.assertTrue(vl.dataProvider().isValid())

        self.assertTrue(vl.commitChanges())
        self.assertEquals(vl.dataProvider().property("_debug_open_mode"), "read-only")
        self.assertTrue(vl.dataProvider().isValid())

        # Manual enterUpdateMode() / leaveUpdateMode() with 2 depths
        self.assertTrue(vl.dataProvider().enterUpdateMode())
        self.assertEquals(vl.dataProvider().property("_debug_open_mode"), "read-write")
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
        self.assertEquals(values, [200])

        got_geom = [f_iter.geometry() for f_iter in features][0].geometry()
        self.assertEquals((got_geom.x(), got_geom.y()), (2.0, 49.0))

        self.assertTrue(vl.dataProvider().changeGeometryValues({fid: QgsGeometry.fromWkt('Point (3 50)')}))
        self.assertTrue(vl.dataProvider().changeAttributeValues({fid: {0: 100}}))

        features = [f_iter for f_iter in vl.getFeatures(QgsFeatureRequest().setFilterFid(fid))]
        values = [f_iter['pk'] for f_iter in features]

        got_geom = [f_iter.geometry() for f_iter in features][0].geometry()
        self.assertEquals((got_geom.x(), got_geom.y()), (3.0, 50.0))

        self.assertTrue(vl.dataProvider().deleteFeatures([fid]))

        # Check that it has really disappeared
        osgeo.gdal.PushErrorHandler('CPLQuietErrorHandler')
        features = [f_iter for f_iter in vl.getFeatures(QgsFeatureRequest().setFilterFid(fid))]
        osgeo.gdal.PopErrorHandler()
        self.assertEquals(features, [])

        self.assertTrue(vl.dataProvider().addAttributes([QgsField("new_field", QVariant.Int, "integer")]))
        self.assertTrue(vl.dataProvider().deleteAttributes([len(vl.dataProvider().fields()) - 1]))

        self.assertTrue(vl.startEditing())
        self.assertEquals(vl.dataProvider().property("_debug_open_mode"), "read-write")

        self.assertTrue(vl.commitChanges())
        self.assertEquals(vl.dataProvider().property("_debug_open_mode"), "read-write")

        self.assertTrue(vl.dataProvider().enterUpdateMode())
        self.assertEquals(vl.dataProvider().property("_debug_open_mode"), "read-write")

        self.assertTrue(vl.dataProvider().leaveUpdateMode())
        self.assertEquals(vl.dataProvider().property("_debug_open_mode"), "read-write")

        self.assertTrue(vl.dataProvider().leaveUpdateMode())
        self.assertEquals(vl.dataProvider().property("_debug_open_mode"), "read-only")

        # Test that update mode will be implictly enabled if doing an action
        # that requires update mode
        (ret, _) = vl.dataProvider().addFeatures([QgsFeature()])
        self.assertTrue(ret)
        self.assertEquals(vl.dataProvider().property("_debug_open_mode"), "read-write")

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

        vl = QgsVectorLayer(u'{}|layerid=0'.format(datasource), u'test', u'ogr')

        os.unlink(datasource)

        self.assertFalse(vl.dataProvider().enterUpdateMode())
        self.assertFalse(vl.dataProvider().enterUpdateMode())
        self.assertEquals(vl.dataProvider().property("_debug_open_mode"), "invalid")

        self.assertFalse(vl.dataProvider().isValid())
        self.assertEquals(len([f for f in vl.dataProvider().getFeatures()]), 0)
        self.assertEquals(len(vl.dataProvider().subLayers()), 0)
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

        vl1 = QgsVectorLayer(u'{}|layerid=0'.format(datasource), u'test', u'ogr')
        vl2 = QgsVectorLayer(u'{}|layerid=0'.format(datasource), u'test', u'ogr')
        self.assertTrue(vl1.startEditing())
        self.assertTrue(vl1.deleteAttributes([1]))
        self.assertTrue(vl1.commitChanges())
        self.assertEquals(len(vl1.fields()) + 1, len(vl2.fields()))
        # Reload
        vl2.reload()
        # And now check that fields are up-to-date
        self.assertEquals(len(vl1.fields()), len(vl2.fields()))

    def testRenameAttributes(self):
        ''' Test renameAttributes() '''

        tmpdir = tempfile.mkdtemp()
        self.dirs_to_cleanup.append(tmpdir)
        srcpath = os.path.join(TEST_DATA_DIR, 'provider')
        for file in glob.glob(os.path.join(srcpath, 'shapefile.*')):
            shutil.copy(os.path.join(srcpath, file), tmpdir)
        datasource = os.path.join(tmpdir, 'shapefile.shp')

        vl = QgsVectorLayer(u'{}|layerid=0'.format(datasource), u'test', u'ogr')
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
        vl = QgsVectorLayer(u'{}|layerid=0'.format(datasource), u'test', u'ogr')
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

        vl = QgsVectorLayer(u'{}|layerid=0'.format(datasource), u'test', u'ogr')
        self.assertTrue(vl.dataProvider().changeGeometryValues({0: QgsGeometry()}))
        vl = None

        vl = QgsVectorLayer(u'{}|layerid=0'.format(datasource), u'test', u'ogr')
        fet = next(vl.getFeatures())
        self.assertIsNone(fet.geometry())

    def testDeleteShapes(self):
        ''' Test fix for #11007 '''

        tmpdir = tempfile.mkdtemp()
        self.dirs_to_cleanup.append(tmpdir)
        srcpath = os.path.join(TEST_DATA_DIR, 'provider')
        for file in glob.glob(os.path.join(srcpath, 'shapefile.*')):
            shutil.copy(os.path.join(srcpath, file), tmpdir)
        datasource = os.path.join(tmpdir, 'shapefile.shp')

        vl = QgsVectorLayer(u'{}|layerid=0'.format(datasource), u'test', u'ogr')
        feature_count = vl.featureCount()
        # Start an iterator that will open a new connection
        iterator = vl.getFeatures()
        f = next(iterator)

        # Delete a feature
        self.assertTrue(vl.startEditing())
        self.assertTrue(vl.deleteFeature(1))
        self.assertTrue(vl.commitChanges())

        # Test the content of the shapefile while it is still opened
        ds = osgeo.ogr.Open(datasource)
        # Test repacking has been done
        self.assertTrue(ds.GetLayer(0).GetFeatureCount(), feature_count - 1)
        ds = None

        vl = None

if __name__ == '__main__':
    unittest.main()
