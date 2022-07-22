# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsColorUtils.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '06/07/2022'
__copyright__ = 'Copyright 2022, The QGIS Project'

import shutil

import qgis  # NOQA

import tempfile
import os
from qgis.PyQt.QtGui import QColor
from qgis.core import (
    QgsSymbolLayerUtils,
    QgsColorUtils,
    QgsReadWriteContext
)
from qgis.PyQt.QtXml import QDomDocument

from qgis.testing import unittest
from utilities import unitTestDataPath


class TestQgsColorUtils(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        try:
            QColor.ExtendedRgb
            cls.has_extended_rgb = True
        except AttributeError:
            cls.has_extended_rgb = False

    def test_color_xml(self):
        """
        Test storing/restoring colors from xml
        """

        doc = QDomDocument()
        context = QgsReadWriteContext()
        element = doc.createElement('element')

        # invalid color
        QgsColorUtils.writeXml(QColor(), 'my_color', doc, element, context)
        res = QgsColorUtils.readXml(element, 'my_color', context)
        self.assertFalse(res.isValid())

        # rgb color
        color = QColor.fromRgbF(1 / 65536, 2 / 65536, 3 / 65536, 4 / 65536)
        QgsColorUtils.writeXml(color, 'my_color', doc, element, context)
        res = QgsColorUtils.readXml(element, 'my_color', context)
        self.assertTrue(res.isValid())
        self.assertEqual(res.spec(), QColor.Rgb)
        self.assertAlmostEqual(res.redF(), 1 / 65536, 5)
        self.assertAlmostEqual(res.greenF(), 2 / 65536, 5)
        self.assertAlmostEqual(res.blueF(), 3 / 65536, 5)
        self.assertAlmostEqual(res.alphaF(), 4 / 65536, 5)

        color = QColor.fromRgb(16, 17, 18, 20)
        QgsColorUtils.writeXml(color, 'my_color', doc, element, context)
        res = QgsColorUtils.readXml(element, 'my_color', context)
        self.assertTrue(res.isValid())
        self.assertEqual(res.spec(), QColor.Rgb)
        self.assertEqual(res.red(), 16)
        self.assertEqual(res.green(), 17)
        self.assertEqual(res.blue(), 18)
        self.assertEqual(res.alpha(), 20)

        # rgb extended color
        if TestQgsColorUtils.has_extended_rgb:
            color = QColor.fromRgbF(-1 / 65536, 2 / 65536, 3 / 65536, 4 / 65536)
            QgsColorUtils.writeXml(color, 'my_rgb_ex_color', doc, element, context)
            res = QgsColorUtils.readXml(element, 'my_rgb_ex_color', context)
            self.assertTrue(res.isValid())
            self.assertEqual(res.spec(), QColor.ExtendedRgb)
            self.assertAlmostEqual(res.redF(), -1 / 65536, 5)
            self.assertAlmostEqual(res.greenF(), 2 / 65536, 5)
            self.assertAlmostEqual(res.blueF(), 3 / 65536, 5)
            self.assertAlmostEqual(res.alphaF(), 4 / 65536, 5)

        # hsv color
        color = QColor.fromHsvF(1 / 65536, 2 / 65536, 3 / 65536, 4 / 65536)
        QgsColorUtils.writeXml(color, 'my_hsv_color', doc, element, context)
        res = QgsColorUtils.readXml(element, 'my_hsv_color', context)
        self.assertTrue(res.isValid())
        self.assertEqual(res.spec(), QColor.Hsv)
        self.assertAlmostEqual(res.hueF(), 1 / 65536, 4)
        self.assertAlmostEqual(res.hsvSaturationF(), 2 / 65536, 5)
        self.assertAlmostEqual(res.valueF(), 3 / 65536, 5)
        self.assertAlmostEqual(res.alphaF(), 4 / 65536, 5)

        # hsl color
        color = QColor.fromHslF(111 / 65536, 12222 / 65536, 333 / 65536, 4 / 65536)
        QgsColorUtils.writeXml(color, 'my_hsl_color', doc, element, context)
        res = QgsColorUtils.readXml(element, 'my_hsl_color', context)
        self.assertTrue(res.isValid())
        self.assertEqual(res.spec(), QColor.Hsl)
        self.assertAlmostEqual(res.hslHueF(), 111 / 65536, 5)
        self.assertAlmostEqual(res.hslSaturationF(), 12222 / 65536, 5)
        self.assertAlmostEqual(res.lightnessF(), 333 / 65536, 4)
        self.assertAlmostEqual(res.alphaF(), 4 / 65536, 5)

        # cmyk color
        color = QColor.fromCmykF(1 / 65536, 2 / 65536, 3 / 65536, 4 / 65536, 5 / 65536)
        QgsColorUtils.writeXml(color, 'my_cmyk_color', doc, element, context)
        res = QgsColorUtils.readXml(element, 'my_cmyk_color', context)
        self.assertTrue(res.isValid())
        self.assertEqual(res.spec(), QColor.Cmyk)
        self.assertAlmostEqual(res.cyanF(), 1 / 65536, 4)
        self.assertAlmostEqual(res.magentaF(), 2 / 65536, 4)
        self.assertAlmostEqual(res.yellowF(), 3 / 65536, 4)
        self.assertAlmostEqual(res.blackF(), 4 / 65536, 4)
        self.assertAlmostEqual(res.alphaF(), 5 / 65536, 5)

        # missing color
        res = QgsColorUtils.readXml(element, 'not there', context)
        self.assertFalse(res.isValid())

    def test_color_string(self):
        """
        Test storing/restoring colors from strings
        """

        # invalid color
        string = QgsColorUtils.colorToString(QColor())
        res = QgsColorUtils.colorFromString(string)
        self.assertFalse(res.isValid())

        # rgb color
        color = QColor.fromRgbF(1 / 65536, 2 / 65536, 3 / 65536, 4 / 65536)
        string = QgsColorUtils.colorToString(color)
        res = QgsColorUtils.colorFromString(string)
        self.assertTrue(res.isValid())
        self.assertEqual(res.spec(), QColor.Rgb)
        self.assertAlmostEqual(res.redF(), 1 / 65536, 5)
        self.assertAlmostEqual(res.greenF(), 2 / 65536, 5)
        self.assertAlmostEqual(res.blueF(), 3 / 65536, 5)
        self.assertAlmostEqual(res.alphaF(), 4 / 65536, 5)

        color = QColor.fromRgb(16, 17, 18, 20)
        string = QgsColorUtils.colorToString(color)
        res = QgsColorUtils.colorFromString(string)
        self.assertTrue(res.isValid())
        self.assertEqual(res.spec(), QColor.Rgb)
        self.assertEqual(res.red(), 16)
        self.assertEqual(res.green(), 17)
        self.assertEqual(res.blue(), 18)
        self.assertEqual(res.alpha(), 20)

        # rgb extended color
        if TestQgsColorUtils.has_extended_rgb:
            color = QColor.fromRgbF(-1 / 65536, 2 / 65536, 3 / 65536, 4 / 65536)
            string = QgsColorUtils.colorToString(color)
            res = QgsColorUtils.colorFromString(string)
            self.assertTrue(res.isValid())
            self.assertEqual(res.spec(), QColor.ExtendedRgb)
            self.assertAlmostEqual(res.redF(), -1 / 65536, 5)
            self.assertAlmostEqual(res.greenF(), 2 / 65536, 5)
            self.assertAlmostEqual(res.blueF(), 3 / 65536, 5)
            self.assertAlmostEqual(res.alphaF(), 4 / 65536, 5)

        # hsv color
        color = QColor.fromHsvF(1 / 65536, 2 / 65536, 3 / 65536, 4 / 65536)
        string = QgsColorUtils.colorToString(color)
        res = QgsColorUtils.colorFromString(string)
        self.assertTrue(res.isValid())
        self.assertEqual(res.spec(), QColor.Hsv)
        self.assertAlmostEqual(res.hueF(), 1 / 65536, 4)
        self.assertAlmostEqual(res.hsvSaturationF(), 2 / 65536, 5)
        self.assertAlmostEqual(res.valueF(), 3 / 65536, 5)
        self.assertAlmostEqual(res.alphaF(), 4 / 65536, 5)

        # hsl color
        color = QColor.fromHslF(111 / 65536, 12222 / 65536, 333 / 65536, 4 / 65536)
        string = QgsColorUtils.colorToString(color)
        res = QgsColorUtils.colorFromString(string)
        self.assertTrue(res.isValid())
        self.assertEqual(res.spec(), QColor.Hsl)
        self.assertAlmostEqual(res.hslHueF(), 111 / 65536, 5)
        self.assertAlmostEqual(res.hslSaturationF(), 12222 / 65536, 5)
        self.assertAlmostEqual(res.lightnessF(), 333 / 65536, 4)
        self.assertAlmostEqual(res.alphaF(), 4 / 65536, 5)

        # cmyk color
        color = QColor.fromCmykF(1 / 65536, 2 / 65536, 3 / 65536, 4 / 65536, 255 / 65536)
        string = QgsColorUtils.colorToString(color)
        res = QgsColorUtils.colorFromString(string)
        self.assertTrue(res.isValid())
        self.assertEqual(res.spec(), QColor.Cmyk)
        self.assertAlmostEqual(res.cyanF(), 1 / 65536, 4)
        self.assertAlmostEqual(res.magentaF(), 2 / 65536, 4)
        self.assertAlmostEqual(res.yellowF(), 3 / 65536, 4)
        self.assertAlmostEqual(res.blackF(), 4 / 65536, 4)
        self.assertAlmostEqual(res.alphaF(), 255 / 65536, 5)

        # invalid string
        res = QgsColorUtils.colorFromString('')
        self.assertFalse(res.isValid())
        res = QgsColorUtils.colorFromString('x')
        self.assertFalse(res.isValid())
        res = QgsColorUtils.colorFromString('2')
        self.assertFalse(res.isValid())

    def test_color_string_compat(self):
        """
        Test storing/restoring colors from string compatibility with deprecated methods
        """

        # rgb color
        color = QColor.fromRgbF(50 / 255, 100 / 255, 150 / 255, 200 / 255)
        string = QgsColorUtils.colorToString(color)
        res = QgsSymbolLayerUtils.decodeColor(string)
        self.assertTrue(res.isValid())
        self.assertEqual(res.red(), 50)
        self.assertEqual(res.green(), 100)
        self.assertEqual(res.blue(), 150)
        self.assertEqual(res.alpha(), 200)
        string = QgsSymbolLayerUtils.encodeColor(color)
        res = QgsColorUtils.colorFromString(string)
        self.assertTrue(res.isValid())
        self.assertEqual(res.red(), 50)
        self.assertEqual(res.green(), 100)
        self.assertEqual(res.blue(), 150)
        self.assertEqual(res.alpha(), 200)

        # rgb extended color
        if TestQgsColorUtils.has_extended_rgb:
            color = QColor.fromRgbF(-50 / 255, 100 / 255, 150 / 255, 200 / 255)
            string = QgsColorUtils.colorToString(color)
            res = QgsSymbolLayerUtils.decodeColor(string)
            self.assertTrue(res.isValid())
            self.assertEqual(res.red(), 0)
            self.assertEqual(res.green(), 100)
            self.assertEqual(res.blue(), 150)
            self.assertEqual(res.alpha(), 200)
            string = QgsSymbolLayerUtils.encodeColor(color)
            res = QgsColorUtils.colorFromString(string)
            self.assertTrue(res.isValid())
            self.assertEqual(res.red(), 0)
            self.assertEqual(res.green(), 100)
            self.assertEqual(res.blue(), 150)
            self.assertEqual(res.alpha(), 200)

        # hsv color
        color = QColor.fromHsvF(50 / 360, 100 / 255, 150 / 255, 200 / 255)
        string = QgsColorUtils.colorToString(color)
        res = QgsSymbolLayerUtils.decodeColor(string)
        self.assertTrue(res.isValid())
        self.assertEqual(res.red(), 150)
        self.assertEqual(res.green(), 140)
        self.assertEqual(res.blue(), 91)
        self.assertEqual(res.alpha(), 200)
        string = QgsSymbolLayerUtils.encodeColor(color)
        res = QgsColorUtils.colorFromString(string)
        self.assertTrue(res.isValid())
        self.assertEqual(res.red(), 150)
        self.assertEqual(res.green(), 140)
        self.assertEqual(res.blue(), 91)
        self.assertEqual(res.alpha(), 200)

        # hsl color
        color = QColor.fromHslF(50 / 360, 100 / 255, 150 / 255, 200 / 255)
        string = QgsColorUtils.colorToString(color)
        res = QgsSymbolLayerUtils.decodeColor(string)
        self.assertTrue(res.isValid())
        self.assertAlmostEqual(res.red(), 191, delta=1)
        self.assertAlmostEqual(res.green(), 177, delta=1)
        self.assertAlmostEqual(res.blue(), 109, delta=1)
        self.assertAlmostEqual(res.alpha(), 200)
        string = QgsSymbolLayerUtils.encodeColor(color)
        res = QgsColorUtils.colorFromString(string)
        self.assertTrue(res.isValid())
        self.assertAlmostEqual(res.red(), 191, delta=1)
        self.assertAlmostEqual(res.green(), 177, delta=1)
        self.assertAlmostEqual(res.blue(), 109, delta=1)
        self.assertEqual(res.alpha(), 200)

        # cmyk color
        color = QColor.fromCmykF(50 / 255, 100 / 255, 150 / 255, 200 / 255, 220 / 255)
        string = QgsColorUtils.colorToString(color)
        res = QgsSymbolLayerUtils.decodeColor(string)
        self.assertTrue(res.isValid())
        self.assertEqual(res.red(), 44)
        self.assertEqual(res.green(), 33)
        self.assertAlmostEqual(res.blue(), 23, delta=1)
        self.assertEqual(res.alpha(), 220)
        string = QgsSymbolLayerUtils.encodeColor(color)
        res = QgsColorUtils.colorFromString(string)
        self.assertTrue(res.isValid())
        self.assertEqual(res.red(), 44)
        self.assertEqual(res.green(), 33)
        self.assertAlmostEqual(res.blue(), 23, delta=1)
        self.assertEqual(res.alpha(), 220)


if __name__ == '__main__':
    unittest.main()
