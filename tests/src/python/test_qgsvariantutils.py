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


from qgis.PyQt.QtCore import QDate, QDateTime, QLocale, QTime, QVariant, QMetaType
from qgis.core import QgsVariantUtils

from qgis.testing import unittest


class TestQgsVariantUtils(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        super().setUpClass()

        QLocale.setDefault(QLocale(QLocale.Language.English))

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

    def test_display_string(self):
        # string
        self.assertEqual(QgsVariantUtils.displayString(QVariant("foo bar")), "foo bar")
        self.assertEqual(
            QgsVariantUtils.displayString(QVariant("foo bar"), 2), "foo bar"
        )

        # integer
        self.assertEqual(QgsVariantUtils.displayString(QVariant(23)), "23")
        self.assertEqual(QgsVariantUtils.displayString(QVariant(23), 1), "23")

        # float
        self.assertEqual(QgsVariantUtils.displayString(QVariant(3460.456)), "3,460.456")
        self.assertEqual(
            QgsVariantUtils.displayString(QVariant(3460.456), 2), "3,460.46"
        )
        self.assertEqual(
            QgsVariantUtils.displayString(QVariant(3460.453), 2), "3,460.45"
        )
        self.assertEqual(QgsVariantUtils.displayString(QVariant(-12.45)), "-12.45")
        self.assertEqual(QgsVariantUtils.displayString(QVariant(-12.45), 1), "-12.4")

        # date and time
        date = QDate(2020, 1, 3)
        time = QTime(12, 3, 17)
        date_time = QDateTime(date, time)
        self.assertEqual(QgsVariantUtils.displayString(QVariant(date)), "2020-01-03")
        self.assertEqual(QgsVariantUtils.displayString(QVariant(date), 3), "2020-01-03")
        self.assertEqual(QgsVariantUtils.displayString(QVariant(time)), "12:03:17.000")
        self.assertEqual(
            QgsVariantUtils.displayString(QVariant(time), 3), "12:03:17.000"
        )
        self.assertEqual(
            QgsVariantUtils.displayString(QVariant(date_time)),
            "2020-01-03T12:03:17.000",
        )
        self.assertEqual(
            QgsVariantUtils.displayString(QVariant(date_time), 3),
            "2020-01-03T12:03:17.000",
        )

        # list of float
        self.assertEqual(
            QgsVariantUtils.displayString(QVariant([-23.2986, 45.456, 6700.34556])),
            "-23.2986;45.456;6,700.34556",
        )
        self.assertEqual(
            QgsVariantUtils.displayString(QVariant([-23.2986, 45.456, 6700.34556]), 2),
            "-23.30;45.46;6,700.35",
        )

        # list of float int and str
        self.assertEqual(
            QgsVariantUtils.displayString(QVariant([45.456, 23, "12.4", 6700.34556])),
            "45.456;23;12.4;6,700.34556",
        )
        self.assertEqual(
            QgsVariantUtils.displayString(
                QVariant([45.456, 23, "12.4", 6700.34556]), 2
            ),
            "45.46;23;12.4;6,700.35",
        )


if __name__ == "__main__":
    unittest.main()
