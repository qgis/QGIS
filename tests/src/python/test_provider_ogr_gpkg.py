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

import os
import re
import shutil
import sys
import tempfile
import time

import qgis  # NOQA
from osgeo import gdal, ogr
from qgis.core import (QgsFeature,
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
                       QgsVectorDataProvider)
from qgis.PyQt.QtCore import QCoreApplication, QVariant
from qgis.testing import start_app, unittest
from qgis.utils import spatialite_connect
from utilities import unitTestDataPath

TEST_DATA_DIR = unitTestDataPath()


def GDAL_COMPUTE_VERSION(maj, min, rev):
    return ((maj) * 1000000 + (min) * 10000 + (rev) * 100)


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

        uri = '{}|layerid=0'.format(filename)
        components = registry.decodeUri('ogr', uri)
        self.assertEqual(components["path"], filename)
        self.assertEqual(components["layerId"], 0)

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
        self.assertEqual(got_geom.asWkb(), reference.asWkb(), 'Expected {}, got {}'.format(reference.asWkt(), got_geom.asWkt()))

    def testCurveGeometryType(self):

        tmpfile = os.path.join(self.basetestpath, 'testCurveGeometryType.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        ds.CreateLayer('test', geom_type=ogr.wkbCurvePolygon)
        ds = None

        vl = QgsVectorLayer('{}'.format(tmpfile), 'test', 'ogr')
        self.assertEqual(vl.dataProvider().subLayers(), [QgsDataProvider.SUBLAYER_SEPARATOR.join(['0', 'test', '0', 'CurvePolygon', 'geom'])])
        f = QgsFeature()
        f.setGeometry(QgsGeometry.fromWkt('POLYGON ((0 0,0 1,1 1,0 0))'))
        vl.dataProvider().addFeatures([f])
        got = [feat for feat in vl.getFeatures()][0]
        got_geom = got.geometry()
        reference = QgsGeometry.fromWkt('CurvePolygon (((0 0, 0 1, 1 1, 0 0)))')
        # The geometries must be binarily identical
        self.assertEqual(got_geom.asWkb(), reference.asWkb(), 'Expected {}, got {}'.format(reference.asWkt(), got_geom.asWkt()))

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

    @unittest.skip(int(gdal.VersionInfo('VERSION_NUM')) < GDAL_COMPUTE_VERSION(2, 1, 2))
    def testGeopackageExtentUpdate(self):
        ''' test https://issues.qgis.org/issues/15273 '''
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
        vl = QgsVectorLayer('{}|layerid=0'.format(tmpfile, 'test', 'ogr'))
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
        self.assertEqual(len(got), 1) # this is the current behavior, broken

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

        related_count, idlist, namelist, desclist, errmsg = vl.listStylesInDatabase()
        self.assertEqual(related_count, -1)
        self.assertEqual(idlist, [])
        self.assertEqual(namelist, [])
        self.assertEqual(desclist, [])
        self.assertNotEqual(errmsg, "")

        qml, errmsg = vl.getStyleFromDatabase("1")
        self.assertEqual(qml, "")
        self.assertNotEqual(errmsg, "")

        qml, success = vl.loadNamedStyle('/idont/exist.gpkg')
        self.assertFalse(success)

        errorMsg = vl.saveStyleToDatabase("name", "description", False, "")
        self.assertNotEqual(errorMsg, "")

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

        related_count, idlist, namelist, desclist, errmsg = vl.listStylesInDatabase()
        self.assertEqual(related_count, 0)
        self.assertEqual(idlist, [])
        self.assertEqual(namelist, [])
        self.assertEqual(desclist, [])
        self.assertNotEqual(errmsg, "")

        qml, errmsg = vl.getStyleFromDatabase("not_existing")
        self.assertEqual(qml, "")
        self.assertNotEqual(errmsg, "")

        qml, success = vl.loadNamedStyle('{}|layerid=0'.format(tmpfile))
        self.assertFalse(success)

        errorMsg = vl.saveStyleToDatabase("name", "description", False, "")
        self.assertEqual(errorMsg, "")

        qml, errmsg = vl.getStyleFromDatabase("not_existing")
        self.assertEqual(qml, "")
        self.assertNotEqual(errmsg, "")

        related_count, idlist, namelist, desclist, errmsg = vl.listStylesInDatabase()
        self.assertEqual(related_count, 1)
        self.assertEqual(errmsg, "")
        self.assertEqual(idlist, ['1'])
        self.assertEqual(namelist, ['name'])
        self.assertEqual(desclist, ['description'])

        qml, errmsg = vl.getStyleFromDatabase("100")
        self.assertEqual(qml, "")
        self.assertNotEqual(errmsg, "")

        qml, errmsg = vl.getStyleFromDatabase("1")
        self.assertTrue(qml.startswith('<!DOCTYPE qgis'), qml)
        self.assertEqual(errmsg, "")

        # Try overwrite it but simulate answer no
        settings = QgsSettings()
        settings.setValue("/qgis/overwriteStyle", False)
        errorMsg = vl.saveStyleToDatabase("name", "description_bis", False, "")
        self.assertNotEqual(errorMsg, "")

        related_count, idlist, namelist, desclist, errmsg = vl.listStylesInDatabase()
        self.assertEqual(related_count, 1)
        self.assertEqual(errmsg, "")
        self.assertEqual(idlist, ['1'])
        self.assertEqual(namelist, ['name'])
        self.assertEqual(desclist, ['description'])

        # Try overwrite it and simulate answer yes
        settings = QgsSettings()
        settings.setValue("/qgis/overwriteStyle", True)
        errorMsg = vl.saveStyleToDatabase("name", "description_bis", False, "")
        self.assertEqual(errorMsg, "")

        related_count, idlist, namelist, desclist, errmsg = vl.listStylesInDatabase()
        self.assertEqual(related_count, 1)
        self.assertEqual(errmsg, "")
        self.assertEqual(idlist, ['1'])
        self.assertEqual(namelist, ['name'])
        self.assertEqual(desclist, ['description_bis'])

        errorMsg = vl2.saveStyleToDatabase("name_test2", "description_test2", True, "")
        self.assertEqual(errorMsg, "")

        errorMsg = vl.saveStyleToDatabase("name2", "description2", True, "")
        self.assertEqual(errorMsg, "")

        errorMsg = vl.saveStyleToDatabase("name3", "description3", True, "")
        self.assertEqual(errorMsg, "")

        related_count, idlist, namelist, desclist, errmsg = vl.listStylesInDatabase()
        self.assertEqual(related_count, 3)
        self.assertEqual(errmsg, "")
        self.assertEqual(idlist, ['1', '3', '4', '2'])
        self.assertEqual(namelist, ['name', 'name2', 'name3', 'name_test2'])
        self.assertEqual(desclist, ['description_bis', 'description2', 'description3', 'name_test2'])

        # Check that layers_style table is not list in subLayers()
        vl = QgsVectorLayer(tmpfile, 'test', 'ogr')
        sublayers = vl.dataProvider().subLayers()
        self.assertEqual(len(sublayers), 2, sublayers)

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

        QgsSettings().setValue("/qgis/walForSqlite3", None)

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
        exporter = QgsVectorLayerExporter(tmpfile, "ogr", fields, QgsWkbTypes.Polygon, QgsCoordinateReferenceSystem(3111), False, options)
        self.assertFalse(exporter.errorCode(),
                         'unexpected export error {}: {}'.format(exporter.errorCode(), exporter.errorMessage()))
        options['layerName'] = 'table2'
        exporter = QgsVectorLayerExporter(tmpfile, "ogr", fields, QgsWkbTypes.Point, QgsCoordinateReferenceSystem(3113), False, options)
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
        ''' test https://issues.qgis.org/issues/17034 '''
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
        self.assertEqual(got_geom.asWkb(), reference.asWkb(), 'Expected {}, got {}'.format(reference.asWkt(), got_geom.asWkt()))

        got = [feat for feat in vl2.getFeatures()][0]
        got_geom = got.geometry()
        self.assertEqual(got['attr'], 101)
        reference = QgsGeometry.fromWkt('Point (5 5)')
        self.assertEqual(got_geom.asWkb(), reference.asWkb(), 'Expected {}, got {}'.format(reference.asWkt(), got_geom.asWkt()))

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
        self.assertEqual(vl.dataProvider().subLayers(), [QgsDataProvider.SUBLAYER_SEPARATOR.join(['0', 'layer1:', '1', 'Point', 'geom:'])])

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
            self.assertEqual(count, 1)

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
            self.assertLessEqual(countNew, 4) # for some reason we get 4 and not 3

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

        time.sleep(1) # so timestamp gets updated
        ds = ogr.Open(tmpfile, update=1)
        ds.CreateLayer('test2', geom_type=ogr.wkbPoint)
        ds = None

        vl2 = QgsVectorLayer(u'{}'.format(tmpfile), 'test', u'ogr')
        vl2.subLayers()
        self.assertEqual(vl2.dataProvider().subLayers(), [QgsDataProvider.SUBLAYER_SEPARATOR.join(['0', 'test', '0', 'Point', 'geom']),
                                                          QgsDataProvider.SUBLAYER_SEPARATOR.join(['1', 'test2', '0', 'Point', 'geom'])])

    def testGeopackageLargeFID(self):

        tmpfile = os.path.join(self.basetestpath, 'testGeopackageLargeFID.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbPoint)
        lyr.CreateField(ogr.FieldDefn('str_field', ogr.OFTString))
        ds = None

        vl = QgsVectorLayer(u'{}'.format(tmpfile) + "|layername=" + "test", 'test', u'ogr')
        f = QgsFeature()
        f.setAttributes([1234567890123, None])
        self.assertTrue(vl.startEditing())
        self.assertTrue(vl.dataProvider().addFeatures([f]))
        self.assertTrue(vl.commitChanges())

        got = [feat for feat in vl.getFeatures()][0]
        self.assertEqual(got['fid'], 1234567890123)

        self.assertTrue(vl.startEditing())
        self.assertTrue(vl.changeGeometry(1234567890123, QgsGeometry.fromWkt('Point (3 50)')))
        self.assertTrue(vl.changeAttributeValue(1234567890123, 1, 'foo'))
        self.assertTrue(vl.commitChanges())

        got = [feat for feat in vl.getFeatures()][0]
        self.assertEqual(got['str_field'], 'foo')
        got_geom = got.geometry()
        self.assertIsNotNone(got_geom)

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
        self.assertEqual([f for f in layer.getFeatures()][0].geometry().asWkt(), 'Polygon ((0.5 0, 0.5 1, 1 1, 1 0, 0.5 0))')
        self.assertEqual([f for f in layer.getFeatures()][1].geometry().asWkt(), 'Polygon ((0.5 1, 0.5 0, 0 0, 0 1, 0.5 1))')

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
        subSet = '|layername=bug_17795|subset=%s' % subSetString

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

    def testRequestWithoutGeometryOnLayerMixedGeometry(self):
        """ Test bugfix for https://issues.qgis.org/issues/19077 """

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
        """ Test buggfix for https://issues.qgis.org/issues/19009 """

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
        """ Test perf improvement for for https://issues.qgis.org/issues/18402 """

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
        self.assertEqual(fc, 3) # didn't notice the hole

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
        exporter = QgsVectorLayerExporter(tmpfile, "ogr", fields, QgsWkbTypes.Polygon, QgsCoordinateReferenceSystem(3111), False, options, QgsFeatureSink.RegeneratePrimaryKey)
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
        p.setAutoTransaction(True)
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
        if int(gdal.VersionInfo('VERSION_NUM')) < GDAL_COMPUTE_VERSION(2, 4, 0):
            return

        tmpfile = os.path.join(self.basetestpath, 'test_json.gpkg')
        testdata_path = unitTestDataPath('provider')
        shutil.copy(os.path.join(unitTestDataPath('provider'), 'test_json.gpkg'), tmpfile)

        vl = QgsVectorLayer('{}|layerid=0'.format(tmpfile, 'foo', 'ogr'))
        self.assertTrue(vl.isValid())

        fields = vl.dataProvider().fields()
        self.assertEqual(fields.at(fields.indexFromName('json_content')).type(), QVariant.Map)

        fi = vl.getFeatures(QgsFeatureRequest())
        f = QgsFeature()

        #test reading dict value from attribute
        while fi.nextFeature(f):
            if f['fid'] == 1:
                self.assertIsInstance(f['json_content'], dict)
                self.assertEqual(f['json_content'], {'foo': 'bar'})
                #test changing dict value in attribute
                f['json_content'] = {'foo': 'baz'}
                self.assertEqual(f['json_content'], {'foo': 'baz'})
                #test changint dict to list
                f['json_content'] = ['eins', 'zwei', 'drei']
                self.assertEqual(f['json_content'], ['eins', 'zwei', 'drei'])
                #test changing list value in attribute
                f['json_content'] = ['eins', 'zwei', 'drei', 4]
                self.assertEqual(f['json_content'], ['eins', 'zwei', 'drei', 4])
                #test changing to complex json structure
                f['json_content'] = {'name': 'Lily', 'age': '0', 'cars': {'car1': ['fiat tipo', 'fiat punto', 'davoser schlitten'], 'car2': 'bobbycar', 'car3': 'tesla'}}
                self.assertEqual(f['json_content'], {'name': 'Lily', 'age': '0', 'cars': {'car1': ['fiat tipo', 'fiat punto', 'davoser schlitten'], 'car2': 'bobbycar', 'car3': 'tesla'}})

        #test adding attribute
        vl.startEditing()
        self.assertTrue(vl.addAttribute(QgsField('json_content2', QVariant.Map, "JSON", 60, 0, 'no comment', QVariant.String)))
        self.assertTrue(vl.commitChanges())

        vl.startEditing()
        self.assertTrue(vl.addAttribute(QgsField('json_content3', QVariant.Map, "JSON", 60, 0, 'no comment', QVariant.String)))
        self.assertTrue(vl.commitChanges())

        #test setting values to new attributes
        while fi.nextFeature(f):
            if f['fid'] == 2:
                f['json_content'] = {'uno': 'foo'}
                f['json_content2'] = ['uno', 'due', 'tre']
                f['json_content3'] = {'uno': ['uno', 'due', 'tre']}
                self.assertEqual(f['json_content'], {'foo': 'baz'})
                self.assertEqual(f['json_content2'], ['uno', 'due', 'tre'])
                self.assertEqual(f['json_content3'], {'uno': ['uno', 'due', 'tre']})

        #test deleting attribute
        vl.startEditing()
        self.assertTrue(vl.deleteAttribute(vl.fields().indexFromName('json_content3')))
        self.assertTrue(vl.commitChanges())

        #test if index of existent field is not -1 and the one of the deleted is -1
        self.assertNotEqual(vl.fields().indexFromName('json_content2'), -1)
        self.assertEqual(vl.fields().indexFromName('json_content3'), -1)

    def test_quote_identifier(self):
        """Regression #21100"""

        tmpfile = os.path.join(self.basetestpath, 'bug21100-wierd_field_names.gpkg')  # spellok
        shutil.copy(os.path.join(unitTestDataPath(''), 'bug21100-wierd_field_names.gpkg'), tmpfile) # spellok
        vl = QgsVectorLayer('{}|layerid=0'.format(tmpfile), 'foo', 'ogr')
        self.assertTrue(vl.isValid())
        for i in range(1, len(vl.fields())):
            self.assertEqual(vl.uniqueValues(i), {'a', 'b', 'c'})


if __name__ == '__main__':
    unittest.main()
