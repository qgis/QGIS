"""QGIS Unit tests for QgsLayoutItemMarker.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "(C) 2020 by Nyall Dawson"
__date__ = "05/04/2020"
__copyright__ = "Copyright 2020, The QGIS Project"

from qgis.PyQt.QtCore import QRectF, Qt
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    QgsCoordinateReferenceSystem,
    QgsLayout,
    QgsLayoutItemMap,
    QgsLayoutItemMarker,
    QgsLayoutItemRegistry,
    QgsLayoutNorthArrowHandler,
    QgsLayoutPoint,
    QgsMarkerSymbol,
    QgsProject,
    QgsReadWriteContext,
    QgsRectangle,
    QgsUnitTypes,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from test_qgslayoutitem import LayoutItemTestCase
from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsLayoutMarker(QgisTestCase, LayoutItemTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "layout_marker"

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls.item_class = QgsLayoutItemMarker

    def __init__(self, methodName):
        """Run once on class initialization."""
        QgisTestCase.__init__(self, methodName)

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
        marker.setId("id")
        self.assertEqual(marker.displayName(), "id")

    def testType(self):
        """Test if type is valid"""
        layout = QgsLayout(QgsProject.instance())
        marker = QgsLayoutItemMarker(layout)

        self.assertEqual(marker.type(), QgsLayoutItemRegistry.ItemType.LayoutMarker)

    def testRender(self):
        """Test marker rendering."""
        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()
        marker = QgsLayoutItemMarker(layout)
        marker.attemptMove(
            QgsLayoutPoint(100, 50, QgsUnitTypes.LayoutUnit.LayoutMillimeters)
        )
        props = {}
        props["color"] = "0,255,255"
        props["outline_width"] = "4"
        props["outline_color"] = "0,0,0"
        props["size"] = "14.4"

        style = QgsMarkerSymbol.createSimple(props)
        marker.setSymbol(style)
        layout.addLayoutItem(marker)

        self.assertTrue(self.render_layout_check("layout_marker_render", layout))

    def testReadWriteXml(self):
        pr = QgsProject()
        l = QgsLayout(pr)
        marker = QgsLayoutItemMarker(l)
        l.addLayoutItem(marker)

        map = QgsLayoutItemMap(l)
        l.addLayoutItem(map)

        props = {}
        props["color"] = "green"
        props["outline_style"] = "no"
        props["size"] = "4.4"

        style = QgsMarkerSymbol.createSimple(props)
        marker.setSymbol(style)

        marker.setLinkedMap(map)
        marker.setNorthMode(QgsLayoutNorthArrowHandler.NorthMode.TrueNorth)
        marker.setNorthOffset(15)

        # save original item to xml
        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")
        self.assertTrue(marker.writeXml(elem, doc, QgsReadWriteContext()))

        marker2 = QgsLayoutItemMarker(l)
        self.assertTrue(
            marker2.readXml(elem.firstChildElement(), doc, QgsReadWriteContext())
        )
        marker2.finalizeRestoreFromXml()

        self.assertEqual(marker2.symbol().symbolLayer(0).color().name(), "#008000")
        self.assertEqual(
            marker2.symbol().symbolLayer(0).strokeStyle(), Qt.PenStyle.NoPen
        )
        self.assertEqual(marker2.symbol().symbolLayer(0).size(), 4.4)

        self.assertEqual(marker2.linkedMap(), map)
        self.assertEqual(
            marker2.northMode(), QgsLayoutNorthArrowHandler.NorthMode.TrueNorth
        )
        self.assertEqual(marker2.northOffset(), 15.0)

    def testBounds(self):
        pr = QgsProject()
        l = QgsLayout(pr)

        shape = QgsLayoutItemMarker(l)
        shape.attemptMove(
            QgsLayoutPoint(10, 20, QgsUnitTypes.LayoutUnit.LayoutMillimeters)
        )
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

    def testNorthArrowWithMapItemRotation(self):
        """Test picture rotation when map item is also rotated"""

        layout = QgsLayout(QgsProject.instance())

        map = QgsLayoutItemMap(layout)
        map.setExtent(QgsRectangle(0, -256, 256, 0))
        layout.addLayoutItem(map)

        marker = QgsLayoutItemMarker(layout)
        layout.addLayoutItem(marker)

        marker.setLinkedMap(map)
        self.assertEqual(marker.linkedMap(), map)

        marker.setNorthMode(QgsLayoutNorthArrowHandler.NorthMode.GridNorth)
        map.setItemRotation(45)
        self.assertEqual(marker.northArrowRotation(), 45)
        map.setMapRotation(-34)
        self.assertEqual(marker.northArrowRotation(), 11)

        # add an offset
        marker.setNorthOffset(-10)
        self.assertEqual(marker.northArrowRotation(), 1)

        map.setItemRotation(55)
        self.assertEqual(marker.northArrowRotation(), 11)

    def testGridNorth(self):
        """Test syncing picture to grid north"""

        layout = QgsLayout(QgsProject.instance())

        map = QgsLayoutItemMap(layout)
        map.setExtent(QgsRectangle(0, -256, 256, 0))
        layout.addLayoutItem(map)

        marker = QgsLayoutItemMarker(layout)
        layout.addLayoutItem(marker)

        marker.setLinkedMap(map)
        self.assertEqual(marker.linkedMap(), map)

        marker.setNorthMode(QgsLayoutNorthArrowHandler.NorthMode.GridNorth)
        map.setMapRotation(45)
        self.assertEqual(marker.northArrowRotation(), 45)

        # add an offset
        marker.setNorthOffset(-10)
        self.assertEqual(marker.northArrowRotation(), 35)

    def testTrueNorth(self):
        """Test syncing picture to true north"""

        layout = QgsLayout(QgsProject.instance())

        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(0, 0, 10, 10))
        map.setCrs(QgsCoordinateReferenceSystem.fromEpsgId(3575))
        map.setExtent(
            QgsRectangle(-2126029.962, -2200807.749, -119078.102, -757031.156)
        )
        layout.addLayoutItem(map)

        marker = QgsLayoutItemMarker(layout)
        layout.addLayoutItem(marker)

        marker.setLinkedMap(map)
        self.assertEqual(marker.linkedMap(), map)

        marker.setNorthMode(QgsLayoutNorthArrowHandler.NorthMode.TrueNorth)
        self.assertAlmostEqual(marker.northArrowRotation(), 37.20, 1)

        # shift map
        map.setExtent(
            QgsRectangle(2120672.293, -3056394.691, 2481640.226, -2796718.780)
        )
        self.assertAlmostEqual(marker.northArrowRotation(), -38.18, 1)

        # rotate map
        map.setMapRotation(45)
        self.assertAlmostEqual(marker.northArrowRotation(), -38.18 + 45, 1)

        # add an offset
        marker.setNorthOffset(-10)
        self.assertAlmostEqual(marker.northArrowRotation(), -38.18 + 35, 1)

    def testRenderWithNorthRotation(self):
        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()

        map = QgsLayoutItemMap(layout)
        map.setExtent(QgsRectangle(0, -256, 256, 0))

        marker = QgsLayoutItemMarker(layout)
        marker.attemptMove(
            QgsLayoutPoint(100, 50, QgsUnitTypes.LayoutUnit.LayoutMillimeters)
        )
        props = {}
        props["color"] = "0,255,255"
        props["outline_style"] = "no"
        props["size"] = "14.4"
        props["name"] = "arrow"
        props["angle"] = "10"

        marker.setLinkedMap(map)
        self.assertEqual(marker.linkedMap(), map)

        marker.setNorthMode(QgsLayoutNorthArrowHandler.NorthMode.GridNorth)
        map.setMapRotation(35)
        self.assertEqual(marker.northArrowRotation(), 35)

        # when rendering, north arrow rotation must be ADDED to symbol rotation! ie.
        # it does not replace it

        style = QgsMarkerSymbol.createSimple(props)
        marker.setSymbol(style)
        layout.addLayoutItem(marker)

        self.assertTrue(self.render_layout_check("layout_marker_render_north", layout))


if __name__ == "__main__":
    unittest.main()
