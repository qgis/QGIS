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
                       QgsCoordinateTransform,
                       QgsMultiPolygon,
                       QgsTriangle,
                       QgsPoint,
                       QgsFields,
                       QgsCoordinateTransformContext,
                       QgsFeatureSink,
                       QgsMemoryProviderUtils,
                       QgsLayerMetadata,
                       QgsUnsetAttributeValue,
                       NULL
                       )
from qgis.PyQt.QtCore import QDate, QTime, QDateTime, QVariant, QDir, QByteArray, QTemporaryDir
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

    def testWritePreferAlias(self):
        """Test preferring field alias."""
        layer = QgsVectorLayer(
            ('Point?crs=epsg:4326&field=name:string(20)&'
             'field=age:integer&field=size:double&index=yes'),
            'test',
            'memory')

        self.assertTrue(layer.isValid())
        myProvider = layer.dataProvider()

        layer.setFieldAlias(0, 'My Name')
        layer.setFieldAlias(2, 'My Size')

        ft = QgsFeature()
        ft.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(10, 10)))
        ft.setAttributes(['Johny', 20, 0.3])
        myResult, myFeatures = myProvider.addFeatures([ft])
        self.assertTrue(myResult)
        self.assertTrue(myFeatures)

        options = QgsVectorFileWriter.SaveVectorOptions()
        options.driverName = 'ESRI Shapefile'
        options.fieldNameSource = QgsVectorFileWriter.Original

        dest = os.path.join(str(QDir.tempPath()), 'alias.shp')
        result, err = QgsVectorFileWriter.writeAsVectorFormatV2(
            layer,
            dest,
            QgsProject.instance().transformContext(),
            options)
        self.assertEqual(result, QgsVectorFileWriter.NoError)

        res = QgsVectorLayer(dest, 'result')
        self.assertTrue(res.isValid())
        self.assertEqual([f.name() for f in res.fields()], ['name', 'age', 'size'])

        options.fieldNameSource = QgsVectorFileWriter.PreferAlias
        dest = os.path.join(str(QDir.tempPath()), 'alias2.shp')
        result, err = QgsVectorFileWriter.writeAsVectorFormatV2(
            layer,
            dest,
            QgsProject.instance().transformContext(),
            options)
        self.assertEqual(result, QgsVectorFileWriter.NoError)

        res = QgsVectorLayer(dest, 'result')
        self.assertTrue(res.isValid())
        self.assertEqual([f.name() for f in res.fields()], ['My Name', 'age', 'My Size'])

    def testWriteWithLongLongField(self):
        ml = QgsVectorLayer('NoGeometry?crs=epsg:4326&field=fldlonglong:long',
                            'test2', 'memory')
        provider = ml.dataProvider()
        feat = QgsFeature()
        feat.setAttributes([2262000000])
        provider.addFeatures([feat])

        filename = os.path.join(str(QDir.tempPath()), 'with_longlong_field')
        crs = QgsCoordinateReferenceSystem('EPSG:4326')
        rc, errmsg = QgsVectorFileWriter.writeAsVectorFormat(ml, filename, 'utf-8', crs, 'GPKG')

        # open the resulting geopackage
        vl = QgsVectorLayer(filename + '.gpkg', '', 'ogr')
        self.assertTrue(vl.isValid())

        # test values
        idx = vl.fields().indexFromName('fldlonglong')
        self.assertEqual(vl.getFeature(1).attributes()[idx], 2262000000)

        del vl
        os.unlink(filename + '.gpkg')

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
        crs = QgsCoordinateReferenceSystem('EPSG:4326')
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
        self.assertTrue(f.attributes()[time_idx].startswith('13:45:22'))
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
        crs = QgsCoordinateReferenceSystem('EPSG:4326')
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
            crs = QgsCoordinateReferenceSystem('EPSG:4326')
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
            crs = QgsCoordinateReferenceSystem('EPSG:4326')
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
        crs = QgsCoordinateReferenceSystem('EPSG:4326')
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
        crs = QgsCoordinateReferenceSystem('EPSG:4326')
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
        crs = QgsCoordinateReferenceSystem('EPSG:4326')
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
        self.assertTrue('PGDUMP' in [f.driverName for f in formats])

        interlis_format = [f for f in formats if f.driverName == 'Interlis 2'][0]
        self.assertEqual(interlis_format.globs, ['*.xtf', '*.xml', '*.ili'])

        # alphabetical sorting
        formats2 = QgsVectorFileWriter.supportedFiltersAndFormats(QgsVectorFileWriter.VectorFormatOptions())
        # print([f.filterString for f in formats2])
        self.assertTrue(formats2[0].filterString < formats2[1].filterString)
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
            if 'GeoJSON - Newline Delimited' in parts[i] or 'GeoJSON - Newline Delimited' in parts[i + 1]:
                # Python's < operator doesn't do locale aware sorting, so skip this problematic one
                continue
            self.assertLess(parts[i].lower(), parts[i + 1].lower())

        # alphabetical sorting
        formats2 = QgsVectorFileWriter.fileFilterString(QgsVectorFileWriter.VectorFormatOptions())
        self.assertNotEqual(formats.index('gpkg'), formats2.index('gpkg'))
        parts = formats2.split(';;')
        for i in range(len(parts) - 1):
            if 'GeoJSON - Newline Delimited' in parts[i] or 'GeoJSON - Newline Delimited' in parts[i + 1]:
                # Python's < operator doesn't do locale aware sorting, so skip this problematic one
                continue
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

    def testCreateDGN(self):
        ml = QgsVectorLayer('Point?crs=epsg:4326', 'test', 'memory')
        provider = ml.dataProvider()
        feat = QgsFeature()
        feat.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(10, 10)))
        provider.addFeatures([feat])

        filename = os.path.join(str(QDir.tempPath()), 'testCreateDGN.dgn')
        crs = QgsCoordinateReferenceSystem('EPSG:4326')
        rc, errmsg = QgsVectorFileWriter.writeAsVectorFormat(ml, filename, 'utf-8', crs, 'DGN')

        # open the resulting file
        vl = QgsVectorLayer(filename, '', 'ogr')
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.featureCount(), 1)
        del vl

        # append
        options = QgsVectorFileWriter.SaveVectorOptions()
        options.driverName = 'DGN'
        options.layerName = 'test'
        options.actionOnExistingFile = QgsVectorFileWriter.AppendToLayerNoNewFields
        write_result, error_message = QgsVectorFileWriter.writeAsVectorFormat(
            ml,
            filename,
            options)
        self.assertEqual(write_result, QgsVectorFileWriter.NoError, error_message)

        # open the resulting file
        vl = QgsVectorLayer(filename, '', 'ogr')
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.featureCount(), 2)
        del vl

        os.unlink(filename)

    def testAddZ(self):
        """Check adding z values to non z input."""
        input = QgsVectorLayer(
            'Point?crs=epsg:4326&field=name:string(20)',
            'test',
            'memory')

        self.assertTrue(input.isValid(), 'Provider not initialized')

        ft = QgsFeature()
        ft.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(10, 10)))
        myResult, myFeatures = input.dataProvider().addFeatures([ft])
        self.assertTrue(myResult)
        self.assertTrue(myFeatures)

        dest_file_name = os.path.join(str(QDir.tempPath()), 'add_z.geojson')
        options = QgsVectorFileWriter.SaveVectorOptions()
        options.overrideGeometryType = QgsWkbTypes.PointZ
        options.driverName = 'GeoJSON'
        write_result, error_message = QgsVectorFileWriter.writeAsVectorFormat(
            input,
            dest_file_name,
            options)
        self.assertEqual(write_result, QgsVectorFileWriter.NoError, error_message)

        # Open result and check
        created_layer = QgsVectorLayer(dest_file_name, 'test', 'ogr')
        self.assertTrue(created_layer.isValid())
        f = next(created_layer.getFeatures(QgsFeatureRequest()))
        self.assertEqual(f.geometry().asWkt(), 'PointZ (10 10 0)')

    def testDropZ(self):
        """Check dropping z values input."""
        input = QgsVectorLayer(
            'PointZ?crs=epsg:4326&field=name:string(20)',
            'test',
            'memory')

        self.assertTrue(input.isValid(), 'Provider not initialized')

        ft = QgsFeature()
        ft.setGeometry(QgsGeometry.fromWkt('PointM(10 10 2)'))
        myResult, myFeatures = input.dataProvider().addFeatures([ft])
        self.assertTrue(myResult)
        self.assertTrue(myFeatures)

        dest_file_name = os.path.join(str(QDir.tempPath()), 'drop_z.geojson')
        options = QgsVectorFileWriter.SaveVectorOptions()
        options.overrideGeometryType = QgsWkbTypes.PointM
        options.driverName = 'GeoJSON'
        write_result, error_message = QgsVectorFileWriter.writeAsVectorFormat(
            input,
            dest_file_name,
            options)
        self.assertEqual(write_result, QgsVectorFileWriter.NoError, error_message)

        # Open result and check
        created_layer = QgsVectorLayer(dest_file_name, 'test', 'ogr')
        self.assertTrue(created_layer.isValid())
        f = next(created_layer.getFeatures(QgsFeatureRequest()))
        self.assertEqual(f.geometry().asWkt(), 'Point (10 10)')

    def testWriteWithStringListField(self):
        """
        Test writing with a string list field
        :return:
        """
        source_fields = QgsFields()
        source_fields.append(QgsField('int', QVariant.Int))
        source_fields.append(QgsField('stringlist', QVariant.StringList, subType=QVariant.String))
        vl = QgsMemoryProviderUtils.createMemoryLayer('test', source_fields)
        f = QgsFeature()
        f.setAttributes([1, ['ab', 'cd']])
        vl.dataProvider().addFeature(f)

        filename = os.path.join(str(QDir.tempPath()), 'with_stringlist_field.geojson')
        rc, errmsg = QgsVectorFileWriter.writeAsVectorFormat(vl,
                                                             filename,
                                                             'utf-8',
                                                             vl.crs(),
                                                             'GeoJSON')

        self.assertEqual(rc, QgsVectorFileWriter.NoError)

        # open the resulting geojson
        vl = QgsVectorLayer(filename, '', 'ogr')
        self.assertTrue(vl.isValid())
        fields = vl.fields()

        # test type of converted field
        idx = fields.indexFromName('stringlist')
        self.assertEqual(fields.at(idx).type(), QVariant.StringList)
        self.assertEqual(fields.at(idx).subType(), QVariant.String)

        self.assertEqual([f.attributes() for f in vl.getFeatures()], [[1, ['ab', 'cd']]])

        os.unlink(filename)

    def testWriteWithIntegerListField(self):
        """
        Test writing with a integer list field
        :return:
        """
        source_fields = QgsFields()
        source_fields.append(QgsField('int', QVariant.Int))
        source_fields.append(QgsField('intlist', QVariant.List, subType=QVariant.Int))
        vl = QgsMemoryProviderUtils.createMemoryLayer('test', source_fields)
        f = QgsFeature()
        f.setAttributes([1, [11, 12]])
        vl.dataProvider().addFeature(f)

        filename = os.path.join(str(QDir.tempPath()), 'with_intlist_field.geojson')
        rc, errmsg = QgsVectorFileWriter.writeAsVectorFormat(vl,
                                                             filename,
                                                             'utf-8',
                                                             vl.crs(),
                                                             'GeoJSON')

        self.assertEqual(rc, QgsVectorFileWriter.NoError)

        # open the resulting geojson
        vl = QgsVectorLayer(filename, '', 'ogr')
        self.assertTrue(vl.isValid())
        fields = vl.fields()

        # test type of converted field
        idx = fields.indexFromName('intlist')
        self.assertEqual(fields.at(idx).type(), QVariant.List)
        self.assertEqual(fields.at(idx).subType(), QVariant.Int)

        self.assertEqual([f.attributes() for f in vl.getFeatures()], [[1, [11, 12]]])

        os.unlink(filename)

    def testWriteWithDoubleListField(self):
        """
        Test writing with a double list field
        :return:
        """
        source_fields = QgsFields()
        source_fields.append(QgsField('int', QVariant.Int))
        source_fields.append(QgsField('reallist', QVariant.List, subType=QVariant.Double))
        vl = QgsMemoryProviderUtils.createMemoryLayer('test', source_fields)
        f = QgsFeature()
        f.setAttributes([1, [11.1, 12.2]])
        vl.dataProvider().addFeature(f)

        filename = os.path.join(str(QDir.tempPath()), 'with_intlist_field.geojson')
        rc, errmsg = QgsVectorFileWriter.writeAsVectorFormat(vl,
                                                             filename,
                                                             'utf-8',
                                                             vl.crs(),
                                                             'GeoJSON')

        self.assertEqual(rc, QgsVectorFileWriter.NoError)

        # open the resulting geojson
        vl = QgsVectorLayer(filename, '', 'ogr')
        self.assertTrue(vl.isValid())
        fields = vl.fields()

        # test type of converted field
        idx = fields.indexFromName('reallist')
        self.assertEqual(fields.at(idx).type(), QVariant.List)
        self.assertEqual(fields.at(idx).subType(), QVariant.Double)

        self.assertEqual([f.attributes() for f in vl.getFeatures()], [[1, [11.1, 12.2]]])

        os.unlink(filename)

    def testWriteWithLongLongListField(self):
        """
        Test writing with a long long list field
        :return:
        """
        source_fields = QgsFields()
        source_fields.append(QgsField('int', QVariant.Int))
        source_fields.append(QgsField('int64list', QVariant.List, subType=QVariant.LongLong))
        vl = QgsMemoryProviderUtils.createMemoryLayer('test', source_fields)
        f = QgsFeature()
        f.setAttributes([1, [1234567890123, 1234567890124]])
        vl.dataProvider().addFeature(f)

        filename = os.path.join(str(QDir.tempPath()), 'with_longlist_field.geojson')
        rc, errmsg = QgsVectorFileWriter.writeAsVectorFormat(vl,
                                                             filename,
                                                             'utf-8',
                                                             vl.crs(),
                                                             'GeoJSON')

        self.assertEqual(rc, QgsVectorFileWriter.NoError)

        # open the resulting gml
        vl = QgsVectorLayer(filename, '', 'ogr')
        self.assertTrue(vl.isValid())
        fields = vl.fields()

        # test type of converted field
        idx = fields.indexFromName('int64list')
        self.assertEqual(fields.at(idx).type(), QVariant.List)
        self.assertEqual(fields.at(idx).subType(), QVariant.LongLong)

        self.assertEqual([f.attributes() for f in vl.getFeatures()], [[1, [1234567890123, 1234567890124]]])

        os.unlink(filename)

    def testWriteWithBinaryField(self):
        """
        Test writing with a binary field
        :return:
        """
        basetestpath = tempfile.mkdtemp()

        tmpfile = os.path.join(basetestpath, 'binaryfield.sqlite')
        ds = ogr.GetDriverByName('SQLite').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbPoint, options=['FID=fid'])
        lyr.CreateField(ogr.FieldDefn('strfield', ogr.OFTString))
        lyr.CreateField(ogr.FieldDefn('intfield', ogr.OFTInteger))
        lyr.CreateField(ogr.FieldDefn('binfield', ogr.OFTBinary))
        lyr.CreateField(ogr.FieldDefn('binfield2', ogr.OFTBinary))
        f = None
        ds = None

        vl = QgsVectorLayer(tmpfile)
        self.assertTrue(vl.isValid())

        # check that 1 of its fields is a bool
        fields = vl.fields()
        self.assertEqual(fields.at(fields.indexFromName('binfield')).type(), QVariant.ByteArray)

        dp = vl.dataProvider()
        f = QgsFeature(fields)
        bin_1 = b'xxx'
        bin_2 = b'yyy'
        bin_val1 = QByteArray(bin_1)
        bin_val2 = QByteArray(bin_2)
        f.setAttributes([1, 'str', 100, bin_val1, bin_val2])
        self.assertTrue(dp.addFeature(f))

        # write a gpkg package with a binary field
        filename = os.path.join(str(QDir.tempPath()), 'with_bin_field')
        rc, errmsg = QgsVectorFileWriter.writeAsVectorFormat(vl,
                                                             filename,
                                                             'utf-8',
                                                             vl.crs(),
                                                             'GPKG')

        self.assertEqual(rc, QgsVectorFileWriter.NoError)

        # open the resulting geopackage
        vl = QgsVectorLayer(filename + '.gpkg', '', 'ogr')
        self.assertTrue(vl.isValid())
        fields = vl.fields()

        # test type of converted field
        idx = fields.indexFromName('binfield')
        self.assertEqual(fields.at(idx).type(), QVariant.ByteArray)
        idx2 = fields.indexFromName('binfield2')
        self.assertEqual(fields.at(idx2).type(), QVariant.ByteArray)

        # test values
        self.assertEqual(vl.getFeature(1).attributes()[idx], bin_val1)
        self.assertEqual(vl.getFeature(1).attributes()[idx2], bin_val2)

        del vl
        os.unlink(filename + '.gpkg')

    def testWriteKMLAxisOrderIssueGDAL3(self):
        """Check axis order issue when writing KML with EPSG:4326."""

        if not ogr.GetDriverByName('KML'):
            return

        vl = QgsVectorLayer(
            'PointZ?crs=epsg:4326&field=name:string(20)',
            'test',
            'memory')

        self.assertTrue(vl.isValid(), 'Provider not initialized')

        ft = QgsFeature()
        ft.setGeometry(QgsGeometry.fromWkt('Point(2 49)'))
        myResult, myFeatures = vl.dataProvider().addFeatures([ft])
        self.assertTrue(myResult)
        self.assertTrue(myFeatures)

        dest_file_name = os.path.join(str(QDir.tempPath()), 'testWriteKMLAxisOrderIssueGDAL3.kml')
        write_result, error_message = QgsVectorFileWriter.writeAsVectorFormat(
            vl,
            dest_file_name,
            'utf-8',
            vl.crs(),
            'KML')
        self.assertEqual(write_result, QgsVectorFileWriter.NoError, error_message)

        # Open result and check
        created_layer = QgsVectorLayer(dest_file_name, 'test', 'ogr')
        self.assertTrue(created_layer.isValid())
        f = next(created_layer.getFeatures(QgsFeatureRequest()))
        self.assertEqual(f.geometry().asWkt(), 'PointZ (2 49 0)')

    def testWriteGpkgWithFID(self):
        """Check writing a memory layer with a FID column takes it as FID"""

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

        dest_file_name = os.path.join(str(QDir.tempPath()), 'testWriteGpkgWithFID.gpkg')
        write_result, error_message = QgsVectorFileWriter.writeAsVectorFormat(
            vl,
            dest_file_name,
            'utf-8',
            vl.crs(),
            'GPKG')
        self.assertEqual(write_result, QgsVectorFileWriter.NoError, error_message)

        # Open result and check
        created_layer = QgsVectorLayer(dest_file_name, 'test', 'ogr')
        self.assertTrue(created_layer.isValid())
        f = next(created_layer.getFeatures(QgsFeatureRequest()))
        self.assertEqual(f.geometry().asWkt(), 'Point (2 49)')
        self.assertEqual(f.attributes(), [123, 'text1'])
        self.assertEqual(f.id(), 123)

    def testWriteTriangle(self):
        """Check we can write geometries with triangle types."""
        layer = QgsVectorLayer(
            ('MultiPolygonZ?crs=epsg:4326&field=name:string(20)'),
            'test',
            'memory')

        ft = QgsFeature()
        geom = QgsMultiPolygon()
        geom.addGeometry(QgsTriangle(QgsPoint(1, 2, 3), QgsPoint(2, 2, 4), QgsPoint(2, 3, 4)))
        ft.setGeometry(geom)
        ft.setAttributes(['Johny'])
        myResult, myFeatures = layer.dataProvider().addFeatures([ft])
        self.assertTrue(myResult)
        self.assertTrue(myFeatures)

        dest_file_name = os.path.join(str(QDir.tempPath()), 'testWriteTriangle.gpkg')
        write_result, error_message = QgsVectorFileWriter.writeAsVectorFormat(
            layer,
            dest_file_name,
            'utf-8',
            layer.crs(),
            'GPKG')
        self.assertEqual(write_result, QgsVectorFileWriter.NoError, error_message)

        # Open result and check
        created_layer = QgsVectorLayer(dest_file_name, 'test', 'ogr')
        self.assertTrue(created_layer.isValid())
        f = next(created_layer.getFeatures(QgsFeatureRequest()))
        self.assertEqual(f.geometry().asWkt(), 'MultiPolygonZ (((1 2 3, 2 2 4, 2 3 4, 1 2 3)))')
        self.assertEqual(f.attributes(), [1, 'Johny'])

    def testWriteConversionErrors(self):
        """Test writing features with attribute values that cannot be
        converted to the destination fields.
        See: GH #36715"""

        vl = QgsVectorLayer('Point?crs=epsg:4326&field=int:integer', 'test', 'memory')
        self.assertTrue(vl.startEditing())
        f = QgsFeature(vl.fields())
        f.setGeometry(QgsGeometry.fromWkt('point(9 45)'))
        f.setAttribute(0, 'QGIS Rocks!')  # not valid!
        self.assertTrue(vl.addFeatures([f]))
        f.setAttribute(0, 12345)  # valid!
        self.assertTrue(vl.addFeatures([f]))

        dest_file_name = os.path.join(str(QDir.tempPath()), 'writer_conversion_errors.shp')
        write_result, error_message = QgsVectorFileWriter.writeAsVectorFormat(
            vl,
            dest_file_name,
            'utf-8',
            QgsCoordinateReferenceSystem(),
            'ESRI Shapefile')
        self.assertEqual(write_result, QgsVectorFileWriter.ErrFeatureWriteFailed, error_message)

        # Open result and check
        created_layer = QgsVectorLayer('{}|layerid=0'.format(dest_file_name), 'test', 'ogr')
        self.assertEqual(created_layer.fields().count(), 1)
        self.assertEqual(created_layer.featureCount(), 1)
        f = next(created_layer.getFeatures(QgsFeatureRequest()))
        self.assertEqual(f['int'], 12345)

    def test_regression_37386(self):
        """Test issue GH #37386"""

        dest_file_name = os.path.join(str(QDir.tempPath()), 'writer_regression_37386.gpkg')
        fields = QgsFields()
        fields.append(QgsField("note", QVariant.Double))
        lyrname = "test1"
        opts = QgsVectorFileWriter.SaveVectorOptions()
        opts.driverName = "GPKG"
        opts.layerName = lyrname
        opts.actionOnExistingFile = QgsVectorFileWriter.CreateOrOverwriteFile
        writer = QgsVectorFileWriter.create(dest_file_name, fields, QgsWkbTypes.Point, QgsCoordinateReferenceSystem.fromEpsgId(4326), QgsCoordinateTransformContext(), opts, QgsFeatureSink.SinkFlags(), None, lyrname)
        self.assertEqual(writer.hasError(), QgsVectorFileWriter.NoError)
        del writer
        vl = QgsVectorLayer(dest_file_name)
        self.assertTrue(vl.isValid())

    def testPersistMetadata(self):
        """
        Test that metadata from the source layer is saved as default for the destination if the
        persist metadat option is enabled
        """
        vl = QgsVectorLayer('Point?crs=epsg:4326&field=int:integer', 'test', 'memory')
        self.assertTrue(vl.startEditing())
        f = QgsFeature(vl.fields())
        f.setGeometry(QgsGeometry.fromWkt('point(9 45)'))
        f.setAttribute(0, 'QGIS Rocks!')  # not valid!
        self.assertTrue(vl.addFeatures([f]))
        f.setAttribute(0, 12345)  # valid!
        self.assertTrue(vl.addFeatures([f]))

        # set some metadata on the source layer
        metadata = QgsLayerMetadata()
        metadata.setTitle('my title')
        metadata.setAbstract('my abstract')
        metadata.setLicenses(['l1', 'l2'])

        dest_file_name = os.path.join(str(QDir.tempPath()), 'save_metadata.gpkg')

        options = QgsVectorFileWriter.SaveVectorOptions()
        options.driverName = 'GPKG'
        options.layerName = 'test'
        options.saveMetadata = True
        options.layerMetadata = metadata

        write_result, error_message, new_file, new_layer = QgsVectorFileWriter.writeAsVectorFormatV3(
            vl,
            dest_file_name,
            QgsProject.instance().transformContext(),
            options)
        self.assertEqual(write_result, QgsVectorFileWriter.ErrFeatureWriteFailed, error_message)

        # Open result and check
        created_layer = QgsVectorLayer(f'{new_file}|layerName={new_layer}', 'test', 'ogr')
        self.assertTrue(created_layer.isValid())

        # layer should have metadata stored
        self.assertEqual(created_layer.metadata().title(), 'my title')
        self.assertEqual(created_layer.metadata().abstract(), 'my abstract')
        self.assertEqual(created_layer.metadata().licenses(), ['l1', 'l2'])

    @unittest.skipIf(int(gdal.VersionInfo('VERSION_NUM')) < GDAL_COMPUTE_VERSION(3, 4, 0), "GDAL 3.4 required")
    def testWriteWithCoordinateEpoch(self):
        """
        Write a dataset with a coordinate epoch to geopackage
        """
        layer = QgsVectorLayer(
            ('Point?crs=epsg:4326&field=name:string(20)&'
             'field=age:integer&field=size:double&index=yes'),
            'test',
            'memory')

        self.assertTrue(layer.isValid())

        crs = QgsCoordinateReferenceSystem('EPSG:4326')
        crs.setCoordinateEpoch(2020.7)
        layer.setCrs(crs)

        ft = QgsFeature()
        ft.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(10, 10)))
        ft.setAttributes(['Johny', 20, 0.3])
        myResult, myFeatures = layer.dataProvider().addFeatures([ft])
        self.assertTrue(myResult)
        self.assertTrue(myFeatures)

        dest_file_name = os.path.join(str(QDir.tempPath()), 'writer_coordinate_epoch.gpkg')
        options = QgsVectorFileWriter.SaveVectorOptions()
        options.driverName = 'GPKG'
        options.layerName = 'test'

        write_result, error_message, new_file, new_layer = QgsVectorFileWriter.writeAsVectorFormatV3(
            layer,
            dest_file_name,
            QgsProject.instance().transformContext(),
            options)
        self.assertEqual(write_result, QgsVectorFileWriter.NoError, error_message)

        # check that coordinate epoch was written to file
        vl = QgsVectorLayer(dest_file_name)
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.crs().coordinateEpoch(), 2020.7)

    def testAddingToOpenedGkg(self):
        """ Test scenario of https://github.com/qgis/QGIS/issues/48154 """

        tmp_dir = QTemporaryDir()
        tmpfile = os.path.join(tmp_dir.path(), 'testAddingToOpenedGkg.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbPoint)
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetGeometry(ogr.CreateGeometryFromWkt('POINT(0 0)'))
        lyr.CreateFeature(f)
        del (lyr)
        del (ds)

        vl = QgsVectorLayer(f'{tmpfile}|layername=test', 'test', 'ogr')
        self.assertTrue(vl.isValid())

        # Test CreateOrOverwriteLayer
        ml = QgsVectorLayer('Point?field=firstfield:int', 'test', 'memory')
        provider = ml.dataProvider()
        ft = QgsFeature()
        ft.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(10, 10)))
        ft.setAttributes([2])
        provider.addFeatures([ft])

        options = QgsVectorFileWriter.SaveVectorOptions()
        options.driverName = 'GPKG'
        options.layerName = 'test2'
        options.actionOnExistingFile = QgsVectorFileWriter.CreateOrOverwriteLayer
        write_result, error_message = QgsVectorFileWriter.writeAsVectorFormat(
            ml,
            tmpfile,
            options)
        self.assertEqual(write_result, QgsVectorFileWriter.NoError, error_message)

        # Check that we can open the layer
        vl2 = QgsVectorLayer(f'{tmpfile}|layername=test2', 'test', 'ogr')
        self.assertTrue(vl2.isValid())

    def testWriteUnsetAttributeToShapefile(self):
        """ Test writing an unset attribute to a shapefile """

        vl = QgsVectorLayer('Point?crs=epsg:4326&field=int:integer', 'test', 'memory')
        self.assertTrue(vl.startEditing())
        f = QgsFeature(vl.fields())
        f.setGeometry(QgsGeometry.fromWkt('point(9 45)'))
        f.setAttribute(0, QgsUnsetAttributeValue('Autonumber'))
        self.assertTrue(vl.addFeatures([f]))
        f.setAttribute(0, 12345)
        self.assertTrue(vl.addFeatures([f]))

        dest_file_name = os.path.join(str(QDir.tempPath()), 'writing_unset_values.shp')
        write_result, error_message = QgsVectorFileWriter.writeAsVectorFormat(
            vl,
            dest_file_name,
            'utf-8',
            QgsCoordinateReferenceSystem(),
            'ESRI Shapefile')
        self.assertEqual(write_result, QgsVectorFileWriter.NoError, error_message)

        # Open result and check
        created_layer = QgsVectorLayer(dest_file_name, 'test', 'ogr')
        self.assertEqual(created_layer.fields().count(), 1)
        self.assertEqual(created_layer.featureCount(), 2)
        features = created_layer.getFeatures(QgsFeatureRequest())
        f = next(features)
        self.assertEqual(f['int'], NULL)
        f = next(features)
        self.assertEqual(f['int'], 12345)

    def testWriteUnsetAttributeToGpkg(self):
        """ Test writing an unset attribute to a gpkg """

        vl = QgsVectorLayer('Point?crs=epsg:4326&field=int:integer', 'test', 'memory')
        self.assertTrue(vl.startEditing())
        f = QgsFeature(vl.fields())
        f.setGeometry(QgsGeometry.fromWkt('point(9 45)'))
        f.setAttribute(0, QgsUnsetAttributeValue('Autonumber'))
        self.assertTrue(vl.addFeatures([f]))
        f.setAttribute(0, 12345)
        self.assertTrue(vl.addFeatures([f]))

        dest_file_name = os.path.join(str(QDir.tempPath()), 'writing_unset_values.gpkg')
        write_result, error_message = QgsVectorFileWriter.writeAsVectorFormat(
            vl,
            dest_file_name,
            'utf-8',
            QgsCoordinateReferenceSystem(),
            'GPKG')
        self.assertEqual(write_result, QgsVectorFileWriter.NoError, error_message)

        # Open result and check
        created_layer = QgsVectorLayer(dest_file_name, 'test', 'ogr')
        self.assertEqual(created_layer.fields().count(), 2)
        self.assertEqual(created_layer.featureCount(), 2)
        features = created_layer.getFeatures(QgsFeatureRequest())
        f = next(features)
        self.assertEqual(f['int'], NULL)
        f = next(features)
        self.assertEqual(f['int'], 12345)


if __name__ == '__main__':
    unittest.main()
