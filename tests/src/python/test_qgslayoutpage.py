"""QGIS Unit tests for QgsLayoutItemPage.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "(C) 2017 by Nyall Dawson"
__date__ = "23/10/2017"
__copyright__ = "Copyright 2017, The QGIS Project"

from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    QgsFillSymbol,
    QgsLayout,
    QgsLayoutItemPage,
    QgsProject,
    QgsReadWriteContext,
    QgsSimpleFillSymbolLayer,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from test_qgslayoutitem import LayoutItemTestCase

start_app()


class TestQgsLayoutPage(QgisTestCase, LayoutItemTestCase):

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls.item_class = QgsLayoutItemPage

    def testDefaults(self):
        p = QgsProject()
        l = QgsLayout(p)
        p = QgsLayoutItemPage(l)
        self.assertTrue(p.pageStyleSymbol())

        fill = QgsSimpleFillSymbolLayer()
        fill_symbol = QgsFillSymbol()
        fill_symbol.changeSymbolLayer(0, fill)
        fill.setColor(Qt.GlobalColor.green)
        fill.setStrokeColor(Qt.GlobalColor.red)
        fill.setStrokeWidth(6)
        p.setPageStyleSymbol(fill_symbol)

        self.assertEqual(p.pageStyleSymbol().symbolLayer(0).color().name(), "#00ff00")
        self.assertEqual(
            p.pageStyleSymbol().symbolLayer(0).strokeColor().name(), "#ff0000"
        )

    def testReadWriteSettings(self):
        p = QgsProject()
        l = QgsLayout(p)
        collection = l.pageCollection()

        fill = QgsSimpleFillSymbolLayer()
        fill_symbol = QgsFillSymbol()
        fill_symbol.changeSymbolLayer(0, fill)
        fill.setColor(Qt.GlobalColor.green)
        fill.setStrokeColor(Qt.GlobalColor.red)
        fill.setStrokeWidth(6)

        page = QgsLayoutItemPage(l)
        page.setPageSize("A4")
        page.setPageStyleSymbol(fill_symbol.clone())
        self.assertEqual(collection.pageNumber(page), -1)
        collection.addPage(page)

        # add a second page
        page2 = QgsLayoutItemPage(l)
        page2.setPageSize("A5")
        fill_symbol.setColor(Qt.GlobalColor.blue)
        page2.setPageStyleSymbol(fill_symbol.clone())
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

        self.assertEqual(
            collection2.page(0).pageStyleSymbol().symbolLayer(0).color().name(),
            "#00ff00",
        )
        self.assertEqual(
            collection2.page(0).pageStyleSymbol().symbolLayer(0).strokeColor().name(),
            "#ff0000",
        )

        self.assertEqual(
            collection2.page(1).pageStyleSymbol().symbolLayer(0).color().name(),
            "#0000ff",
        )
        self.assertEqual(
            collection2.page(1).pageStyleSymbol().symbolLayer(0).strokeColor().name(),
            "#ff0000",
        )


if __name__ == "__main__":
    unittest.main()
