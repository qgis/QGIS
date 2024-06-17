"""QGIS Unit tests for QgsFeature.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Germán Carrillo'
__date__ = '06/10/2012'
__copyright__ = 'Copyright 2012, The QGIS Project'

import os

from qgis.core import (
    NULL,
    QgsFeature,
    QgsField,
    QgsFields,
    QgsGeometry,
    QgsPoint,
    QgsPointXY,
    QgsUnsetAttributeValue,
    QgsVectorLayer,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from qgis.PyQt.QtCore import QVariant, QDate, QTime, QDateTime

from utilities import unitTestDataPath

start_app()


class TestQgsFeature(QgisTestCase):

    def test_CreateFeature(self):
        feat = QgsFeature(0)
        feat.initAttributes(1)
        feat.setAttribute(0, "text")
        feat.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(123, 456)))
        myId = feat.id()
        myExpectedId = 0
        myMessage = f'\nExpected: {myExpectedId}\nGot: {myId}'
        assert myId == myExpectedId, myMessage

    def test_FeatureDefaultConstructor(self):
        """Test for FID_IS_NULL default constructors See: https://github.com/qgis/QGIS/issues/36962"""
        feat = QgsFeature()
        # it should be FID_NULL std::numeric_limits<QgsFeatureId>::min(),
        # not sure if I can test the exact value in python
        self.assertNotEqual(feat.id(), 0)
        self.assertLess(feat.id(), 0)

        feat = QgsFeature(QgsFields())
        self.assertNotEqual(feat.id(), 0)
        self.assertLess(feat.id(), 0)

        feat = QgsFeature(1234)
        self.assertEqual(feat.id(), 1234)

        feat = QgsFeature(QgsFields(), 1234)
        self.assertEqual(feat.id(), 1234)

    def test_equality(self):
        fields = QgsFields()
        field1 = QgsField('my_field')
        fields.append(field1)
        field2 = QgsField('my_field2')
        fields.append(field2)

        feat = QgsFeature(fields, 0)
        feat.initAttributes(1)
        feat.setAttribute(0, "text")
        feat.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(123, 456)))

        self.assertNotEqual(feat, QgsFeature())

        feat2 = QgsFeature(fields, 0)
        feat2.initAttributes(1)
        feat2.setAttribute(0, "text")
        feat2.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(123, 456)))

        self.assertEqual(feat, feat2)

        feat2.setId(5)
        self.assertNotEqual(feat, feat2)
        feat2.setId(0)
        self.assertEqual(feat, feat2)

        feat2.setAttribute(0, "text2")
        self.assertNotEqual(feat, feat2)
        feat2.setAttribute(0, "text")
        self.assertEqual(feat, feat2)

        feat2.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(1231, 4561)))
        self.assertNotEqual(feat, feat2)
        feat2.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(123, 456)))
        self.assertEqual(feat, feat2)

        field2 = QgsField('my_field3')
        fields.append(field2)
        feat2.setFields(fields)
        self.assertNotEqual(feat, feat2)

    def test_hash(self):
        fields = QgsFields()
        field1 = QgsField('my_field')
        fields.append(field1)
        field2 = QgsField('my_field2')
        fields.append(field2)

        feat = QgsFeature(fields, 0)
        feat.initAttributes(1)
        feat.setAttribute(0, "text")
        feat.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(123, 456)))

        self.assertIsNotNone(hash(feat))

        # try a second identical feature, hash should be the same
        feat2 = QgsFeature(fields, 0)
        feat2.initAttributes(1)
        feat2.setAttribute(0, "text")
        feat2.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(123, 456)))

        self.assertEqual(hash(feat), hash(feat2))

        # different feature, different hash
        feat2.setId(100)
        self.assertNotEqual(hash(feat), hash(feat2))

    def test_ValidFeature(self):
        myPath = os.path.join(unitTestDataPath(), 'points.shp')
        myLayer = QgsVectorLayer(myPath, 'Points', 'ogr')
        provider = myLayer.dataProvider()
        fit = provider.getFeatures()
        feat = QgsFeature()
        fit.nextFeature(feat)
        fit.close()
        myValidValue = feat.isValid()
        myMessage = f"\nExpected: True\nGot: {myValidValue}"
        assert myValidValue, myMessage

    def test_Validity(self):
        f = QgsFeature()
        self.assertFalse(f.isValid())
        f.setGeometry(QgsGeometry())
        self.assertTrue(f.isValid())
        f.setValid(False)
        self.assertFalse(f.isValid())
        fields = QgsFields()
        field1 = QgsField('my_field')
        fields.append(field1)
        field2 = QgsField('my_field2')
        fields.append(field2)
        f.setFields(fields)
        f.setAttribute(0, 0)
        self.assertTrue(f.isValid())
        f.setValid(False)
        self.assertFalse(f.isValid())
        f.setId(27)
        self.assertTrue(f.isValid())

    def test_Attributes(self):
        myPath = os.path.join(unitTestDataPath(), 'lines.shp')
        myLayer = QgsVectorLayer(myPath, 'Lines', 'ogr')
        provider = myLayer.dataProvider()
        fit = provider.getFeatures()
        feat = QgsFeature()
        fit.nextFeature(feat)
        fit.close()
        myAttributes = feat.attributes()
        myExpectedAttributes = ["Highway", 1]

        # Only for printing purposes
        myExpectedAttributes = ["Highway", 1]
        myMessage = f'\nExpected: {myExpectedAttributes}\nGot: {myAttributes}'

        assert myAttributes == myExpectedAttributes, myMessage

    def test_SetAttributes(self):
        feat = QgsFeature()
        feat.initAttributes(1)
        feat.setAttributes([0])
        feat.setAttributes([NULL])
        assert [NULL] == feat.attributes()

        # Test different type of attributes
        attributes = [
            -95985674563452,
            12,
            34.3,
            False,
            "QGIS",
            "some value",
            QVariant("foo"),
            QDate(2023, 1, 1),
            QTime(12, 11, 10),
            QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10)),
            True
        ]
        feat.initAttributes(len(attributes))
        feat.setAttributes(attributes)
        self.assertEqual(feat.attributes(), attributes)

    def test_setAttribute(self):
        feat = QgsFeature()
        feat.initAttributes(1)
        with self.assertRaises(KeyError):
            feat.setAttribute(-1, 5)
        with self.assertRaises(KeyError):
            feat.setAttribute(10, 5)
        self.assertTrue(feat.setAttribute(0, 5))

        # Test different type of attributes
        attributes = [
            -9585674563452,
            34.3,
            False,
            "QGIS",
            QVariant("foo"),
            QDate(2023, 1, 1),
            QTime(12, 11, 10),
            QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10))
        ]
        self.assertEqual(feat.attributeCount(), 1)
        for attribute in attributes:
            self.assertTrue(feat.setAttribute(0, attribute))
            self.assertEqual(feat.attribute(0), attribute)

            feat.setAttribute(0, None)
            self.assertEqual(feat.attribute(0), NULL)

            feat[0] = attribute
            self.assertEqual(feat.attribute(0), attribute)

    def test_SetAttributeByName(self):
        fields = QgsFields()
        field1 = QgsField('my_field')
        fields.append(field1)
        field2 = QgsField('my_field2')
        fields.append(field2)

        feat = QgsFeature(fields)
        feat.initAttributes(2)

        feat['my_field'] = 'foo'
        feat['my_field2'] = 'bah'
        self.assertEqual(feat.attributes(), ['foo', 'bah'])
        self.assertEqual(feat.attribute('my_field'), 'foo')
        self.assertEqual(feat.attribute('my_field2'), 'bah')

        # Test different type of attributes
        attributes = [
            {'name': 'int', 'value': -9585674563452},
            {'name': 'float', 'value': 34.3},
            {'name': 'bool', 'value': False},
            {'name': 'string', 'value': 'QGIS'},
            {'name': 'variant', 'value': QVariant('foo')},
            {'name': 'date', 'value': QDate(2023, 1, 1)},
            {'name': 'time', 'value': QTime(12, 11, 10)},
            {'name': 'datetime', 'value': QDateTime(QDate(2020, 5, 6), QTime(8, 9, 10))}
        ]
        fields = QgsFields()
        for attribute in attributes:
            fields.append(QgsField(attribute['name']))

        feat = QgsFeature(fields)
        feat.initAttributes(len(attributes))

        for attr in attributes:
            name = attr['name']
            value = attr['value']
            feat.setAttribute(name, value)
            self.assertEqual(feat.attribute(name), value)

            feat.setAttribute(name, None)
            self.assertEqual(feat.attribute(name), NULL)

            feat[name] = value
            self.assertEqual(feat.attribute(name), value)

    def test_DeleteAttribute(self):
        feat = QgsFeature()
        feat.initAttributes(3)
        feat[0] = "text1"
        feat[1] = "text2"
        feat[2] = "text3"
        feat.deleteAttribute(1)
        myAttrs = [feat[0], feat[1]]
        myExpectedAttrs = ["text1", "text3"]
        myMessage = f'\nExpected: {str(myExpectedAttrs)}\nGot: {str(myAttrs)}'
        assert myAttrs == myExpectedAttrs, myMessage

    def test_DeleteAttributeByName(self):
        fields = QgsFields()
        field1 = QgsField('my_field')
        fields.append(field1)
        field2 = QgsField('my_field2')
        fields.append(field2)

        feat = QgsFeature(fields)
        feat.initAttributes(2)
        feat[0] = "text1"
        feat[1] = "text2"
        with self.assertRaises(KeyError):
            feat.deleteAttribute('not present')
        self.assertTrue(feat.deleteAttribute('my_field'))
        self.assertEqual(feat.attributes(), ['text2'])

    def test_SetGeometry(self):
        feat = QgsFeature()
        feat.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(123, 456)))
        myGeometry = feat.geometry()
        myExpectedGeometry = "!None"
        myMessage = f'\nExpected: {myExpectedGeometry}\nGot: {myGeometry}'
        assert myGeometry is not None, myMessage

        # set from QgsAbstractGeometry
        feat.setGeometry(QgsPoint(12, 34))
        self.assertEqual(feat.geometry().asWkt(), 'Point (12 34)')

    def testAttributeCount(self):
        f = QgsFeature()
        self.assertEqual(f.attributeCount(), 0)
        f.setAttributes([1, 2, 3])
        self.assertEqual(f.attributeCount(), 3)

    def testResizeAttributes(self):
        f = QgsFeature()
        f.resizeAttributes(3)
        self.assertEqual(f.attributes(), [NULL, NULL, NULL])
        f.setAttributes([1, 2, 3])
        f.resizeAttributes(3)
        self.assertEqual(f.attributes(), [1, 2, 3])
        f.resizeAttributes(5)
        self.assertEqual(f.attributes(), [1, 2, 3, NULL, NULL])
        f.resizeAttributes(2)
        self.assertEqual(f.attributes(), [1, 2])

    def testPadAttributes(self):
        f = QgsFeature()
        f.padAttributes(3)
        self.assertEqual(f.attributes(), [NULL, NULL, NULL])
        f.setAttributes([1, 2, 3])
        f.padAttributes(0)
        self.assertEqual(f.attributes(), [1, 2, 3])
        f.padAttributes(2)
        self.assertEqual(f.attributes(), [1, 2, 3, NULL, NULL])
        f.padAttributes(3)
        self.assertEqual(f.attributes(), [1, 2, 3, NULL, NULL, NULL, NULL, NULL])

    def testAttributeMap(self):
        # start with a feature with no fields
        f = QgsFeature()
        f.setAttributes([1, 'a', NULL])
        with self.assertRaises(ValueError):
            _ = f.attributeMap()

        # set fields
        fields = QgsFields()
        field1 = QgsField('my_field')
        fields.append(field1)
        field2 = QgsField('my_field2')
        fields.append(field2)
        field3 = QgsField('my_field3')
        fields.append(field3)
        f.setFields(fields)
        f.setAttributes([1, 'a', NULL])
        self.assertEqual(f.attributeMap(), {'my_field': 1, 'my_field2': 'a', 'my_field3': NULL})

        # unbalanced fields/attributes -- should be handled gracefully
        # less attributes than fields
        f.setAttributes([1, 'a'])
        with self.assertRaises(ValueError):
            _ = f.attributeMap()
        f.setAttributes([1, 'a', 2, 3])
        # more attributes than fields
        with self.assertRaises(ValueError):
            _ = f.attributeMap()

    def testUnsetFeature(self):
        f = QgsFeature()
        f.setAttributes([1, 'a', NULL, QgsUnsetAttributeValue(), QgsUnsetAttributeValue('Autonumber')])
        with self.assertRaises(KeyError):
            f.isUnsetValue(-1)
        with self.assertRaises(KeyError):
            f.isUnsetValue(5)
        self.assertFalse(f.isUnsetValue(0))
        self.assertFalse(f.isUnsetValue(1))
        self.assertFalse(f.isUnsetValue(2))
        self.assertTrue(f.isUnsetValue(3))
        self.assertTrue(f.isUnsetValue(4))


if __name__ == '__main__':
    unittest.main()
