# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsLayoutItemPolyline.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2016 by Paul Blottiere'
__date__ = '14/03/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'

import qgis  # NOQA

from qgis.PyQt.QtGui import QPolygonF
from qgis.PyQt.QtCore import QPointF
from qgis.PyQt.QtXml import QDomDocument

from qgis.core import (QgsLayoutItemPolyline,
                       QgsLayoutItemRegistry,
                       QgsLayout,
                       QgsLineSymbol,
                       QgsProject,
                       QgsReadWriteContext)
from qgis.testing import (start_app,
                          unittest
                          )
from utilities import unitTestDataPath
from qgslayoutchecker import QgsLayoutChecker
from test_qgslayoutitem import LayoutItemTestCase

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsLayoutPolyline(unittest.TestCase, LayoutItemTestCase):

    @classmethod
    def setUpClass(cls):
        cls.item_class = QgsLayoutItemPolyline

    def __init__(self, methodName):
        """Run once on class initialization."""
        unittest.TestCase.__init__(self, methodName)

        # create composition
        self.layout = QgsLayout(QgsProject.instance())
        self.layout.initializeDefaults()

        # create
        polygon = QPolygonF()
        polygon.append(QPointF(0.0, 0.0))
        polygon.append(QPointF(100.0, 0.0))
        polygon.append(QPointF(200.0, 100.0))
        polygon.append(QPointF(100.0, 200.0))

        self.polyline = QgsLayoutItemPolyline(
            polygon, self.layout)
        self.layout.addLayoutItem(self.polyline)

        # style
        props = {}
        props["color"] = "0,0,0,255"
        props["width"] = "10.0"
        props["capstyle"] = "square"

        style = QgsLineSymbol.createSimple(props)
        self.polyline.setSymbol(style)

    def testNodes(self):
        polygon = QPolygonF()
        polygon.append(QPointF(0.0, 0.0))
        polygon.append(QPointF(100.0, 0.0))
        polygon.append(QPointF(200.0, 100.0))
        polygon.append(QPointF(100.0, 200.0))

        p = QgsLayoutItemPolyline(polygon, self.layout)
        self.assertEqual(p.nodes(), polygon)

        polygon = QPolygonF()
        polygon.append(QPointF(0.0, 0.0))
        polygon.append(QPointF(1000.0, 0.0))
        polygon.append(QPointF(2000.0, 100.0))
        polygon.append(QPointF(1000.0, 200.0))

        p.setNodes(polygon)
        self.assertEqual(p.nodes(), polygon)

    def testDisplayName(self):
        """Test if displayName is valid"""

        self.assertEqual(self.polyline.displayName(), "<Polyline>")

    def testType(self):
        """Test if type is valid"""

        self.assertEqual(
            self.polyline.type(), QgsLayoutItemRegistry.LayoutPolyline)

    def testDefaultStyle(self):
        """Test polygon rendering with default style."""

        self.polyline.setDisplayNodes(False)
        checker = QgsLayoutChecker(
            'composerpolyline_defaultstyle', self.layout)
        checker.setControlPathPrefix("composer_polyline")
        myTestResult, myMessage = checker.testLayout()
        assert myTestResult, myMessage

    def testDisplayNodes(self):
        """Test displayNodes method"""

        self.polyline.setDisplayNodes(True)
        checker = QgsLayoutChecker(
            'composerpolyline_displaynodes', self.layout)
        checker.setControlPathPrefix("composer_polyline")
        myTestResult, myMessage = checker.testLayout()
        assert myTestResult, myMessage

        self.polyline.setDisplayNodes(False)
        checker = QgsLayoutChecker(
            'composerpolyline_defaultstyle', self.layout)
        checker.setControlPathPrefix("composer_polyline")
        myTestResult, myMessage = checker.testLayout()
        assert myTestResult, myMessage

    def testSelectedNode(self):
        """Test selectedNode and deselectNode methods"""

        self.polyline.setDisplayNodes(True)

        self.polyline.setSelectedNode(3)
        checker = QgsLayoutChecker(
            'composerpolyline_selectednode', self.layout)
        checker.setControlPathPrefix("composer_polyline")
        myTestResult, myMessage = checker.testLayout()
        assert myTestResult, myMessage

        self.polyline.deselectNode()
        self.polyline.setDisplayNodes(False)
        checker = QgsLayoutChecker(
            'composerpolyline_defaultstyle', self.layout)
        checker.setControlPathPrefix("composer_polyline")
        myTestResult, myMessage = checker.testLayout()
        assert myTestResult, myMessage

    def testEndArrow(self):
        self.polyline.setEndMarker(QgsLayoutItemPolyline.ArrowHead)
        self.polyline.setArrowHeadWidth(30.0)

        checker = QgsLayoutChecker('composerpolyline_endArrow', self.layout)
        checker.setControlPathPrefix("composer_polyline")
        myTestResult, myMessage = checker.testLayout()
        assert myTestResult, myMessage

        self.polyline.setEndMarker(QgsLayoutItemPolyline.NoMarker)

    def testRemoveNode(self):
        """Test removeNode method"""

        rc = self.polyline.removeNode(100)
        self.assertEqual(rc, False)

        checker = QgsLayoutChecker(
            'composerpolyline_defaultstyle', self.layout)
        checker.setControlPathPrefix("composer_polyline")
        myTestResult, myMessage = checker.testLayout()
        assert myTestResult, myMessage

        self.assertEqual(self.polyline.nodesSize(), 4)
        rc = self.polyline.removeNode(3)
        self.assertEqual(rc, True)
        self.assertEqual(self.polyline.nodesSize(), 3)

        checker = QgsLayoutChecker(
            'composerpolyline_removednode', self.layout)
        checker.setControlPathPrefix("composer_polyline")
        myTestResult, myMessage = checker.testLayout()
        assert myTestResult, myMessage

    def testAddNode(self):
        """Test addNode method"""

        # default searching radius is 10
        self.assertEqual(self.polyline.nodesSize(), 4)
        rc = self.polyline.addNode(QPointF(50.0, 10.0))
        self.assertEqual(rc, False)

        # default searching radius is 10
        self.assertEqual(self.polyline.nodesSize(), 4)
        rc = self.polyline.addNode(QPointF(50.0, 9.99))
        self.assertEqual(rc, True)
        self.assertEqual(self.polyline.nodesSize(), 5)

    def testAddNodeCustomRadius(self):
        """Test addNode with custom radius"""

        # default searching radius is 10
        self.assertEqual(self.polyline.nodesSize(), 4)
        rc = self.polyline.addNode(QPointF(50.0, 8.1), True, 8.0)
        self.assertEqual(rc, False)
        self.assertEqual(self.polyline.nodesSize(), 4)

        # default searching radius is 10
        rc = self.polyline.addNode(QPointF(50.0, 7.9), True, 8.0)
        self.assertEqual(rc, True)
        self.assertEqual(self.polyline.nodesSize(), 5)

    def testAddNodeWithoutCheckingArea(self):
        """Test addNode without checking the maximum distance allowed"""

        # default searching radius is 10
        self.assertEqual(self.polyline.nodesSize(), 4)
        rc = self.polyline.addNode(QPointF(50.0, 20.0))
        self.assertEqual(rc, False)
        self.assertEqual(self.polyline.nodesSize(), 4)

        # default searching radius is 10
        self.assertEqual(self.polyline.nodesSize(), 4)
        rc = self.polyline.addNode(QPointF(50.0, 20.0), False)
        self.assertEqual(rc, True)
        self.assertEqual(self.polyline.nodesSize(), 5)

        checker = QgsLayoutChecker(
            'composerpolyline_addnode', self.layout)
        checker.setControlPathPrefix("composer_polyline")
        myTestResult, myMessage = checker.testLayout()
        assert myTestResult, myMessage

    def testMoveNode(self):
        """Test moveNode method"""

        rc = self.polyline.moveNode(30, QPointF(100.0, 300.0))
        self.assertEqual(rc, False)

        rc = self.polyline.moveNode(3, QPointF(100.0, 150.0))
        self.assertEqual(rc, True)

        checker = QgsLayoutChecker(
            'composerpolyline_movenode', self.layout)
        checker.setControlPathPrefix("composer_polyline")
        myTestResult, myMessage = checker.testLayout()
        assert myTestResult, myMessage

    def testNodeAtPosition(self):
        """Test nodeAtPosition method"""

        # default searching radius is 10
        rc = self.polyline.nodeAtPosition(QPointF(100.0, 210.0))
        self.assertEqual(rc, -1)

        # default searching radius is 10
        rc = self.polyline.nodeAtPosition(
            QPointF(100.0, 210.0), False)
        self.assertEqual(rc, 3)

        # default searching radius is 10
        rc = self.polyline.nodeAtPosition(
            QPointF(100.0, 210.0), True, 10.1)
        self.assertEqual(rc, 3)

    def testReadWriteXml(self):
        pr = QgsProject()
        l = QgsLayout(pr)

        p = QPolygonF()
        p.append(QPointF(0.0, 0.0))
        p.append(QPointF(100.0, 0.0))
        p.append(QPointF(200.0, 100.0))
        shape = QgsLayoutItemPolyline(p, l)

        props = {}
        props["color"] = "255,0,0,255"
        props["width"] = "10.0"
        props["capstyle"] = "square"

        style = QgsLineSymbol.createSimple(props)
        shape.setSymbol(style)

        #save original item to xml
        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")
        self.assertTrue(shape.writeXml(elem, doc, QgsReadWriteContext()))

        shape2 = QgsLayoutItemPolyline(l)
        self.assertTrue(shape2.readXml(elem.firstChildElement(), doc, QgsReadWriteContext()))

        self.assertEqual(shape2.nodes(), shape.nodes())
        self.assertEqual(shape2.symbol().symbolLayer(0).color().name(), '#ff0000')

    def testBounds(self):
        pr = QgsProject()
        l = QgsLayout(pr)

        p = QPolygonF()
        p.append(QPointF(50.0, 30.0))
        p.append(QPointF(100.0, 10.0))
        p.append(QPointF(200.0, 100.0))
        shape = QgsLayoutItemPolyline(p, l)

        props = {}
        props["color"] = "255,0,0,255"
        props["width"] = "6.0"
        props["capstyle"] = "square"

        style = QgsLineSymbol.createSimple(props)
        shape.setSymbol(style)

        # scene bounding rect should include symbol outline
        bounds = shape.sceneBoundingRect()
        self.assertEqual(bounds.left(), 47.0)
        self.assertEqual(bounds.right(), 203.0)
        self.assertEqual(bounds.top(), 7.0)
        self.assertEqual(bounds.bottom(), 103.0)

        # rectWithFrame should include symbol outline too
        bounds = shape.rectWithFrame()
        self.assertEqual(bounds.left(), -3.0)
        self.assertEqual(bounds.right(), 153.0)
        self.assertEqual(bounds.top(), -3.0)
        self.assertEqual(bounds.bottom(), 93.0)

    def testHorizontalLine(self):
        pr = QgsProject()
        l = QgsLayout(pr)
        l.initializeDefaults()

        p = QPolygonF()
        p.append(QPointF(50.0, 100.0))
        p.append(QPointF(100.0, 100.0))
        shape = QgsLayoutItemPolyline(p, l)
        l.addLayoutItem(shape)

        props = {}
        props["color"] = "0,0,0,255"
        props["width"] = "10.0"
        props["capstyle"] = "square"

        style = QgsLineSymbol.createSimple(props)
        shape.setSymbol(style)

        checker = QgsLayoutChecker(
            'composerpolyline_hozline', l)
        checker.setControlPathPrefix("composer_polyline")
        myTestResult, myMessage = checker.testLayout()
        assert myTestResult, myMessage

    def testVerticalLine(self):
        pr = QgsProject()
        l = QgsLayout(pr)
        l.initializeDefaults()

        p = QPolygonF()
        p.append(QPointF(100.0, 50.0))
        p.append(QPointF(100.0, 100.0))
        shape = QgsLayoutItemPolyline(p, l)
        l.addLayoutItem(shape)

        props = {}
        props["color"] = "0,0,0,255"
        props["width"] = "10.0"
        props["capstyle"] = "square"

        style = QgsLineSymbol.createSimple(props)
        shape.setSymbol(style)

        checker = QgsLayoutChecker(
            'composerpolyline_vertline', l)
        checker.setControlPathPrefix("composer_polyline")
        myTestResult, myMessage = checker.testLayout()
        assert myTestResult, myMessage


if __name__ == '__main__':
    unittest.main()
