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
from qgis.PyQt import sip

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
                       QgsLayoutItemPicture,
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
        item1 = QgsLayoutItemPicture(l)
        item1.attemptMove(QgsLayoutPoint(4, 8, QgsUnitTypes.LayoutMillimeters))
        item1.attemptResize(QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        l.addItem(item1)
        item2 = QgsLayoutItemPicture(l)
        item2.attemptMove(QgsLayoutPoint(6, 10, QgsUnitTypes.LayoutMillimeters))
        item2.attemptResize(QgsLayoutSize(10, 9, QgsUnitTypes.LayoutMillimeters))
        l.addItem(item2)
        item3 = QgsLayoutItemPicture(l)
        item3.attemptMove(QgsLayoutPoint(0.8, 1.2, QgsUnitTypes.LayoutCentimeters))
        item3.attemptResize(QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutCentimeters))
        l.addItem(item3)

        QgsLayoutAligner.alignItems(l, [item1, item2, item3], QgsLayoutAligner.AlignLeft)
        self.assertEqual(item1.positionWithUnits(), QgsLayoutPoint(4, 8, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item1.sizeWithUnits(), QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.positionWithUnits(), QgsLayoutPoint(4, 10, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.sizeWithUnits(), QgsLayoutSize(10, 9, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item3.positionWithUnits(), QgsLayoutPoint(0.4, 1.2, QgsUnitTypes.LayoutCentimeters))
        self.assertEqual(item3.sizeWithUnits(), QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutCentimeters))

        QgsLayoutAligner.alignItems(l, [item1, item2, item3], QgsLayoutAligner.AlignHCenter)
        self.assertEqual(item1.positionWithUnits(), QgsLayoutPoint(4, 8, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item1.sizeWithUnits(), QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.positionWithUnits(), QgsLayoutPoint(8, 10, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.sizeWithUnits(), QgsLayoutSize(10, 9, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item3.positionWithUnits(), QgsLayoutPoint(0.4, 1.2, QgsUnitTypes.LayoutCentimeters))
        self.assertEqual(item3.sizeWithUnits(), QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutCentimeters))

        QgsLayoutAligner.alignItems(l, [item1, item2, item3], QgsLayoutAligner.AlignRight)
        self.assertEqual(item1.positionWithUnits(), QgsLayoutPoint(4, 8, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item1.sizeWithUnits(), QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.positionWithUnits(), QgsLayoutPoint(12, 10, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.sizeWithUnits(), QgsLayoutSize(10, 9, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item3.positionWithUnits(), QgsLayoutPoint(0.4, 1.2, QgsUnitTypes.LayoutCentimeters))
        self.assertEqual(item3.sizeWithUnits(), QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutCentimeters))

        QgsLayoutAligner.alignItems(l, [item1, item2, item3], QgsLayoutAligner.AlignTop)
        self.assertEqual(item1.positionWithUnits(), QgsLayoutPoint(4, 8, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item1.sizeWithUnits(), QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.positionWithUnits(), QgsLayoutPoint(12, 8, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.sizeWithUnits(), QgsLayoutSize(10, 9, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item3.positionWithUnits(), QgsLayoutPoint(0.4, 0.8, QgsUnitTypes.LayoutCentimeters))
        self.assertEqual(item3.sizeWithUnits(), QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutCentimeters))

        QgsLayoutAligner.alignItems(l, [item1, item2, item3], QgsLayoutAligner.AlignVCenter)
        self.assertEqual(item1.positionWithUnits(), QgsLayoutPoint(4, 10, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item1.sizeWithUnits(), QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.positionWithUnits(), QgsLayoutPoint(12, 11.5, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.sizeWithUnits(), QgsLayoutSize(10, 9, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item3.positionWithUnits(), QgsLayoutPoint(0.4, 0.8, QgsUnitTypes.LayoutCentimeters))
        self.assertEqual(item3.sizeWithUnits(), QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutCentimeters))

        QgsLayoutAligner.alignItems(l, [item1, item2, item3], QgsLayoutAligner.AlignBottom)
        self.assertEqual(item1.positionWithUnits(), QgsLayoutPoint(4, 12, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item1.sizeWithUnits(), QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.positionWithUnits(), QgsLayoutPoint(12, 15, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.sizeWithUnits(), QgsLayoutSize(10, 9, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item3.positionWithUnits(), QgsLayoutPoint(0.4, 0.8, QgsUnitTypes.LayoutCentimeters))
        self.assertEqual(item3.sizeWithUnits(), QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutCentimeters))

    def testDistribute(self):
        p = QgsProject()
        l = QgsLayout(p)

        # add some items
        item1 = QgsLayoutItemPicture(l)
        item1.attemptMove(QgsLayoutPoint(4, 8, QgsUnitTypes.LayoutMillimeters))
        item1.attemptResize(QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        l.addItem(item1)
        item2 = QgsLayoutItemPicture(l)
        item2.attemptMove(QgsLayoutPoint(7, 10, QgsUnitTypes.LayoutMillimeters))
        item2.attemptResize(QgsLayoutSize(10, 9, QgsUnitTypes.LayoutMillimeters))
        l.addItem(item2)
        item3 = QgsLayoutItemPicture(l)
        item3.attemptMove(QgsLayoutPoint(0.8, 1.2, QgsUnitTypes.LayoutCentimeters))
        item3.attemptResize(QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutCentimeters))
        l.addItem(item3)

        QgsLayoutAligner.distributeItems(l, [item1, item2, item3], QgsLayoutAligner.DistributeLeft)
        self.assertEqual(item1.positionWithUnits(), QgsLayoutPoint(4, 8, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item1.sizeWithUnits(), QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        self.assertAlmostEqual(item2.positionWithUnits().x(), 6.0, 3)
        self.assertEqual(item2.positionWithUnits().units(), QgsUnitTypes.LayoutMillimeters)
        self.assertEqual(item2.sizeWithUnits(), QgsLayoutSize(10, 9, QgsUnitTypes.LayoutMillimeters))
        self.assertAlmostEqual(item3.positionWithUnits().x(), 0.8, 3)
        self.assertEqual(item3.positionWithUnits().units(), QgsUnitTypes.LayoutCentimeters)
        self.assertEqual(item3.sizeWithUnits(), QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutCentimeters))

        QgsLayoutAligner.distributeItems(l, [item1, item2, item3], QgsLayoutAligner.DistributeHCenter)
        self.assertAlmostEqual(item1.positionWithUnits().x(), 5.0, 3)
        self.assertEqual(item1.positionWithUnits().units(), QgsUnitTypes.LayoutMillimeters)
        self.assertEqual(item1.sizeWithUnits(), QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        self.assertAlmostEqual(item2.positionWithUnits().x(), 6.0, 3)
        self.assertEqual(item2.positionWithUnits().units(), QgsUnitTypes.LayoutMillimeters)
        self.assertEqual(item2.sizeWithUnits(), QgsLayoutSize(10, 9, QgsUnitTypes.LayoutMillimeters))
        self.assertAlmostEqual(item3.positionWithUnits().x(), 0.8, 3)
        self.assertEqual(item3.positionWithUnits().units(), QgsUnitTypes.LayoutCentimeters)
        self.assertEqual(item3.sizeWithUnits(), QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutCentimeters))

        QgsLayoutAligner.distributeItems(l, [item1, item2, item3], QgsLayoutAligner.DistributeRight)
        self.assertAlmostEqual(item1.positionWithUnits().x(), 3.0, 3)
        self.assertEqual(item1.positionWithUnits().units(), QgsUnitTypes.LayoutMillimeters)
        self.assertEqual(item1.sizeWithUnits(), QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        self.assertAlmostEqual(item2.positionWithUnits().x(), 6.0, 3)
        self.assertEqual(item2.positionWithUnits().units(), QgsUnitTypes.LayoutMillimeters)
        self.assertEqual(item2.sizeWithUnits(), QgsLayoutSize(10, 9, QgsUnitTypes.LayoutMillimeters))
        self.assertAlmostEqual(item3.positionWithUnits().x(), 0.8, 3)
        self.assertEqual(item3.positionWithUnits().units(), QgsUnitTypes.LayoutCentimeters)
        self.assertEqual(item3.sizeWithUnits(), QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutCentimeters))

        QgsLayoutAligner.distributeItems(l, [item1, item2, item3], QgsLayoutAligner.DistributeTop)
        self.assertAlmostEqual(item1.positionWithUnits().y(), 8.0, 3)
        self.assertEqual(item1.positionWithUnits().units(), QgsUnitTypes.LayoutMillimeters)
        self.assertEqual(item1.sizeWithUnits(), QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        self.assertAlmostEqual(item2.positionWithUnits().y(), 10.0, 3)
        self.assertEqual(item2.positionWithUnits().units(), QgsUnitTypes.LayoutMillimeters)
        self.assertEqual(item2.sizeWithUnits(), QgsLayoutSize(10, 9, QgsUnitTypes.LayoutMillimeters))
        self.assertAlmostEqual(item3.positionWithUnits().y(), 1.2, 3)
        self.assertEqual(item3.positionWithUnits().units(), QgsUnitTypes.LayoutCentimeters)
        self.assertEqual(item3.sizeWithUnits(), QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutCentimeters))

        QgsLayoutAligner.distributeItems(l, [item1, item2, item3], QgsLayoutAligner.DistributeVCenter)
        self.assertAlmostEqual(item1.positionWithUnits().y(), 8.0, 3)
        self.assertEqual(item1.positionWithUnits().units(), QgsUnitTypes.LayoutMillimeters)
        self.assertEqual(item1.sizeWithUnits(), QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        self.assertAlmostEqual(item2.positionWithUnits().y(), 12.5, 3)
        self.assertEqual(item2.positionWithUnits().units(), QgsUnitTypes.LayoutMillimeters)
        self.assertEqual(item2.sizeWithUnits(), QgsLayoutSize(10, 9, QgsUnitTypes.LayoutMillimeters))
        self.assertAlmostEqual(item3.positionWithUnits().y(), 1.2, 3)
        self.assertEqual(item3.positionWithUnits().units(), QgsUnitTypes.LayoutCentimeters)
        self.assertEqual(item3.sizeWithUnits(), QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutCentimeters))

        QgsLayoutAligner.distributeItems(l, [item1, item2, item3], QgsLayoutAligner.DistributeBottom)
        self.assertAlmostEqual(item1.positionWithUnits().y(), 8.0, 3)
        self.assertEqual(item1.positionWithUnits().units(), QgsUnitTypes.LayoutMillimeters)
        self.assertEqual(item1.sizeWithUnits(), QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        self.assertAlmostEqual(item2.positionWithUnits().y(), 15.0, 3)
        self.assertEqual(item2.positionWithUnits().units(), QgsUnitTypes.LayoutMillimeters)
        self.assertEqual(item2.sizeWithUnits(), QgsLayoutSize(10, 9, QgsUnitTypes.LayoutMillimeters))
        self.assertAlmostEqual(item3.positionWithUnits().y(), 1.2, 3)
        self.assertEqual(item3.positionWithUnits().units(), QgsUnitTypes.LayoutCentimeters)
        self.assertEqual(item3.sizeWithUnits(), QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutCentimeters))

    def testResize(self):
        p = QgsProject()
        l = QgsLayout(p)

        # add some items
        item1 = QgsLayoutItemPicture(l)
        item1.attemptMove(QgsLayoutPoint(4, 8, QgsUnitTypes.LayoutMillimeters))
        item1.attemptResize(QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        l.addItem(item1)
        item2 = QgsLayoutItemPicture(l)
        item2.attemptMove(QgsLayoutPoint(7, 10, QgsUnitTypes.LayoutMillimeters))
        item2.attemptResize(QgsLayoutSize(10, 9, QgsUnitTypes.LayoutMillimeters))
        l.addItem(item2)
        item3 = QgsLayoutItemPicture(l)
        item3.attemptMove(QgsLayoutPoint(0.8, 1.2, QgsUnitTypes.LayoutCentimeters))
        item3.attemptResize(QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutCentimeters))
        l.addItem(item3)

        QgsLayoutAligner.resizeItems(l, [item1, item2, item3], QgsLayoutAligner.ResizeNarrowest)
        self.assertEqual(item1.sizeWithUnits(), QgsLayoutSize(10, 12, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.sizeWithUnits(), QgsLayoutSize(10, 9, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item3.sizeWithUnits(), QgsLayoutSize(1.0, 1.6, QgsUnitTypes.LayoutCentimeters))
        l.undoStack().stack().undo()

        QgsLayoutAligner.resizeItems(l, [item1, item2, item3], QgsLayoutAligner.ResizeWidest)
        self.assertEqual(item1.sizeWithUnits(), QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.sizeWithUnits(), QgsLayoutSize(18, 9, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item3.sizeWithUnits(), QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutCentimeters))
        l.undoStack().stack().undo()

        QgsLayoutAligner.resizeItems(l, [item1, item2, item3], QgsLayoutAligner.ResizeShortest)
        self.assertEqual(item1.sizeWithUnits(), QgsLayoutSize(18, 9, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.sizeWithUnits(), QgsLayoutSize(10, 9, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item3.sizeWithUnits(), QgsLayoutSize(1.8, 0.9, QgsUnitTypes.LayoutCentimeters))
        l.undoStack().stack().undo()

        QgsLayoutAligner.resizeItems(l, [item1, item2, item3], QgsLayoutAligner.ResizeTallest)
        self.assertEqual(item1.sizeWithUnits(), QgsLayoutSize(18, 16, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.sizeWithUnits(), QgsLayoutSize(10, 16, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item3.sizeWithUnits(), QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutCentimeters))
        l.undoStack().stack().undo()

        item2.attemptResize(QgsLayoutSize(10, 19, QgsUnitTypes.LayoutMillimeters))
        QgsLayoutAligner.resizeItems(l, [item1, item2, item3], QgsLayoutAligner.ResizeToSquare)
        self.assertEqual(item1.sizeWithUnits(), QgsLayoutSize(18, 18, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item2.sizeWithUnits(), QgsLayoutSize(19, 19, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(item3.sizeWithUnits(), QgsLayoutSize(1.8, 1.8, QgsUnitTypes.LayoutCentimeters))

        l.undoStack().stack().undo()
        QgsLayoutAligner.resizeItems(l, [item1], QgsLayoutAligner.ResizeToSquare)
        self.assertEqual(item1.sizeWithUnits(), QgsLayoutSize(18, 18, QgsUnitTypes.LayoutMillimeters))


if __name__ == '__main__':
    unittest.main()
