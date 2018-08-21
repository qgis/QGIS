# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsVectorFileWriter.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
from builtins import next
from builtins import str

__author__ = 'Tim Sutton'
__date__ = '20/08/2012'
__copyright__ = 'Copyright 2012, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.core import (QgsVectorLayer,
                       QgsFeature,
                       QgsField,
                       QgsGeometry,
                       QgsPointXY,
                       QgsCoordinateReferenceSystem,
                       QgsVectorFileWriter,
                       QgsFeatureRequest,
                       QgsProject,
                       QgsWkbTypes,
                       QgsRectangle,
                       QgsCoordinateTransform
                       )
from qgis.PyQt.QtCore import QDate, QTime, QDateTime, QVariant, QDir
import os
import tempfile
import osgeo.gdal  # NOQA
from osgeo import gdal, ogr
from qgis.testing import start_app, unittest
from utilities import writeShape, compareWkt, unitTestDataPath

TEST_DATA_DIR = unitTestDataPath()
start_app()


def GDAL_COMPUTE_VERSION(maj, min, rev):
    return ((maj) * 1000000 + (min) * 10000 + (rev) * 100)


class TestFieldValueConverter(QgsVectorFileWriter.FieldValueConverter):

    def __init__(self, layer):
        QgsVectorFileWriter.FieldValueConverter.__init__(self)
        self.layer = layer

    def fieldDefinition(self, field):
        idx = self.layer.fields().indexFromName(field.name())
        if idx == 0:
            return self.layer.fields()[idx]
        elif idx == 2:
            return QgsField('conv_attr', QVariant.String)
        return QgsField('unexpected_idx')

    def convert(self, idx, value):
        if idx == 0:
            return value
        elif idx == 2:
            if value == 3:
                return 'converted_val'
            else:
                return 'unexpected_val!'
        return 'unexpected_idx'


class TestQgsVectorFileWriter(unittest.TestCase):
    mMemoryLayer = None

    def testWrite(self):
        """Check we can write a vector file."""
        self.mMemoryLayer = QgsVectorLayer(
            ('Point?crs=epsg:4326&field=name:string(20)&'
             'field=age:integer&field=size:double&index=yes'),
            'test',
            'memory')

        self.assertIsNotNone(self.mMemoryLayer, 'Provider not initialized')
        myProvider = self.mMemoryLayer.dataProvider()
        self.assertIsNotNone(myProvider)

        ft = QgsFeature()
        ft.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(10, 10)))
        ft.setAttributes(['Johny', 20, 0.3])
        myResult, myFeatures = myProvider.addFeatures([ft])
        self.assertTrue(myResult)
        self.assertTrue(myFeatures)

        writeShape(self.mMemoryLayer, 'writetest.shp')

    def testWriteWithLongLongField(self):
        ml = QgsVectorLayer('NoGeometry?crs=epsg:4326&field=fldlonglong:long',
                            'test2', 'memory')
        provider = ml.dataProvider()
        feat = QgsFeature()
        feat.setAttributes([2262000000])
        provider.addFeatures([feat])

        filename = os.path.join(str(QDir.tempPath()), 'with_longlong_field')
        crs = QgsCoordinateReferenceSystem()
        crs.createFromId(4326, QgsCoordinateReferenceSystem.EpsgCrsId)
        rc, errmsg = QgsVectorFileWriter.writeAsVectorFormat(ml, filename, 'utf-8', crs, 'GPKG')

        # open the resulting geopackage
        vl = QgsVectorLayer(filename + '.gpkg', '', 'ogr')
        self.assertTrue(vl.isValid())

        # test values
        idx = vl.fields().indexFromName('fldlonglong')
        self.assertEqual(vl.getFeature(1).attributes()[idx], 2262000000)

    def testWriteWithBoolField(self):

        # init connection string
        dbconn = 'dbname=\'qgis_test\''
        if 'QGIS_PGTEST_DB' in os.environ:
            dbconn = os.environ['QGIS_PGTEST_DB']

        # create a vector layer
        vl = QgsVectorLayer('{} table="qgis_test"."boolean_table" sql='.format(dbconn), "testbool", "postgres")
        self.assertTrue(vl.isValid())

        # check that 1 of its fields is a bool
        fields = vl.fields()
        self.assertEqual(fields.at(fields.indexFromName('fld1')).type(), QVariant.Bool)

        # write a gpkg package with a bool field
        crs = QgsCoordinateReferenceSystem()
        crs.createFromId(4326, QgsCoordinateReferenceSystem.EpsgCrsId)
        filename = os.path.join(str(QDir.tempPath()), 'with_bool_field')
        rc, errmsg = QgsVectorFileWriter.writeAsVectorFormat(vl,
                                                             filename,
                                                             'utf-8',
                                                             crs,
                                                             'GPKG')

        self.assertEqual(rc, QgsVectorFileWriter.NoError)

        # open the resulting geopackage
        vl = QgsVectorLayer(filename + '.gpkg', '', 'ogr')
        self.assertTrue(vl.isValid())
        fields = vl.fields()

        # test type of converted field
        idx = fields.indexFromName('fld1')
        self.assertEqual(fields.at(idx).type(), QVariant.Bool)

        # test values
        self.assertEqual(vl.getFeature(1).attributes()[idx], 1)
        self.assertEqual(vl.getFeature(2).attributes()[idx], 0)

    def testDateTimeWriteShapefile(self):
        """Check writing date and time fields to an ESRI shapefile."""
        ml = QgsVectorLayer(
            ('Point?crs=epsg:4326&field=id:int&'
             'field=date_f:date&field=time_f:time&field=dt_f:datetime'),
            'test',
            'memory')

        self.assertTrue(ml.isValid())
        provider = ml.dataProvider()
        self.assertIsNotNone(provider)

        ft = QgsFeature()
        ft.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(10, 10)))
        ft.setAttributes([1, QDate(2014, 3, 5), QTime(13, 45, 22), QDateTime(QDate(2014, 3, 5), QTime(13, 45, 22))])
        res, features = provider.addFeatures([ft])
        self.assertTrue(res)
        self.assertTrue(features)

        dest_file_name = os.path.join(str(QDir.tempPath()), 'datetime.shp')
        crs = QgsCoordinateReferenceSystem()
        crs.createFromId(4326, QgsCoordinateReferenceSystem.EpsgCrsId)
        write_result, error_message = QgsVectorFileWriter.writeAsVectorFormat(
            ml,
            dest_file_name,
            'utf-8',
            crs,
            'ESRI Shapefile')
        self.assertEqual(write_result, QgsVectorFileWriter.NoError, error_message)

        # Open result and check
        created_layer = QgsVectorLayer('{}|layerid=0'.format(dest_file_name), 'test', 'ogr')

        fields = created_layer.dataProvider().fields()
        self.assertEqual(fields.at(fields.indexFromName('date_f')).type(), QVariant.Date)
        # shapefiles do not support time types, result should be string
        self.assertEqual(fields.at(fields.indexFromName('time_f')).type(), QVariant.String)
        # shapefiles do not support datetime types, result should be string
        self.assertEqual(fields.at(fields.indexFromName('dt_f')).type(), QVariant.String)

        f = next(created_layer.getFeatures(QgsFeatureRequest()))

        date_idx = created_layer.fields().lookupField('date_f')
        self.assertIsInstance(f.attributes()[date_idx], QDate)
        self.assertEqual(f.attributes()[date_idx], QDate(2014, 3, 5))
        time_idx = created_layer.fields().lookupField('time_f')
        # shapefiles do not support time types
        self.assertIsInstance(f.attributes()[time_idx], str)
        self.assertEqual(f.attributes()[time_idx], '13:45:22')
        # shapefiles do not support datetime types
        datetime_idx = created_layer.fields().lookupField('dt_f')
        self.assertIsInstance(f.attributes()[datetime_idx], str)
        self.assertEqual(f.attributes()[datetime_idx],
                         QDateTime(QDate(2014, 3, 5), QTime(13, 45, 22)).toString("yyyy/MM/dd hh:mm:ss.zzz"))

    def testWriterWithExtent(self):
        """Check writing using extent filter."""
        source_file = os.path.join(TEST_DATA_DIR, 'points.shp')
        source_layer = QgsVectorLayer(source_file, 'Points', 'ogr')
        self.assertTrue(source_layer.isValid())

        options = QgsVectorFileWriter.SaveVectorOptions()
        options.driverName = 'ESRI Shapefile'
        options.filterExtent = QgsRectangle(-111, 26, -96, 38)

        dest_file_name = os.path.join(str(QDir.tempPath()), 'extent_no_transform.shp')
        write_result, error_message = QgsVectorFileWriter.writeAsVectorFormat(
            source_layer,
            dest_file_name,
            options)
        self.assertEqual(write_result, QgsVectorFileWriter.NoError, error_message)

        # Open result and check
        created_layer = QgsVectorLayer('{}|layerid=0'.format(dest_file_name), 'test', 'ogr')
        features = [f for f in created_layer.getFeatures()]
        self.assertEqual(len(features), 5)
        for f in features:
            self.assertTrue(f.geometry().intersects(options.filterExtent))

    def testWriterWithExtentAndReprojection(self):
        """Check writing using extent filter with reprojection."""
        source_file = os.path.join(TEST_DATA_DIR, 'points.shp')
        source_layer = QgsVectorLayer(source_file, 'Points', 'ogr')
        self.assertTrue(source_layer.isValid())

        options = QgsVectorFileWriter.SaveVectorOptions()
        options.driverName = 'ESRI Shapefile'
        options.filterExtent = QgsRectangle(-12511460, 3045157, -10646621, 4683497)
        options.ct = QgsCoordinateTransform(source_layer.crs(), QgsCoordinateReferenceSystem.fromEpsgId(3785), QgsProject.instance())

        dest_file_name = os.path.join(str(QDir.tempPath()), 'extent_transform.shp')
        write_result, error_message = QgsVectorFileWriter.writeAsVectorFormat(
            source_layer,
            dest_file_name,
            options)
        self.assertEqual(write_result, QgsVectorFileWriter.NoError, error_message)

        # Open result and check
        created_layer = QgsVectorLayer('{}|layerid=0'.format(dest_file_name), 'test', 'ogr')
        features = [f for f in created_layer.getFeatures()]
        self.assertEqual(len(features), 5)
        for f in features:
            self.assertTrue(f.geometry().intersects(options.filterExtent))

    def testDateTimeWriteTabfile(self):
        """Check writing date and time fields to an MapInfo tabfile."""
        ml = QgsVectorLayer(
            ('Point?crs=epsg:4326&field=id:int&'
             'field=date_f:date&field=time_f:time&field=dt_f:datetime'),
            'test',
            'memory')

        self.assertIsNotNone(ml, 'Provider not initialized')
        self.assertTrue(ml.isValid(), 'Source layer not valid')
        provider = ml.dataProvider()
        self.assertIsNotNone(provider)

        ft = QgsFeature()
        ft.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(10, 10)))
        ft.setAttributes([1, QDate(2014, 3, 5), QTime(13, 45, 22), QDateTime(QDate(2014, 3, 5), QTime(13, 45, 22))])
        res, features = provider.addFeatures([ft])
        self.assertTrue(res)
        self.assertTrue(features)

        dest_file_name = os.path.join(str(QDir.tempPath()), 'datetime.tab')
        crs = QgsCoordinateReferenceSystem()
        crs.createFromId(4326, QgsCoordinateReferenceSystem.EpsgCrsId)
        write_result, error_message = QgsVectorFileWriter.writeAsVectorFormat(
            ml,
            dest_file_name,
            'utf-8',
            crs,
            'MapInfo File')
        self.assertEqual(write_result, QgsVectorFileWriter.NoError, error_message)

        # Open result and check
        created_layer = QgsVectorLayer('{}|layerid=0'.format(dest_file_name), 'test', 'ogr')

        fields = created_layer.dataProvider().fields()
        self.assertEqual(fields.at(fields.indexFromName('date_f')).type(), QVariant.Date)
        self.assertEqual(fields.at(fields.indexFromName('time_f')).type(), QVariant.Time)
        self.assertEqual(fields.at(fields.indexFromName('dt_f')).type(), QVariant.DateTime)

        f = next(created_layer.getFeatures(QgsFeatureRequest()))

        date_idx = created_layer.fields().lookupField('date_f')
        self.assertIsInstance(f.attributes()[date_idx], QDate)
        self.assertEqual(f.attributes()[date_idx], QDate(2014, 3, 5))
        time_idx = created_layer.fields().lookupField('time_f')
        self.assertIsInstance(f.attributes()[time_idx], QTime)
        self.assertEqual(f.attributes()[time_idx], QTime(13, 45, 22))
        datetime_idx = created_layer.fields().lookupField('dt_f')
        self.assertIsInstance(f.attributes()[datetime_idx], QDateTime)
        self.assertEqual(f.attributes()[datetime_idx], QDateTime(QDate(2014, 3, 5), QTime(13, 45, 22)))

    def testWriteShapefileWithZ(self):
        """Check writing geometries with Z dimension to an ESRI shapefile."""

        # start by saving a memory layer and forcing z
        ml = QgsVectorLayer(
            ('Point?crs=epsg:4326&field=id:int'),
            'test',
            'memory')

        self.assertIsNotNone(ml, 'Provider not initialized')
        self.assertTrue(ml.isValid(), 'Source layer not valid')
        provider = ml.dataProvider()
        self.assertIsNotNone(provider)

        ft = QgsFeature()
        ft.setGeometry(QgsGeometry.fromWkt('PointZ (1 2 3)'))
        ft.setAttributes([1])
        res, features = provider.addFeatures([ft])
        self.assertTrue(res)
        self.assertTrue(features)

        # check with both a standard PointZ and 25d style Point25D type
        for t in [QgsWkbTypes.PointZ, QgsWkbTypes.Point25D]:
            dest_file_name = os.path.join(str(QDir.tempPath()), 'point_{}.shp'.format(QgsWkbTypes.displayString(t)))
            crs = QgsCoordinateReferenceSystem()
            crs.createFromId(4326, QgsCoordinateReferenceSystem.EpsgCrsId)
            write_result, error_message = QgsVectorFileWriter.writeAsVectorFormat(
                ml,
                dest_file_name,
                'utf-8',
                crs,
                'ESRI Shapefile',
                overrideGeometryType=t)
            self.assertEqual(write_result, QgsVectorFileWriter.NoError, error_message)

            # Open result and check
            created_layer = QgsVectorLayer('{}|layerid=0'.format(dest_file_name), 'test', 'ogr')
            f = next(created_layer.getFeatures(QgsFeatureRequest()))
            g = f.geometry()
            wkt = g.asWkt()
            expWkt = 'PointZ (1 2 3)'
            self.assertTrue(compareWkt(expWkt, wkt),
                            "saving geometry with Z failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt))

            # also try saving out the shapefile version again, as an extra test
            # this tests that saving a layer with z WITHOUT explicitly telling the writer to keep z values,
            # will stay retain the z values
            dest_file_name = os.path.join(str(QDir.tempPath()),
                                          'point_{}_copy.shp'.format(QgsWkbTypes.displayString(t)))
            crs = QgsCoordinateReferenceSystem()
            crs.createFromId(4326, QgsCoordinateReferenceSystem.EpsgCrsId)
            write_result, error_message = QgsVectorFileWriter.writeAsVectorFormat(
                created_layer,
                dest_file_name,
                'utf-8',
                crs,
                'ESRI Shapefile')
            self.assertEqual(write_result, QgsVectorFileWriter.NoError, error_message)

            # Open result and check
            created_layer_from_shp = QgsVectorLayer('{}|layerid=0'.format(dest_file_name), 'test', 'ogr')
            f = next(created_layer_from_shp.getFeatures(QgsFeatureRequest()))
            g = f.geometry()
            wkt = g.asWkt()
            self.assertTrue(compareWkt(expWkt, wkt),
                            "saving geometry with Z failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt))

    def testWriteShapefileWithMultiConversion(self):
        """Check writing geometries to an ESRI shapefile with conversion to multi."""
        ml = QgsVectorLayer(
            ('Point?crs=epsg:4326&field=id:int'),
            'test',
            'memory')

        self.assertIsNotNone(ml, 'Provider not initialized')
        self.assertTrue(ml.isValid(), 'Source layer not valid')
        provider = ml.dataProvider()
        self.assertIsNotNone(provider)

        ft = QgsFeature()
        ft.setGeometry(QgsGeometry.fromWkt('Point (1 2)'))
        ft.setAttributes([1])
        res, features = provider.addFeatures([ft])
        self.assertTrue(res)
        self.assertTrue(features)

        dest_file_name = os.path.join(str(QDir.tempPath()), 'to_multi.shp')
        crs = QgsCoordinateReferenceSystem()
        crs.createFromId(4326, QgsCoordinateReferenceSystem.EpsgCrsId)
        write_result, error_message = QgsVectorFileWriter.writeAsVectorFormat(
            ml,
            dest_file_name,
            'utf-8',
            crs,
            'ESRI Shapefile',
            forceMulti=True)
        self.assertEqual(write_result, QgsVectorFileWriter.NoError, error_message)

        # Open result and check
        created_layer = QgsVectorLayer('{}|layerid=0'.format(dest_file_name), 'test', 'ogr')
        f = next(created_layer.getFeatures(QgsFeatureRequest()))
        g = f.geometry()
        wkt = g.asWkt()
        expWkt = 'MultiPoint ((1 2))'
        self.assertTrue(compareWkt(expWkt, wkt),
                        "saving geometry with multi conversion failed: mismatch Expected:\n%s\nGot:\n%s\n" % (
                        expWkt, wkt))

    def testWriteShapefileWithAttributeSubsets(self):
        """Tests writing subsets of attributes to files."""
        ml = QgsVectorLayer(
            ('Point?crs=epsg:4326&field=id:int&field=field1:int&field=field2:int&field=field3:int'),
            'test',
            'memory')

        self.assertIsNotNone(ml, 'Provider not initialized')
        self.assertTrue(ml.isValid(), 'Source layer not valid')
        provider = ml.dataProvider()
        self.assertIsNotNone(provider)

        ft = QgsFeature()
        ft.setGeometry(QgsGeometry.fromWkt('Point (1 2)'))
        ft.setAttributes([1, 11, 12, 13])
        res, features = provider.addFeatures([ft])
        self.assertTrue(res)
        self.assertTrue(features)

        # first write out with all attributes
        dest_file_name = os.path.join(str(QDir.tempPath()), 'all_attributes.shp')
        crs = QgsCoordinateReferenceSystem()
        crs.createFromId(4326, QgsCoordinateReferenceSystem.EpsgCrsId)
        write_result, error_message = QgsVectorFileWriter.writeAsVectorFormat(
            ml,
            dest_file_name,
            'utf-8',
            crs,
            'ESRI Shapefile',
            attributes=[])
        self.assertEqual(write_result, QgsVectorFileWriter.NoError, error_message)

        # Open result and check
        created_layer = QgsVectorLayer('{}|layerid=0'.format(dest_file_name), 'test', 'ogr')
        self.assertEqual(created_layer.fields().count(), 4)
        f = next(created_layer.getFeatures(QgsFeatureRequest()))
        self.assertEqual(f['id'], 1)
        self.assertEqual(f['field1'], 11)
        self.assertEqual(f['field2'], 12)
        self.assertEqual(f['field3'], 13)

        # now test writing out only a subset of attributes
        dest_file_name = os.path.join(str(QDir.tempPath()), 'subset_attributes.shp')
        write_result, error_message = QgsVectorFileWriter.writeAsVectorFormat(
            ml,
            dest_file_name,
            'utf-8',
            crs,
            'ESRI Shapefile',
            attributes=[1, 3])
        self.assertEqual(write_result, QgsVectorFileWriter.NoError, error_message)

        # Open result and check
        created_layer = QgsVectorLayer('{}|layerid=0'.format(dest_file_name), 'test', 'ogr')
        self.assertEqual(created_layer.fields().count(), 2)
        f = next(created_layer.getFeatures(QgsFeatureRequest()))
        self.assertEqual(f['field1'], 11)
        self.assertEqual(f['field3'], 13)

        # finally test writing no attributes
        dest_file_name = os.path.join(str(QDir.tempPath()), 'no_attributes.shp')
        write_result, error_message = QgsVectorFileWriter.writeAsVectorFormat(
            ml,
            dest_file_name,
            'utf-8',
            crs,
            'ESRI Shapefile',
            skipAttributeCreation=True)
        self.assertEqual(write_result, QgsVectorFileWriter.NoError, error_message)

        # Open result and check
        created_layer = QgsVectorLayer('{}|layerid=0'.format(dest_file_name), 'test', 'ogr')
        # expect only a default 'FID' field for shapefiles
        self.assertEqual(created_layer.fields().count(), 1)
        self.assertEqual(created_layer.fields()[0].name(), 'FID')
        # in this case we also check that the geometry exists, to make sure feature has been correctly written
        # even without attributes
        f = next(created_layer.getFeatures(QgsFeatureRequest()))
        g = f.geometry()
        wkt = g.asWkt()
        expWkt = 'Point (1 2)'
        self.assertTrue(compareWkt(expWkt, wkt),
                        "geometry not saved correctly when saving without attributes : mismatch Expected:\n%s\nGot:\n%s\n" % (
                        expWkt, wkt))
        self.assertEqual(f['FID'], 0)

    def testValueConverter(self):
        """Tests writing a layer with a field value converter."""
        ml = QgsVectorLayer(
            ('Point?field=nonconv:int&field=ignored:string&field=converted:int'),
            'test',
            'memory')

        self.assertIsNotNone(ml, 'Provider not initialized')
        self.assertTrue(ml.isValid(), 'Source layer not valid')
        provider = ml.dataProvider()
        self.assertIsNotNone(provider)
        self.assertEqual(ml.fields().count(), 3)

        ft = QgsFeature()
        ft.setAttributes([1, 'ignored', 3])
        res, features = provider.addFeatures([ft])
        self.assertTrue(res)
        self.assertTrue(features)

        dest_file_name = os.path.join(str(QDir.tempPath()), 'value_converter.shp')
        converter = TestFieldValueConverter(ml)
        write_result, error_message = QgsVectorFileWriter.writeAsVectorFormat(
            ml,
            dest_file_name,
            'utf-8',
            QgsCoordinateReferenceSystem(),
            'ESRI Shapefile',
            attributes=[0, 2],
            fieldValueConverter=converter)
        self.assertEqual(write_result, QgsVectorFileWriter.NoError, error_message)

        # Open result and check
        created_layer = QgsVectorLayer('{}|layerid=0'.format(dest_file_name), 'test', 'ogr')
        self.assertEqual(created_layer.fields().count(), 2)
        f = next(created_layer.getFeatures(QgsFeatureRequest()))
        self.assertEqual(f['nonconv'], 1)
        self.assertEqual(f['conv_attr'], 'converted_val')

    def testInteger64WriteTabfile(self):
        """Check writing Integer64 fields to an MapInfo tabfile (which does not support that type)."""
        ml = QgsVectorLayer(
            ('Point?crs=epsg:4326&field=int8:int8'),
            'test',
            'memory')

        self.assertIsNotNone(ml, 'Provider not initialized')
        self.assertTrue(ml.isValid(), 'Source layer not valid')
        provider = ml.dataProvider()
        self.assertIsNotNone(provider)

        ft = QgsFeature()
        ft.setAttributes([2123456789])
        res, features = provider.addFeatures([ft])
        self.assertTrue(res)
        self.assertTrue(features)

        dest_file_name = os.path.join(str(QDir.tempPath()), 'integer64.tab')
        crs = QgsCoordinateReferenceSystem()
        crs.createFromId(4326, QgsCoordinateReferenceSystem.EpsgCrsId)
        write_result, error_message = QgsVectorFileWriter.writeAsVectorFormat(
            ml,
            dest_file_name,
            'utf-8',
            crs,
            'MapInfo File')
        self.assertEqual(write_result, QgsVectorFileWriter.NoError, error_message)

        # Open result and check
        created_layer = QgsVectorLayer('{}|layerid=0'.format(dest_file_name), 'test', 'ogr')

        fields = created_layer.dataProvider().fields()
        self.assertEqual(fields.at(fields.indexFromName('int8')).type(), QVariant.Double)

        f = next(created_layer.getFeatures(QgsFeatureRequest()))

        int8_idx = created_layer.fields().lookupField('int8')
        self.assertEqual(f.attributes()[int8_idx], 2123456789)

    def testDefaultDatasetOptions(self):
        """ Test retrieving default dataset options for a format """

        # NOTE - feel free to adapt these if the defaults change!
        options = QgsVectorFileWriter.defaultDatasetOptions('not a format')
        self.assertEqual(options, [])
        options = QgsVectorFileWriter.defaultDatasetOptions('ESRI Shapefile')
        self.assertEqual(options, [])
        options = QgsVectorFileWriter.defaultDatasetOptions('GML')
        # just test a few
        self.assertTrue('GML3_LONGSRS=YES' in options)
        self.assertTrue('STRIP_PREFIX=NO' in options)

    def testDefaultLayerOptions(self):
        """ Test retrieving default layer options for a format """

        # NOTE - feel free to adapt these if the defaults change!
        options = QgsVectorFileWriter.defaultLayerOptions('not a format')
        self.assertEqual(options, [])
        options = QgsVectorFileWriter.defaultLayerOptions('ESRI Shapefile')
        self.assertEqual(options, ['RESIZE=NO'])
        options = QgsVectorFileWriter.defaultLayerOptions('GML')
        self.assertEqual(options, [])

    def testOverwriteLayer(self):
        """Tests writing a layer with a field value converter."""

        ml = QgsVectorLayer('Point?field=firstfield:int', 'test', 'memory')
        provider = ml.dataProvider()

        ft = QgsFeature()
        ft.setAttributes([1])
        provider.addFeatures([ft])

        options = QgsVectorFileWriter.SaveVectorOptions()
        options.driverName = 'GPKG'
        options.layerName = 'test'
        filename = '/vsimem/out.gpkg'
        write_result, error_message = QgsVectorFileWriter.writeAsVectorFormat(
            ml,
            filename,
            options)
        self.assertEqual(write_result, QgsVectorFileWriter.NoError, error_message)

        ds = ogr.Open(filename, update=1)
        lyr = ds.GetLayerByName('test')
        self.assertIsNotNone(lyr)
        f = lyr.GetNextFeature()
        self.assertEqual(f['firstfield'], 1)
        ds.CreateLayer('another_layer')
        del f
        del lyr
        del ds

        caps = QgsVectorFileWriter.editionCapabilities(filename)
        self.assertTrue((caps & QgsVectorFileWriter.CanAddNewLayer))
        self.assertTrue((caps & QgsVectorFileWriter.CanAppendToExistingLayer))
        self.assertTrue((caps & QgsVectorFileWriter.CanAddNewFieldsToExistingLayer))
        self.assertTrue((caps & QgsVectorFileWriter.CanDeleteLayer))

        self.assertTrue(QgsVectorFileWriter.targetLayerExists(filename, 'test'))

        self.assertFalse(QgsVectorFileWriter.areThereNewFieldsToCreate(filename, 'test', ml, [0]))

        # Test CreateOrOverwriteLayer
        ml = QgsVectorLayer('Point?field=firstfield:int', 'test', 'memory')
        provider = ml.dataProvider()

        ft = QgsFeature()
        ft.setAttributes([2])
        provider.addFeatures([ft])

        options = QgsVectorFileWriter.SaveVectorOptions()
        options.driverName = 'GPKG'
        options.layerName = 'test'
        options.actionOnExistingFile = QgsVectorFileWriter.CreateOrOverwriteLayer
        filename = '/vsimem/out.gpkg'
        write_result, error_message = QgsVectorFileWriter.writeAsVectorFormat(
            ml,
            filename,
            options)
        self.assertEqual(write_result, QgsVectorFileWriter.NoError, error_message)

        ds = ogr.Open(filename)
        lyr = ds.GetLayerByName('test')
        self.assertIsNotNone(lyr)
        f = lyr.GetNextFeature()
        self.assertEqual(f['firstfield'], 2)
        # another_layer should still exist
        self.assertIsNotNone(ds.GetLayerByName('another_layer'))
        del f
        del lyr
        del ds

        # Test CreateOrOverwriteFile
        ml = QgsVectorLayer('Point?field=firstfield:int', 'test', 'memory')
        provider = ml.dataProvider()

        ft = QgsFeature()
        ft.setAttributes([3])
        provider.addFeatures([ft])

        options = QgsVectorFileWriter.SaveVectorOptions()
        options.driverName = 'GPKG'
        options.layerName = 'test'
        filename = '/vsimem/out.gpkg'
        write_result, error_message = QgsVectorFileWriter.writeAsVectorFormat(
            ml,
            filename,
            options)
        self.assertEqual(write_result, QgsVectorFileWriter.NoError, error_message)

        ds = ogr.Open(filename)
        lyr = ds.GetLayerByName('test')
        self.assertIsNotNone(lyr)
        f = lyr.GetNextFeature()
        self.assertEqual(f['firstfield'], 3)
        # another_layer should no longer exist
        self.assertIsNone(ds.GetLayerByName('another_layer'))
        del f
        del lyr
        del ds

        # Test AppendToLayerNoNewFields
        ml = QgsVectorLayer('Point?field=firstfield:int&field=secondfield:int', 'test', 'memory')
        provider = ml.dataProvider()

        ft = QgsFeature()
        ft.setAttributes([4, -10])
        provider.addFeatures([ft])

        self.assertTrue(QgsVectorFileWriter.areThereNewFieldsToCreate(filename, 'test', ml, [0, 1]))

        options = QgsVectorFileWriter.SaveVectorOptions()
        options.driverName = 'GPKG'
        options.layerName = 'test'
        options.actionOnExistingFile = QgsVectorFileWriter.AppendToLayerNoNewFields
        filename = '/vsimem/out.gpkg'
        write_result, error_message = QgsVectorFileWriter.writeAsVectorFormat(
            ml,
            filename,
            options)
        self.assertEqual(write_result, QgsVectorFileWriter.NoError, error_message)

        ds = ogr.Open(filename)
        lyr = ds.GetLayerByName('test')
        self.assertEqual(lyr.GetLayerDefn().GetFieldCount(), 1)
        self.assertIsNotNone(lyr)
        f = lyr.GetNextFeature()
        self.assertEqual(f['firstfield'], 3)
        f = lyr.GetNextFeature()
        self.assertEqual(f['firstfield'], 4)
        del f
        del lyr
        del ds

        # Test AppendToLayerAddFields
        ml = QgsVectorLayer('Point?field=firstfield:int&field=secondfield:int', 'test', 'memory')
        provider = ml.dataProvider()

        ft = QgsFeature()
        ft.setAttributes([5, -1])
        provider.addFeatures([ft])

        self.assertTrue(QgsVectorFileWriter.areThereNewFieldsToCreate(filename, 'test', ml, [0, 1]))

        options = QgsVectorFileWriter.SaveVectorOptions()
        options.driverName = 'GPKG'
        options.layerName = 'test'
        options.actionOnExistingFile = QgsVectorFileWriter.AppendToLayerAddFields
        filename = '/vsimem/out.gpkg'
        write_result, error_message = QgsVectorFileWriter.writeAsVectorFormat(
            ml,
            filename,
            options)
        self.assertEqual(write_result, QgsVectorFileWriter.NoError, error_message)

        ds = ogr.Open(filename)
        lyr = ds.GetLayerByName('test')
        self.assertEqual(lyr.GetLayerDefn().GetFieldCount(), 2)
        self.assertIsNotNone(lyr)
        f = lyr.GetNextFeature()
        self.assertEqual(f['firstfield'], 3)
        if hasattr(f, "IsFieldSetAndNotNull"):
            # GDAL >= 2.2
            self.assertFalse(f.IsFieldSetAndNotNull('secondfield'))
        else:
            self.assertFalse(f.IsFieldSet('secondfield'))
        f = lyr.GetNextFeature()
        self.assertEqual(f['firstfield'], 4)
        if hasattr(f, "IsFieldSetAndNotNull"):
            self.assertFalse(f.IsFieldSetAndNotNull('secondfield'))
        else:
            self.assertFalse(f.IsFieldSet('secondfield'))
        f = lyr.GetNextFeature()
        self.assertEqual(f['firstfield'], 5)
        self.assertEqual(f['secondfield'], -1)
        del f
        del lyr
        del ds

        gdal.Unlink(filename)

    def testSupportedFiltersAndFormat(self):
        # test with formats in recommended order
        formats = QgsVectorFileWriter.supportedFiltersAndFormats(QgsVectorFileWriter.SortRecommended)
        self.assertEqual(formats[0].filterString, 'GeoPackage (*.gpkg *.GPKG)')
        self.assertEqual(formats[0].driverName, 'GPKG')
        self.assertEqual(formats[0].globs, ['*.gpkg'])
        self.assertEqual(formats[1].filterString, 'ESRI Shapefile (*.shp *.SHP)')
        self.assertEqual(formats[1].driverName, 'ESRI Shapefile')
        self.assertEqual(formats[1].globs, ['*.shp'])
        self.assertTrue('ODS' in [f.driverName for f in formats])

        interlis_format = [f for f in formats if f.driverName == 'Interlis 2'][0]
        self.assertEqual(interlis_format.globs, ['*.xtf', '*.xml', '*.ili'])

        # alphabetical sorting
        formats2 = QgsVectorFileWriter.supportedFiltersAndFormats(QgsVectorFileWriter.VectorFormatOptions())
        self.assertTrue(formats2[0].driverName < formats2[1].driverName)
        self.assertCountEqual([f.driverName for f in formats], [f.driverName for f in formats2])
        self.assertNotEqual(formats2[0].driverName, 'GeoPackage')

        # skip non-spatial
        formats = QgsVectorFileWriter.supportedFiltersAndFormats(QgsVectorFileWriter.SkipNonSpatialFormats)
        self.assertFalse('ODS' in [f.driverName for f in formats])

    def testOgrDriverList(self):
        # test with drivers in recommended order
        drivers = QgsVectorFileWriter.ogrDriverList(QgsVectorFileWriter.SortRecommended)
        self.assertEqual(drivers[0].longName, 'GeoPackage')
        self.assertEqual(drivers[0].driverName, 'GPKG')
        self.assertEqual(drivers[1].longName, 'ESRI Shapefile')
        self.assertEqual(drivers[1].driverName, 'ESRI Shapefile')
        self.assertTrue('ODS' in [f.driverName for f in drivers])

        # ensure that XLSX comes before SQLite, because we should sort on longName, not driverName!
        ms_xlsx_index = next(i for i, v in enumerate(drivers) if v.driverName == 'XLSX')
        sqlite_index = next(i for i, v in enumerate(drivers) if v.driverName == 'SQLite')
        self.assertLess(ms_xlsx_index, sqlite_index)

        self.assertIn('[XLSX]', drivers[ms_xlsx_index].longName)

        # alphabetical sorting
        drivers2 = QgsVectorFileWriter.ogrDriverList(QgsVectorFileWriter.VectorFormatOptions())
        self.assertTrue(drivers2[0].longName < drivers2[1].longName)
        self.assertCountEqual([d.driverName for d in drivers], [d.driverName for d in drivers2])
        self.assertNotEqual(drivers2[0].driverName, 'GPKG')

        # skip non-spatial
        formats = QgsVectorFileWriter.ogrDriverList(QgsVectorFileWriter.SkipNonSpatialFormats)
        self.assertFalse('ODS' in [f.driverName for f in formats])

    def testSupportedFormatExtensions(self):
        formats = QgsVectorFileWriter.supportedFormatExtensions()
        self.assertTrue('gpkg' in formats)
        self.assertFalse('exe' in formats)
        self.assertEqual(formats[0], 'gpkg')
        self.assertEqual(formats[1], 'shp')
        self.assertTrue('ods' in formats)
        self.assertTrue('xtf' in formats)
        self.assertTrue('ili' in formats)

        for i in range(2, len(formats) - 1):
            self.assertLess(formats[i].lower(), formats[i + 1].lower())

        # alphabetical sorting
        formats2 = QgsVectorFileWriter.supportedFormatExtensions(QgsVectorFileWriter.VectorFormatOptions())
        self.assertTrue(formats2[0] < formats2[1])
        self.assertCountEqual(formats, formats2)
        self.assertNotEqual(formats2[0], 'gpkg')
        for i in range(0, len(formats2) - 1):
            self.assertLess(formats2[i].lower(), formats2[i + 1].lower())

        formats = QgsVectorFileWriter.supportedFormatExtensions(QgsVectorFileWriter.SkipNonSpatialFormats)
        self.assertFalse('ods' in formats)

    def testFileFilterString(self):
        formats = QgsVectorFileWriter.fileFilterString()
        self.assertTrue('gpkg' in formats)
        self.assertTrue('shp' in formats)
        self.assertLess(formats.index('gpkg'), formats.index('shp'))
        self.assertTrue('ods' in formats)
        parts = formats.split(';;')
        for i in range(2, len(parts) - 1):
            self.assertLess(parts[i].lower(), parts[i + 1].lower())

        # alphabetical sorting
        formats2 = QgsVectorFileWriter.fileFilterString(QgsVectorFileWriter.VectorFormatOptions())
        self.assertNotEqual(formats.index('gpkg'), formats2.index('gpkg'))
        parts = formats2.split(';;')
        for i in range(len(parts) - 1):
            self.assertLess(parts[i].lower(), parts[i + 1].lower())

        # hide non spatial
        formats = QgsVectorFileWriter.fileFilterString(QgsVectorFileWriter.SkipNonSpatialFormats)
        self.assertFalse('ods' in formats)

    def testDriverForExtension(self):
        self.assertEqual(QgsVectorFileWriter.driverForExtension('shp'), 'ESRI Shapefile')
        self.assertEqual(QgsVectorFileWriter.driverForExtension('SHP'), 'ESRI Shapefile')
        self.assertEqual(QgsVectorFileWriter.driverForExtension('sHp'), 'ESRI Shapefile')
        self.assertEqual(QgsVectorFileWriter.driverForExtension('.shp'), 'ESRI Shapefile')
        self.assertEqual(QgsVectorFileWriter.driverForExtension('tab'), 'MapInfo File')
        self.assertEqual(QgsVectorFileWriter.driverForExtension('.GML'), 'GML')
        self.assertEqual(QgsVectorFileWriter.driverForExtension('not a format'), '')
        self.assertEqual(QgsVectorFileWriter.driverForExtension(''), '')

    def testSupportsFeatureStyles(self):
        self.assertFalse(QgsVectorFileWriter.supportsFeatureStyles('ESRI Shapefile'))
        self.assertFalse(QgsVectorFileWriter.supportsFeatureStyles('not a driver'))
        self.assertTrue(QgsVectorFileWriter.supportsFeatureStyles('DXF'))
        self.assertTrue(QgsVectorFileWriter.supportsFeatureStyles('KML'))
        self.assertTrue(QgsVectorFileWriter.supportsFeatureStyles('MapInfo File'))
        self.assertTrue(QgsVectorFileWriter.supportsFeatureStyles('MapInfo MIF'))

    def testOverwriteGPKG(self):
        """Test that overwriting the same origin GPKG file works only if the layername is different"""

        # Prepare test data
        ml = QgsVectorLayer('Point?field=firstfield:int&field=secondfield:int', 'test', 'memory')
        provider = ml.dataProvider()
        ft = QgsFeature()
        ft.setAttributes([4, -10])
        provider.addFeatures([ft])
        filehandle, filename = tempfile.mkstemp('.gpkg')

        options = QgsVectorFileWriter.SaveVectorOptions()
        options.driverName = 'GPKG'
        options.layerName = 'test'
        write_result, error_message = QgsVectorFileWriter.writeAsVectorFormat(
            ml,
            filename,
            options)
        self.assertEqual(write_result, QgsVectorFileWriter.NoError, error_message)

        # Real test
        vl = QgsVectorLayer("%s|layername=test" % filename, 'src_test', 'ogr')
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.featureCount(), 1)

        # This must fail
        write_result, error_message = QgsVectorFileWriter.writeAsVectorFormat(
            vl,
            filename,
            options)
        self.assertEqual(write_result, QgsVectorFileWriter.ErrCreateDataSource)
        self.assertEqual(error_message, 'Cannot overwrite a OGR layer in place')

        options.layerName = 'test2'
        write_result, error_message = QgsVectorFileWriter.writeAsVectorFormat(
            vl,
            filename,
            options)
        self.assertEqual(write_result, QgsVectorFileWriter.NoError, error_message)


if __name__ == '__main__':
    unittest.main()
