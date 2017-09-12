# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsLayoutItem.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2017 by Nyall Dawson'
__date__ = '17/01/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'
import qgis  # NOQA

from qgis.testing import start_app, unittest
from qgis.core import (QgsProject,
                       QgsLayout,
                       QgsLayoutItemMap,
                       QgsLayoutObject,
                       QgsProperty,
                       QgsLayoutMeasurement,
                       QgsUnitTypes)
from qgis.PyQt.QtGui import QColor


start_app()


class TestQgsLayoutItem(unittest.TestCase):

    def testDataDefinedFrameColor(self):
        layout = QgsLayout(QgsProject.instance())

        item = QgsLayoutItemMap(layout)
        item.setFrameEnabled(True)

        item.setFrameStrokeColor(QColor(255, 0, 0))
        self.assertEqual(item.frameStrokeColor(), QColor(255, 0, 0))
        self.assertEqual(item.pen().color().name(), QColor(255, 0, 0).name())

        item.dataDefinedProperties().setProperty(QgsLayoutObject.FrameColor, QgsProperty.fromExpression("'blue'"))
        item.refreshDataDefinedProperty()
        self.assertEqual(item.frameStrokeColor(), QColor(255, 0, 0))  # should not change
        self.assertEqual(item.pen().color().name(), QColor(0, 0, 255).name())

    def testFrameWidth(self):
        layout = QgsLayout(QgsProject.instance())

        item = QgsLayoutItemMap(layout)
        item.setFrameEnabled(True)

        item.setFrameStrokeWidth(QgsLayoutMeasurement(10, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item.frameStrokeWidth(), QgsLayoutMeasurement(10, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item.pen().width(), 10.0)

        item.setFrameStrokeWidth(QgsLayoutMeasurement(10, QgsUnitTypes.LayoutCentimeters))
        self.assertEqual(item.frameStrokeWidth(), QgsLayoutMeasurement(10, QgsUnitTypes.LayoutCentimeters))
        self.assertEqual(item.pen().width(), 100.0)

    def testDataDefinedBackgroundColor(self):
        layout = QgsLayout(QgsProject.instance())

        item = QgsLayoutItemMap(layout)

        item.setBackgroundColor(QColor(255, 0, 0))
        self.assertEqual(item.backgroundColor(), QColor(255, 0, 0))
        self.assertEqual(item.brush().color().name(), QColor(255, 0, 0).name())

        item.dataDefinedProperties().setProperty(QgsLayoutObject.BackgroundColor, QgsProperty.fromExpression("'blue'"))
        item.refreshDataDefinedProperty()
        self.assertEqual(item.backgroundColor(), QColor(255, 0, 0))  # should not change
        self.assertEqual(item.brush().color().name(), QColor(0, 0, 255).name())


if __name__ == '__main__':
    unittest.main()
