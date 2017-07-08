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
                       QgsPoint,
                       QgsCoordinateReferenceSystem,
                       QgsVectorFileWriter,
                       QgsFeatureRequest,
                       QgsWKBTypes,
                       QgsRectangle,
                       QgsCoordinateTransform
                       )
from qgis.PyQt.QtCore import QDate, QTime, QDateTime, QVariant, QDir
import os
import osgeo.gdal
from osgeo import gdal, ogr
import platform
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
        ft.setGeometry(QgsGeometry.fromPoint(QgsPoint(10, 10)))
        ft.setAttributes(['Johny', 20, 0.3])
        myResult, myFeatures = myProvider.addFeatures([ft])
        self.assertTrue(myResult)
        self.assertTrue(myFeatures)

        writeShape(self.mMemoryLayer, 'writetest.shp')

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
        ft.setGeometry(QgsGeometry.fromPoint(QgsPoint(10, 10)))
        ft.setAttributes([1, QDate(2014, 3, 5), QTime(13, 45, 22), QDateTime(QDate(2014, 3, 5), QTime(13, 45, 22))])
        res, features = provider.addFeatures([ft])
        self.assertTrue(res)
        self.assertTrue(features)

        dest_file_name = os.path.join(str(QDir.tempPath()), 'datetime.shp')
        crs = QgsCoordinateReferenceSystem()
        crs.createFromId(4326, QgsCoordinateReferenceSystem.EpsgCrsId)
        write_result = QgsVectorFileWriter.writeAsVectorFormat(
            ml,
            dest_file_name,
            'utf-8',
            crs,
            'ESRI Shapefile')
        self.assertEqual(write_result, QgsVectorFileWriter.NoError)

        # Open result and check
        created_layer = QgsVectorLayer(u'{}|layerid=0'.format(dest_file_name), u'test', u'ogr')

        fields = created_layer.dataProvider().fields()
        self.assertEqual(fields.at(fields.indexFromName('date_f')).type(), QVariant.Date)
        #shapefiles do not support time types, result should be string
        self.assertEqual(fields.at(fields.indexFromName('time_f')).type(), QVariant.String)
        #shapefiles do not support datetime types, result should be string
        self.assertEqual(fields.at(fields.indexFromName('dt_f')).type(), QVariant.String)

        f = next(created_layer.getFeatures(QgsFeatureRequest()))

        date_idx = created_layer.fieldNameIndex('date_f')
        self.assertIsInstance(f.attributes()[date_idx], QDate)
        self.assertEqual(f.attributes()[date_idx], QDate(2014, 3, 5))
        time_idx = created_layer.fieldNameIndex('time_f')
        #shapefiles do not support time types
        self.assertIsInstance(f.attributes()[time_idx], str)
        self.assertEqual(f.attributes()[time_idx], '13:45:22')
        #shapefiles do not support datetime types
        datetime_idx = created_layer.fieldNameIndex('dt_f')
        self.assertIsInstance(f.attributes()[datetime_idx], str)
        self.assertEqual(f.attributes()[datetime_idx], QDateTime(QDate(2014, 3, 5), QTime(13, 45, 22)).toString("yyyy/MM/dd hh:mm:ss.zzz"))

    def testWriterWithExtent(self):
        """Check writing using extent filter."""
        source_file = os.path.join(TEST_DATA_DIR, 'points.shp')
        source_layer = QgsVectorLayer(source_file, 'Points', 'ogr')
        self.assertTrue(source_layer.isValid())

        options = QgsVectorFileWriter.SaveVectorOptions()
        options.driverName = 'ESRI Shapefile'
        options.filterExtent = QgsRectangle(-111, 26, -96, 38)

        dest_file_name = os.path.join(str(QDir.tempPath()), 'extent_no_transform.shp')
        write_result = QgsVectorFileWriter.writeAsVectorFormat(
            source_layer,
            dest_file_name,
            options)
        self.assertEqual(write_result, QgsVectorFileWriter.NoError)

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
        crs = QgsCoordinateReferenceSystem()
        self.assertTrue(crs.createFromOgcWmsCrs('EPSG:3785'))
        ct = QgsCoordinateTransform(source_layer.crs(), crs)
        options.ct = ct

        dest_file_name = os.path.join(str(QDir.tempPath()), 'extent_transform.shp')
        write_result = QgsVectorFileWriter.writeAsVectorFormat(
            source_layer,
            dest_file_name,
            options)
        self.assertEqual(write_result, QgsVectorFileWriter.NoError)

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
        ft.setGeometry(QgsGeometry.fromPoint(QgsPoint(10, 10)))
        ft.setAttributes([1, QDate(2014, 3, 5), QTime(13, 45, 22), QDateTime(QDate(2014, 3, 5), QTime(13, 45, 22))])
        res, features = provider.addFeatures([ft])
        self.assertTrue(res)
        self.assertTrue(features)

        dest_file_name = os.path.join(str(QDir.tempPath()), 'datetime.tab')
        crs = QgsCoordinateReferenceSystem()
        crs.createFromId(4326, QgsCoordinateReferenceSystem.EpsgCrsId)
        write_result = QgsVectorFileWriter.writeAsVectorFormat(
            ml,
            dest_file_name,
            'utf-8',
            crs,
            'MapInfo File')
        self.assertEqual(write_result, QgsVectorFileWriter.NoError)

        # Open result and check
        created_layer = QgsVectorLayer(u'{}|layerid=0'.format(dest_file_name), u'test', u'ogr')

        fields = created_layer.dataProvider().fields()
        self.assertEqual(fields.at(fields.indexFromName('date_f')).type(), QVariant.Date)
        self.assertEqual(fields.at(fields.indexFromName('time_f')).type(), QVariant.Time)
        self.assertEqual(fields.at(fields.indexFromName('dt_f')).type(), QVariant.DateTime)

        f = next(created_layer.getFeatures(QgsFeatureRequest()))

        date_idx = created_layer.fieldNameIndex('date_f')
        self.assertIsInstance(f.attributes()[date_idx], QDate)
        self.assertEqual(f.attributes()[date_idx], QDate(2014, 3, 5))
        time_idx = created_layer.fieldNameIndex('time_f')
        self.assertIsInstance(f.attributes()[time_idx], QTime)
        self.assertEqual(f.attributes()[time_idx], QTime(13, 45, 22))
        datetime_idx = created_layer.fieldNameIndex('dt_f')
        self.assertIsInstance(f.attributes()[datetime_idx], QDateTime)
        self.assertEqual(f.attributes()[datetime_idx], QDateTime(QDate(2014, 3, 5), QTime(13, 45, 22)))

    def testWriteShapefileWithZ(self):
        """Check writing geometries with Z dimension to an ESRI shapefile."""

        #start by saving a memory layer and forcing z
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
        for t in [QgsWKBTypes.PointZ, QgsWKBTypes.Point25D]:
            dest_file_name = os.path.join(str(QDir.tempPath()), 'point_{}.shp'.format(QgsWKBTypes.displayString(t)))
            crs = QgsCoordinateReferenceSystem()
            crs.createFromId(4326, QgsCoordinateReferenceSystem.EpsgCrsId)
            write_result = QgsVectorFileWriter.writeAsVectorFormat(
                ml,
                dest_file_name,
                'utf-8',
                crs,
                'ESRI Shapefile',
                overrideGeometryType=t)
            self.assertEqual(write_result, QgsVectorFileWriter.NoError)

            # Open result and check
            created_layer = QgsVectorLayer(u'{}|layerid=0'.format(dest_file_name), u'test', u'ogr')
            f = next(created_layer.getFeatures(QgsFeatureRequest()))
            g = f.geometry()
            wkt = g.exportToWkt()
            expWkt = 'PointZ (1 2 3)'
            self.assertTrue(compareWkt(expWkt, wkt), "saving geometry with Z failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt))

            #also try saving out the shapefile version again, as an extra test
            #this tests that saving a layer with z WITHOUT explicitly telling the writer to keep z values,
            #will stay retain the z values
            dest_file_name = os.path.join(str(QDir.tempPath()), 'point_{}_copy.shp'.format(QgsWKBTypes.displayString(t)))
            crs = QgsCoordinateReferenceSystem()
            crs.createFromId(4326, QgsCoordinateReferenceSystem.EpsgCrsId)
            write_result = QgsVectorFileWriter.writeAsVectorFormat(
                created_layer,
                dest_file_name,
                'utf-8',
                crs,
                'ESRI Shapefile')
            self.assertEqual(write_result, QgsVectorFileWriter.NoError)

            # Open result and check
            created_layer_from_shp = QgsVectorLayer(u'{}|layerid=0'.format(dest_file_name), u'test', u'ogr')
            f = next(created_layer_from_shp.getFeatures(QgsFeatureRequest()))
            g = f.geometry()
            wkt = g.exportToWkt()
            self.assertTrue(compareWkt(expWkt, wkt), "saving geometry with Z failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt))

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
        write_result = QgsVectorFileWriter.writeAsVectorFormat(
            ml,
            dest_file_name,
            'utf-8',
            crs,
            'ESRI Shapefile',
            forceMulti=True)
        self.assertEqual(write_result, QgsVectorFileWriter.NoError)

        # Open result and check
        created_layer = QgsVectorLayer(u'{}|layerid=0'.format(dest_file_name), u'test', u'ogr')
        f = next(created_layer.getFeatures(QgsFeatureRequest()))
        g = f.geometry()
        wkt = g.exportToWkt()
        expWkt = 'MultiPoint ((1 2))'
        self.assertTrue(compareWkt(expWkt, wkt), "saving geometry with multi conversion failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt))

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

        #first write out with all attributes
        dest_file_name = os.path.join(str(QDir.tempPath()), 'all_attributes.shp')
        crs = QgsCoordinateReferenceSystem()
        crs.createFromId(4326, QgsCoordinateReferenceSystem.EpsgCrsId)
        write_result = QgsVectorFileWriter.writeAsVectorFormat(
            ml,
            dest_file_name,
            'utf-8',
            crs,
            'ESRI Shapefile',
            attributes=[])
        self.assertEqual(write_result, QgsVectorFileWriter.NoError)

        # Open result and check
        created_layer = QgsVectorLayer(u'{}|layerid=0'.format(dest_file_name), u'test', u'ogr')
        self.assertEqual(created_layer.fields().count(), 4)
        f = next(created_layer.getFeatures(QgsFeatureRequest()))
        self.assertEqual(f['id'], 1)
        self.assertEqual(f['field1'], 11)
        self.assertEqual(f['field2'], 12)
        self.assertEqual(f['field3'], 13)

        #now test writing out only a subset of attributes
        dest_file_name = os.path.join(str(QDir.tempPath()), 'subset_attributes.shp')
        write_result = QgsVectorFileWriter.writeAsVectorFormat(
            ml,
            dest_file_name,
            'utf-8',
            crs,
            'ESRI Shapefile',
            attributes=[1, 3])
        self.assertEqual(write_result, QgsVectorFileWriter.NoError)

        # Open result and check
        created_layer = QgsVectorLayer(u'{}|layerid=0'.format(dest_file_name), u'test', u'ogr')
        self.assertEqual(created_layer.fields().count(), 2)
        f = next(created_layer.getFeatures(QgsFeatureRequest()))
        self.assertEqual(f['field1'], 11)
        self.assertEqual(f['field3'], 13)

        #finally test writing no attributes
        dest_file_name = os.path.join(str(QDir.tempPath()), 'no_attributes.shp')
        write_result = QgsVectorFileWriter.writeAsVectorFormat(
            ml,
            dest_file_name,
            'utf-8',
            crs,
            'ESRI Shapefile',
            skipAttributeCreation=True)
        self.assertEqual(write_result, QgsVectorFileWriter.NoError)

        # Open result and check
        created_layer = QgsVectorLayer(u'{}|layerid=0'.format(dest_file_name), u'test', u'ogr')
        # expect only a default 'FID' field for shapefiles
        self.assertEqual(created_layer.fields().count(), 1)
        self.assertEqual(created_layer.fields()[0].name(), 'FID')
        # in this case we also check that the geometry exists, to make sure feature has been correctly written
        # even without attributes
        f = next(created_layer.getFeatures(QgsFeatureRequest()))
        g = f.geometry()
        wkt = g.exportToWkt()
        expWkt = 'Point (1 2)'
        self.assertTrue(compareWkt(expWkt, wkt), "geometry not saved correctly when saving without attributes : mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt))
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
        write_result = QgsVectorFileWriter.writeAsVectorFormat(
            ml,
            dest_file_name,
            'utf-8',
            QgsCoordinateReferenceSystem(),
            'ESRI Shapefile',
            attributes=[0, 2],
            fieldValueConverter=converter)
        self.assertEqual(write_result, QgsVectorFileWriter.NoError)

        # Open result and check
        created_layer = QgsVectorLayer(u'{}|layerid=0'.format(dest_file_name), u'test', u'ogr')
        self.assertEqual(created_layer.fields().count(), 2)
        f = next(created_layer.getFeatures(QgsFeatureRequest()))
        self.assertEqual(f['nonconv'], 1)
        self.assertEqual(f['conv_attr'], 'converted_val')

    @unittest.expectedFailure(int(osgeo.gdal.VersionInfo('VERSION_NUM')) < GDAL_COMPUTE_VERSION(2, 0, 0))
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
        write_result = QgsVectorFileWriter.writeAsVectorFormat(
            ml,
            dest_file_name,
            'utf-8',
            crs,
            'MapInfo File')
        self.assertEqual(write_result, QgsVectorFileWriter.NoError)

        # Open result and check
        created_layer = QgsVectorLayer(u'{}|layerid=0'.format(dest_file_name), u'test', u'ogr')

        fields = created_layer.dataProvider().fields()
        self.assertEqual(fields.at(fields.indexFromName('int8')).type(), QVariant.Double)

        f = next(created_layer.getFeatures(QgsFeatureRequest()))

        int8_idx = created_layer.fieldNameIndex('int8')
        self.assertEqual(f.attributes()[int8_idx], 2123456789)

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
        write_result = QgsVectorFileWriter.writeAsVectorFormat(
            ml,
            filename,
            options)
        self.assertEqual(write_result, QgsVectorFileWriter.NoError)

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
        write_result = QgsVectorFileWriter.writeAsVectorFormat(
            ml,
            filename,
            options)
        self.assertEqual(write_result, QgsVectorFileWriter.NoError)

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
        write_result = QgsVectorFileWriter.writeAsVectorFormat(
            ml,
            filename,
            options)
        self.assertEqual(write_result, QgsVectorFileWriter.NoError)

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
        write_result = QgsVectorFileWriter.writeAsVectorFormat(
            ml,
            filename,
            options)
        self.assertEqual(write_result, QgsVectorFileWriter.NoError)

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
        write_result = QgsVectorFileWriter.writeAsVectorFormat(
            ml,
            filename,
            options)
        self.assertEqual(write_result, QgsVectorFileWriter.NoError)

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


if __name__ == '__main__':
    unittest.main()
