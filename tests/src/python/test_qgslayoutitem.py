"""QGIS Unit tests for QgsLayoutItem.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "(C) 2017 by Nyall Dawson"
__date__ = "17/01/2017"
__copyright__ = "Copyright 2017, The QGIS Project"

import os

from qgis.PyQt.QtCore import Qt, QRectF
from qgis.PyQt.QtGui import QColor, QPainter, QImage
from qgis.PyQt.QtTest import QSignalSpy
from qgis.core import (
    QgsLayout,
    QgsLayoutItem,
    QgsLayoutItemLabel,
    QgsLayoutItemMap,
    QgsLayoutItemShape,
    QgsLayoutMeasurement,
    QgsLayoutObject,
    QgsLayoutPoint,
    QgsLayoutSize,
    QgsProject,
    QgsProperty,
    QgsUnitTypes,
    QgsFillSymbol,
    QgsSimpleFillSymbolLayer,
    QgsLayoutRenderContext,
    QgsLayoutItemElevationProfile,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

TEST_DATA_DIR = unitTestDataPath()

start_app()


class LayoutItemTestCase:
    """
    This is a collection of generic tests for QgsLayoutItem subclasses.
    To make use of it, subclass it and set self.item_class to a QgsLayoutItem subclass you want to test.
    """

    def make_item(self, layout):
        if hasattr(self, "item_class"):
            return self.item_class(layout)
        else:
            return self.createItem(layout)

    def testRequiresRasterization(self):
        l = QgsLayout(QgsProject.instance())
        item = self.make_item(l)
        self.assertFalse(item.requiresRasterization())
        item.setBlendMode(QPainter.CompositionMode.CompositionMode_SourceIn)
        self.assertTrue(item.requiresRasterization())


class TestQgsLayoutItem(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "composer_effects"

    def testDataDefinedFrameColor(self):
        layout = QgsLayout(QgsProject.instance())

        item = QgsLayoutItemMap(layout)
        item.setFrameEnabled(True)

        item.setFrameStrokeColor(QColor(255, 0, 0))
        self.assertEqual(item.frameStrokeColor(), QColor(255, 0, 0))
        self.assertEqual(item.pen().color().name(), QColor(255, 0, 0).name())

        item.dataDefinedProperties().setProperty(
            QgsLayoutObject.DataDefinedProperty.FrameColor,
            QgsProperty.fromExpression("'blue'"),
        )
        item.refreshDataDefinedProperty()
        self.assertEqual(
            item.frameStrokeColor(), QColor(255, 0, 0)
        )  # should not change
        self.assertEqual(item.pen().color().name(), QColor(0, 0, 255).name())

    def testFrameWidth(self):
        layout = QgsLayout(QgsProject.instance())

        item = QgsLayoutItemMap(layout)
        item.setFrameEnabled(True)

        item.setFrameStrokeWidth(
            QgsLayoutMeasurement(10, QgsUnitTypes.LayoutUnit.LayoutMillimeters)
        )
        self.assertEqual(
            item.frameStrokeWidth(),
            QgsLayoutMeasurement(10, QgsUnitTypes.LayoutUnit.LayoutMillimeters),
        )
        self.assertEqual(item.pen().width(), 10.0)

        item.setFrameStrokeWidth(
            QgsLayoutMeasurement(10, QgsUnitTypes.LayoutUnit.LayoutCentimeters)
        )
        self.assertEqual(
            item.frameStrokeWidth(),
            QgsLayoutMeasurement(10, QgsUnitTypes.LayoutUnit.LayoutCentimeters),
        )
        self.assertEqual(item.pen().width(), 100.0)

    def testDataDefinedBackgroundColor(self):
        layout = QgsLayout(QgsProject.instance())

        item = QgsLayoutItemMap(layout)

        item.setBackgroundColor(QColor(255, 0, 0))
        self.assertEqual(item.backgroundColor(), QColor(255, 0, 0))
        self.assertEqual(item.brush().color().name(), QColor(255, 0, 0).name())

        item.dataDefinedProperties().setProperty(
            QgsLayoutObject.DataDefinedProperty.BackgroundColor,
            QgsProperty.fromExpression("'blue'"),
        )
        item.refreshDataDefinedProperty()
        self.assertEqual(
            item.backgroundColor(False), QColor(255, 0, 0)
        )  # should not change
        self.assertEqual(item.backgroundColor(True).name(), item.brush().color().name())
        self.assertEqual(item.brush().color().name(), QColor(0, 0, 255).name())

    def testSelected(self):
        """
        Ensure that items are selectable
        """
        layout = QgsLayout(QgsProject.instance())
        item = QgsLayoutItemMap(layout)
        item.setSelected(True)
        self.assertTrue(item.isSelected())
        item.setSelected(False)
        self.assertFalse(item.isSelected())

    def testLocked(self):
        layout = QgsLayout(QgsProject.instance())
        item = QgsLayoutItemMap(layout)

        lock_changed_spy = QSignalSpy(item.lockChanged)
        item.setLocked(True)
        self.assertTrue(item.isLocked())
        self.assertEqual(len(lock_changed_spy), 1)
        item.setLocked(True)
        self.assertEqual(len(lock_changed_spy), 1)

        item.setLocked(False)
        self.assertFalse(item.isLocked())
        self.assertEqual(len(lock_changed_spy), 2)
        item.setLocked(False)
        self.assertEqual(len(lock_changed_spy), 2)

    def testFrameBleed(self):
        layout = QgsLayout(QgsProject.instance())
        item = QgsLayoutItemMap(layout)
        item.setFrameEnabled(False)
        self.assertEqual(item.estimatedFrameBleed(), 0)

        item.setFrameStrokeWidth(
            QgsLayoutMeasurement(10, QgsUnitTypes.LayoutUnit.LayoutMillimeters)
        )
        item.setFrameEnabled(False)
        self.assertEqual(item.estimatedFrameBleed(), 0)
        item.setFrameEnabled(True)
        self.assertEqual(item.estimatedFrameBleed(), 5)  # only half bleeds out!

        item.setFrameStrokeWidth(
            QgsLayoutMeasurement(10, QgsUnitTypes.LayoutUnit.LayoutCentimeters)
        )
        self.assertEqual(item.estimatedFrameBleed(), 50)  # only half bleeds out!

    def testRectWithFrame(self):
        layout = QgsLayout(QgsProject.instance())
        item = QgsLayoutItemMap(layout)
        item.attemptMove(
            QgsLayoutPoint(6, 10, QgsUnitTypes.LayoutUnit.LayoutMillimeters)
        )
        item.attemptResize(
            QgsLayoutSize(18, 12, QgsUnitTypes.LayoutUnit.LayoutMillimeters)
        )

        item.setFrameEnabled(False)
        self.assertEqual(item.rectWithFrame(), QRectF(0, 0, 18, 12))
        item.setFrameStrokeWidth(
            QgsLayoutMeasurement(10, QgsUnitTypes.LayoutUnit.LayoutMillimeters)
        )
        item.setFrameEnabled(False)
        self.assertEqual(item.rectWithFrame(), QRectF(0, 0, 18, 12))
        item.setFrameEnabled(True)
        self.assertEqual(item.rectWithFrame(), QRectF(-5.0, -5.0, 28.0, 22.0))
        item.setFrameStrokeWidth(
            QgsLayoutMeasurement(10, QgsUnitTypes.LayoutUnit.LayoutCentimeters)
        )
        self.assertEqual(item.rectWithFrame(), QRectF(-50.0, -50.0, 118.0, 112.0))

    def testDisplayName(self):
        layout = QgsLayout(QgsProject.instance())
        item = QgsLayoutItemShape(layout)
        self.assertEqual(item.displayName(), "<Rectangle>")
        item.setId("a")
        self.assertEqual(item.displayName(), "a")
        self.assertEqual(item.id(), "a")

    def testCasting(self):
        """
        Test that sip correctly casts stuff
        """
        p = QgsProject()
        p.read(os.path.join(TEST_DATA_DIR, "layouts", "layout_casting.qgs"))

        layout = p.layoutManager().layouts()[0]

        # check a method which often fails casting
        map = layout.itemById("map")
        self.assertIsInstance(map, QgsLayoutItemMap)
        label = layout.itemById("label")
        self.assertIsInstance(label, QgsLayoutItemLabel)

        # another method -- sometimes this fails casting for different(?) reasons
        # make sure we start from a new project so sip hasn't remembered item instances
        p2 = QgsProject()
        p2.read(os.path.join(TEST_DATA_DIR, "layouts", "layout_casting.qgs"))
        layout = p2.layoutManager().layouts()[0]

        items = layout.items()
        map2 = [i for i in items if isinstance(i, QgsLayoutItem) and i.id() == "map"][0]
        self.assertIsInstance(map2, QgsLayoutItemMap)
        label2 = [
            i for i in items if isinstance(i, QgsLayoutItem) and i.id() == "label"
        ][0]
        self.assertIsInstance(label2, QgsLayoutItemLabel)

    def testContainsAdvancedEffectsAndRasterization(self):
        layout = QgsLayout(QgsProject.instance())
        item = QgsLayoutItemLabel(layout)

        self.assertFalse(item.containsAdvancedEffects())

        # item opacity requires that the individual item be flattened to a raster item
        item.setItemOpacity(0.5)
        self.assertTrue(item.containsAdvancedEffects())
        # but not the WHOLE layout
        self.assertFalse(item.requiresRasterization())
        item.dataDefinedProperties().setProperty(
            QgsLayoutObject.DataDefinedProperty.Opacity,
            QgsProperty.fromExpression("100"),
        )
        item.refresh()
        self.assertFalse(item.containsAdvancedEffects())
        self.assertFalse(item.requiresRasterization())
        item.dataDefinedProperties().setProperty(
            QgsLayoutObject.DataDefinedProperty.Opacity, QgsProperty()
        )
        item.refresh()
        self.assertTrue(item.containsAdvancedEffects())
        self.assertFalse(item.requiresRasterization())
        item.setItemOpacity(1.0)
        self.assertFalse(item.containsAdvancedEffects())
        self.assertFalse(item.requiresRasterization())

        # item blend mode is NOT an advanced effect -- rather it requires that the WHOLE layout be rasterized to achieve
        item.setBlendMode(QPainter.CompositionMode.CompositionMode_DestinationAtop)
        self.assertFalse(item.containsAdvancedEffects())
        self.assertTrue(item.requiresRasterization())

    def test_blend_mode_rendering_designer_preview(self):
        """
        Test rendering of blend modes while in designer dialogs
        """
        p = QgsProject()
        l = QgsLayout(p)
        self.assertTrue(l.renderContext().isPreviewRender())

        l.initializeDefaults()
        item1 = QgsLayoutItemShape(l)
        item1.attemptSetSceneRect(QRectF(20, 20, 150, 100))
        item1.setShapeType(QgsLayoutItemShape.Shape.Rectangle)
        simple_fill = QgsSimpleFillSymbolLayer()
        fill_symbol = QgsFillSymbol()
        fill_symbol.changeSymbolLayer(0, simple_fill)
        simple_fill.setColor(QColor(255, 150, 0))
        simple_fill.setStrokeColor(Qt.GlobalColor.black)
        item1.setSymbol(fill_symbol)

        l.addLayoutItem(item1)
        item2 = QgsLayoutItemShape(l)
        item2.attemptSetSceneRect(QRectF(50, 50, 150, 100))
        item2.setShapeType(QgsLayoutItemShape.Shape.Rectangle)
        l.addLayoutItem(item2)
        simple_fill = QgsSimpleFillSymbolLayer()
        fill_symbol = QgsFillSymbol()
        fill_symbol.changeSymbolLayer(0, simple_fill)
        simple_fill.setColor(QColor(0, 100, 150))
        simple_fill.setStrokeColor(Qt.GlobalColor.black)
        item2.setSymbol(fill_symbol)

        item2.setBlendMode(QPainter.CompositionMode.CompositionMode_Multiply)

        page_item = l.pageCollection().page(0)
        paper_rect = QRectF(
            page_item.pos().x(),
            page_item.pos().y(),
            page_item.rect().width(),
            page_item.rect().height(),
        )

        im = QImage(1122, 794, QImage.Format.Format_ARGB32)
        im.fill(Qt.GlobalColor.transparent)
        im.setDotsPerMeterX(int(300 / 25.4 * 1000))
        im.setDotsPerMeterY(int(300 / 25.4 * 1000))
        painter = QPainter(im)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)

        l.render(
            painter,
            QRectF(0, 0, painter.device().width(), painter.device().height()),
            paper_rect,
        )
        painter.end()

        self.assertTrue(
            self.image_check(
                "blend_modes_preview_mode",
                "composereffects_blend",
                im,
                allowed_mismatch=0,
            )
        )

    def test_blend_mode_rendering_export(self):
        """
        Test rendering of blend modes when exporting layouts
        """
        p = QgsProject()
        l = QgsLayout(p)

        l.initializeDefaults()
        item1 = QgsLayoutItemShape(l)
        item1.attemptSetSceneRect(QRectF(20, 20, 150, 100))
        item1.setShapeType(QgsLayoutItemShape.Shape.Rectangle)
        simple_fill = QgsSimpleFillSymbolLayer()
        fill_symbol = QgsFillSymbol()
        fill_symbol.changeSymbolLayer(0, simple_fill)
        simple_fill.setColor(QColor(255, 150, 0))
        simple_fill.setStrokeColor(Qt.GlobalColor.black)
        item1.setSymbol(fill_symbol)

        l.addLayoutItem(item1)
        item2 = QgsLayoutItemShape(l)
        item2.attemptSetSceneRect(QRectF(50, 50, 150, 100))
        item2.setShapeType(QgsLayoutItemShape.Shape.Rectangle)
        l.addLayoutItem(item2)
        simple_fill = QgsSimpleFillSymbolLayer()
        fill_symbol = QgsFillSymbol()
        fill_symbol.changeSymbolLayer(0, simple_fill)
        simple_fill.setColor(QColor(0, 100, 150))
        simple_fill.setStrokeColor(Qt.GlobalColor.black)
        item2.setSymbol(fill_symbol)

        item2.setBlendMode(QPainter.CompositionMode.CompositionMode_Multiply)

        self.assertTrue(self.render_layout_check("composereffects_blend", l))

    def test_blend_mode_rendering_export_no_advanced_effects(self):
        """
        Test rendering of blend modes when exporting layouts when advanced
        effects are disabled.

        Blend mode will be completely disabled in this scenario
        """
        p = QgsProject()
        l = QgsLayout(p)
        l.renderContext().setFlag(
            QgsLayoutRenderContext.Flag.FlagUseAdvancedEffects, False
        )

        l.initializeDefaults()
        item1 = QgsLayoutItemShape(l)
        item1.attemptSetSceneRect(QRectF(20, 20, 150, 100))
        item1.setShapeType(QgsLayoutItemShape.Shape.Rectangle)
        simple_fill = QgsSimpleFillSymbolLayer()
        fill_symbol = QgsFillSymbol()
        fill_symbol.changeSymbolLayer(0, simple_fill)
        simple_fill.setColor(QColor(255, 150, 0))
        simple_fill.setStrokeColor(Qt.GlobalColor.black)
        item1.setSymbol(fill_symbol)

        l.addLayoutItem(item1)
        item2 = QgsLayoutItemShape(l)
        item2.attemptSetSceneRect(QRectF(50, 50, 150, 100))
        item2.setShapeType(QgsLayoutItemShape.Shape.Rectangle)
        l.addLayoutItem(item2)
        simple_fill = QgsSimpleFillSymbolLayer()
        fill_symbol = QgsFillSymbol()
        fill_symbol.changeSymbolLayer(0, simple_fill)
        simple_fill.setColor(QColor(0, 100, 150))
        simple_fill.setStrokeColor(Qt.GlobalColor.black)
        item2.setSymbol(fill_symbol)

        item2.setBlendMode(QPainter.CompositionMode.CompositionMode_Multiply)

        self.assertTrue(
            self.render_layout_check("composereffects_blend_no_advanced_effects", l)
        )

    def test_blend_mode_rendering_export_force_vector(self):
        """
        Test rendering of blend modes when exporting layouts when force
        vector flag is on.

        Blend mode will be completely disabled in this scenario
        """
        p = QgsProject()
        l = QgsLayout(p)
        l.renderContext().setFlag(
            QgsLayoutRenderContext.Flag.FlagForceVectorOutput, True
        )

        l.initializeDefaults()
        item1 = QgsLayoutItemShape(l)
        item1.attemptSetSceneRect(QRectF(20, 20, 150, 100))
        item1.setShapeType(QgsLayoutItemShape.Shape.Rectangle)
        simple_fill = QgsSimpleFillSymbolLayer()
        fill_symbol = QgsFillSymbol()
        fill_symbol.changeSymbolLayer(0, simple_fill)
        simple_fill.setColor(QColor(255, 150, 0))
        simple_fill.setStrokeColor(Qt.GlobalColor.black)
        item1.setSymbol(fill_symbol)

        l.addLayoutItem(item1)
        item2 = QgsLayoutItemShape(l)
        item2.attemptSetSceneRect(QRectF(50, 50, 150, 100))
        item2.setShapeType(QgsLayoutItemShape.Shape.Rectangle)
        l.addLayoutItem(item2)
        simple_fill = QgsSimpleFillSymbolLayer()
        fill_symbol = QgsFillSymbol()
        fill_symbol.changeSymbolLayer(0, simple_fill)
        simple_fill.setColor(QColor(0, 100, 150))
        simple_fill.setStrokeColor(Qt.GlobalColor.black)
        item2.setSymbol(fill_symbol)

        item2.setBlendMode(QPainter.CompositionMode.CompositionMode_Multiply)

        self.assertTrue(
            self.render_layout_check("composereffects_blend_no_advanced_effects", l)
        )


if __name__ == "__main__":
    unittest.main()
