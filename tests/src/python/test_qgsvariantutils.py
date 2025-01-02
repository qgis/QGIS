"""QGIS Unit tests for QgsVariantUtils.

From build dir, run: ctest -R PyQgsVariantUtils -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""

__author__ = "Alessandro Pasotti"
__date__ = "11/10/224"
__copyright__ = "Copyright 2024, The QGIS Project"


from qgis.PyQt.QtCore import QVariant, QMetaType
from qgis.core import QgsVariantUtils

from qgis.testing import unittest


class TestQgsVariantUtils(unittest.TestCase):

    def test_is_numeric_type(self):

        self.assertTrue(QgsVariantUtils.isNumericType(QMetaType.Type.Int))
        self.assertTrue(QgsVariantUtils.isNumericType(QMetaType.Type.UInt))
        self.assertTrue(QgsVariantUtils.isNumericType(QMetaType.Type.LongLong))
        self.assertTrue(QgsVariantUtils.isNumericType(QMetaType.Type.ULongLong))
        self.assertTrue(QgsVariantUtils.isNumericType(QMetaType.Type.Double))
        self.assertTrue(QgsVariantUtils.isNumericType(QMetaType.Type.Float))
        self.assertTrue(QgsVariantUtils.isNumericType(QMetaType.Type.Short))
        self.assertTrue(QgsVariantUtils.isNumericType(QMetaType.Type.UShort))
        self.assertTrue(QgsVariantUtils.isNumericType(QMetaType.Type.Char))
        self.assertTrue(QgsVariantUtils.isNumericType(QMetaType.Type.UChar))
        self.assertTrue(QgsVariantUtils.isNumericType(QMetaType.Type.SChar))

        self.assertFalse(QgsVariantUtils.isNumericType(QMetaType.Type.Bool))
        self.assertFalse(QgsVariantUtils.isNumericType(QMetaType.Type.QString))
        self.assertFalse(QgsVariantUtils.isNumericType(QMetaType.Type.QByteArray))
        self.assertFalse(QgsVariantUtils.isNumericType(QMetaType.Type.QDateTime))
        self.assertFalse(QgsVariantUtils.isNumericType(QMetaType.Type.QDate))
        self.assertFalse(QgsVariantUtils.isNumericType(QMetaType.Type.QTime))


if __name__ == "__main__":
    unittest.main()
