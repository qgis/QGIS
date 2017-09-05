# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsLayoutPageCollection

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '18/07/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA
import sip

from qgis.core import (QgsUnitTypes,
                       QgsLayout,
                       QgsLayoutItemPage,
                       QgsLayoutSize,
                       QgsLayoutObject,
                       QgsProject,
                       QgsProperty,
                       QgsLayoutPageCollection,
                       QgsSimpleFillSymbolLayer,
                       QgsFillSymbol,
                       QgsReadWriteContext)
from qgis.PyQt.QtCore import Qt, QCoreApplication, QEvent, QPointF, QRectF
from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtXml import QDomDocument

from qgis.testing import start_app, unittest

start_app()


class TestQgsLayoutPageCollection(unittest.TestCase):

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
        fill.setColor(Qt.green)
        fill.setStrokeColor(Qt.red)
        fill.setStrokeWidth(6)
        collection.setPageStyleSymbol(fill_symbol)
        self.assertEqual(collection.pageStyleSymbol().symbolLayer(0).color().name(), '#00ff00')
        self.assertEqual(collection.pageStyleSymbol().symbolLayer(0).strokeColor().name(), '#ff0000')

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
        page.setPageSize('A4')
        self.assertEqual(collection.pageNumber(page), -1)

        collection.addPage(page)

        self.assertTrue(page in l.items())

        self.assertEqual(collection.pageCount(), 1)
        self.assertEqual(collection.pages(), [page])
        self.assertFalse(collection.page(-1))
        self.assertEqual(collection.page(0), page)
        self.assertFalse(collection.page(1))
        self.assertEqual(collection.pageNumber(page), 0)

        # add a second page
        page2 = QgsLayoutItemPage(l)
        page2.setPageSize('A5')
        collection.addPage(page2)

        self.assertEqual(collection.pageCount(), 2)
        self.assertEqual(collection.pages(), [page, page2])
        self.assertFalse(collection.page(-1))
        self.assertEqual(collection.page(0), page)
        self.assertEqual(collection.page(1), page2)
        self.assertEqual(collection.pageNumber(page2), 1)

        # insert a page
        page3 = QgsLayoutItemPage(l)
        page3.setPageSize('A3')
        collection.insertPage(page3, 1)
        self.assertTrue(page3 in l.items())

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
        QCoreApplication.sendPostedEvents(None, QEvent.DeferredDelete)
        self.assertTrue(sip.isdeleted(page3))

        del l
        QCoreApplication.sendPostedEvents(None, QEvent.DeferredDelete)
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
        page.setPageSize('A4')
        collection.addPage(page)
        # add a second page
        page2 = QgsLayoutItemPage(l)
        page2.setPageSize('A5')
        collection.addPage(page2)

        page_about_to_be_removed_spy = QSignalSpy(collection.pageAboutToBeRemoved)

        # delete page
        collection.deletePage(None)
        self.assertEqual(collection.pageCount(), 2)
        self.assertEqual(len(page_about_to_be_removed_spy), 0)

        page3 = QgsLayoutItemPage(l)
        # try deleting a page not in collection
        collection.deletePage(page3)
        QCoreApplication.sendPostedEvents(None, QEvent.DeferredDelete)
        self.assertFalse(sip.isdeleted(page3))
        self.assertEqual(collection.pageCount(), 2)
        self.assertEqual(len(page_about_to_be_removed_spy), 0)

        collection.deletePage(page)
        self.assertEqual(collection.pageCount(), 1)
        self.assertFalse(page in collection.pages())
        QCoreApplication.sendPostedEvents(None, QEvent.DeferredDelete)
        self.assertTrue(sip.isdeleted(page))
        self.assertEqual(len(page_about_to_be_removed_spy), 1)
        self.assertEqual(page_about_to_be_removed_spy[-1][0], 0)

        collection.deletePage(page2)
        self.assertEqual(collection.pageCount(), 0)
        self.assertFalse(collection.pages())
        QCoreApplication.sendPostedEvents(None, QEvent.DeferredDelete)
        self.assertTrue(sip.isdeleted(page2))
        self.assertEqual(len(page_about_to_be_removed_spy), 2)
        self.assertEqual(page_about_to_be_removed_spy[-1][0], 0)

    def testMaxPageWidth(self):
        """
        Test calculating maximum page width
        """
        p = QgsProject()
        l = QgsLayout(p)
        collection = l.pageCollection()

        # add a page
        page = QgsLayoutItemPage(l)
        page.setPageSize('A4')
        collection.addPage(page)
        self.assertEqual(collection.maximumPageWidth(), 210.0)

        # add a second page
        page2 = QgsLayoutItemPage(l)
        page2.setPageSize('A3')
        collection.addPage(page2)
        self.assertEqual(collection.maximumPageWidth(), 297.0)

        # add a page with other units
        page3 = QgsLayoutItemPage(l)
        page3.setPageSize(QgsLayoutSize(100, 100, QgsUnitTypes.LayoutMeters))
        collection.addPage(page3)
        self.assertEqual(collection.maximumPageWidth(), 100000.0)

    def testReflow(self):
        """
        Test reflowing pages
        """
        p = QgsProject()
        l = QgsLayout(p)
        collection = l.pageCollection()

        # add a page
        page = QgsLayoutItemPage(l)
        page.setPageSize('A4')
        collection.addPage(page)

        # should be positioned at origin
        self.assertEqual(page.pos().x(), 0)
        self.assertEqual(page.pos().y(), 0)

        # second page
        page2 = QgsLayoutItemPage(l)
        page2.setPageSize('A5')
        collection.addPage(page2)

        self.assertEqual(page.pos().x(), 0)
        self.assertEqual(page.pos().y(), 0)
        self.assertEqual(page2.pos().x(), 0)
        self.assertEqual(page2.pos().y(), 307)

        # third page, slotted in middle
        page3 = QgsLayoutItemPage(l)
        page3.setPageSize('A3')
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

    def testDataDefinedSize(self):
        p = QgsProject()
        l = QgsLayout(p)
        collection = l.pageCollection()

        # add some pages
        page = QgsLayoutItemPage(l)
        page.setPageSize('A4')
        collection.addPage(page)
        page2 = QgsLayoutItemPage(l)
        page2.setPageSize('A5')
        collection.addPage(page2)
        page3 = QgsLayoutItemPage(l)
        page3.setPageSize('A5')
        collection.addPage(page3)

        self.assertEqual(page.pos().x(), 0)
        self.assertEqual(page.pos().y(), 0)
        self.assertEqual(page2.pos().x(), 0)
        self.assertEqual(page2.pos().y(), 307)
        self.assertEqual(page3.pos().x(), 0)
        self.assertEqual(page3.pos().y(), 527)

        page.dataDefinedProperties().setProperty(QgsLayoutObject.ItemHeight, QgsProperty.fromExpression('50*3'))
        page.refresh()
        collection.reflow()
        self.assertEqual(page.pos().x(), 0)
        self.assertEqual(page.pos().y(), 0)
        self.assertEqual(page2.pos().x(), 0)
        self.assertEqual(page2.pos().y(), 160)
        self.assertEqual(page3.pos().x(), 0)
        self.assertEqual(page3.pos().y(), 380)

        page2.dataDefinedProperties().setProperty(QgsLayoutObject.ItemHeight, QgsProperty.fromExpression('50-20'))
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
        page.setPageSize('A4')
        collection.addPage(page)

        self.assertEqual(collection.pageNumberForPoint(QPointF(-100, -100)), 0)
        self.assertEqual(collection.pageNumberForPoint(QPointF(-100, -1)), 0)
        self.assertEqual(collection.pageNumberForPoint(QPointF(-100, 1)), 0)
        self.assertEqual(collection.pageNumberForPoint(QPointF(-100, 270)), 0)
        self.assertEqual(collection.pageNumberForPoint(QPointF(-100, 1270)), 0)

        self.assertEqual(collection.positionOnPage(QPointF(-100, -100)), QPointF(-100, -100))
        self.assertEqual(collection.positionOnPage(QPointF(-100, -1)), QPointF(-100, -1))
        self.assertEqual(collection.positionOnPage(QPointF(-100, 1)), QPointF(-100, 1))
        self.assertEqual(collection.positionOnPage(QPointF(-100, 270)), QPointF(-100, 270))
        self.assertEqual(collection.positionOnPage(QPointF(-100, 1270)), QPointF(-100, 973))

        page2 = QgsLayoutItemPage(l)
        page2.setPageSize('A5')
        collection.addPage(page2)

        self.assertEqual(collection.pageNumberForPoint(QPointF(-100, -100)), 0)
        self.assertEqual(collection.pageNumberForPoint(QPointF(-100, -1)), 0)
        self.assertEqual(collection.pageNumberForPoint(QPointF(-100, 1)), 0)
        self.assertEqual(collection.pageNumberForPoint(QPointF(-100, 270)), 0)
        self.assertEqual(collection.pageNumberForPoint(QPointF(-100, 370)), 1)
        self.assertEqual(collection.pageNumberForPoint(QPointF(-100, 1270)), 1)

        self.assertEqual(collection.positionOnPage(QPointF(-100, -100)), QPointF(-100, -100))
        self.assertEqual(collection.positionOnPage(QPointF(-100, -1)), QPointF(-100, -1))
        self.assertEqual(collection.positionOnPage(QPointF(-100, 1)), QPointF(-100, 1))
        self.assertEqual(collection.positionOnPage(QPointF(-100, 270)), QPointF(-100, 270))
        self.assertEqual(collection.positionOnPage(QPointF(-100, 370)), QPointF(-100, 63))
        self.assertEqual(collection.positionOnPage(QPointF(-100, 1270)), QPointF(-100, 753))

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
        page.setPageSize('A4')
        collection.addPage(page)

        self.assertFalse(collection.pageAtPoint(QPointF(10, -1)))
        self.assertEqual(collection.pageAtPoint(QPointF(1, 1)), page)
        self.assertEqual(collection.pageAtPoint(QPointF(10, 10)), page)
        self.assertFalse(collection.pageAtPoint(QPointF(-10, 10)))
        self.assertFalse(collection.pageAtPoint(QPointF(1000, 10)))
        self.assertFalse(collection.pageAtPoint(QPointF(10, -10)))
        self.assertFalse(collection.pageAtPoint(QPointF(10, 1000)))

        page2 = QgsLayoutItemPage(l)
        page2.setPageSize('A5')
        collection.addPage(page2)

        self.assertEqual(collection.pageAtPoint(QPointF(1, 1)), page)
        self.assertEqual(collection.pageAtPoint(QPointF(10, 10)), page)
        self.assertFalse(collection.pageAtPoint(QPointF(-10, 10)))
        self.assertFalse(collection.pageAtPoint(QPointF(1000, 10)))
        self.assertFalse(collection.pageAtPoint(QPointF(10, -10)))
        self.assertEqual(collection.pageAtPoint(QPointF(10, 330)), page2)
        self.assertEqual(collection.pageAtPoint(QPointF(10, 500)), page2)
        self.assertFalse(collection.pageAtPoint(QPointF(10, 600)))

    def testVisiblePages(self):
        p = QgsProject()
        l = QgsLayout(p)
        collection = l.pageCollection()

        self.assertFalse(collection.visiblePages(QRectF(0, 0, 10, 10)))
        self.assertFalse(collection.visiblePageNumbers(QRectF(0, 0, 10, 10)))

        # add a page
        page = QgsLayoutItemPage(l)
        page.setPageSize('A4')
        collection.addPage(page)

        self.assertFalse(collection.visiblePages(QRectF(-10, -10, 5, 5)))
        self.assertFalse(collection.visiblePageNumbers(QRectF(-10, -10, 5, 5)))
        self.assertEqual(collection.visiblePages(QRectF(-10, -10, 15, 15)), [page])
        self.assertEqual(collection.visiblePageNumbers(QRectF(-10, -10, 15, 15)), [0])
        self.assertEqual(collection.visiblePages(QRectF(200, 200, 115, 115)), [page])
        self.assertEqual(collection.visiblePageNumbers(QRectF(200, 200, 115, 115)), [0])

        page2 = QgsLayoutItemPage(l)
        page2.setPageSize('A5')
        collection.addPage(page2)

        self.assertFalse(collection.visiblePages(QRectF(-10, -10, 5, 5)))
        self.assertFalse(collection.visiblePageNumbers(QRectF(-10, -10, 5, 5)))
        self.assertEqual(collection.visiblePages(QRectF(-10, -10, 15, 15)), [page])
        self.assertEqual(collection.visiblePageNumbers(QRectF(-10, -10, 15, 15)), [0])
        self.assertEqual(collection.visiblePages(QRectF(200, 200, 115, 115)), [page])
        self.assertEqual(collection.visiblePageNumbers(QRectF(200, 200, 115, 115)), [0])

        self.assertEqual(collection.visiblePages(QRectF(200, 200, 115, 615)), [page])
        self.assertEqual(collection.visiblePageNumbers(QRectF(200, 200, 115, 115)), [0])
        self.assertEqual(collection.visiblePages(QRectF(100, 200, 115, 615)), [page, page2])
        self.assertEqual(collection.visiblePageNumbers(QRectF(100, 200, 115, 115)), [0, 1])
        self.assertEqual(collection.visiblePages(QRectF(100, 310, 115, 615)), [page2])
        self.assertEqual(collection.visiblePageNumbers(QRectF(100, 310, 115, 115)), [1])

    def testTakePage(self):
        p = QgsProject()
        l = QgsLayout(p)
        collection = l.pageCollection()

        # add some pages
        page = QgsLayoutItemPage(l)
        page.setPageSize('A4')
        collection.addPage(page)
        page2 = QgsLayoutItemPage(l)
        page2.setPageSize('A5')
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
        fill.setColor(Qt.green)
        fill.setStrokeColor(Qt.red)
        fill.setStrokeWidth(6)
        collection.setPageStyleSymbol(fill_symbol)

        # add a page
        page = QgsLayoutItemPage(l)
        page.setPageSize('A4')
        self.assertEqual(collection.pageNumber(page), -1)

        collection.addPage(page)

        # add a second page
        page2 = QgsLayoutItemPage(l)
        page2.setPageSize('A5')
        collection.addPage(page2)

        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")
        self.assertTrue(collection.writeXml(elem, doc, QgsReadWriteContext()))

        l2 = QgsLayout(p)
        collection2 = l2.pageCollection()

        self.assertTrue(collection2.readXml(elem.firstChildElement(), doc, QgsReadWriteContext()))

        self.assertEqual(collection2.pageCount(), 2)
        self.assertEqual(collection2.page(0).pageSize().width(), 210)
        self.assertEqual(collection2.page(0).pageSize().height(), 297)
        self.assertEqual(collection2.page(1).pageSize().width(), 148)
        self.assertEqual(collection2.page(1).pageSize().height(), 210)

        self.assertEqual(collection2.pageStyleSymbol().symbolLayer(0).color().name(), '#00ff00')
        self.assertEqual(collection2.pageStyleSymbol().symbolLayer(0).strokeColor().name(), '#ff0000')

    def testUndoRedo(self):
        p = QgsProject()
        l = QgsLayout(p)
        collection = l.pageCollection()

        # add a page
        page = QgsLayoutItemPage(l)
        page.setPageSize('A4')
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
        page2.setPageSize('A5')
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


if __name__ == '__main__':
    unittest.main()
