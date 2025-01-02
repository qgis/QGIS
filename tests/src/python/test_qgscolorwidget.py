"""QGIS Unit tests for QgsColorWidget

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Julien Cabieces"
__date__ = "02/05/2024"
__copyright__ = "Copyright 2024, The QGIS Project"

from qgis.PyQt.QtGui import QColor
from qgis.gui import QgsColorWidget

import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsColorWidget(QgisTestCase):

    def testAlterColor(self):
        """
        test alterColor method
        """

        # rgb
        color = QColor(12, 34, 56)
        QgsColorWidget.alterColor(color, QgsColorWidget.ColorComponent.Red, 112)
        self.assertEqual(color, QColor(112, 34, 56, 255))
        self.assertEqual(color.spec(), QColor.Spec.Rgb)
        QgsColorWidget.alterColor(color, QgsColorWidget.ColorComponent.Green, 134)
        self.assertEqual(color.spec(), QColor.Spec.Rgb)
        self.assertEqual(color, QColor(112, 134, 56, 255))
        QgsColorWidget.alterColor(color, QgsColorWidget.ColorComponent.Blue, 156)
        self.assertEqual(color.spec(), QColor.Spec.Rgb)
        self.assertEqual(color, QColor(112, 134, 156, 255))
        QgsColorWidget.alterColor(color, QgsColorWidget.ColorComponent.Alpha, 210)
        self.assertEqual(color.spec(), QColor.Spec.Rgb)
        self.assertEqual(color, QColor(112, 134, 156, 210))

        # hsv
        QgsColorWidget.alterColor(color, QgsColorWidget.ColorComponent.Hue, 100)
        self.assertEqual(color.spec(), QColor.Spec.Hsv)
        self.assertEqual(color, QColor.fromHsv(100, 72, 156, 210))
        QgsColorWidget.alterColor(color, QgsColorWidget.ColorComponent.Saturation, 150)
        self.assertEqual(color.spec(), QColor.Spec.Hsv)
        self.assertEqual(color, QColor.fromHsv(100, 150, 156, 210))
        QgsColorWidget.alterColor(color, QgsColorWidget.ColorComponent.Value, 200)
        self.assertEqual(color.spec(), QColor.Spec.Hsv)
        self.assertEqual(color, QColor.fromHsv(100, 150, 200, 210))

        # clipped value
        QgsColorWidget.alterColor(color, QgsColorWidget.ColorComponent.Value, 300)
        self.assertEqual(color.spec(), QColor.Spec.Hsv)
        self.assertEqual(color, QColor.fromHsv(100, 150, 255, 210))
        QgsColorWidget.alterColor(color, QgsColorWidget.ColorComponent.Value, -2)
        self.assertEqual(color.spec(), QColor.Spec.Hsv)
        self.assertEqual(color, QColor.fromHsv(100, 150, 0, 210))

        # cmyk
        QgsColorWidget.alterColor(color, QgsColorWidget.ColorComponent.Cyan, 22)
        self.assertEqual(color.spec(), QColor.Spec.Cmyk)
        self.assertEqual(color, QColor.fromCmyk(22, 0, 0, 255, 210))
        QgsColorWidget.alterColor(color, QgsColorWidget.ColorComponent.Magenta, 33)
        self.assertEqual(color.spec(), QColor.Spec.Cmyk)
        self.assertEqual(color, QColor.fromCmyk(22, 33, 0, 255, 210))
        QgsColorWidget.alterColor(color, QgsColorWidget.ColorComponent.Yellow, 44)
        self.assertEqual(color.spec(), QColor.Spec.Cmyk)
        self.assertEqual(color, QColor.fromCmyk(22, 33, 44, 255, 210))
        QgsColorWidget.alterColor(color, QgsColorWidget.ColorComponent.Black, 55)
        self.assertEqual(color.spec(), QColor.Spec.Cmyk)
        self.assertEqual(color, QColor.fromCmyk(22, 33, 44, 55, 210))

    def testSetComponentValue(self):
        """
        test setComponentValue method
        """
        w = QgsColorWidget()

        w.setColor(QColor(12, 34, 56))
        self.assertEqual(w.color(), QColor(12, 34, 56))

        # rgb
        w.setComponent(QgsColorWidget.ColorComponent.Red)
        w.setComponentValue(112)
        self.assertEqual(w.color(), QColor(112, 34, 56, 255))
        self.assertEqual(w.color().spec(), QColor.Spec.Rgb)
        w.setComponent(QgsColorWidget.ColorComponent.Green)
        w.setComponentValue(134)
        self.assertEqual(w.color().spec(), QColor.Spec.Rgb)
        self.assertEqual(w.color(), QColor(112, 134, 56, 255))
        w.setComponent(QgsColorWidget.ColorComponent.Blue)
        w.setComponentValue(156)
        self.assertEqual(w.color().spec(), QColor.Spec.Rgb)
        self.assertEqual(w.color(), QColor(112, 134, 156, 255))
        w.setComponent(QgsColorWidget.ColorComponent.Alpha)
        w.setComponentValue(210)
        self.assertEqual(w.color().spec(), QColor.Spec.Rgb)
        self.assertEqual(w.color(), QColor(112, 134, 156, 210))

        # hsv
        w.setComponent(QgsColorWidget.ColorComponent.Hue)
        w.setComponentValue(100)
        self.assertEqual(w.color().spec(), QColor.Spec.Hsv)
        self.assertEqual(w.color(), QColor.fromHsv(100, 72, 156, 210))
        w.setComponent(QgsColorWidget.ColorComponent.Saturation)
        w.setComponentValue(150)
        self.assertEqual(w.color().spec(), QColor.Spec.Hsv)
        self.assertEqual(w.color(), QColor.fromHsv(100, 150, 156, 210))
        w.setComponent(QgsColorWidget.ColorComponent.Value)
        w.setComponentValue(200)
        self.assertEqual(w.color().spec(), QColor.Spec.Hsv)
        self.assertEqual(w.color(), QColor.fromHsv(100, 150, 200, 210))

        # clipped value
        w.setComponent(QgsColorWidget.ColorComponent.Value)
        w.setComponentValue(300)
        self.assertEqual(w.color().spec(), QColor.Spec.Hsv)
        self.assertEqual(w.color(), QColor.fromHsv(100, 150, 255, 210))
        w.setComponent(QgsColorWidget.ColorComponent.Value)
        w.setComponentValue(-2)
        self.assertEqual(w.color().spec(), QColor.Spec.Hsv)
        self.assertEqual(w.color(), QColor.fromHsv(100, 150, 0, 210))

        # Multiple component has no effect
        w.setComponent(QgsColorWidget.ColorComponent.Multiple)
        w.setComponentValue(18)
        self.assertEqual(w.color().spec(), QColor.Spec.Hsv)
        self.assertEqual(w.color(), QColor.fromHsv(100, 150, 0, 210))

        # set an achromatic color to check it keeps the hue
        w.setColor(QColor(130, 130, 130, 255))
        self.assertEqual(w.color().spec(), QColor.Spec.Rgb)
        self.assertEqual(w.color(), QColor(130, 130, 130, 255))
        w.setComponent(QgsColorWidget.ColorComponent.Value)
        w.setComponentValue(42)
        self.assertEqual(w.color(), QColor.fromHsv(100, 0, 42, 255))
        self.assertEqual(w.componentValue(QgsColorWidget.ColorComponent.Hue), 100)

        # cmyk
        w.setComponent(QgsColorWidget.ColorComponent.Cyan)
        w.setComponentValue(22)
        self.assertEqual(w.color().spec(), QColor.Spec.Cmyk)
        self.assertEqual(w.color(), QColor.fromCmyk(22, 0, 0, 213, 255))
        w.setComponent(QgsColorWidget.ColorComponent.Magenta)
        w.setComponentValue(33)
        self.assertEqual(w.color().spec(), QColor.Spec.Cmyk)
        self.assertEqual(w.color(), QColor.fromCmyk(22, 33, 0, 213, 255))
        w.setComponent(QgsColorWidget.ColorComponent.Yellow)
        w.setComponentValue(44)
        self.assertEqual(w.color().spec(), QColor.Spec.Cmyk)
        self.assertEqual(w.color(), QColor.fromCmyk(22, 33, 44, 213, 255))
        w.setComponent(QgsColorWidget.ColorComponent.Black)
        w.setComponentValue(55)
        self.assertEqual(w.color().spec(), QColor.Spec.Cmyk)
        self.assertEqual(w.color(), QColor.fromCmyk(22, 33, 44, 55, 255))

        # set an achromatic color to check it keeps the hue (set from former cmyk values)
        self.assertEqual(w.color().hue(), 30)
        w.setColor(QColor(130, 130, 130, 255))
        self.assertEqual(w.color().spec(), QColor.Spec.Rgb)
        self.assertEqual(w.color(), QColor(130, 130, 130, 255))
        w.setComponent(QgsColorWidget.ColorComponent.Value)
        w.setComponentValue(42)
        self.assertEqual(w.color(), QColor.fromHsv(30, 0, 42, 255))
        self.assertEqual(w.componentValue(QgsColorWidget.ColorComponent.Hue), 30)


if __name__ == "__main__":
    unittest.main()
