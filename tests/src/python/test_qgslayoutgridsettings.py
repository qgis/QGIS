# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsLayoutGridSettings.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '05/07/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.core import (QgsProject,
                       QgsLayout,
                       QgsLayoutGridSettings,
                       QgsLayoutMeasurement,
                       QgsUnitTypes,
                       QgsLayoutPoint,
                       QgsLayoutItemPage)
from qgis.PyQt.QtGui import (QPen,
                             QColor)

from qgis.testing import start_app, unittest

start_app()


class TestQgsLayoutGridSettings(unittest.TestCase):

    def testGettersSetters(self):
        p = QgsProject()
        l = QgsLayout(p)
        s = QgsLayoutGridSettings()
        s.setResolution(QgsLayoutMeasurement(5, QgsUnitTypes.LayoutPoints))
        self.assertEqual(s.resolution().length(), 5.0)
        self.assertEqual(s.resolution().units(), QgsUnitTypes.LayoutPoints)

        s.setOffset(QgsLayoutPoint(6, 7, QgsUnitTypes.LayoutPixels))
        self.assertEqual(s.offset().x(), 6.0)
        self.assertEqual(s.offset().y(), 7.0)
        self.assertEqual(s.offset().units(), QgsUnitTypes.LayoutPixels)

        s.setPen(QPen(QColor(255, 0, 255)))
        self.assertEqual(s.pen().color().name(), QColor(255, 0, 255).name())

        s.setStyle(QgsLayoutGridSettings.StyleDots)
        self.assertEqual(s.style(), QgsLayoutGridSettings.StyleDots)


if __name__ == '__main__':
    unittest.main()
