"""QGIS Unit tests for QgsLayoutItemPolygon.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "(C) 2016 by Paul Blottiere"
__date__ = "14/03/2016"
__copyright__ = "Copyright 2016, The QGIS Project"

from qgis.PyQt.QtCore import QPointF, QRectF
from qgis.PyQt.QtGui import QImage, QPainter, QPolygonF
from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    QgsFillSymbol,
    QgsLayout,
    QgsLayoutItem,
    QgsLayoutItemPolygon,
    QgsLayoutItemRegistry,
    QgsLayoutItemRenderContext,
    QgsLayoutUtils,
    QgsProject,
    QgsReadWriteContext,
    QgsLayoutItemMap,
    QgsRectangle,
    Qgis,
    QgsGeometryGeneratorSymbolLayer,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from test_qgslayoutitem import LayoutItemTestCase
from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsLayoutPolygon(QgisTestCase, LayoutItemTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "composer_polygon"

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls.item_class = QgsLayoutItemPolygon

    def __init__(self, methodName):
        """Run once on class initialization."""
        QgisTestCase.__init__(self, methodName)

        # create composition
        self.layout = QgsLayout(QgsProject.instance())
        self.layout.initializeDefaults()

        # create
        polygon = QPolygonF()
        polygon.append(QPointF(0.0, 0.0))
        polygon.append(QPointF(100.0, 0.0))
        polygon.append(QPointF(200.0, 100.0))
        polygon.append(QPointF(100.0, 200.0))

        self.polygon = QgsLayoutItemPolygon(polygon, self.layout)
        self.layout.addLayoutItem(self.polygon)

        # style
        props = {}
        props["color"] = "green"
        props["style"] = "solid"
        props["style_border"] = "solid"
        props["color_border"] = "black"
        props["width_border"] = "10.0"
        props["joinstyle"] = "miter"

        style = QgsFillSymbol.createSimple(props)
        self.polygon.setSymbol(style)

    def testNodes(self):
        polygon = QPolygonF()
        polygon.append(QPointF(0.0, 0.0))
        polygon.append(QPointF(100.0, 0.0))
        polygon.append(QPointF(200.0, 100.0))
        polygon.append(QPointF(100.0, 200.0))

        p = QgsLayoutItemPolygon(polygon, self.layout)
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

        self.assertEqual(self.polygon.displayName(), "<Polygon>")

    def testType(self):
        """Test if type is valid"""

        self.assertEqual(
            self.polygon.type(), QgsLayoutItemRegistry.ItemType.LayoutPolygon
        )

    def testDefaultStyle(self):
        """Test polygon rendering with default style."""

        self.polygon.setDisplayNodes(False)
        self.assertTrue(
            self.render_layout_check("composerpolygon_defaultstyle", self.layout)
        )

    def testDisplayNodes(self):
        """Test displayNodes method"""

        self.polygon.setDisplayNodes(True)
        self.assertTrue(
            self.render_layout_check("composerpolygon_displaynodes", self.layout)
        )

        self.polygon.setDisplayNodes(False)
        self.assertTrue(
            self.render_layout_check("composerpolygon_defaultstyle", self.layout)
        )

    def testSelectedNode(self):
        """Test selectedNode and deselectNode methods"""

        self.polygon.setDisplayNodes(True)

        self.polygon.setSelectedNode(3)
        self.assertTrue(
            self.render_layout_check("composerpolygon_selectednode", self.layout)
        )

        self.polygon.deselectNode()
        self.polygon.setDisplayNodes(False)
        self.assertTrue(
            self.render_layout_check("composerpolygon_defaultstyle", self.layout)
        )

    def testRemoveNode(self):
        """Test removeNode method"""

        rc = self.polygon.removeNode(100)
        self.assertEqual(rc, False)

        self.assertTrue(
            self.render_layout_check("composerpolygon_defaultstyle", self.layout)
        )

        self.assertEqual(self.polygon.nodesSize(), 4)

    def testAddNode(self):
        """Test addNode method"""

        # default searching radius is 10
        self.assertEqual(self.polygon.nodesSize(), 4)
        rc = self.polygon.addNode(QPointF(50.0, 10.0))
        self.assertEqual(rc, False)

        # default searching radius is 10
        self.assertEqual(self.polygon.nodesSize(), 4)
        rc = self.polygon.addNode(QPointF(50.0, 9.99))
        self.assertEqual(rc, True)
        self.assertEqual(self.polygon.nodesSize(), 5)

    def testAddNodeCustomRadius(self):
        """Test addNode with custom radius"""

        # default searching radius is 10
        self.assertEqual(self.polygon.nodesSize(), 4)
        rc = self.polygon.addNode(QPointF(50.0, 8.1), True, 8.0)
        self.assertEqual(rc, False)
        self.assertEqual(self.polygon.nodesSize(), 4)

        # default searching radius is 10
        rc = self.polygon.addNode(QPointF(50.0, 7.9), True, 8.0)
        self.assertEqual(rc, True)
        self.assertEqual(self.polygon.nodesSize(), 5)

    def testAddNodeWithoutCheckingArea(self):
        """Test addNode without checking the maximum distance allowed"""

        # default searching radius is 10
        self.assertEqual(self.polygon.nodesSize(), 4)
        rc = self.polygon.addNode(QPointF(50.0, 20.0))
        self.assertEqual(rc, False)
        self.assertEqual(self.polygon.nodesSize(), 4)

        # default searching radius is 10
        self.assertEqual(self.polygon.nodesSize(), 4)
        rc = self.polygon.addNode(QPointF(50.0, 20.0), False)
        self.assertEqual(rc, True)
        self.assertEqual(self.polygon.nodesSize(), 5)

        self.assertTrue(
            self.render_layout_check("composerpolygon_addnode", self.layout)
        )

    def testMoveNode(self):
        """Test moveNode method"""

        rc = self.polygon.moveNode(30, QPointF(100.0, 300.0))
        self.assertEqual(rc, False)

        rc = self.polygon.moveNode(3, QPointF(100.0, 150.0))
        self.assertEqual(rc, True)

        self.assertTrue(
            self.render_layout_check("composerpolygon_movenode", self.layout)
        )

    def testNodeAtPosition(self):
        """Test nodeAtPosition method"""

        p = QPolygonF()
        p.append(QPointF(0.0, 0.0))
        p.append(QPointF(100.0, 0.0))
        p.append(QPointF(200.0, 100.0))
        p.append(QPointF(100.0, 200.0))

        polygon = QgsLayoutItemPolygon(p, self.layout)

        # default searching radius is 10
        rc = polygon.nodeAtPosition(QPointF(100.0, 210.0))
        self.assertEqual(rc, -1)

        # default searching radius is 10
        rc = polygon.nodeAtPosition(QPointF(100.0, 210.0), False)
        self.assertEqual(rc, 3)

        # default searching radius is 10
        rc = polygon.nodeAtPosition(QPointF(100.0, 210.0), True, 10.1)
        self.assertEqual(rc, 3)

    def testReadWriteXml(self):
        pr = QgsProject()
        l = QgsLayout(pr)

        p = QPolygonF()
        p.append(QPointF(0.0, 0.0))
        p.append(QPointF(100.0, 0.0))
        p.append(QPointF(200.0, 100.0))
        shape = QgsLayoutItemPolygon(p, l)

        props = {}
        props["color"] = "green"
        props["style"] = "solid"
        props["style_border"] = "solid"
        props["color_border"] = "red"
        props["width_border"] = "10.0"
        props["joinstyle"] = "miter"

        style = QgsFillSymbol.createSimple(props)
        shape.setSymbol(style)

        # save original item to xml
        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")
        self.assertTrue(shape.writeXml(elem, doc, QgsReadWriteContext()))

        shape2 = QgsLayoutItemPolygon(l)
        self.assertTrue(
            shape2.readXml(elem.firstChildElement(), doc, QgsReadWriteContext())
        )

        self.assertEqual(shape2.nodes(), shape.nodes())
        self.assertEqual(shape2.symbol().symbolLayer(0).color().name(), "#008000")
        self.assertEqual(shape2.symbol().symbolLayer(0).strokeColor().name(), "#ff0000")

    def testBounds(self):
        pr = QgsProject()
        l = QgsLayout(pr)

        p = QPolygonF()
        p.append(QPointF(50.0, 30.0))
        p.append(QPointF(100.0, 10.0))
        p.append(QPointF(200.0, 100.0))
        shape = QgsLayoutItemPolygon(p, l)

        props = {}
        props["color"] = "green"
        props["style"] = "solid"
        props["style_border"] = "solid"
        props["color_border"] = "red"
        props["width_border"] = "6.0"
        props["joinstyle"] = "miter"

        style = QgsFillSymbol.createSimple(props)
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

    def testClipPath(self):
        pr = QgsProject()
        l = QgsLayout(pr)

        p = QPolygonF()
        p.append(QPointF(50.0, 30.0))
        p.append(QPointF(100.0, 10.0))
        p.append(QPointF(200.0, 100.0))
        shape = QgsLayoutItemPolygon(p, l)

        # must be a closed polygon, in scene coordinates!
        self.assertEqual(
            shape.clipPath().asWkt(), "Polygon ((50 30, 100 10, 200 100, 50 30))"
        )
        self.assertTrue(
            int(shape.itemFlags() & QgsLayoutItem.Flag.FlagProvidesClipPath)
        )

        spy = QSignalSpy(shape.clipPathChanged)
        self.assertTrue(shape.addNode(QPointF(150, 110), False))
        self.assertEqual(
            shape.clipPath().asWkt(),
            "Polygon ((50 30, 100 10, 200 100, 150 110, 50 30))",
        )
        self.assertEqual(len(spy), 1)

        shape.removeNode(3)
        self.assertEqual(len(spy), 2)
        self.assertEqual(
            shape.clipPath().asWkt(), "Polygon ((50 30, 100 10, 200 100, 50 30))"
        )

        shape.moveNode(2, QPointF(180, 100))
        self.assertEqual(len(spy), 3)
        self.assertEqual(
            shape.clipPath().asWkt(), "Polygon ((50 30, 100 10, 180 100, 50 30))"
        )

        shape.setNodes(p)
        self.assertEqual(len(spy), 4)
        self.assertEqual(
            shape.clipPath().asWkt(), "Polygon ((100 40, 150 20, 250 110, 100 40))"
        )

        shape.attemptSetSceneRect(QRectF(30, 10, 100, 200))
        self.assertEqual(
            shape.clipPath().asWkt(), "Polygon ((30 30, 80 10, 180 100, 30 30))"
        )
        # bit gross - this needs fixing in the item. It shouldn't rely on a draw operation to update the
        # path as a result of a move/resize
        im = QImage()
        p = QPainter(im)
        rc = QgsLayoutUtils.createRenderContextForLayout(l, p)
        shape.draw(QgsLayoutItemRenderContext(rc))
        p.end()
        self.assertEqual(len(spy), 5)

    def test_generator(self):
        project = QgsProject()
        layout = QgsLayout(project)
        layout.initializeDefaults()

        p = QPolygonF()
        p.append(QPointF(0.0, 0.0))
        p.append(QPointF(100.0, 10.0))
        p.append(QPointF(200.0, 100.0))
        shape = QgsLayoutItemPolygon(p, layout)
        layout.addLayoutItem(shape)

        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(0, 0, 10, 10))
        map.zoomToExtent(QgsRectangle(1, 1, 2, 2))
        layout.addLayoutItem(map)

        props = {}
        props["color"] = "green"
        props["style"] = "solid"
        props["style_border"] = "solid"
        props["color_border"] = "red"
        props["width_border"] = "6.0"
        props["joinstyle"] = "miter"

        sub_symbol = QgsFillSymbol.createSimple(props)

        line_symbol = QgsFillSymbol()
        generator = QgsGeometryGeneratorSymbolLayer.create(
            {
                "geometryModifier": "geom_from_wkt('POLYGON((10 10,287 10,287 200,10 200,10 10))')",
                "SymbolType": "Fill",
            }
        )
        generator.setUnits(Qgis.RenderUnit.Millimeters)
        generator.setSubSymbol(sub_symbol)

        line_symbol.changeSymbolLayer(0, generator)
        shape.setSymbol(line_symbol)

        self.assertTrue(self.render_layout_check("polygon_generator", layout))


if __name__ == "__main__":
    unittest.main()
