"""QGIS Unit tests for QgsTextFormat.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Mathieu Pellerin"
__date__ = "2024-10-20"
__copyright__ = "Copyright 2024, The QGIS Project"

from qgis.PyQt.QtCore import Qt, QPointF, QSizeF, QT_VERSION_STR
from qgis.PyQt.QtGui import QFont, QColor, QPainter
from qgis.PyQt.QtXml import (
    QDomDocument,
)
from qgis.core import (
    Qgis,
    QgsMapUnitScale,
    QgsTextFormat,
    QgsProperty,
    QgsPalLayerSettings,
    QgsReadWriteContext,
    QgsTextBufferSettings,
    QgsTextMaskSettings,
    QgsSymbolLayerReference,
    QgsTextBackgroundSettings,
    QgsMarkerSymbol,
    QgsTextShadowSettings,
    QgsRenderContext,
    QgsFontUtils,
    QgsVectorLayer,
)
import unittest
from qgis.testing import start_app, QgisTestCase
from utilities import getTestFont

start_app()


def createEmptyLayer():
    layer = QgsVectorLayer("Point", "addfeat", "memory")
    assert layer.isValid()
    return layer


class PyQgsTextFormat(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        QgsFontUtils.loadStandardTestFonts(["Bold", "Oblique"])

    def testValid(self):
        t = QgsTextFormat()
        self.assertFalse(t.isValid())

        tt = QgsTextFormat(t)
        self.assertFalse(tt.isValid())

        t.setValid()
        self.assertTrue(t.isValid())
        tt = QgsTextFormat(t)
        self.assertTrue(tt.isValid())

        doc = QDomDocument()
        elem = t.writeXml(doc, QgsReadWriteContext())
        parent = doc.createElement("settings")
        parent.appendChild(elem)
        t3 = QgsTextFormat()
        t3.readXml(parent, QgsReadWriteContext())
        self.assertTrue(t3.isValid())

        t = QgsTextFormat()
        t.buffer().setEnabled(True)
        self.assertTrue(t.isValid())

        t = QgsTextFormat()
        t.background().setEnabled(True)
        self.assertTrue(t.isValid())

        t = QgsTextFormat()
        t.shadow().setEnabled(True)
        self.assertTrue(t.isValid())

        t = QgsTextFormat()
        t.mask().setEnabled(True)
        self.assertTrue(t.isValid())

        t = QgsTextFormat()
        t.font()
        self.assertFalse(t.isValid())
        t.setFont(QFont())
        self.assertTrue(t.isValid())

        t = QgsTextFormat()
        t.setNamedStyle("Bold")
        self.assertTrue(t.isValid())

        t = QgsTextFormat()
        t.setSize(20)
        self.assertTrue(t.isValid())

        t = QgsTextFormat()
        t.setSizeUnit(Qgis.RenderUnit.Pixels)
        self.assertTrue(t.isValid())

        t = QgsTextFormat()
        t.setSizeMapUnitScale(QgsMapUnitScale(5, 10))
        self.assertTrue(t.isValid())

        t = QgsTextFormat()
        t.setColor(QColor(255, 0, 0))
        self.assertTrue(t.isValid())

        t = QgsTextFormat()
        t.setOpacity(0.2)
        self.assertTrue(t.isValid())

        t = QgsTextFormat()
        t.setBlendMode(QPainter.CompositionMode.CompositionMode_Darken)
        self.assertTrue(t.isValid())

        t = QgsTextFormat()
        t.setLineHeight(20)
        self.assertTrue(t.isValid())

        t = QgsTextFormat()
        t.setLineHeightUnit(Qgis.RenderUnit.Points)
        self.assertTrue(t.isValid())

        t = QgsTextFormat()
        t.setTabStopDistance(3)
        self.assertTrue(t.isValid())

        t = QgsTextFormat()
        t.setTabPositions([QgsTextFormat.Tab(4), QgsTextFormat.Tab(8)])
        self.assertTrue(t.isValid())

        t = QgsTextFormat()
        t.setTabStopDistanceUnit(Qgis.RenderUnit.Points)
        self.assertTrue(t.isValid())

        t = QgsTextFormat()
        t.setTabStopDistanceMapUnitScale(QgsMapUnitScale(5, 10))
        self.assertTrue(t.isValid())

        t = QgsTextFormat()
        t.setOrientation(QgsTextFormat.TextOrientation.VerticalOrientation)
        self.assertTrue(t.isValid())

        t = QgsTextFormat()
        t.setCapitalization(Qgis.Capitalization.TitleCase)
        self.assertTrue(t.isValid())

        t = QgsTextFormat()
        t.setAllowHtmlFormatting(True)
        self.assertTrue(t.isValid())

        t = QgsTextFormat()
        t.setPreviewBackgroundColor(QColor(255, 0, 0))
        self.assertTrue(t.isValid())

        t = QgsTextFormat()
        t.setFamilies(["Arial", "Comic Sans"])
        self.assertTrue(t.isValid())

        t = QgsTextFormat()
        t.setStretchFactor(110)
        self.assertTrue(t.isValid())

        t = QgsTextFormat()
        t.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.Bold, QgsProperty.fromValue(True)
        )
        self.assertTrue(t.isValid())

        t = QgsTextFormat()
        t.setForcedBold(True)
        self.assertTrue(t.isValid())

        t = QgsTextFormat()
        t.setForcedItalic(True)
        self.assertTrue(t.isValid())

    def test_tab(self):
        pos = QgsTextFormat.Tab(4)
        self.assertEqual(pos, QgsTextFormat.Tab(4))
        self.assertNotEqual(pos, QgsTextFormat.Tab(14))
        self.assertEqual(str(pos), "<QgsTextFormat.Tab: 4>")

    def createBufferSettings(self):
        s = QgsTextBufferSettings()
        s.setEnabled(True)
        s.setSize(5)
        s.setSizeUnit(Qgis.RenderUnit.Points)
        s.setSizeMapUnitScale(QgsMapUnitScale(1, 2))
        s.setColor(QColor(255, 0, 0))
        s.setFillBufferInterior(True)
        s.setOpacity(0.5)
        s.setJoinStyle(Qt.PenJoinStyle.RoundJoin)
        s.setBlendMode(QPainter.CompositionMode.CompositionMode_DestinationAtop)
        return s

    def testBufferEquality(self):
        s = self.createBufferSettings()
        s2 = self.createBufferSettings()
        self.assertEqual(s, s2)

        s.setEnabled(False)
        self.assertNotEqual(s, s2)
        s = self.createBufferSettings()

        s.setSize(15)
        self.assertNotEqual(s, s2)
        s = self.createBufferSettings()

        s.setSizeUnit(Qgis.RenderUnit.Pixels)
        self.assertNotEqual(s, s2)
        s = self.createBufferSettings()

        s.setSizeMapUnitScale(QgsMapUnitScale(11, 12))
        self.assertNotEqual(s, s2)
        s = self.createBufferSettings()

        s.setColor(QColor(255, 255, 0))
        self.assertNotEqual(s, s2)
        s = self.createBufferSettings()

        s.setFillBufferInterior(False)
        self.assertNotEqual(s, s2)
        s = self.createBufferSettings()

        s.setOpacity(0.6)
        self.assertNotEqual(s, s2)
        s = self.createBufferSettings()

        s.setJoinStyle(Qt.PenJoinStyle.MiterJoin)
        self.assertNotEqual(s, s2)
        s = self.createBufferSettings()

        s.setBlendMode(QPainter.CompositionMode.CompositionMode_Darken)
        self.assertNotEqual(s, s2)

    def checkBufferSettings(self, s):
        """test QgsTextBufferSettings"""
        self.assertTrue(s.enabled())
        self.assertEqual(s.size(), 5)
        self.assertEqual(s.sizeUnit(), Qgis.RenderUnit.Points)
        self.assertEqual(s.sizeMapUnitScale(), QgsMapUnitScale(1, 2))
        self.assertEqual(s.color(), QColor(255, 0, 0))
        self.assertTrue(s.fillBufferInterior())
        self.assertEqual(s.opacity(), 0.5)
        self.assertEqual(s.joinStyle(), Qt.PenJoinStyle.RoundJoin)
        self.assertEqual(
            s.blendMode(), QPainter.CompositionMode.CompositionMode_DestinationAtop
        )

    def testBufferGettersSetters(self):
        s = self.createBufferSettings()
        self.checkBufferSettings(s)

        # some other checks
        s.setEnabled(False)
        self.assertFalse(s.enabled())
        s.setEnabled(True)
        self.assertTrue(s.enabled())
        s.setFillBufferInterior(False)
        self.assertFalse(s.fillBufferInterior())
        s.setFillBufferInterior(True)
        self.assertTrue(s.fillBufferInterior())

    def testBufferReadWriteXml(self):
        """test saving and restoring state of a buffer to xml"""
        doc = QDomDocument("testdoc")
        s = self.createBufferSettings()
        elem = s.writeXml(doc)
        parent = doc.createElement("settings")
        parent.appendChild(elem)
        t = QgsTextBufferSettings()
        t.readXml(parent)
        self.checkBufferSettings(t)

    def testBufferCopy(self):
        s = self.createBufferSettings()
        s2 = s
        self.checkBufferSettings(s2)
        s3 = QgsTextBufferSettings(s)
        self.checkBufferSettings(s3)

    def createMaskSettings(self):
        s = QgsTextMaskSettings()
        s.setEnabled(True)
        s.setType(QgsTextMaskSettings.MaskType.MaskBuffer)
        s.setSize(5)
        s.setSizeUnit(Qgis.RenderUnit.Points)
        s.setSizeMapUnitScale(QgsMapUnitScale(1, 2))
        s.setOpacity(0.5)
        s.setJoinStyle(Qt.PenJoinStyle.RoundJoin)
        s.setMaskedSymbolLayers(
            [
                QgsSymbolLayerReference("layerid1", "symbol1"),
                QgsSymbolLayerReference("layerid2", "symbol2"),
            ]
        )
        return s

    def testMaskEquality(self):
        s = self.createMaskSettings()
        s2 = self.createMaskSettings()
        self.assertEqual(s, s2)

        s.setEnabled(False)
        self.assertNotEqual(s, s2)
        s = self.createMaskSettings()

        s.setSize(15)
        self.assertNotEqual(s, s2)
        s = self.createMaskSettings()

        s.setSizeUnit(Qgis.RenderUnit.Pixels)
        self.assertNotEqual(s, s2)
        s = self.createMaskSettings()

        s.setSizeMapUnitScale(QgsMapUnitScale(11, 12))
        self.assertNotEqual(s, s2)
        s = self.createMaskSettings()

        s.setOpacity(0.6)
        self.assertNotEqual(s, s2)
        s = self.createMaskSettings()

        s.setJoinStyle(Qt.PenJoinStyle.MiterJoin)
        self.assertNotEqual(s, s2)
        s = self.createMaskSettings()

        s.setMaskedSymbolLayers(
            [
                QgsSymbolLayerReference("layerid11", "symbol1"),
                QgsSymbolLayerReference("layerid21", "symbol2"),
            ]
        )
        self.assertNotEqual(s, s2)

    def checkMaskSettings(self, s):
        """test QgsTextMaskSettings"""
        self.assertTrue(s.enabled())
        self.assertEqual(s.type(), QgsTextMaskSettings.MaskType.MaskBuffer)
        self.assertEqual(s.size(), 5)
        self.assertEqual(s.sizeUnit(), Qgis.RenderUnit.Points)
        self.assertEqual(s.sizeMapUnitScale(), QgsMapUnitScale(1, 2))
        self.assertEqual(s.opacity(), 0.5)
        self.assertEqual(s.joinStyle(), Qt.PenJoinStyle.RoundJoin)
        self.assertEqual(
            s.maskedSymbolLayers(),
            [
                QgsSymbolLayerReference("layerid1", "symbol1"),
                QgsSymbolLayerReference("layerid2", "symbol2"),
            ],
        )

    def testMaskGettersSetters(self):
        s = self.createMaskSettings()
        self.checkMaskSettings(s)

        # some other checks
        s.setEnabled(False)
        self.assertFalse(s.enabled())

    def testMaskReadWriteXml(self):
        """test saving and restoring state of a mask to xml"""
        doc = QDomDocument("testdoc")
        s = self.createMaskSettings()
        elem = s.writeXml(doc)
        parent = doc.createElement("settings")
        parent.appendChild(elem)
        t = QgsTextMaskSettings()
        t.readXml(parent)
        self.checkMaskSettings(t)

    def testMaskCopy(self):
        s = self.createMaskSettings()
        s2 = s
        self.checkMaskSettings(s2)
        s3 = QgsTextMaskSettings(s)
        self.checkMaskSettings(s3)

    def createBackgroundSettings(self):
        s = QgsTextBackgroundSettings()
        s.setEnabled(True)
        s.setType(QgsTextBackgroundSettings.ShapeType.ShapeEllipse)
        s.setSvgFile("svg.svg")
        s.setSizeType(QgsTextBackgroundSettings.SizeType.SizePercent)
        s.setSize(QSizeF(1, 2))
        s.setSizeUnit(Qgis.RenderUnit.Points)
        s.setSizeMapUnitScale(QgsMapUnitScale(1, 2))
        s.setRotationType(QgsTextBackgroundSettings.RotationType.RotationFixed)
        s.setRotation(45)
        s.setOffset(QPointF(3, 4))
        s.setOffsetUnit(Qgis.RenderUnit.MapUnits)
        s.setOffsetMapUnitScale(QgsMapUnitScale(5, 6))
        s.setRadii(QSizeF(11, 12))
        s.setRadiiUnit(Qgis.RenderUnit.Percentage)
        s.setRadiiMapUnitScale(QgsMapUnitScale(15, 16))
        s.setFillColor(QColor(255, 0, 0))
        s.setStrokeColor(QColor(0, 255, 0))
        s.setOpacity(0.5)
        s.setJoinStyle(Qt.PenJoinStyle.RoundJoin)
        s.setBlendMode(QPainter.CompositionMode.CompositionMode_DestinationAtop)
        s.setStrokeWidth(7)
        s.setStrokeWidthUnit(Qgis.RenderUnit.Points)
        s.setStrokeWidthMapUnitScale(QgsMapUnitScale(QgsMapUnitScale(25, 26)))

        marker = QgsMarkerSymbol()
        marker.setColor(QColor(100, 112, 134))
        s.setMarkerSymbol(marker)

        return s

    def testBackgroundEquality(self):
        s = self.createBackgroundSettings()
        s2 = self.createBackgroundSettings()
        self.assertEqual(s, s2)

        s.setEnabled(False)
        self.assertNotEqual(s, s2)
        s = self.createBackgroundSettings()

        s.setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        self.assertNotEqual(s, s2)
        s = self.createBackgroundSettings()

        s.setSvgFile("svg2.svg")
        self.assertNotEqual(s, s2)
        s = self.createBackgroundSettings()

        s.setSizeType(QgsTextBackgroundSettings.SizeType.SizeFixed)
        self.assertNotEqual(s, s2)
        s = self.createBackgroundSettings()

        s.setSize(QSizeF(1, 22))
        self.assertNotEqual(s, s2)
        s = self.createBackgroundSettings()

        s.setSizeUnit(Qgis.RenderUnit.Pixels)
        self.assertNotEqual(s, s2)
        s = self.createBackgroundSettings()

        s.setSizeMapUnitScale(QgsMapUnitScale(11, 22))
        self.assertNotEqual(s, s2)
        s = self.createBackgroundSettings()

        s.setRotationType(QgsTextBackgroundSettings.RotationType.RotationSync)
        self.assertNotEqual(s, s2)
        s = self.createBackgroundSettings()

        s.setRotation(145)
        self.assertNotEqual(s, s2)
        s = self.createBackgroundSettings()

        s.setOffset(QPointF(31, 41))
        self.assertNotEqual(s, s2)
        s = self.createBackgroundSettings()

        s.setOffsetUnit(Qgis.RenderUnit.Pixels)
        self.assertNotEqual(s, s2)
        s = self.createBackgroundSettings()

        s.setOffsetMapUnitScale(QgsMapUnitScale(15, 16))
        self.assertNotEqual(s, s2)
        s = self.createBackgroundSettings()

        s.setRadii(QSizeF(111, 112))
        self.assertNotEqual(s, s2)
        s = self.createBackgroundSettings()

        s.setRadiiUnit(Qgis.RenderUnit.Points)
        self.assertNotEqual(s, s2)
        s = self.createBackgroundSettings()

        s.setRadiiMapUnitScale(QgsMapUnitScale(151, 161))
        self.assertNotEqual(s, s2)
        s = self.createBackgroundSettings()

        s.setFillColor(QColor(255, 255, 0))
        self.assertNotEqual(s, s2)
        s = self.createBackgroundSettings()

        s.setStrokeColor(QColor(0, 255, 255))
        self.assertNotEqual(s, s2)
        s = self.createBackgroundSettings()

        s.setOpacity(0.6)
        self.assertNotEqual(s, s2)
        s = self.createBackgroundSettings()

        s.setJoinStyle(Qt.PenJoinStyle.MiterJoin)
        self.assertNotEqual(s, s2)
        s = self.createBackgroundSettings()

        s.setBlendMode(QPainter.CompositionMode.CompositionMode_Darken)
        self.assertNotEqual(s, s2)
        s = self.createBackgroundSettings()

        s.setStrokeWidth(17)
        self.assertNotEqual(s, s2)
        s = self.createBackgroundSettings()

        s.setStrokeWidthUnit(Qgis.RenderUnit.Pixels)
        self.assertNotEqual(s, s2)
        s = self.createBackgroundSettings()

        s.setStrokeWidthMapUnitScale(QgsMapUnitScale(QgsMapUnitScale(251, 261)))
        self.assertNotEqual(s, s2)

    def checkBackgroundSettings(self, s):
        """test QgsTextBackgroundSettings"""
        self.assertTrue(s.enabled())
        self.assertEqual(s.type(), QgsTextBackgroundSettings.ShapeType.ShapeEllipse)
        self.assertEqual(s.svgFile(), "svg.svg")
        self.assertEqual(s.sizeType(), QgsTextBackgroundSettings.SizeType.SizePercent)
        self.assertEqual(s.size(), QSizeF(1, 2))
        self.assertEqual(s.sizeUnit(), Qgis.RenderUnit.Points)
        self.assertEqual(s.sizeMapUnitScale(), QgsMapUnitScale(1, 2))
        self.assertEqual(
            s.rotationType(), QgsTextBackgroundSettings.RotationType.RotationFixed
        )
        self.assertEqual(s.rotation(), 45)
        self.assertEqual(s.offset(), QPointF(3, 4))
        self.assertEqual(s.offsetUnit(), Qgis.RenderUnit.MapUnits)
        self.assertEqual(s.offsetMapUnitScale(), QgsMapUnitScale(5, 6))
        self.assertEqual(s.radii(), QSizeF(11, 12))
        self.assertEqual(s.radiiUnit(), Qgis.RenderUnit.Percentage)
        self.assertEqual(s.radiiMapUnitScale(), QgsMapUnitScale(15, 16))
        self.assertEqual(s.fillColor(), QColor(255, 0, 0))
        self.assertEqual(s.strokeColor(), QColor(0, 255, 0))
        self.assertEqual(s.opacity(), 0.5)
        self.assertEqual(s.joinStyle(), Qt.PenJoinStyle.RoundJoin)
        self.assertEqual(
            s.blendMode(), QPainter.CompositionMode.CompositionMode_DestinationAtop
        )
        self.assertEqual(s.strokeWidth(), 7)
        self.assertEqual(s.strokeWidthUnit(), Qgis.RenderUnit.Points)
        self.assertEqual(s.strokeWidthMapUnitScale(), QgsMapUnitScale(25, 26))
        self.assertEqual(s.markerSymbol().color().name(), "#647086")

    def testBackgroundGettersSetters(self):
        s = self.createBackgroundSettings()
        self.checkBackgroundSettings(s)

        # some other checks
        s.setEnabled(False)
        self.assertFalse(s.enabled())
        s.setEnabled(True)
        self.assertTrue(s.enabled())

    def testBackgroundCopy(self):
        s = self.createBackgroundSettings()
        s2 = s
        self.checkBackgroundSettings(s2)
        s3 = QgsTextBackgroundSettings(s)
        self.checkBackgroundSettings(s3)

    def testBackgroundReadWriteXml(self):
        """test saving and restoring state of a background to xml"""
        doc = QDomDocument("testdoc")
        s = self.createBackgroundSettings()
        elem = s.writeXml(doc, QgsReadWriteContext())
        parent = doc.createElement("settings")
        parent.appendChild(elem)
        t = QgsTextBackgroundSettings()
        t.readXml(parent, QgsReadWriteContext())
        self.checkBackgroundSettings(t)

    def createShadowSettings(self):
        s = QgsTextShadowSettings()
        s.setEnabled(True)
        s.setShadowPlacement(QgsTextShadowSettings.ShadowPlacement.ShadowBuffer)
        s.setOffsetAngle(45)
        s.setOffsetDistance(75)
        s.setOffsetUnit(Qgis.RenderUnit.MapUnits)
        s.setOffsetMapUnitScale(QgsMapUnitScale(5, 6))
        s.setOffsetGlobal(True)
        s.setBlurRadius(11)
        s.setBlurRadiusUnit(Qgis.RenderUnit.Percentage)
        s.setBlurRadiusMapUnitScale(QgsMapUnitScale(15, 16))
        s.setBlurAlphaOnly(True)
        s.setColor(QColor(255, 0, 0))
        s.setOpacity(0.5)
        s.setScale(123)
        s.setBlendMode(QPainter.CompositionMode.CompositionMode_DestinationAtop)
        return s

    def testShadowEquality(self):
        s = self.createShadowSettings()
        s2 = self.createShadowSettings()
        self.assertEqual(s, s2)

        s.setEnabled(False)
        self.assertNotEqual(s, s2)
        s = self.createShadowSettings()

        s.setShadowPlacement(QgsTextShadowSettings.ShadowPlacement.ShadowText)
        self.assertNotEqual(s, s2)
        s = self.createShadowSettings()

        s.setOffsetAngle(145)
        self.assertNotEqual(s, s2)
        s = self.createShadowSettings()

        s.setOffsetDistance(175)
        self.assertNotEqual(s, s2)
        s = self.createShadowSettings()

        s.setOffsetUnit(Qgis.RenderUnit.Pixels)
        self.assertNotEqual(s, s2)
        s = self.createShadowSettings()

        s.setOffsetMapUnitScale(QgsMapUnitScale(15, 16))
        self.assertNotEqual(s, s2)
        s = self.createShadowSettings()

        s.setOffsetGlobal(False)
        self.assertNotEqual(s, s2)
        s = self.createShadowSettings()

        s.setBlurRadius(21)
        self.assertNotEqual(s, s2)
        s = self.createShadowSettings()

        s.setBlurRadiusUnit(Qgis.RenderUnit.Points)
        self.assertNotEqual(s, s2)
        s = self.createShadowSettings()

        s.setBlurRadiusMapUnitScale(QgsMapUnitScale(115, 116))
        self.assertNotEqual(s, s2)
        s = self.createShadowSettings()

        s.setBlurAlphaOnly(False)
        self.assertNotEqual(s, s2)
        s = self.createShadowSettings()

        s.setColor(QColor(255, 255, 0))
        self.assertNotEqual(s, s2)
        s = self.createShadowSettings()

        s.setOpacity(0.6)
        self.assertNotEqual(s, s2)
        s = self.createShadowSettings()

        s.setScale(23)
        self.assertNotEqual(s, s2)
        s = self.createShadowSettings()

        s.setBlendMode(QPainter.CompositionMode.CompositionMode_Darken)
        self.assertNotEqual(s, s2)

    def checkShadowSettings(self, s):
        """test QgsTextShadowSettings"""
        self.assertTrue(s.enabled())
        self.assertEqual(
            s.shadowPlacement(), QgsTextShadowSettings.ShadowPlacement.ShadowBuffer
        )
        self.assertEqual(s.offsetAngle(), 45)
        self.assertEqual(s.offsetDistance(), 75)
        self.assertEqual(s.offsetUnit(), Qgis.RenderUnit.MapUnits)
        self.assertEqual(s.offsetMapUnitScale(), QgsMapUnitScale(5, 6))
        self.assertTrue(s.offsetGlobal())
        self.assertEqual(s.blurRadius(), 11)
        self.assertEqual(s.blurRadiusUnit(), Qgis.RenderUnit.Percentage)
        self.assertEqual(s.blurRadiusMapUnitScale(), QgsMapUnitScale(15, 16))
        self.assertTrue(s.blurAlphaOnly())
        self.assertEqual(s.color(), QColor(255, 0, 0))
        self.assertEqual(s.opacity(), 0.5)
        self.assertEqual(s.scale(), 123)
        self.assertEqual(
            s.blendMode(), QPainter.CompositionMode.CompositionMode_DestinationAtop
        )

    def testShadowGettersSetters(self):
        s = self.createShadowSettings()
        self.checkShadowSettings(s)

        # some other checks
        s.setEnabled(False)
        self.assertFalse(s.enabled())
        s.setEnabled(True)
        self.assertTrue(s.enabled())
        s.setOffsetGlobal(False)
        self.assertFalse(s.offsetGlobal())
        s.setOffsetGlobal(True)
        self.assertTrue(s.offsetGlobal())
        s.setBlurAlphaOnly(False)
        self.assertFalse(s.blurAlphaOnly())
        s.setBlurAlphaOnly(True)
        self.assertTrue(s.blurAlphaOnly())

    def testShadowCopy(self):
        s = self.createShadowSettings()
        s2 = s
        self.checkShadowSettings(s2)
        s3 = QgsTextShadowSettings(s)
        self.checkShadowSettings(s3)

    def testShadowReadWriteXml(self):
        """test saving and restoring state of a shadow to xml"""
        doc = QDomDocument("testdoc")
        s = self.createShadowSettings()
        elem = s.writeXml(doc)
        parent = doc.createElement("settings")
        parent.appendChild(elem)
        t = QgsTextShadowSettings()
        t.readXml(parent)
        self.checkShadowSettings(t)

    def createFormatSettings(self):
        s = QgsTextFormat()
        s.buffer().setEnabled(True)
        s.buffer().setSize(25)
        s.mask().setEnabled(True)
        s.mask().setSize(32)
        s.background().setEnabled(True)
        s.background().setSvgFile("test.svg")
        s.shadow().setEnabled(True)
        s.shadow().setOffsetAngle(223)
        font = getTestFont()
        font.setKerning(False)
        s.setFont(font)
        s.setCapitalization(Qgis.Capitalization.TitleCase)
        s.setNamedStyle("Italic")
        s.setFamilies(["Arial", "Comic Sans"])
        s.setSize(5)
        s.setSizeUnit(Qgis.RenderUnit.Points)
        s.setSizeMapUnitScale(QgsMapUnitScale(1, 2))
        s.setColor(QColor(255, 0, 0))
        s.setOpacity(0.5)
        s.setBlendMode(QPainter.CompositionMode.CompositionMode_DestinationAtop)
        s.setLineHeight(5)
        s.setLineHeightUnit(Qgis.RenderUnit.Inches)
        s.setPreviewBackgroundColor(QColor(100, 150, 200))
        s.setOrientation(QgsTextFormat.TextOrientation.VerticalOrientation)
        s.setAllowHtmlFormatting(True)
        s.setForcedBold(True)
        s.setForcedItalic(True)

        s.setTabStopDistance(4.5)
        s.setTabPositions([QgsTextFormat.Tab(5), QgsTextFormat.Tab(17)])
        s.setTabStopDistanceUnit(Qgis.RenderUnit.RenderInches)
        s.setTabStopDistanceMapUnitScale(QgsMapUnitScale(11, 12))

        s.setStretchFactor(110)

        s.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.Bold, QgsProperty.fromExpression("1>2")
        )
        return s

    def testFormatEquality(self):
        s = self.createFormatSettings()
        s2 = self.createFormatSettings()
        self.assertEqual(s, s2)

        s.buffer().setEnabled(False)
        self.assertNotEqual(s, s2)
        s = self.createFormatSettings()

        s.buffer().setSize(12)
        self.assertNotEqual(s, s2)
        s = self.createFormatSettings()

        s.mask().setEnabled(False)
        self.assertNotEqual(s, s2)
        s = self.createFormatSettings()

        s.mask().setSize(12)
        self.assertNotEqual(s, s2)
        s = self.createFormatSettings()

        s.background().setEnabled(False)
        self.assertNotEqual(s, s2)
        s = self.createFormatSettings()

        s.background().setSvgFile("test2.svg")
        self.assertNotEqual(s, s2)
        s = self.createFormatSettings()

        s.shadow().setEnabled(False)
        self.assertNotEqual(s, s2)
        s = self.createFormatSettings()

        s.shadow().setOffsetAngle(123)
        self.assertNotEqual(s, s2)
        s = self.createFormatSettings()

        font = getTestFont()
        font.setKerning(True)
        s.setFont(font)
        self.assertNotEqual(s, s2)
        s = self.createFormatSettings()

        s.setNamedStyle("Bold")
        self.assertNotEqual(s, s2)
        s = self.createFormatSettings()

        s.setSize(15)
        self.assertNotEqual(s, s2)
        s = self.createFormatSettings()

        s.setSizeUnit(Qgis.RenderUnit.Pixels)
        self.assertNotEqual(s, s2)
        s = self.createFormatSettings()

        s.setSizeMapUnitScale(QgsMapUnitScale(11, 12))
        self.assertNotEqual(s, s2)
        s = self.createFormatSettings()

        s.setColor(QColor(255, 255, 0))
        self.assertNotEqual(s, s2)
        s = self.createFormatSettings()

        s.setOpacity(0.6)
        self.assertNotEqual(s, s2)
        s = self.createFormatSettings()

        s.setBlendMode(QPainter.CompositionMode.CompositionMode_Darken)
        self.assertNotEqual(s, s2)
        s = self.createFormatSettings()

        s.setLineHeight(15)
        self.assertNotEqual(s, s2)
        s = self.createFormatSettings()

        s.setLineHeightUnit(Qgis.RenderUnit.Points)
        self.assertNotEqual(s, s2)
        s = self.createFormatSettings()

        s.setPreviewBackgroundColor(QColor(100, 250, 200))
        self.assertNotEqual(s, s2)
        s = self.createFormatSettings()

        s.setOrientation(QgsTextFormat.TextOrientation.HorizontalOrientation)
        self.assertNotEqual(s, s2)
        s = self.createFormatSettings()

        s.setAllowHtmlFormatting(False)
        self.assertNotEqual(s, s2)
        s = self.createFormatSettings()

        s.setForcedBold(False)
        self.assertNotEqual(s, s2)
        s = self.createFormatSettings()

        s.setForcedItalic(False)
        self.assertNotEqual(s, s2)
        s = self.createFormatSettings()

        s.setCapitalization(Qgis.Capitalization.ForceFirstLetterToCapital)
        self.assertNotEqual(s, s2)
        s = self.createFormatSettings()

        s.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.Bold, QgsProperty.fromExpression("1>3")
        )
        self.assertNotEqual(s, s2)

        s = self.createFormatSettings()
        s.setFamilies(["Times New Roman"])
        self.assertNotEqual(s, s2)

        s = self.createFormatSettings()
        s.setStretchFactor(120)
        self.assertNotEqual(s, s2)

        s = self.createFormatSettings()
        s.setTabStopDistance(120)
        self.assertNotEqual(s, s2)

        s = self.createFormatSettings()
        s.setTabPositions([QgsTextFormat.Tab(11), QgsTextFormat.Tab(13)])
        self.assertNotEqual(s, s2)

        s = self.createFormatSettings()
        s.setTabStopDistanceUnit(Qgis.RenderUnit.Points)
        self.assertNotEqual(s, s2)

        s = self.createFormatSettings()
        s.setTabStopDistanceMapUnitScale(QgsMapUnitScale(111, 122))
        self.assertNotEqual(s, s2)

    def checkTextFormat(self, s):
        """test QgsTextFormat"""
        self.assertTrue(s.buffer().enabled())
        self.assertEqual(s.buffer().size(), 25)
        self.assertTrue(s.mask().enabled())
        self.assertEqual(s.mask().size(), 32)
        self.assertTrue(s.background().enabled())
        self.assertEqual(s.background().svgFile(), "test.svg")
        self.assertTrue(s.shadow().enabled())
        self.assertEqual(s.shadow().offsetAngle(), 223)
        self.assertEqual(s.font().family(), "QGIS Vera Sans")
        self.assertEqual(s.families(), ["Arial", "Comic Sans"])
        self.assertFalse(s.font().kerning())
        self.assertEqual(s.namedStyle(), "Italic")
        self.assertEqual(s.size(), 5)
        self.assertEqual(s.sizeUnit(), Qgis.RenderUnit.Points)
        self.assertEqual(s.sizeMapUnitScale(), QgsMapUnitScale(1, 2))
        self.assertEqual(s.color(), QColor(255, 0, 0))
        self.assertEqual(s.opacity(), 0.5)
        self.assertEqual(
            s.blendMode(), QPainter.CompositionMode.CompositionMode_DestinationAtop
        )
        self.assertEqual(s.lineHeight(), 5)
        self.assertEqual(s.lineHeightUnit(), Qgis.RenderUnit.Inches)
        self.assertEqual(s.previewBackgroundColor().name(), "#6496c8")
        self.assertEqual(
            s.orientation(), QgsTextFormat.TextOrientation.VerticalOrientation
        )
        self.assertEqual(s.capitalization(), Qgis.Capitalization.TitleCase)
        self.assertTrue(s.allowHtmlFormatting())
        self.assertEqual(
            s.dataDefinedProperties()
            .property(QgsPalLayerSettings.Property.Bold)
            .expressionString(),
            "1>2",
        )
        self.assertTrue(s.forcedBold())
        self.assertTrue(s.forcedItalic())
        self.assertEqual(s.tabStopDistance(), 4.5)
        self.assertEqual(
            s.tabPositions(), [QgsTextFormat.Tab(5), QgsTextFormat.Tab(17)]
        )
        self.assertEqual(s.tabStopDistanceUnit(), Qgis.RenderUnit.Inches)
        self.assertEqual(s.tabStopDistanceMapUnitScale(), QgsMapUnitScale(11, 12))

        if int(QT_VERSION_STR.split(".")[0]) > 6 or (
            int(QT_VERSION_STR.split(".")[0]) == 6
            and int(QT_VERSION_STR.split(".")[1]) >= 3
        ):
            self.assertEqual(s.stretchFactor(), 110)

    def testFormatGettersSetters(self):
        s = self.createFormatSettings()
        self.checkTextFormat(s)

    def testFormatCopy(self):
        s = self.createFormatSettings()
        s2 = s
        self.checkTextFormat(s2)
        s3 = QgsTextFormat(s)
        self.checkTextFormat(s3)

    def testFormatReadWriteXml(self):
        """test saving and restoring state of a shadow to xml"""
        doc = QDomDocument("testdoc")
        s = self.createFormatSettings()
        elem = s.writeXml(doc, QgsReadWriteContext())
        parent = doc.createElement("settings")
        parent.appendChild(elem)
        t = QgsTextFormat()
        t.readXml(parent, QgsReadWriteContext())
        self.checkTextFormat(t)

    def testFormatToFromMimeData(self):
        """Test converting format to and from mime data"""
        s = self.createFormatSettings()
        md = s.toMimeData()
        from_mime, ok = QgsTextFormat.fromMimeData(None)
        self.assertFalse(ok)
        from_mime, ok = QgsTextFormat.fromMimeData(md)
        self.assertTrue(ok)
        self.checkTextFormat(from_mime)

    def testRestoreUsingFamilyList(self):
        format = QgsTextFormat()

        doc = QDomDocument("testdoc")
        elem = format.writeXml(doc, QgsReadWriteContext())
        parent = doc.createElement("settings")
        parent.appendChild(elem)
        doc.appendChild(parent)

        # swap out font name in xml to one which doesn't exist on system
        xml = doc.toString()
        xml = xml.replace(QFont().family(), "NOT A REAL FONT")
        doc = QDomDocument("testdoc")
        doc.setContent(xml)
        parent = doc.firstChildElement("settings")

        t = QgsTextFormat()
        t.readXml(parent, QgsReadWriteContext())
        # should be default font
        self.assertEqual(t.font().family(), QFont().family())

        format.setFamilies(["not real", "still not real", getTestFont().family()])

        doc = QDomDocument("testdoc")
        elem = format.writeXml(doc, QgsReadWriteContext())
        parent = doc.createElement("settings")
        parent.appendChild(elem)
        doc.appendChild(parent)

        # swap out font name in xml to one which doesn't exist on system
        xml = doc.toString()
        xml = xml.replace(QFont().family(), "NOT A REAL FONT")
        doc = QDomDocument("testdoc")
        doc.setContent(xml)
        parent = doc.firstChildElement("settings")

        t = QgsTextFormat()
        t.readXml(parent, QgsReadWriteContext())
        self.assertEqual(
            t.families(), ["not real", "still not real", getTestFont().family()]
        )
        # should have skipped the missing fonts and fallen back to the test font family entry, NOT the default application font!
        self.assertEqual(t.font().family(), getTestFont().family())

    def testMultiplyOpacity(self):

        s = self.createFormatSettings()
        old_opacity = s.opacity()
        old_buffer_opacity = s.buffer().opacity()
        old_shadow_opacity = s.shadow().opacity()
        old_mask_opacity = s.mask().opacity()

        s.multiplyOpacity(0.5)

        self.assertEqual(s.opacity(), old_opacity * 0.5)
        self.assertEqual(s.buffer().opacity(), old_buffer_opacity * 0.5)
        self.assertEqual(s.shadow().opacity(), old_shadow_opacity * 0.5)
        self.assertEqual(s.mask().opacity(), old_mask_opacity * 0.5)

        s.multiplyOpacity(2.0)
        self.checkTextFormat(s)

    def testContainsAdvancedEffects(self):
        t = QgsTextFormat()
        self.assertFalse(t.containsAdvancedEffects())
        t.setBlendMode(QPainter.CompositionMode.CompositionMode_DestinationAtop)
        self.assertTrue(t.containsAdvancedEffects())

        t = QgsTextFormat()
        t.buffer().setBlendMode(
            QPainter.CompositionMode.CompositionMode_DestinationAtop
        )
        self.assertFalse(t.containsAdvancedEffects())
        t.buffer().setEnabled(True)
        self.assertTrue(t.containsAdvancedEffects())
        t.buffer().setBlendMode(QPainter.CompositionMode.CompositionMode_SourceOver)
        self.assertFalse(t.containsAdvancedEffects())

        t = QgsTextFormat()
        t.background().setBlendMode(
            QPainter.CompositionMode.CompositionMode_DestinationAtop
        )
        self.assertFalse(t.containsAdvancedEffects())
        t.background().setEnabled(True)
        self.assertTrue(t.containsAdvancedEffects())
        t.background().setBlendMode(QPainter.CompositionMode.CompositionMode_SourceOver)
        self.assertFalse(t.containsAdvancedEffects())

        t = QgsTextFormat()
        t.shadow().setBlendMode(
            QPainter.CompositionMode.CompositionMode_DestinationAtop
        )
        self.assertFalse(t.containsAdvancedEffects())
        t.shadow().setEnabled(True)
        self.assertTrue(t.containsAdvancedEffects())
        t.shadow().setBlendMode(QPainter.CompositionMode.CompositionMode_SourceOver)
        self.assertFalse(t.containsAdvancedEffects())

    def testRestoringAndSavingMissingFont(self):
        # test that a missing font on text format load will still save with the same missing font unless manually changed
        document = QDomDocument()
        document.setContent(
            '<text-style fontFamily="__MISSING__" previewBkgrdColor="255,255,255,255,rgb:1,1,1,1" fieldName="" tabStopDistance="80" tabStopDistanceMapUnitScale="3x:0,0,0,0,0,0" useSubstitutions="0" forcedItalic="0" fontWeight="50" multilineHeightUnit="Percentage" allowHtml="0" fontKerning="1" multilineHeight="1" fontSizeUnit="Point" fontSize="11" blendMode="0" tabStopDistanceUnit="Point" isExpression="1" fontStrikeout="0" fontLetterSpacing="0" fontSizeMapUnitScale="3x:0,0,0,0,0,0" textColor="50,50,50,255,rgb:0.19607843137254902,0.19607843137254902,0.19607843137254902,1" legendString="Aa" textOrientation="horizontal" namedStyle="Regular" fontItalic="0" fontUnderline="0" forcedBold="0" capitalization="0" textOpacity="1" fontWordSpacing="0"><families/><substitutions/></text-style>'
        )

        context = QgsReadWriteContext()
        text_format = QgsTextFormat()
        text_format.readXml(document.documentElement(), context)

        self.assertFalse(text_format.fontFound())
        self.assertTrue(text_format.font().family() != "__MISSING__")

        # when writign the settings to XML, the missing font family should still be there
        element = text_format.writeXml(document, context)
        self.assertEqual(element.attribute("fontFamily"), "__MISSING__")

        font = getTestFont()
        text_format.setFont(font)

        # when writing the settings to XML, the originally missing font family should have been replaced by the new font family
        element = text_format.writeXml(document, context)
        self.assertEqual(element.attribute("fontFamily"), "QGIS Vera Sans")

    def testDataDefinedBufferSettings(self):
        f = QgsTextFormat()
        context = QgsRenderContext()

        # buffer enabled
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.BufferDraw, QgsProperty.fromExpression("1")
        )
        f.updateDataDefinedProperties(context)
        self.assertTrue(f.buffer().enabled())
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.BufferDraw, QgsProperty.fromExpression("0")
        )
        context = QgsRenderContext()
        f.updateDataDefinedProperties(context)
        self.assertFalse(f.buffer().enabled())

        # buffer size
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.BufferSize, QgsProperty.fromExpression("7.8")
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.buffer().size(), 7.8)
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.BufferUnit,
            QgsProperty.fromExpression("'pixel'"),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.buffer().sizeUnit(), Qgis.RenderUnit.Pixels)

        # buffer opacity
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.BufferOpacity, QgsProperty.fromExpression("37")
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.buffer().opacity(), 0.37)

        # blend mode
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.BufferBlendMode,
            QgsProperty.fromExpression("'burn'"),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(
            f.buffer().blendMode(), QPainter.CompositionMode.CompositionMode_ColorBurn
        )

        # join style
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.BufferJoinStyle,
            QgsProperty.fromExpression("'miter'"),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.buffer().joinStyle(), Qt.PenJoinStyle.MiterJoin)

        # color
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.BufferColor,
            QgsProperty.fromExpression("'#ff0088'"),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.buffer().color().name(), "#ff0088")

    def testDataDefinedMaskSettings(self):
        f = QgsTextFormat()
        context = QgsRenderContext()

        # mask enabled
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.MaskEnabled, QgsProperty.fromExpression("1")
        )
        f.updateDataDefinedProperties(context)
        self.assertTrue(f.mask().enabled())
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.MaskEnabled, QgsProperty.fromExpression("0")
        )
        context = QgsRenderContext()
        f.updateDataDefinedProperties(context)
        self.assertFalse(f.mask().enabled())

        # mask buffer size
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.MaskBufferSize,
            QgsProperty.fromExpression("7.8"),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.mask().size(), 7.8)
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.MaskBufferUnit,
            QgsProperty.fromExpression("'pixel'"),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.mask().sizeUnit(), Qgis.RenderUnit.Pixels)

        # mask opacity
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.MaskOpacity, QgsProperty.fromExpression("37")
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.mask().opacity(), 0.37)

        # join style
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.MaskJoinStyle,
            QgsProperty.fromExpression("'miter'"),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.mask().joinStyle(), Qt.PenJoinStyle.MiterJoin)

    def testDataDefinedBackgroundSettings(self):
        f = QgsTextFormat()
        context = QgsRenderContext()

        # background enabled
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShapeDraw, QgsProperty.fromExpression("1")
        )
        f.updateDataDefinedProperties(context)
        self.assertTrue(f.background().enabled())
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShapeDraw, QgsProperty.fromExpression("0")
        )
        context = QgsRenderContext()
        f.updateDataDefinedProperties(context)
        self.assertFalse(f.background().enabled())

        # background size
        f.background().setSize(QSizeF(13, 14))
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShapeSizeX, QgsProperty.fromExpression("7.8")
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.background().size().width(), 7.8)
        self.assertEqual(f.background().size().height(), 14)
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShapeSizeY, QgsProperty.fromExpression("17.8")
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.background().size().width(), 7.8)
        self.assertEqual(f.background().size().height(), 17.8)

        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShapeSizeUnits,
            QgsProperty.fromExpression("'pixel'"),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.background().sizeUnit(), Qgis.RenderUnit.Pixels)

        # shape kind
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShapeKind,
            QgsProperty.fromExpression("'square'"),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(
            f.background().type(), QgsTextBackgroundSettings.ShapeType.ShapeSquare
        )
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShapeKind,
            QgsProperty.fromExpression("'ellipse'"),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(
            f.background().type(), QgsTextBackgroundSettings.ShapeType.ShapeEllipse
        )
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShapeKind,
            QgsProperty.fromExpression("'circle'"),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(
            f.background().type(), QgsTextBackgroundSettings.ShapeType.ShapeCircle
        )
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShapeKind, QgsProperty.fromExpression("'svg'")
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(
            f.background().type(), QgsTextBackgroundSettings.ShapeType.ShapeSVG
        )
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShapeKind,
            QgsProperty.fromExpression("'marker'"),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(
            f.background().type(), QgsTextBackgroundSettings.ShapeType.ShapeMarkerSymbol
        )
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShapeKind, QgsProperty.fromExpression("'rect'")
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(
            f.background().type(), QgsTextBackgroundSettings.ShapeType.ShapeRectangle
        )

        # size type
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShapeSizeType,
            QgsProperty.fromExpression("'fixed'"),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(
            f.background().sizeType(), QgsTextBackgroundSettings.SizeType.SizeFixed
        )
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShapeSizeType,
            QgsProperty.fromExpression("'buffer'"),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(
            f.background().sizeType(), QgsTextBackgroundSettings.SizeType.SizeBuffer
        )

        # svg path
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShapeSVGFile,
            QgsProperty.fromExpression("'my.svg'"),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.background().svgFile(), "my.svg")

        # shape rotation
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShapeRotation, QgsProperty.fromExpression("67")
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.background().rotation(), 67)
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShapeRotationType,
            QgsProperty.fromExpression("'offset'"),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(
            f.background().rotationType(),
            QgsTextBackgroundSettings.RotationType.RotationOffset,
        )
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShapeRotationType,
            QgsProperty.fromExpression("'fixed'"),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(
            f.background().rotationType(),
            QgsTextBackgroundSettings.RotationType.RotationFixed,
        )
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShapeRotationType,
            QgsProperty.fromExpression("'sync'"),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(
            f.background().rotationType(),
            QgsTextBackgroundSettings.RotationType.RotationSync,
        )

        # shape offset
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShapeOffset,
            QgsProperty.fromExpression("'7,9'"),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.background().offset(), QPointF(7, 9))
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShapeOffsetUnits,
            QgsProperty.fromExpression("'pixel'"),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.background().offsetUnit(), Qgis.RenderUnit.Pixels)

        # shape radii
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShapeRadii,
            QgsProperty.fromExpression("'18,19'"),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.background().radii(), QSizeF(18, 19))
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShapeRadiiUnits,
            QgsProperty.fromExpression("'pixel'"),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.background().radiiUnit(), Qgis.RenderUnit.Pixels)

        # shape opacity
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShapeOpacity, QgsProperty.fromExpression("37")
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.background().opacity(), 0.37)

        # color
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShapeFillColor,
            QgsProperty.fromExpression("'#ff0088'"),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.background().fillColor().name(), "#ff0088")
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShapeStrokeColor,
            QgsProperty.fromExpression("'#8800ff'"),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.background().strokeColor().name(), "#8800ff")

        # stroke width
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShapeStrokeWidth,
            QgsProperty.fromExpression("88"),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.background().strokeWidth(), 88)
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShapeStrokeWidthUnits,
            QgsProperty.fromExpression("'pixel'"),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.background().strokeWidthUnit(), Qgis.RenderUnit.Pixels)

        # blend mode
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShapeBlendMode,
            QgsProperty.fromExpression("'burn'"),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(
            f.background().blendMode(),
            QPainter.CompositionMode.CompositionMode_ColorBurn,
        )

        # join style
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShapeJoinStyle,
            QgsProperty.fromExpression("'miter'"),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.background().joinStyle(), Qt.PenJoinStyle.MiterJoin)

    def testDataDefinedShadowSettings(self):
        f = QgsTextFormat()
        context = QgsRenderContext()

        # shadow enabled
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShadowDraw, QgsProperty.fromExpression("1")
        )
        f.updateDataDefinedProperties(context)
        self.assertTrue(f.shadow().enabled())
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShadowDraw, QgsProperty.fromExpression("0")
        )
        context = QgsRenderContext()
        f.updateDataDefinedProperties(context)
        self.assertFalse(f.shadow().enabled())

        # placement type
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShadowUnder,
            QgsProperty.fromExpression("'text'"),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(
            f.shadow().shadowPlacement(),
            QgsTextShadowSettings.ShadowPlacement.ShadowText,
        )
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShadowUnder,
            QgsProperty.fromExpression("'buffer'"),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(
            f.shadow().shadowPlacement(),
            QgsTextShadowSettings.ShadowPlacement.ShadowBuffer,
        )
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShadowUnder,
            QgsProperty.fromExpression("'background'"),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(
            f.shadow().shadowPlacement(),
            QgsTextShadowSettings.ShadowPlacement.ShadowShape,
        )
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShadowUnder,
            QgsProperty.fromExpression("'svg'"),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(
            f.shadow().shadowPlacement(),
            QgsTextShadowSettings.ShadowPlacement.ShadowLowest,
        )
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShadowUnder,
            QgsProperty.fromExpression("'lowest'"),
        )

        # offset angle
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShadowOffsetAngle,
            QgsProperty.fromExpression("67"),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.shadow().offsetAngle(), 67)

        # offset distance
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShadowOffsetDist,
            QgsProperty.fromExpression("38"),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.shadow().offsetDistance(), 38)
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShadowOffsetUnits,
            QgsProperty.fromExpression("'pixel'"),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.shadow().offsetUnit(), Qgis.RenderUnit.Pixels)

        # radius
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShadowRadius, QgsProperty.fromExpression("58")
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.shadow().blurRadius(), 58)
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShadowRadiusUnits,
            QgsProperty.fromExpression("'pixel'"),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.shadow().blurRadiusUnit(), Qgis.RenderUnit.Pixels)

        # opacity
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShadowOpacity, QgsProperty.fromExpression("37")
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.shadow().opacity(), 0.37)

        # color
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShadowColor,
            QgsProperty.fromExpression("'#ff0088'"),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.shadow().color().name(), "#ff0088")

        # blend mode
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.ShadowBlendMode,
            QgsProperty.fromExpression("'burn'"),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(
            f.shadow().blendMode(), QPainter.CompositionMode.CompositionMode_ColorBurn
        )

    def testDataDefinedFormatSettings(self):
        f = QgsTextFormat()
        font = f.font()
        font.setUnderline(True)
        font.setStrikeOut(True)
        font.setWordSpacing(5.7)
        font.setLetterSpacing(QFont.SpacingType.AbsoluteSpacing, 3.3)
        f.setFont(font)
        context = QgsRenderContext()

        # family
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.Family,
            QgsProperty.fromExpression(
                f"'{QgsFontUtils.getStandardTestFont().family()}'"
            ),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.font().family(), QgsFontUtils.getStandardTestFont().family())

        # style
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.FontStyle, QgsProperty.fromExpression("'Bold'")
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.font().styleName(), "Bold")
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.FontStyle,
            QgsProperty.fromExpression("'Roman'"),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.font().styleName(), "Roman")
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.Bold, QgsProperty.fromExpression("1")
        )
        f.updateDataDefinedProperties(context)
        self.assertTrue(f.font().bold())
        self.assertFalse(f.font().italic())
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.Bold, QgsProperty.fromExpression("0")
        )
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.Italic, QgsProperty.fromExpression("1")
        )
        f.updateDataDefinedProperties(context)
        self.assertFalse(f.font().bold())
        self.assertTrue(f.font().italic())
        self.assertTrue(f.font().underline())
        self.assertTrue(f.font().strikeOut())
        self.assertAlmostEqual(f.font().wordSpacing(), 5.7, 1)
        self.assertAlmostEqual(f.font().letterSpacing(), 3.3, 1)

        # underline
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.Underline, QgsProperty.fromExpression("0")
        )
        f.updateDataDefinedProperties(context)
        self.assertFalse(f.font().underline())
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.Underline, QgsProperty.fromExpression("1")
        )
        f.updateDataDefinedProperties(context)
        self.assertTrue(f.font().underline())

        # strikeout
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.Strikeout, QgsProperty.fromExpression("0")
        )
        f.updateDataDefinedProperties(context)
        self.assertFalse(f.font().strikeOut())
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.Strikeout, QgsProperty.fromExpression("1")
        )
        f.updateDataDefinedProperties(context)
        self.assertTrue(f.font().strikeOut())

        # color
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.Color, QgsProperty.fromExpression("'#ff0088'")
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.color().name(), "#ff0088")

        # size
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.Size, QgsProperty.fromExpression("38")
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.size(), 38)
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.FontSizeUnit,
            QgsProperty.fromExpression("'pixel'"),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.sizeUnit(), Qgis.RenderUnit.Pixels)

        # opacity
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.FontOpacity, QgsProperty.fromExpression("37")
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.opacity(), 0.37)

        # letter spacing
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.FontLetterSpacing,
            QgsProperty.fromExpression("58"),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.font().letterSpacing(), 58)

        # word spacing
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.FontWordSpacing,
            QgsProperty.fromExpression("8"),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.font().wordSpacing(), 8)

        # blend mode
        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.FontBlendMode,
            QgsProperty.fromExpression("'burn'"),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(
            f.blendMode(), QPainter.CompositionMode.CompositionMode_ColorBurn
        )

        if int(QT_VERSION_STR.split(".")[0]) > 6 or (
            int(QT_VERSION_STR.split(".")[0]) == 6
            and int(QT_VERSION_STR.split(".")[1]) >= 3
        ):
            # stretch
            f.dataDefinedProperties().setProperty(
                QgsPalLayerSettings.Property.FontStretchFactor,
                QgsProperty.fromExpression("135"),
            )
            f.updateDataDefinedProperties(context)
            self.assertEqual(f.stretchFactor(), 135)

        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.TabStopDistance,
            QgsProperty.fromExpression("15"),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.tabStopDistance(), 15)
        self.assertFalse(f.tabPositions())

        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.TabStopDistance,
            QgsProperty.fromValue([11, 14]),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(
            f.tabPositions(), [QgsTextFormat.Tab(11), QgsTextFormat.Tab(14)]
        )

        f.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.TabStopDistance,
            QgsProperty.fromValue(["13.5", "15.8"]),
        )
        f.updateDataDefinedProperties(context)
        self.assertEqual(
            f.tabPositions(), [QgsTextFormat.Tab(13.5), QgsTextFormat.Tab(15.8)]
        )

    def testFontFoundFromLayer(self):
        layer = createEmptyLayer()
        layer.setCustomProperty("labeling/fontFamily", "asdasd")
        f = QgsTextFormat()
        f.readFromLayer(layer)
        self.assertFalse(f.fontFound())

        font = getTestFont()
        layer.setCustomProperty("labeling/fontFamily", font.family())
        f.readFromLayer(layer)
        self.assertTrue(f.fontFound())

    def testFontFoundFromXml(self):
        doc = QDomDocument("testdoc")
        f = QgsTextFormat()
        elem = f.writeXml(doc, QgsReadWriteContext())
        elem.setAttribute("fontFamily", "asdfasdfsadf")
        parent = doc.createElement("parent")
        parent.appendChild(elem)

        f.readXml(parent, QgsReadWriteContext())
        self.assertFalse(f.fontFound())

        font = getTestFont()
        elem.setAttribute("fontFamily", font.family())
        f.readXml(parent, QgsReadWriteContext())
        self.assertTrue(f.fontFound())

    def testFromQFont(self):
        qfont = getTestFont()
        qfont.setPointSizeF(16.5)
        qfont.setLetterSpacing(QFont.SpacingType.AbsoluteSpacing, 3)

        format = QgsTextFormat.fromQFont(qfont)
        self.assertEqual(format.font().family(), qfont.family())
        self.assertEqual(format.font().letterSpacing(), 3.0)
        self.assertEqual(format.size(), 16.5)
        self.assertEqual(format.sizeUnit(), Qgis.RenderUnit.Points)

        qfont.setPixelSize(12)
        format = QgsTextFormat.fromQFont(qfont)
        self.assertEqual(format.size(), 12.0)
        self.assertEqual(format.sizeUnit(), Qgis.RenderUnit.Pixels)

    def testToQFont(self):
        s = QgsTextFormat()
        f = getTestFont()
        f.setLetterSpacing(QFont.SpacingType.AbsoluteSpacing, 3)
        s.setFont(f)
        s.setNamedStyle("Italic")
        s.setSize(5.5)
        s.setSizeUnit(Qgis.RenderUnit.Points)

        qfont = s.toQFont()
        self.assertEqual(qfont.family(), f.family())
        self.assertEqual(qfont.pointSizeF(), 5.5)
        self.assertEqual(qfont.letterSpacing(), 3.0)

        s.setSize(5)
        s.setSizeUnit(Qgis.RenderUnit.Pixels)
        qfont = s.toQFont()
        self.assertEqual(qfont.pixelSize(), 5)

        s.setSize(5)
        s.setSizeUnit(Qgis.RenderUnit.Millimeters)
        qfont = s.toQFont()
        self.assertAlmostEqual(qfont.pointSizeF(), 14.17, 2)

        s.setSizeUnit(Qgis.RenderUnit.Inches)
        qfont = s.toQFont()
        self.assertAlmostEqual(qfont.pointSizeF(), 360.0, 2)

        self.assertFalse(qfont.bold())
        s.setForcedBold(True)
        qfont = s.toQFont()
        self.assertTrue(qfont.bold())

        self.assertFalse(qfont.italic())
        s.setForcedItalic(True)
        qfont = s.toQFont()
        self.assertTrue(qfont.italic())

        if int(QT_VERSION_STR.split(".")[0]) > 6 or (
            int(QT_VERSION_STR.split(".")[0]) == 6
            and int(QT_VERSION_STR.split(".")[1]) >= 3
        ):
            s.setStretchFactor(115)
            qfont = s.toQFont()
            self.assertEqual(qfont.stretch(), 115)


if __name__ == "__main__":
    unittest.main()
