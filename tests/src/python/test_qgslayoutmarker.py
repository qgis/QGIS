# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsLayoutItemMarker.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2020 by Nyall Dawson'
__date__ = '05/04/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import qgis  # NOQA

from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtXml import QDomDocument

from qgis.core import (QgsLayoutItemMarker,
                       QgsLayoutItemRegistry,
                       QgsLayout,
                       QgsMarkerSymbol,
                       QgsProject,
                       QgsReadWriteContext,
                       QgsLayoutPoint,
                       QgsUnitTypes)
from qgis.testing import (start_app,
                          unittest
                          )
from utilities import unitTestDataPath
from qgslayoutchecker import QgsLayoutChecker
from test_qgslayoutitem import LayoutItemTestCase

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsLayoutMarker(unittest.TestCase, LayoutItemTestCase):

    @classmethod
    def setUpClass(cls):
        cls.item_class = QgsLayoutItemMarker

    def __init__(self, methodName):
        """Run once on class initialization."""
        unittest.TestCase.__init__(self, methodName)

        # style
        props = {}
        props["color"] = "green"
        props["style"] = "solid"
        props["style_border"] = "solid"
        props["color_border"] = "black"
        props["width_border"] = "10.0"
        props["joinstyle"] = "miter"

        style = QgsMarkerSymbol.createSimple(props)

    def testDisplayName(self):
        """Test if displayName is valid"""

        layout = QgsLayout(QgsProject.instance())
        marker = QgsLayoutItemMarker(layout)
        self.assertEqual(marker.displayName(), "<Marker>")
        marker.setId('id')
        self.assertEqual(marker.displayName(), "id")

    def testType(self):
        """Test if type is valid"""
        layout = QgsLayout(QgsProject.instance())
        marker = QgsLayoutItemMarker(layout)

        self.assertEqual(
            marker.type(), QgsLayoutItemRegistry.LayoutMarker)

    def testRender(self):
        """Test marker rendering."""
        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()
        marker = QgsLayoutItemMarker(layout)
        marker.attemptMove(QgsLayoutPoint(100, 50, QgsUnitTypes.LayoutMillimeters))
        props = {}
        props["color"] = "0,255,255"
        props["outline_width"] = "4"
        props["outline_color"] = "0,0,0"
        props["size"] = "14.4"

        style = QgsMarkerSymbol.createSimple(props)
        marker.setSymbol(style)
        layout.addLayoutItem(marker)
        checker = QgsLayoutChecker(
            'layout_marker_render', layout)
        checker.setControlPathPrefix("layout_marker")
        myTestResult, myMessage = checker.testLayout()
        assert myTestResult, myMessage

    def testReadWriteXml(self):
        pr = QgsProject()
        l = QgsLayout(pr)
        marker = QgsLayoutItemMarker(l)
        l.addLayoutItem(marker)

        props = {}
        props["color"] = "green"
        props["outline_style"] = "no"
        props["size"] = "4.4"

        style = QgsMarkerSymbol.createSimple(props)
        marker.setSymbol(style)

        #save original item to xml
        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")
        self.assertTrue(marker.writeXml(elem, doc, QgsReadWriteContext()))

        marker2 = QgsLayoutItemMarker(l)
        self.assertTrue(marker2.readXml(elem.firstChildElement(), doc, QgsReadWriteContext()))

        self.assertEqual(marker2.symbol().symbolLayer(0).color().name(), '#008000')
        self.assertEqual(marker2.symbol().symbolLayer(0).strokeStyle(), Qt.NoPen)
        self.assertEqual(marker2.symbol().symbolLayer(0).size(), 4.4)

    def testBounds(self):
        pr = QgsProject()
        l = QgsLayout(pr)

        shape = QgsLayoutItemMarker(l)
        shape.attemptMove(QgsLayoutPoint(10, 20, QgsUnitTypes.LayoutMillimeters))
        props = {}
        props["shape"] = "square"
        props["size"] = "6"
        props["outline_width"] = "2"
        style = QgsMarkerSymbol.createSimple(props)
        shape.setSymbol(style)

        # these must match symbol size
        size = shape.sizeWithUnits().toQSizeF()
        self.assertAlmostEqual(size.width(), 8.0846, 1)
        self.assertAlmostEqual(size.height(), 8.08, 1)
        pos = shape.positionWithUnits().toQPointF()
        self.assertAlmostEqual(pos.x(), 10.0, 1)
        self.assertAlmostEqual(pos.y(), 20.0, 1)

        # these are just rough!
        bounds = shape.sceneBoundingRect()
        self.assertAlmostEqual(bounds.left(), 0.957, 1)
        self.assertAlmostEqual(bounds.right(), 19.04, 1)
        self.assertAlmostEqual(bounds.top(), 10.95, 1)
        self.assertAlmostEqual(bounds.bottom(), 29.04, 1)


if __name__ == '__main__':
    unittest.main()
