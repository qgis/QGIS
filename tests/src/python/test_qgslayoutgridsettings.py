# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsLayoutGridSettings.

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
                       QgsLayoutGridSettings,
                       QgsLayoutMeasurement,
                       QgsUnitTypes,
                       QgsLayoutPoint,
                       QgsLayoutItemPage,
                       QgsReadWriteContext)
from qgis.PyQt.QtGui import (QPen,
                             QColor)
from qgis.PyQt.QtXml import QDomDocument

from qgis.testing import start_app, unittest

start_app()


class TestQgsLayoutGridSettings(unittest.TestCase):

    def testGettersSetters(self):
        p = QgsProject()
        l = QgsLayout(p)
        s = QgsLayoutGridSettings(l)
        s.setResolution(QgsLayoutMeasurement(5, QgsUnitTypes.LayoutPoints))
        self.assertEqual(s.resolution().length(), 5.0)
        self.assertEqual(s.resolution().units(), QgsUnitTypes.LayoutPoints)

        s.setOffset(QgsLayoutPoint(6, 7, QgsUnitTypes.LayoutPixels))
        self.assertEqual(s.offset().x(), 6.0)
        self.assertEqual(s.offset().y(), 7.0)
        self.assertEqual(s.offset().units(), QgsUnitTypes.LayoutPixels)

        s.setPen(QPen(QColor(255, 0, 255)))
        self.assertEqual(s.pen().color().name(), QColor(255, 0, 255).name())

        s.setStyle(QgsLayoutGridSettings.StyleDots)
        self.assertEqual(s.style(), QgsLayoutGridSettings.StyleDots)

    def testReadWriteXml(self):
        p = QgsProject()
        l = QgsLayout(p)
        s = QgsLayoutGridSettings(l)
        s.setResolution(QgsLayoutMeasurement(5, QgsUnitTypes.LayoutPoints))
        s.setOffset(QgsLayoutPoint(6, 7, QgsUnitTypes.LayoutPixels))

        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")
        self.assertTrue(s.writeXml(elem, doc, QgsReadWriteContext()))

        s2 = QgsLayoutGridSettings(l)
        self.assertTrue(s2.readXml(elem.firstChildElement(), doc, QgsReadWriteContext()))

        self.assertEqual(s2.resolution().length(), 5.0)
        self.assertEqual(s2.resolution().units(), QgsUnitTypes.LayoutPoints)
        self.assertEqual(s2.offset().x(), 6.0)
        self.assertEqual(s2.offset().y(), 7.0)
        self.assertEqual(s2.offset().units(), QgsUnitTypes.LayoutPixels)

    def testUndoRedo(self):
        p = QgsProject()
        l = QgsLayout(p)
        g = l.gridSettings()
        g.setResolution(QgsLayoutMeasurement(15, QgsUnitTypes.LayoutPoints))

        # these two commands should be 'collapsed'
        g.setOffset(QgsLayoutPoint(555, 10, QgsUnitTypes.LayoutPoints))
        g.setOffset(QgsLayoutPoint(5, 10, QgsUnitTypes.LayoutPoints))

        # these two commands should be 'collapsed'
        g.setResolution(QgsLayoutMeasurement(45, QgsUnitTypes.LayoutInches))
        g.setResolution(QgsLayoutMeasurement(35, QgsUnitTypes.LayoutInches))

        self.assertEqual(g.offset().x(), 5.0)
        self.assertEqual(g.offset().y(), 10.0)
        self.assertEqual(g.offset().units(), QgsUnitTypes.LayoutPoints)
        self.assertEqual(g.resolution().length(), 35.0)
        self.assertEqual(g.resolution().units(), QgsUnitTypes.LayoutInches)

        l.undoStack().stack().undo()
        self.assertEqual(g.offset().x(), 5.0)
        self.assertEqual(g.offset().y(), 10.0)
        self.assertEqual(g.offset().units(), QgsUnitTypes.LayoutPoints)
        self.assertEqual(g.resolution().length(), 15.0)
        self.assertEqual(g.resolution().units(), QgsUnitTypes.LayoutPoints)

        l.undoStack().stack().undo()
        self.assertEqual(g.offset().x(), 0.0)
        self.assertEqual(g.offset().y(), 0.0)
        self.assertEqual(g.offset().units(), QgsUnitTypes.LayoutMillimeters)
        self.assertEqual(g.resolution().length(), 15.0)
        self.assertEqual(g.resolution().units(), QgsUnitTypes.LayoutPoints)

        l.undoStack().stack().redo()
        self.assertEqual(g.offset().x(), 5.0)
        self.assertEqual(g.offset().y(), 10.0)
        self.assertEqual(g.offset().units(), QgsUnitTypes.LayoutPoints)
        self.assertEqual(g.resolution().length(), 15.0)
        self.assertEqual(g.resolution().units(), QgsUnitTypes.LayoutPoints)

        l.undoStack().stack().redo()
        self.assertEqual(g.offset().x(), 5.0)
        self.assertEqual(g.offset().y(), 10.0)
        self.assertEqual(g.offset().units(), QgsUnitTypes.LayoutPoints)
        self.assertEqual(g.resolution().length(), 35.0)
        self.assertEqual(g.resolution().units(), QgsUnitTypes.LayoutInches)


if __name__ == '__main__':
    unittest.main()
