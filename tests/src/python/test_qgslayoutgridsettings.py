"""QGIS Unit tests for QgsLayoutGridSettings.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "05/07/2017"
__copyright__ = "Copyright 2017, The QGIS Project"

from qgis.PyQt.QtGui import QColor, QPen
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    QgsLayout,
    QgsLayoutGridSettings,
    QgsLayoutMeasurement,
    QgsLayoutPoint,
    QgsProject,
    QgsReadWriteContext,
    QgsUnitTypes,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsLayoutGridSettings(QgisTestCase):

    def testGettersSetters(self):
        p = QgsProject()
        l = QgsLayout(p)
        s = QgsLayoutGridSettings(l)
        s.setResolution(QgsLayoutMeasurement(5, QgsUnitTypes.LayoutUnit.LayoutPoints))
        self.assertEqual(s.resolution().length(), 5.0)
        self.assertEqual(s.resolution().units(), QgsUnitTypes.LayoutUnit.LayoutPoints)

        s.setOffset(QgsLayoutPoint(6, 7, QgsUnitTypes.LayoutUnit.LayoutPixels))
        self.assertEqual(s.offset().x(), 6.0)
        self.assertEqual(s.offset().y(), 7.0)
        self.assertEqual(s.offset().units(), QgsUnitTypes.LayoutUnit.LayoutPixels)

        s.setPen(QPen(QColor(255, 0, 255)))
        self.assertEqual(s.pen().color().name(), QColor(255, 0, 255).name())

        s.setStyle(QgsLayoutGridSettings.Style.StyleDots)
        self.assertEqual(s.style(), QgsLayoutGridSettings.Style.StyleDots)

    def testReadWriteXml(self):
        p = QgsProject()
        l = QgsLayout(p)
        s = QgsLayoutGridSettings(l)
        s.setResolution(QgsLayoutMeasurement(5, QgsUnitTypes.LayoutUnit.LayoutPoints))
        s.setOffset(QgsLayoutPoint(6, 7, QgsUnitTypes.LayoutUnit.LayoutPixels))

        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")
        self.assertTrue(s.writeXml(elem, doc, QgsReadWriteContext()))

        s2 = QgsLayoutGridSettings(l)
        self.assertTrue(
            s2.readXml(elem.firstChildElement(), doc, QgsReadWriteContext())
        )

        self.assertEqual(s2.resolution().length(), 5.0)
        self.assertEqual(s2.resolution().units(), QgsUnitTypes.LayoutUnit.LayoutPoints)
        self.assertEqual(s2.offset().x(), 6.0)
        self.assertEqual(s2.offset().y(), 7.0)
        self.assertEqual(s2.offset().units(), QgsUnitTypes.LayoutUnit.LayoutPixels)

    def testUndoRedo(self):
        p = QgsProject()
        l = QgsLayout(p)
        g = l.gridSettings()
        g.setResolution(QgsLayoutMeasurement(15, QgsUnitTypes.LayoutUnit.LayoutPoints))

        # these two commands should be 'collapsed'
        g.setOffset(QgsLayoutPoint(555, 10, QgsUnitTypes.LayoutUnit.LayoutPoints))
        g.setOffset(QgsLayoutPoint(5, 10, QgsUnitTypes.LayoutUnit.LayoutPoints))

        # these two commands should be 'collapsed'
        g.setResolution(QgsLayoutMeasurement(45, QgsUnitTypes.LayoutUnit.LayoutInches))
        g.setResolution(QgsLayoutMeasurement(35, QgsUnitTypes.LayoutUnit.LayoutInches))

        self.assertEqual(g.offset().x(), 5.0)
        self.assertEqual(g.offset().y(), 10.0)
        self.assertEqual(g.offset().units(), QgsUnitTypes.LayoutUnit.LayoutPoints)
        self.assertEqual(g.resolution().length(), 35.0)
        self.assertEqual(g.resolution().units(), QgsUnitTypes.LayoutUnit.LayoutInches)

        l.undoStack().stack().undo()
        self.assertEqual(g.offset().x(), 5.0)
        self.assertEqual(g.offset().y(), 10.0)
        self.assertEqual(g.offset().units(), QgsUnitTypes.LayoutUnit.LayoutPoints)
        self.assertEqual(g.resolution().length(), 15.0)
        self.assertEqual(g.resolution().units(), QgsUnitTypes.LayoutUnit.LayoutPoints)

        l.undoStack().stack().undo()
        self.assertEqual(g.offset().x(), 0.0)
        self.assertEqual(g.offset().y(), 0.0)
        self.assertEqual(g.offset().units(), QgsUnitTypes.LayoutUnit.LayoutMillimeters)
        self.assertEqual(g.resolution().length(), 15.0)
        self.assertEqual(g.resolution().units(), QgsUnitTypes.LayoutUnit.LayoutPoints)

        l.undoStack().stack().redo()
        self.assertEqual(g.offset().x(), 5.0)
        self.assertEqual(g.offset().y(), 10.0)
        self.assertEqual(g.offset().units(), QgsUnitTypes.LayoutUnit.LayoutPoints)
        self.assertEqual(g.resolution().length(), 15.0)
        self.assertEqual(g.resolution().units(), QgsUnitTypes.LayoutUnit.LayoutPoints)

        l.undoStack().stack().redo()
        self.assertEqual(g.offset().x(), 5.0)
        self.assertEqual(g.offset().y(), 10.0)
        self.assertEqual(g.offset().units(), QgsUnitTypes.LayoutUnit.LayoutPoints)
        self.assertEqual(g.resolution().length(), 35.0)
        self.assertEqual(g.resolution().units(), QgsUnitTypes.LayoutUnit.LayoutInches)


if __name__ == "__main__":
    unittest.main()
