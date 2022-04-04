# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsField.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '16/08/2015'
__copyright__ = 'Copyright 2015, The QGIS Project'

import qgis  # NOQA

from qgis.core import QgsVectorLayer, NULL
from qgis.testing import start_app, unittest
from qgis.PyQt.QtCore import QVariant, QDate, QDateTime, QTime

start_app()


class TestQgsFields(unittest.TestCase):

    def test_exceptions(self):
        ml = QgsVectorLayer("Point?crs=epsg:4236&field=id:integer&field=value:double",
                            "test_data", "memory")
        assert ml.isValid()
        fields = ml.fields()

        # check no error
        fields.remove(1)
        # check exceptions raised
        with self.assertRaises(KeyError):
            fields.remove(-1)
        with self.assertRaises(KeyError):
            fields.remove(111)

        fields = ml.fields()

        # check no error
        self.assertEqual("value", fields[1].name())
        self.assertEqual("value", fields[-1].name())
        # check exceptions raised
        with self.assertRaises(IndexError):
            fields[111]

        # check no error
        self.assertEqual("value", fields['value'].name())
        self.assertEqual("id", fields['ID'].name())
        # check exceptions raised
        with self.assertRaises(KeyError):
            fields['arg']

        # check no error
        fields.at(1)
        # check exceptions raised
        with self.assertRaises(KeyError):
            fields.at(-1)
        with self.assertRaises(KeyError):
            fields.at(111)

        # check no error
        fields.field(1)
        # check exceptions raised
        with self.assertRaises(KeyError):
            fields.field(-1)
        with self.assertRaises(KeyError):
            fields.field(111)

        # check no error
        fields.field('value')
        # check exceptions raised
        with self.assertRaises(KeyError):
            fields.field('bad')

        # check no error
        fields.fieldOrigin(1)
        # check exceptions raised
        with self.assertRaises(KeyError):
            fields.fieldOrigin(-1)
        with self.assertRaises(KeyError):
            fields.fieldOrigin(111)

        # check no error
        fields.fieldOriginIndex(1)
        # check exceptions raised
        with self.assertRaises(KeyError):
            fields.fieldOriginIndex(-1)
        with self.assertRaises(KeyError):
            fields.fieldOriginIndex(111)

        # check no error
        fields.iconForField(1)
        # check exceptions raised
        with self.assertRaises(KeyError):
            fields.iconForField(-1)
        with self.assertRaises(KeyError):
            fields.iconForField(111)

    def test_names(self):
        ml = QgsVectorLayer(
            "Point?crs=epsg:4236"
            + "&field=id:integer"
            + "&field=value:double"
            + "&field=crazy:double",
            "test_data",
            "memory")

        assert ml.isValid()
        fields = ml.fields()

        expected_fields = ['id', 'value', 'crazy']

        self.assertEqual(fields.names(), expected_fields)
        fields.remove(1)
        expected_fields = ['id', 'crazy']
        self.assertEqual(fields.names(), expected_fields)

    def test_convert_compatible(self):
        """Test convertCompatible"""

        vl = QgsVectorLayer('Point?crs=epsg:4326&field=int:integer', 'test', 'memory')

        # Valid values
        self.assertTrue(vl.fields()[0].convertCompatible(123.0))
        self.assertTrue(vl.fields()[0].convertCompatible(123))
        # Check NULL/invalid
        self.assertIsNone(vl.fields()[0].convertCompatible(None))
        self.assertEqual(vl.fields()[0].convertCompatible(QVariant(QVariant.Int)), NULL)
        # Not valid
        with self.assertRaises(ValueError) as cm:
            vl.fields()[0].convertCompatible('QGIS Rocks!')
        self.assertEqual(str(cm.exception), 'Value could not be converted to field type int: Value "QGIS Rocks!" is not a number')

        with self.assertRaises(ValueError) as cm:
            self.assertFalse(vl.fields()[0].convertCompatible(QDate(2020, 6, 30)))
        self.assertEqual(str(cm.exception),
                         'Value could not be converted to field type int: Could not convert value "2020-06-30" to target type')

        # Not valid: overflow
        with self.assertRaises(ValueError) as cm:
            self.assertFalse(vl.fields()[0].convertCompatible(2147483647 + 1))
        self.assertEqual(str(cm.exception),
                         'Value could not be converted to field type int: Value "2147483648" is too large for integer field')
        # Valid: narrow cast with loss of precision (!)
        self.assertTrue(vl.fields()[0].convertCompatible(123.123))

        vl = QgsVectorLayer('Point?crs=epsg:4326&field=date:date', 'test', 'memory')
        self.assertTrue(vl.fields()[0].convertCompatible(QDate(2020, 6, 30)))
        # Not valid
        with self.assertRaises(ValueError) as cm:
            self.assertFalse(vl.fields()[0].convertCompatible('QGIS Rocks!'))
        self.assertEqual(str(cm.exception),
                         'Value could not be converted to field type QDate: Could not convert value "QGIS Rocks!" to target type')
        with self.assertRaises(ValueError) as cm:
            self.assertFalse(vl.fields()[0].convertCompatible(123))
        self.assertEqual(str(cm.exception),
                         'Value could not be converted to field type QDate: Could not convert value "123" to target type')

        # Strings can store almost anything
        vl = QgsVectorLayer('Point?crs=epsg:4326&field=text:string(30)', 'test', 'memory')
        self.assertTrue(vl.fields()[0].convertCompatible(QDate(2020, 6, 30)))
        self.assertTrue(vl.fields()[0].convertCompatible('QGIS Rocks!'))
        self.assertTrue(vl.fields()[0].convertCompatible(123))
        self.assertTrue(vl.fields()[0].convertCompatible(123.456))
        # string overflow
        self.assertEqual(vl.fields()[0].length(), 30)
        with self.assertRaises(ValueError) as cm:
            self.assertTrue(vl.fields()[0].convertCompatible('x' * 31))
        self.assertEqual(str(cm.exception),
                         'Value could not be converted to field type QString: String of length 31 exceeds maximum field length (30)')

        vl = QgsVectorLayer('Point?crs=epsg:4326&field=double:double', 'test', 'memory')

        # Valid values
        self.assertTrue(vl.fields()[0].convertCompatible(123.0))
        self.assertTrue(vl.fields()[0].convertCompatible(123))
        # Check NULL/invalid
        self.assertIsNone(vl.fields()[0].convertCompatible(None))
        self.assertEqual(vl.fields()[0].convertCompatible(NULL), NULL)
        self.assertTrue(vl.fields()[0].convertCompatible(QVariant.Double))
        # Not valid
        with self.assertRaises(ValueError) as cm:
            self.assertFalse(vl.fields()[0].convertCompatible('QGIS Rocks!'))
        self.assertEqual(str(cm.exception),
                         'Value could not be converted to field type double: Could not convert value "QGIS Rocks!" to target type')
        with self.assertRaises(ValueError) as cm:
            self.assertFalse(vl.fields()[0].convertCompatible(QDate(2020, 6, 30)))
        self.assertEqual(str(cm.exception),
                         'Value could not be converted to field type double: Could not convert value "2020-06-30" to target type')


if __name__ == '__main__':
    unittest.main()
