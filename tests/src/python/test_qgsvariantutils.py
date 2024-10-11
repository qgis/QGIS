# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsVariantUtils.

From build dir, run: ctest -R PyQgsVariantUtils -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
__author__ = 'Alessandro Pasotti'
__date__ = '11/10/224'
__copyright__ = 'Copyright 2024, The QGIS Project'


from qgis.PyQt.QtCore import QVariant, QMetaType
from qgis.core import QgsVariantUtils

from qgis.testing import unittest


class TestQgsVariantUtils(unittest.TestCase):

    def test_is_numeric_type(self):

        self.assertTrue(QgsVariantUtils.isNumericType(QMetaType.Int))
        self.assertTrue(QgsVariantUtils.isNumericType(QMetaType.UInt))
        self.assertTrue(QgsVariantUtils.isNumericType(QMetaType.LongLong))
        self.assertTrue(QgsVariantUtils.isNumericType(QMetaType.ULongLong))
        self.assertTrue(QgsVariantUtils.isNumericType(QMetaType.Double))
        self.assertTrue(QgsVariantUtils.isNumericType(QMetaType.Float))
        self.assertTrue(QgsVariantUtils.isNumericType(QMetaType.Short))
        self.assertTrue(QgsVariantUtils.isNumericType(QMetaType.UShort))
        self.assertTrue(QgsVariantUtils.isNumericType(QMetaType.Char))
        self.assertTrue(QgsVariantUtils.isNumericType(QMetaType.UChar))
        self.assertTrue(QgsVariantUtils.isNumericType(QMetaType.SChar))

        self.assertFalse(QgsVariantUtils.isNumericType(QMetaType.Bool))
        self.assertFalse(QgsVariantUtils.isNumericType(QMetaType.QString))
        self.assertFalse(QgsVariantUtils.isNumericType(QMetaType.QByteArray))
        self.assertFalse(QgsVariantUtils.isNumericType(QMetaType.QDateTime))
        self.assertFalse(QgsVariantUtils.isNumericType(QMetaType.QDate))
        self.assertFalse(QgsVariantUtils.isNumericType(QMetaType.QTime))


if __name__ == '__main__':
    unittest.main()
