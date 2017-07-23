# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsLayoutSnapper.

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
                       QgsLayoutSnapper,
                       QgsLayoutMeasurement,
                       QgsUnitTypes,
                       QgsLayoutPoint)
from qgis.PyQt.QtCore import QRectF
from qgis.PyQt.QtGui import (QTransform,
                             QPen,
                             QColor)

from qgis.testing import start_app, unittest

start_app()


class TestQgsLayoutSnapper(unittest.TestCase):

    def testGettersSetters(self):
        s = QgsLayoutSnapper()
        s.setGridResolution(QgsLayoutMeasurement(5, QgsUnitTypes.LayoutPoints))
        self.assertEqual(s.gridResolution().length(), 5.0)
        self.assertEqual(s.gridResolution().units(), QgsUnitTypes.LayoutPoints)

        s.setGridOffset(QgsLayoutPoint(6, 7, QgsUnitTypes.LayoutPixels))
        self.assertEqual(s.gridOffset().x(), 6.0)
        self.assertEqual(s.gridOffset().y(), 7.0)
        self.assertEqual(s.gridOffset().units(), QgsUnitTypes.LayoutPixels)

        s.setGridPen(QPen(QColor(255, 0, 255)))
        self.assertEqual(s.gridPen().color().name(), QColor(255, 0, 255).name())

        s.setGridStyle(QgsLayoutSnapper.GridDots)
        self.assertEqual(s.gridStyle(), QgsLayoutSnapper.GridDots)


if __name__ == '__main__':
    unittest.main()
