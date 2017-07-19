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
                       QgsProject,
                       QgsLayoutPageCollection,
                       QgsSimpleFillSymbolLayer,
                       QgsFillSymbol)
from qgis.PyQt.QtCore import Qt, QCoreApplication, QEvent
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
        collection.addPage(page)

        self.assertTrue(page in l.items())

        self.assertEqual(collection.pageCount(), 1)
        self.assertEqual(collection.pages(), [page])
        self.assertFalse(collection.page(-1))
        self.assertEqual(collection.page(0), page)
        self.assertFalse(collection.page(1))

        # add a second page
        page2 = QgsLayoutItemPage(l)
        page2.setPageSize('A5')
        collection.addPage(page2)

        self.assertEqual(collection.pageCount(), 2)
        self.assertEqual(collection.pages(), [page, page2])
        self.assertFalse(collection.page(-1))
        self.assertEqual(collection.page(0), page)
        self.assertEqual(collection.page(1), page2)

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

        #add a page
        page = QgsLayoutItemPage(l)
        page.setPageSize('A4')
        collection.addPage(page)

        #should be positioned at origin
        self.assertEqual(page.pos().x(), 0)
        self.assertEqual(page.pos().y(), 0)

        #second page
        page2 = QgsLayoutItemPage(l)
        page2.setPageSize('A5')
        collection.addPage(page2)

        self.assertEqual(page.pos().x(), 0)
        self.assertEqual(page.pos().y(), 0)
        self.assertEqual(page2.pos().x(), 0)
        self.assertEqual(page2.pos().y(), 307)

        #third page, slotted in middle
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

if __name__ == '__main__':
    unittest.main()
