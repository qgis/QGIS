"""QGIS Unit tests for QgsStyleModel.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "10/09/2018"
__copyright__ = "Copyright 2018, The QGIS Project"

from qgis.PyQt.QtCore import QModelIndex, QSize, Qt
import unittest
from qgis.testing import start_app, QgisTestCase
from qgis.PyQt.QtGui import QColor
from qgis.core import (
    QgsAbstract3DSymbol,
    QgsFillSymbol,
    QgsGeometry,
    QgsLegendPatchShape,
    QgsLimitedRandomColorRamp,
    QgsLinePatternFillSymbolLayer,
    QgsLineSymbol,
    QgsMarkerSymbol,
    QgsPalLayerSettings,
    QgsStyle,
    QgsStyleModel,
    QgsStyleProxyModel,
    QgsSymbol,
    QgsTextFormat,
    QgsWkbTypes,
)

start_app()


class Dummy3dSymbol(QgsAbstract3DSymbol):

    def __init__(self):
        super().__init__()
        self.layer_types = [
            QgsWkbTypes.GeometryType.PointGeometry,
            QgsWkbTypes.GeometryType.LineGeometry,
        ]

    @staticmethod
    def create():
        return Dummy3dSymbol()

    def type(self):
        return "dummy"

    def clone(self):
        return Dummy3dSymbol()

    def readXml(self, elem, context):
        pass

    def writeXml(self, elem, context):
        pass

    def compatibleGeometryTypes(self):
        return self.layer_types


def createMarkerSymbol():
    symbol = QgsMarkerSymbol.createSimple(
        {"color": "100,150,50", "name": "square", "size": "3.0"}
    )
    return symbol


def createLineSymbol():
    symbol = QgsLineSymbol.createSimple({"color": "100,150,50"})
    return symbol


def createFillSymbol():
    symbol = QgsFillSymbol.createSimple({"color": "100,150,50"})
    return symbol


class TestQgsStyleModel(QgisTestCase):

    def test_style_with_symbols(self):

        style = QgsStyle()
        style.createMemoryDatabase()
        style.setName("style name")
        style.setFileName("/home/me/my.db")

        # style with only symbols

        symbol_a = createMarkerSymbol()
        symbol_a.setColor(QColor(255, 10, 10))
        self.assertTrue(style.addSymbol("a", symbol_a, True))
        style.tagSymbol(QgsStyle.StyleEntity.SymbolEntity, "a", ["tag 1", "tag 2"])
        symbol_B = createMarkerSymbol()
        symbol_B.setColor(QColor(10, 255, 10))
        self.assertTrue(style.addSymbol("B ", symbol_B, True))
        symbol_b = createFillSymbol()
        symbol_b.setColor(QColor(10, 255, 10))
        self.assertTrue(style.addSymbol("b", symbol_b, True))
        symbol_C = createLineSymbol()
        symbol_C.setColor(QColor(10, 255, 10))
        self.assertTrue(style.addSymbol("C", symbol_C, True))
        style.tagSymbol(QgsStyle.StyleEntity.SymbolEntity, "C", ["tag 3"])
        style.addFavorite(QgsStyle.StyleEntity.SymbolEntity, "C")
        symbol_C = createMarkerSymbol()
        symbol_C.setColor(QColor(10, 255, 10))
        self.assertTrue(style.addSymbol(" ----c/- ", symbol_C, True))

        model = QgsStyleModel(style)
        self.assertEqual(model.rowCount(), 5)
        self.assertEqual(model.columnCount(), 2)

        self.assertEqual(model.headerData(0, Qt.Orientation.Horizontal), "Name")
        self.assertEqual(model.headerData(1, Qt.Orientation.Horizontal), "Tags")

        self.assertTrue(model.index(0, 0).isValid())
        self.assertFalse(model.index(10, 0).isValid())
        self.assertFalse(model.index(0, 10).isValid())

        self.assertFalse(model.parent(model.index(0, 0)).isValid())

        self.assertFalse(model.flags(model.index(-1, 0)) & Qt.ItemFlag.ItemIsEditable)
        self.assertFalse(model.flags(model.index(5, 0)) & Qt.ItemFlag.ItemIsEditable)

        self.assertFalse(model.flags(model.index(0, 1)) & Qt.ItemFlag.ItemIsEditable)
        self.assertTrue(model.flags(model.index(0, 0)) & Qt.ItemFlag.ItemIsEditable)

        self.assertEqual(
            model.data(model.index(0, 0), QgsStyleModel.Role.StyleName), "style name"
        )
        self.assertEqual(
            model.data(model.index(0, 0), QgsStyleModel.Role.StyleFileName),
            "/home/me/my.db",
        )

        self.assertEqual(
            model.data(model.index(0, 0), QgsStyleModel.Role.EntityName), " ----c/- "
        )
        self.assertEqual(
            model.data(model.index(1, 0), QgsStyleModel.Role.EntityName), "B "
        )
        self.assertEqual(
            model.data(model.index(2, 0), QgsStyleModel.Role.EntityName), "C"
        )

        for role in (Qt.ItemDataRole.DisplayRole, Qt.ItemDataRole.EditRole):
            self.assertIsNone(model.data(model.index(-1, 0), role))
            self.assertIsNone(model.data(model.index(-1, 1), role))
            self.assertEqual(model.data(model.index(0, 0), role), " ----c/- ")
            self.assertFalse(model.data(model.index(0, 1), role))
            self.assertIsNone(model.data(model.index(0, 2), role))
            self.assertIsNone(model.data(model.index(0, -1), role))
            self.assertEqual(model.data(model.index(1, 0), role), "B ")
            self.assertFalse(model.data(model.index(1, 1), role))
            self.assertEqual(model.data(model.index(2, 0), role), "C")
            self.assertEqual(model.data(model.index(2, 1), role), "tag 3")
            self.assertEqual(model.data(model.index(3, 0), role), "a")
            self.assertEqual(model.data(model.index(3, 1), role), "tag 1, tag 2")
            self.assertEqual(model.data(model.index(4, 0), role), "b")
            self.assertFalse(model.data(model.index(4, 1), role))
            self.assertIsNone(model.data(model.index(5, 0), role))
            self.assertIsNone(model.data(model.index(5, 1), role))

        # decorations
        self.assertIsNone(
            model.data(model.index(-1, 0), Qt.ItemDataRole.DecorationRole)
        )
        self.assertIsNone(model.data(model.index(0, 1), Qt.ItemDataRole.DecorationRole))
        self.assertIsNone(model.data(model.index(5, 0), Qt.ItemDataRole.DecorationRole))
        self.assertFalse(
            model.data(model.index(0, 0), Qt.ItemDataRole.DecorationRole).isNull()
        )

        self.assertEqual(
            model.data(model.index(0, 0), QgsStyleModel.Role.TypeRole),
            QgsStyle.StyleEntity.SymbolEntity,
        )
        self.assertEqual(
            model.data(model.index(1, 0), QgsStyleModel.Role.TypeRole),
            QgsStyle.StyleEntity.SymbolEntity,
        )
        self.assertEqual(
            model.data(model.index(4, 0), QgsStyleModel.Role.TypeRole),
            QgsStyle.StyleEntity.SymbolEntity,
        )

        self.assertEqual(
            model.data(model.index(0, 0), QgsStyleModel.Role.IsFavoriteRole), False
        )
        self.assertEqual(
            model.data(model.index(1, 0), QgsStyleModel.Role.IsFavoriteRole), False
        )
        self.assertEqual(
            model.data(model.index(2, 0), QgsStyleModel.Role.IsFavoriteRole), True
        )
        self.assertEqual(
            model.data(model.index(3, 0), QgsStyleModel.Role.IsFavoriteRole), False
        )
        self.assertEqual(
            model.data(model.index(4, 0), QgsStyleModel.Role.IsFavoriteRole), False
        )

    def test_style_with_ramps(self):
        style = QgsStyle()
        style.createMemoryDatabase()

        # style with only ramps

        ramp_a = QgsLimitedRandomColorRamp(5)
        self.assertTrue(style.addColorRamp("a", ramp_a, True))
        style.tagSymbol(QgsStyle.StyleEntity.ColorrampEntity, "a", ["tag 1", "tag 2"])
        symbol_B = QgsLimitedRandomColorRamp()
        self.assertTrue(style.addColorRamp("B ", symbol_B, True))
        symbol_b = QgsLimitedRandomColorRamp()
        self.assertTrue(style.addColorRamp("b", symbol_b, True))
        symbol_C = QgsLimitedRandomColorRamp()
        self.assertTrue(style.addColorRamp("C", symbol_C, True))
        style.tagSymbol(QgsStyle.StyleEntity.ColorrampEntity, "C", ["tag 3"])
        style.addFavorite(QgsStyle.StyleEntity.ColorrampEntity, "C")
        symbol_C = QgsLimitedRandomColorRamp()
        self.assertTrue(style.addColorRamp(" ----c/- ", symbol_C, True))

        model = QgsStyleModel(style)
        self.assertEqual(model.rowCount(), 5)
        self.assertEqual(model.columnCount(), 2)

        self.assertTrue(model.index(0, 0).isValid())
        self.assertFalse(model.index(10, 0).isValid())
        self.assertFalse(model.index(0, 10).isValid())

        self.assertFalse(model.parent(model.index(0, 0)).isValid())

        for role in (Qt.ItemDataRole.DisplayRole, Qt.ItemDataRole.EditRole):
            self.assertIsNone(model.data(model.index(-1, 0), role))
            self.assertIsNone(model.data(model.index(-1, 1), role))
            self.assertEqual(model.data(model.index(0, 0), role), " ----c/- ")
            self.assertFalse(model.data(model.index(0, 1), role))
            self.assertIsNone(model.data(model.index(0, 2), role))
            self.assertIsNone(model.data(model.index(0, -1), role))
            self.assertEqual(model.data(model.index(1, 0), role), "B ")
            self.assertFalse(model.data(model.index(1, 1), role))
            self.assertEqual(model.data(model.index(2, 0), role), "C")
            self.assertEqual(model.data(model.index(2, 1), role), "tag 3")
            self.assertEqual(model.data(model.index(3, 0), role), "a")
            self.assertEqual(model.data(model.index(3, 1), role), "tag 1, tag 2")
            self.assertEqual(model.data(model.index(4, 0), role), "b")
            self.assertFalse(model.data(model.index(4, 1), role))
            self.assertIsNone(model.data(model.index(5, 0), role))
            self.assertIsNone(model.data(model.index(5, 1), role))

        # decorations
        self.assertIsNone(
            model.data(model.index(-1, 0), Qt.ItemDataRole.DecorationRole)
        )
        self.assertIsNone(model.data(model.index(0, 1), Qt.ItemDataRole.DecorationRole))
        self.assertIsNone(model.data(model.index(5, 0), Qt.ItemDataRole.DecorationRole))
        self.assertFalse(
            model.data(model.index(0, 0), Qt.ItemDataRole.DecorationRole).isNull()
        )

        self.assertEqual(
            model.data(model.index(0, 0), QgsStyleModel.Role.TypeRole),
            QgsStyle.StyleEntity.ColorrampEntity,
        )
        self.assertEqual(
            model.data(model.index(1, 0), QgsStyleModel.Role.TypeRole),
            QgsStyle.StyleEntity.ColorrampEntity,
        )
        self.assertEqual(
            model.data(model.index(4, 0), QgsStyleModel.Role.TypeRole),
            QgsStyle.StyleEntity.ColorrampEntity,
        )

        self.assertEqual(
            model.data(model.index(0, 0), QgsStyleModel.Role.IsFavoriteRole), False
        )
        self.assertEqual(
            model.data(model.index(1, 0), QgsStyleModel.Role.IsFavoriteRole), False
        )
        self.assertEqual(
            model.data(model.index(2, 0), QgsStyleModel.Role.IsFavoriteRole), True
        )
        self.assertEqual(
            model.data(model.index(3, 0), QgsStyleModel.Role.IsFavoriteRole), False
        )
        self.assertEqual(
            model.data(model.index(4, 0), QgsStyleModel.Role.IsFavoriteRole), False
        )

    def test_style_with_text_formats(self):
        style = QgsStyle()
        style.createMemoryDatabase()

        # style with text formats

        format_a = QgsTextFormat()
        self.assertTrue(style.addTextFormat("a", format_a, True))
        style.tagSymbol(QgsStyle.StyleEntity.TextFormatEntity, "a", ["tag 1", "tag 2"])
        format_B = QgsTextFormat()
        self.assertTrue(style.addTextFormat("B ", format_B, True))
        format_b = QgsTextFormat()
        self.assertTrue(style.addTextFormat("b", format_b, True))
        format_C = QgsTextFormat()
        self.assertTrue(style.addTextFormat("C", format_C, True))
        style.tagSymbol(QgsStyle.StyleEntity.TextFormatEntity, "C", ["tag 3"])
        style.addFavorite(QgsStyle.StyleEntity.TextFormatEntity, "C")
        format_C = QgsTextFormat()
        self.assertTrue(style.addTextFormat(" ----c/- ", format_C, True))

        model = QgsStyleModel(style)
        self.assertEqual(model.rowCount(), 5)
        self.assertEqual(model.columnCount(), 2)

        self.assertTrue(model.index(0, 0).isValid())
        self.assertFalse(model.index(10, 0).isValid())
        self.assertFalse(model.index(0, 10).isValid())

        self.assertFalse(model.parent(model.index(0, 0)).isValid())

        for role in (Qt.ItemDataRole.DisplayRole, Qt.ItemDataRole.EditRole):
            self.assertIsNone(model.data(model.index(-1, 0), role))
            self.assertIsNone(model.data(model.index(-1, 1), role))
            self.assertEqual(model.data(model.index(0, 0), role), " ----c/- ")
            self.assertFalse(model.data(model.index(0, 1), role))
            self.assertIsNone(model.data(model.index(0, 2), role))
            self.assertIsNone(model.data(model.index(0, -1), role))
            self.assertEqual(model.data(model.index(1, 0), role), "B ")
            self.assertFalse(model.data(model.index(1, 1), role))
            self.assertEqual(model.data(model.index(2, 0), role), "C")
            self.assertEqual(model.data(model.index(2, 1), role), "tag 3")
            self.assertEqual(model.data(model.index(3, 0), role), "a")
            self.assertEqual(model.data(model.index(3, 1), role), "tag 1, tag 2")
            self.assertEqual(model.data(model.index(4, 0), role), "b")
            self.assertFalse(model.data(model.index(4, 1), role))
            self.assertIsNone(model.data(model.index(5, 0), role))
            self.assertIsNone(model.data(model.index(5, 1), role))

        # decorations
        self.assertIsNone(
            model.data(model.index(-1, 0), Qt.ItemDataRole.DecorationRole)
        )
        self.assertIsNone(model.data(model.index(0, 1), Qt.ItemDataRole.DecorationRole))
        self.assertIsNone(model.data(model.index(5, 0), Qt.ItemDataRole.DecorationRole))
        self.assertFalse(
            model.data(model.index(0, 0), Qt.ItemDataRole.DecorationRole).isNull()
        )

        self.assertEqual(
            model.data(model.index(0, 0), QgsStyleModel.Role.TypeRole),
            QgsStyle.StyleEntity.TextFormatEntity,
        )
        self.assertEqual(
            model.data(model.index(1, 0), QgsStyleModel.Role.TypeRole),
            QgsStyle.StyleEntity.TextFormatEntity,
        )
        self.assertEqual(
            model.data(model.index(4, 0), QgsStyleModel.Role.TypeRole),
            QgsStyle.StyleEntity.TextFormatEntity,
        )

        self.assertEqual(
            model.data(model.index(0, 0), QgsStyleModel.Role.IsFavoriteRole), False
        )
        self.assertEqual(
            model.data(model.index(1, 0), QgsStyleModel.Role.IsFavoriteRole), False
        )
        self.assertEqual(
            model.data(model.index(2, 0), QgsStyleModel.Role.IsFavoriteRole), True
        )
        self.assertEqual(
            model.data(model.index(3, 0), QgsStyleModel.Role.IsFavoriteRole), False
        )
        self.assertEqual(
            model.data(model.index(4, 0), QgsStyleModel.Role.IsFavoriteRole), False
        )

    def test_style_with_label_settings(self):
        style = QgsStyle()
        style.createMemoryDatabase()

        # style with label settings

        format_a = QgsPalLayerSettings()
        format_a.layerType = QgsWkbTypes.GeometryType.PointGeometry
        self.assertTrue(style.addLabelSettings("a", format_a, True))
        style.tagSymbol(
            QgsStyle.StyleEntity.LabelSettingsEntity, "a", ["tag 1", "tag 2"]
        )
        format_B = QgsPalLayerSettings()
        format_B.layerType = QgsWkbTypes.GeometryType.LineGeometry
        self.assertTrue(style.addLabelSettings("B ", format_B, True))
        format_b = QgsPalLayerSettings()
        self.assertTrue(style.addLabelSettings("b", format_b, True))
        format_C = QgsPalLayerSettings()
        self.assertTrue(style.addLabelSettings("C", format_C, True))
        style.tagSymbol(QgsStyle.StyleEntity.LabelSettingsEntity, "C", ["tag 3"])
        style.addFavorite(QgsStyle.StyleEntity.LabelSettingsEntity, "C")
        format_C = QgsPalLayerSettings()
        self.assertTrue(style.addLabelSettings(" ----c/- ", format_C, True))

        self.assertEqual(
            style.labelSettingsLayerType("a"), QgsWkbTypes.GeometryType.PointGeometry
        )
        self.assertEqual(
            style.labelSettingsLayerType("B "), QgsWkbTypes.GeometryType.LineGeometry
        )

        model = QgsStyleModel(style)
        self.assertEqual(model.rowCount(), 5)
        self.assertEqual(model.columnCount(), 2)

        self.assertTrue(model.index(0, 0).isValid())
        self.assertFalse(model.index(10, 0).isValid())
        self.assertFalse(model.index(0, 10).isValid())

        self.assertFalse(model.parent(model.index(0, 0)).isValid())

        for role in (Qt.ItemDataRole.DisplayRole, Qt.ItemDataRole.EditRole):
            self.assertIsNone(model.data(model.index(-1, 0), role))
            self.assertIsNone(model.data(model.index(-1, 1), role))
            self.assertEqual(model.data(model.index(0, 0), role), " ----c/- ")
            self.assertFalse(model.data(model.index(0, 1), role))
            self.assertIsNone(model.data(model.index(0, 2), role))
            self.assertIsNone(model.data(model.index(0, -1), role))
            self.assertEqual(model.data(model.index(1, 0), role), "B ")
            self.assertFalse(model.data(model.index(1, 1), role))
            self.assertEqual(model.data(model.index(2, 0), role), "C")
            self.assertEqual(model.data(model.index(2, 1), role), "tag 3")
            self.assertEqual(model.data(model.index(3, 0), role), "a")
            self.assertEqual(model.data(model.index(3, 1), role), "tag 1, tag 2")
            self.assertEqual(model.data(model.index(4, 0), role), "b")
            self.assertFalse(model.data(model.index(4, 1), role))
            self.assertIsNone(model.data(model.index(5, 0), role))
            self.assertIsNone(model.data(model.index(5, 1), role))

        # decorations
        self.assertIsNone(
            model.data(model.index(-1, 0), Qt.ItemDataRole.DecorationRole)
        )
        self.assertIsNone(model.data(model.index(0, 1), Qt.ItemDataRole.DecorationRole))
        self.assertIsNone(model.data(model.index(5, 0), Qt.ItemDataRole.DecorationRole))
        self.assertFalse(
            model.data(model.index(0, 0), Qt.ItemDataRole.DecorationRole).isNull()
        )

        self.assertEqual(
            model.data(model.index(0, 0), QgsStyleModel.Role.TypeRole),
            QgsStyle.StyleEntity.LabelSettingsEntity,
        )
        self.assertEqual(
            model.data(model.index(1, 0), QgsStyleModel.Role.TypeRole),
            QgsStyle.StyleEntity.LabelSettingsEntity,
        )
        self.assertEqual(
            model.data(model.index(4, 0), QgsStyleModel.Role.TypeRole),
            QgsStyle.StyleEntity.LabelSettingsEntity,
        )

        self.assertEqual(
            model.data(model.index(0, 0), QgsStyleModel.Role.IsFavoriteRole), False
        )
        self.assertEqual(
            model.data(model.index(1, 0), QgsStyleModel.Role.IsFavoriteRole), False
        )
        self.assertEqual(
            model.data(model.index(2, 0), QgsStyleModel.Role.IsFavoriteRole), True
        )
        self.assertEqual(
            model.data(model.index(3, 0), QgsStyleModel.Role.IsFavoriteRole), False
        )
        self.assertEqual(
            model.data(model.index(4, 0), QgsStyleModel.Role.IsFavoriteRole), False
        )

        self.assertEqual(
            model.data(model.index(1, 0), QgsStyleModel.Role.LayerTypeRole),
            QgsWkbTypes.GeometryType.LineGeometry,
        )
        self.assertEqual(
            model.data(model.index(3, 0), QgsStyleModel.Role.LayerTypeRole),
            QgsWkbTypes.GeometryType.PointGeometry,
        )

    def test_style_with_legend_patch_shapes(self):
        style = QgsStyle()
        style.createMemoryDatabase()

        # style with legend patch shapes

        shape_a = QgsLegendPatchShape(
            QgsSymbol.SymbolType.Marker, QgsGeometry.fromWkt("Point( 4 5 )")
        )
        self.assertTrue(style.addLegendPatchShape("a", shape_a, True))
        style.tagSymbol(
            QgsStyle.StyleEntity.LegendPatchShapeEntity, "a", ["tag 1", "tag 2"]
        )
        shape_B = QgsLegendPatchShape(
            QgsSymbol.SymbolType.Line, QgsGeometry.fromWkt("LineString( 4 5, 6 4 )")
        )
        self.assertTrue(style.addLegendPatchShape("B ", shape_B, True))
        shape_b = QgsLegendPatchShape(
            QgsSymbol.SymbolType.Line, QgsGeometry.fromWkt("LineString( 14 5, 6 4 )")
        )
        self.assertTrue(style.addLegendPatchShape("b", shape_b, True))
        shape_C = QgsLegendPatchShape(
            QgsSymbol.SymbolType.Fill,
            QgsGeometry.fromWkt("Polygon(( 4 5, 6 4, 7 3, 4 5) )"),
        )
        self.assertTrue(style.addLegendPatchShape("C", shape_C, True))
        style.tagSymbol(QgsStyle.StyleEntity.LegendPatchShapeEntity, "C", ["tag 3"])
        style.addFavorite(QgsStyle.StyleEntity.LegendPatchShapeEntity, "C")
        shape_C = QgsLegendPatchShape(
            QgsSymbol.SymbolType.Fill,
            QgsGeometry.fromWkt("Polygon(( 4 5, 16 4, 7 3, 4 5) )"),
        )
        self.assertTrue(style.addLegendPatchShape(" ----c/- ", shape_C, True))

        self.assertEqual(
            style.legendPatchShapeSymbolType("a"), QgsSymbol.SymbolType.Marker
        )
        self.assertEqual(
            style.legendPatchShapeSymbolType("B "), QgsSymbol.SymbolType.Line
        )

        model = QgsStyleModel(style)
        self.assertEqual(model.rowCount(), 5)
        self.assertEqual(model.columnCount(), 2)

        self.assertTrue(model.index(0, 0).isValid())
        self.assertFalse(model.index(10, 0).isValid())
        self.assertFalse(model.index(0, 10).isValid())

        self.assertFalse(model.parent(model.index(0, 0)).isValid())

        for role in (Qt.ItemDataRole.DisplayRole, Qt.ItemDataRole.EditRole):
            self.assertIsNone(model.data(model.index(-1, 0), role))
            self.assertIsNone(model.data(model.index(-1, 1), role))
            self.assertEqual(model.data(model.index(0, 0), role), " ----c/- ")
            self.assertFalse(model.data(model.index(0, 1), role))
            self.assertIsNone(model.data(model.index(0, 2), role))
            self.assertIsNone(model.data(model.index(0, -1), role))
            self.assertEqual(model.data(model.index(1, 0), role), "B ")
            self.assertFalse(model.data(model.index(1, 1), role))
            self.assertEqual(model.data(model.index(2, 0), role), "C")
            self.assertEqual(model.data(model.index(2, 1), role), "tag 3")
            self.assertEqual(model.data(model.index(3, 0), role), "a")
            self.assertEqual(model.data(model.index(3, 1), role), "tag 1, tag 2")
            self.assertEqual(model.data(model.index(4, 0), role), "b")
            self.assertFalse(model.data(model.index(4, 1), role))
            self.assertIsNone(model.data(model.index(5, 0), role))
            self.assertIsNone(model.data(model.index(5, 1), role))

        # decorations
        self.assertIsNone(
            model.data(model.index(-1, 0), Qt.ItemDataRole.DecorationRole)
        )
        self.assertIsNone(model.data(model.index(0, 1), Qt.ItemDataRole.DecorationRole))
        self.assertIsNone(model.data(model.index(5, 0), Qt.ItemDataRole.DecorationRole))
        self.assertFalse(
            model.data(model.index(0, 0), Qt.ItemDataRole.DecorationRole).isNull()
        )

        self.assertEqual(
            model.data(model.index(0, 0), QgsStyleModel.Role.TypeRole),
            QgsStyle.StyleEntity.LegendPatchShapeEntity,
        )
        self.assertEqual(
            model.data(model.index(1, 0), QgsStyleModel.Role.TypeRole),
            QgsStyle.StyleEntity.LegendPatchShapeEntity,
        )
        self.assertEqual(
            model.data(model.index(4, 0), QgsStyleModel.Role.TypeRole),
            QgsStyle.StyleEntity.LegendPatchShapeEntity,
        )

        self.assertEqual(
            model.data(model.index(0, 0), QgsStyleModel.Role.IsFavoriteRole), False
        )
        self.assertEqual(
            model.data(model.index(1, 0), QgsStyleModel.Role.IsFavoriteRole), False
        )
        self.assertEqual(
            model.data(model.index(2, 0), QgsStyleModel.Role.IsFavoriteRole), True
        )
        self.assertEqual(
            model.data(model.index(3, 0), QgsStyleModel.Role.IsFavoriteRole), False
        )
        self.assertEqual(
            model.data(model.index(4, 0), QgsStyleModel.Role.IsFavoriteRole), False
        )

        self.assertEqual(
            model.data(model.index(0, 0), QgsStyleModel.Role.SymbolTypeRole),
            QgsSymbol.SymbolType.Fill,
        )
        self.assertEqual(
            model.data(model.index(1, 0), QgsStyleModel.Role.SymbolTypeRole),
            QgsSymbol.SymbolType.Line,
        )
        self.assertEqual(
            model.data(model.index(2, 0), QgsStyleModel.Role.SymbolTypeRole),
            QgsSymbol.SymbolType.Fill,
        )
        self.assertEqual(
            model.data(model.index(3, 0), QgsStyleModel.Role.SymbolTypeRole),
            QgsSymbol.SymbolType.Marker,
        )
        self.assertEqual(
            model.data(model.index(4, 0), QgsStyleModel.Role.SymbolTypeRole),
            QgsSymbol.SymbolType.Line,
        )

    def test_style_with_3d_symbols(self):
        style = QgsStyle()
        style.createMemoryDatabase()

        # style with 3d symbols

        symbol_a = Dummy3dSymbol()
        self.assertTrue(style.addSymbol3D("a", symbol_a, True))
        style.tagSymbol(QgsStyle.StyleEntity.Symbol3DEntity, "a", ["tag 1", "tag 2"])
        symbol_B = Dummy3dSymbol()
        self.assertTrue(style.addSymbol3D("B ", symbol_B, True))
        symbol_b = Dummy3dSymbol()
        self.assertTrue(style.addSymbol3D("b", symbol_b, True))
        symbol_C = Dummy3dSymbol()
        self.assertTrue(style.addSymbol3D("C", symbol_C, True))
        style.tagSymbol(QgsStyle.StyleEntity.Symbol3DEntity, "C", ["tag 3"])
        style.addFavorite(QgsStyle.StyleEntity.Symbol3DEntity, "C")
        symbol_C = Dummy3dSymbol()
        self.assertTrue(style.addSymbol3D(" ----c/- ", symbol_C, True))

        model = QgsStyleModel(style)
        self.assertEqual(model.rowCount(), 5)
        self.assertEqual(model.columnCount(), 2)

        self.assertTrue(model.index(0, 0).isValid())
        self.assertFalse(model.index(10, 0).isValid())
        self.assertFalse(model.index(0, 10).isValid())

        self.assertFalse(model.parent(model.index(0, 0)).isValid())

        for role in (Qt.ItemDataRole.DisplayRole, Qt.ItemDataRole.EditRole):
            self.assertIsNone(model.data(model.index(-1, 0), role))
            self.assertIsNone(model.data(model.index(-1, 1), role))
            self.assertEqual(model.data(model.index(0, 0), role), " ----c/- ")
            self.assertFalse(model.data(model.index(0, 1), role))
            self.assertIsNone(model.data(model.index(0, 2), role))
            self.assertIsNone(model.data(model.index(0, -1), role))
            self.assertEqual(model.data(model.index(1, 0), role), "B ")
            self.assertFalse(model.data(model.index(1, 1), role))
            self.assertEqual(model.data(model.index(2, 0), role), "C")
            self.assertEqual(model.data(model.index(2, 1), role), "tag 3")
            self.assertEqual(model.data(model.index(3, 0), role), "a")
            self.assertEqual(model.data(model.index(3, 1), role), "tag 1, tag 2")
            self.assertEqual(model.data(model.index(4, 0), role), "b")
            self.assertFalse(model.data(model.index(4, 1), role))
            self.assertIsNone(model.data(model.index(5, 0), role))
            self.assertIsNone(model.data(model.index(5, 1), role))

        # decorations
        self.assertIsNone(
            model.data(model.index(-1, 0), Qt.ItemDataRole.DecorationRole)
        )
        self.assertIsNone(model.data(model.index(0, 1), Qt.ItemDataRole.DecorationRole))
        self.assertIsNone(model.data(model.index(5, 0), Qt.ItemDataRole.DecorationRole))
        # self.assertFalse(model.data(model.index(0, 0), Qt.DecorationRole).isNull())

        self.assertEqual(
            model.data(model.index(0, 0), QgsStyleModel.Role.TypeRole),
            QgsStyle.StyleEntity.Symbol3DEntity,
        )
        self.assertEqual(
            model.data(model.index(1, 0), QgsStyleModel.Role.TypeRole),
            QgsStyle.StyleEntity.Symbol3DEntity,
        )
        self.assertEqual(
            model.data(model.index(4, 0), QgsStyleModel.Role.TypeRole),
            QgsStyle.StyleEntity.Symbol3DEntity,
        )

        self.assertEqual(
            model.data(model.index(0, 0), QgsStyleModel.Role.IsFavoriteRole), False
        )
        self.assertEqual(
            model.data(model.index(1, 0), QgsStyleModel.Role.IsFavoriteRole), False
        )
        self.assertEqual(
            model.data(model.index(2, 0), QgsStyleModel.Role.IsFavoriteRole), True
        )
        self.assertEqual(
            model.data(model.index(3, 0), QgsStyleModel.Role.IsFavoriteRole), False
        )
        self.assertEqual(
            model.data(model.index(4, 0), QgsStyleModel.Role.IsFavoriteRole), False
        )

        self.assertEqual(
            model.data(
                model.index(0, 0), QgsStyleModel.Role.CompatibleGeometryTypesRole
            ),
            [0, 1],
        )
        self.assertEqual(
            model.data(
                model.index(1, 0), QgsStyleModel.Role.CompatibleGeometryTypesRole
            ),
            [0, 1],
        )
        self.assertEqual(
            model.data(
                model.index(2, 0), QgsStyleModel.Role.CompatibleGeometryTypesRole
            ),
            [0, 1],
        )
        self.assertEqual(
            model.data(
                model.index(3, 0), QgsStyleModel.Role.CompatibleGeometryTypesRole
            ),
            [0, 1],
        )
        self.assertEqual(
            model.data(
                model.index(4, 0), QgsStyleModel.Role.CompatibleGeometryTypesRole
            ),
            [0, 1],
        )
        self.assertEqual(
            model.data(
                model.index(5, 0), QgsStyleModel.Role.CompatibleGeometryTypesRole
            ),
            None,
        )

    def test_mixed_style(self):
        """
        Test style with both symbols and ramps
        """
        style = QgsStyle()
        style.createMemoryDatabase()

        # style with mixed contents

        symbol_a = createMarkerSymbol()
        symbol_a.setColor(QColor(255, 10, 10))
        self.assertTrue(style.addSymbol("a", symbol_a, True))
        style.tagSymbol(QgsStyle.StyleEntity.SymbolEntity, "a", ["tag 1", "tag 2"])
        symbol_B = createMarkerSymbol()
        symbol_B.setColor(QColor(10, 255, 10))
        self.assertTrue(style.addSymbol("B ", symbol_B, True))
        symbol_b = createFillSymbol()
        symbol_b.setColor(QColor(10, 255, 10))
        self.assertTrue(style.addSymbol("b", symbol_b, True))
        symbol_C = createLineSymbol()
        symbol_C.setColor(QColor(10, 255, 10))
        self.assertTrue(style.addSymbol("C", symbol_C, True))
        style.tagSymbol(QgsStyle.StyleEntity.SymbolEntity, "C", ["tag 3"])
        symbol_C = createMarkerSymbol()
        symbol_C.setColor(QColor(10, 255, 10))
        self.assertTrue(style.addSymbol(" ----c/- ", symbol_C, True))
        ramp_a = QgsLimitedRandomColorRamp(5)
        self.assertTrue(style.addColorRamp("ramp a", ramp_a, True))
        style.tagSymbol(
            QgsStyle.StyleEntity.ColorrampEntity, "ramp a", ["tag 1", "tag 2"]
        )
        symbol_B = QgsLimitedRandomColorRamp()
        self.assertTrue(style.addColorRamp("ramp B ", symbol_B, True))
        symbol_b = QgsLimitedRandomColorRamp()
        self.assertTrue(style.addColorRamp("ramp c", symbol_b, True))
        format_a = QgsTextFormat()
        self.assertTrue(style.addTextFormat("format a", format_a, True))
        style.tagSymbol(
            QgsStyle.StyleEntity.TextFormatEntity, "format a", ["tag 1", "tag 2"]
        )
        symbol_B = QgsTextFormat()
        self.assertTrue(style.addTextFormat("format B ", symbol_B, True))
        symbol_b = QgsTextFormat()
        self.assertTrue(style.addTextFormat("format c", symbol_b, True))
        settings_a = QgsPalLayerSettings()
        self.assertTrue(style.addLabelSettings("settings a", settings_a, True))
        style.tagSymbol(
            QgsStyle.StyleEntity.LabelSettingsEntity, "settings a", ["tag 1", "tag 2"]
        )
        symbol_B = QgsPalLayerSettings()
        self.assertTrue(style.addLabelSettings("settings B ", symbol_B, True))
        symbol_b = QgsPalLayerSettings()
        self.assertTrue(style.addLabelSettings("settings c", symbol_b, True))
        shape_a = QgsLegendPatchShape(
            QgsSymbol.SymbolType.Marker, QgsGeometry.fromWkt("Point (4 5)")
        )
        self.assertTrue(style.addLegendPatchShape("shape a", shape_a, True))
        style.tagSymbol(
            QgsStyle.StyleEntity.LegendPatchShapeEntity, "shape a", ["tag 1", "tag 2"]
        )
        shape_B = QgsLegendPatchShape(
            QgsSymbol.SymbolType.Marker, QgsGeometry.fromWkt("Point (14 15)")
        )
        self.assertTrue(style.addLegendPatchShape("shape B ", shape_B, True))
        shape_b = QgsLegendPatchShape(
            QgsSymbol.SymbolType.Marker, QgsGeometry.fromWkt("Point (14 25)")
        )
        self.assertTrue(style.addLegendPatchShape("shape c", shape_b, True))
        symbol3d_a = Dummy3dSymbol()
        self.assertTrue(style.addSymbol3D("symbol3d a", symbol3d_a, True))
        style.tagSymbol(
            QgsStyle.StyleEntity.Symbol3DEntity, "symbol3d a", ["tag 1", "tag 2"]
        )
        symbol3d_B = Dummy3dSymbol()
        self.assertTrue(style.addSymbol3D("symbol3d B ", symbol3d_B, True))
        symbol3d_b = Dummy3dSymbol()
        self.assertTrue(style.addSymbol3D("symbol3d c", symbol3d_b, True))

        model = QgsStyleModel(style)
        self.assertEqual(model.rowCount(), 20)
        self.assertEqual(model.columnCount(), 2)

        self.assertTrue(model.index(0, 0).isValid())
        self.assertFalse(model.index(20, 0).isValid())
        self.assertFalse(model.index(0, 10).isValid())

        self.assertFalse(model.parent(model.index(0, 0)).isValid())

        for role in (Qt.ItemDataRole.DisplayRole, Qt.ItemDataRole.EditRole):
            self.assertIsNone(model.data(model.index(-1, 0), role))
            self.assertIsNone(model.data(model.index(-1, 1), role))
            self.assertEqual(model.data(model.index(0, 0), role), " ----c/- ")
            self.assertFalse(model.data(model.index(0, 1), role))
            self.assertIsNone(model.data(model.index(0, 2), role))
            self.assertIsNone(model.data(model.index(0, -1), role))
            self.assertEqual(model.data(model.index(1, 0), role), "B ")
            self.assertFalse(model.data(model.index(1, 1), role))
            self.assertEqual(model.data(model.index(2, 0), role), "C")
            self.assertEqual(model.data(model.index(2, 1), role), "tag 3")
            self.assertEqual(model.data(model.index(3, 0), role), "a")
            self.assertEqual(model.data(model.index(3, 1), role), "tag 1, tag 2")
            self.assertEqual(model.data(model.index(4, 0), role), "b")
            self.assertFalse(model.data(model.index(4, 1), role))
            self.assertEqual(model.data(model.index(5, 0), role), "ramp B ")
            self.assertFalse(model.data(model.index(5, 1), role))
            self.assertEqual(model.data(model.index(6, 0), role), "ramp a")
            self.assertEqual(model.data(model.index(6, 1), role), "tag 1, tag 2")
            self.assertEqual(model.data(model.index(7, 0), role), "ramp c")
            self.assertFalse(model.data(model.index(7, 1), role))
            self.assertEqual(model.data(model.index(8, 0), role), "format B ")
            self.assertFalse(model.data(model.index(8, 1), role))
            self.assertEqual(model.data(model.index(9, 0), role), "format a")
            self.assertEqual(model.data(model.index(9, 1), role), "tag 1, tag 2")
            self.assertEqual(model.data(model.index(10, 0), role), "format c")
            self.assertFalse(model.data(model.index(10, 1), role))
            self.assertEqual(model.data(model.index(11, 0), role), "settings B ")
            self.assertFalse(model.data(model.index(11, 1), role))
            self.assertEqual(model.data(model.index(12, 0), role), "settings a")
            self.assertEqual(model.data(model.index(12, 1), role), "tag 1, tag 2")
            self.assertEqual(model.data(model.index(13, 0), role), "settings c")
            self.assertFalse(model.data(model.index(13, 1), role))
            self.assertEqual(model.data(model.index(14, 0), role), "shape B ")
            self.assertFalse(model.data(model.index(14, 1), role))
            self.assertEqual(model.data(model.index(15, 0), role), "shape a")
            self.assertEqual(model.data(model.index(15, 1), role), "tag 1, tag 2")
            self.assertEqual(model.data(model.index(16, 0), role), "shape c")
            self.assertFalse(model.data(model.index(16, 1), role))
            self.assertEqual(model.data(model.index(17, 0), role), "symbol3d B ")
            self.assertFalse(model.data(model.index(17, 1), role))
            self.assertEqual(model.data(model.index(18, 0), role), "symbol3d a")
            self.assertEqual(model.data(model.index(18, 1), role), "tag 1, tag 2")
            self.assertEqual(model.data(model.index(19, 0), role), "symbol3d c")
            self.assertFalse(model.data(model.index(19, 1), role))
            self.assertIsNone(model.data(model.index(20, 0), role))
            self.assertIsNone(model.data(model.index(20, 1), role))

        # decorations
        self.assertIsNone(
            model.data(model.index(-1, 0), Qt.ItemDataRole.DecorationRole)
        )
        self.assertIsNone(model.data(model.index(0, 1), Qt.ItemDataRole.DecorationRole))
        self.assertIsNone(
            model.data(model.index(20, 0), Qt.ItemDataRole.DecorationRole)
        )
        self.assertFalse(
            model.data(model.index(0, 0), Qt.ItemDataRole.DecorationRole).isNull()
        )
        self.assertFalse(
            model.data(model.index(5, 0), Qt.ItemDataRole.DecorationRole).isNull()
        )
        self.assertFalse(
            model.data(model.index(8, 0), Qt.ItemDataRole.DecorationRole).isNull()
        )
        self.assertFalse(
            model.data(model.index(11, 0), Qt.ItemDataRole.DecorationRole).isNull()
        )
        self.assertFalse(
            model.data(model.index(14, 0), Qt.ItemDataRole.DecorationRole).isNull()
        )
        # self.assertFalse(model.data(model.index(17, 0), Qt.DecorationRole).isNull())

        self.assertEqual(
            model.data(model.index(0, 0), QgsStyleModel.Role.TypeRole),
            QgsStyle.StyleEntity.SymbolEntity,
        )
        self.assertEqual(
            model.data(model.index(1, 0), QgsStyleModel.Role.TypeRole),
            QgsStyle.StyleEntity.SymbolEntity,
        )
        self.assertEqual(
            model.data(model.index(4, 0), QgsStyleModel.Role.TypeRole),
            QgsStyle.StyleEntity.SymbolEntity,
        )
        self.assertEqual(
            model.data(model.index(5, 0), QgsStyleModel.Role.TypeRole),
            QgsStyle.StyleEntity.ColorrampEntity,
        )
        self.assertEqual(
            model.data(model.index(6, 0), QgsStyleModel.Role.TypeRole),
            QgsStyle.StyleEntity.ColorrampEntity,
        )
        self.assertEqual(
            model.data(model.index(7, 0), QgsStyleModel.Role.TypeRole),
            QgsStyle.StyleEntity.ColorrampEntity,
        )
        self.assertEqual(
            model.data(model.index(8, 0), QgsStyleModel.Role.TypeRole),
            QgsStyle.StyleEntity.TextFormatEntity,
        )
        self.assertEqual(
            model.data(model.index(9, 0), QgsStyleModel.Role.TypeRole),
            QgsStyle.StyleEntity.TextFormatEntity,
        )
        self.assertEqual(
            model.data(model.index(10, 0), QgsStyleModel.Role.TypeRole),
            QgsStyle.StyleEntity.TextFormatEntity,
        )
        self.assertEqual(
            model.data(model.index(11, 0), QgsStyleModel.Role.TypeRole),
            QgsStyle.StyleEntity.LabelSettingsEntity,
        )
        self.assertEqual(
            model.data(model.index(12, 0), QgsStyleModel.Role.TypeRole),
            QgsStyle.StyleEntity.LabelSettingsEntity,
        )
        self.assertEqual(
            model.data(model.index(13, 0), QgsStyleModel.Role.TypeRole),
            QgsStyle.StyleEntity.LabelSettingsEntity,
        )
        self.assertEqual(
            model.data(model.index(14, 0), QgsStyleModel.Role.TypeRole),
            QgsStyle.StyleEntity.LegendPatchShapeEntity,
        )
        self.assertEqual(
            model.data(model.index(15, 0), QgsStyleModel.Role.TypeRole),
            QgsStyle.StyleEntity.LegendPatchShapeEntity,
        )
        self.assertEqual(
            model.data(model.index(16, 0), QgsStyleModel.Role.TypeRole),
            QgsStyle.StyleEntity.LegendPatchShapeEntity,
        )
        self.assertEqual(
            model.data(model.index(17, 0), QgsStyleModel.Role.TypeRole),
            QgsStyle.StyleEntity.Symbol3DEntity,
        )
        self.assertEqual(
            model.data(model.index(18, 0), QgsStyleModel.Role.TypeRole),
            QgsStyle.StyleEntity.Symbol3DEntity,
        )
        self.assertEqual(
            model.data(model.index(19, 0), QgsStyleModel.Role.TypeRole),
            QgsStyle.StyleEntity.Symbol3DEntity,
        )

    def test_add_delete_symbols(self):
        style = QgsStyle()
        style.createMemoryDatabase()

        model = QgsStyleModel(style)
        self.assertEqual(model.rowCount(), 0)

        symbol = createMarkerSymbol()
        self.assertTrue(style.addSymbol("a", symbol, True))
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )

        symbol = createMarkerSymbol()
        self.assertTrue(style.addSymbol("c", symbol, True))
        self.assertEqual(model.rowCount(), 2)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "c"
        )

        symbol = createMarkerSymbol()
        self.assertTrue(style.addSymbol("b", symbol, True))
        self.assertEqual(model.rowCount(), 3)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "b"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "c"
        )

        symbol = createMarkerSymbol()
        self.assertTrue(style.addSymbol("1", symbol, True))
        self.assertEqual(model.rowCount(), 4)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "1"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "b"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "c"
        )

        self.assertFalse(style.removeSymbol("xxxx"))
        self.assertEqual(model.rowCount(), 4)

        self.assertTrue(style.removeSymbol("b"))
        self.assertEqual(model.rowCount(), 3)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "1"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "c"
        )

        self.assertTrue(style.removeSymbol("1"))
        self.assertEqual(model.rowCount(), 2)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "c"
        )

        self.assertTrue(style.removeSymbol("c"))
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )

        self.assertTrue(style.removeSymbol("a"))
        self.assertEqual(model.rowCount(), 0)

    def test_add_remove_ramps(self):
        style = QgsStyle()
        style.createMemoryDatabase()

        model = QgsStyleModel(style)
        symbol = createMarkerSymbol()
        self.assertTrue(style.addSymbol("a", symbol, True))
        symbol = createMarkerSymbol()
        self.assertTrue(style.addSymbol("c", symbol, True))
        self.assertEqual(model.rowCount(), 2)

        ramp_a = QgsLimitedRandomColorRamp(5)
        self.assertTrue(style.addColorRamp("ramp a", ramp_a, True))
        self.assertEqual(model.rowCount(), 3)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "c"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp a"
        )

        symbol_B = QgsLimitedRandomColorRamp()
        self.assertTrue(style.addColorRamp("ramp c", symbol_B, True))
        self.assertEqual(model.rowCount(), 4)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "c"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp a"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "ramp c"
        )

        symbol_b = QgsLimitedRandomColorRamp()
        self.assertTrue(style.addColorRamp("ramp b", symbol_b, True))
        self.assertEqual(model.rowCount(), 5)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "c"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp a"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "ramp b"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "ramp c"
        )

        self.assertTrue(style.removeColorRamp("ramp a"))
        self.assertEqual(model.rowCount(), 4)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "c"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp b"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "ramp c"
        )

    def test_add_remove_text_formats(self):
        style = QgsStyle()
        style.createMemoryDatabase()

        model = QgsStyleModel(style)
        symbol = createMarkerSymbol()
        self.assertTrue(style.addSymbol("a", symbol, True))
        symbol = createMarkerSymbol()
        self.assertTrue(style.addSymbol("c", symbol, True))
        self.assertEqual(model.rowCount(), 2)

        ramp_a = QgsLimitedRandomColorRamp(5)
        self.assertTrue(style.addColorRamp("ramp a", ramp_a, True))
        self.assertEqual(model.rowCount(), 3)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "c"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp a"
        )

        format_a = QgsTextFormat()
        self.assertTrue(style.addTextFormat("format a", format_a, True))
        self.assertEqual(model.rowCount(), 4)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "c"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp a"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "format a"
        )

        format_B = QgsTextFormat()
        self.assertTrue(style.addTextFormat("format c", format_B, True))
        self.assertEqual(model.rowCount(), 5)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "c"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp a"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "format a"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "format c"
        )

        symbol_b = QgsTextFormat()
        self.assertTrue(style.addTextFormat("format b", symbol_b, True))
        self.assertEqual(model.rowCount(), 6)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "c"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp a"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "format a"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "format b"
        )
        self.assertEqual(
            model.data(model.index(5, 0), Qt.ItemDataRole.DisplayRole), "format c"
        )

        self.assertTrue(style.removeTextFormat("format a"))
        self.assertEqual(model.rowCount(), 5)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "c"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp a"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "format b"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "format c"
        )

        self.assertTrue(style.removeColorRamp("ramp a"))
        self.assertEqual(model.rowCount(), 4)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "c"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "format b"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "format c"
        )

        self.assertTrue(style.removeSymbol("c"))
        self.assertEqual(model.rowCount(), 3)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "format b"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "format c"
        )

    def test_add_remove_label_settings(self):
        style = QgsStyle()
        style.createMemoryDatabase()

        model = QgsStyleModel(style)
        symbol = createMarkerSymbol()
        self.assertTrue(style.addSymbol("a", symbol, True))
        symbol = createMarkerSymbol()
        self.assertTrue(style.addSymbol("c", symbol, True))
        self.assertEqual(model.rowCount(), 2)

        ramp_a = QgsLimitedRandomColorRamp(5)
        self.assertTrue(style.addColorRamp("ramp a", ramp_a, True))
        self.assertEqual(model.rowCount(), 3)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "c"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp a"
        )

        format_a = QgsTextFormat()
        self.assertTrue(style.addTextFormat("format a", format_a, True))
        self.assertEqual(model.rowCount(), 4)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "c"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp a"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "format a"
        )

        settings_a = QgsPalLayerSettings()
        self.assertTrue(style.addLabelSettings("settings a", settings_a, True))
        self.assertEqual(model.rowCount(), 5)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "c"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp a"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "format a"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "settings a"
        )

        settings_B = QgsPalLayerSettings()
        self.assertTrue(style.addLabelSettings("settings c", settings_B, True))
        self.assertEqual(model.rowCount(), 6)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "c"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp a"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "format a"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "settings a"
        )
        self.assertEqual(
            model.data(model.index(5, 0), Qt.ItemDataRole.DisplayRole), "settings c"
        )

        settings_b = QgsPalLayerSettings()
        self.assertTrue(style.addLabelSettings("settings b", settings_b, True))
        self.assertEqual(model.rowCount(), 7)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "c"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp a"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "format a"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "settings a"
        )
        self.assertEqual(
            model.data(model.index(5, 0), Qt.ItemDataRole.DisplayRole), "settings b"
        )
        self.assertEqual(
            model.data(model.index(6, 0), Qt.ItemDataRole.DisplayRole), "settings c"
        )

        self.assertTrue(style.removeLabelSettings("settings a"))
        self.assertEqual(model.rowCount(), 6)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "c"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp a"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "format a"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "settings b"
        )
        self.assertEqual(
            model.data(model.index(5, 0), Qt.ItemDataRole.DisplayRole), "settings c"
        )

        self.assertTrue(style.removeTextFormat("format a"))
        self.assertEqual(model.rowCount(), 5)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "c"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp a"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "settings b"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "settings c"
        )

        self.assertTrue(style.removeColorRamp("ramp a"))
        self.assertEqual(model.rowCount(), 4)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "c"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "settings b"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "settings c"
        )

        self.assertTrue(style.removeSymbol("c"))
        self.assertEqual(model.rowCount(), 3)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "settings b"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "settings c"
        )

    def test_add_remove_legend_patch_shape(self):
        style = QgsStyle()
        style.createMemoryDatabase()

        model = QgsStyleModel(style)
        symbol = createMarkerSymbol()
        self.assertTrue(style.addSymbol("a", symbol, True))
        symbol = createMarkerSymbol()
        self.assertTrue(style.addSymbol("c", symbol, True))
        self.assertEqual(model.rowCount(), 2)

        ramp_a = QgsLimitedRandomColorRamp(5)
        self.assertTrue(style.addColorRamp("ramp a", ramp_a, True))
        self.assertEqual(model.rowCount(), 3)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "c"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp a"
        )

        format_a = QgsTextFormat()
        self.assertTrue(style.addTextFormat("format a", format_a, True))
        self.assertEqual(model.rowCount(), 4)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "c"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp a"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "format a"
        )

        settings_a = QgsPalLayerSettings()
        self.assertTrue(style.addLabelSettings("settings a", settings_a, True))
        self.assertEqual(model.rowCount(), 5)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "c"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp a"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "format a"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "settings a"
        )

        shape_a = QgsLegendPatchShape()
        self.assertTrue(style.addLegendPatchShape("shape a", shape_a, True))
        self.assertEqual(model.rowCount(), 6)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "c"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp a"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "format a"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "settings a"
        )
        self.assertEqual(
            model.data(model.index(5, 0), Qt.ItemDataRole.DisplayRole), "shape a"
        )

        shape_B = QgsLegendPatchShape()
        self.assertTrue(style.addLegendPatchShape("shape c", shape_B, True))
        self.assertEqual(model.rowCount(), 7)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "c"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp a"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "format a"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "settings a"
        )
        self.assertEqual(
            model.data(model.index(5, 0), Qt.ItemDataRole.DisplayRole), "shape a"
        )
        self.assertEqual(
            model.data(model.index(6, 0), Qt.ItemDataRole.DisplayRole), "shape c"
        )

        shape_b = QgsLegendPatchShape()
        self.assertTrue(style.addLegendPatchShape("shape b", shape_b, True))
        self.assertEqual(model.rowCount(), 8)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "c"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp a"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "format a"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "settings a"
        )
        self.assertEqual(
            model.data(model.index(5, 0), Qt.ItemDataRole.DisplayRole), "shape a"
        )
        self.assertEqual(
            model.data(model.index(6, 0), Qt.ItemDataRole.DisplayRole), "shape b"
        )
        self.assertEqual(
            model.data(model.index(7, 0), Qt.ItemDataRole.DisplayRole), "shape c"
        )

        self.assertTrue(
            style.removeEntityByName(
                QgsStyle.StyleEntity.LegendPatchShapeEntity, "shape a"
            )
        )
        self.assertEqual(model.rowCount(), 7)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "c"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp a"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "format a"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "settings a"
        )
        self.assertEqual(
            model.data(model.index(5, 0), Qt.ItemDataRole.DisplayRole), "shape b"
        )
        self.assertEqual(
            model.data(model.index(6, 0), Qt.ItemDataRole.DisplayRole), "shape c"
        )

        self.assertTrue(
            style.removeEntityByName(
                QgsStyle.StyleEntity.LabelSettingsEntity, "settings a"
            )
        )
        self.assertEqual(model.rowCount(), 6)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "c"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp a"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "format a"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "shape b"
        )
        self.assertEqual(
            model.data(model.index(5, 0), Qt.ItemDataRole.DisplayRole), "shape c"
        )

        self.assertTrue(
            style.removeEntityByName(QgsStyle.StyleEntity.TextFormatEntity, "format a")
        )
        self.assertEqual(model.rowCount(), 5)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "c"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp a"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "shape b"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "shape c"
        )

        self.assertTrue(
            style.removeEntityByName(QgsStyle.StyleEntity.ColorrampEntity, "ramp a")
        )
        self.assertEqual(model.rowCount(), 4)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "c"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "shape b"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "shape c"
        )

        self.assertTrue(
            style.removeEntityByName(QgsStyle.StyleEntity.SymbolEntity, "c")
        )
        self.assertEqual(model.rowCount(), 3)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "shape b"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "shape c"
        )

    def test_add_symbol3d(self):
        style = QgsStyle()
        style.createMemoryDatabase()

        model = QgsStyleModel(style)
        symbol = createMarkerSymbol()
        self.assertTrue(style.addSymbol("a", symbol, True))
        symbol = createMarkerSymbol()
        self.assertTrue(style.addSymbol("c", symbol, True))
        self.assertEqual(model.rowCount(), 2)

        ramp_a = QgsLimitedRandomColorRamp(5)
        self.assertTrue(style.addColorRamp("ramp a", ramp_a, True))
        self.assertEqual(model.rowCount(), 3)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "c"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp a"
        )

        format_a = QgsTextFormat()
        self.assertTrue(style.addTextFormat("format a", format_a, True))
        self.assertEqual(model.rowCount(), 4)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "c"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp a"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "format a"
        )

        settings_a = QgsPalLayerSettings()
        self.assertTrue(style.addLabelSettings("settings a", settings_a, True))
        self.assertEqual(model.rowCount(), 5)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "c"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp a"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "format a"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "settings a"
        )

        shape_a = QgsLegendPatchShape()
        self.assertTrue(style.addLegendPatchShape("shape a", shape_a, True))
        self.assertEqual(model.rowCount(), 6)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "c"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp a"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "format a"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "settings a"
        )
        self.assertEqual(
            model.data(model.index(5, 0), Qt.ItemDataRole.DisplayRole), "shape a"
        )

        symbol3d_a = Dummy3dSymbol()
        self.assertTrue(style.addSymbol3D("symbol3d a", symbol3d_a, True))
        self.assertEqual(model.rowCount(), 7)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "c"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp a"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "format a"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "settings a"
        )
        self.assertEqual(
            model.data(model.index(5, 0), Qt.ItemDataRole.DisplayRole), "shape a"
        )
        self.assertEqual(
            model.data(model.index(6, 0), Qt.ItemDataRole.DisplayRole), "symbol3d a"
        )

        symbol3d_B = Dummy3dSymbol()
        self.assertTrue(style.addSymbol3D("symbol3d c", symbol3d_B, True))
        self.assertEqual(model.rowCount(), 8)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "c"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp a"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "format a"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "settings a"
        )
        self.assertEqual(
            model.data(model.index(5, 0), Qt.ItemDataRole.DisplayRole), "shape a"
        )
        self.assertEqual(
            model.data(model.index(6, 0), Qt.ItemDataRole.DisplayRole), "symbol3d a"
        )
        self.assertEqual(
            model.data(model.index(7, 0), Qt.ItemDataRole.DisplayRole), "symbol3d c"
        )

        symbol3d_b = Dummy3dSymbol()
        self.assertTrue(style.addSymbol3D("symbol3d b", symbol3d_b, True))
        self.assertEqual(model.rowCount(), 9)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "c"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp a"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "format a"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "settings a"
        )
        self.assertEqual(
            model.data(model.index(5, 0), Qt.ItemDataRole.DisplayRole), "shape a"
        )
        self.assertEqual(
            model.data(model.index(6, 0), Qt.ItemDataRole.DisplayRole), "symbol3d a"
        )
        self.assertEqual(
            model.data(model.index(7, 0), Qt.ItemDataRole.DisplayRole), "symbol3d b"
        )
        self.assertEqual(
            model.data(model.index(8, 0), Qt.ItemDataRole.DisplayRole), "symbol3d c"
        )

        self.assertTrue(
            style.removeEntityByName(QgsStyle.StyleEntity.Symbol3DEntity, "symbol3d a")
        )
        self.assertEqual(model.rowCount(), 8)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "c"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp a"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "format a"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "settings a"
        )
        self.assertEqual(
            model.data(model.index(5, 0), Qt.ItemDataRole.DisplayRole), "shape a"
        )
        self.assertEqual(
            model.data(model.index(6, 0), Qt.ItemDataRole.DisplayRole), "symbol3d b"
        )
        self.assertEqual(
            model.data(model.index(7, 0), Qt.ItemDataRole.DisplayRole), "symbol3d c"
        )

        self.assertTrue(
            style.removeEntityByName(
                QgsStyle.StyleEntity.LegendPatchShapeEntity, "shape a"
            )
        )
        self.assertEqual(model.rowCount(), 7)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "c"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp a"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "format a"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "settings a"
        )
        self.assertEqual(
            model.data(model.index(5, 0), Qt.ItemDataRole.DisplayRole), "symbol3d b"
        )
        self.assertEqual(
            model.data(model.index(6, 0), Qt.ItemDataRole.DisplayRole), "symbol3d c"
        )

        self.assertTrue(
            style.removeEntityByName(
                QgsStyle.StyleEntity.LabelSettingsEntity, "settings a"
            )
        )
        self.assertEqual(model.rowCount(), 6)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "c"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp a"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "format a"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "symbol3d b"
        )
        self.assertEqual(
            model.data(model.index(5, 0), Qt.ItemDataRole.DisplayRole), "symbol3d c"
        )

        self.assertTrue(
            style.removeEntityByName(QgsStyle.StyleEntity.TextFormatEntity, "format a")
        )
        self.assertEqual(model.rowCount(), 5)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "c"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp a"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "symbol3d b"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "symbol3d c"
        )

        self.assertTrue(
            style.removeEntityByName(QgsStyle.StyleEntity.ColorrampEntity, "ramp a")
        )
        self.assertEqual(model.rowCount(), 4)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "c"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "symbol3d b"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "symbol3d c"
        )

        self.assertTrue(
            style.removeEntityByName(QgsStyle.StyleEntity.SymbolEntity, "c")
        )
        self.assertEqual(model.rowCount(), 3)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "symbol3d b"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "symbol3d c"
        )

    def test_renamed(self):
        style = QgsStyle()
        style.createMemoryDatabase()

        model = QgsStyleModel(style)
        symbol = createMarkerSymbol()
        self.assertTrue(style.addSymbol("a", symbol, True))
        symbol = createMarkerSymbol()
        self.assertTrue(style.addSymbol("c", symbol, True))
        ramp_a = QgsLimitedRandomColorRamp(5)
        self.assertTrue(style.addColorRamp("ramp a", ramp_a, True))
        symbol_B = QgsLimitedRandomColorRamp()
        self.assertTrue(style.addColorRamp("ramp c", symbol_B, True))
        format_a = QgsTextFormat()
        self.assertTrue(style.addTextFormat("format a", format_a, True))
        format_B = QgsTextFormat()
        self.assertTrue(style.addTextFormat("format c", format_B, True))
        settings_a = QgsPalLayerSettings()
        self.assertTrue(style.addLabelSettings("settings a", settings_a, True))
        settings_B = QgsPalLayerSettings()
        self.assertTrue(style.addLabelSettings("settings c", settings_B, True))
        shape_a = QgsLegendPatchShape()
        self.assertTrue(style.addLegendPatchShape("shape a", shape_a, True))
        shape_B = QgsLegendPatchShape()
        self.assertTrue(style.addLegendPatchShape("shape c", shape_B, True))
        symbol3d_a = Dummy3dSymbol()
        self.assertTrue(style.addSymbol3D("symbol3d a", symbol3d_a, True))
        symbol3d_B = Dummy3dSymbol()
        self.assertTrue(style.addSymbol3D("symbol3d c", symbol3d_B, True))

        self.assertEqual(model.rowCount(), 12)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "a"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "c"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp a"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "ramp c"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "format a"
        )
        self.assertEqual(
            model.data(model.index(5, 0), Qt.ItemDataRole.DisplayRole), "format c"
        )
        self.assertEqual(
            model.data(model.index(6, 0), Qt.ItemDataRole.DisplayRole), "settings a"
        )
        self.assertEqual(
            model.data(model.index(7, 0), Qt.ItemDataRole.DisplayRole), "settings c"
        )
        self.assertEqual(
            model.data(model.index(8, 0), Qt.ItemDataRole.DisplayRole), "shape a"
        )
        self.assertEqual(
            model.data(model.index(9, 0), Qt.ItemDataRole.DisplayRole), "shape c"
        )
        self.assertEqual(
            model.data(model.index(10, 0), Qt.ItemDataRole.DisplayRole), "symbol3d a"
        )
        self.assertEqual(
            model.data(model.index(11, 0), Qt.ItemDataRole.DisplayRole), "symbol3d c"
        )

        self.assertTrue(style.renameSymbol("a", "b"))
        self.assertEqual(model.rowCount(), 12)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "b"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "c"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp a"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "ramp c"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "format a"
        )
        self.assertEqual(
            model.data(model.index(5, 0), Qt.ItemDataRole.DisplayRole), "format c"
        )
        self.assertEqual(
            model.data(model.index(6, 0), Qt.ItemDataRole.DisplayRole), "settings a"
        )
        self.assertEqual(
            model.data(model.index(7, 0), Qt.ItemDataRole.DisplayRole), "settings c"
        )
        self.assertEqual(
            model.data(model.index(8, 0), Qt.ItemDataRole.DisplayRole), "shape a"
        )
        self.assertEqual(
            model.data(model.index(9, 0), Qt.ItemDataRole.DisplayRole), "shape c"
        )
        self.assertEqual(
            model.data(model.index(10, 0), Qt.ItemDataRole.DisplayRole), "symbol3d a"
        )
        self.assertEqual(
            model.data(model.index(11, 0), Qt.ItemDataRole.DisplayRole), "symbol3d c"
        )

        self.assertTrue(style.renameSymbol("b", "d"))
        self.assertEqual(model.rowCount(), 12)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "c"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "d"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp a"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "ramp c"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "format a"
        )
        self.assertEqual(
            model.data(model.index(5, 0), Qt.ItemDataRole.DisplayRole), "format c"
        )
        self.assertEqual(
            model.data(model.index(6, 0), Qt.ItemDataRole.DisplayRole), "settings a"
        )
        self.assertEqual(
            model.data(model.index(7, 0), Qt.ItemDataRole.DisplayRole), "settings c"
        )
        self.assertEqual(
            model.data(model.index(8, 0), Qt.ItemDataRole.DisplayRole), "shape a"
        )
        self.assertEqual(
            model.data(model.index(9, 0), Qt.ItemDataRole.DisplayRole), "shape c"
        )
        self.assertEqual(
            model.data(model.index(10, 0), Qt.ItemDataRole.DisplayRole), "symbol3d a"
        )
        self.assertEqual(
            model.data(model.index(11, 0), Qt.ItemDataRole.DisplayRole), "symbol3d c"
        )

        self.assertTrue(style.renameSymbol("d", "e"))
        self.assertEqual(model.rowCount(), 12)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "c"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "e"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp a"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "ramp c"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "format a"
        )
        self.assertEqual(
            model.data(model.index(5, 0), Qt.ItemDataRole.DisplayRole), "format c"
        )
        self.assertEqual(
            model.data(model.index(6, 0), Qt.ItemDataRole.DisplayRole), "settings a"
        )
        self.assertEqual(
            model.data(model.index(7, 0), Qt.ItemDataRole.DisplayRole), "settings c"
        )
        self.assertEqual(
            model.data(model.index(8, 0), Qt.ItemDataRole.DisplayRole), "shape a"
        )
        self.assertEqual(
            model.data(model.index(9, 0), Qt.ItemDataRole.DisplayRole), "shape c"
        )
        self.assertEqual(
            model.data(model.index(10, 0), Qt.ItemDataRole.DisplayRole), "symbol3d a"
        )
        self.assertEqual(
            model.data(model.index(11, 0), Qt.ItemDataRole.DisplayRole), "symbol3d c"
        )

        self.assertTrue(style.renameSymbol("c", "f"))
        self.assertEqual(model.rowCount(), 12)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "e"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "f"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp a"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "ramp c"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "format a"
        )
        self.assertEqual(
            model.data(model.index(5, 0), Qt.ItemDataRole.DisplayRole), "format c"
        )
        self.assertEqual(
            model.data(model.index(6, 0), Qt.ItemDataRole.DisplayRole), "settings a"
        )
        self.assertEqual(
            model.data(model.index(7, 0), Qt.ItemDataRole.DisplayRole), "settings c"
        )
        self.assertEqual(
            model.data(model.index(8, 0), Qt.ItemDataRole.DisplayRole), "shape a"
        )
        self.assertEqual(
            model.data(model.index(9, 0), Qt.ItemDataRole.DisplayRole), "shape c"
        )
        self.assertEqual(
            model.data(model.index(10, 0), Qt.ItemDataRole.DisplayRole), "symbol3d a"
        )
        self.assertEqual(
            model.data(model.index(11, 0), Qt.ItemDataRole.DisplayRole), "symbol3d c"
        )

        self.assertTrue(style.renameColorRamp("ramp a", "ramp b"))
        self.assertEqual(model.rowCount(), 12)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "e"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "f"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp b"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "ramp c"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "format a"
        )
        self.assertEqual(
            model.data(model.index(5, 0), Qt.ItemDataRole.DisplayRole), "format c"
        )
        self.assertEqual(
            model.data(model.index(6, 0), Qt.ItemDataRole.DisplayRole), "settings a"
        )
        self.assertEqual(
            model.data(model.index(7, 0), Qt.ItemDataRole.DisplayRole), "settings c"
        )
        self.assertEqual(
            model.data(model.index(8, 0), Qt.ItemDataRole.DisplayRole), "shape a"
        )
        self.assertEqual(
            model.data(model.index(9, 0), Qt.ItemDataRole.DisplayRole), "shape c"
        )
        self.assertEqual(
            model.data(model.index(10, 0), Qt.ItemDataRole.DisplayRole), "symbol3d a"
        )
        self.assertEqual(
            model.data(model.index(11, 0), Qt.ItemDataRole.DisplayRole), "symbol3d c"
        )

        self.assertTrue(style.renameColorRamp("ramp b", "ramp d"))
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "e"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "f"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp c"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "ramp d"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "format a"
        )
        self.assertEqual(
            model.data(model.index(5, 0), Qt.ItemDataRole.DisplayRole), "format c"
        )
        self.assertEqual(
            model.data(model.index(6, 0), Qt.ItemDataRole.DisplayRole), "settings a"
        )
        self.assertEqual(
            model.data(model.index(7, 0), Qt.ItemDataRole.DisplayRole), "settings c"
        )
        self.assertEqual(
            model.data(model.index(8, 0), Qt.ItemDataRole.DisplayRole), "shape a"
        )
        self.assertEqual(
            model.data(model.index(9, 0), Qt.ItemDataRole.DisplayRole), "shape c"
        )
        self.assertEqual(
            model.data(model.index(10, 0), Qt.ItemDataRole.DisplayRole), "symbol3d a"
        )
        self.assertEqual(
            model.data(model.index(11, 0), Qt.ItemDataRole.DisplayRole), "symbol3d c"
        )

        self.assertTrue(style.renameColorRamp("ramp d", "ramp e"))
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "e"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "f"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp c"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "ramp e"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "format a"
        )
        self.assertEqual(
            model.data(model.index(5, 0), Qt.ItemDataRole.DisplayRole), "format c"
        )
        self.assertEqual(
            model.data(model.index(6, 0), Qt.ItemDataRole.DisplayRole), "settings a"
        )
        self.assertEqual(
            model.data(model.index(7, 0), Qt.ItemDataRole.DisplayRole), "settings c"
        )
        self.assertEqual(
            model.data(model.index(8, 0), Qt.ItemDataRole.DisplayRole), "shape a"
        )
        self.assertEqual(
            model.data(model.index(9, 0), Qt.ItemDataRole.DisplayRole), "shape c"
        )
        self.assertEqual(
            model.data(model.index(10, 0), Qt.ItemDataRole.DisplayRole), "symbol3d a"
        )
        self.assertEqual(
            model.data(model.index(11, 0), Qt.ItemDataRole.DisplayRole), "symbol3d c"
        )

        self.assertTrue(style.renameColorRamp("ramp c", "ramp f"))
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "e"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "f"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp e"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "ramp f"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "format a"
        )
        self.assertEqual(
            model.data(model.index(5, 0), Qt.ItemDataRole.DisplayRole), "format c"
        )
        self.assertEqual(
            model.data(model.index(6, 0), Qt.ItemDataRole.DisplayRole), "settings a"
        )
        self.assertEqual(
            model.data(model.index(7, 0), Qt.ItemDataRole.DisplayRole), "settings c"
        )
        self.assertEqual(
            model.data(model.index(8, 0), Qt.ItemDataRole.DisplayRole), "shape a"
        )
        self.assertEqual(
            model.data(model.index(9, 0), Qt.ItemDataRole.DisplayRole), "shape c"
        )
        self.assertEqual(
            model.data(model.index(10, 0), Qt.ItemDataRole.DisplayRole), "symbol3d a"
        )
        self.assertEqual(
            model.data(model.index(11, 0), Qt.ItemDataRole.DisplayRole), "symbol3d c"
        )

        self.assertTrue(style.renameTextFormat("format a", "format b"))
        self.assertEqual(model.rowCount(), 12)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "e"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "f"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp e"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "ramp f"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "format b"
        )
        self.assertEqual(
            model.data(model.index(5, 0), Qt.ItemDataRole.DisplayRole), "format c"
        )
        self.assertEqual(
            model.data(model.index(6, 0), Qt.ItemDataRole.DisplayRole), "settings a"
        )
        self.assertEqual(
            model.data(model.index(7, 0), Qt.ItemDataRole.DisplayRole), "settings c"
        )
        self.assertEqual(
            model.data(model.index(8, 0), Qt.ItemDataRole.DisplayRole), "shape a"
        )
        self.assertEqual(
            model.data(model.index(9, 0), Qt.ItemDataRole.DisplayRole), "shape c"
        )
        self.assertEqual(
            model.data(model.index(10, 0), Qt.ItemDataRole.DisplayRole), "symbol3d a"
        )
        self.assertEqual(
            model.data(model.index(11, 0), Qt.ItemDataRole.DisplayRole), "symbol3d c"
        )

        self.assertTrue(style.renameTextFormat("format b", "format d"))
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "e"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "f"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp e"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "ramp f"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "format c"
        )
        self.assertEqual(
            model.data(model.index(5, 0), Qt.ItemDataRole.DisplayRole), "format d"
        )
        self.assertEqual(
            model.data(model.index(6, 0), Qt.ItemDataRole.DisplayRole), "settings a"
        )
        self.assertEqual(
            model.data(model.index(7, 0), Qt.ItemDataRole.DisplayRole), "settings c"
        )
        self.assertEqual(
            model.data(model.index(8, 0), Qt.ItemDataRole.DisplayRole), "shape a"
        )
        self.assertEqual(
            model.data(model.index(9, 0), Qt.ItemDataRole.DisplayRole), "shape c"
        )
        self.assertEqual(
            model.data(model.index(10, 0), Qt.ItemDataRole.DisplayRole), "symbol3d a"
        )
        self.assertEqual(
            model.data(model.index(11, 0), Qt.ItemDataRole.DisplayRole), "symbol3d c"
        )

        self.assertTrue(style.renameTextFormat("format d", "format e"))
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "e"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "f"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp e"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "ramp f"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "format c"
        )
        self.assertEqual(
            model.data(model.index(5, 0), Qt.ItemDataRole.DisplayRole), "format e"
        )
        self.assertEqual(
            model.data(model.index(6, 0), Qt.ItemDataRole.DisplayRole), "settings a"
        )
        self.assertEqual(
            model.data(model.index(7, 0), Qt.ItemDataRole.DisplayRole), "settings c"
        )
        self.assertEqual(
            model.data(model.index(8, 0), Qt.ItemDataRole.DisplayRole), "shape a"
        )
        self.assertEqual(
            model.data(model.index(9, 0), Qt.ItemDataRole.DisplayRole), "shape c"
        )
        self.assertEqual(
            model.data(model.index(10, 0), Qt.ItemDataRole.DisplayRole), "symbol3d a"
        )
        self.assertEqual(
            model.data(model.index(11, 0), Qt.ItemDataRole.DisplayRole), "symbol3d c"
        )

        self.assertTrue(style.renameTextFormat("format c", "format f"))
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "e"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "f"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp e"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "ramp f"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "format e"
        )
        self.assertEqual(
            model.data(model.index(5, 0), Qt.ItemDataRole.DisplayRole), "format f"
        )
        self.assertEqual(
            model.data(model.index(6, 0), Qt.ItemDataRole.DisplayRole), "settings a"
        )
        self.assertEqual(
            model.data(model.index(7, 0), Qt.ItemDataRole.DisplayRole), "settings c"
        )
        self.assertEqual(
            model.data(model.index(8, 0), Qt.ItemDataRole.DisplayRole), "shape a"
        )
        self.assertEqual(
            model.data(model.index(9, 0), Qt.ItemDataRole.DisplayRole), "shape c"
        )
        self.assertEqual(
            model.data(model.index(10, 0), Qt.ItemDataRole.DisplayRole), "symbol3d a"
        )
        self.assertEqual(
            model.data(model.index(11, 0), Qt.ItemDataRole.DisplayRole), "symbol3d c"
        )

        self.assertTrue(style.renameLabelSettings("settings a", "settings b"))
        self.assertEqual(model.rowCount(), 12)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "e"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "f"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp e"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "ramp f"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "format e"
        )
        self.assertEqual(
            model.data(model.index(5, 0), Qt.ItemDataRole.DisplayRole), "format f"
        )
        self.assertEqual(
            model.data(model.index(6, 0), Qt.ItemDataRole.DisplayRole), "settings b"
        )
        self.assertEqual(
            model.data(model.index(7, 0), Qt.ItemDataRole.DisplayRole), "settings c"
        )
        self.assertEqual(
            model.data(model.index(8, 0), Qt.ItemDataRole.DisplayRole), "shape a"
        )
        self.assertEqual(
            model.data(model.index(9, 0), Qt.ItemDataRole.DisplayRole), "shape c"
        )
        self.assertEqual(
            model.data(model.index(10, 0), Qt.ItemDataRole.DisplayRole), "symbol3d a"
        )
        self.assertEqual(
            model.data(model.index(11, 0), Qt.ItemDataRole.DisplayRole), "symbol3d c"
        )

        self.assertTrue(style.renameLabelSettings("settings b", "settings d"))
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "e"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "f"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp e"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "ramp f"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "format e"
        )
        self.assertEqual(
            model.data(model.index(5, 0), Qt.ItemDataRole.DisplayRole), "format f"
        )
        self.assertEqual(
            model.data(model.index(6, 0), Qt.ItemDataRole.DisplayRole), "settings c"
        )
        self.assertEqual(
            model.data(model.index(7, 0), Qt.ItemDataRole.DisplayRole), "settings d"
        )
        self.assertEqual(
            model.data(model.index(8, 0), Qt.ItemDataRole.DisplayRole), "shape a"
        )
        self.assertEqual(
            model.data(model.index(9, 0), Qt.ItemDataRole.DisplayRole), "shape c"
        )
        self.assertEqual(
            model.data(model.index(10, 0), Qt.ItemDataRole.DisplayRole), "symbol3d a"
        )
        self.assertEqual(
            model.data(model.index(11, 0), Qt.ItemDataRole.DisplayRole), "symbol3d c"
        )

        self.assertTrue(style.renameLabelSettings("settings d", "settings e"))
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "e"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "f"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp e"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "ramp f"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "format e"
        )
        self.assertEqual(
            model.data(model.index(5, 0), Qt.ItemDataRole.DisplayRole), "format f"
        )
        self.assertEqual(
            model.data(model.index(6, 0), Qt.ItemDataRole.DisplayRole), "settings c"
        )
        self.assertEqual(
            model.data(model.index(7, 0), Qt.ItemDataRole.DisplayRole), "settings e"
        )
        self.assertEqual(
            model.data(model.index(8, 0), Qt.ItemDataRole.DisplayRole), "shape a"
        )
        self.assertEqual(
            model.data(model.index(9, 0), Qt.ItemDataRole.DisplayRole), "shape c"
        )
        self.assertEqual(
            model.data(model.index(10, 0), Qt.ItemDataRole.DisplayRole), "symbol3d a"
        )
        self.assertEqual(
            model.data(model.index(11, 0), Qt.ItemDataRole.DisplayRole), "symbol3d c"
        )

        self.assertTrue(style.renameLabelSettings("settings c", "settings f"))
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "e"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "f"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp e"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "ramp f"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "format e"
        )
        self.assertEqual(
            model.data(model.index(5, 0), Qt.ItemDataRole.DisplayRole), "format f"
        )
        self.assertEqual(
            model.data(model.index(6, 0), Qt.ItemDataRole.DisplayRole), "settings e"
        )
        self.assertEqual(
            model.data(model.index(7, 0), Qt.ItemDataRole.DisplayRole), "settings f"
        )
        self.assertEqual(
            model.data(model.index(8, 0), Qt.ItemDataRole.DisplayRole), "shape a"
        )
        self.assertEqual(
            model.data(model.index(9, 0), Qt.ItemDataRole.DisplayRole), "shape c"
        )
        self.assertEqual(
            model.data(model.index(10, 0), Qt.ItemDataRole.DisplayRole), "symbol3d a"
        )
        self.assertEqual(
            model.data(model.index(11, 0), Qt.ItemDataRole.DisplayRole), "symbol3d c"
        )

        self.assertTrue(style.renameLegendPatchShape("shape a", "shape b"))
        self.assertEqual(model.rowCount(), 12)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "e"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "f"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp e"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "ramp f"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "format e"
        )
        self.assertEqual(
            model.data(model.index(5, 0), Qt.ItemDataRole.DisplayRole), "format f"
        )
        self.assertEqual(
            model.data(model.index(6, 0), Qt.ItemDataRole.DisplayRole), "settings e"
        )
        self.assertEqual(
            model.data(model.index(7, 0), Qt.ItemDataRole.DisplayRole), "settings f"
        )
        self.assertEqual(
            model.data(model.index(8, 0), Qt.ItemDataRole.DisplayRole), "shape b"
        )
        self.assertEqual(
            model.data(model.index(9, 0), Qt.ItemDataRole.DisplayRole), "shape c"
        )
        self.assertEqual(
            model.data(model.index(10, 0), Qt.ItemDataRole.DisplayRole), "symbol3d a"
        )
        self.assertEqual(
            model.data(model.index(11, 0), Qt.ItemDataRole.DisplayRole), "symbol3d c"
        )

        self.assertTrue(style.renameLegendPatchShape("shape b", "shape d"))
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "e"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "f"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp e"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "ramp f"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "format e"
        )
        self.assertEqual(
            model.data(model.index(5, 0), Qt.ItemDataRole.DisplayRole), "format f"
        )
        self.assertEqual(
            model.data(model.index(6, 0), Qt.ItemDataRole.DisplayRole), "settings e"
        )
        self.assertEqual(
            model.data(model.index(7, 0), Qt.ItemDataRole.DisplayRole), "settings f"
        )
        self.assertEqual(
            model.data(model.index(8, 0), Qt.ItemDataRole.DisplayRole), "shape c"
        )
        self.assertEqual(
            model.data(model.index(9, 0), Qt.ItemDataRole.DisplayRole), "shape d"
        )
        self.assertEqual(
            model.data(model.index(10, 0), Qt.ItemDataRole.DisplayRole), "symbol3d a"
        )
        self.assertEqual(
            model.data(model.index(11, 0), Qt.ItemDataRole.DisplayRole), "symbol3d c"
        )

        self.assertTrue(style.renameLegendPatchShape("shape d", "shape e"))
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "e"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "f"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp e"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "ramp f"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "format e"
        )
        self.assertEqual(
            model.data(model.index(5, 0), Qt.ItemDataRole.DisplayRole), "format f"
        )
        self.assertEqual(
            model.data(model.index(6, 0), Qt.ItemDataRole.DisplayRole), "settings e"
        )
        self.assertEqual(
            model.data(model.index(7, 0), Qt.ItemDataRole.DisplayRole), "settings f"
        )
        self.assertEqual(
            model.data(model.index(8, 0), Qt.ItemDataRole.DisplayRole), "shape c"
        )
        self.assertEqual(
            model.data(model.index(9, 0), Qt.ItemDataRole.DisplayRole), "shape e"
        )
        self.assertEqual(
            model.data(model.index(10, 0), Qt.ItemDataRole.DisplayRole), "symbol3d a"
        )
        self.assertEqual(
            model.data(model.index(11, 0), Qt.ItemDataRole.DisplayRole), "symbol3d c"
        )

        self.assertTrue(style.renameLegendPatchShape("shape c", "shape f"))
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "e"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "f"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp e"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "ramp f"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "format e"
        )
        self.assertEqual(
            model.data(model.index(5, 0), Qt.ItemDataRole.DisplayRole), "format f"
        )
        self.assertEqual(
            model.data(model.index(6, 0), Qt.ItemDataRole.DisplayRole), "settings e"
        )
        self.assertEqual(
            model.data(model.index(7, 0), Qt.ItemDataRole.DisplayRole), "settings f"
        )
        self.assertEqual(
            model.data(model.index(8, 0), Qt.ItemDataRole.DisplayRole), "shape e"
        )
        self.assertEqual(
            model.data(model.index(9, 0), Qt.ItemDataRole.DisplayRole), "shape f"
        )
        self.assertEqual(
            model.data(model.index(10, 0), Qt.ItemDataRole.DisplayRole), "symbol3d a"
        )
        self.assertEqual(
            model.data(model.index(11, 0), Qt.ItemDataRole.DisplayRole), "symbol3d c"
        )

        self.assertTrue(style.renameSymbol3D("symbol3d a", "symbol3d b"))
        self.assertEqual(model.rowCount(), 12)
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "e"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "f"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp e"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "ramp f"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "format e"
        )
        self.assertEqual(
            model.data(model.index(5, 0), Qt.ItemDataRole.DisplayRole), "format f"
        )
        self.assertEqual(
            model.data(model.index(6, 0), Qt.ItemDataRole.DisplayRole), "settings e"
        )
        self.assertEqual(
            model.data(model.index(7, 0), Qt.ItemDataRole.DisplayRole), "settings f"
        )
        self.assertEqual(
            model.data(model.index(8, 0), Qt.ItemDataRole.DisplayRole), "shape e"
        )
        self.assertEqual(
            model.data(model.index(9, 0), Qt.ItemDataRole.DisplayRole), "shape f"
        )
        self.assertEqual(
            model.data(model.index(10, 0), Qt.ItemDataRole.DisplayRole), "symbol3d b"
        )
        self.assertEqual(
            model.data(model.index(11, 0), Qt.ItemDataRole.DisplayRole), "symbol3d c"
        )

        self.assertTrue(style.renameSymbol3D("symbol3d b", "symbol3d d"))
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "e"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "f"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp e"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "ramp f"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "format e"
        )
        self.assertEqual(
            model.data(model.index(5, 0), Qt.ItemDataRole.DisplayRole), "format f"
        )
        self.assertEqual(
            model.data(model.index(6, 0), Qt.ItemDataRole.DisplayRole), "settings e"
        )
        self.assertEqual(
            model.data(model.index(7, 0), Qt.ItemDataRole.DisplayRole), "settings f"
        )
        self.assertEqual(
            model.data(model.index(8, 0), Qt.ItemDataRole.DisplayRole), "shape e"
        )
        self.assertEqual(
            model.data(model.index(9, 0), Qt.ItemDataRole.DisplayRole), "shape f"
        )
        self.assertEqual(
            model.data(model.index(10, 0), Qt.ItemDataRole.DisplayRole), "symbol3d c"
        )
        self.assertEqual(
            model.data(model.index(11, 0), Qt.ItemDataRole.DisplayRole), "symbol3d d"
        )

        self.assertTrue(style.renameSymbol3D("symbol3d d", "symbol3d e"))
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "e"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "f"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp e"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "ramp f"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "format e"
        )
        self.assertEqual(
            model.data(model.index(5, 0), Qt.ItemDataRole.DisplayRole), "format f"
        )
        self.assertEqual(
            model.data(model.index(6, 0), Qt.ItemDataRole.DisplayRole), "settings e"
        )
        self.assertEqual(
            model.data(model.index(7, 0), Qt.ItemDataRole.DisplayRole), "settings f"
        )
        self.assertEqual(
            model.data(model.index(8, 0), Qt.ItemDataRole.DisplayRole), "shape e"
        )
        self.assertEqual(
            model.data(model.index(9, 0), Qt.ItemDataRole.DisplayRole), "shape f"
        )
        self.assertEqual(
            model.data(model.index(10, 0), Qt.ItemDataRole.DisplayRole), "symbol3d c"
        )
        self.assertEqual(
            model.data(model.index(11, 0), Qt.ItemDataRole.DisplayRole), "symbol3d e"
        )

        self.assertTrue(style.renameSymbol3D("symbol3d c", "symbol3d f"))
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole), "e"
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "f"
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole), "ramp e"
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole), "ramp f"
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "format e"
        )
        self.assertEqual(
            model.data(model.index(5, 0), Qt.ItemDataRole.DisplayRole), "format f"
        )
        self.assertEqual(
            model.data(model.index(6, 0), Qt.ItemDataRole.DisplayRole), "settings e"
        )
        self.assertEqual(
            model.data(model.index(7, 0), Qt.ItemDataRole.DisplayRole), "settings f"
        )
        self.assertEqual(
            model.data(model.index(8, 0), Qt.ItemDataRole.DisplayRole), "shape e"
        )
        self.assertEqual(
            model.data(model.index(9, 0), Qt.ItemDataRole.DisplayRole), "shape f"
        )
        self.assertEqual(
            model.data(model.index(10, 0), Qt.ItemDataRole.DisplayRole), "symbol3d e"
        )
        self.assertEqual(
            model.data(model.index(11, 0), Qt.ItemDataRole.DisplayRole), "symbol3d f"
        )

    def test_tags_changed(self):
        style = QgsStyle()
        style.createMemoryDatabase()

        model = QgsStyleModel(style)
        symbol = createMarkerSymbol()
        self.assertTrue(style.addSymbol("a", symbol, True))
        symbol = createMarkerSymbol()
        self.assertTrue(style.addSymbol("c", symbol, True))
        ramp_a = QgsLimitedRandomColorRamp(5)
        self.assertTrue(style.addColorRamp("ramp a", ramp_a, True))
        symbol_B = QgsLimitedRandomColorRamp()
        self.assertTrue(style.addColorRamp("ramp c", symbol_B, True))
        format_a = QgsTextFormat()
        self.assertTrue(style.addTextFormat("format a", format_a, True))
        format_B = QgsTextFormat()
        self.assertTrue(style.addTextFormat("format c", format_B, True))
        format_a = QgsPalLayerSettings()
        self.assertTrue(style.addLabelSettings("settings a", format_a, True))
        format_B = QgsPalLayerSettings()
        self.assertTrue(style.addLabelSettings("settings c", format_B, True))
        shape_a = QgsLegendPatchShape()
        self.assertTrue(style.addLegendPatchShape("shape a", shape_a, True))
        shape_B = QgsLegendPatchShape()
        self.assertTrue(style.addLegendPatchShape("shape c", shape_B, True))
        symbol3d_a = Dummy3dSymbol()
        self.assertTrue(style.addSymbol3D("symbol3d a", symbol3d_a, True))
        symbol3d_B = Dummy3dSymbol()
        self.assertTrue(style.addSymbol3D("symbol3d c", symbol3d_B, True))

        self.assertFalse(model.data(model.index(0, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(1, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(2, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(3, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(4, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(5, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(6, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(7, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(8, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(9, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(10, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(11, 1), Qt.ItemDataRole.DisplayRole))

        style.tagSymbol(QgsStyle.StyleEntity.SymbolEntity, "a", ["t1", "t2"])
        self.assertEqual(
            model.data(model.index(0, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertFalse(model.data(model.index(1, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(2, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(3, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(4, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(5, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(6, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(7, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(8, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(9, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(10, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(11, 1), Qt.ItemDataRole.DisplayRole))

        style.tagSymbol(QgsStyle.StyleEntity.SymbolEntity, "a", ["t3"])
        self.assertEqual(
            model.data(model.index(0, 1), Qt.ItemDataRole.DisplayRole), "t1, t2, t3"
        )
        self.assertFalse(model.data(model.index(1, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(2, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(3, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(4, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(5, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(6, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(7, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(8, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(9, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(10, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(11, 1), Qt.ItemDataRole.DisplayRole))

        style.tagSymbol(QgsStyle.StyleEntity.ColorrampEntity, "ramp a", ["t1", "t2"])
        self.assertEqual(
            model.data(model.index(0, 1), Qt.ItemDataRole.DisplayRole), "t1, t2, t3"
        )
        self.assertFalse(model.data(model.index(1, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(2, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertFalse(model.data(model.index(3, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(4, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(5, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(6, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(7, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(8, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(9, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(10, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(11, 1), Qt.ItemDataRole.DisplayRole))

        style.tagSymbol(QgsStyle.StyleEntity.ColorrampEntity, "ramp c", ["t3"])
        self.assertEqual(
            model.data(model.index(0, 1), Qt.ItemDataRole.DisplayRole), "t1, t2, t3"
        )
        self.assertFalse(model.data(model.index(1, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(2, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertEqual(
            model.data(model.index(3, 1), Qt.ItemDataRole.DisplayRole), "t3"
        )
        self.assertFalse(model.data(model.index(4, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(5, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(6, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(7, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(8, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(9, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(10, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(11, 1), Qt.ItemDataRole.DisplayRole))

        style.tagSymbol(QgsStyle.StyleEntity.SymbolEntity, "c", ["t4"])
        self.assertEqual(
            model.data(model.index(0, 1), Qt.ItemDataRole.DisplayRole), "t1, t2, t3"
        )
        self.assertEqual(
            model.data(model.index(1, 1), Qt.ItemDataRole.DisplayRole), "t4"
        )
        self.assertEqual(
            model.data(model.index(2, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertEqual(
            model.data(model.index(3, 1), Qt.ItemDataRole.DisplayRole), "t3"
        )
        self.assertFalse(model.data(model.index(4, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(5, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(6, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(7, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(8, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(9, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(10, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(11, 1), Qt.ItemDataRole.DisplayRole))

        style.detagSymbol(QgsStyle.StyleEntity.SymbolEntity, "c", ["t4"])
        self.assertEqual(
            model.data(model.index(0, 1), Qt.ItemDataRole.DisplayRole), "t1, t2, t3"
        )
        self.assertFalse(model.data(model.index(1, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(2, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertEqual(
            model.data(model.index(3, 1), Qt.ItemDataRole.DisplayRole), "t3"
        )
        self.assertFalse(model.data(model.index(4, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(5, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(6, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(7, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(8, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(9, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(10, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(11, 1), Qt.ItemDataRole.DisplayRole))

        style.detagSymbol(QgsStyle.StyleEntity.ColorrampEntity, "ramp a", ["t1"])
        self.assertEqual(
            model.data(model.index(0, 1), Qt.ItemDataRole.DisplayRole), "t1, t2, t3"
        )
        self.assertFalse(model.data(model.index(1, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(2, 1), Qt.ItemDataRole.DisplayRole), "t2"
        )
        self.assertEqual(
            model.data(model.index(3, 1), Qt.ItemDataRole.DisplayRole), "t3"
        )
        self.assertFalse(model.data(model.index(4, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(5, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(6, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(7, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(8, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(9, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(10, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(11, 1), Qt.ItemDataRole.DisplayRole))

        style.tagSymbol(QgsStyle.StyleEntity.TextFormatEntity, "format a", ["t1", "t2"])
        self.assertEqual(
            model.data(model.index(0, 1), Qt.ItemDataRole.DisplayRole), "t1, t2, t3"
        )
        self.assertFalse(model.data(model.index(1, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(2, 1), Qt.ItemDataRole.DisplayRole), "t2"
        )
        self.assertEqual(
            model.data(model.index(3, 1), Qt.ItemDataRole.DisplayRole), "t3"
        )
        self.assertEqual(
            model.data(model.index(4, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertFalse(model.data(model.index(5, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(6, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(7, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(8, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(9, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(10, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(11, 1), Qt.ItemDataRole.DisplayRole))

        style.tagSymbol(QgsStyle.StyleEntity.TextFormatEntity, "format c", ["t3"])
        self.assertEqual(
            model.data(model.index(0, 1), Qt.ItemDataRole.DisplayRole), "t1, t2, t3"
        )
        self.assertFalse(model.data(model.index(1, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(2, 1), Qt.ItemDataRole.DisplayRole), "t2"
        )
        self.assertEqual(
            model.data(model.index(3, 1), Qt.ItemDataRole.DisplayRole), "t3"
        )
        self.assertEqual(
            model.data(model.index(4, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertEqual(
            model.data(model.index(5, 1), Qt.ItemDataRole.DisplayRole), "t3"
        )
        self.assertFalse(model.data(model.index(6, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(7, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(8, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(9, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(10, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(11, 1), Qt.ItemDataRole.DisplayRole))

        style.tagSymbol(QgsStyle.StyleEntity.TextFormatEntity, "c", ["t6"])
        self.assertEqual(
            model.data(model.index(0, 1), Qt.ItemDataRole.DisplayRole), "t1, t2, t3"
        )
        self.assertFalse(model.data(model.index(1, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(2, 1), Qt.ItemDataRole.DisplayRole), "t2"
        )
        self.assertEqual(
            model.data(model.index(3, 1), Qt.ItemDataRole.DisplayRole), "t3"
        )
        self.assertEqual(
            model.data(model.index(4, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertEqual(
            model.data(model.index(5, 1), Qt.ItemDataRole.DisplayRole), "t3"
        )
        self.assertFalse(model.data(model.index(6, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(7, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(8, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(9, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(10, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(11, 1), Qt.ItemDataRole.DisplayRole))

        style.detagSymbol(QgsStyle.StyleEntity.TextFormatEntity, "format c", ["t3"])
        self.assertEqual(
            model.data(model.index(0, 1), Qt.ItemDataRole.DisplayRole), "t1, t2, t3"
        )
        self.assertFalse(model.data(model.index(1, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(2, 1), Qt.ItemDataRole.DisplayRole), "t2"
        )
        self.assertEqual(
            model.data(model.index(3, 1), Qt.ItemDataRole.DisplayRole), "t3"
        )
        self.assertEqual(
            model.data(model.index(4, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertFalse(model.data(model.index(5, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(6, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(7, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(8, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(9, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(10, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(11, 1), Qt.ItemDataRole.DisplayRole))

        style.tagSymbol(
            QgsStyle.StyleEntity.LabelSettingsEntity, "settings a", ["t1", "t2"]
        )
        self.assertEqual(
            model.data(model.index(0, 1), Qt.ItemDataRole.DisplayRole), "t1, t2, t3"
        )
        self.assertFalse(model.data(model.index(1, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(2, 1), Qt.ItemDataRole.DisplayRole), "t2"
        )
        self.assertEqual(
            model.data(model.index(3, 1), Qt.ItemDataRole.DisplayRole), "t3"
        )
        self.assertEqual(
            model.data(model.index(4, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertFalse(model.data(model.index(5, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(6, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertFalse(model.data(model.index(7, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(8, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(9, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(10, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(11, 1), Qt.ItemDataRole.DisplayRole))

        style.tagSymbol(QgsStyle.StyleEntity.LabelSettingsEntity, "settings c", ["t3"])
        self.assertEqual(
            model.data(model.index(0, 1), Qt.ItemDataRole.DisplayRole), "t1, t2, t3"
        )
        self.assertFalse(model.data(model.index(1, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(2, 1), Qt.ItemDataRole.DisplayRole), "t2"
        )
        self.assertEqual(
            model.data(model.index(3, 1), Qt.ItemDataRole.DisplayRole), "t3"
        )
        self.assertEqual(
            model.data(model.index(4, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertFalse(model.data(model.index(5, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(6, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertEqual(
            model.data(model.index(7, 1), Qt.ItemDataRole.DisplayRole), "t3"
        )
        self.assertFalse(model.data(model.index(8, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(9, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(10, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(11, 1), Qt.ItemDataRole.DisplayRole))

        style.tagSymbol(QgsStyle.StyleEntity.LabelSettingsEntity, "c", ["t7"])
        self.assertEqual(
            model.data(model.index(0, 1), Qt.ItemDataRole.DisplayRole), "t1, t2, t3"
        )
        self.assertFalse(model.data(model.index(1, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(2, 1), Qt.ItemDataRole.DisplayRole), "t2"
        )
        self.assertEqual(
            model.data(model.index(3, 1), Qt.ItemDataRole.DisplayRole), "t3"
        )
        self.assertEqual(
            model.data(model.index(4, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertFalse(model.data(model.index(5, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(6, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertEqual(
            model.data(model.index(7, 1), Qt.ItemDataRole.DisplayRole), "t3"
        )
        self.assertFalse(model.data(model.index(8, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(9, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(10, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(11, 1), Qt.ItemDataRole.DisplayRole))

        style.detagSymbol(
            QgsStyle.StyleEntity.LabelSettingsEntity, "settings c", ["t3"]
        )
        self.assertEqual(
            model.data(model.index(0, 1), Qt.ItemDataRole.DisplayRole), "t1, t2, t3"
        )
        self.assertFalse(model.data(model.index(1, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(2, 1), Qt.ItemDataRole.DisplayRole), "t2"
        )
        self.assertEqual(
            model.data(model.index(3, 1), Qt.ItemDataRole.DisplayRole), "t3"
        )
        self.assertEqual(
            model.data(model.index(4, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertFalse(model.data(model.index(5, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(6, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertFalse(model.data(model.index(7, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(8, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(9, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(10, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(11, 1), Qt.ItemDataRole.DisplayRole))

        style.tagSymbol(
            QgsStyle.StyleEntity.LabelSettingsEntity, "settings a", ["t1", "t2"]
        )
        self.assertEqual(
            model.data(model.index(0, 1), Qt.ItemDataRole.DisplayRole), "t1, t2, t3"
        )
        self.assertFalse(model.data(model.index(1, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(2, 1), Qt.ItemDataRole.DisplayRole), "t2"
        )
        self.assertEqual(
            model.data(model.index(3, 1), Qt.ItemDataRole.DisplayRole), "t3"
        )
        self.assertEqual(
            model.data(model.index(4, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertFalse(model.data(model.index(5, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(6, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertFalse(model.data(model.index(7, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(8, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(9, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(10, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(11, 1), Qt.ItemDataRole.DisplayRole))

        style.tagSymbol(QgsStyle.StyleEntity.LabelSettingsEntity, "settings c", ["t3"])
        self.assertEqual(
            model.data(model.index(0, 1), Qt.ItemDataRole.DisplayRole), "t1, t2, t3"
        )
        self.assertFalse(model.data(model.index(1, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(2, 1), Qt.ItemDataRole.DisplayRole), "t2"
        )
        self.assertEqual(
            model.data(model.index(3, 1), Qt.ItemDataRole.DisplayRole), "t3"
        )
        self.assertEqual(
            model.data(model.index(4, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertFalse(model.data(model.index(5, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(6, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertEqual(
            model.data(model.index(7, 1), Qt.ItemDataRole.DisplayRole), "t3"
        )
        self.assertFalse(model.data(model.index(8, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(9, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(10, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(11, 1), Qt.ItemDataRole.DisplayRole))

        style.tagSymbol(QgsStyle.StyleEntity.LabelSettingsEntity, "c", ["t7"])
        self.assertEqual(
            model.data(model.index(0, 1), Qt.ItemDataRole.DisplayRole), "t1, t2, t3"
        )
        self.assertFalse(model.data(model.index(1, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(2, 1), Qt.ItemDataRole.DisplayRole), "t2"
        )
        self.assertEqual(
            model.data(model.index(3, 1), Qt.ItemDataRole.DisplayRole), "t3"
        )
        self.assertEqual(
            model.data(model.index(4, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertFalse(model.data(model.index(5, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(6, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertEqual(
            model.data(model.index(7, 1), Qt.ItemDataRole.DisplayRole), "t3"
        )
        self.assertFalse(model.data(model.index(8, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(9, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(10, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(11, 1), Qt.ItemDataRole.DisplayRole))

        style.detagSymbol(
            QgsStyle.StyleEntity.LabelSettingsEntity, "settings c", ["t3"]
        )
        self.assertEqual(
            model.data(model.index(0, 1), Qt.ItemDataRole.DisplayRole), "t1, t2, t3"
        )
        self.assertFalse(model.data(model.index(1, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(2, 1), Qt.ItemDataRole.DisplayRole), "t2"
        )
        self.assertEqual(
            model.data(model.index(3, 1), Qt.ItemDataRole.DisplayRole), "t3"
        )
        self.assertEqual(
            model.data(model.index(4, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertFalse(model.data(model.index(5, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(6, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertFalse(model.data(model.index(7, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(8, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(9, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(10, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(11, 1), Qt.ItemDataRole.DisplayRole))

        style.tagSymbol(
            QgsStyle.StyleEntity.LegendPatchShapeEntity, "shape a", ["t1", "t2"]
        )
        self.assertEqual(
            model.data(model.index(0, 1), Qt.ItemDataRole.DisplayRole), "t1, t2, t3"
        )
        self.assertFalse(model.data(model.index(1, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(2, 1), Qt.ItemDataRole.DisplayRole), "t2"
        )
        self.assertEqual(
            model.data(model.index(3, 1), Qt.ItemDataRole.DisplayRole), "t3"
        )
        self.assertEqual(
            model.data(model.index(4, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertFalse(model.data(model.index(5, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(6, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertFalse(model.data(model.index(7, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(8, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertFalse(model.data(model.index(9, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(10, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(11, 1), Qt.ItemDataRole.DisplayRole))

        style.tagSymbol(QgsStyle.StyleEntity.LegendPatchShapeEntity, "shape c", ["t3"])
        self.assertEqual(
            model.data(model.index(0, 1), Qt.ItemDataRole.DisplayRole), "t1, t2, t3"
        )
        self.assertFalse(model.data(model.index(1, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(2, 1), Qt.ItemDataRole.DisplayRole), "t2"
        )
        self.assertEqual(
            model.data(model.index(3, 1), Qt.ItemDataRole.DisplayRole), "t3"
        )
        self.assertEqual(
            model.data(model.index(4, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertFalse(model.data(model.index(5, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(6, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertFalse(model.data(model.index(7, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(8, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertEqual(
            model.data(model.index(9, 1), Qt.ItemDataRole.DisplayRole), "t3"
        )
        self.assertFalse(model.data(model.index(10, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(11, 1), Qt.ItemDataRole.DisplayRole))

        style.tagSymbol(QgsStyle.StyleEntity.LegendPatchShapeEntity, "c", ["t7"])
        self.assertEqual(
            model.data(model.index(0, 1), Qt.ItemDataRole.DisplayRole), "t1, t2, t3"
        )
        self.assertFalse(model.data(model.index(1, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(2, 1), Qt.ItemDataRole.DisplayRole), "t2"
        )
        self.assertEqual(
            model.data(model.index(3, 1), Qt.ItemDataRole.DisplayRole), "t3"
        )
        self.assertEqual(
            model.data(model.index(4, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertFalse(model.data(model.index(5, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(6, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertFalse(model.data(model.index(7, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(8, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertEqual(
            model.data(model.index(9, 1), Qt.ItemDataRole.DisplayRole), "t3"
        )
        self.assertFalse(model.data(model.index(10, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(11, 1), Qt.ItemDataRole.DisplayRole))

        style.detagSymbol(
            QgsStyle.StyleEntity.LegendPatchShapeEntity, "shape c", ["t3"]
        )
        self.assertEqual(
            model.data(model.index(0, 1), Qt.ItemDataRole.DisplayRole), "t1, t2, t3"
        )
        self.assertFalse(model.data(model.index(1, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(2, 1), Qt.ItemDataRole.DisplayRole), "t2"
        )
        self.assertEqual(
            model.data(model.index(3, 1), Qt.ItemDataRole.DisplayRole), "t3"
        )
        self.assertEqual(
            model.data(model.index(4, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertFalse(model.data(model.index(5, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(6, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertFalse(model.data(model.index(7, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(8, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertFalse(model.data(model.index(9, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(10, 1), Qt.ItemDataRole.DisplayRole))
        self.assertFalse(model.data(model.index(11, 1), Qt.ItemDataRole.DisplayRole))

        style.tagSymbol(QgsStyle.StyleEntity.Symbol3DEntity, "symbol3d a", ["t1", "t2"])
        self.assertEqual(
            model.data(model.index(0, 1), Qt.ItemDataRole.DisplayRole), "t1, t2, t3"
        )
        self.assertFalse(model.data(model.index(1, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(2, 1), Qt.ItemDataRole.DisplayRole), "t2"
        )
        self.assertEqual(
            model.data(model.index(3, 1), Qt.ItemDataRole.DisplayRole), "t3"
        )
        self.assertEqual(
            model.data(model.index(4, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertFalse(model.data(model.index(5, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(6, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertFalse(model.data(model.index(7, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(8, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertFalse(model.data(model.index(9, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(10, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertFalse(model.data(model.index(11, 1), Qt.ItemDataRole.DisplayRole))

        style.tagSymbol(QgsStyle.StyleEntity.Symbol3DEntity, "symbol3d c", ["t3"])
        self.assertEqual(
            model.data(model.index(0, 1), Qt.ItemDataRole.DisplayRole), "t1, t2, t3"
        )
        self.assertFalse(model.data(model.index(1, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(2, 1), Qt.ItemDataRole.DisplayRole), "t2"
        )
        self.assertEqual(
            model.data(model.index(3, 1), Qt.ItemDataRole.DisplayRole), "t3"
        )
        self.assertEqual(
            model.data(model.index(4, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertFalse(model.data(model.index(5, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(6, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertFalse(model.data(model.index(7, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(8, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertFalse(model.data(model.index(9, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(10, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertEqual(
            model.data(model.index(11, 1), Qt.ItemDataRole.DisplayRole), "t3"
        )

        style.tagSymbol(QgsStyle.StyleEntity.Symbol3DEntity, "c", ["t7"])
        self.assertEqual(
            model.data(model.index(0, 1), Qt.ItemDataRole.DisplayRole), "t1, t2, t3"
        )
        self.assertFalse(model.data(model.index(1, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(2, 1), Qt.ItemDataRole.DisplayRole), "t2"
        )
        self.assertEqual(
            model.data(model.index(3, 1), Qt.ItemDataRole.DisplayRole), "t3"
        )
        self.assertEqual(
            model.data(model.index(4, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertFalse(model.data(model.index(5, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(6, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertFalse(model.data(model.index(7, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(8, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertFalse(model.data(model.index(9, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(10, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertEqual(
            model.data(model.index(11, 1), Qt.ItemDataRole.DisplayRole), "t3"
        )

        style.detagSymbol(QgsStyle.StyleEntity.Symbol3DEntity, "symbol3d c", ["t3"])
        self.assertEqual(
            model.data(model.index(0, 1), Qt.ItemDataRole.DisplayRole), "t1, t2, t3"
        )
        self.assertFalse(model.data(model.index(1, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(2, 1), Qt.ItemDataRole.DisplayRole), "t2"
        )
        self.assertEqual(
            model.data(model.index(3, 1), Qt.ItemDataRole.DisplayRole), "t3"
        )
        self.assertEqual(
            model.data(model.index(4, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertFalse(model.data(model.index(5, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(6, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertFalse(model.data(model.index(7, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(8, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertFalse(model.data(model.index(9, 1), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            model.data(model.index(10, 1), Qt.ItemDataRole.DisplayRole), "t1, t2"
        )
        self.assertFalse(model.data(model.index(11, 1), Qt.ItemDataRole.DisplayRole))

    def test_filter_proxy(self):
        style = QgsStyle()
        style.createMemoryDatabase()

        symbol_a = createMarkerSymbol()
        symbol_a.setColor(QColor(255, 10, 10))
        self.assertTrue(style.addSymbol("a", symbol_a, True))
        style.tagSymbol(QgsStyle.StyleEntity.SymbolEntity, "a", ["tag 1", "tag 2"])
        symbol_B = createMarkerSymbol()
        symbol_B.setColor(QColor(10, 255, 10))
        self.assertTrue(style.addSymbol("BB", symbol_B, True))
        symbol_b = createFillSymbol()
        symbol_b.setColor(QColor(10, 255, 10))
        self.assertTrue(style.addSymbol("b", symbol_b, True))
        symbol_C = createLineSymbol()
        symbol_C.setColor(QColor(10, 255, 10))
        self.assertTrue(style.addSymbol("C", symbol_C, True))
        style.tagSymbol(QgsStyle.StyleEntity.SymbolEntity, "C", ["tag 3"])
        symbol_C = createMarkerSymbol()
        symbol_C.setColor(QColor(10, 255, 10))
        self.assertTrue(style.addSymbol("another", symbol_C, True))
        ramp_a = QgsLimitedRandomColorRamp(5)
        self.assertTrue(style.addColorRamp("ramp a", ramp_a, True))
        style.tagSymbol(
            QgsStyle.StyleEntity.ColorrampEntity, "ramp a", ["tag 1", "tag 2"]
        )
        symbol_B = QgsLimitedRandomColorRamp()
        self.assertTrue(style.addColorRamp("ramp BB", symbol_B, True))
        symbol_b = QgsLimitedRandomColorRamp()
        self.assertTrue(style.addColorRamp("ramp c", symbol_b, True))
        format_a = QgsTextFormat()
        self.assertTrue(style.addTextFormat("format a", format_a, True))
        style.tagSymbol(
            QgsStyle.StyleEntity.TextFormatEntity, "format a", ["tag 1", "tag 2"]
        )
        format_B = QgsTextFormat()
        self.assertTrue(style.addTextFormat("format BB", format_B, True))
        format_b = QgsTextFormat()
        self.assertTrue(style.addTextFormat("format c", format_b, True))
        format_a = QgsPalLayerSettings()
        format_a.layerType = QgsWkbTypes.GeometryType.PointGeometry
        self.assertTrue(style.addLabelSettings("settings a", format_a, True))
        style.tagSymbol(
            QgsStyle.StyleEntity.LabelSettingsEntity, "settings a", ["tag 1", "tag 2"]
        )
        format_B = QgsPalLayerSettings()
        format_B.layerType = QgsWkbTypes.GeometryType.LineGeometry
        self.assertTrue(style.addLabelSettings("settings BB", format_B, True))
        format_b = QgsPalLayerSettings()
        format_b.layerType = QgsWkbTypes.GeometryType.LineGeometry
        self.assertTrue(style.addLabelSettings("settings c", format_b, True))

        shape_a = QgsLegendPatchShape(
            QgsSymbol.SymbolType.Marker, QgsGeometry.fromWkt("Point( 4 5 )")
        )
        self.assertTrue(style.addLegendPatchShape("shape a", shape_a, True))
        style.tagSymbol(
            QgsStyle.StyleEntity.LegendPatchShapeEntity, "shape a", ["tag 1", "tag 2"]
        )
        shape_B = QgsLegendPatchShape(
            QgsSymbol.SymbolType.Line, QgsGeometry.fromWkt("LineString( 4 5, 6 5 )")
        )
        self.assertTrue(style.addLegendPatchShape("shape BB", shape_B, True))
        shape_b = QgsLegendPatchShape(
            QgsSymbol.SymbolType.Line, QgsGeometry.fromWkt("LineString( 14 5, 6 5 )")
        )
        self.assertTrue(style.addLegendPatchShape("shape c", shape_b, True))

        symbol3d_a = Dummy3dSymbol()
        self.assertTrue(style.addSymbol3D("sym3d a", symbol3d_a, True))
        style.tagSymbol(
            QgsStyle.StyleEntity.Symbol3DEntity, "sym3d a", ["tag 1", "tag 2"]
        )
        symbol3d_B = Dummy3dSymbol()
        symbol3d_B.layer_types = [QgsWkbTypes.GeometryType.PolygonGeometry]
        self.assertTrue(style.addSymbol3D("sym3d BB", symbol3d_B, True))
        symbol3d_B = Dummy3dSymbol()
        symbol3d_B.layer_types = [QgsWkbTypes.GeometryType.LineGeometry]
        self.assertTrue(style.addSymbol3D("sym3d c", symbol3d_B, True))

        model = QgsStyleProxyModel(style)
        self.assertEqual(model.rowCount(), 20)

        # filter string
        model.setFilterString("xx")
        self.assertEqual(model.filterString(), "xx")
        self.assertEqual(model.rowCount(), 0)
        model.setFilterString("b")
        self.assertEqual(model.rowCount(), 7)
        self.assertEqual(model.data(model.index(0, 0)), "b")
        self.assertEqual(model.data(model.index(1, 0)), "BB")
        self.assertEqual(model.data(model.index(2, 0)), "format BB")
        self.assertEqual(model.data(model.index(3, 0)), "ramp BB")
        self.assertEqual(model.data(model.index(4, 0)), "settings BB")
        self.assertEqual(model.data(model.index(5, 0)), "shape BB")
        self.assertEqual(model.data(model.index(6, 0)), "sym3d BB")
        model.setFilterString("bb")
        self.assertEqual(model.rowCount(), 6)
        self.assertEqual(model.data(model.index(0, 0)), "BB")
        self.assertEqual(model.data(model.index(1, 0)), "format BB")
        self.assertEqual(model.data(model.index(2, 0)), "ramp BB")
        self.assertEqual(model.data(model.index(3, 0)), "settings BB")
        self.assertEqual(model.data(model.index(4, 0)), "shape BB")
        self.assertEqual(model.data(model.index(5, 0)), "sym3d BB")
        model.setFilterString("tag 1")
        self.assertEqual(model.rowCount(), 6)
        self.assertEqual(model.data(model.index(0, 0)), "a")
        self.assertEqual(model.data(model.index(1, 0)), "format a")
        self.assertEqual(model.data(model.index(2, 0)), "ramp a")
        self.assertEqual(model.data(model.index(3, 0)), "settings a")
        self.assertEqual(model.data(model.index(4, 0)), "shape a")
        self.assertEqual(model.data(model.index(5, 0)), "sym3d a")
        model.setFilterString("TAG 1")
        self.assertEqual(model.rowCount(), 6)
        self.assertEqual(model.data(model.index(0, 0)), "a")
        self.assertEqual(model.data(model.index(1, 0)), "format a")
        self.assertEqual(model.data(model.index(2, 0)), "ramp a")
        self.assertEqual(model.data(model.index(3, 0)), "settings a")
        self.assertEqual(model.data(model.index(4, 0)), "shape a")
        self.assertEqual(model.data(model.index(5, 0)), "sym3d a")
        model.setFilterString("ram b")
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "ramp BB")
        model.setFilterString("mat b")
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "format BB")
        model.setFilterString("ta ram")  # match ta -> "tag 1", ram -> "ramp a"
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "ramp a")
        model.setFilterString("ta ings")
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "settings a")
        model.setFilterString("ta hap")
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "shape a")
        model.setFilterString("ta 3d")
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "sym3d a")
        model.setFilterString("")
        self.assertEqual(model.rowCount(), 20)

        # entity type
        model.setEntityFilter(QgsStyle.StyleEntity.SymbolEntity)
        self.assertEqual(model.rowCount(), 20)
        model.setEntityFilter(QgsStyle.StyleEntity.TextFormatEntity)
        self.assertEqual(model.rowCount(), 20)
        model.setEntityFilter(QgsStyle.StyleEntity.ColorrampEntity)
        self.assertEqual(model.rowCount(), 20)
        model.setEntityFilterEnabled(True)
        self.assertEqual(model.rowCount(), 3)
        self.assertEqual(model.data(model.index(0, 0)), "ramp a")
        self.assertEqual(model.data(model.index(1, 0)), "ramp BB")
        self.assertEqual(model.data(model.index(2, 0)), "ramp c")
        model.setFilterString("BB")
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "ramp BB")
        model.setFilterString("")

        model.setEntityFilter(QgsStyle.StyleEntity.SymbolEntity)
        self.assertEqual(model.rowCount(), 5)
        self.assertEqual(model.data(model.index(0, 0)), "a")
        self.assertEqual(model.data(model.index(1, 0)), "another")
        self.assertEqual(model.data(model.index(2, 0)), "b")
        self.assertEqual(model.data(model.index(3, 0)), "BB")
        self.assertEqual(model.data(model.index(4, 0)), "C")
        model.setFilterString("ot")
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "another")
        model.setFilterString("")

        model.setEntityFilter(QgsStyle.StyleEntity.TextFormatEntity)
        self.assertEqual(model.rowCount(), 3)
        self.assertEqual(model.data(model.index(0, 0)), "format a")
        self.assertEqual(model.data(model.index(1, 0)), "format BB")
        self.assertEqual(model.data(model.index(2, 0)), "format c")
        model.setFilterString("BB")
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "format BB")
        model.setFilterString("")

        model.setEntityFilter(QgsStyle.StyleEntity.LabelSettingsEntity)
        self.assertEqual(model.rowCount(), 3)
        self.assertEqual(model.data(model.index(0, 0)), "settings a")
        self.assertEqual(model.data(model.index(1, 0)), "settings BB")
        self.assertEqual(model.data(model.index(2, 0)), "settings c")
        model.setFilterString("BB")
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "settings BB")
        model.setFilterString("")

        model.setEntityFilter(QgsStyle.StyleEntity.LegendPatchShapeEntity)
        self.assertEqual(model.rowCount(), 3)
        self.assertEqual(model.data(model.index(0, 0)), "shape a")
        self.assertEqual(model.data(model.index(1, 0)), "shape BB")
        self.assertEqual(model.data(model.index(2, 0)), "shape c")
        model.setFilterString("BB")
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "shape BB")
        model.setFilterString("")

        model.setEntityFilter(QgsStyle.StyleEntity.Symbol3DEntity)
        self.assertEqual(model.rowCount(), 3)
        self.assertEqual(model.data(model.index(0, 0)), "sym3d a")
        self.assertEqual(model.data(model.index(1, 0)), "sym3d BB")
        self.assertEqual(model.data(model.index(2, 0)), "sym3d c")
        model.setFilterString("BB")
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "sym3d BB")
        model.setFilterString("")

        model.setEntityFilter(QgsStyle.StyleEntity.SymbolEntity)
        model.setEntityFilterEnabled(False)
        self.assertEqual(model.rowCount(), 20)

        # symbol type filter
        model.setSymbolType(QgsSymbol.SymbolType.Line)
        self.assertEqual(model.rowCount(), 20)
        model.setSymbolType(QgsSymbol.SymbolType.Marker)
        self.assertEqual(model.rowCount(), 20)
        model.setSymbolTypeFilterEnabled(True)
        self.assertEqual(model.rowCount(), 16)
        self.assertEqual(model.data(model.index(0, 0)), "a")
        self.assertEqual(model.data(model.index(1, 0)), "another")
        self.assertEqual(model.data(model.index(2, 0)), "BB")
        self.assertEqual(model.data(model.index(3, 0)), "format a")
        self.assertEqual(model.data(model.index(4, 0)), "format BB")
        self.assertEqual(model.data(model.index(5, 0)), "format c")
        self.assertEqual(model.data(model.index(6, 0)), "ramp a")
        self.assertEqual(model.data(model.index(7, 0)), "ramp BB")
        self.assertEqual(model.data(model.index(8, 0)), "ramp c")
        self.assertEqual(model.data(model.index(9, 0)), "settings a")
        self.assertEqual(model.data(model.index(10, 0)), "settings BB")
        self.assertEqual(model.data(model.index(11, 0)), "settings c")
        self.assertEqual(model.data(model.index(12, 0)), "shape a")
        self.assertEqual(model.data(model.index(13, 0)), "sym3d a")
        self.assertEqual(model.data(model.index(14, 0)), "sym3d BB")
        self.assertEqual(model.data(model.index(15, 0)), "sym3d c")

        model.setEntityFilterEnabled(True)
        self.assertEqual(model.rowCount(), 3)
        self.assertEqual(model.data(model.index(0, 0)), "a")
        self.assertEqual(model.data(model.index(1, 0)), "another")
        self.assertEqual(model.data(model.index(2, 0)), "BB")
        model.setFilterString("oth")
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "another")
        model.setEntityFilterEnabled(False)
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "another")
        model.setEntityFilterEnabled(True)
        model.setFilterString("")
        model.setSymbolType(QgsSymbol.SymbolType.Line)
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "C")
        model.setSymbolType(QgsSymbol.SymbolType.Fill)
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "b")

        model.setEntityFilter(QgsStyle.StyleEntity.LegendPatchShapeEntity)
        model.setSymbolType(QgsSymbol.SymbolType.Marker)
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "shape a")
        model.setSymbolType(QgsSymbol.SymbolType.Line)
        self.assertEqual(model.rowCount(), 2)
        self.assertEqual(model.data(model.index(0, 0)), "shape BB")
        self.assertEqual(model.data(model.index(1, 0)), "shape c")
        model.setFilterString("BB")
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "shape BB")

        model.setFilterString("")
        model.setSymbolTypeFilterEnabled(False)
        model.setEntityFilterEnabled(False)
        self.assertEqual(model.rowCount(), 20)

        # tag id filter
        self.assertEqual(model.tagId(), -1)
        tag_1_id = style.tagId("tag 1")
        tag_3_id = style.tagId("tag 3")
        model.setTagId(tag_1_id)
        self.assertEqual(model.tagId(), tag_1_id)
        self.assertEqual(model.rowCount(), 6)
        self.assertEqual(model.data(model.index(0, 0)), "a")
        self.assertEqual(model.data(model.index(1, 0)), "format a")
        self.assertEqual(model.data(model.index(2, 0)), "ramp a")
        self.assertEqual(model.data(model.index(3, 0)), "settings a")
        self.assertEqual(model.data(model.index(4, 0)), "shape a")
        self.assertEqual(model.data(model.index(5, 0)), "sym3d a")
        model.setEntityFilterEnabled(True)
        model.setEntityFilter(QgsStyle.StyleEntity.ColorrampEntity)
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "ramp a")
        model.setEntityFilter(QgsStyle.StyleEntity.TextFormatEntity)
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "format a")
        model.setEntityFilter(QgsStyle.StyleEntity.LabelSettingsEntity)
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "settings a")
        model.setEntityFilter(QgsStyle.StyleEntity.LegendPatchShapeEntity)
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "shape a")
        model.setEntityFilter(QgsStyle.StyleEntity.Symbol3DEntity)
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "sym3d a")
        model.setEntityFilterEnabled(False)
        model.setFilterString("ra")
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "ramp a")
        model.setFilterString("for")
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "format a")
        model.setFilterString("set")
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "settings a")
        model.setFilterString("hap")
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "shape a")
        model.setFilterString("3d")
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "sym3d a")
        model.setEntityFilterEnabled(False)
        model.setFilterString("")
        model.setTagId(-1)
        self.assertEqual(model.rowCount(), 20)
        model.setTagId(tag_3_id)
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "C")
        style.tagSymbol(QgsStyle.StyleEntity.ColorrampEntity, "ramp c", ["tag 3"])
        self.assertEqual(model.rowCount(), 2)
        self.assertEqual(model.data(model.index(0, 0)), "C")
        self.assertEqual(model.data(model.index(1, 0)), "ramp c")
        style.detagSymbol(QgsStyle.StyleEntity.ColorrampEntity, "ramp c")
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "C")
        style.tagSymbol(QgsStyle.StyleEntity.TextFormatEntity, "format c", ["tag 3"])
        self.assertEqual(model.rowCount(), 2)
        self.assertEqual(model.data(model.index(0, 0)), "C")
        self.assertEqual(model.data(model.index(1, 0)), "format c")
        style.detagSymbol(QgsStyle.StyleEntity.TextFormatEntity, "format c")
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "C")
        style.tagSymbol(
            QgsStyle.StyleEntity.LabelSettingsEntity, "settings c", ["tag 3"]
        )
        self.assertEqual(model.rowCount(), 2)
        self.assertEqual(model.data(model.index(0, 0)), "C")
        self.assertEqual(model.data(model.index(1, 0)), "settings c")
        style.detagSymbol(QgsStyle.StyleEntity.LabelSettingsEntity, "settings c")
        style.tagSymbol(
            QgsStyle.StyleEntity.LegendPatchShapeEntity, "shape c", ["tag 3"]
        )
        self.assertEqual(model.rowCount(), 2)
        self.assertEqual(model.data(model.index(0, 0)), "C")
        self.assertEqual(model.data(model.index(1, 0)), "shape c")
        style.detagSymbol(QgsStyle.StyleEntity.LegendPatchShapeEntity, "shape c")
        style.tagSymbol(QgsStyle.StyleEntity.Symbol3DEntity, "sym3d c", ["tag 3"])
        self.assertEqual(model.rowCount(), 2)
        self.assertEqual(model.data(model.index(0, 0)), "C")
        self.assertEqual(model.data(model.index(1, 0)), "sym3d c")
        style.detagSymbol(QgsStyle.StyleEntity.Symbol3DEntity, "sym3d c")
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "C")
        model.setTagId(-1)
        self.assertEqual(model.rowCount(), 20)

        # tag string filter
        self.assertFalse(model.tagString())
        model.setTagString("tag 1")
        self.assertEqual(model.tagString(), "tag 1")
        self.assertEqual(model.rowCount(), 6)
        self.assertEqual(model.data(model.index(0, 0)), "a")
        self.assertEqual(model.data(model.index(1, 0)), "format a")
        self.assertEqual(model.data(model.index(2, 0)), "ramp a")
        self.assertEqual(model.data(model.index(3, 0)), "settings a")
        self.assertEqual(model.data(model.index(4, 0)), "shape a")
        self.assertEqual(model.data(model.index(5, 0)), "sym3d a")
        model.setEntityFilterEnabled(True)
        model.setEntityFilter(QgsStyle.StyleEntity.ColorrampEntity)
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "ramp a")
        model.setEntityFilter(QgsStyle.StyleEntity.TextFormatEntity)
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "format a")
        model.setEntityFilter(QgsStyle.StyleEntity.LabelSettingsEntity)
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "settings a")
        model.setEntityFilter(QgsStyle.StyleEntity.LegendPatchShapeEntity)
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "shape a")
        model.setEntityFilter(QgsStyle.StyleEntity.Symbol3DEntity)
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "sym3d a")
        model.setEntityFilterEnabled(False)
        model.setFilterString("ra")
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "ramp a")
        model.setFilterString("for")
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "format a")
        model.setFilterString("set")
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "settings a")
        model.setFilterString("hap")
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "shape a")
        model.setFilterString("3d")
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "sym3d a")
        model.setEntityFilterEnabled(False)
        model.setFilterString("")
        model.setTagString("")
        self.assertEqual(model.rowCount(), 20)
        model.setTagString("tag 3")
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "C")
        style.tagSymbol(QgsStyle.StyleEntity.ColorrampEntity, "ramp c", ["tag 3"])
        self.assertEqual(model.rowCount(), 2)
        self.assertEqual(model.data(model.index(0, 0)), "C")
        self.assertEqual(model.data(model.index(1, 0)), "ramp c")
        style.detagSymbol(QgsStyle.StyleEntity.ColorrampEntity, "ramp c")
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "C")
        style.tagSymbol(QgsStyle.StyleEntity.TextFormatEntity, "format c", ["tag 3"])
        self.assertEqual(model.rowCount(), 2)
        self.assertEqual(model.data(model.index(0, 0)), "C")
        self.assertEqual(model.data(model.index(1, 0)), "format c")
        style.detagSymbol(QgsStyle.StyleEntity.TextFormatEntity, "format c")
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "C")
        style.tagSymbol(
            QgsStyle.StyleEntity.LabelSettingsEntity, "settings c", ["tag 3"]
        )
        self.assertEqual(model.rowCount(), 2)
        self.assertEqual(model.data(model.index(0, 0)), "C")
        self.assertEqual(model.data(model.index(1, 0)), "settings c")
        style.detagSymbol(QgsStyle.StyleEntity.LabelSettingsEntity, "settings c")
        style.tagSymbol(
            QgsStyle.StyleEntity.LegendPatchShapeEntity, "shape c", ["tag 3"]
        )
        self.assertEqual(model.rowCount(), 2)
        self.assertEqual(model.data(model.index(0, 0)), "C")
        self.assertEqual(model.data(model.index(1, 0)), "shape c")
        style.detagSymbol(QgsStyle.StyleEntity.LegendPatchShapeEntity, "shape c")
        style.tagSymbol(QgsStyle.StyleEntity.Symbol3DEntity, "sym3d c", ["tag 3"])
        self.assertEqual(model.rowCount(), 2)
        self.assertEqual(model.data(model.index(0, 0)), "C")
        self.assertEqual(model.data(model.index(1, 0)), "sym3d c")
        style.detagSymbol(QgsStyle.StyleEntity.Symbol3DEntity, "sym3d c")
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "C")
        model.setTagString("")
        self.assertEqual(model.rowCount(), 20)

        # favorite filter
        style.addFavorite(QgsStyle.StyleEntity.ColorrampEntity, "ramp c")
        style.addFavorite(QgsStyle.StyleEntity.SymbolEntity, "another")
        style.addFavorite(QgsStyle.StyleEntity.TextFormatEntity, "format c")
        style.addFavorite(QgsStyle.StyleEntity.LabelSettingsEntity, "settings c")
        style.addFavorite(QgsStyle.StyleEntity.LegendPatchShapeEntity, "shape c")
        style.addFavorite(QgsStyle.StyleEntity.Symbol3DEntity, "sym3d c")
        self.assertEqual(model.favoritesOnly(), False)
        model.setFavoritesOnly(True)
        self.assertEqual(model.favoritesOnly(), True)
        self.assertEqual(model.rowCount(), 6)
        self.assertEqual(model.data(model.index(0, 0)), "another")
        self.assertEqual(model.data(model.index(1, 0)), "format c")
        self.assertEqual(model.data(model.index(2, 0)), "ramp c")
        self.assertEqual(model.data(model.index(3, 0)), "settings c")
        self.assertEqual(model.data(model.index(4, 0)), "shape c")
        self.assertEqual(model.data(model.index(5, 0)), "sym3d c")
        model.setEntityFilterEnabled(True)
        model.setEntityFilter(QgsStyle.StyleEntity.ColorrampEntity)
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "ramp c")
        model.setEntityFilter(QgsStyle.StyleEntity.TextFormatEntity)
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "format c")
        model.setEntityFilter(QgsStyle.StyleEntity.LabelSettingsEntity)
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "settings c")
        model.setEntityFilter(QgsStyle.StyleEntity.LegendPatchShapeEntity)
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "shape c")
        model.setEntityFilter(QgsStyle.StyleEntity.Symbol3DEntity)
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "sym3d c")
        model.setEntityFilterEnabled(False)
        model.setFilterString("er")
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "another")
        model.setEntityFilterEnabled(False)
        model.setFilterString("")
        self.assertEqual(model.rowCount(), 6)
        style.addFavorite(QgsStyle.StyleEntity.ColorrampEntity, "ramp a")
        self.assertEqual(model.rowCount(), 7)
        self.assertEqual(model.data(model.index(0, 0)), "another")
        self.assertEqual(model.data(model.index(1, 0)), "format c")
        self.assertEqual(model.data(model.index(2, 0)), "ramp a")
        self.assertEqual(model.data(model.index(3, 0)), "ramp c")
        self.assertEqual(model.data(model.index(4, 0)), "settings c")
        self.assertEqual(model.data(model.index(5, 0)), "shape c")
        self.assertEqual(model.data(model.index(6, 0)), "sym3d c")
        style.removeFavorite(QgsStyle.StyleEntity.ColorrampEntity, "ramp a")
        self.assertEqual(model.rowCount(), 6)
        self.assertEqual(model.data(model.index(0, 0)), "another")
        self.assertEqual(model.data(model.index(1, 0)), "format c")
        self.assertEqual(model.data(model.index(2, 0)), "ramp c")
        self.assertEqual(model.data(model.index(3, 0)), "settings c")
        self.assertEqual(model.data(model.index(4, 0)), "shape c")
        self.assertEqual(model.data(model.index(5, 0)), "sym3d c")
        style.addFavorite(QgsStyle.StyleEntity.TextFormatEntity, "format a")
        self.assertEqual(model.rowCount(), 7)
        self.assertEqual(model.data(model.index(0, 0)), "another")
        self.assertEqual(model.data(model.index(1, 0)), "format a")
        self.assertEqual(model.data(model.index(2, 0)), "format c")
        self.assertEqual(model.data(model.index(3, 0)), "ramp c")
        self.assertEqual(model.data(model.index(4, 0)), "settings c")
        self.assertEqual(model.data(model.index(5, 0)), "shape c")
        self.assertEqual(model.data(model.index(6, 0)), "sym3d c")
        style.removeFavorite(QgsStyle.StyleEntity.TextFormatEntity, "format a")
        self.assertEqual(model.rowCount(), 6)
        self.assertEqual(model.data(model.index(0, 0)), "another")
        self.assertEqual(model.data(model.index(1, 0)), "format c")
        self.assertEqual(model.data(model.index(2, 0)), "ramp c")
        self.assertEqual(model.data(model.index(3, 0)), "settings c")
        self.assertEqual(model.data(model.index(4, 0)), "shape c")
        self.assertEqual(model.data(model.index(5, 0)), "sym3d c")
        style.addFavorite(QgsStyle.StyleEntity.LabelSettingsEntity, "settings a")
        self.assertEqual(model.rowCount(), 7)
        self.assertEqual(model.data(model.index(0, 0)), "another")
        self.assertEqual(model.data(model.index(1, 0)), "format c")
        self.assertEqual(model.data(model.index(2, 0)), "ramp c")
        self.assertEqual(model.data(model.index(3, 0)), "settings a")
        self.assertEqual(model.data(model.index(4, 0)), "settings c")
        self.assertEqual(model.data(model.index(5, 0)), "shape c")
        self.assertEqual(model.data(model.index(6, 0)), "sym3d c")
        style.removeFavorite(QgsStyle.StyleEntity.LabelSettingsEntity, "settings a")
        self.assertEqual(model.rowCount(), 6)
        self.assertEqual(model.data(model.index(0, 0)), "another")
        self.assertEqual(model.data(model.index(1, 0)), "format c")
        self.assertEqual(model.data(model.index(2, 0)), "ramp c")
        self.assertEqual(model.data(model.index(3, 0)), "settings c")
        self.assertEqual(model.data(model.index(4, 0)), "shape c")
        self.assertEqual(model.data(model.index(5, 0)), "sym3d c")
        style.addFavorite(QgsStyle.StyleEntity.LegendPatchShapeEntity, "shape a")
        self.assertEqual(model.rowCount(), 7)
        self.assertEqual(model.data(model.index(0, 0)), "another")
        self.assertEqual(model.data(model.index(1, 0)), "format c")
        self.assertEqual(model.data(model.index(2, 0)), "ramp c")
        self.assertEqual(model.data(model.index(3, 0)), "settings c")
        self.assertEqual(model.data(model.index(4, 0)), "shape a")
        self.assertEqual(model.data(model.index(5, 0)), "shape c")
        self.assertEqual(model.data(model.index(6, 0)), "sym3d c")
        style.removeFavorite(QgsStyle.StyleEntity.LegendPatchShapeEntity, "shape a")
        self.assertEqual(model.rowCount(), 6)
        self.assertEqual(model.data(model.index(0, 0)), "another")
        self.assertEqual(model.data(model.index(1, 0)), "format c")
        self.assertEqual(model.data(model.index(2, 0)), "ramp c")
        self.assertEqual(model.data(model.index(3, 0)), "settings c")
        self.assertEqual(model.data(model.index(4, 0)), "shape c")
        self.assertEqual(model.data(model.index(5, 0)), "sym3d c")
        style.addFavorite(QgsStyle.StyleEntity.Symbol3DEntity, "sym3d a")
        self.assertEqual(model.rowCount(), 7)
        self.assertEqual(model.data(model.index(0, 0)), "another")
        self.assertEqual(model.data(model.index(1, 0)), "format c")
        self.assertEqual(model.data(model.index(2, 0)), "ramp c")
        self.assertEqual(model.data(model.index(3, 0)), "settings c")
        self.assertEqual(model.data(model.index(4, 0)), "shape c")
        self.assertEqual(model.data(model.index(5, 0)), "sym3d a")
        self.assertEqual(model.data(model.index(6, 0)), "sym3d c")
        style.removeFavorite(QgsStyle.StyleEntity.Symbol3DEntity, "sym3d a")
        self.assertEqual(model.rowCount(), 6)
        self.assertEqual(model.data(model.index(0, 0)), "another")
        self.assertEqual(model.data(model.index(1, 0)), "format c")
        self.assertEqual(model.data(model.index(2, 0)), "ramp c")
        self.assertEqual(model.data(model.index(3, 0)), "settings c")
        self.assertEqual(model.data(model.index(4, 0)), "shape c")
        self.assertEqual(model.data(model.index(5, 0)), "sym3d c")

        style.addFavorite(QgsStyle.StyleEntity.SymbolEntity, "BB")
        self.assertEqual(model.rowCount(), 7)
        self.assertEqual(model.data(model.index(0, 0)), "another")
        self.assertEqual(model.data(model.index(1, 0)), "BB")
        self.assertEqual(model.data(model.index(2, 0)), "format c")
        self.assertEqual(model.data(model.index(3, 0)), "ramp c")
        self.assertEqual(model.data(model.index(4, 0)), "settings c")
        self.assertEqual(model.data(model.index(5, 0)), "shape c")
        self.assertEqual(model.data(model.index(6, 0)), "sym3d c")
        style.removeFavorite(QgsStyle.StyleEntity.SymbolEntity, "another")
        self.assertEqual(model.rowCount(), 6)
        self.assertEqual(model.data(model.index(0, 0)), "BB")
        self.assertEqual(model.data(model.index(1, 0)), "format c")
        self.assertEqual(model.data(model.index(2, 0)), "ramp c")
        self.assertEqual(model.data(model.index(3, 0)), "settings c")
        self.assertEqual(model.data(model.index(4, 0)), "shape c")
        self.assertEqual(model.data(model.index(5, 0)), "sym3d c")
        model.setFavoritesOnly(False)
        self.assertEqual(model.rowCount(), 20)

        # smart group filter
        style.addSmartgroup("smart", "AND", ["tag 3"], [], ["c"], [])
        self.assertEqual(model.smartGroupId(), -1)
        model.setSmartGroupId(1)
        self.assertEqual(model.smartGroupId(), 1)
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "C")
        style.addSmartgroup("smart", "OR", ["tag 3"], [], ["c"], [])
        model.setSmartGroupId(2)
        self.assertEqual(model.rowCount(), 6)
        self.assertEqual(model.data(model.index(0, 0)), "C")
        self.assertEqual(model.data(model.index(1, 0)), "format c")
        self.assertEqual(model.data(model.index(2, 0)), "ramp c")
        self.assertEqual(model.data(model.index(3, 0)), "settings c")
        self.assertEqual(model.data(model.index(4, 0)), "shape c")
        self.assertEqual(model.data(model.index(5, 0)), "sym3d c")
        style.tagSymbol(QgsStyle.StyleEntity.SymbolEntity, "a", ["tag 3"])
        self.assertEqual(model.rowCount(), 7)
        self.assertEqual(model.data(model.index(0, 0)), "a")
        self.assertEqual(model.data(model.index(1, 0)), "C")
        self.assertEqual(model.data(model.index(2, 0)), "format c")
        self.assertEqual(model.data(model.index(3, 0)), "ramp c")
        self.assertEqual(model.data(model.index(4, 0)), "settings c")
        self.assertEqual(model.data(model.index(5, 0)), "shape c")
        self.assertEqual(model.data(model.index(6, 0)), "sym3d c")
        style.renameColorRamp("ramp c", "x")
        self.assertEqual(model.rowCount(), 6)
        self.assertEqual(model.data(model.index(0, 0)), "a")
        self.assertEqual(model.data(model.index(1, 0)), "C")
        self.assertEqual(model.data(model.index(2, 0)), "format c")
        self.assertEqual(model.data(model.index(3, 0)), "settings c")
        self.assertEqual(model.data(model.index(4, 0)), "shape c")
        self.assertEqual(model.data(model.index(5, 0)), "sym3d c")
        style.renameTextFormat("format c", "x")
        self.assertEqual(model.rowCount(), 5)
        self.assertEqual(model.data(model.index(0, 0)), "a")
        self.assertEqual(model.data(model.index(1, 0)), "C")
        self.assertEqual(model.data(model.index(2, 0)), "settings c")
        self.assertEqual(model.data(model.index(3, 0)), "shape c")
        self.assertEqual(model.data(model.index(4, 0)), "sym3d c")
        style.renameLabelSettings("settings c", "x")
        self.assertEqual(model.rowCount(), 4)
        self.assertEqual(model.data(model.index(0, 0)), "a")
        self.assertEqual(model.data(model.index(1, 0)), "C")
        self.assertEqual(model.data(model.index(2, 0)), "shape c")
        self.assertEqual(model.data(model.index(3, 0)), "sym3d c")
        style.renameLegendPatchShape("shape c", "x")
        self.assertEqual(model.rowCount(), 3)
        self.assertEqual(model.data(model.index(0, 0)), "a")
        self.assertEqual(model.data(model.index(1, 0)), "C")
        self.assertEqual(model.data(model.index(2, 0)), "sym3d c")
        style.renameSymbol3D("sym3d c", "x")
        self.assertEqual(model.rowCount(), 2)
        self.assertEqual(model.data(model.index(0, 0)), "a")
        self.assertEqual(model.data(model.index(1, 0)), "C")
        model.setSmartGroupId(-1)
        self.assertEqual(model.rowCount(), 20)

        model.setEntityFilter(QgsStyle.StyleEntity.LabelSettingsEntity)
        model.setEntityFilterEnabled(True)
        self.assertEqual(model.rowCount(), 3)
        model.setLayerType(QgsWkbTypes.GeometryType.PointGeometry)
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "settings a")
        model.setLayerType(QgsWkbTypes.GeometryType.LineGeometry)
        self.assertEqual(model.rowCount(), 2)
        self.assertEqual(model.data(model.index(0, 0)), "settings BB")
        self.assertEqual(model.data(model.index(1, 0)), "x")
        model.setLayerType(QgsWkbTypes.GeometryType.PolygonGeometry)
        self.assertEqual(model.rowCount(), 0)
        model.setLayerType(QgsWkbTypes.GeometryType.UnknownGeometry)
        self.assertEqual(model.rowCount(), 3)

        model.setEntityFilter(QgsStyle.StyleEntity.Symbol3DEntity)
        model.setEntityFilterEnabled(True)
        self.assertEqual(model.rowCount(), 3)
        model.setLayerType(QgsWkbTypes.GeometryType.PointGeometry)
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "sym3d a")
        model.setLayerType(QgsWkbTypes.GeometryType.LineGeometry)
        self.assertEqual(model.rowCount(), 2)
        self.assertEqual(model.data(model.index(0, 0)), "sym3d a")
        self.assertEqual(model.data(model.index(1, 0)), "x")
        model.setLayerType(QgsWkbTypes.GeometryType.PolygonGeometry)
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0)), "sym3d BB")
        model.setLayerType(QgsWkbTypes.GeometryType.UnknownGeometry)
        self.assertEqual(model.rowCount(), 3)

    def testIconSize(self):
        """
        Test that model has responsive icon sizes for decorations
        """
        style = QgsStyle()
        style.createMemoryDatabase()

        symbol_a = createMarkerSymbol()
        symbol_a.setColor(QColor(255, 10, 10))
        self.assertTrue(style.addSymbol("a", symbol_a, True))
        ramp_a = QgsLimitedRandomColorRamp(5)
        self.assertTrue(style.addColorRamp("ramp a", ramp_a, True))
        format_a = QgsTextFormat()
        self.assertTrue(style.addTextFormat("format a", format_a, True))
        settings_a = QgsPalLayerSettings()
        self.assertTrue(style.addLabelSettings("settings a", settings_a, True))
        shape_a = QgsLegendPatchShape(
            QgsSymbol.SymbolType.Marker, QgsGeometry.fromWkt("Point ( 4 5 )")
        )
        self.assertTrue(style.addLegendPatchShape("shape a", shape_a, True))
        symbol3d_a = Dummy3dSymbol()
        self.assertTrue(style.addSymbol3D("symbol3d a", symbol3d_a, True))

        # TODO - should be 6 when 3d symbols get an icon
        for i in range(5):
            model = QgsStyleModel(style)
            self.assertEqual(model.rowCount(), 6)
            icon = model.data(model.index(i, 0), Qt.ItemDataRole.DecorationRole)
            # by default, only 24x24 icon
            self.assertEqual(icon.availableSizes(), [QSize(24, 24)])
            self.assertEqual(icon.actualSize(QSize(10, 10)), QSize(10, 10))
            self.assertEqual(icon.actualSize(QSize(24, 24)), QSize(24, 24))
            self.assertEqual(icon.actualSize(QSize(90, 90)), QSize(24, 24))

            model.addDesiredIconSize(QSize(24, 24))
            model.addDesiredIconSize(QSize(100, 90))
            icon = model.data(model.index(i, 0), Qt.ItemDataRole.DecorationRole)
            self.assertEqual(icon.availableSizes(), [QSize(24, 24), QSize(100, 90)])
            self.assertEqual(icon.actualSize(QSize(10, 10)), QSize(10, 10))
            self.assertEqual(icon.actualSize(QSize(24, 24)), QSize(24, 24))
            self.assertEqual(icon.actualSize(QSize(25, 25)), QSize(25, 22))
            self.assertEqual(icon.actualSize(QSize(90, 90)), QSize(90, 81))
            self.assertEqual(icon.actualSize(QSize(125, 125)), QSize(100, 90))

            model = QgsStyleModel(style)
            model.addDesiredIconSize(QSize(100, 90))
            model.addDesiredIconSize(QSize(200, 180))
            icon = model.data(model.index(i, 0), Qt.ItemDataRole.DecorationRole)
            self.assertEqual(icon.availableSizes(), [QSize(100, 90), QSize(200, 180)])
            self.assertEqual(icon.actualSize(QSize(24, 24)), QSize(24, 21))
            self.assertEqual(icon.actualSize(QSize(25, 25)), QSize(25, 22))
            self.assertEqual(icon.actualSize(QSize(90, 90)), QSize(90, 81))
            self.assertEqual(icon.actualSize(QSize(125, 125)), QSize(125, 112))
            self.assertEqual(icon.actualSize(QSize(225, 225)), QSize(200, 180))

    def testSetData(self):
        """
        Test model set data
        """
        style = QgsStyle()
        style.createMemoryDatabase()

        symbol_a = createMarkerSymbol()
        symbol_a.setColor(QColor(255, 10, 10))
        self.assertTrue(style.addSymbol("a", symbol_a, True))
        ramp_a = QgsLimitedRandomColorRamp(5)
        self.assertTrue(style.addColorRamp("ramp a", ramp_a, True))
        format_a = QgsTextFormat()
        self.assertTrue(style.addTextFormat("format a", format_a, True))
        settings_a = QgsPalLayerSettings()
        self.assertTrue(style.addLabelSettings("settings a", settings_a, True))
        shape_a = QgsLegendPatchShape(
            QgsSymbol.SymbolType.Marker, QgsGeometry.fromWkt("Point ( 4 5 )")
        )
        self.assertTrue(style.addLegendPatchShape("shape a", shape_a, True))
        symbol3d_a = Dummy3dSymbol()
        self.assertTrue(style.addSymbol3D("symbol3d a", symbol3d_a, True))

        model = QgsStyleModel(style)
        self.assertEqual(model.rowCount(), 6)

        self.assertEqual(style.symbolNames(), ["a"])
        self.assertFalse(model.setData(QModelIndex(), "b", Qt.ItemDataRole.EditRole))
        self.assertFalse(
            model.setData(model.index(0, 1), "b", Qt.ItemDataRole.EditRole)
        )
        self.assertTrue(
            model.setData(
                model.index(0, 0), "new symbol name", Qt.ItemDataRole.EditRole
            )
        )
        self.assertEqual(
            model.data(model.index(0, 0), Qt.ItemDataRole.DisplayRole),
            "new symbol name",
        )

        self.assertEqual(style.symbolNames(), ["new symbol name"])
        self.assertTrue(
            model.setData(model.index(1, 0), "ramp new name", Qt.ItemDataRole.EditRole)
        )
        self.assertEqual(
            model.data(model.index(1, 0), Qt.ItemDataRole.DisplayRole), "ramp new name"
        )
        self.assertEqual(style.colorRampNames(), ["ramp new name"])

        self.assertTrue(
            model.setData(
                model.index(2, 0), "format new name", Qt.ItemDataRole.EditRole
            )
        )
        self.assertEqual(
            model.data(model.index(2, 0), Qt.ItemDataRole.DisplayRole),
            "format new name",
        )
        self.assertEqual(style.textFormatNames(), ["format new name"])

        self.assertTrue(
            model.setData(
                model.index(3, 0), "settings new name", Qt.ItemDataRole.EditRole
            )
        )
        self.assertEqual(
            model.data(model.index(3, 0), Qt.ItemDataRole.DisplayRole),
            "settings new name",
        )
        self.assertEqual(style.labelSettingsNames(), ["settings new name"])

        self.assertTrue(
            model.setData(model.index(4, 0), "shape new name", Qt.ItemDataRole.EditRole)
        )
        self.assertEqual(
            model.data(model.index(4, 0), Qt.ItemDataRole.DisplayRole), "shape new name"
        )
        self.assertEqual(style.legendPatchShapeNames(), ["shape new name"])

        self.assertTrue(
            model.setData(
                model.index(5, 0), "symbol3d new name", Qt.ItemDataRole.EditRole
            )
        )
        self.assertEqual(
            model.data(model.index(5, 0), Qt.ItemDataRole.DisplayRole),
            "symbol3d new name",
        )
        self.assertEqual(style.symbol3DNames(), ["symbol3d new name"])

    def test_reset_symbollayer_ids(self):
        """
        Test that we have different symbol layer ids every time we get symbol from style
        """
        style = QgsStyle()
        style.createMemoryDatabase()

        layer = QgsLinePatternFillSymbolLayer()
        fill_symbol = QgsFillSymbol([layer])

        self.assertEqual(len(fill_symbol.symbolLayers()), 1)
        subsymbol = fill_symbol.symbolLayers()[0].subSymbol()
        self.assertTrue(subsymbol)
        self.assertEqual(len(subsymbol.symbolLayers()), 1)
        child_sl = subsymbol.symbolLayers()[0]
        self.assertTrue(child_sl)
        old_id = child_sl.id()
        self.assertTrue(child_sl.id())

        self.assertTrue(style.addSymbol("fillsymbol", fill_symbol, True))

        new_fill_symbol = style.symbol("fillsymbol")
        self.assertEqual(len(new_fill_symbol.symbolLayers()), 1)
        subsymbol = new_fill_symbol.symbolLayers()[0].subSymbol()
        self.assertTrue(subsymbol)
        self.assertEqual(len(subsymbol.symbolLayers()), 1)
        child_sl = subsymbol.symbolLayers()[0]
        self.assertTrue(child_sl)
        self.assertTrue(child_sl.id())
        self.assertTrue(child_sl.id() != old_id)


if __name__ == "__main__":
    unittest.main()
