"""QGIS Unit tests for QgsMagneticModel.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

from qgis.PyQt.QtCore import QDate, QTime, QDateTime
from qgis.core import Qgis, QgsMagneticModel, QgsExpression
from qgis.testing import start_app, QgisTestCase
import unittest

app = start_app()


@unittest.skipIf(not Qgis.hasGeographicLib(), "GeographicLib is not available")
class TestQgsMagneticModel(QgisTestCase):

    def test_invalid_model(self):
        model = QgsMagneticModel("xxxx")
        self.assertFalse(model.isValid())
        self.assertEqual(model.error()[:11], "Cannot open")
        self.assertFalse(model.dateTime().isValid())

    @unittest.skipIf(
        not QgsMagneticModel("wmm2025").isValid(), "WMM2025 is not available"
    )
    def test_wmm2025(self):
        model = QgsMagneticModel("wmm2025")
        self.assertTrue(model.isValid())
        self.assertEqual(model.description(), "World Magnetic Model 2025")
        self.assertEqual(
            model.dateTime(), QDateTime(QDate(2024, 11, 13), QTime(0, 0, 0))
        )
        self.assertEqual(model.file()[-11:], "wmm2025.wmm")
        self.assertEqual(model.name(), "wmm2025")
        self.assertFalse(model.error())
        self.assertEqual(model.maximumHeight(), 850000.0)
        self.assertEqual(model.minimumHeight(), -1000.0)
        self.assertEqual(model.maximumYear(), 2030.0)
        self.assertEqual(model.minimumYear(), 2025.0)
        self.assertEqual(model.degree(), 12)
        self.assertEqual(model.order(), 12)

        res, d = model.declination(2026.6, -35, 138, 0)
        self.assertTrue(res)
        self.assertAlmostEqual(d, 7.875942468396237, 5)

        res, i = model.inclination(2026.6, -35, 138, 0)
        self.assertTrue(res)
        self.assertAlmostEqual(i, -67.00907306480912, 5)

        res, bx, by, bz = model.getComponents(2026.6, -35, 138, 0)
        self.assertTrue(res)
        self.assertAlmostEqual(bx, 3175.8637216334573, 5)
        self.assertAlmostEqual(by, 22958.01937821404, 5)
        self.assertAlmostEqual(bz, 54624.79689642607, 5)

        res, bx, by, bz, bxt, byt, bzt = model.getComponentsWithTimeDerivatives(
            2026.6, -35, 138, 0
        )
        self.assertTrue(res)
        self.assertAlmostEqual(bx, 3175.8637216334573, 5)
        self.assertAlmostEqual(by, 22958.01937821404, 5)
        self.assertAlmostEqual(bz, 54624.79689642607, 5)
        self.assertAlmostEqual(bxt, 8.384503078423284, 5)
        self.assertAlmostEqual(byt, 1.9202063131442957, 5)
        self.assertAlmostEqual(bzt, -9.966187924386709, 5)

        res, h, f, d, i = QgsMagneticModel.fieldComponents(bx, by, bz)
        self.assertTrue(res)
        self.assertAlmostEqual(h, 23176.642641867675, 5)
        self.assertAlmostEqual(f, 59338.227140053976, 5)
        self.assertAlmostEqual(d, 7.875942468396237, 5)
        self.assertAlmostEqual(i, -67.00907306480912, 5)

        res, h, f, d, i, ht, ft, dt, it = (
            QgsMagneticModel.fieldComponentsWithTimeDerivatives(
                bx, by, bz, bxt, byt, bzt
            )
        )
        self.assertTrue(res)
        self.assertAlmostEqual(h, 23176.642641867675, 5)
        self.assertAlmostEqual(f, 59338.227140053976, 5)
        self.assertAlmostEqual(d, 7.875942468396237, 5)
        self.assertAlmostEqual(i, -67.00907306480912, 5)
        self.assertAlmostEqual(ht, 3.0510101911950582, 5)
        self.assertAlmostEqual(ft, -7.982860984120447, 5)
        self.assertAlmostEqual(dt, 0.019881621989770698, 5)
        self.assertAlmostEqual(it, 0.0064706436479898075, 5)

    @unittest.skipIf(
        not QgsMagneticModel("wmm2025").isValid(), "WMM2025 is not available"
    )
    def test_expression_functions(self):
        # bad model name
        exp = QgsExpression(
            "magnetic_declination('xxxxxx', make_datetime(2026,7,1,12,0,0), -35, 138, 0)"
        )
        res = exp.evaluate()
        self.assertIsNone(res)
        self.assertEqual(
            exp.evalErrorString()[:36], "Cannot evaluate magnetic declination"
        )
        exp = QgsExpression(
            "magnetic_inclination('xxxxxx', make_datetime(2026,7,1,12,0,0), -35, 138, 0)"
        )
        res = exp.evaluate()
        self.assertIsNone(res)
        self.assertEqual(
            exp.evalErrorString()[:36], "Cannot evaluate magnetic inclination"
        )
        exp = QgsExpression(
            "magnetic_declination_rate_of_change('xxxxxx', make_datetime(2026,7,1,12,0,0), -35, 138, 0)"
        )
        res = exp.evaluate()
        self.assertIsNone(res)
        self.assertEqual(
            exp.evalErrorString()[:51],
            "Cannot evaluate magnetic declination rate of change",
        )
        exp = QgsExpression(
            "magnetic_inclination_rate_of_change('xxxxxx', make_datetime(2026,7,1,12,0,0), -35, 138, 0)"
        )
        res = exp.evaluate()
        self.assertIsNone(res)
        self.assertEqual(
            exp.evalErrorString()[:51],
            "Cannot evaluate magnetic inclination rate of change",
        )

        # bad dates
        exp = QgsExpression(
            "magnetic_declination('wmm2025', 'not a date', -35, 138, 0)"
        )
        res = exp.evaluate()
        self.assertIsNone(res)
        self.assertEqual(
            exp.evalErrorString(),
            "Function `magnetic_declination` requires a valid date",
        )

        exp = QgsExpression(
            "magnetic_declination_rate_of_change('wmm2025', 'not a date', -35, 138, 0)"
        )
        res = exp.evaluate()
        self.assertIsNone(res)
        self.assertEqual(
            exp.evalErrorString(),
            "Function `magnetic_declination_rate_of_change` requires a valid date",
        )

        exp = QgsExpression(
            "magnetic_inclination('wmm2025', 'not a date', -35, 138, 0)"
        )
        res = exp.evaluate()
        self.assertIsNone(res)
        self.assertEqual(
            exp.evalErrorString(),
            "Function `magnetic_inclination` requires a valid date",
        )

        exp = QgsExpression(
            "magnetic_inclination_rate_of_change('wmm2025', 'not a date', -35, 138, 0)"
        )
        res = exp.evaluate()
        self.assertIsNone(res)
        self.assertEqual(
            exp.evalErrorString(),
            "Function `magnetic_inclination_rate_of_change` requires a valid date",
        )

        # good values
        exp = QgsExpression(
            "magnetic_declination('wmm2025', make_datetime(2026,7,1,12,0,0), -35, 138, 0)"
        )
        res = exp.evaluate()
        self.assertAlmostEqual(res, 7.873899808374294, 5)

        exp = QgsExpression(
            "magnetic_declination_rate_of_change('wmm2025', make_datetime(2026,7,1,12,0,0), -35, 138, 0)"
        )
        res = exp.evaluate()
        self.assertAlmostEqual(res, 0.019881621989770698, 5)

        exp = QgsExpression(
            "magnetic_inclination('wmm2025', make_datetime(2026,7,1,12,0,0), -35, 138, 0)"
        )
        res = exp.evaluate()
        self.assertAlmostEqual(res, -67.0097378346844, 5)

        exp = QgsExpression(
            "magnetic_inclination_rate_of_change('wmm2025', make_datetime(2026,7,1,12,0,0), -35, 138, 0)"
        )
        res = exp.evaluate()
        self.assertAlmostEqual(res, 0.0064706436479898075, 5)


if __name__ == "__main__":
    unittest.main()
