# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsLayoutAligner

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '3/10/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA
import sip

from qgis.core import (QgsUnitTypes,
                       QgsLayout,
                       QgsLayoutAligner,
                       QgsLayoutItemPage,
                       QgsLayoutGuide,
                       QgsLayoutObject,
                       QgsProject,
                       QgsProperty,
                       QgsLayoutPageCollection,
                       QgsLayoutMeasurement,
                       QgsLayoutItemMap,
                       QgsLayoutSize,
                       QgsLayoutPoint)
from qgis.PyQt.QtCore import Qt, QCoreApplication, QEvent, QPointF, QRectF
from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtXml import QDomDocument

from qgis.testing import start_app, unittest

start_app()


class TestQgsLayoutAligner(unittest.TestCase):

    def testAlign(self):
        p = QgsProject()
        l = QgsLayout(p)

        # add some items
        item1 = QgsLayoutItemMap(l)
        item1.attemptMove(QgsLayoutPoint(4, 8, QgsUnitTypes.LayoutMillimeters))
        item1.attemptResize(QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        l.addItem(item1)
        item2 = QgsLayoutItemMap(l)
        item2.attemptMove(QgsLayoutPoint(6, 10, QgsUnitTypes.LayoutMillimeters))
        item2.attemptResize(QgsLayoutSize(10, 9, QgsUnitTypes.LayoutMillimeters))
        l.addItem(item2)
        item3 = QgsLayoutItemMap(l)
        item3.attemptMove(QgsLayoutPoint(0.8, 1.2, QgsUnitTypes.LayoutCentimeters))
        item3.attemptResize(QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutCentimeters))
        l.addItem(item3)

        QgsLayoutAligner.alignItems(l, [item1, item2, item3], QgsLayoutAligner.Left)
        self.assertEqual(item1.positionWithUnits(), QgsLayoutPoint(4, 8, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item1.sizeWithUnits(), QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.positionWithUnits(), QgsLayoutPoint(4, 10, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.sizeWithUnits(), QgsLayoutSize(10, 9, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item3.positionWithUnits(), QgsLayoutPoint(0.4, 1.2, QgsUnitTypes.LayoutCentimeters))
        self.assertEqual(item3.sizeWithUnits(), QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutCentimeters))

        QgsLayoutAligner.alignItems(l, [item1, item2, item3], QgsLayoutAligner.HCenter)
        self.assertEqual(item1.positionWithUnits(), QgsLayoutPoint(4, 8, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item1.sizeWithUnits(), QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.positionWithUnits(), QgsLayoutPoint(8, 10, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.sizeWithUnits(), QgsLayoutSize(10, 9, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item3.positionWithUnits(), QgsLayoutPoint(0.4, 1.2, QgsUnitTypes.LayoutCentimeters))
        self.assertEqual(item3.sizeWithUnits(), QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutCentimeters))

        QgsLayoutAligner.alignItems(l, [item1, item2, item3], QgsLayoutAligner.Right)
        self.assertEqual(item1.positionWithUnits(), QgsLayoutPoint(4, 8, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item1.sizeWithUnits(), QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.positionWithUnits(), QgsLayoutPoint(12, 10, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.sizeWithUnits(), QgsLayoutSize(10, 9, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item3.positionWithUnits(), QgsLayoutPoint(0.4, 1.2, QgsUnitTypes.LayoutCentimeters))
        self.assertEqual(item3.sizeWithUnits(), QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutCentimeters))

        QgsLayoutAligner.alignItems(l, [item1, item2, item3], QgsLayoutAligner.Top)
        self.assertEqual(item1.positionWithUnits(), QgsLayoutPoint(4, 8, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item1.sizeWithUnits(), QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.positionWithUnits(), QgsLayoutPoint(12, 8, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.sizeWithUnits(), QgsLayoutSize(10, 9, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item3.positionWithUnits(), QgsLayoutPoint(0.4, 0.8, QgsUnitTypes.LayoutCentimeters))
        self.assertEqual(item3.sizeWithUnits(), QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutCentimeters))

        QgsLayoutAligner.alignItems(l, [item1, item2, item3], QgsLayoutAligner.VCenter)
        self.assertEqual(item1.positionWithUnits(), QgsLayoutPoint(4, 10, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item1.sizeWithUnits(), QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.positionWithUnits(), QgsLayoutPoint(12, 11.5, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.sizeWithUnits(), QgsLayoutSize(10, 9, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item3.positionWithUnits(), QgsLayoutPoint(0.4, 0.8, QgsUnitTypes.LayoutCentimeters))
        self.assertEqual(item3.sizeWithUnits(), QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutCentimeters))

        QgsLayoutAligner.alignItems(l, [item1, item2, item3], QgsLayoutAligner.Bottom)
        self.assertEqual(item1.positionWithUnits(), QgsLayoutPoint(4, 12, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item1.sizeWithUnits(), QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.positionWithUnits(), QgsLayoutPoint(12, 15, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.sizeWithUnits(), QgsLayoutSize(10, 9, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item3.positionWithUnits(), QgsLayoutPoint(0.4, 0.8, QgsUnitTypes.LayoutCentimeters))
        self.assertEqual(item3.sizeWithUnits(), QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutCentimeters))


if __name__ == '__main__':
    unittest.main()
