# -*- coding: utf-8 -*-
"""QGIS Unit tests for the OGR/GPKG provider.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Even Rouault'
__date__ = '2016-04-21'
__copyright__ = 'Copyright 2016, Even Rouault'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

import os
import tempfile
import shutil
import glob
from osgeo import gdal, ogr

from qgis.PyQt.QtCore import QCoreApplication, QSettings
from qgis.core import QgsVectorLayer, QgsVectorLayerImport, QgsFeature, QgsGeometry, QgsFeatureRequest, QgsRectangle
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

start_app()


def GDAL_COMPUTE_VERSION(maj, min, rev):
    return ((maj) * 1000000 + (min) * 10000 + (rev) * 100)


class ErrorReceiver():

    def __init__(self):
        self.msg = None

    def receiveError(self, msg):
        self.msg = msg


class TestPyQgsOGRProviderGpkg(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        # Create test layer
        cls.basetestpath = tempfile.mkdtemp()

        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("TestPyQgsOGRProviderGpkg.com")
        QCoreApplication.setApplicationName("TestPyQgsOGRProviderGpkg")
        QSettings().clear()
        start_app()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        shutil.rmtree(cls.basetestpath, True)
        QSettings().clear()

    def testSingleToMultiPolygonPromotion(self):

        tmpfile = os.path.join(self.basetestpath, 'testSingleToMultiPolygonPromotion.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbMultiPolygon)
        ds = None

        vl = QgsVectorLayer(u'{}|layerid=0'.format(tmpfile), u'test', u'ogr')
        f = QgsFeature()
        f.setGeometry(QgsGeometry.fromWkt('POLYGON ((0 0,0 1,1 1,0 0))'))
        vl.dataProvider().addFeatures([f])
        got = [f for f in vl.getFeatures()][0]
        got_geom = got.geometry()
        reference = QgsGeometry.fromWkt('MultiPolygon (((0 0, 0 1, 1 1, 0 0)))')
        # The geometries must be binarily identical
        self.assertEqual(got_geom.asWkb(), reference.asWkb(), 'Expected {}, got {}'.format(reference.exportToWkt(), got_geom.exportToWkt()))

    @unittest.expectedFailure(int(gdal.VersionInfo('VERSION_NUM')) < GDAL_COMPUTE_VERSION(2, 0, 0))
    def testCurveGeometryType(self):

        tmpfile = os.path.join(self.basetestpath, 'testCurveGeometryType.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbCurvePolygon)
        ds = None

        vl = QgsVectorLayer(u'{}'.format(tmpfile), u'test', u'ogr')
        self.assertEqual(vl.dataProvider().subLayers(), [u'0:test:0:CurvePolygon'])
        f = QgsFeature()
        f.setGeometry(QgsGeometry.fromWkt('POLYGON ((0 0,0 1,1 1,0 0))'))
        vl.dataProvider().addFeatures([f])
        got = [f for f in vl.getFeatures()][0]
        got_geom = got.geometry()
        reference = QgsGeometry.fromWkt('CurvePolygon (((0 0, 0 1, 1 1, 0 0)))')
        # The geometries must be binarily identical
        self.assertEqual(got_geom.asWkb(), reference.asWkb(), 'Expected {}, got {}'.format(reference.exportToWkt(), got_geom.exportToWkt()))

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

    @unittest.expectedFailure(int(gdal.VersionInfo('VERSION_NUM')) < GDAL_COMPUTE_VERSION(2, 0, 0))
    # We need GDAL 2.0 to issue PRAGMA journal_mode
    # Note: for that case, we don't strictly need turning on WAL
    def testBug15351_closeIter_commit_closeProvider(self):
        self.internalTestBug15351('closeIter_commit_closeProvider')

    @unittest.expectedFailure(int(gdal.VersionInfo('VERSION_NUM')) < GDAL_COMPUTE_VERSION(2, 0, 0))
    # We need GDAL 2.0 to issue PRAGMA journal_mode
    def testBug15351_closeIter_commit_closeProvider(self):
        self.internalTestBug15351('closeIter_commit_closeProvider')

    @unittest.expectedFailure(int(gdal.VersionInfo('VERSION_NUM')) < GDAL_COMPUTE_VERSION(2, 0, 0))
    # We need GDAL 2.0 to issue PRAGMA journal_mode
    def testBug15351_commit_closeIter_closeProvider(self):
        self.internalTestBug15351('commit_closeIter_closeProvider')

    def testGeopackageExtentUpdate(self):
        ''' test http://hub.qgis.org/issues/15273 '''
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

    def testDisablewalForSqlite3(self):
        ''' Test disabling walForSqlite3 setting '''
        QSettings().setValue("/qgis/walForSqlite3", False)

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
        ds = ogr.Open(tmpfile)
        lyr = ds.ExecuteSQL('PRAGMA journal_mode')
        f = lyr.GetNextFeature()
        res = f.GetField(0)
        ds.ReleaseResultSet(lyr)
        ds = None
        self.assertEqual(res, 'delete')

        self.assertTrue(vl.startEditing())
        feature = next(vl.getFeatures())
        self.assertTrue(vl.changeAttributeValue(feature.id(), 1, 1001))

        # Commit changes
        cbk = ErrorReceiver()
        vl.dataProvider().raiseError.connect(cbk.receiveError)
        self.assertTrue(vl.commitChanges())
        self.assertIsNone(cbk.msg)
        vl = None

        QSettings().setValue("/qgis/walForSqlite3", None)

    def testSimulatedDBManagerImport(self):
        uri = 'point?field=f1:int'
        uri += '&field=f2:double(6,4)'
        uri += '&field=f3:string(20)'
        lyr = QgsVectorLayer(uri, "x", "memory")
        self.assertTrue(lyr.isValid())
        f = QgsFeature(lyr.fields())
        f['f1'] = 1
        f['f2'] = 123.456
        f['f3'] = '12345678.90123456789'
        f2 = QgsFeature(lyr.fields())
        f2['f1'] = 2
        lyr.dataProvider().addFeatures([f, f2])

        tmpfile = os.path.join(self.basetestpath, 'testSimulatedDBManagerImport.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        ds = None
        options = {}
        options['update'] = True
        options['driverName'] = 'GPKG'
        options['layerName'] = 'my_out_table'
        err = QgsVectorLayerImport.importLayer(lyr, tmpfile, "ogr", lyr.crs(), False, False, options)
        self.assertEqual(err[0], QgsVectorLayerImport.NoError,
                         'unexpected import error {0}'.format(err))
        lyr = QgsVectorLayer(tmpfile + "|layername=my_out_table", "y", "ogr")
        self.assertTrue(lyr.isValid())
        features = lyr.getFeatures()
        f = next(features)
        self.assertEqual(f['f1'], 1)
        self.assertEqual(f['f2'], 123.456)
        self.assertEqual(f['f3'], '12345678.90123456789')
        f = next(features)
        self.assertEqual(f['f1'], 2)
        features = None

        # Test overwriting without overwrite option
        err = QgsVectorLayerImport.importLayer(lyr, tmpfile, "ogr", lyr.crs(), False, False, options)
        self.assertEqual(err[0], QgsVectorLayerImport.ErrCreateDataSource)

        # Test overwriting
        lyr = QgsVectorLayer(uri, "x", "memory")
        self.assertTrue(lyr.isValid())
        f = QgsFeature(lyr.fields())
        f['f1'] = 3
        lyr.dataProvider().addFeatures([f])
        options['overwrite'] = True
        err = QgsVectorLayerImport.importLayer(lyr, tmpfile, "ogr", lyr.crs(), False, False, options)
        self.assertEqual(err[0], QgsVectorLayerImport.NoError,
                         'unexpected import error {0}'.format(err))
        lyr = QgsVectorLayer(tmpfile + "|layername=my_out_table", "y", "ogr")
        self.assertTrue(lyr.isValid())
        features = lyr.getFeatures()
        f = next(features)
        self.assertEqual(f['f1'], 3)
        features = None


if __name__ == '__main__':
    unittest.main()
