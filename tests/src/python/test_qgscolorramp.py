# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsColorRamp subclasses.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Alexander Bruy'
__date__ = '26/05/2019'
__copyright__ = 'Copyright 2019, The QGIS Project'

import qgis  # NOQA

from qgis.testing import unittest, start_app
from qgis.core import QgsColorBrewerColorRamp, QgsCptCityColorRamp, QgsGradientColorRamp, QgsSettings, QgsApplication
from qgis.PyQt.QtCore import QCoreApplication
from qgis.PyQt.QtGui import QColor


class TestQgsColorRamp(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("QGIS_TestQgsColorRamp.com")
        QCoreApplication.setApplicationName("QGIS_TestQgsColorRamp")
        QgsSettings().clear()
        start_app()

    def testColorBrewerColorRamp(self):
        """Test Color Brewer color ramp"""
        ramp = QgsColorBrewerColorRamp(schemeName="Spectral", colors=5, inverted=False)
        self.assertTrue(ramp)

        # color scheme name
        self.assertEqual(ramp.schemeName(), "Spectral")

        # number of colors
        self.assertEqual(ramp.colors(), 5)
        ramp.setColors(13)
        self.assertEqual(ramp.colors(), 13)

        # color for value
        ramp.setColors(5)
        self.assertEqual(ramp.color(0).name(), "#d7191c")
        self.assertEqual(ramp.color(1).name(), "#2b83ba")
        self.assertEqual(ramp.color(0.5).name(), "#ffffbf")
        self.assertEqual(ramp.color(2).name(), "#000000")
        self.assertEqual(ramp.color(-1).name(), "#000000")
        self.assertEqual(ramp.color(float('nan')).name(), "#000000")

    def testCptCityColorRamp(self):
        """Test Cpt-city color ramp"""
        ramp = QgsCptCityColorRamp(schemeName="cb/div/BrBG_", variantName="05", inverted=False)
        self.assertTrue(ramp)

        # associated files
        self.assertTrue(ramp.fileLoaded())
        fileName = ramp.fileName()[len(QgsApplication.pkgDataPath()):]
        self.assertEqual(fileName, "/resources/cpt-city-qgis-min/cb/div/BrBG_05.svg")
        fileName = ramp.copyingFileName()[len(QgsApplication.pkgDataPath()):]
        self.assertEqual(fileName, "/resources/cpt-city-qgis-min/cb/COPYING.xml")
        fileName = ramp.descFileName()[len(QgsApplication.pkgDataPath()):]
        self.assertEqual(fileName, "/resources/cpt-city-qgis-min/cb/div/DESC.xml")

        # color scheme name
        self.assertEqual(ramp.schemeName(), "cb/div/BrBG_")
        self.assertEqual(ramp.variantName(), "05")

        # number of colors
        self.assertEqual(ramp.count(), 6)

        # color for value
        self.assertEqual(ramp.color(0).name(), "#a6611a")
        self.assertEqual(ramp.color(1).name(), "#018571")
        self.assertEqual(ramp.color(0.5).name(), "#f5f5f5")
        self.assertEqual(ramp.color(2).name(), "#018571")
        self.assertEqual(ramp.color(-1).name(), "#a6611a")
        self.assertEqual(ramp.color(float('nan')).name(), "#018571")

    def testGradientColorRamp(self):
        """Test gradient color ramp"""
        ramp = QgsGradientColorRamp(color1=QColor(0, 0, 255), color2=QColor(0, 255, 0), discrete=False)
        self.assertTrue(ramp)

        # number of colors
        self.assertEqual(ramp.count(), 2)
        self.assertEqual(ramp.color1().name(), "#0000ff")
        self.assertEqual(ramp.color2().name(), "#00ff00")

        # color for value
        self.assertEqual(ramp.color(0).name(), "#0000ff")
        self.assertEqual(ramp.color(1).name(), "#00ff00")
        self.assertEqual(ramp.color(0.5).name(), "#008080")
        self.assertEqual(ramp.color(2).name(), "#00ff00")
        self.assertEqual(ramp.color(-1).name(), "#0000ff")
        self.assertEqual(ramp.color(float('nan')).name(), "#00ff00")


if __name__ == "__main__":
    unittest.main()
