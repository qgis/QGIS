# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsLayout

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
                       QgsLayoutGuide,
                       QgsLayoutObject,
                       QgsProject,
                       QgsProperty,
                       QgsLayoutPageCollection,
                       QgsLayoutMeasurement,
                       QgsFillSymbol,
                       QgsReadWriteContext)
from qgis.PyQt.QtCore import Qt, QCoreApplication, QEvent, QPointF, QRectF
from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtXml import QDomDocument

from qgis.testing import start_app, unittest

start_app()


class TestQgsLayout(unittest.TestCase):

    def testReadWriteXml(self):
        p = QgsProject()
        l = QgsLayout(p)
        l.setName('my layout')
        l.setUnits(QgsUnitTypes.LayoutInches)
        collection = l.pageCollection()

        # add a page
        page = QgsLayoutItemPage(l)
        page.setPageSize('A6')
        collection.addPage(page)

        grid = l.gridSettings()
        grid.setResolution(QgsLayoutMeasurement(5, QgsUnitTypes.LayoutPoints))

        g1 = QgsLayoutGuide(QgsLayoutGuide.Horizontal, QgsLayoutMeasurement(5, QgsUnitTypes.LayoutCentimeters),
                            l.pageCollection().page(0))
        l.guides().addGuide(g1)

        snapper = l.snapper()
        snapper.setSnapTolerance(7)

        doc = QDomDocument("testdoc")
        elem = l.writeXml(doc, QgsReadWriteContext())

        l2 = QgsLayout(p)
        self.assertTrue(l2.readXml(elem, doc, QgsReadWriteContext()))
        self.assertEqual(l2.name(), 'my layout')
        self.assertEqual(l2.units(), QgsUnitTypes.LayoutInches)

        collection2 = l2.pageCollection()
        self.assertEqual(collection2.pageCount(), 1)
        self.assertAlmostEqual(collection2.page(0).pageSize().width(), 105, 4)
        self.assertEqual(collection2.page(0).pageSize().height(), 148)
        self.assertEqual(l2.gridSettings().resolution().length(), 5.0)
        self.assertEqual(l2.gridSettings().resolution().units(), QgsUnitTypes.LayoutPoints)
        self.assertEqual(l2.guides().guidesOnPage(0)[0].orientation(), QgsLayoutGuide.Horizontal)
        self.assertEqual(l2.guides().guidesOnPage(0)[0].position().length(), 5.0)
        self.assertEqual(l2.guides().guidesOnPage(0)[0].position().units(), QgsUnitTypes.LayoutCentimeters)
        self.assertEqual(l2.snapper().snapTolerance(), 7)


if __name__ == '__main__':
    unittest.main()
