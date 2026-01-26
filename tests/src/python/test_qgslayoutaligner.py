"""QGIS Unit tests for QgsLayoutAligner

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "3/10/2017"
__copyright__ = "Copyright 2017, The QGIS Project"


from qgis.core import (
    QgsLayout,
    QgsLayoutAligner,
    QgsLayoutItem,
    QgsLayoutItemPicture,
    QgsLayoutPoint,
    QgsLayoutSize,
    QgsProject,
    QgsUnitTypes,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsLayoutAligner(QgisTestCase):

    def testAlign(self):
        p = QgsProject()
        l = QgsLayout(p)

        # add some items
        item1 = QgsLayoutItemPicture(l)
        item1.attemptMove(
            QgsLayoutPoint(4, 8, QgsUnitTypes.LayoutUnit.LayoutMillimeters)
        )
        item1.attemptResize(
            QgsLayoutSize(18, 12, QgsUnitTypes.LayoutUnit.LayoutMillimeters)
        )
        l.addItem(item1)
        item2 = QgsLayoutItemPicture(l)
        item2.attemptMove(
            QgsLayoutPoint(6, 10, QgsUnitTypes.LayoutUnit.LayoutMillimeters)
        )
        item2.attemptResize(
            QgsLayoutSize(10, 9, QgsUnitTypes.LayoutUnit.LayoutMillimeters)
        )
        item2.setReferencePoint(QgsLayoutItem.ReferencePoint.LowerMiddle)
        l.addItem(item2)
        # NOTE: item3 has measurement units specified in Centimeters, see below!
        item3 = QgsLayoutItemPicture(l)
        item3.attemptMove(
            QgsLayoutPoint(0.8, 1.2, QgsUnitTypes.LayoutUnit.LayoutCentimeters)
        )
        item3.attemptResize(
            QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutUnit.LayoutCentimeters)
        )
        item3.setReferencePoint(QgsLayoutItem.ReferencePoint.UpperRight)
        l.addItem(item3)

        QgsLayoutAligner.alignItems(
            l, [item1, item2, item3], QgsLayoutAligner.Alignment.AlignLeft
        )
        self.assertEqual(
            item1.positionWithUnits(),
            QgsLayoutPoint(4, 8, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertEqual(
            item1.sizeWithUnits(),
            QgsLayoutSize(18, 12, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertEqual(
            item2.positionWithUnits(),
            QgsLayoutPoint(9, 19, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertEqual(
            item2.sizeWithUnits(),
            QgsLayoutSize(10, 9, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertEqual(
            item3.positionWithUnits(),
            QgsLayoutPoint(2.2, 1.2, QgsUnitTypes.LayoutUnit.LayoutCentimeters),
        )
        self.assertEqual(
            item3.sizeWithUnits(),
            QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutUnit.LayoutCentimeters),
        )

        QgsLayoutAligner.alignItems(
            l, [item1, item2, item3], QgsLayoutAligner.Alignment.AlignHCenter
        )
        self.assertEqual(
            item1.positionWithUnits(),
            QgsLayoutPoint(4, 8, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertEqual(
            item1.sizeWithUnits(),
            QgsLayoutSize(18, 12, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertEqual(
            item2.positionWithUnits(),
            QgsLayoutPoint(13, 19, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertEqual(
            item2.sizeWithUnits(),
            QgsLayoutSize(10, 9, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertEqual(
            item3.positionWithUnits(),
            QgsLayoutPoint(2.2, 1.2, QgsUnitTypes.LayoutUnit.LayoutCentimeters),
        )
        self.assertEqual(
            item3.sizeWithUnits(),
            QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutUnit.LayoutCentimeters),
        )

        QgsLayoutAligner.alignItems(
            l, [item1, item2, item3], QgsLayoutAligner.Alignment.AlignRight
        )
        self.assertEqual(
            item1.positionWithUnits(),
            QgsLayoutPoint(4, 8, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertEqual(
            item1.sizeWithUnits(),
            QgsLayoutSize(18, 12, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertEqual(
            item2.positionWithUnits(),
            QgsLayoutPoint(17, 19, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertEqual(
            item2.sizeWithUnits(),
            QgsLayoutSize(10, 9, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertEqual(
            item3.positionWithUnits(),
            QgsLayoutPoint(2.2, 1.2, QgsUnitTypes.LayoutUnit.LayoutCentimeters),
        )
        self.assertEqual(
            item3.sizeWithUnits(),
            QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutUnit.LayoutCentimeters),
        )

        QgsLayoutAligner.alignItems(
            l, [item1, item2, item3], QgsLayoutAligner.Alignment.AlignTop
        )
        self.assertEqual(
            item1.positionWithUnits(),
            QgsLayoutPoint(4, 8, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertEqual(
            item1.sizeWithUnits(),
            QgsLayoutSize(18, 12, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertEqual(
            item2.positionWithUnits(),
            QgsLayoutPoint(17, 17, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertEqual(
            item2.sizeWithUnits(),
            QgsLayoutSize(10, 9, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertEqual(
            item3.positionWithUnits(),
            QgsLayoutPoint(2.2, 0.8, QgsUnitTypes.LayoutUnit.LayoutCentimeters),
        )
        self.assertEqual(
            item3.sizeWithUnits(),
            QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutUnit.LayoutCentimeters),
        )

        QgsLayoutAligner.alignItems(
            l, [item1, item2, item3], QgsLayoutAligner.Alignment.AlignVCenter
        )
        self.assertEqual(
            item1.positionWithUnits(),
            QgsLayoutPoint(4, 10, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertEqual(
            item1.sizeWithUnits(),
            QgsLayoutSize(18, 12, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertEqual(
            item2.positionWithUnits(),
            QgsLayoutPoint(17, 20.5, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertEqual(
            item2.sizeWithUnits(),
            QgsLayoutSize(10, 9, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertEqual(
            item3.positionWithUnits(),
            QgsLayoutPoint(2.2, 0.8, QgsUnitTypes.LayoutUnit.LayoutCentimeters),
        )
        self.assertEqual(
            item3.sizeWithUnits(),
            QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutUnit.LayoutCentimeters),
        )

        QgsLayoutAligner.alignItems(
            l, [item1, item2, item3], QgsLayoutAligner.Alignment.AlignBottom
        )
        self.assertEqual(
            item1.positionWithUnits(),
            QgsLayoutPoint(4, 12, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertEqual(
            item1.sizeWithUnits(),
            QgsLayoutSize(18, 12, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertEqual(
            item2.positionWithUnits(),
            QgsLayoutPoint(17, 24, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertEqual(
            item2.sizeWithUnits(),
            QgsLayoutSize(10, 9, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertEqual(
            item3.positionWithUnits(),
            QgsLayoutPoint(2.2, 0.8, QgsUnitTypes.LayoutUnit.LayoutCentimeters),
        )
        self.assertEqual(
            item3.sizeWithUnits(),
            QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutUnit.LayoutCentimeters),
        )

    def testDistribute(self):
        p = QgsProject()
        l = QgsLayout(p)

        # add some items
        item1 = QgsLayoutItemPicture(l)
        item1.attemptMove(
            QgsLayoutPoint(4, 8, QgsUnitTypes.LayoutUnit.LayoutMillimeters)
        )
        item1.attemptResize(
            QgsLayoutSize(18, 12, QgsUnitTypes.LayoutUnit.LayoutMillimeters)
        )
        l.addItem(item1)
        item2 = QgsLayoutItemPicture(l)
        item2.attemptMove(
            QgsLayoutPoint(7, 10, QgsUnitTypes.LayoutUnit.LayoutMillimeters)
        )
        item2.attemptResize(
            QgsLayoutSize(10, 9, QgsUnitTypes.LayoutUnit.LayoutMillimeters)
        )
        item2.setReferencePoint(QgsLayoutItem.ReferencePoint.LowerMiddle)
        l.addItem(item2)
        # NOTE: item3 has measurement units specified in Centimeters, see below!
        item3 = QgsLayoutItemPicture(l)
        item3.attemptMove(
            QgsLayoutPoint(0.8, 1.2, QgsUnitTypes.LayoutUnit.LayoutCentimeters)
        )
        item3.attemptResize(
            QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutUnit.LayoutCentimeters)
        )
        item3.setReferencePoint(QgsLayoutItem.ReferencePoint.UpperRight)
        l.addItem(item3)

        QgsLayoutAligner.distributeItems(
            l, [item1, item2, item3], QgsLayoutAligner.Distribution.DistributeLeft
        )
        self.assertAlmostEqual(item1.positionWithUnits().x(), 4.0, 3)
        self.assertEqual(
            item1.positionWithUnits(),
            QgsLayoutPoint(4, 8, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertEqual(
            item1.sizeWithUnits(),
            QgsLayoutSize(18, 12, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertAlmostEqual(item2.positionWithUnits().x(), 11.0, 3)
        self.assertEqual(
            item2.positionWithUnits().units(), QgsUnitTypes.LayoutUnit.LayoutMillimeters
        )
        self.assertEqual(
            item2.sizeWithUnits(),
            QgsLayoutSize(10, 9, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertAlmostEqual(item3.positionWithUnits().x(), 2.6, 3)
        self.assertEqual(
            item3.positionWithUnits().units(), QgsUnitTypes.LayoutUnit.LayoutCentimeters
        )
        self.assertEqual(
            item3.sizeWithUnits(),
            QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutUnit.LayoutCentimeters),
        )

        QgsLayoutAligner.distributeItems(
            l, [item1, item2, item3], QgsLayoutAligner.Distribution.DistributeHCenter
        )
        self.assertAlmostEqual(item1.positionWithUnits().x(), 5.0, 3)
        self.assertEqual(
            item1.positionWithUnits().units(), QgsUnitTypes.LayoutUnit.LayoutMillimeters
        )
        self.assertEqual(
            item1.sizeWithUnits(),
            QgsLayoutSize(18, 12, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertAlmostEqual(item2.positionWithUnits().x(), 11.0, 3)
        self.assertEqual(
            item2.positionWithUnits().units(), QgsUnitTypes.LayoutUnit.LayoutMillimeters
        )
        self.assertEqual(
            item2.sizeWithUnits(),
            QgsLayoutSize(10, 9, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertAlmostEqual(item3.positionWithUnits().x(), 2.6, 3)
        self.assertEqual(
            item3.positionWithUnits().units(), QgsUnitTypes.LayoutUnit.LayoutCentimeters
        )
        self.assertEqual(
            item3.sizeWithUnits(),
            QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutUnit.LayoutCentimeters),
        )

        QgsLayoutAligner.distributeItems(
            l, [item1, item2, item3], QgsLayoutAligner.Distribution.DistributeRight
        )
        self.assertAlmostEqual(item1.positionWithUnits().x(), 3.0, 3)
        self.assertEqual(
            item1.positionWithUnits().units(), QgsUnitTypes.LayoutUnit.LayoutMillimeters
        )
        self.assertEqual(
            item1.sizeWithUnits(),
            QgsLayoutSize(18, 12, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertAlmostEqual(item2.positionWithUnits().x(), 11.0, 3)
        self.assertEqual(
            item2.positionWithUnits().units(), QgsUnitTypes.LayoutUnit.LayoutMillimeters
        )
        self.assertEqual(
            item2.sizeWithUnits(),
            QgsLayoutSize(10, 9, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertAlmostEqual(item3.positionWithUnits().x(), 2.6, 3)
        self.assertEqual(
            item3.positionWithUnits().units(), QgsUnitTypes.LayoutUnit.LayoutCentimeters
        )
        self.assertEqual(
            item3.sizeWithUnits(),
            QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutUnit.LayoutCentimeters),
        )

        QgsLayoutAligner.distributeItems(
            l, [item1, item2, item3], QgsLayoutAligner.Distribution.DistributeTop
        )
        self.assertAlmostEqual(item1.positionWithUnits().y(), 8.0, 3)
        self.assertEqual(
            item1.positionWithUnits().units(), QgsUnitTypes.LayoutUnit.LayoutMillimeters
        )
        self.assertEqual(
            item1.sizeWithUnits(),
            QgsLayoutSize(18, 12, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertAlmostEqual(item2.positionWithUnits().y(), 19.0, 3)
        self.assertEqual(
            item2.positionWithUnits().units(), QgsUnitTypes.LayoutUnit.LayoutMillimeters
        )
        self.assertEqual(
            item2.sizeWithUnits(),
            QgsLayoutSize(10, 9, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertAlmostEqual(item3.positionWithUnits().y(), 1.2, 3)
        self.assertEqual(
            item3.positionWithUnits().units(), QgsUnitTypes.LayoutUnit.LayoutCentimeters
        )
        self.assertEqual(
            item3.sizeWithUnits(),
            QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutUnit.LayoutCentimeters),
        )

        QgsLayoutAligner.distributeItems(
            l, [item1, item2, item3], QgsLayoutAligner.Distribution.DistributeVCenter
        )
        self.assertAlmostEqual(item1.positionWithUnits().y(), 8.0, 3)
        self.assertEqual(
            item1.positionWithUnits().units(), QgsUnitTypes.LayoutUnit.LayoutMillimeters
        )
        self.assertEqual(
            item1.sizeWithUnits(),
            QgsLayoutSize(18, 12, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertAlmostEqual(item2.positionWithUnits().y(), 21.5, 3)
        self.assertEqual(
            item2.positionWithUnits().units(), QgsUnitTypes.LayoutUnit.LayoutMillimeters
        )
        self.assertEqual(
            item2.sizeWithUnits(),
            QgsLayoutSize(10, 9, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertAlmostEqual(item3.positionWithUnits().y(), 1.2, 3)
        self.assertEqual(
            item3.positionWithUnits().units(), QgsUnitTypes.LayoutUnit.LayoutCentimeters
        )
        self.assertEqual(
            item3.sizeWithUnits(),
            QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutUnit.LayoutCentimeters),
        )

        QgsLayoutAligner.distributeItems(
            l, [item1, item2, item3], QgsLayoutAligner.Distribution.DistributeBottom
        )
        self.assertAlmostEqual(item1.positionWithUnits().y(), 8.0, 3)
        self.assertEqual(
            item1.positionWithUnits().units(), QgsUnitTypes.LayoutUnit.LayoutMillimeters
        )
        self.assertEqual(
            item1.sizeWithUnits(),
            QgsLayoutSize(18, 12, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertAlmostEqual(item2.positionWithUnits().y(), 24.0, 3)
        self.assertEqual(
            item2.positionWithUnits().units(), QgsUnitTypes.LayoutUnit.LayoutMillimeters
        )
        self.assertEqual(
            item2.sizeWithUnits(),
            QgsLayoutSize(10, 9, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertAlmostEqual(item3.positionWithUnits().y(), 1.2, 3)
        self.assertEqual(
            item3.positionWithUnits().units(), QgsUnitTypes.LayoutUnit.LayoutCentimeters
        )
        self.assertEqual(
            item3.sizeWithUnits(),
            QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutUnit.LayoutCentimeters),
        )

        QgsLayoutAligner.distributeItems(
            l, [item1, item2, item3], QgsLayoutAligner.Distribution.DistributeHSpace
        )
        self.assertAlmostEqual(item1.positionWithUnits().x(), 3.0, 3)
        self.assertEqual(
            item1.positionWithUnits().units(), QgsUnitTypes.LayoutUnit.LayoutMillimeters
        )
        self.assertEqual(
            item1.sizeWithUnits(),
            QgsLayoutSize(18, 12, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertAlmostEqual(item2.positionWithUnits().x(), 14.5, 3)
        self.assertEqual(
            item2.positionWithUnits().units(), QgsUnitTypes.LayoutUnit.LayoutMillimeters
        )
        self.assertEqual(
            item2.sizeWithUnits(),
            QgsLayoutSize(10, 9, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertAlmostEqual(item3.positionWithUnits().x(), 2.6, 3)
        self.assertEqual(
            item3.positionWithUnits().units(), QgsUnitTypes.LayoutUnit.LayoutCentimeters
        )
        self.assertEqual(
            item3.sizeWithUnits(),
            QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutUnit.LayoutCentimeters),
        )

        QgsLayoutAligner.distributeItems(
            l, [item1, item2, item3], QgsLayoutAligner.Distribution.DistributeVSpace
        )
        self.assertAlmostEqual(item1.positionWithUnits().y(), 8.0, 3)
        self.assertEqual(
            item1.positionWithUnits().units(), QgsUnitTypes.LayoutUnit.LayoutMillimeters
        )
        self.assertEqual(
            item1.sizeWithUnits(),
            QgsLayoutSize(18, 12, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertAlmostEqual(item2.positionWithUnits().y(), 28.0, 3)
        self.assertEqual(
            item2.positionWithUnits().units(), QgsUnitTypes.LayoutUnit.LayoutMillimeters
        )
        self.assertEqual(
            item2.sizeWithUnits(),
            QgsLayoutSize(10, 9, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertAlmostEqual(item3.positionWithUnits().y(), 1.15, 3)
        self.assertEqual(
            item3.positionWithUnits().units(), QgsUnitTypes.LayoutUnit.LayoutCentimeters
        )
        self.assertEqual(
            item3.sizeWithUnits(),
            QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutUnit.LayoutCentimeters),
        )

    def testResize(self):
        p = QgsProject()
        l = QgsLayout(p)

        # add some items
        item1 = QgsLayoutItemPicture(l)
        item1.attemptMove(
            QgsLayoutPoint(4, 8, QgsUnitTypes.LayoutUnit.LayoutMillimeters)
        )
        item1.attemptResize(
            QgsLayoutSize(18, 12, QgsUnitTypes.LayoutUnit.LayoutMillimeters)
        )
        l.addItem(item1)
        item2 = QgsLayoutItemPicture(l)
        item2.attemptMove(
            QgsLayoutPoint(7, 10, QgsUnitTypes.LayoutUnit.LayoutMillimeters)
        )
        item2.attemptResize(
            QgsLayoutSize(10, 9, QgsUnitTypes.LayoutUnit.LayoutMillimeters)
        )
        item2.setReferencePoint(QgsLayoutItem.ReferencePoint.LowerMiddle)
        l.addItem(item2)
        # NOTE: item3 has measurement units specified in Centimeters, see below!
        item3 = QgsLayoutItemPicture(l)
        item3.attemptMove(
            QgsLayoutPoint(0.8, 1.2, QgsUnitTypes.LayoutUnit.LayoutCentimeters)
        )
        item3.attemptResize(
            QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutUnit.LayoutCentimeters)
        )
        item3.setReferencePoint(QgsLayoutItem.ReferencePoint.UpperRight)
        l.addItem(item3)

        QgsLayoutAligner.resizeItems(
            l, [item1, item2, item3], QgsLayoutAligner.Resize.ResizeNarrowest
        )
        self.assertEqual(
            item1.sizeWithUnits(),
            QgsLayoutSize(10, 12, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertEqual(
            item2.sizeWithUnits(),
            QgsLayoutSize(10, 9, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertEqual(
            item3.sizeWithUnits(),
            QgsLayoutSize(1.0, 1.6, QgsUnitTypes.LayoutUnit.LayoutCentimeters),
        )
        l.undoStack().stack().undo()

        QgsLayoutAligner.resizeItems(
            l, [item1, item2, item3], QgsLayoutAligner.Resize.ResizeWidest
        )
        self.assertEqual(
            item1.sizeWithUnits(),
            QgsLayoutSize(18, 12, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertEqual(
            item2.sizeWithUnits(),
            QgsLayoutSize(18, 9, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertEqual(
            item3.sizeWithUnits(),
            QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutUnit.LayoutCentimeters),
        )
        l.undoStack().stack().undo()

        QgsLayoutAligner.resizeItems(
            l, [item1, item2, item3], QgsLayoutAligner.Resize.ResizeShortest
        )
        self.assertEqual(
            item1.sizeWithUnits(),
            QgsLayoutSize(18, 9, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertEqual(
            item2.sizeWithUnits(),
            QgsLayoutSize(10, 9, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertEqual(
            item3.sizeWithUnits(),
            QgsLayoutSize(1.8, 0.9, QgsUnitTypes.LayoutUnit.LayoutCentimeters),
        )
        l.undoStack().stack().undo()

        QgsLayoutAligner.resizeItems(
            l, [item1, item2, item3], QgsLayoutAligner.Resize.ResizeTallest
        )
        self.assertEqual(
            item1.sizeWithUnits(),
            QgsLayoutSize(18, 16, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertEqual(
            item2.sizeWithUnits(),
            QgsLayoutSize(10, 16, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertEqual(
            item3.sizeWithUnits(),
            QgsLayoutSize(1.8, 1.6, QgsUnitTypes.LayoutUnit.LayoutCentimeters),
        )
        l.undoStack().stack().undo()

        item2.attemptResize(
            QgsLayoutSize(10, 19, QgsUnitTypes.LayoutUnit.LayoutMillimeters)
        )
        QgsLayoutAligner.resizeItems(
            l, [item1, item2, item3], QgsLayoutAligner.Resize.ResizeToSquare
        )
        self.assertEqual(
            item1.sizeWithUnits(),
            QgsLayoutSize(18, 18, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertEqual(
            item2.sizeWithUnits(),
            QgsLayoutSize(19, 19, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertEqual(
            item3.sizeWithUnits(),
            QgsLayoutSize(1.8, 1.8, QgsUnitTypes.LayoutUnit.LayoutCentimeters),
        )

        l.undoStack().stack().undo()
        QgsLayoutAligner.resizeItems(l, [item1], QgsLayoutAligner.Resize.ResizeToSquare)
        self.assertEqual(
            item1.sizeWithUnits(),
            QgsLayoutSize(18, 18, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )


if __name__ == "__main__":
    unittest.main()
