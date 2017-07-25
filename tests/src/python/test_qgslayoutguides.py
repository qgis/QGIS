# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsLayoutGuide/QgsLayoutGuideCollection.

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
                       QgsLayoutGuide,
                       QgsLayoutMeasurement,
                       QgsUnitTypes,
                       QgsLayoutPoint,
                       QgsLayoutItemPage)
from qgis.PyQt.QtGui import (QPen,
                             QColor)

from qgis.testing import start_app, unittest

start_app()


class TestQgsLayoutGuide(unittest.TestCase):

    def testGuideGettersSetters(self):
        p = QgsProject()
        l = QgsLayout(p)
        l.initializeDefaults()  # add a page
        g = QgsLayoutGuide(l, QgsLayoutGuide.Horizontal, QgsLayoutMeasurement(5, QgsUnitTypes.LayoutCentimeters))
        self.assertEqual(g.orientation(), QgsLayoutGuide.Horizontal)
        self.assertEqual(g.position().length(), 5.0)
        self.assertEqual(g.position().units(), QgsUnitTypes.LayoutCentimeters)

        g.setPosition(QgsLayoutMeasurement(15, QgsUnitTypes.LayoutInches))
        self.assertEqual(g.position().length(), 15.0)
        self.assertEqual(g.position().units(), QgsUnitTypes.LayoutInches)

        g.setPage(1)
        self.assertEqual(g.page(), 1)

    def testUpdateGuide(self):
        p = QgsProject()
        l = QgsLayout(p)
        l.initializeDefaults() # add a page
        g = QgsLayoutGuide(l, QgsLayoutGuide.Horizontal, QgsLayoutMeasurement(5, QgsUnitTypes.LayoutCentimeters))
        g.update()

        self.assertTrue(g.item().isVisible())
        self.assertEqual(g.item().line().x1(), 0)
        self.assertEqual(g.item().line().y1(), 50)
        self.assertEqual(g.item().line().x2(), 297)
        self.assertEqual(g.item().line().y2(), 50)

        g.setPosition(QgsLayoutMeasurement(15, QgsUnitTypes.LayoutMillimeters))
        g.update()
        self.assertTrue(g.item().isVisible())
        self.assertEqual(g.item().line().x1(), 0)
        self.assertEqual(g.item().line().y1(), 15)
        self.assertEqual(g.item().line().x2(), 297)
        self.assertEqual(g.item().line().y2(), 15)

        g.setPage(10)
        g.update()
        self.assertFalse(g.item().isVisible())

        g.setPage(0)
        g.update()
        self.assertTrue(g.item().isVisible())

        #throw it off the bottom of the page
        g.setPosition(QgsLayoutMeasurement(1115, QgsUnitTypes.LayoutMillimeters))
        g.update()
        self.assertFalse(g.item().isVisible())


if __name__ == '__main__':
    unittest.main()
