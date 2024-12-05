"""QGIS Unit tests for QgsLayoutPageCollection

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "18/07/2017"
__copyright__ = "Copyright 2017, The QGIS Project"

from qgis.PyQt import sip
from qgis.PyQt.QtCore import QCoreApplication, QEvent, QPointF, QRectF, Qt
from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    QgsFillSymbol,
    QgsLayout,
    QgsLayoutGuide,
    QgsLayoutItemPage,
    QgsLayoutItemShape,
    QgsLayoutMeasurement,
    QgsLayoutObject,
    QgsLayoutPoint,
    QgsLayoutSize,
    QgsMargins,
    QgsProject,
    QgsProperty,
    QgsReadWriteContext,
    QgsSimpleFillSymbolLayer,
    QgsUnitTypes,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsLayoutPageCollection(QgisTestCase):

    def testLayout(self):
        # test that layouts have a collection
        p = QgsProject()
        l = QgsLayout(p)
        self.assertTrue(l.pageCollection())
        self.assertEqual(l.pageCollection().layout(), l)

    def testSymbol(self):
        """
        Test setting a page symbol for the collection
        """
        p = QgsProject()
        l = QgsLayout(p)
        collection = l.pageCollection()
        self.assertTrue(collection.pageStyleSymbol())

        fill = QgsSimpleFillSymbolLayer()
        fill_symbol = QgsFillSymbol()
        fill_symbol.changeSymbolLayer(0, fill)
        fill.setColor(Qt.GlobalColor.green)
        fill.setStrokeColor(Qt.GlobalColor.red)
        fill.setStrokeWidth(6)
        collection.setPageStyleSymbol(fill_symbol)
        self.assertEqual(
            collection.pageStyleSymbol().symbolLayer(0).color().name(), "#00ff00"
        )
        self.assertEqual(
            collection.pageStyleSymbol().symbolLayer(0).strokeColor().name(), "#ff0000"
        )

    def testPages(self):
        """
        Test adding/retrieving/deleting pages from the collection
        """
        p = QgsProject()
        l = QgsLayout(p)
        collection = l.pageCollection()

        self.assertEqual(collection.pageCount(), 0)
        self.assertFalse(collection.pages())
        self.assertFalse(collection.page(-1))
        self.assertFalse(collection.page(0))
        self.assertFalse(collection.page(1))

        # add a page
        page = QgsLayoutItemPage(l)
        page.setPageSize("A4")
        self.assertEqual(collection.pageNumber(page), -1)

        collection.addPage(page)

        self.assertIn(page, l.items())

        self.assertEqual(collection.pageCount(), 1)
        self.assertEqual(collection.pages(), [page])
        self.assertFalse(collection.page(-1))
        self.assertEqual(collection.page(0), page)
        self.assertFalse(collection.page(1))
        self.assertEqual(collection.pageNumber(page), 0)

        # add a second page
        page2 = QgsLayoutItemPage(l)
        page2.setPageSize("A5")
        collection.addPage(page2)

        self.assertEqual(collection.pageCount(), 2)
        self.assertEqual(collection.pages(), [page, page2])
        self.assertFalse(collection.page(-1))
        self.assertEqual(collection.page(0), page)
        self.assertEqual(collection.page(1), page2)
        self.assertEqual(collection.pageNumber(page2), 1)

        # insert a page
        page3 = QgsLayoutItemPage(l)
        page3.setPageSize("A3")
        collection.insertPage(page3, 1)
        self.assertIn(page3, l.items())

        self.assertEqual(collection.pageCount(), 3)
        self.assertEqual(collection.pages(), [page, page3, page2])
        self.assertEqual(collection.page(0), page)
        self.assertEqual(collection.page(1), page3)
        self.assertEqual(collection.page(2), page2)
        self.assertEqual(collection.pageNumber(page3), 1)

        # delete page
        collection.deletePage(-1)
        self.assertEqual(collection.pageCount(), 3)
        self.assertEqual(collection.pages(), [page, page3, page2])
        collection.deletePage(100)
        self.assertEqual(collection.pageCount(), 3)
        self.assertEqual(collection.pages(), [page, page3, page2])
        collection.deletePage(1)
        self.assertEqual(collection.pageCount(), 2)
        self.assertEqual(collection.pages(), [page, page2])

        # make sure page was deleted
        QCoreApplication.sendPostedEvents(None, QEvent.Type.DeferredDelete)
        self.assertTrue(sip.isdeleted(page3))

        del l
        QCoreApplication.sendPostedEvents(None, QEvent.Type.DeferredDelete)
        self.assertTrue(sip.isdeleted(page))
        self.assertTrue(sip.isdeleted(page2))

    def testDeletePages(self):
        """
        Test deleting pages from the collection
        """
        p = QgsProject()
        l = QgsLayout(p)
        collection = l.pageCollection()

        # add a page
        page = QgsLayoutItemPage(l)
        page.setPageSize("A4")
        collection.addPage(page)
        # add a second page
        page2 = QgsLayoutItemPage(l)
        page2.setPageSize("A5")
        collection.addPage(page2)

        page_about_to_be_removed_spy = QSignalSpy(collection.pageAboutToBeRemoved)

        # delete page
        collection.deletePage(None)
        self.assertEqual(collection.pageCount(), 2)
        self.assertEqual(len(page_about_to_be_removed_spy), 0)

        page3 = QgsLayoutItemPage(l)
        # try deleting a page not in collection
        collection.deletePage(page3)
        QCoreApplication.sendPostedEvents(None, QEvent.Type.DeferredDelete)
        self.assertFalse(sip.isdeleted(page3))
        self.assertEqual(collection.pageCount(), 2)
        self.assertEqual(len(page_about_to_be_removed_spy), 0)

        self.assertEqual(
            l.layoutBounds(ignorePages=False), QRectF(0.0, 0.0, 210.0, 517.0)
        )
        collection.deletePage(page)
        self.assertEqual(collection.pageCount(), 1)
        self.assertEqual(
            l.layoutBounds(ignorePages=False), QRectF(0.0, 0.0, 148.0, 210.0)
        )
        self.assertNotIn(page, collection.pages())
        QCoreApplication.sendPostedEvents(None, QEvent.Type.DeferredDelete)
        self.assertTrue(sip.isdeleted(page))
        self.assertEqual(len(page_about_to_be_removed_spy), 1)
        self.assertEqual(page_about_to_be_removed_spy[-1][0], 0)

        collection.deletePage(page2)
        self.assertEqual(collection.pageCount(), 0)
        self.assertFalse(collection.pages())
        self.assertEqual(l.layoutBounds(ignorePages=False), QRectF())
        QCoreApplication.sendPostedEvents(None, QEvent.Type.DeferredDelete)
        self.assertTrue(sip.isdeleted(page2))
        self.assertEqual(len(page_about_to_be_removed_spy), 2)
        self.assertEqual(page_about_to_be_removed_spy[-1][0], 0)

    def testClear(self):
        """
        Test clearing the collection
        """
        p = QgsProject()
        l = QgsLayout(p)
        collection = l.pageCollection()

        collection.clear()

        # add a page
        page = QgsLayoutItemPage(l)
        page.setPageSize("A4")
        collection.addPage(page)
        # add a second page
        page2 = QgsLayoutItemPage(l)
        page2.setPageSize("A5")
        collection.addPage(page2)

        page_about_to_be_removed_spy = QSignalSpy(collection.pageAboutToBeRemoved)

        # clear
        collection.clear()
        self.assertEqual(collection.pageCount(), 0)
        self.assertEqual(len(page_about_to_be_removed_spy), 2)

        QCoreApplication.sendPostedEvents(None, QEvent.Type.DeferredDelete)
        self.assertTrue(sip.isdeleted(page))
        self.assertTrue(sip.isdeleted(page2))

    def testExtendByNewPage(self):
        """
        Test extend by adding new page
        """
        p = QgsProject()
        l = QgsLayout(p)
        collection = l.pageCollection()

        # no existing page to extend
        self.assertIsNone(collection.extendByNewPage())
        self.assertEqual(collection.pageCount(), 0)

        # add a page
        page = QgsLayoutItemPage(l)
        page.setPageSize(QgsLayoutSize(10, 10))
        collection.addPage(page)
        self.assertEqual(collection.pageCount(), 1)

        new_page = collection.extendByNewPage()
        self.assertIsNotNone(new_page)
        self.assertEqual(collection.pageCount(), 2)
        self.assertEqual(new_page.sizeWithUnits(), page.sizeWithUnits())

        new_page.setPageSize(QgsLayoutSize(20, 20))
        new_page2 = collection.extendByNewPage()
        self.assertIsNotNone(new_page2)
        self.assertEqual(collection.pageCount(), 3)
        self.assertEqual(new_page2.sizeWithUnits(), new_page.sizeWithUnits())

    def testMaxPageWidthAndSize(self):
        """
        Test calculating maximum page width and size
        """
        p = QgsProject()
        l = QgsLayout(p)
        collection = l.pageCollection()

        # add a page
        page = QgsLayoutItemPage(l)
        page.setPageSize("A4")
        collection.addPage(page)
        self.assertEqual(collection.maximumPageWidth(), 210.0)
        self.assertEqual(collection.maximumPageSize().width(), 210.0)
        self.assertEqual(collection.maximumPageSize().height(), 297.0)

        # add a second page
        page2 = QgsLayoutItemPage(l)
        page2.setPageSize("A3")
        collection.addPage(page2)
        self.assertEqual(collection.maximumPageWidth(), 297.0)
        self.assertEqual(collection.maximumPageSize().width(), 297.0)
        self.assertEqual(collection.maximumPageSize().height(), 420.0)

        # add a page with other units
        page3 = QgsLayoutItemPage(l)
        page3.setPageSize(QgsLayoutSize(100, 100, QgsUnitTypes.LayoutUnit.LayoutMeters))
        collection.addPage(page3)
        self.assertEqual(collection.maximumPageWidth(), 100000.0)
        self.assertEqual(collection.maximumPageSize().width(), 100000.0)
        self.assertEqual(collection.maximumPageSize().height(), 100000.0)

    def testUniformPageSizes(self):
        """
        Test detection of uniform page sizes
        """
        p = QgsProject()
        l = QgsLayout(p)
        collection = l.pageCollection()

        self.assertTrue(collection.hasUniformPageSizes())

        # add a page
        page = QgsLayoutItemPage(l)
        page.setPageSize("A4")
        collection.addPage(page)
        self.assertTrue(collection.hasUniformPageSizes())

        # add a second page
        page2 = QgsLayoutItemPage(l)
        page2.setPageSize(
            QgsLayoutSize(21.0, 29.7, QgsUnitTypes.LayoutUnit.LayoutCentimeters)
        )
        collection.addPage(page2)
        self.assertTrue(collection.hasUniformPageSizes())

        # add a page with other units
        page3 = QgsLayoutItemPage(l)
        page3.setPageSize("A5")
        collection.addPage(page3)
        self.assertFalse(collection.hasUniformPageSizes())

    def testReflow(self):
        """
        Test reflowing pages
        """
        p = QgsProject()
        l = QgsLayout(p)
        collection = l.pageCollection()

        # add a page
        page = QgsLayoutItemPage(l)
        page.setPageSize("A4")
        collection.addPage(page)

        # should be positioned at origin
        self.assertEqual(page.pos().x(), 0)
        self.assertEqual(page.pos().y(), 0)

        # second page
        page2 = QgsLayoutItemPage(l)
        page2.setPageSize("A5")
        collection.addPage(page2)

        self.assertEqual(page.pos().x(), 0)
        self.assertEqual(page.pos().y(), 0)
        self.assertEqual(page2.pos().x(), 0)
        self.assertEqual(page2.pos().y(), 307)

        # third page, slotted in middle
        page3 = QgsLayoutItemPage(l)
        page3.setPageSize("A3")
        collection.insertPage(page3, 1)

        self.assertEqual(page.pos().x(), 0)
        self.assertEqual(page.pos().y(), 0)
        self.assertEqual(page2.pos().x(), 0)
        self.assertEqual(page2.pos().y(), 737)
        self.assertEqual(page3.pos().x(), 0)
        self.assertEqual(page3.pos().y(), 307)

        page.setPageSize(QgsLayoutSize(100, 120))
        # no update until reflow is called
        self.assertEqual(page.pos().x(), 0)
        self.assertEqual(page.pos().y(), 0)
        self.assertEqual(page2.pos().x(), 0)
        self.assertEqual(page2.pos().y(), 737)
        self.assertEqual(page3.pos().x(), 0)
        self.assertEqual(page3.pos().y(), 307)
        collection.reflow()
        self.assertEqual(page.pos().x(), 0)
        self.assertEqual(page.pos().y(), 0)
        self.assertEqual(page2.pos().x(), 0)
        self.assertEqual(page2.pos().y(), 560)
        self.assertEqual(page3.pos().x(), 0)
        self.assertEqual(page3.pos().y(), 130)

    def testInsertPageWithItems(self):
        p = QgsProject()
        l = QgsLayout(p)
        collection = l.pageCollection()

        # add a page
        page = QgsLayoutItemPage(l)
        page.setPageSize("A4")
        collection.addPage(page)
        page2 = QgsLayoutItemPage(l)
        page2.setPageSize("A5")
        collection.addPage(page2)

        # item on pages
        shape1 = QgsLayoutItemShape(l)
        shape1.attemptResize(QgsLayoutSize(90, 50))
        shape1.attemptMove(QgsLayoutPoint(90, 50), page=0)
        l.addLayoutItem(shape1)

        shape2 = QgsLayoutItemShape(l)
        shape2.attemptResize(QgsLayoutSize(110, 50))
        shape2.attemptMove(QgsLayoutPoint(100, 150), page=1)
        l.addLayoutItem(shape2)

        self.assertEqual(shape1.page(), 0)
        self.assertEqual(shape2.page(), 1)

        # third page, slotted in middle
        page3 = QgsLayoutItemPage(l)
        page3.setPageSize("A3")
        collection.insertPage(page3, 0)

        # check item position
        self.assertEqual(shape1.page(), 1)
        self.assertEqual(shape1.pagePositionWithUnits(), QgsLayoutPoint(90, 50))
        self.assertEqual(shape2.page(), 2)
        self.assertEqual(shape2.pagePositionWithUnits(), QgsLayoutPoint(100, 150))

    def testDeletePageWithItems(self):
        p = QgsProject()
        l = QgsLayout(p)
        collection = l.pageCollection()

        # add a page
        page = QgsLayoutItemPage(l)
        page.setPageSize("A4")
        collection.addPage(page)
        page2 = QgsLayoutItemPage(l)
        page2.setPageSize("A4")
        collection.addPage(page2)
        page3 = QgsLayoutItemPage(l)
        page3.setPageSize("A4")
        collection.addPage(page3)

        # item on pages
        shape1 = QgsLayoutItemShape(l)
        shape1.attemptResize(QgsLayoutSize(90, 50))
        shape1.attemptMove(QgsLayoutPoint(90, 50), page=0)
        l.addLayoutItem(shape1)

        shape2 = QgsLayoutItemShape(l)
        shape2.attemptResize(QgsLayoutSize(110, 50))
        shape2.attemptMove(QgsLayoutPoint(100, 150), page=2)
        l.addLayoutItem(shape2)

        self.assertEqual(shape1.page(), 0)
        self.assertEqual(shape2.page(), 2)

        collection.deletePage(1)

        # check item position
        self.assertEqual(shape1.page(), 0)
        self.assertEqual(shape1.pagePositionWithUnits(), QgsLayoutPoint(90, 50))
        self.assertEqual(shape2.page(), 1)
        self.assertEqual(shape2.pagePositionWithUnits(), QgsLayoutPoint(100, 150))

    def testDeletePageWithItems2(self):
        p = QgsProject()
        l = QgsLayout(p)
        collection = l.pageCollection()

        # add a page
        page = QgsLayoutItemPage(l)
        page.setPageSize("A4")
        collection.addPage(page)
        page2 = QgsLayoutItemPage(l)
        page2.setPageSize("A4")
        collection.addPage(page2)
        page3 = QgsLayoutItemPage(l)
        page3.setPageSize("A4")
        collection.addPage(page3)

        # item on pages
        shape1 = QgsLayoutItemShape(l)
        shape1.attemptResize(QgsLayoutSize(90, 50))
        shape1.attemptMove(QgsLayoutPoint(90, 50), page=0)
        l.addLayoutItem(shape1)

        shape2 = QgsLayoutItemShape(l)
        shape2.attemptResize(QgsLayoutSize(110, 50))
        shape2.attemptMove(QgsLayoutPoint(100, 150), page=2)
        l.addLayoutItem(shape2)

        self.assertEqual(shape1.page(), 0)
        self.assertEqual(shape2.page(), 2)

        collection.deletePage(page2)

        # check item position
        self.assertEqual(shape1.page(), 0)
        self.assertEqual(shape1.pagePositionWithUnits(), QgsLayoutPoint(90, 50))
        self.assertEqual(shape2.page(), 1)
        self.assertEqual(shape2.pagePositionWithUnits(), QgsLayoutPoint(100, 150))

    def testDataDefinedSize(self):
        p = QgsProject()
        l = QgsLayout(p)
        collection = l.pageCollection()

        # add some pages
        page = QgsLayoutItemPage(l)
        page.setPageSize("A4")
        collection.addPage(page)
        page2 = QgsLayoutItemPage(l)
        page2.setPageSize("A5")
        collection.addPage(page2)
        page3 = QgsLayoutItemPage(l)
        page3.setPageSize("A5")
        collection.addPage(page3)

        self.assertEqual(page.pos().x(), 0)
        self.assertEqual(page.pos().y(), 0)
        self.assertEqual(page2.pos().x(), 0)
        self.assertEqual(page2.pos().y(), 307)
        self.assertEqual(page3.pos().x(), 0)
        self.assertEqual(page3.pos().y(), 527)

        page.dataDefinedProperties().setProperty(
            QgsLayoutObject.DataDefinedProperty.ItemHeight,
            QgsProperty.fromExpression("50*3"),
        )
        page.refresh()
        collection.reflow()
        self.assertEqual(page.pos().x(), 0)
        self.assertEqual(page.pos().y(), 0)
        self.assertEqual(page2.pos().x(), 0)
        self.assertEqual(page2.pos().y(), 160)
        self.assertEqual(page3.pos().x(), 0)
        self.assertEqual(page3.pos().y(), 380)

        page2.dataDefinedProperties().setProperty(
            QgsLayoutObject.DataDefinedProperty.ItemHeight,
            QgsProperty.fromExpression("50-20"),
        )
        page2.refresh()
        collection.reflow()
        self.assertEqual(page.pos().x(), 0)
        self.assertEqual(page.pos().y(), 0)
        self.assertEqual(page2.pos().x(), 0)
        self.assertEqual(page2.pos().y(), 160)
        self.assertEqual(page3.pos().x(), 0)
        self.assertEqual(page3.pos().y(), 200)

    def testPositionOnPage(self):
        """
        Test pageNumberForPoint and positionOnPage
        """
        p = QgsProject()
        l = QgsLayout(p)
        collection = l.pageCollection()

        # add a page
        page = QgsLayoutItemPage(l)
        page.setPageSize("A4")
        collection.addPage(page)

        self.assertEqual(collection.pageNumberForPoint(QPointF(-100, -100)), 0)
        self.assertEqual(collection.pageNumberForPoint(QPointF(-100, -1)), 0)
        self.assertEqual(collection.pageNumberForPoint(QPointF(-100, 1)), 0)
        self.assertEqual(collection.pageNumberForPoint(QPointF(-100, 270)), 0)
        self.assertEqual(collection.pageNumberForPoint(QPointF(-100, 1270)), 0)

        self.assertEqual(
            collection.positionOnPage(QPointF(-100, -100)), QPointF(-100, -100)
        )
        self.assertEqual(
            collection.positionOnPage(QPointF(-100, -1)), QPointF(-100, -1)
        )
        self.assertEqual(collection.positionOnPage(QPointF(-100, 1)), QPointF(-100, 1))
        self.assertEqual(
            collection.positionOnPage(QPointF(-100, 270)), QPointF(-100, 270)
        )
        self.assertEqual(
            collection.positionOnPage(QPointF(-100, 1270)), QPointF(-100, 973)
        )

        page2 = QgsLayoutItemPage(l)
        page2.setPageSize("A5")
        collection.addPage(page2)

        self.assertEqual(collection.pageNumberForPoint(QPointF(-100, -100)), 0)
        self.assertEqual(collection.pageNumberForPoint(QPointF(-100, -1)), 0)
        self.assertEqual(collection.pageNumberForPoint(QPointF(-100, 1)), 0)
        self.assertEqual(collection.pageNumberForPoint(QPointF(-100, 270)), 0)
        self.assertEqual(collection.pageNumberForPoint(QPointF(-100, 370)), 1)
        self.assertEqual(collection.pageNumberForPoint(QPointF(-100, 1270)), 1)

        self.assertEqual(
            collection.positionOnPage(QPointF(-100, -100)), QPointF(-100, -100)
        )
        self.assertEqual(
            collection.positionOnPage(QPointF(-100, -1)), QPointF(-100, -1)
        )
        self.assertEqual(collection.positionOnPage(QPointF(-100, 1)), QPointF(-100, 1))
        self.assertEqual(
            collection.positionOnPage(QPointF(-100, 270)), QPointF(-100, 270)
        )
        self.assertEqual(
            collection.positionOnPage(QPointF(-100, 370)), QPointF(-100, 63)
        )
        self.assertEqual(
            collection.positionOnPage(QPointF(-100, 1270)), QPointF(-100, 753)
        )

    def testPredictionPageNumberForPoint(self):
        """
        Test predictPageNumberForPoint
        """
        p = QgsProject()
        l = QgsLayout(p)
        collection = l.pageCollection()

        # no crash if no pages
        self.assertEqual(collection.predictPageNumberForPoint(QPointF(1, 1)), 0)

        # add a page
        page = QgsLayoutItemPage(l)
        page.setPageSize(QgsLayoutSize(100, 100))
        collection.addPage(page)

        self.assertEqual(collection.predictPageNumberForPoint(QPointF(-100, -100)), 0)
        self.assertEqual(collection.predictPageNumberForPoint(QPointF(-100, -1)), 0)
        self.assertEqual(collection.predictPageNumberForPoint(QPointF(-100, 20)), 0)
        self.assertEqual(collection.predictPageNumberForPoint(QPointF(-100, 120)), 1)
        self.assertEqual(collection.predictPageNumberForPoint(QPointF(-100, 230)), 2)
        self.assertEqual(collection.predictPageNumberForPoint(QPointF(-100, 350)), 3)

        page2 = QgsLayoutItemPage(l)
        page2.setPageSize(QgsLayoutSize(100, 50))
        collection.addPage(page2)

        self.assertEqual(collection.predictPageNumberForPoint(QPointF(-100, -100)), 0)
        self.assertEqual(collection.predictPageNumberForPoint(QPointF(-100, -1)), 0)
        self.assertEqual(collection.predictPageNumberForPoint(QPointF(-100, 20)), 0)
        self.assertEqual(collection.predictPageNumberForPoint(QPointF(-100, 120)), 1)
        self.assertEqual(collection.predictPageNumberForPoint(QPointF(-100, 230)), 2)
        self.assertEqual(collection.predictPageNumberForPoint(QPointF(-100, 280)), 3)
        self.assertEqual(collection.predictPageNumberForPoint(QPointF(-100, 340)), 4)
        self.assertEqual(collection.predictPageNumberForPoint(QPointF(-100, 370)), 5)

        page3 = QgsLayoutItemPage(l)
        page3.setPageSize(QgsLayoutSize(100, 200))
        collection.addPage(page3)

        self.assertEqual(collection.predictPageNumberForPoint(QPointF(-100, -100)), 0)
        self.assertEqual(collection.predictPageNumberForPoint(QPointF(-100, -1)), 0)
        self.assertEqual(collection.predictPageNumberForPoint(QPointF(-100, 20)), 0)
        self.assertEqual(collection.predictPageNumberForPoint(QPointF(-100, 120)), 1)
        self.assertEqual(collection.predictPageNumberForPoint(QPointF(-100, 230)), 2)
        self.assertEqual(collection.predictPageNumberForPoint(QPointF(-100, 280)), 2)
        self.assertEqual(collection.predictPageNumberForPoint(QPointF(-100, 340)), 2)
        self.assertEqual(collection.predictPageNumberForPoint(QPointF(-100, 370)), 2)
        self.assertEqual(collection.predictPageNumberForPoint(QPointF(-100, 470)), 3)

    def testPageAtPoint(self):
        """
        Test pageAtPoint
        """
        p = QgsProject()
        l = QgsLayout(p)
        collection = l.pageCollection()

        self.assertFalse(collection.pageAtPoint(QPointF(0, 0)))
        self.assertFalse(collection.pageAtPoint(QPointF(10, 10)))

        # add a page
        page = QgsLayoutItemPage(l)
        page.setPageSize("A4")
        collection.addPage(page)

        self.assertFalse(collection.pageAtPoint(QPointF(10, -1)))
        self.assertEqual(collection.pageAtPoint(QPointF(1, 1)), page)
        self.assertEqual(collection.pageAtPoint(QPointF(10, 10)), page)
        self.assertFalse(collection.pageAtPoint(QPointF(-10, 10)))
        self.assertFalse(collection.pageAtPoint(QPointF(1000, 10)))
        self.assertFalse(collection.pageAtPoint(QPointF(10, -10)))
        self.assertFalse(collection.pageAtPoint(QPointF(10, 1000)))

        page2 = QgsLayoutItemPage(l)
        page2.setPageSize("A5")
        collection.addPage(page2)

        self.assertEqual(collection.pageAtPoint(QPointF(1, 1)), page)
        self.assertEqual(collection.pageAtPoint(QPointF(10, 10)), page)
        self.assertFalse(collection.pageAtPoint(QPointF(-10, 10)))
        self.assertFalse(collection.pageAtPoint(QPointF(1000, 10)))
        self.assertFalse(collection.pageAtPoint(QPointF(10, -10)))
        self.assertEqual(collection.pageAtPoint(QPointF(10, 330)), page2)
        self.assertEqual(collection.pageAtPoint(QPointF(10, 500)), page2)
        self.assertFalse(collection.pageAtPoint(QPointF(10, 600)))

    def testPagePositionToLayout(self):
        """
        Test pagePositionToLayoutPosition
        """
        p = QgsProject()
        l = QgsLayout(p)
        collection = l.pageCollection()

        # invalid pages
        self.assertEqual(
            collection.pagePositionToLayoutPosition(-1, QgsLayoutPoint(1, 1)),
            QPointF(1, 1),
        )
        self.assertEqual(
            collection.pagePositionToLayoutPosition(0, QgsLayoutPoint(1, 1)),
            QPointF(1, 1),
        )
        self.assertEqual(
            collection.pagePositionToLayoutPosition(100, QgsLayoutPoint(1, 1)),
            QPointF(1, 1),
        )

        # add a page
        page = QgsLayoutItemPage(l)
        page.setPageSize("A4")
        collection.addPage(page)

        # invalid pages
        self.assertEqual(
            collection.pagePositionToLayoutPosition(-1, QgsLayoutPoint(1, 1)),
            QPointF(1, 1),
        )
        self.assertEqual(
            collection.pagePositionToLayoutPosition(1, QgsLayoutPoint(1, 1)),
            QPointF(1, 1),
        )
        # valid page
        self.assertEqual(
            collection.pagePositionToLayoutPosition(0, QgsLayoutPoint(1, 1)),
            QPointF(1, 1),
        )
        self.assertEqual(
            collection.pagePositionToLayoutPosition(0, QgsLayoutPoint(5, 6)),
            QPointF(5, 6),
        )
        self.assertEqual(
            collection.pagePositionToLayoutPosition(
                0, QgsLayoutPoint(5, 6, QgsUnitTypes.LayoutUnit.LayoutCentimeters)
            ),
            QPointF(50, 60),
        )

        page2 = QgsLayoutItemPage(l)
        page2.setPageSize("A5")
        collection.addPage(page2)

        # invalid pages
        self.assertEqual(
            collection.pagePositionToLayoutPosition(-1, QgsLayoutPoint(1, 1)),
            QPointF(1, 1),
        )
        self.assertEqual(
            collection.pagePositionToLayoutPosition(3, QgsLayoutPoint(1, 1)),
            QPointF(1, 1),
        )
        # valid pages
        self.assertEqual(
            collection.pagePositionToLayoutPosition(0, QgsLayoutPoint(1, 1)),
            QPointF(1, 1),
        )
        self.assertEqual(
            collection.pagePositionToLayoutPosition(0, QgsLayoutPoint(5, 6)),
            QPointF(5, 6),
        )
        self.assertEqual(
            collection.pagePositionToLayoutPosition(
                0, QgsLayoutPoint(5, 6, QgsUnitTypes.LayoutUnit.LayoutCentimeters)
            ),
            QPointF(50, 60),
        )
        self.assertEqual(
            collection.pagePositionToLayoutPosition(1, QgsLayoutPoint(1, 1)),
            QPointF(1, 308.0),
        )
        self.assertEqual(
            collection.pagePositionToLayoutPosition(1, QgsLayoutPoint(5, 6)),
            QPointF(5, 313.0),
        )
        self.assertEqual(
            collection.pagePositionToLayoutPosition(
                1, QgsLayoutPoint(0.5, 0.6, QgsUnitTypes.LayoutUnit.LayoutCentimeters)
            ),
            QPointF(5, 313.0),
        )

    def testPagePositionToAbsolute(self):
        """
        Test pagePositionToAbsolute
        """
        p = QgsProject()
        l = QgsLayout(p)
        collection = l.pageCollection()

        # invalid pages
        self.assertEqual(
            collection.pagePositionToAbsolute(-1, QgsLayoutPoint(1, 1)),
            QgsLayoutPoint(1, 1),
        )
        self.assertEqual(
            collection.pagePositionToAbsolute(0, QgsLayoutPoint(1, 1)),
            QgsLayoutPoint(1, 1),
        )
        self.assertEqual(
            collection.pagePositionToAbsolute(100, QgsLayoutPoint(1, 1)),
            QgsLayoutPoint(1, 1),
        )

        # add a page
        page = QgsLayoutItemPage(l)
        page.setPageSize("A4")
        collection.addPage(page)

        # invalid pages
        self.assertEqual(
            collection.pagePositionToAbsolute(-1, QgsLayoutPoint(1, 1)),
            QgsLayoutPoint(1, 1),
        )
        self.assertEqual(
            collection.pagePositionToAbsolute(1, QgsLayoutPoint(1, 1)),
            QgsLayoutPoint(1, 1),
        )
        # valid page
        self.assertEqual(
            collection.pagePositionToAbsolute(0, QgsLayoutPoint(1, 1)),
            QgsLayoutPoint(1, 1),
        )
        self.assertEqual(
            collection.pagePositionToAbsolute(0, QgsLayoutPoint(5, 6)),
            QgsLayoutPoint(5, 6),
        )
        self.assertEqual(
            collection.pagePositionToAbsolute(
                0, QgsLayoutPoint(5, 6, QgsUnitTypes.LayoutUnit.LayoutCentimeters)
            ),
            QgsLayoutPoint(5, 6, QgsUnitTypes.LayoutUnit.LayoutCentimeters),
        )

        page2 = QgsLayoutItemPage(l)
        page2.setPageSize("A5")
        collection.addPage(page2)

        # invalid pages
        self.assertEqual(
            collection.pagePositionToAbsolute(-1, QgsLayoutPoint(1, 1)),
            QgsLayoutPoint(1, 1),
        )
        self.assertEqual(
            collection.pagePositionToAbsolute(3, QgsLayoutPoint(1, 1)),
            QgsLayoutPoint(1, 1),
        )
        # valid pages
        self.assertEqual(
            collection.pagePositionToAbsolute(0, QgsLayoutPoint(1, 1)),
            QgsLayoutPoint(1, 1),
        )
        self.assertEqual(
            collection.pagePositionToAbsolute(0, QgsLayoutPoint(5, 6)),
            QgsLayoutPoint(5, 6),
        )
        self.assertEqual(
            collection.pagePositionToAbsolute(
                0, QgsLayoutPoint(5, 6, QgsUnitTypes.LayoutUnit.LayoutCentimeters)
            ),
            QgsLayoutPoint(5, 6, QgsUnitTypes.LayoutUnit.LayoutCentimeters),
        )
        self.assertEqual(
            collection.pagePositionToAbsolute(1, QgsLayoutPoint(1, 1)),
            QgsLayoutPoint(1, 308.0),
        )
        self.assertEqual(
            collection.pagePositionToAbsolute(1, QgsLayoutPoint(5, 6)),
            QgsLayoutPoint(5, 313.0),
        )
        self.assertEqual(
            collection.pagePositionToAbsolute(
                1, QgsLayoutPoint(0.5, 0.6, QgsUnitTypes.LayoutUnit.LayoutCentimeters)
            ),
            QgsLayoutPoint(0.5, 31.3, QgsUnitTypes.LayoutUnit.LayoutCentimeters),
        )

    def testVisiblePages(self):
        p = QgsProject()
        l = QgsLayout(p)
        collection = l.pageCollection()

        self.assertFalse(collection.visiblePages(QRectF(0, 0, 10, 10)))
        self.assertFalse(collection.visiblePageNumbers(QRectF(0, 0, 10, 10)))

        # add a page
        page = QgsLayoutItemPage(l)
        page.setPageSize("A4")
        collection.addPage(page)

        self.assertFalse(collection.visiblePages(QRectF(-10, -10, 5, 5)))
        self.assertFalse(collection.visiblePageNumbers(QRectF(-10, -10, 5, 5)))
        self.assertEqual(collection.visiblePages(QRectF(-10, -10, 15, 15)), [page])
        self.assertEqual(collection.visiblePageNumbers(QRectF(-10, -10, 15, 15)), [0])
        self.assertEqual(collection.visiblePages(QRectF(200, 200, 115, 115)), [page])
        self.assertEqual(collection.visiblePageNumbers(QRectF(200, 200, 115, 115)), [0])

        page2 = QgsLayoutItemPage(l)
        page2.setPageSize("A5")
        collection.addPage(page2)

        self.assertFalse(collection.visiblePages(QRectF(-10, -10, 5, 5)))
        self.assertFalse(collection.visiblePageNumbers(QRectF(-10, -10, 5, 5)))
        self.assertEqual(collection.visiblePages(QRectF(-10, -10, 15, 15)), [page])
        self.assertEqual(collection.visiblePageNumbers(QRectF(-10, -10, 15, 15)), [0])
        self.assertEqual(collection.visiblePages(QRectF(200, 200, 115, 115)), [page])
        self.assertEqual(collection.visiblePageNumbers(QRectF(200, 200, 115, 115)), [0])

        self.assertEqual(collection.visiblePages(QRectF(200, 200, 115, 615)), [page])
        self.assertEqual(collection.visiblePageNumbers(QRectF(200, 200, 115, 115)), [0])
        self.assertEqual(
            collection.visiblePages(QRectF(100, 200, 115, 615)), [page, page2]
        )
        self.assertEqual(
            collection.visiblePageNumbers(QRectF(100, 200, 115, 115)), [0, 1]
        )
        self.assertEqual(collection.visiblePages(QRectF(100, 310, 115, 615)), [page2])
        self.assertEqual(collection.visiblePageNumbers(QRectF(100, 310, 115, 115)), [1])

    def testTakePage(self):
        p = QgsProject()
        l = QgsLayout(p)
        collection = l.pageCollection()

        # add some pages
        page = QgsLayoutItemPage(l)
        page.setPageSize("A4")
        collection.addPage(page)
        page2 = QgsLayoutItemPage(l)
        page2.setPageSize("A5")
        collection.addPage(page2)

        self.assertEqual(collection.pageCount(), 2)

        self.assertFalse(collection.takePage(None))

        self.assertEqual(collection.takePage(page), page)
        self.assertFalse(sip.isdeleted(page))

        self.assertEqual(collection.pageCount(), 1)
        self.assertEqual(collection.pages(), [page2])
        self.assertEqual(collection.page(0), page2)

        self.assertEqual(collection.takePage(page2), page2)
        self.assertFalse(sip.isdeleted(page2))

        self.assertEqual(collection.pageCount(), 0)
        self.assertEqual(collection.pages(), [])
        self.assertFalse(collection.page(0))

    def testReadWriteXml(self):
        p = QgsProject()
        l = QgsLayout(p)
        collection = l.pageCollection()

        fill = QgsSimpleFillSymbolLayer()
        fill_symbol = QgsFillSymbol()
        fill_symbol.changeSymbolLayer(0, fill)
        fill.setColor(Qt.GlobalColor.green)
        fill.setStrokeColor(Qt.GlobalColor.red)
        fill.setStrokeWidth(6)
        collection.setPageStyleSymbol(fill_symbol)

        # add a page
        page = QgsLayoutItemPage(l)
        page.setPageSize("A4")
        self.assertEqual(collection.pageNumber(page), -1)

        collection.addPage(page)

        # add a second page
        page2 = QgsLayoutItemPage(l)
        page2.setPageSize("A5")
        collection.addPage(page2)

        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")
        self.assertTrue(collection.writeXml(elem, doc, QgsReadWriteContext()))

        l2 = QgsLayout(p)
        collection2 = l2.pageCollection()

        self.assertTrue(
            collection2.readXml(elem.firstChildElement(), doc, QgsReadWriteContext())
        )

        self.assertEqual(collection2.pageCount(), 2)
        self.assertEqual(collection2.page(0).pageSize().width(), 210)
        self.assertEqual(collection2.page(0).pageSize().height(), 297)
        self.assertEqual(collection2.page(1).pageSize().width(), 148)
        self.assertEqual(collection2.page(1).pageSize().height(), 210)

        self.assertEqual(
            collection2.pageStyleSymbol().symbolLayer(0).color().name(), "#00ff00"
        )
        self.assertEqual(
            collection2.pageStyleSymbol().symbolLayer(0).strokeColor().name(), "#ff0000"
        )

    def testUndoRedo(self):
        p = QgsProject()
        l = QgsLayout(p)
        collection = l.pageCollection()

        # add a page
        page = QgsLayoutItemPage(l)
        page.setPageSize("A4")
        collection.addPage(page)
        self.assertEqual(collection.pageCount(), 1)

        l.undoStack().stack().undo()
        self.assertEqual(collection.pageCount(), 0)

        l.undoStack().stack().redo()
        self.assertEqual(collection.pageCount(), 1)
        # make sure page is accessible
        self.assertEqual(collection.page(0).pageSize().width(), 210)

        # add a second page
        page2 = QgsLayoutItemPage(l)
        page2.setPageSize("A5")
        collection.addPage(page2)

        # delete page
        collection.deletePage(collection.page(0))
        self.assertEqual(collection.pageCount(), 1)

        l.undoStack().stack().undo()
        self.assertEqual(collection.pageCount(), 2)
        # make sure pages are accessible
        self.assertEqual(collection.page(0).pageSize().width(), 210)
        self.assertEqual(collection.page(1).pageSize().width(), 148)

        l.undoStack().stack().undo()
        self.assertEqual(collection.pageCount(), 1)
        l.undoStack().stack().undo()
        self.assertEqual(collection.pageCount(), 0)

        l.undoStack().stack().redo()
        self.assertEqual(collection.pageCount(), 1)
        self.assertEqual(collection.page(0).pageSize().width(), 210)
        l.undoStack().stack().redo()
        self.assertEqual(collection.pageCount(), 2)
        self.assertEqual(collection.page(0).pageSize().width(), 210)
        self.assertEqual(collection.page(1).pageSize().width(), 148)
        l.undoStack().stack().redo()
        self.assertEqual(collection.pageCount(), 1)
        self.assertEqual(collection.page(0).pageSize().width(), 148)

    def testResizeToContents(self):
        p = QgsProject()
        l = QgsLayout(p)

        # no items -- no crash!
        l.pageCollection().resizeToContents(
            QgsMargins(1, 2, 3, 4), QgsUnitTypes.LayoutUnit.LayoutCentimeters
        )
        page = QgsLayoutItemPage(l)
        page.setPageSize("A5", QgsLayoutItemPage.Orientation.Landscape)
        l.pageCollection().addPage(page)
        # no items, no change
        l.pageCollection().resizeToContents(
            QgsMargins(1, 2, 3, 4), QgsUnitTypes.LayoutUnit.LayoutCentimeters
        )
        self.assertEqual(l.pageCollection().pageCount(), 1)
        self.assertAlmostEqual(
            l.pageCollection().page(0).sizeWithUnits().width(), 210.0, 2
        )
        self.assertAlmostEqual(
            l.pageCollection().page(0).sizeWithUnits().height(), 148.0, 2
        )

        p = QgsProject()
        l = QgsLayout(p)
        shape1 = QgsLayoutItemShape(l)
        shape1.attemptResize(QgsLayoutSize(90, 50))
        shape1.attemptMove(QgsLayoutPoint(90, 50))
        shape1.setItemRotation(45, False)
        l.addLayoutItem(shape1)
        shape2 = QgsLayoutItemShape(l)
        shape2.attemptResize(QgsLayoutSize(110, 50))
        shape2.attemptMove(QgsLayoutPoint(100, 150), True, False, 0)
        l.addLayoutItem(shape2)
        shape3 = QgsLayoutItemShape(l)
        l.addLayoutItem(shape3)
        shape3.attemptResize(QgsLayoutSize(50, 100))
        shape3.attemptMove(QgsLayoutPoint(210, 250), True, False, 0)
        shape4 = QgsLayoutItemShape(l)
        l.addLayoutItem(shape4)
        shape4.attemptResize(QgsLayoutSize(50, 30))
        shape4.attemptMove(QgsLayoutPoint(10, 340), True, False, 0)
        shape4.setVisibility(False)

        # resize with no existing pages
        l.pageCollection().resizeToContents(
            QgsMargins(1, 2, 3, 4), QgsUnitTypes.LayoutUnit.LayoutCentimeters
        )
        self.assertEqual(l.pageCollection().pageCount(), 1)

        self.assertAlmostEqual(
            l.pageCollection().page(0).sizeWithUnits().width(), 290.3, 2
        )
        self.assertAlmostEqual(
            l.pageCollection().page(0).sizeWithUnits().height(), 380.36, 2
        )
        self.assertAlmostEqual(
            l.pageCollection().page(0).sizeWithUnits().units(),
            QgsUnitTypes.LayoutUnit.LayoutMillimeters,
        )

        self.assertAlmostEqual(shape1.positionWithUnits().x(), 90.15, 2)
        self.assertAlmostEqual(shape1.positionWithUnits().y(), 20.21, 2)
        self.assertAlmostEqual(shape2.positionWithUnits().x(), 100.15, 2)
        self.assertAlmostEqual(shape2.positionWithUnits().y(), 120.21, 2)
        self.assertAlmostEqual(shape3.positionWithUnits().x(), 210.15, 2)
        self.assertAlmostEqual(shape3.positionWithUnits().y(), 220.21, 2)
        self.assertAlmostEqual(shape4.positionWithUnits().x(), 10.15, 2)
        self.assertAlmostEqual(shape4.positionWithUnits().y(), 310.21, 2)

        # add a second page
        page2 = QgsLayoutItemPage(l)
        page2.setPageSize("A4", QgsLayoutItemPage.Orientation.Landscape)
        l.pageCollection().addPage(page2)

        # add some guides
        g1 = QgsLayoutGuide(
            Qt.Orientation.Horizontal,
            QgsLayoutMeasurement(2.5, QgsUnitTypes.LayoutUnit.LayoutCentimeters),
            l.pageCollection().page(0),
        )
        l.guides().addGuide(g1)
        g2 = QgsLayoutGuide(
            Qt.Orientation.Vertical,
            QgsLayoutMeasurement(4.5, QgsUnitTypes.LayoutUnit.LayoutCentimeters),
            l.pageCollection().page(0),
        )
        l.guides().addGuide(g2)

        # second page should be removed
        l.pageCollection().resizeToContents(
            QgsMargins(0, 0, 0, 0), QgsUnitTypes.LayoutUnit.LayoutCentimeters
        )
        self.assertEqual(l.pageCollection().pageCount(), 1)

        self.assertAlmostEqual(
            l.pageCollection().page(0).sizeWithUnits().width(), 250.3, 2
        )
        self.assertAlmostEqual(
            l.pageCollection().page(0).sizeWithUnits().height(), 320.36, 2
        )
        self.assertAlmostEqual(
            l.pageCollection().page(0).sizeWithUnits().units(),
            QgsUnitTypes.LayoutUnit.LayoutMillimeters,
        )

        self.assertAlmostEqual(g1.position().length(), 0.5, 2)
        self.assertAlmostEqual(g2.position().length(), 3.5, 2)


if __name__ == "__main__":
    unittest.main()
