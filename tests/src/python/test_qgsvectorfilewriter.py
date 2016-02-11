# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsVectorFileWriter.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Tim Sutton'
__date__ = '20/08/2012'
__copyright__ = 'Copyright 2012, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis

from qgis.core import (QgsVectorLayer,
                       QgsFeature,
                       QgsGeometry,
                       QgsPoint,
                       QgsCoordinateReferenceSystem,
                       QgsVectorFileWriter,
                       QgsFeatureRequest
                       )
from PyQt4.QtCore import QDate, QTime, QDateTime, QVariant, QDir
import os
from qgis.testing import (
    start_app,
    unittest
)

from utilities import writeShape

start_app()


class TestQgsVectorLayer(unittest.TestCase):

    mMemoryLayer = None

    def testWrite(self):
        """Check we can write a vector file."""
        self.mMemoryLayer = QgsVectorLayer(
            ('Point?crs=epsg:4326&field=name:string(20)&'
             'field=age:integer&field=size:double&index=yes'),
            'test',
            'memory')

        assert self.mMemoryLayer is not None, 'Provider not initialised'
        myProvider = self.mMemoryLayer.dataProvider()
        assert myProvider is not None

        ft = QgsFeature()
        ft.setGeometry(QgsGeometry.fromPoint(QgsPoint(10, 10)))
        ft.setAttributes(['Johny', 20, 0.3])
        myResult, myFeatures = myProvider.addFeatures([ft])
        assert myResult
        assert len(myFeatures) > 0

        writeShape(self.mMemoryLayer, 'writetest.shp')

    def testDateTimeWriteShapefile(self):
        """Check writing date and time fields to an ESRI shapefile."""
        ml = QgsVectorLayer(
            ('Point?crs=epsg:4326&field=id:int&'
             'field=date_f:date&field=time_f:time&field=dt_f:datetime'),
            'test',
            'memory')

        assert ml is not None, 'Provider not initialised'
        assert ml.isValid(), 'Source layer not valid'
        provider = ml.dataProvider()
        assert provider is not None

        ft = QgsFeature()
        ft.setGeometry(QgsGeometry.fromPoint(QgsPoint(10, 10)))
        ft.setAttributes([1, QDate(2014, 3, 5), QTime(13, 45, 22), QDateTime(QDate(2014, 3, 5), QTime(13, 45, 22))])
        res, features = provider.addFeatures([ft])
        assert res
        assert len(features) > 0

        dest_file_name = os.path.join(str(QDir.tempPath()), 'datetime.shp')
        print(dest_file_name)
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
        #shapefiles do not support datetime types, result should be date
        self.assertEqual(fields.at(fields.indexFromName('dt_f')).type(), QVariant.Date)

        f = created_layer.getFeatures(QgsFeatureRequest()).next()

        date_idx = created_layer.fieldNameIndex('date_f')
        assert isinstance(f.attributes()[date_idx], QDate)
        self.assertEqual(f.attributes()[date_idx], QDate(2014, 3, 5))
        time_idx = created_layer.fieldNameIndex('time_f')
        #shapefiles do not support time types
        assert isinstance(f.attributes()[time_idx], basestring)
        self.assertEqual(f.attributes()[time_idx], '13:45:22')
        #shapefiles do not support datetime types
        datetime_idx = created_layer.fieldNameIndex('dt_f')
        assert isinstance(f.attributes()[datetime_idx], QDate)
        self.assertEqual(f.attributes()[datetime_idx], QDate(2014, 3, 5))

    def testDateTimeWriteTabfile(self):
        """Check writing date and time fields to an MapInfo tabfile."""
        ml = QgsVectorLayer(
            ('Point?crs=epsg:4326&field=id:int&'
             'field=date_f:date&field=time_f:time&field=dt_f:datetime'),
            'test',
            'memory')

        assert ml is not None, 'Provider not initialised'
        assert ml.isValid(), 'Source layer not valid'
        provider = ml.dataProvider()
        assert provider is not None

        ft = QgsFeature()
        ft.setGeometry(QgsGeometry.fromPoint(QgsPoint(10, 10)))
        ft.setAttributes([1, QDate(2014, 3, 5), QTime(13, 45, 22), QDateTime(QDate(2014, 3, 5), QTime(13, 45, 22))])
        res, features = provider.addFeatures([ft])
        assert res
        assert len(features) > 0

        dest_file_name = os.path.join(str(QDir.tempPath()), 'datetime.tab')
        print(dest_file_name)
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

        f = created_layer.getFeatures(QgsFeatureRequest()).next()

        date_idx = created_layer.fieldNameIndex('date_f')
        assert isinstance(f.attributes()[date_idx], QDate)
        self.assertEqual(f.attributes()[date_idx], QDate(2014, 3, 5))
        time_idx = created_layer.fieldNameIndex('time_f')
        assert isinstance(f.attributes()[time_idx], QTime)
        self.assertEqual(f.attributes()[time_idx], QTime(13, 45, 22))
        datetime_idx = created_layer.fieldNameIndex('dt_f')
        assert isinstance(f.attributes()[datetime_idx], QDateTime)
        self.assertEqual(f.attributes()[datetime_idx], QDateTime(QDate(2014, 3, 5), QTime(13, 45, 22)))

if __name__ == '__main__':
    unittest.main()
