# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsTextRenderer.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '2016-09'
__copyright__ = 'Copyright 2016, The QGIS Project'

import os

import qgis  # NOQA
from PyQt5.QtSvg import QSvgGenerator
from qgis.PyQt.QtCore import (Qt, QT_VERSION_STR, QSizeF, QPointF, QRectF, QDir, QSize)
from qgis.PyQt.QtGui import (QColor, QPainter, QFont, QImage, QBrush, QPen)
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (Qgis,
                       QgsTextBufferSettings,
                       QgsTextMaskSettings,
                       QgsTextBackgroundSettings,
                       QgsTextShadowSettings,
                       QgsTextFormat,
                       QgsUnitTypes,
                       QgsMapUnitScale,
                       QgsVectorLayer,
                       QgsTextRenderer,
                       QgsMapSettings,
                       QgsReadWriteContext,
                       QgsRenderContext,
                       QgsRectangle,
                       QgsRenderChecker,
                       QgsBlurEffect,
                       QgsMarkerSymbol,
                       QgsFillSymbol,
                       QgsSimpleFillSymbolLayer,
                       QgsPalLayerSettings,
                       QgsProperty,
                       QgsFontUtils,
                       QgsSymbolLayerId,
                       QgsSymbolLayerReference,
                       QgsStringUtils,
                       QgsTextDocument,
                       QgsTextDocumentMetrics)
from qgis.testing import unittest, start_app

from utilities import getTestFont, svgSymbolsPath

start_app()


def createEmptyLayer():
    layer = QgsVectorLayer("Point", "addfeat", "memory")
    assert layer.isValid()
    return layer


class PyQgsTextRenderer(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.report = "<h1>Python QgsTextRenderer Tests</h1>\n"
        QgsFontUtils.loadStandardTestFonts(['Bold', 'Oblique'])

    @classmethod
    def tearDownClass(cls):
        report_file_path = "%s/qgistest.html" % QDir.tempPath()
        with open(report_file_path, 'a') as report_file:
            report_file.write(cls.report)

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
        t.setNamedStyle('Bold')
        self.assertTrue(t.isValid())

        t = QgsTextFormat()
        t.setSize(20)
        self.assertTrue(t.isValid())

        t = QgsTextFormat()
        t.setSizeUnit(QgsUnitTypes.RenderPixels)
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
        t.setBlendMode(QPainter.CompositionMode_Darken)
        self.assertTrue(t.isValid())

        t = QgsTextFormat()
        t.setLineHeight(20)
        self.assertTrue(t.isValid())

        t = QgsTextFormat()
        t.setLineHeightUnit(QgsUnitTypes.RenderPoints)
        self.assertTrue(t.isValid())

        t = QgsTextFormat()
        t.setOrientation(QgsTextFormat.VerticalOrientation)
        self.assertTrue(t.isValid())

        t = QgsTextFormat()
        t.setCapitalization(QgsStringUtils.TitleCase)
        self.assertTrue(t.isValid())

        t = QgsTextFormat()
        t.setAllowHtmlFormatting(True)
        self.assertTrue(t.isValid())

        t = QgsTextFormat()
        t.setPreviewBackgroundColor(QColor(255, 0, 0))
        self.assertTrue(t.isValid())

        t = QgsTextFormat()
        t.setFamilies(['Arial', 'Comic Sans'])
        self.assertTrue(t.isValid())

        t = QgsTextFormat()
        t.setStretchFactor(110)
        self.assertTrue(t.isValid())

        t = QgsTextFormat()
        t.dataDefinedProperties().setProperty(QgsPalLayerSettings.Bold, QgsProperty.fromValue(True))
        self.assertTrue(t.isValid())

        t = QgsTextFormat()
        t.setForcedBold(True)
        self.assertTrue(t.isValid())

        t = QgsTextFormat()
        t.setForcedItalic(True)
        self.assertTrue(t.isValid())

    def testAlignmentConversion(self):
        self.assertEqual(QgsTextRenderer.convertQtHAlignment(Qt.AlignLeft), QgsTextRenderer.AlignLeft)
        self.assertEqual(QgsTextRenderer.convertQtHAlignment(Qt.AlignRight), QgsTextRenderer.AlignRight)
        self.assertEqual(QgsTextRenderer.convertQtHAlignment(Qt.AlignHCenter), QgsTextRenderer.AlignCenter)
        self.assertEqual(QgsTextRenderer.convertQtHAlignment(Qt.AlignJustify), QgsTextRenderer.AlignJustify)
        # not supported, should fallback to left
        self.assertEqual(QgsTextRenderer.convertQtHAlignment(Qt.AlignAbsolute), QgsTextRenderer.AlignLeft)

        self.assertEqual(QgsTextRenderer.convertQtVAlignment(Qt.AlignTop), QgsTextRenderer.AlignTop)
        self.assertEqual(QgsTextRenderer.convertQtVAlignment(Qt.AlignBottom), QgsTextRenderer.AlignBottom)
        self.assertEqual(QgsTextRenderer.convertQtVAlignment(Qt.AlignVCenter), QgsTextRenderer.AlignVCenter)
        # note supported, should fallback to bottom
        self.assertEqual(QgsTextRenderer.convertQtVAlignment(Qt.AlignBaseline), QgsTextRenderer.AlignBottom)

    def createBufferSettings(self):
        s = QgsTextBufferSettings()
        s.setEnabled(True)
        s.setSize(5)
        s.setSizeUnit(QgsUnitTypes.RenderPoints)
        s.setSizeMapUnitScale(QgsMapUnitScale(1, 2))
        s.setColor(QColor(255, 0, 0))
        s.setFillBufferInterior(True)
        s.setOpacity(0.5)
        s.setJoinStyle(Qt.RoundJoin)
        s.setBlendMode(QPainter.CompositionMode_DestinationAtop)
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

        s.setSizeUnit(QgsUnitTypes.RenderPixels)
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

        s.setJoinStyle(Qt.MiterJoin)
        self.assertNotEqual(s, s2)
        s = self.createBufferSettings()

        s.setBlendMode(QPainter.CompositionMode_Darken)
        self.assertNotEqual(s, s2)

    def checkBufferSettings(self, s):
        """ test QgsTextBufferSettings """
        self.assertTrue(s.enabled())
        self.assertEqual(s.size(), 5)
        self.assertEqual(s.sizeUnit(), QgsUnitTypes.RenderPoints)
        self.assertEqual(s.sizeMapUnitScale(), QgsMapUnitScale(1, 2))
        self.assertEqual(s.color(), QColor(255, 0, 0))
        self.assertTrue(s.fillBufferInterior())
        self.assertEqual(s.opacity(), 0.5)
        self.assertEqual(s.joinStyle(), Qt.RoundJoin)
        self.assertEqual(s.blendMode(), QPainter.CompositionMode_DestinationAtop)

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
        s.setType(QgsTextMaskSettings.MaskBuffer)
        s.setSize(5)
        s.setSizeUnit(QgsUnitTypes.RenderPoints)
        s.setSizeMapUnitScale(QgsMapUnitScale(1, 2))
        s.setOpacity(0.5)
        s.setJoinStyle(Qt.RoundJoin)
        s.setMaskedSymbolLayers([QgsSymbolLayerReference("layerid1", QgsSymbolLayerId("symbol", 1)),
                                 QgsSymbolLayerReference("layerid2", QgsSymbolLayerId("symbol2", 2))])
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

        s.setSizeUnit(QgsUnitTypes.RenderPixels)
        self.assertNotEqual(s, s2)
        s = self.createMaskSettings()

        s.setSizeMapUnitScale(QgsMapUnitScale(11, 12))
        self.assertNotEqual(s, s2)
        s = self.createMaskSettings()

        s.setOpacity(0.6)
        self.assertNotEqual(s, s2)
        s = self.createMaskSettings()

        s.setJoinStyle(Qt.MiterJoin)
        self.assertNotEqual(s, s2)
        s = self.createMaskSettings()

        s.setMaskedSymbolLayers([QgsSymbolLayerReference("layerid11", QgsSymbolLayerId("symbol", 1)),
                                 QgsSymbolLayerReference("layerid21", QgsSymbolLayerId("symbol2", 2))])
        self.assertNotEqual(s, s2)

    def checkMaskSettings(self, s):
        """ test QgsTextMaskSettings """
        self.assertTrue(s.enabled())
        self.assertEqual(s.type(), QgsTextMaskSettings.MaskBuffer)
        self.assertEqual(s.size(), 5)
        self.assertEqual(s.sizeUnit(), QgsUnitTypes.RenderPoints)
        self.assertEqual(s.sizeMapUnitScale(), QgsMapUnitScale(1, 2))
        self.assertEqual(s.opacity(), 0.5)
        self.assertEqual(s.joinStyle(), Qt.RoundJoin)
        self.assertEqual(s.maskedSymbolLayers(), [QgsSymbolLayerReference("layerid1", QgsSymbolLayerId("symbol", 1)),
                                                  QgsSymbolLayerReference("layerid2", QgsSymbolLayerId("symbol2", 2))])

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
        s.setType(QgsTextBackgroundSettings.ShapeEllipse)
        s.setSvgFile('svg.svg')
        s.setSizeType(QgsTextBackgroundSettings.SizePercent)
        s.setSize(QSizeF(1, 2))
        s.setSizeUnit(QgsUnitTypes.RenderPoints)
        s.setSizeMapUnitScale(QgsMapUnitScale(1, 2))
        s.setRotationType(QgsTextBackgroundSettings.RotationFixed)
        s.setRotation(45)
        s.setOffset(QPointF(3, 4))
        s.setOffsetUnit(QgsUnitTypes.RenderMapUnits)
        s.setOffsetMapUnitScale(QgsMapUnitScale(5, 6))
        s.setRadii(QSizeF(11, 12))
        s.setRadiiUnit(QgsUnitTypes.RenderPercentage)
        s.setRadiiMapUnitScale(QgsMapUnitScale(15, 16))
        s.setFillColor(QColor(255, 0, 0))
        s.setStrokeColor(QColor(0, 255, 0))
        s.setOpacity(0.5)
        s.setJoinStyle(Qt.RoundJoin)
        s.setBlendMode(QPainter.CompositionMode_DestinationAtop)
        s.setStrokeWidth(7)
        s.setStrokeWidthUnit(QgsUnitTypes.RenderPoints)
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

        s.setType(QgsTextBackgroundSettings.ShapeRectangle)
        self.assertNotEqual(s, s2)
        s = self.createBackgroundSettings()

        s.setSvgFile('svg2.svg')
        self.assertNotEqual(s, s2)
        s = self.createBackgroundSettings()

        s.setSizeType(QgsTextBackgroundSettings.SizeFixed)
        self.assertNotEqual(s, s2)
        s = self.createBackgroundSettings()

        s.setSize(QSizeF(1, 22))
        self.assertNotEqual(s, s2)
        s = self.createBackgroundSettings()

        s.setSizeUnit(QgsUnitTypes.RenderPixels)
        self.assertNotEqual(s, s2)
        s = self.createBackgroundSettings()

        s.setSizeMapUnitScale(QgsMapUnitScale(11, 22))
        self.assertNotEqual(s, s2)
        s = self.createBackgroundSettings()

        s.setRotationType(QgsTextBackgroundSettings.RotationSync)
        self.assertNotEqual(s, s2)
        s = self.createBackgroundSettings()

        s.setRotation(145)
        self.assertNotEqual(s, s2)
        s = self.createBackgroundSettings()

        s.setOffset(QPointF(31, 41))
        self.assertNotEqual(s, s2)
        s = self.createBackgroundSettings()

        s.setOffsetUnit(QgsUnitTypes.RenderPixels)
        self.assertNotEqual(s, s2)
        s = self.createBackgroundSettings()

        s.setOffsetMapUnitScale(QgsMapUnitScale(15, 16))
        self.assertNotEqual(s, s2)
        s = self.createBackgroundSettings()

        s.setRadii(QSizeF(111, 112))
        self.assertNotEqual(s, s2)
        s = self.createBackgroundSettings()

        s.setRadiiUnit(QgsUnitTypes.RenderPoints)
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

        s.setJoinStyle(Qt.MiterJoin)
        self.assertNotEqual(s, s2)
        s = self.createBackgroundSettings()

        s.setBlendMode(QPainter.CompositionMode_Darken)
        self.assertNotEqual(s, s2)
        s = self.createBackgroundSettings()

        s.setStrokeWidth(17)
        self.assertNotEqual(s, s2)
        s = self.createBackgroundSettings()

        s.setStrokeWidthUnit(QgsUnitTypes.RenderPixels)
        self.assertNotEqual(s, s2)
        s = self.createBackgroundSettings()

        s.setStrokeWidthMapUnitScale(QgsMapUnitScale(QgsMapUnitScale(251, 261)))
        self.assertNotEqual(s, s2)

    def checkBackgroundSettings(self, s):
        """ test QgsTextBackgroundSettings """
        self.assertTrue(s.enabled())
        self.assertEqual(s.type(), QgsTextBackgroundSettings.ShapeEllipse)
        self.assertEqual(s.svgFile(), 'svg.svg')
        self.assertEqual(s.sizeType(), QgsTextBackgroundSettings.SizePercent)
        self.assertEqual(s.size(), QSizeF(1, 2))
        self.assertEqual(s.sizeUnit(), QgsUnitTypes.RenderPoints)
        self.assertEqual(s.sizeMapUnitScale(), QgsMapUnitScale(1, 2))
        self.assertEqual(s.rotationType(), QgsTextBackgroundSettings.RotationFixed)
        self.assertEqual(s.rotation(), 45)
        self.assertEqual(s.offset(), QPointF(3, 4))
        self.assertEqual(s.offsetUnit(), QgsUnitTypes.RenderMapUnits)
        self.assertEqual(s.offsetMapUnitScale(), QgsMapUnitScale(5, 6))
        self.assertEqual(s.radii(), QSizeF(11, 12))
        self.assertEqual(s.radiiUnit(), QgsUnitTypes.RenderPercentage)
        self.assertEqual(s.radiiMapUnitScale(), QgsMapUnitScale(15, 16))
        self.assertEqual(s.fillColor(), QColor(255, 0, 0))
        self.assertEqual(s.strokeColor(), QColor(0, 255, 0))
        self.assertEqual(s.opacity(), 0.5)
        self.assertEqual(s.joinStyle(), Qt.RoundJoin)
        self.assertEqual(s.blendMode(), QPainter.CompositionMode_DestinationAtop)
        self.assertEqual(s.strokeWidth(), 7)
        self.assertEqual(s.strokeWidthUnit(), QgsUnitTypes.RenderPoints)
        self.assertEqual(s.strokeWidthMapUnitScale(), QgsMapUnitScale(25, 26))
        self.assertEqual(s.markerSymbol().color().name(), '#647086')

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
        s.setShadowPlacement(QgsTextShadowSettings.ShadowBuffer)
        s.setOffsetAngle(45)
        s.setOffsetDistance(75)
        s.setOffsetUnit(QgsUnitTypes.RenderMapUnits)
        s.setOffsetMapUnitScale(QgsMapUnitScale(5, 6))
        s.setOffsetGlobal(True)
        s.setBlurRadius(11)
        s.setBlurRadiusUnit(QgsUnitTypes.RenderPercentage)
        s.setBlurRadiusMapUnitScale(QgsMapUnitScale(15, 16))
        s.setBlurAlphaOnly(True)
        s.setColor(QColor(255, 0, 0))
        s.setOpacity(0.5)
        s.setScale(123)
        s.setBlendMode(QPainter.CompositionMode_DestinationAtop)
        return s

    def testShadowEquality(self):
        s = self.createShadowSettings()
        s2 = self.createShadowSettings()
        self.assertEqual(s, s2)

        s.setEnabled(False)
        self.assertNotEqual(s, s2)
        s = self.createShadowSettings()

        s.setShadowPlacement(QgsTextShadowSettings.ShadowText)
        self.assertNotEqual(s, s2)
        s = self.createShadowSettings()

        s.setOffsetAngle(145)
        self.assertNotEqual(s, s2)
        s = self.createShadowSettings()

        s.setOffsetDistance(175)
        self.assertNotEqual(s, s2)
        s = self.createShadowSettings()

        s.setOffsetUnit(QgsUnitTypes.RenderPixels)
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

        s.setBlurRadiusUnit(QgsUnitTypes.RenderPoints)
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

        s.setBlendMode(QPainter.CompositionMode_Darken)
        self.assertNotEqual(s, s2)

    def checkShadowSettings(self, s):
        """ test QgsTextShadowSettings """
        self.assertTrue(s.enabled())
        self.assertEqual(s.shadowPlacement(), QgsTextShadowSettings.ShadowBuffer)
        self.assertEqual(s.offsetAngle(), 45)
        self.assertEqual(s.offsetDistance(), 75)
        self.assertEqual(s.offsetUnit(), QgsUnitTypes.RenderMapUnits)
        self.assertEqual(s.offsetMapUnitScale(), QgsMapUnitScale(5, 6))
        self.assertTrue(s.offsetGlobal())
        self.assertEqual(s.blurRadius(), 11)
        self.assertEqual(s.blurRadiusUnit(), QgsUnitTypes.RenderPercentage)
        self.assertEqual(s.blurRadiusMapUnitScale(), QgsMapUnitScale(15, 16))
        self.assertTrue(s.blurAlphaOnly())
        self.assertEqual(s.color(), QColor(255, 0, 0))
        self.assertEqual(s.opacity(), 0.5)
        self.assertEqual(s.scale(), 123)
        self.assertEqual(s.blendMode(), QPainter.CompositionMode_DestinationAtop)

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
        s.background().setSvgFile('test.svg')
        s.shadow().setEnabled(True)
        s.shadow().setOffsetAngle(223)
        font = getTestFont()
        font.setKerning(False)
        s.setFont(font)
        s.setCapitalization(QgsStringUtils.TitleCase)
        s.setNamedStyle('Italic')
        s.setFamilies(['Arial', 'Comic Sans'])
        s.setSize(5)
        s.setSizeUnit(QgsUnitTypes.RenderPoints)
        s.setSizeMapUnitScale(QgsMapUnitScale(1, 2))
        s.setColor(QColor(255, 0, 0))
        s.setOpacity(0.5)
        s.setBlendMode(QPainter.CompositionMode_DestinationAtop)
        s.setLineHeight(5)
        s.setLineHeightUnit(QgsUnitTypes.RenderInches)
        s.setPreviewBackgroundColor(QColor(100, 150, 200))
        s.setOrientation(QgsTextFormat.VerticalOrientation)
        s.setAllowHtmlFormatting(True)
        s.setForcedBold(True)
        s.setForcedItalic(True)

        s.setStretchFactor(110)

        s.dataDefinedProperties().setProperty(QgsPalLayerSettings.Bold, QgsProperty.fromExpression('1>2'))
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

        s.background().setSvgFile('test2.svg')
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

        s.setNamedStyle('Bold')
        self.assertNotEqual(s, s2)
        s = self.createFormatSettings()

        s.setSize(15)
        self.assertNotEqual(s, s2)
        s = self.createFormatSettings()

        s.setSizeUnit(QgsUnitTypes.RenderPixels)
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

        s.setBlendMode(QPainter.CompositionMode_Darken)
        self.assertNotEqual(s, s2)
        s = self.createFormatSettings()

        s.setLineHeight(15)
        self.assertNotEqual(s, s2)
        s = self.createFormatSettings()

        s.setLineHeightUnit(QgsUnitTypes.RenderPoints)
        self.assertNotEqual(s, s2)
        s = self.createFormatSettings()

        s.setPreviewBackgroundColor(QColor(100, 250, 200))
        self.assertNotEqual(s, s2)
        s = self.createFormatSettings()

        s.setOrientation(QgsTextFormat.HorizontalOrientation)
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

        s.setCapitalization(QgsStringUtils.ForceFirstLetterToCapital)
        self.assertNotEqual(s, s2)
        s = self.createFormatSettings()

        s.dataDefinedProperties().setProperty(QgsPalLayerSettings.Bold, QgsProperty.fromExpression('1>3'))
        self.assertNotEqual(s, s2)

        s = self.createFormatSettings()
        s.setFamilies(['Times New Roman'])
        self.assertNotEqual(s, s2)

        s = self.createFormatSettings()
        s.setStretchFactor(120)
        self.assertNotEqual(s, s2)

    def checkTextFormat(self, s):
        """ test QgsTextFormat """
        self.assertTrue(s.buffer().enabled())
        self.assertEqual(s.buffer().size(), 25)
        self.assertTrue(s.mask().enabled())
        self.assertEqual(s.mask().size(), 32)
        self.assertTrue(s.background().enabled())
        self.assertEqual(s.background().svgFile(), 'test.svg')
        self.assertTrue(s.shadow().enabled())
        self.assertEqual(s.shadow().offsetAngle(), 223)
        self.assertEqual(s.font().family(), 'QGIS Vera Sans')
        self.assertEqual(s.families(), ['Arial', 'Comic Sans'])
        self.assertFalse(s.font().kerning())
        self.assertEqual(s.namedStyle(), 'Italic')
        self.assertEqual(s.size(), 5)
        self.assertEqual(s.sizeUnit(), QgsUnitTypes.RenderPoints)
        self.assertEqual(s.sizeMapUnitScale(), QgsMapUnitScale(1, 2))
        self.assertEqual(s.color(), QColor(255, 0, 0))
        self.assertEqual(s.opacity(), 0.5)
        self.assertEqual(s.blendMode(), QPainter.CompositionMode_DestinationAtop)
        self.assertEqual(s.lineHeight(), 5)
        self.assertEqual(s.lineHeightUnit(), QgsUnitTypes.RenderInches)
        self.assertEqual(s.previewBackgroundColor().name(), '#6496c8')
        self.assertEqual(s.orientation(), QgsTextFormat.VerticalOrientation)
        self.assertEqual(s.capitalization(), QgsStringUtils.TitleCase)
        self.assertTrue(s.allowHtmlFormatting())
        self.assertEqual(s.dataDefinedProperties().property(QgsPalLayerSettings.Bold).expressionString(), '1>2')
        self.assertTrue(s.forcedBold())
        self.assertTrue(s.forcedItalic())

        if int(QT_VERSION_STR.split('.')[0]) > 6 or (
                int(QT_VERSION_STR.split('.')[0]) == 6 and int(QT_VERSION_STR.split('.')[1]) >= 3):
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
        xml = xml.replace(QFont().family(), 'NOT A REAL FONT')
        doc = QDomDocument("testdoc")
        doc.setContent(xml)
        parent = doc.firstChildElement('settings')

        t = QgsTextFormat()
        t.readXml(parent, QgsReadWriteContext())
        # should be default font
        self.assertEqual(t.font().family(), QFont().family())

        format.setFamilies(['not real', 'still not real', getTestFont().family()])

        doc = QDomDocument("testdoc")
        elem = format.writeXml(doc, QgsReadWriteContext())
        parent = doc.createElement("settings")
        parent.appendChild(elem)
        doc.appendChild(parent)

        # swap out font name in xml to one which doesn't exist on system
        xml = doc.toString()
        xml = xml.replace(QFont().family(), 'NOT A REAL FONT')
        doc = QDomDocument("testdoc")
        doc.setContent(xml)
        parent = doc.firstChildElement('settings')

        t = QgsTextFormat()
        t.readXml(parent, QgsReadWriteContext())
        self.assertEqual(t.families(), ['not real', 'still not real', getTestFont().family()])
        # should have skipped the missing fonts and fallen back to the test font family entry, NOT the default application font!
        self.assertEqual(t.font().family(), getTestFont().family())

    def containsAdvancedEffects(self):
        t = QgsTextFormat()
        self.assertFalse(t.containsAdvancedEffects())
        t.setBlendMode(QPainter.CompositionMode_DestinationAtop)
        self.assertTrue(t.containsAdvancedEffects())

        t = QgsTextFormat()
        t.buffer().setBlendMode(QPainter.CompositionMode_DestinationAtop)
        self.assertFalse(t.containsAdvancedEffects())
        t.buffer().setEnabled(True)
        self.assertTrue(t.containsAdvancedEffects())
        t.buffer().setBlendMode(QPainter.CompositionMode_SourceOver)
        self.assertFalse(t.containsAdvancedEffects())

        t = QgsTextFormat()
        t.background().setBlendMode(QPainter.CompositionMode_DestinationAtop)
        self.assertFalse(t.containsAdvancedEffects())
        t.background().setEnabled(True)
        self.assertTrue(t.containsAdvancedEffects())
        t.background().setBlendMode(QPainter.CompositionMode_SourceOver)
        self.assertFalse(t.containsAdvancedEffects())

        t = QgsTextFormat()
        t.shadow().setBlendMode(QPainter.CompositionMode_DestinationAtop)
        self.assertFalse(t.containsAdvancedEffects())
        t.shadow().setEnabled(True)
        self.assertTrue(t.containsAdvancedEffects())
        t.shadow().setBlendMode(QPainter.CompositionMode_SourceOver)
        self.assertFalse(t.containsAdvancedEffects())

    def testDataDefinedBufferSettings(self):
        f = QgsTextFormat()
        context = QgsRenderContext()

        # buffer enabled
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.BufferDraw, QgsProperty.fromExpression('1'))
        f.updateDataDefinedProperties(context)
        self.assertTrue(f.buffer().enabled())
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.BufferDraw, QgsProperty.fromExpression('0'))
        context = QgsRenderContext()
        f.updateDataDefinedProperties(context)
        self.assertFalse(f.buffer().enabled())

        # buffer size
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.BufferSize, QgsProperty.fromExpression('7.8'))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.buffer().size(), 7.8)
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.BufferUnit, QgsProperty.fromExpression("'pixel'"))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.buffer().sizeUnit(), QgsUnitTypes.RenderPixels)

        # buffer opacity
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.BufferOpacity, QgsProperty.fromExpression('37'))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.buffer().opacity(), 0.37)

        # blend mode
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.BufferBlendMode, QgsProperty.fromExpression("'burn'"))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.buffer().blendMode(), QPainter.CompositionMode_ColorBurn)

        # join style
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.BufferJoinStyle,
                                              QgsProperty.fromExpression("'miter'"))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.buffer().joinStyle(), Qt.MiterJoin)

        # color
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.BufferColor, QgsProperty.fromExpression("'#ff0088'"))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.buffer().color().name(), '#ff0088')

    def testDataDefinedMaskSettings(self):
        f = QgsTextFormat()
        context = QgsRenderContext()

        # mask enabled
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.MaskEnabled, QgsProperty.fromExpression('1'))
        f.updateDataDefinedProperties(context)
        self.assertTrue(f.mask().enabled())
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.MaskEnabled, QgsProperty.fromExpression('0'))
        context = QgsRenderContext()
        f.updateDataDefinedProperties(context)
        self.assertFalse(f.mask().enabled())

        # mask buffer size
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.MaskBufferSize, QgsProperty.fromExpression('7.8'))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.mask().size(), 7.8)
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.MaskBufferUnit, QgsProperty.fromExpression("'pixel'"))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.mask().sizeUnit(), QgsUnitTypes.RenderPixels)

        # mask opacity
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.MaskOpacity, QgsProperty.fromExpression('37'))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.mask().opacity(), 0.37)

        # join style
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.MaskJoinStyle, QgsProperty.fromExpression("'miter'"))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.mask().joinStyle(), Qt.MiterJoin)

    def testDataDefinedBackgroundSettings(self):
        f = QgsTextFormat()
        context = QgsRenderContext()

        # background enabled
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.ShapeDraw, QgsProperty.fromExpression('1'))
        f.updateDataDefinedProperties(context)
        self.assertTrue(f.background().enabled())
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.ShapeDraw, QgsProperty.fromExpression('0'))
        context = QgsRenderContext()
        f.updateDataDefinedProperties(context)
        self.assertFalse(f.background().enabled())

        # background size
        f.background().setSize(QSizeF(13, 14))
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.ShapeSizeX, QgsProperty.fromExpression('7.8'))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.background().size().width(), 7.8)
        self.assertEqual(f.background().size().height(), 14)
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.ShapeSizeY, QgsProperty.fromExpression('17.8'))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.background().size().width(), 7.8)
        self.assertEqual(f.background().size().height(), 17.8)

        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.ShapeSizeUnits, QgsProperty.fromExpression("'pixel'"))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.background().sizeUnit(), QgsUnitTypes.RenderPixels)

        # shape kind
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.ShapeKind, QgsProperty.fromExpression("'square'"))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.background().type(), QgsTextBackgroundSettings.ShapeSquare)
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.ShapeKind, QgsProperty.fromExpression("'ellipse'"))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.background().type(), QgsTextBackgroundSettings.ShapeEllipse)
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.ShapeKind, QgsProperty.fromExpression("'circle'"))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.background().type(), QgsTextBackgroundSettings.ShapeCircle)
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.ShapeKind, QgsProperty.fromExpression("'svg'"))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.background().type(), QgsTextBackgroundSettings.ShapeSVG)
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.ShapeKind, QgsProperty.fromExpression("'marker'"))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.background().type(), QgsTextBackgroundSettings.ShapeMarkerSymbol)
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.ShapeKind, QgsProperty.fromExpression("'rect'"))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.background().type(), QgsTextBackgroundSettings.ShapeRectangle)

        # size type
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.ShapeSizeType, QgsProperty.fromExpression("'fixed'"))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.background().sizeType(), QgsTextBackgroundSettings.SizeFixed)
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.ShapeSizeType, QgsProperty.fromExpression("'buffer'"))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.background().sizeType(), QgsTextBackgroundSettings.SizeBuffer)

        # svg path
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.ShapeSVGFile, QgsProperty.fromExpression("'my.svg'"))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.background().svgFile(), 'my.svg')

        # shape rotation
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.ShapeRotation, QgsProperty.fromExpression('67'))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.background().rotation(), 67)
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.ShapeRotationType,
                                              QgsProperty.fromExpression("'offset'"))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.background().rotationType(), QgsTextBackgroundSettings.RotationOffset)
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.ShapeRotationType,
                                              QgsProperty.fromExpression("'fixed'"))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.background().rotationType(), QgsTextBackgroundSettings.RotationFixed)
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.ShapeRotationType,
                                              QgsProperty.fromExpression("'sync'"))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.background().rotationType(), QgsTextBackgroundSettings.RotationSync)

        # shape offset
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.ShapeOffset, QgsProperty.fromExpression("'7,9'"))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.background().offset(), QPointF(7, 9))
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.ShapeOffsetUnits,
                                              QgsProperty.fromExpression("'pixel'"))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.background().offsetUnit(), QgsUnitTypes.RenderPixels)

        # shape radii
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.ShapeRadii, QgsProperty.fromExpression("'18,19'"))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.background().radii(), QSizeF(18, 19))
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.ShapeRadiiUnits,
                                              QgsProperty.fromExpression("'pixel'"))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.background().radiiUnit(), QgsUnitTypes.RenderPixels)

        # shape opacity
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.ShapeOpacity, QgsProperty.fromExpression('37'))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.background().opacity(), 0.37)

        # color
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.ShapeFillColor,
                                              QgsProperty.fromExpression("'#ff0088'"))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.background().fillColor().name(), '#ff0088')
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.ShapeStrokeColor,
                                              QgsProperty.fromExpression("'#8800ff'"))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.background().strokeColor().name(), '#8800ff')

        # stroke width
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.ShapeStrokeWidth, QgsProperty.fromExpression('88'))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.background().strokeWidth(), 88)
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.ShapeStrokeWidthUnits,
                                              QgsProperty.fromExpression("'pixel'"))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.background().strokeWidthUnit(), QgsUnitTypes.RenderPixels)

        # blend mode
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.ShapeBlendMode, QgsProperty.fromExpression("'burn'"))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.background().blendMode(), QPainter.CompositionMode_ColorBurn)

        # join style
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.ShapeJoinStyle, QgsProperty.fromExpression("'miter'"))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.background().joinStyle(), Qt.MiterJoin)

    def testDataDefinedShadowSettings(self):
        f = QgsTextFormat()
        context = QgsRenderContext()

        # shadow enabled
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.ShadowDraw, QgsProperty.fromExpression('1'))
        f.updateDataDefinedProperties(context)
        self.assertTrue(f.shadow().enabled())
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.ShadowDraw, QgsProperty.fromExpression('0'))
        context = QgsRenderContext()
        f.updateDataDefinedProperties(context)
        self.assertFalse(f.shadow().enabled())

        # placement type
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.ShadowUnder, QgsProperty.fromExpression("'text'"))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.shadow().shadowPlacement(), QgsTextShadowSettings.ShadowText)
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.ShadowUnder, QgsProperty.fromExpression("'buffer'"))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.shadow().shadowPlacement(), QgsTextShadowSettings.ShadowBuffer)
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.ShadowUnder,
                                              QgsProperty.fromExpression("'background'"))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.shadow().shadowPlacement(), QgsTextShadowSettings.ShadowShape)
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.ShadowUnder, QgsProperty.fromExpression("'svg'"))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.shadow().shadowPlacement(), QgsTextShadowSettings.ShadowLowest)
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.ShadowUnder, QgsProperty.fromExpression("'lowest'"))

        # offset angle
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.ShadowOffsetAngle, QgsProperty.fromExpression('67'))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.shadow().offsetAngle(), 67)

        # offset distance
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.ShadowOffsetDist, QgsProperty.fromExpression('38'))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.shadow().offsetDistance(), 38)
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.ShadowOffsetUnits,
                                              QgsProperty.fromExpression("'pixel'"))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.shadow().offsetUnit(), QgsUnitTypes.RenderPixels)

        # radius
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.ShadowRadius, QgsProperty.fromExpression('58'))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.shadow().blurRadius(), 58)
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.ShadowRadiusUnits,
                                              QgsProperty.fromExpression("'pixel'"))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.shadow().blurRadiusUnit(), QgsUnitTypes.RenderPixels)

        # opacity
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.ShadowOpacity, QgsProperty.fromExpression('37'))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.shadow().opacity(), 0.37)

        # color
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.ShadowColor, QgsProperty.fromExpression("'#ff0088'"))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.shadow().color().name(), '#ff0088')

        # blend mode
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.ShadowBlendMode, QgsProperty.fromExpression("'burn'"))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.shadow().blendMode(), QPainter.CompositionMode_ColorBurn)

    def testDataDefinedFormatSettings(self):
        f = QgsTextFormat()
        font = f.font()
        font.setUnderline(True)
        font.setStrikeOut(True)
        font.setWordSpacing(5.7)
        font.setLetterSpacing(QFont.AbsoluteSpacing, 3.3)
        f.setFont(font)
        context = QgsRenderContext()

        # family
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.Family, QgsProperty.fromExpression(
            "'{}'".format(QgsFontUtils.getStandardTestFont().family())))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.font().family(), QgsFontUtils.getStandardTestFont().family())

        # style
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.FontStyle, QgsProperty.fromExpression("'Bold'"))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.font().styleName(), 'Bold')
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.FontStyle, QgsProperty.fromExpression("'Roman'"))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.font().styleName(), 'Roman')
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.Bold, QgsProperty.fromExpression("1"))
        f.updateDataDefinedProperties(context)
        self.assertTrue(f.font().bold())
        self.assertFalse(f.font().italic())
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.Bold, QgsProperty.fromExpression("0"))
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.Italic, QgsProperty.fromExpression("1"))
        f.updateDataDefinedProperties(context)
        self.assertFalse(f.font().bold())
        self.assertTrue(f.font().italic())
        self.assertTrue(f.font().underline())
        self.assertTrue(f.font().strikeOut())
        self.assertAlmostEqual(f.font().wordSpacing(), 5.7, 1)
        self.assertAlmostEqual(f.font().letterSpacing(), 3.3, 1)

        # underline
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.Underline, QgsProperty.fromExpression("0"))
        f.updateDataDefinedProperties(context)
        self.assertFalse(f.font().underline())
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.Underline, QgsProperty.fromExpression("1"))
        f.updateDataDefinedProperties(context)
        self.assertTrue(f.font().underline())

        # strikeout
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.Strikeout, QgsProperty.fromExpression("0"))
        f.updateDataDefinedProperties(context)
        self.assertFalse(f.font().strikeOut())
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.Strikeout, QgsProperty.fromExpression("1"))
        f.updateDataDefinedProperties(context)
        self.assertTrue(f.font().strikeOut())

        # color
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.Color, QgsProperty.fromExpression("'#ff0088'"))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.color().name(), '#ff0088')

        # size
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.Size, QgsProperty.fromExpression('38'))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.size(), 38)
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.FontSizeUnit, QgsProperty.fromExpression("'pixel'"))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.sizeUnit(), QgsUnitTypes.RenderPixels)

        # opacity
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.FontOpacity, QgsProperty.fromExpression('37'))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.opacity(), 0.37)

        # letter spacing
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.FontLetterSpacing, QgsProperty.fromExpression('58'))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.font().letterSpacing(), 58)

        # word spacing
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.FontWordSpacing, QgsProperty.fromExpression('8'))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.font().wordSpacing(), 8)

        # blend mode
        f.dataDefinedProperties().setProperty(QgsPalLayerSettings.FontBlendMode, QgsProperty.fromExpression("'burn'"))
        f.updateDataDefinedProperties(context)
        self.assertEqual(f.blendMode(), QPainter.CompositionMode_ColorBurn)

        if int(QT_VERSION_STR.split('.')[0]) > 6 or (
                int(QT_VERSION_STR.split('.')[0]) == 6 and int(QT_VERSION_STR.split('.')[1]) >= 3):
            # stretch
            f.dataDefinedProperties().setProperty(QgsPalLayerSettings.FontStretchFactor, QgsProperty.fromExpression("135"))
            f.updateDataDefinedProperties(context)
            self.assertEqual(f.stretchFactor(), 135)

    def testFontFoundFromLayer(self):
        layer = createEmptyLayer()
        layer.setCustomProperty('labeling/fontFamily', 'asdasd')
        f = QgsTextFormat()
        f.readFromLayer(layer)
        self.assertFalse(f.fontFound())

        font = getTestFont()
        layer.setCustomProperty('labeling/fontFamily', font.family())
        f.readFromLayer(layer)
        self.assertTrue(f.fontFound())

    def testFontFoundFromXml(self):
        doc = QDomDocument("testdoc")
        f = QgsTextFormat()
        elem = f.writeXml(doc, QgsReadWriteContext())
        elem.setAttribute('fontFamily', 'asdfasdfsadf')
        parent = doc.createElement("parent")
        parent.appendChild(elem)

        f.readXml(parent, QgsReadWriteContext())
        self.assertFalse(f.fontFound())

        font = getTestFont()
        elem.setAttribute('fontFamily', font.family())
        f.readXml(parent, QgsReadWriteContext())
        self.assertTrue(f.fontFound())

    def testFromQFont(self):
        qfont = getTestFont()
        qfont.setPointSizeF(16.5)
        qfont.setLetterSpacing(QFont.AbsoluteSpacing, 3)

        format = QgsTextFormat.fromQFont(qfont)
        self.assertEqual(format.font().family(), qfont.family())
        self.assertEqual(format.font().letterSpacing(), 3.0)
        self.assertEqual(format.size(), 16.5)
        self.assertEqual(format.sizeUnit(), QgsUnitTypes.RenderPoints)

        qfont.setPixelSize(12)
        format = QgsTextFormat.fromQFont(qfont)
        self.assertEqual(format.size(), 12.0)
        self.assertEqual(format.sizeUnit(), QgsUnitTypes.RenderPixels)

    def testToQFont(self):
        s = QgsTextFormat()
        f = getTestFont()
        f.setLetterSpacing(QFont.AbsoluteSpacing, 3)
        s.setFont(f)
        s.setNamedStyle('Italic')
        s.setSize(5.5)
        s.setSizeUnit(QgsUnitTypes.RenderPoints)

        qfont = s.toQFont()
        self.assertEqual(qfont.family(), f.family())
        self.assertEqual(qfont.pointSizeF(), 5.5)
        self.assertEqual(qfont.letterSpacing(), 3.0)

        s.setSize(5)
        s.setSizeUnit(QgsUnitTypes.RenderPixels)
        qfont = s.toQFont()
        self.assertEqual(qfont.pixelSize(), 5)

        s.setSize(5)
        s.setSizeUnit(QgsUnitTypes.RenderMillimeters)
        qfont = s.toQFont()
        self.assertAlmostEqual(qfont.pointSizeF(), 14.17, 2)

        s.setSizeUnit(QgsUnitTypes.RenderInches)
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

        if int(QT_VERSION_STR.split('.')[0]) > 6 or (
                int(QT_VERSION_STR.split('.')[0]) == 6 and int(QT_VERSION_STR.split('.')[1]) >= 3):
            s.setStretchFactor(115)
            qfont = s.toQFont()
            self.assertEqual(qfont.stretch(), 115)

    def testFontMetrics(self):
        """
        Test calculating font metrics from scaled text formats
        """
        s = QgsTextFormat()
        f = getTestFont()
        s.setFont(f)
        s.setSize(12)
        s.setSizeUnit(QgsUnitTypes.RenderPoints)

        string = 'xxxxxxxxxxxxxxxxxxxxxx'

        image = QImage(400, 400, QImage.Format_RGB32)
        painter = QPainter(image)
        context = QgsRenderContext.fromQPainter(painter)
        context.setScaleFactor(1)
        metrics = QgsTextRenderer.fontMetrics(context, s)
        context.setScaleFactor(2)
        metrics2 = QgsTextRenderer.fontMetrics(context, s)
        painter.end()

        self.assertAlmostEqual(metrics.width(string), 51.9, 1)
        self.assertAlmostEqual(metrics2.width(string), 104.15, 1)

    def imageCheck(self, name, reference_image, image):
        PyQgsTextRenderer.report += "<h2>Render {}</h2>\n".format(name)
        temp_dir = QDir.tempPath() + '/'
        file_name = temp_dir + name + ".png"
        image.save(file_name, "PNG")
        checker = QgsRenderChecker()
        checker.setControlPathPrefix("text_renderer")
        checker.setControlName(reference_image)
        checker.setRenderedImage(file_name)
        checker.setColorTolerance(2)
        result = checker.compareImages(name, 20)
        if checker.report():
            PyQgsTextRenderer.report += checker.report()
            print(checker.report())
        return result

    def checkRender(self, format, name, part=None, angle=0, alignment=QgsTextRenderer.AlignLeft,
                    text=['test'],
                    rect=QRectF(100, 100, 50, 250),
                    vAlignment=QgsTextRenderer.AlignTop,
                    flags=Qgis.TextRendererFlags(),
                    image_size=400,
                    mode=Qgis.TextLayoutMode.Rectangle):

        image = QImage(image_size, image_size, QImage.Format_RGB32)

        painter = QPainter()
        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 50, 50))
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI
        context.setFlag(QgsRenderContext.ApplyScalingWorkaroundForTextRendering, True)

        painter.begin(image)
        painter.setRenderHint(QPainter.Antialiasing)
        image.fill(QColor(152, 219, 249))

        painter.setBrush(QBrush(QColor(182, 239, 255)))
        painter.setPen(Qt.NoPen)
        # to highlight rect on image
        # painter.drawRect(rect)

        if part is not None:
            QgsTextRenderer.drawPart(rect,
                                     angle,
                                     alignment,
                                     text,
                                     context,
                                     format,
                                     part)
        else:
            QgsTextRenderer.drawText(rect,
                                     angle,
                                     alignment,
                                     text,
                                     context,
                                     format,
                                     vAlignment=vAlignment,
                                     flags=flags,
                                     mode=mode)

        painter.setFont(format.scaledFont(context))
        painter.setPen(QPen(QColor(255, 0, 255, 200)))
        # For comparison with QPainter's methods:
        # if alignment == QgsTextRenderer.AlignCenter:
        #     align = Qt.AlignHCenter
        # elif alignment == QgsTextRenderer.AlignRight:
        #     align = Qt.AlignRight
        # else:
        #     align = Qt.AlignLeft
        # painter.drawText(rect, align, '\n'.join(text))

        painter.end()
        return self.imageCheck(name, name, image)

    def checkRenderPoint(self, format, name, part=None, angle=0, alignment=QgsTextRenderer.AlignLeft,
                         text=['test'],
                         point=QPointF(100, 200),
                         image_size=400):
        image = QImage(image_size, image_size, QImage.Format_RGB32)

        painter = QPainter()
        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 50, 50))
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI

        painter.begin(image)
        painter.setRenderHint(QPainter.Antialiasing)
        image.fill(QColor(152, 219, 249))

        painter.setBrush(QBrush(QColor(182, 239, 255)))
        painter.setPen(Qt.NoPen)
        # to highlight point on image
        # painter.drawRect(QRectF(point.x() - 5, point.y() - 5, 10, 10))

        if part is not None:
            QgsTextRenderer.drawPart(point,
                                     angle,
                                     alignment,
                                     text,
                                     context,
                                     format,
                                     part)
        else:
            QgsTextRenderer.drawText(point,
                                     angle,
                                     alignment,
                                     text,
                                     context,
                                     format)

        painter.setFont(format.scaledFont(context))
        painter.setPen(QPen(QColor(255, 0, 255, 200)))
        # For comparison with QPainter's methods:
        # painter.drawText(point, '\n'.join(text))

        painter.end()
        return self.imageCheck(name, name, image)

    def testDrawMassiveFont(self):
        """
        Test that we aren't bitten by https://bugreports.qt.io/browse/QTBUG-98778

        This test should pass when there's a correct WORD space between the 'a' and 't' characters, or fail when
        the spacing between these characters is nill or close to a letter spacing
        """
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(1100)
        assert self.checkRender(format, 'massive_font', rect=QRectF(-800, -600, 1000, 1000), text=['a t'], image_size=800)

    def testDrawRectMixedHtml(self):
        """
        Test drawing text in rect mode with mixed html fonts
        """
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setAllowHtmlFormatting(True)
        format.setSize(30)
        assert self.checkRender(format, 'rect_html', rect=QRectF(100, 100, 100, 100), text=['first <span style="font-size:50pt">line</span>', 'second <span style="font-size:50pt">line</span>', 'third line'])

    def testDrawDocumentRect(self):
        """
        Test drawing text document in rect mode
        """
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setAllowHtmlFormatting(True)
        format.setSize(30)

        image = QImage(400, 400, QImage.Format_RGB32)

        painter = QPainter()
        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 50, 50))
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI
        context.setFlag(QgsRenderContext.ApplyScalingWorkaroundForTextRendering, True)

        painter.begin(image)
        painter.setRenderHint(QPainter.Antialiasing)
        image.fill(QColor(152, 219, 249))

        painter.setBrush(QBrush(QColor(182, 239, 255)))
        painter.setPen(Qt.NoPen)

        doc = QgsTextDocument.fromHtml(['first <span style="font-size:50pt">line</span>', 'second <span style="font-size:50pt">line</span>', 'third line'])

        metrics = QgsTextDocumentMetrics.calculateMetrics(doc, format, context, QgsTextRenderer.FONT_WORKAROUND_SCALE)

        QgsTextRenderer.drawDocument(QRectF(100, 100, 100, 100),
                                     format,
                                     doc,
                                     metrics,
                                     context,
                                     mode=Qgis.TextLayoutMode.Rectangle)

        painter.end()

        self.assertTrue(self.imageCheck('draw_document_rect', 'draw_document_rect', image))

    def testDrawRectCapHeightMode(self):
        """
        Test drawing text in rect mode with cap height based line heights
        """
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(30)
        assert self.checkRender(format, 'rect_cap_height_mode', rect=QRectF(100, 100, 100, 100), text=['first line', 'second line', 'third line'], mode=Qgis.TextLayoutMode.RectangleCapHeightBased)

    def testDrawRectCapHeightModeMixedHtml(self):
        """
        Test drawing text in rect mode with cap height based line heights
        """
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setAllowHtmlFormatting(True)
        format.setSize(30)
        assert self.checkRender(format, 'rect_cap_height_mode_html', rect=QRectF(100, 100, 100, 100), text=['first <span style="font-size:50pt">line</span>', 'second <span style="font-size:50pt">line</span>', 'third line'], mode=Qgis.TextLayoutMode.RectangleCapHeightBased)

    def testDrawDocumentRectCapHeightMode(self):
        """
        Test drawing text document in rect cap height mode
        """
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setAllowHtmlFormatting(True)
        format.setSize(30)

        image = QImage(400, 400, QImage.Format_RGB32)

        painter = QPainter()
        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 50, 50))
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI
        context.setFlag(QgsRenderContext.ApplyScalingWorkaroundForTextRendering, True)

        painter.begin(image)
        painter.setRenderHint(QPainter.Antialiasing)
        image.fill(QColor(152, 219, 249))

        painter.setBrush(QBrush(QColor(182, 239, 255)))
        painter.setPen(Qt.NoPen)

        doc = QgsTextDocument.fromHtml(['first <span style="font-size:50pt">line</span>', 'second <span style="font-size:50pt">line</span>', 'third line'])

        metrics = QgsTextDocumentMetrics.calculateMetrics(doc, format, context, QgsTextRenderer.FONT_WORKAROUND_SCALE)

        QgsTextRenderer.drawDocument(QRectF(100, 100, 100, 100),
                                     format,
                                     doc,
                                     metrics,
                                     context,
                                     mode=Qgis.TextLayoutMode.RectangleCapHeightBased)

        painter.end()

        self.assertTrue(self.imageCheck('draw_document_rect_cap_height', 'draw_document_rect_cap_height', image))

    def testDrawRectAscentMode(self):
        """
        Test drawing text in rect mode with cap height based line heights
        """
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(30)
        assert self.checkRender(format, 'rect_ascent_mode', rect=QRectF(100, 100, 100, 100), text=['first line', 'second line', 'third line'], mode=Qgis.TextLayoutMode.RectangleAscentBased)

    def testDrawRectAscentModeMixedHtml(self):
        """
        Test drawing text in rect mode with ascent based line heights
        """
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setAllowHtmlFormatting(True)
        format.setSize(30)
        assert self.checkRender(format, 'rect_ascent_mode_html', rect=QRectF(100, 100, 100, 100), text=['first <span style="font-size:50pt">line</span>', 'second <span style="font-size:50pt">line</span>', 'third line'], mode=Qgis.TextLayoutMode.RectangleAscentBased)

    def testDrawDocumentRectAscentMode(self):
        """
        Test drawing text document in rect ascent mode
        """
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setAllowHtmlFormatting(True)
        format.setSize(30)

        image = QImage(400, 400, QImage.Format_RGB32)

        painter = QPainter()
        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 50, 50))
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI
        context.setFlag(QgsRenderContext.ApplyScalingWorkaroundForTextRendering, True)

        painter.begin(image)
        painter.setRenderHint(QPainter.Antialiasing)
        image.fill(QColor(152, 219, 249))

        painter.setBrush(QBrush(QColor(182, 239, 255)))
        painter.setPen(Qt.NoPen)

        doc = QgsTextDocument.fromHtml(['first <span style="font-size:50pt">line</span>', 'second <span style="font-size:50pt">line</span>', 'third line'])

        metrics = QgsTextDocumentMetrics.calculateMetrics(doc, format, context, QgsTextRenderer.FONT_WORKAROUND_SCALE)

        QgsTextRenderer.drawDocument(QRectF(100, 100, 100, 100),
                                     format,
                                     doc,
                                     metrics,
                                     context,
                                     mode=Qgis.TextLayoutMode.RectangleAscentBased)

        painter.end()

        self.assertTrue(self.imageCheck('draw_document_rect_ascent', 'draw_document_rect_ascent', image))

    def testDrawDocumentShadowPlacement(self):
        """
        Test drawing text document with shadow placement lowest
        """
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setAllowHtmlFormatting(True)
        format.setSize(30)
        format.setColor(QColor(255, 255, 255))

        format.shadow().setEnabled(True)
        format.shadow().setShadowPlacement(QgsTextShadowSettings.ShadowLowest)
        format.shadow().setOpacity(1.0)
        format.shadow().setBlurRadius(0)
        format.shadow().setOffsetDistance(5)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderMillimeters)

        image = QImage(400, 400, QImage.Format_RGB32)

        painter = QPainter()
        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 50, 50))
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI
        context.setFlag(QgsRenderContext.ApplyScalingWorkaroundForTextRendering, True)

        painter.begin(image)
        painter.setRenderHint(QPainter.Antialiasing)
        image.fill(QColor(152, 219, 249))

        painter.setBrush(QBrush(QColor(182, 239, 255)))
        painter.setPen(Qt.NoPen)

        doc = QgsTextDocument.fromHtml(['first <span style="font-size:50pt">line</span>', 'second <span style="font-size:50pt">line</span>', 'third line'])

        metrics = QgsTextDocumentMetrics.calculateMetrics(doc, format, context, QgsTextRenderer.FONT_WORKAROUND_SCALE)

        QgsTextRenderer.drawDocument(QRectF(100, 100, 100, 100),
                                     format,
                                     doc,
                                     metrics,
                                     context,
                                     mode=Qgis.TextLayoutMode.RectangleAscentBased)

        painter.end()

        self.assertTrue(self.imageCheck('draw_document_shadow_lowest', 'draw_document_shadow_lowest', image))

    def testDrawForcedItalic(self):
        """
        Test drawing with forced italic
        """
        format = QgsTextFormat()
        format.setFont(getTestFont())
        format.setSize(30)
        format.setForcedItalic(True)
        assert self.checkRender(format, 'forced_italic', text=['Forced italic'])

    @unittest.skipIf(int(QT_VERSION_STR.split('.')[0]) < 6 or (int(QT_VERSION_STR.split('.')[0]) == 6 and int(QT_VERSION_STR.split('.')[1]) < 3), 'Too old Qt')
    def testDrawSmallCaps(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setCapitalization(Qgis.Capitalization.SmallCaps)
        format.setSize(30)
        assert self.checkRender(format, 'mixed_small_caps', text=['Small Caps'])

    @unittest.skipIf(int(QT_VERSION_STR.split('.')[0]) < 6 or (int(QT_VERSION_STR.split('.')[0]) == 6 and int(QT_VERSION_STR.split('.')[1]) < 3), 'Too old Qt')
    def testDrawAllSmallCaps(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(30)
        format.setCapitalization(Qgis.Capitalization.AllSmallCaps)
        assert self.checkRender(format, 'all_small_caps', text=['Small Caps'])

    @unittest.skipIf(int(QT_VERSION_STR.split('.')[0]) < 6 or (int(QT_VERSION_STR.split('.')[0]) == 6 and int(QT_VERSION_STR.split('.')[1]) < 3), 'Too old Qt')
    def testDrawStretch(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(30)
        format.setStretchFactor(150)
        assert self.checkRender(format, 'stretch_expand')

    @unittest.skipIf(int(QT_VERSION_STR.split('.')[0]) < 6 or (int(QT_VERSION_STR.split('.')[0]) == 6 and int(QT_VERSION_STR.split('.')[1]) < 3), 'Too old Qt')
    def testDrawStretchCondense(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(30)
        format.setStretchFactor(50)
        assert self.checkRender(format, 'stretch_condense')

    def testDrawBackgroundDisabled(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.background().setEnabled(False)
        assert self.checkRender(format, 'background_disabled', QgsTextRenderer.Background)

    def testDrawBackgroundRectangleFixedSizeMapUnits(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeRectangle)
        format.background().setSize(QSizeF(20, 10))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderMapUnits)
        assert self.checkRender(format, 'background_rect_mapunits', QgsTextRenderer.Background)

    def testDrawBackgroundRectangleFixedSizeWithRotatedText(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(40)
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeRectangle)
        format.background().setSize(QSizeF(20, 20))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderMillimeters)
        assert self.checkRenderPoint(format, 'background_rect_fixed_rotated_text', angle=3.141 / 4)

    def testDrawBackgroundRectangleBufferSizeWithRotatedText(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(40)
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeRectangle)
        format.background().setSize(QSizeF(2, 3))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeBuffer)
        format.background().setSizeUnit(QgsUnitTypes.RenderMillimeters)
        assert self.checkRenderPoint(format, 'background_rect_buffer_rotated_text', angle=3.141 / 4)

    def testDrawBackgroundRectangleMultilineFixedSizeMapUnits(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeRectangle)
        format.background().setSize(QSizeF(20, 10))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderMapUnits)
        assert self.checkRender(format, 'background_rect_multiline_mapunits', QgsTextRenderer.Background,
                                text=['test', 'multi', 'line'])

    def testDrawBackgroundPointMultilineFixedSizeMapUnits(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeRectangle)
        format.background().setSize(QSizeF(20, 10))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderMapUnits)
        assert self.checkRenderPoint(format, 'background_point_multiline_mapunits', QgsTextRenderer.Background,
                                     text=['test', 'multi', 'line'])

    def testDrawBackgroundRectangleMultilineBufferMapUnits(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeRectangle)
        format.background().setSize(QSizeF(4, 2))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeBuffer)
        format.background().setSizeUnit(QgsUnitTypes.RenderMapUnits)
        assert self.checkRender(format, 'background_rect_multiline_buffer_mapunits', QgsTextRenderer.Background,
                                text=['test', 'multi', 'line'])

    def testDrawBackgroundPointMultilineBufferMapUnits(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeRectangle)
        format.background().setSize(QSizeF(4, 2))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeBuffer)
        format.background().setSizeUnit(QgsUnitTypes.RenderMapUnits)
        assert self.checkRenderPoint(format, 'background_point_multiline_buffer_mapunits', QgsTextRenderer.Background,
                                     text=['test', 'multi', 'line'])

    def testDrawBackgroundPointFixedSizeMapUnits(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeRectangle)
        format.background().setSize(QSizeF(20, 10))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderMapUnits)
        assert self.checkRenderPoint(format, 'background_point_mapunits', QgsTextRenderer.Background,
                                     text=['Testy'])

    def testDrawBackgroundRectangleCenterAlignFixedSizeMapUnits(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeRectangle)
        format.background().setSize(QSizeF(20, 10))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderMapUnits)
        assert self.checkRender(format, 'background_rect_center_mapunits', QgsTextRenderer.Background,
                                alignment=QgsTextRenderer.AlignCenter)

    def testDrawBackgroundPointCenterAlignFixedSizeMapUnits(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeRectangle)
        format.background().setSize(QSizeF(20, 10))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderMapUnits)
        assert self.checkRenderPoint(format, 'background_point_center_mapunits', QgsTextRenderer.Background,
                                     alignment=QgsTextRenderer.AlignCenter)

    def testDrawBackgroundRectangleRightAlignFixedSizeMapUnits(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeRectangle)
        format.background().setSize(QSizeF(20, 10))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderMapUnits)
        assert self.checkRender(format, 'background_rect_right_mapunits', QgsTextRenderer.Background,
                                alignment=QgsTextRenderer.AlignRight)

    def testDrawBackgroundPointRightAlignFixedSizeMapUnits(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeRectangle)
        format.background().setSize(QSizeF(20, 10))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderMapUnits)
        assert self.checkRenderPoint(format, 'background_point_right_mapunits', QgsTextRenderer.Background,
                                     alignment=QgsTextRenderer.AlignRight)

    def testDrawBackgroundRectangleFixedSizeMM(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeRectangle)
        format.background().setSize(QSizeF(30, 20))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderMillimeters)
        assert self.checkRender(format, 'background_rect_mm', QgsTextRenderer.Background)

    def testDrawBackgroundRectangleFixedSizePixels(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeRectangle)
        format.background().setSize(QSizeF(60, 80))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderPixels)
        assert self.checkRender(format, 'background_rect_pixels', QgsTextRenderer.Background)

    def testDrawBackgroundRectBufferPixels(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.background().setEnabled(True)
        svg = os.path.join(
            svgSymbolsPath(), 'backgrounds', 'background_square.svg')
        format.background().setSvgFile(svg)
        format.background().setType(QgsTextBackgroundSettings.ShapeRectangle)
        format.background().setSize(QSizeF(30, 50))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeBuffer)
        format.background().setSizeUnit(QgsUnitTypes.RenderPixels)
        assert self.checkRender(format, 'background_rect_buffer_pixels', QgsTextRenderer.Background,
                                rect=QRectF(100, 100, 100, 100))

    def testDrawBackgroundRectRightAlignBufferPixels(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.background().setEnabled(True)
        svg = os.path.join(
            svgSymbolsPath(), 'backgrounds', 'background_square.svg')
        format.background().setSvgFile(svg)
        format.background().setType(QgsTextBackgroundSettings.ShapeRectangle)
        format.background().setSize(QSizeF(30, 50))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeBuffer)
        format.background().setSizeUnit(QgsUnitTypes.RenderPixels)
        assert self.checkRender(format, 'background_rect_right_buffer_pixels', QgsTextRenderer.Background,
                                alignment=QgsTextRenderer.AlignRight,
                                rect=QRectF(100, 100, 100, 100))

    def testDrawBackgroundRectCenterAlignBufferPixels(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.background().setEnabled(True)
        svg = os.path.join(
            svgSymbolsPath(), 'backgrounds', 'background_square.svg')
        format.background().setSvgFile(svg)
        format.background().setType(QgsTextBackgroundSettings.ShapeRectangle)
        format.background().setSize(QSizeF(30, 50))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeBuffer)
        format.background().setSizeUnit(QgsUnitTypes.RenderPixels)
        assert self.checkRender(format, 'background_rect_center_buffer_pixels', QgsTextRenderer.Background,
                                alignment=QgsTextRenderer.AlignCenter,
                                rect=QRectF(100, 100, 100, 100))

    def testDrawBackgroundPointBufferPixels(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.background().setEnabled(True)
        svg = os.path.join(
            svgSymbolsPath(), 'backgrounds', 'background_square.svg')
        format.background().setSvgFile(svg)
        format.background().setType(QgsTextBackgroundSettings.ShapeRectangle)
        format.background().setSize(QSizeF(30, 50))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeBuffer)
        format.background().setSizeUnit(QgsUnitTypes.RenderPixels)
        assert self.checkRenderPoint(format, 'background_point_buffer_pixels', QgsTextRenderer.Background,
                                     point=QPointF(100, 100))

    def testDrawBackgroundPointRightAlignBufferPixels(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.background().setEnabled(True)
        svg = os.path.join(
            svgSymbolsPath(), 'backgrounds', 'background_square.svg')
        format.background().setSvgFile(svg)
        format.background().setType(QgsTextBackgroundSettings.ShapeRectangle)
        format.background().setSize(QSizeF(30, 50))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeBuffer)
        format.background().setSizeUnit(QgsUnitTypes.RenderPixels)
        assert self.checkRenderPoint(format, 'background_point_right_buffer_pixels', QgsTextRenderer.Background,
                                     alignment=QgsTextRenderer.AlignRight,
                                     point=QPointF(100, 100))

    def testDrawBackgroundPointCenterAlignBufferPixels(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.background().setEnabled(True)
        svg = os.path.join(
            svgSymbolsPath(), 'backgrounds', 'background_square.svg')
        format.background().setSvgFile(svg)
        format.background().setType(QgsTextBackgroundSettings.ShapeRectangle)
        format.background().setSize(QSizeF(30, 50))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeBuffer)
        format.background().setSizeUnit(QgsUnitTypes.RenderPixels)
        assert self.checkRenderPoint(format, 'background_point_center_buffer_pixels', QgsTextRenderer.Background,
                                     alignment=QgsTextRenderer.AlignCenter,
                                     point=QPointF(100, 100))

    def testDrawBackgroundRectBufferMapUnits(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.background().setEnabled(True)
        svg = os.path.join(
            svgSymbolsPath(), 'backgrounds', 'background_square.svg')
        format.background().setSvgFile(svg)
        format.background().setType(QgsTextBackgroundSettings.ShapeRectangle)
        format.background().setSize(QSizeF(4, 6))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeBuffer)
        format.background().setSizeUnit(QgsUnitTypes.RenderMapUnits)
        assert self.checkRender(format, 'background_rect_buffer_mapunits', QgsTextRenderer.Background,
                                rect=QRectF(100, 100, 100, 100))

    def testDrawBackgroundRectBufferMM(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.background().setEnabled(True)
        svg = os.path.join(
            svgSymbolsPath(), 'backgrounds', 'background_square.svg')
        format.background().setSvgFile(svg)
        format.background().setType(QgsTextBackgroundSettings.ShapeRectangle)
        format.background().setSize(QSizeF(10, 16))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeBuffer)
        format.background().setSizeUnit(QgsUnitTypes.RenderMillimeters)
        assert self.checkRender(format, 'background_rect_buffer_mm', QgsTextRenderer.Background,
                                rect=QRectF(100, 100, 100, 100))

    def testDrawBackgroundEllipse(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeEllipse)
        format.background().setSize(QSizeF(60, 80))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderPixels)
        assert self.checkRender(format, 'background_ellipse_pixels', QgsTextRenderer.Background)

    def testDrawBackgroundSvgFixedPixels(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.background().setEnabled(True)
        svg = os.path.join(
            svgSymbolsPath(), 'backgrounds', 'background_square.svg')
        format.background().setSvgFile(svg)
        format.background().setType(QgsTextBackgroundSettings.ShapeSVG)
        format.background().setSize(QSizeF(60, 80))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderPixels)
        assert self.checkRender(format, 'background_svg_fixed_pixels', QgsTextRenderer.Background)

    def testDrawBackgroundSvgFixedMapUnits(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.background().setEnabled(True)
        svg = os.path.join(
            svgSymbolsPath(), 'backgrounds', 'background_square.svg')
        format.background().setSvgFile(svg)
        format.background().setType(QgsTextBackgroundSettings.ShapeSVG)
        format.background().setSize(QSizeF(20, 20))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderMapUnits)
        assert self.checkRender(format, 'background_svg_fixed_mapunits', QgsTextRenderer.Background)

    def testDrawBackgroundSvgFixedMM(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.background().setEnabled(True)
        svg = os.path.join(
            svgSymbolsPath(), 'backgrounds', 'background_square.svg')
        format.background().setSvgFile(svg)
        format.background().setType(QgsTextBackgroundSettings.ShapeSVG)
        format.background().setSize(QSizeF(30, 30))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderMillimeters)
        assert self.checkRender(format, 'background_svg_fixed_mm', QgsTextRenderer.Background)

    def testDrawBackgroundRotationSynced(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeRectangle)
        format.background().setSize(QSizeF(30, 20))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderMillimeters)
        format.background().setRotation(45)  # should be ignored
        format.background().setRotationType(QgsTextBackgroundSettings.RotationSync)
        assert self.checkRender(format, 'background_rotation_sync', QgsTextRenderer.Background, angle=20)

    def testDrawBackgroundSvgBufferPixels(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.background().setEnabled(True)
        svg = os.path.join(
            svgSymbolsPath(), 'backgrounds', 'background_square.svg')
        format.background().setSvgFile(svg)
        format.background().setType(QgsTextBackgroundSettings.ShapeSVG)
        format.background().setSize(QSizeF(30, 30))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeBuffer)
        format.background().setSizeUnit(QgsUnitTypes.RenderPixels)
        assert self.checkRender(format, 'background_svg_buffer_pixels', QgsTextRenderer.Background,
                                rect=QRectF(100, 100, 100, 100))

    def testDrawBackgroundSvgBufferMapUnits(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.background().setEnabled(True)
        svg = os.path.join(
            svgSymbolsPath(), 'backgrounds', 'background_square.svg')
        format.background().setSvgFile(svg)
        format.background().setType(QgsTextBackgroundSettings.ShapeSVG)
        format.background().setSize(QSizeF(4, 4))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeBuffer)
        format.background().setSizeUnit(QgsUnitTypes.RenderMapUnits)
        assert self.checkRender(format, 'background_svg_buffer_mapunits', QgsTextRenderer.Background,
                                rect=QRectF(100, 100, 100, 100))

    def testDrawBackgroundSvgBufferMM(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.background().setEnabled(True)
        svg = os.path.join(
            svgSymbolsPath(), 'backgrounds', 'background_square.svg')
        format.background().setSvgFile(svg)
        format.background().setType(QgsTextBackgroundSettings.ShapeSVG)
        format.background().setSize(QSizeF(10, 10))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeBuffer)
        format.background().setSizeUnit(QgsUnitTypes.RenderMillimeters)
        assert self.checkRender(format, 'background_svg_buffer_mm', QgsTextRenderer.Background,
                                rect=QRectF(100, 100, 100, 100))

    def testDrawBackgroundMarkerFixedPixels(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.background().setEnabled(True)
        format.background().setMarkerSymbol(QgsMarkerSymbol.createSimple(
            {'color': '#ffffff', 'size': '3', 'outline_color': 'red', 'outline_width': '3'}))
        format.background().setType(QgsTextBackgroundSettings.ShapeMarkerSymbol)
        format.background().setSize(QSizeF(60, 80))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderPixels)
        assert self.checkRender(format, 'background_marker_fixed_pixels', QgsTextRenderer.Background)

    def testDrawBackgroundMarkerFixedMapUnits(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.background().setEnabled(True)
        format.background().setMarkerSymbol(QgsMarkerSymbol.createSimple(
            {'color': '#ffffff', 'size': '3', 'outline_color': 'red', 'outline_width': '3'}))
        format.background().setType(QgsTextBackgroundSettings.ShapeMarkerSymbol)
        format.background().setSize(QSizeF(20, 20))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderMapUnits)
        assert self.checkRender(format, 'background_marker_fixed_mapunits', QgsTextRenderer.Background)

    def testDrawBackgroundMarkerFixedMM(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.background().setEnabled(True)
        format.background().setMarkerSymbol(QgsMarkerSymbol.createSimple(
            {'color': '#ffffff', 'size': '3', 'outline_color': 'red', 'outline_width': '3'}))
        format.background().setType(QgsTextBackgroundSettings.ShapeMarkerSymbol)
        format.background().setSize(QSizeF(30, 30))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderMillimeters)
        assert self.checkRender(format, 'background_marker_fixed_mm', QgsTextRenderer.Background)

    def testDrawBackgroundMarkerBufferPixels(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.background().setEnabled(True)
        format.background().setMarkerSymbol(QgsMarkerSymbol.createSimple(
            {'color': '#ffffff', 'size': '3', 'outline_color': 'red', 'outline_width': '3'}))
        format.background().setType(QgsTextBackgroundSettings.ShapeMarkerSymbol)
        format.background().setSize(QSizeF(30, 30))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeBuffer)
        format.background().setSizeUnit(QgsUnitTypes.RenderPixels)
        assert self.checkRender(format, 'background_marker_buffer_pixels', QgsTextRenderer.Background,
                                rect=QRectF(100, 100, 100, 100))

    def testDrawBackgroundMarkerBufferMapUnits(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.background().setEnabled(True)
        format.background().setMarkerSymbol(QgsMarkerSymbol.createSimple(
            {'color': '#ffffff', 'size': '3', 'outline_color': 'red', 'outline_width': '3'}))
        format.background().setType(QgsTextBackgroundSettings.ShapeMarkerSymbol)
        format.background().setSize(QSizeF(4, 4))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeBuffer)
        format.background().setSizeUnit(QgsUnitTypes.RenderMapUnits)
        assert self.checkRender(format, 'background_marker_buffer_mapunits', QgsTextRenderer.Background,
                                rect=QRectF(100, 100, 100, 100))

    def testDrawBackgroundMarkerBufferMM(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.background().setEnabled(True)
        format.background().setMarkerSymbol(QgsMarkerSymbol.createSimple(
            {'color': '#ffffff', 'size': '3', 'outline_color': 'red', 'outline_width': '3'}))
        format.background().setType(QgsTextBackgroundSettings.ShapeMarkerSymbol)
        format.background().setSize(QSizeF(10, 10))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeBuffer)
        format.background().setSizeUnit(QgsUnitTypes.RenderMillimeters)
        assert self.checkRender(format, 'background_marker_buffer_mm', QgsTextRenderer.Background,
                                rect=QRectF(100, 100, 100, 100))

    def testDrawBackgroundRotationFixed(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeRectangle)
        format.background().setSize(QSizeF(30, 20))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderMillimeters)
        format.background().setRotation(45)
        format.background().setRotationType(QgsTextBackgroundSettings.RotationFixed)
        assert self.checkRender(format, 'background_rotation_fixed', QgsTextRenderer.Background, angle=20)

    def testDrawRotationOffset(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeRectangle)
        format.background().setSize(QSizeF(30, 20))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderMillimeters)
        format.background().setRotation(45)
        format.background().setRotationType(QgsTextBackgroundSettings.RotationOffset)
        assert self.checkRender(format, 'background_rotation_offset', QgsTextRenderer.Background, angle=20)

    def testDrawBackgroundOffsetMM(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeRectangle)
        format.background().setSize(QSizeF(30, 20))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderMillimeters)
        format.background().setOffset(QPointF(30, 20))
        format.background().setOffsetUnit(QgsUnitTypes.RenderMillimeters)
        assert self.checkRender(format, 'background_offset_mm', QgsTextRenderer.Background)

    def testDrawBackgroundOffsetMapUnits(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeRectangle)
        format.background().setSize(QSizeF(30, 20))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderMillimeters)
        format.background().setOffset(QPointF(10, 5))
        format.background().setOffsetUnit(QgsUnitTypes.RenderMapUnits)
        assert self.checkRender(format, 'background_offset_mapunits', QgsTextRenderer.Background)

    def testDrawBackgroundRadiiMM(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeRectangle)
        format.background().setSize(QSizeF(30, 20))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderMillimeters)
        format.background().setRadii(QSizeF(6, 4))
        format.background().setRadiiUnit(QgsUnitTypes.RenderMillimeters)
        assert self.checkRender(format, 'background_radii_mm', QgsTextRenderer.Background)

    def testDrawBackgroundRadiiMapUnits(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeRectangle)
        format.background().setSize(QSizeF(30, 20))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderMillimeters)
        format.background().setRadii(QSizeF(3, 2))
        format.background().setRadiiUnit(QgsUnitTypes.RenderMapUnits)
        assert self.checkRender(format, 'background_radii_mapunits', QgsTextRenderer.Background)

    def testDrawBackgroundOpacity(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeRectangle)
        format.background().setSize(QSizeF(30, 20))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeFixed)
        format.background().setOpacity(0.6)
        assert self.checkRender(format, 'background_opacity', QgsTextRenderer.Background)

    def testDrawBackgroundFillColor(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeRectangle)
        format.background().setSize(QSizeF(30, 20))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeFixed)
        format.background().setFillColor(QColor(50, 100, 50))
        assert self.checkRender(format, 'background_fillcolor', QgsTextRenderer.Background)

    def testDrawBackgroundFillSymbol(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeRectangle)
        format.background().setSize(QSizeF(30, 20))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeFixed)
        format.background().setFillColor(QColor(255, 0, 0))
        fill = QgsSimpleFillSymbolLayer()
        fill_symbol = QgsFillSymbol()
        fill_symbol.changeSymbolLayer(0, fill)
        fill.setColor(QColor(0, 255, 0, 25))
        fill.setStrokeColor(QColor(0, 0, 255))
        fill.setStrokeWidth(6)
        format.background().setFillSymbol(fill_symbol)

        assert self.checkRender(format, 'background_fillsymbol', QgsTextRenderer.Background)

    def testDrawBackgroundStroke(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeRectangle)
        format.background().setSize(QSizeF(30, 20))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeFixed)
        format.background().setStrokeColor(QColor(50, 100, 50))
        format.background().setStrokeWidth(3)
        format.background().setStrokeWidthUnit(QgsUnitTypes.RenderMillimeters)
        assert self.checkRender(format, 'background_outline', QgsTextRenderer.Background)

    def testDrawBackgroundEffect(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeRectangle)
        format.background().setSize(QSizeF(30, 20))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeFixed)
        format.background().setPaintEffect(QgsBlurEffect.create({'blur_level': '10', 'enabled': '1'}))
        assert self.checkRender(format, 'background_effect', QgsTextRenderer.Background, text=['test'])

    def testDrawText(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        assert self.checkRender(format, 'text_bold', QgsTextRenderer.Text, text=['test'])

    def testDrawTextPoint(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        assert self.checkRenderPoint(format, 'text_point_bold', QgsTextRenderer.Text, text=['test'])

    def testDrawTextNamedStyle(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        # need to call getTestFont to make sure font style is installed and ready to go
        temp_font = getTestFont('Bold Oblique')  # NOQA
        format.setFont(getTestFont())
        format.setNamedStyle('Bold Oblique')
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        assert self.checkRender(format, 'text_named_style', QgsTextRenderer.Text, text=['test'])

    def testDrawTextColor(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setColor(QColor(0, 255, 0))
        assert self.checkRender(format, 'text_color', QgsTextRenderer.Text, text=['test'])

    def testDrawTextOpacity(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setOpacity(0.7)
        assert self.checkRender(format, 'text_opacity', QgsTextRenderer.Text, text=['test'])

    def testDrawTextBlendMode(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setColor(QColor(100, 100, 100))
        format.setBlendMode(QPainter.CompositionMode_Difference)
        assert self.checkRender(format, 'text_blend_mode', QgsTextRenderer.Text, text=['test'])

    def testDrawTextAngle(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        assert self.checkRender(format, 'text_angled', QgsTextRenderer.Text, angle=90 / 180 * 3.141, text=['test'])

    def testDrawTextMapUnits(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(5)
        format.setSizeUnit(QgsUnitTypes.RenderMapUnits)
        assert self.checkRender(format, 'text_mapunits', QgsTextRenderer.Text, text=['test'])

    def testDrawTextPixels(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(50)
        format.setSizeUnit(QgsUnitTypes.RenderPixels)
        assert self.checkRender(format, 'text_pixels', QgsTextRenderer.Text, text=['test'])

    def testDrawMultiLineText(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        assert self.checkRender(format, 'text_multiline', QgsTextRenderer.Text, text=['test', 'multi', 'line'])

    def testDrawMultiLineTextPoint(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        assert self.checkRenderPoint(format, 'text_point_multiline', QgsTextRenderer.Text,
                                     text=['test', 'multi', 'line'])

    def testDrawLineHeightText(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setLineHeight(1.5)
        assert self.checkRender(format, 'text_line_height', QgsTextRenderer.Text, text=['test', 'multi', 'line'])

    def testDrawLineHeightAbsolute(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setLineHeight(20)
        format.setLineHeightUnit(QgsUnitTypes.RenderPoints)
        assert self.checkRender(format, 'text_line_absolute_height', QgsTextRenderer.Text, text=['test', 'multi', 'line'])

    def testDrawLineHeightAbsolute(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setLineHeight(20)
        format.setLineHeightUnit(QgsUnitTypes.RenderMillimeters)
        assert self.checkRender(format, 'text_line_absolute_mm_height', QgsTextRenderer.Text, text=['test', 'multi', 'line'])

    def testDrawBufferSizeMM(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.buffer().setEnabled(True)
        format.buffer().setSize(2)
        format.buffer().setSizeUnit(QgsUnitTypes.RenderMillimeters)
        assert self.checkRender(format, 'text_buffer_mm', QgsTextRenderer.Buffer, text=['test'])

    def testDrawBufferDisabled(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.buffer().setEnabled(False)
        assert self.checkRender(format, 'text_disabled_buffer', QgsTextRenderer.Buffer, text=['test'])

    def testDrawBufferSizeMapUnits(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.buffer().setEnabled(True)
        format.buffer().setSize(2)
        format.buffer().setSizeUnit(QgsUnitTypes.RenderMapUnits)
        assert self.checkRender(format, 'text_buffer_mapunits', QgsTextRenderer.Buffer, text=['test'])

    def testDrawBufferSizePixels(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.buffer().setEnabled(True)
        format.buffer().setSize(10)
        format.buffer().setSizeUnit(QgsUnitTypes.RenderPixels)
        assert self.checkRender(format, 'text_buffer_pixels', QgsTextRenderer.Buffer, text=['test'])

    def testDrawBufferSizePercentage(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.buffer().setEnabled(True)
        format.buffer().setSize(10)
        format.buffer().setSizeUnit(QgsUnitTypes.RenderPercentage)
        assert self.checkRender(format, 'text_buffer_percentage', QgsTextRenderer.Buffer, text=['test'])

    def testDrawBufferColor(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.buffer().setEnabled(True)
        format.buffer().setSize(2)
        format.buffer().setSizeUnit(QgsUnitTypes.RenderMillimeters)
        format.buffer().setColor(QColor(0, 255, 0))
        assert self.checkRender(format, 'text_buffer_color', QgsTextRenderer.Buffer, text=['test'])

    def testDrawBufferOpacity(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.buffer().setEnabled(True)
        format.buffer().setSize(2)
        format.buffer().setSizeUnit(QgsUnitTypes.RenderMillimeters)
        format.buffer().setOpacity(0.5)
        assert self.checkRender(format, 'text_buffer_opacity', QgsTextRenderer.Buffer, text=['test'])

    def testDrawBufferFillInterior(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.buffer().setEnabled(True)
        format.buffer().setSize(2)
        format.buffer().setSizeUnit(QgsUnitTypes.RenderMillimeters)
        format.buffer().setFillBufferInterior(True)
        assert self.checkRender(format, 'text_buffer_interior', QgsTextRenderer.Buffer, text=['test'])

    def testDrawBufferEffect(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.buffer().setEnabled(True)
        format.buffer().setSize(2)
        format.buffer().setSizeUnit(QgsUnitTypes.RenderMillimeters)
        format.buffer().setPaintEffect(QgsBlurEffect.create({'blur_level': '10', 'enabled': '1'}))
        assert self.checkRender(format, 'text_buffer_effect', QgsTextRenderer.Buffer, text=['test'])

    def testDrawShadow(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setColor(QColor(255, 255, 255))
        format.shadow().setEnabled(True)
        format.shadow().setShadowPlacement(QgsTextShadowSettings.ShadowText)
        format.shadow().setOpacity(1.0)
        format.shadow().setBlurRadius(0)
        format.shadow().setOffsetDistance(5)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderMillimeters)
        assert self.checkRender(format, 'shadow_enabled', QgsTextRenderer.Text, text=['test'])

    def testDrawShadowOffsetAngle(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setColor(QColor(255, 255, 255))
        format.shadow().setEnabled(True)
        format.shadow().setShadowPlacement(QgsTextShadowSettings.ShadowText)
        format.shadow().setOpacity(1.0)
        format.shadow().setBlurRadius(0)
        format.shadow().setOffsetDistance(5)
        format.shadow().setOffsetAngle(0)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderMillimeters)
        assert self.checkRender(format, 'shadow_offset_angle', QgsTextRenderer.Text, text=['test'])

    def testDrawShadowOffsetMapUnits(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setColor(QColor(255, 255, 255))
        format.shadow().setEnabled(True)
        format.shadow().setShadowPlacement(QgsTextShadowSettings.ShadowText)
        format.shadow().setOpacity(1.0)
        format.shadow().setBlurRadius(0)
        format.shadow().setOffsetDistance(10)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderMapUnits)
        assert self.checkRender(format, 'shadow_offset_mapunits', QgsTextRenderer.Text, text=['test'])

    def testDrawShadowOffsetPixels(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setColor(QColor(255, 255, 255))
        format.shadow().setEnabled(True)
        format.shadow().setShadowPlacement(QgsTextShadowSettings.ShadowText)
        format.shadow().setOpacity(1.0)
        format.shadow().setBlurRadius(0)
        format.shadow().setOffsetDistance(10)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderPixels)
        assert self.checkRender(format, 'shadow_offset_pixels', QgsTextRenderer.Text, text=['test'])

    def testDrawShadowOffsetPercentage(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setColor(QColor(255, 255, 255))
        format.shadow().setEnabled(True)
        format.shadow().setShadowPlacement(QgsTextShadowSettings.ShadowText)
        format.shadow().setOpacity(1.0)
        format.shadow().setBlurRadius(0)
        format.shadow().setOffsetDistance(10)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderPercentage)
        assert self.checkRender(format, 'shadow_offset_percentage', QgsTextRenderer.Text, text=['test'])

    def testDrawShadowBlurRadiusMM(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setColor(QColor(255, 255, 255))
        format.shadow().setEnabled(True)
        format.shadow().setShadowPlacement(QgsTextShadowSettings.ShadowText)
        format.shadow().setOpacity(1.0)
        format.shadow().setOffsetDistance(5)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderMillimeters)
        format.shadow().setBlurRadius(1)
        format.shadow().setBlurRadiusUnit(QgsUnitTypes.RenderMillimeters)
        assert self.checkRender(format, 'shadow_radius_mm', QgsTextRenderer.Text, text=['test'])

    def testDrawShadowBlurRadiusMapUnits(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setColor(QColor(255, 255, 255))
        format.shadow().setEnabled(True)
        format.shadow().setShadowPlacement(QgsTextShadowSettings.ShadowText)
        format.shadow().setOpacity(1.0)
        format.shadow().setOffsetDistance(5)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderMillimeters)
        format.shadow().setBlurRadius(3)
        format.shadow().setBlurRadiusUnit(QgsUnitTypes.RenderMapUnits)
        assert self.checkRender(format, 'shadow_radius_mapunits', QgsTextRenderer.Text, text=['test'])

    def testDrawShadowBlurRadiusPixels(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setColor(QColor(255, 255, 255))
        format.shadow().setEnabled(True)
        format.shadow().setShadowPlacement(QgsTextShadowSettings.ShadowText)
        format.shadow().setOpacity(1.0)
        format.shadow().setOffsetDistance(5)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderMillimeters)
        format.shadow().setBlurRadius(3)
        format.shadow().setBlurRadiusUnit(QgsUnitTypes.RenderPixels)
        assert self.checkRender(format, 'shadow_radius_pixels', QgsTextRenderer.Text, text=['test'])

    def testDrawShadowBlurRadiusPercentage(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setColor(QColor(255, 255, 255))
        format.shadow().setEnabled(True)
        format.shadow().setShadowPlacement(QgsTextShadowSettings.ShadowText)
        format.shadow().setOpacity(1.0)
        format.shadow().setOffsetDistance(5)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderMillimeters)
        format.shadow().setBlurRadius(5)
        format.shadow().setBlurRadiusUnit(QgsUnitTypes.RenderPercentage)
        assert self.checkRender(format, 'shadow_radius_percentage', QgsTextRenderer.Text, text=['test'])

    def testDrawShadowOpacity(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setColor(QColor(255, 255, 255))
        format.shadow().setEnabled(True)
        format.shadow().setShadowPlacement(QgsTextShadowSettings.ShadowText)
        format.shadow().setOpacity(0.5)
        format.shadow().setBlurRadius(0)
        format.shadow().setOffsetDistance(5)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderMillimeters)
        assert self.checkRender(format, 'shadow_opacity', QgsTextRenderer.Text, text=['test'])

    def testDrawShadowColor(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setColor(QColor(255, 255, 255))
        format.shadow().setEnabled(True)
        format.shadow().setShadowPlacement(QgsTextShadowSettings.ShadowText)
        format.shadow().setColor(QColor(255, 255, 0))
        format.shadow().setBlurRadius(0)
        format.shadow().setOffsetDistance(5)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderMillimeters)
        assert self.checkRender(format, 'shadow_color', QgsTextRenderer.Text, text=['test'])

    def testDrawShadowWithJustifyAlign(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.shadow().setEnabled(True)
        format.shadow().setShadowPlacement(QgsTextShadowSettings.ShadowText)
        format.shadow().setOpacity(0.5)
        format.shadow().setBlurRadius(0)
        format.shadow().setOffsetDistance(5)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderMillimeters)
        assert self.checkRender(format, 'text_justify_aligned_with_shadow',
                                text=['a t est', 'off', 'justification', 'align'],
                                alignment=QgsTextRenderer.AlignJustify, rect=QRectF(100, 100, 200, 100))

    def testDrawShadowScale(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setColor(QColor(255, 255, 255))
        format.shadow().setEnabled(True)
        format.shadow().setShadowPlacement(QgsTextShadowSettings.ShadowText)
        format.shadow().setScale(50)
        format.shadow().setBlurRadius(0)
        format.shadow().setOffsetDistance(5)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderMillimeters)
        assert self.checkRender(format, 'shadow_scale_50', QgsTextRenderer.Text, text=['test'])

    def testDrawShadowScaleUp(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setColor(QColor(255, 255, 255))
        format.shadow().setEnabled(True)
        format.shadow().setShadowPlacement(QgsTextShadowSettings.ShadowText)
        format.shadow().setScale(150)
        format.shadow().setBlurRadius(0)
        format.shadow().setOffsetDistance(5)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderMillimeters)
        assert self.checkRender(format, 'shadow_scale_150', QgsTextRenderer.Text, text=['test'])

    def testDrawShadowBackgroundPlacement(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setColor(QColor(255, 255, 255))
        format.shadow().setEnabled(True)
        format.shadow().setShadowPlacement(QgsTextShadowSettings.ShadowShape)
        format.shadow().setBlurRadius(0)
        format.shadow().setOffsetDistance(5)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderMillimeters)
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeRectangle)
        format.background().setSize(QSizeF(20, 10))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderMapUnits)
        assert self.checkRender(format, 'shadow_placement_background', QgsTextRenderer.Background, text=['test'])

    def testDrawShadowBufferPlacement(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setColor(QColor(255, 255, 255))
        format.shadow().setEnabled(True)
        format.shadow().setShadowPlacement(QgsTextShadowSettings.ShadowBuffer)
        format.shadow().setBlurRadius(0)
        format.shadow().setOffsetDistance(5)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderMillimeters)
        format.buffer().setEnabled(True)
        format.buffer().setSize(4)
        format.buffer().setSizeUnit(QgsUnitTypes.RenderMillimeters)
        assert self.checkRender(format, 'shadow_placement_buffer', QgsTextRenderer.Buffer, text=['test'])

    def testDrawTextWithBuffer(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.buffer().setEnabled(True)
        format.buffer().setSize(4)
        format.buffer().setSizeUnit(QgsUnitTypes.RenderMillimeters)
        assert self.checkRender(format, 'text_with_buffer', text=['test'], rect=QRectF(100, 100, 200, 100))

    def testDrawTextWithBackground(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeRectangle)
        format.background().setSize(QSizeF(20, 10))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderMapUnits)
        assert self.checkRender(format, 'text_with_background', text=['test'], rect=QRectF(100, 100, 200, 100))

    def testDrawTextWithBufferAndBackground(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeRectangle)
        format.background().setSize(QSizeF(20, 10))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderMapUnits)
        format.buffer().setEnabled(True)
        format.buffer().setSize(4)
        format.buffer().setColor(QColor(100, 255, 100))
        format.buffer().setSizeUnit(QgsUnitTypes.RenderMillimeters)
        assert self.checkRender(format, 'text_with_buffer_and_background', text=['test'],
                                rect=QRectF(100, 100, 200, 100))

    def testDrawTextWithShadowAndBuffer(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.shadow().setEnabled(True)
        format.shadow().setOpacity(1.0)
        format.shadow().setBlurRadius(0)
        format.shadow().setOffsetDistance(5)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderMillimeters)
        format.shadow().setColor(QColor(255, 100, 100))
        format.buffer().setEnabled(True)
        format.buffer().setSize(4)
        format.buffer().setColor(QColor(100, 255, 100))
        format.buffer().setSizeUnit(QgsUnitTypes.RenderMillimeters)
        assert self.checkRender(format, 'text_with_shadow_and_buffer', text=['test'], rect=QRectF(100, 100, 200, 100))

    def testDrawTextWithShadowBelowTextAndBuffer(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.shadow().setEnabled(True)
        format.shadow().setOpacity(1.0)
        format.shadow().setBlurRadius(0)
        format.shadow().setOffsetDistance(5)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderMillimeters)
        format.shadow().setColor(QColor(255, 100, 100))
        format.shadow().setShadowPlacement(QgsTextShadowSettings.ShadowText)
        format.buffer().setEnabled(True)
        format.buffer().setSize(4)
        format.buffer().setColor(QColor(100, 255, 100))
        format.buffer().setSizeUnit(QgsUnitTypes.RenderMillimeters)
        assert self.checkRender(format, 'text_with_shadow_below_text_and_buffer', text=['test'],
                                rect=QRectF(100, 100, 200, 100))

    def testDrawTextWithBackgroundAndShadow(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.shadow().setEnabled(True)
        format.shadow().setOpacity(1.0)
        format.shadow().setBlurRadius(0)
        format.shadow().setOffsetDistance(5)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderMillimeters)
        format.shadow().setColor(QColor(255, 100, 100))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeRectangle)
        format.background().setSize(QSizeF(20, 10))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderMapUnits)
        assert self.checkRender(format, 'text_with_shadow_and_background', text=['test'],
                                rect=QRectF(100, 100, 200, 100))

    def testDrawTextWithShadowBelowTextAndBackground(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.shadow().setEnabled(True)
        format.shadow().setOpacity(1.0)
        format.shadow().setBlurRadius(0)
        format.shadow().setOffsetDistance(5)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderMillimeters)
        format.shadow().setColor(QColor(255, 100, 100))
        format.shadow().setShadowPlacement(QgsTextShadowSettings.ShadowText)
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeRectangle)
        format.background().setSize(QSizeF(20, 10))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderMapUnits)
        assert self.checkRender(format, 'text_with_shadow_below_text_and_background', text=['test'],
                                rect=QRectF(100, 100, 200, 100))

    def testDrawTextWithBackgroundBufferAndShadow(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.shadow().setEnabled(True)
        format.shadow().setOpacity(1.0)
        format.shadow().setBlurRadius(0)
        format.shadow().setOffsetDistance(5)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderMillimeters)
        format.shadow().setColor(QColor(255, 100, 100))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeRectangle)
        format.background().setSize(QSizeF(20, 10))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderMapUnits)
        format.buffer().setEnabled(True)
        format.buffer().setSize(4)
        format.buffer().setColor(QColor(100, 255, 100))
        format.buffer().setSizeUnit(QgsUnitTypes.RenderMillimeters)
        assert self.checkRender(format, 'text_with_shadow_buffer_and_background', text=['test'],
                                rect=QRectF(100, 100, 200, 100))

    def testDrawTextWithBackgroundBufferAndShadowBelowText(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.shadow().setEnabled(True)
        format.shadow().setOpacity(1.0)
        format.shadow().setBlurRadius(0)
        format.shadow().setOffsetDistance(5)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderMillimeters)
        format.shadow().setColor(QColor(255, 100, 100))
        format.shadow().setShadowPlacement(QgsTextShadowSettings.ShadowText)
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeRectangle)
        format.background().setSize(QSizeF(20, 10))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderMapUnits)
        format.buffer().setEnabled(True)
        format.buffer().setSize(4)
        format.buffer().setColor(QColor(100, 255, 100))
        format.buffer().setSizeUnit(QgsUnitTypes.RenderMillimeters)
        assert self.checkRender(format, 'text_with_shadow_below_text_buffer_and_background', text=['test'],
                                rect=QRectF(100, 100, 200, 100))

    def testDrawTextWithBackgroundBufferAndShadowBelowBuffer(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.shadow().setEnabled(True)
        format.shadow().setOpacity(1.0)
        format.shadow().setBlurRadius(0)
        format.shadow().setOffsetDistance(5)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderMillimeters)
        format.shadow().setColor(QColor(255, 100, 100))
        format.shadow().setShadowPlacement(QgsTextShadowSettings.ShadowBuffer)
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeRectangle)
        format.background().setSize(QSizeF(20, 10))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderMapUnits)
        format.buffer().setEnabled(True)
        format.buffer().setSize(4)
        format.buffer().setColor(QColor(100, 255, 100))
        format.buffer().setSizeUnit(QgsUnitTypes.RenderMillimeters)
        assert self.checkRender(format, 'text_with_shadow_below_buffer_and_background', text=['test'],
                                rect=QRectF(100, 100, 200, 100))

    def testDrawTextRectMultilineRightAlign(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        assert self.checkRender(format, 'text_rect_multiline_right_aligned', text=['test', 'right', 'aligned'],
                                alignment=QgsTextRenderer.AlignRight, rect=QRectF(100, 100, 200, 100))

    def testDrawTextRectRightAlign(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        assert self.checkRender(format, 'text_rect_right_aligned', text=['test'],
                                alignment=QgsTextRenderer.AlignRight, rect=QRectF(100, 100, 200, 100))

    def testDrawTextRectMultilineJustifyAlign(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.buffer().setEnabled(True)
        format.buffer().setSize(4)
        format.buffer().setSizeUnit(QgsUnitTypes.RenderMillimeters)
        assert self.checkRender(format, 'text_rect_multiline_justify_aligned',
                                text=['a t est', 'off', 'justification', 'align'],
                                alignment=QgsTextRenderer.AlignJustify, rect=QRectF(100, 100, 200, 100))

    def testDrawTextRectJustifyAlign(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        assert self.checkRender(format, 'text_rect_justify_aligned', text=['test'],
                                alignment=QgsTextRenderer.AlignJustify, rect=QRectF(100, 100, 200, 100))

    def testDrawTextRectMultiparagraphJustifyAlign(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.buffer().setEnabled(True)
        format.buffer().setSize(4)
        format.buffer().setSizeUnit(QgsUnitTypes.RenderMillimeters)
        assert self.checkRender(format, 'text_rect_multiparagraph_justify_aligned',
                                text=['a t est', 'of justify', '', 'with two', 'pgraphs'],
                                alignment=QgsTextRenderer.AlignJustify, rect=QRectF(50, 100, 250, 100))

    def testDrawTextRectWordWrapSingleLine(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        painter = QPainter()
        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 50, 50))
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI
        context.setFlag(QgsRenderContext.ApplyScalingWorkaroundForTextRendering, True)

        self.assertTrue(QgsTextRenderer.textRequiresWrapping(context, 'a test of word wrap', 100, format))
        self.assertTrue(QgsTextRenderer.textRequiresWrapping(context, 'a test of word wrap', 200, format))
        self.assertTrue(QgsTextRenderer.textRequiresWrapping(context, 'a test of word wrap', 400, format))
        self.assertFalse(QgsTextRenderer.textRequiresWrapping(context, 'a test of word wrap', 500, format))

        self.assertEqual(QgsTextRenderer.wrappedText(context, 'a test of word wrap', 50, format), ['a', 'test', 'of', 'word', 'wrap'])
        self.assertEqual(QgsTextRenderer.wrappedText(context, 'a test of word wrap', 200, format), ['a test of', 'word', 'wrap'])
        self.assertEqual(QgsTextRenderer.wrappedText(context, 'a test of word wrap', 400, format), ['a test of word', 'wrap'])
        self.assertEqual(QgsTextRenderer.wrappedText(context, 'a test of word wrap', 500, format),
                         ['a test of word wrap'])

        # text height should account for wrapping
        self.assertGreater(QgsTextRenderer.textHeight(
            context, format, ['a test of word wrap'],
            mode=QgsTextRenderer.Rect, flags=Qgis.TextRendererFlag.WrapLines, maxLineWidth=200),
            QgsTextRenderer.textHeight(context, format, ['a test of word wrap'], mode=QgsTextRenderer.Rect) * 2.75)

        assert self.checkRender(format, 'text_rect_word_wrap_single_line', text=['a test of word wrap'],
                                alignment=QgsTextRenderer.AlignLeft, rect=QRectF(100, 100, 200, 100),
                                flags=Qgis.TextRendererFlag.WrapLines)

    def testDrawTextRectWordWrapMultiLine(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        painter = QPainter()
        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 50, 50))
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI
        context.setFlag(QgsRenderContext.ApplyScalingWorkaroundForTextRendering, True)

        # text height should account for wrapping
        self.assertGreater(QgsTextRenderer.textHeight(
            context, format, ['a test of word wrap', 'with bit more'],
            mode=QgsTextRenderer.Rect, flags=Qgis.TextRendererFlag.WrapLines, maxLineWidth=200),
            QgsTextRenderer.textHeight(context, format, ['a test of word wrap with with bit more'], mode=QgsTextRenderer.Rect) * 4.75)

        assert self.checkRender(format, 'text_rect_word_wrap_multi_line', text=['a test of word wrap', 'with bit more'],
                                alignment=QgsTextRenderer.AlignLeft, rect=QRectF(100, 100, 200, 100),
                                flags=Qgis.TextRendererFlag.WrapLines)

    def testDrawTextRectWordWrapWithJustify(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        assert self.checkRender(format, 'text_rect_word_wrap_justify', text=['a test of word wrap'],
                                alignment=QgsTextRenderer.AlignJustify, rect=QRectF(100, 100, 200, 100),
                                flags=Qgis.TextRendererFlag.WrapLines)

    def testDrawTextRectMultilineBottomAlign(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)

        assert self.checkRender(format, 'text_rect_multiline_bottom_aligned', text=['test', 'bottom', 'aligned'],
                                alignment=QgsTextRenderer.AlignLeft, rect=QRectF(100, 100, 200, 100),
                                vAlignment=QgsTextRenderer.AlignBottom)

    def testDrawTextRectBottomAlign(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)

        assert self.checkRender(format, 'text_rect_bottom_aligned', text=['bottom aligned'],
                                alignment=QgsTextRenderer.AlignLeft, rect=QRectF(100, 100, 200, 100),
                                vAlignment=QgsTextRenderer.AlignBottom)

    def testDrawTextRectMultilineVCenterAlign(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)

        assert self.checkRender(format, 'text_rect_multiline_vcenter_aligned', text=['test', 'center', 'aligned'],
                                alignment=QgsTextRenderer.AlignLeft, rect=QRectF(100, 100, 200, 100),
                                vAlignment=QgsTextRenderer.AlignVCenter)

    def testDrawTextRectVCenterAlign(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)

        assert self.checkRender(format, 'text_rect_vcenter_aligned', text=['center aligned'],
                                alignment=QgsTextRenderer.AlignLeft, rect=QRectF(100, 100, 200, 100),
                                vAlignment=QgsTextRenderer.AlignVCenter)

    def testDrawTextRectMultilineCenterAlign(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        assert self.checkRender(format, 'text_rect_multiline_center_aligned', text=['test', 'c', 'aligned'],
                                alignment=QgsTextRenderer.AlignCenter, rect=QRectF(100, 100, 200, 100))

    def testDrawTextRectCenterAlign(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setFont(getTestFont('bold'))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        assert self.checkRender(format, 'text_rect_center_aligned', text=['test'],
                                alignment=QgsTextRenderer.AlignCenter, rect=QRectF(100, 100, 200, 100))

    def testDrawTextPointMultilineRightAlign(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        assert self.checkRenderPoint(format, 'text_point_right_multiline_aligned', text=['test', 'right', 'aligned'],
                                     alignment=QgsTextRenderer.AlignRight, point=QPointF(300, 200))

    def testDrawTextPointMultilineCenterAlign(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        assert self.checkRenderPoint(format, 'text_point_center_multiline_aligned', text=['test', 'center', 'aligned'],
                                     alignment=QgsTextRenderer.AlignCenter, point=QPointF(200, 200))

    def testDrawTextPointRightAlign(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        assert self.checkRenderPoint(format, 'text_point_right_aligned', text=['test'],
                                     alignment=QgsTextRenderer.AlignRight, point=QPointF(300, 200))

    def testDrawTextPointJustifyAlign(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        assert self.checkRenderPoint(format, 'text_point_justify_aligned', text=['test'],
                                     alignment=QgsTextRenderer.AlignJustify, point=QPointF(100, 200))

    def testDrawTextPointMultilineJustifyAlign(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        assert self.checkRenderPoint(format, 'text_point_justify_multiline_aligned',
                                     text=['a t est', 'off', 'justification', 'align'],
                                     alignment=QgsTextRenderer.AlignJustify, point=QPointF(100, 200))

    def testDrawTextPointCenterAlign(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        assert self.checkRenderPoint(format, 'text_point_center_aligned', text=['test'],
                                     alignment=QgsTextRenderer.AlignCenter, point=QPointF(200, 200))

    def testDrawTextDataDefinedColorPoint(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setColor(QColor(0, 255, 0))
        format.dataDefinedProperties().setProperty(QgsPalLayerSettings.Color, QgsProperty.fromExpression("'#bb00cc'"))
        assert self.checkRenderPoint(format, 'text_dd_color_point', None, text=['test'], point=QPointF(50, 200))

    def testDrawTextDataDefinedColorRect(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setColor(QColor(0, 255, 0))
        format.dataDefinedProperties().setProperty(QgsPalLayerSettings.Color, QgsProperty.fromExpression("'#bb00cc'"))
        assert self.checkRender(format, 'text_dd_color_rect', None, text=['test'],
                                alignment=QgsTextRenderer.AlignCenter, rect=QRectF(100, 100, 100, 100))

    def testDrawTextDataDefinedBufferColorPoint(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setColor(QColor(0, 255, 0))
        format.dataDefinedProperties().setProperty(QgsPalLayerSettings.BufferColor,
                                                   QgsProperty.fromExpression("'#bb00cc'"))
        format.buffer().setEnabled(True)
        format.buffer().setSize(5)
        assert self.checkRenderPoint(format, 'text_dd_buffer_color', None, text=['test'], point=QPointF(50, 200))

    def testHtmlFormatting(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setColor(QColor(0, 255, 0))
        format.setAllowHtmlFormatting(True)
        assert self.checkRenderPoint(format, 'text_html_formatting', None, text=[
            '<s>t</s><span style="text-decoration: overline">e</span><span style="color: red">s<span style="color: rgba(255,0,0,0.5); text-decoration: underline">t</span></span>'],
            point=QPointF(50, 200))

    def testHtmlFormattingBuffer(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setColor(QColor(0, 255, 0))
        format.setAllowHtmlFormatting(True)
        format.buffer().setEnabled(True)
        format.buffer().setSize(5)
        format.buffer().setColor(QColor(50, 150, 200))
        assert self.checkRenderPoint(format, 'text_html_formatting_buffer', None, text=[
            '<s>t</s><span style="text-decoration: overline">e</span><span style="color: red">s<span style="text-decoration: underline">t</span></span>'],
            point=QPointF(50, 200))

    def testHtmlFormattingShadow(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setColor(QColor(0, 255, 0))
        format.setAllowHtmlFormatting(True)
        format.shadow().setEnabled(True)
        format.shadow().setOffsetDistance(5)
        format.shadow().setBlurRadius(0)
        format.shadow().setColor(QColor(50, 150, 200))
        assert self.checkRenderPoint(format, 'text_html_formatting_shadow', None, text=[
            '<s>t</s><span style="text-decoration: overline">e</span><span style="color: red">s<span style="text-decoration: underline">t</span></span>'],
            point=QPointF(50, 200))

    def testHtmlFormattingBufferShadow(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setColor(QColor(0, 255, 0))
        format.setAllowHtmlFormatting(True)
        format.buffer().setEnabled(True)
        format.buffer().setSize(5)
        format.buffer().setColor(QColor(200, 50, 150))
        format.shadow().setEnabled(True)
        format.shadow().setOffsetDistance(5)
        format.shadow().setBlurRadius(0)
        format.shadow().setColor(QColor(50, 150, 200))
        assert self.checkRenderPoint(format, 'text_html_formatting_buffer_shadow', None, text=[
            '<s>t</s><span style="text-decoration: overline">e</span><span style="color: red">s<span style="text-decoration: underline">t</span></span>'],
            point=QPointF(50, 200))

    def testHtmlFormattingVertical(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setColor(QColor(0, 255, 0))
        format.setAllowHtmlFormatting(True)
        format.setOrientation(QgsTextFormat.VerticalOrientation)
        assert self.checkRenderPoint(format, 'text_html_formatting_vertical', None, text=[
            '<s>t</s><span style="text-decoration: overline">e</span><span style="color: red">s<span style="text-decoration: underline">t</span></span>'],
            point=QPointF(50, 200))

    def testHtmlFormattingBufferVertical(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setColor(QColor(0, 255, 0))
        format.setAllowHtmlFormatting(True)
        format.buffer().setEnabled(True)
        format.buffer().setSize(5)
        format.buffer().setColor(QColor(50, 150, 200))
        format.setOrientation(QgsTextFormat.VerticalOrientation)
        assert self.checkRenderPoint(format, 'text_html_formatting_buffer_vertical', None, text=[
            '<s>t</s><span style="text-decoration: overline">e</span><span style="color: red">s<span style="text-decoration: underline">t</span></span>'],
            point=QPointF(50, 200))

    def testHtmlMixedMetricFormatting(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('regular'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setColor(QColor(0, 255, 0))
        format.setAllowHtmlFormatting(True)
        assert self.checkRenderPoint(format, 'text_html_mixed_metric_formatting', None, text=[
            '<i>t</i><b style="font-size: 30pt">e</b><p><span style="color: red">s<span style="color: rgba(255,0,0,0.5); text-decoration: underline; font-size:80pt">t</span></span>'],
            point=QPointF(50, 200))

    def testHtmlMixedMetricLineHeight(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('regular'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setColor(QColor(0, 255, 0))
        format.setAllowHtmlFormatting(True)
        format.setLineHeight(0.5)
        assert self.checkRenderPoint(format, 'text_html_mixed_metric_formatting_line_height', None, text=[
            '<i>t</i><b style="font-size: 30pt">e</b><p><span style="color: red">s<span style="color: rgba(255,0,0,0.5); text-decoration: underline; font-size:80pt">t</span></span>'],
            point=QPointF(50, 200))

    def testHtmlMixedMetricFormattingBuffer(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setColor(QColor(0, 255, 0))
        format.setAllowHtmlFormatting(True)
        format.buffer().setEnabled(True)
        format.buffer().setSize(5)
        format.buffer().setColor(QColor(50, 150, 200))
        assert self.checkRenderPoint(format, 'text_html_mixed_metric_formatting_buffer', None, text=[
            '<i>t</i><b style="font-size: 30pt">e</b><p><span style="color: red">s<span style="color: rgba(255,0,0,0.5); text-decoration: underline; font-size:80pt">t</span></span>'],
            point=QPointF(50, 200))

    def testHtmlMixedMetricFormattingShadow(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setColor(QColor(0, 255, 0))
        format.setAllowHtmlFormatting(True)
        format.shadow().setEnabled(True)
        format.shadow().setOffsetDistance(5)
        format.shadow().setBlurRadius(0)
        format.shadow().setColor(QColor(50, 150, 200))
        assert self.checkRenderPoint(format, 'text_html_mixed_metric_formatting_shadow', None, text=[
            '<i>t</i><b style="font-size: 30pt">e</b><p><span style="color: red">s<span style="color: rgba(255,0,0,0.5); text-decoration: underline; font-size:80pt">t</span></span>'],
            point=QPointF(50, 200))

    def testHtmlMixedMetricFormattingBufferShadow(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setColor(QColor(0, 255, 0))
        format.setAllowHtmlFormatting(True)
        format.buffer().setEnabled(True)
        format.buffer().setSize(5)
        format.buffer().setColor(QColor(200, 50, 150))
        format.shadow().setEnabled(True)
        format.shadow().setOffsetDistance(5)
        format.shadow().setBlurRadius(0)
        format.shadow().setColor(QColor(50, 150, 200))
        assert self.checkRenderPoint(format, 'text_html_mixed_metric_formatting_buffer_shadow', None, text=[
            '<i>t</i><b style="font-size: 30pt">e</b><p><span style="color: red">s<span style="color: rgba(255,0,0,0.5); text-decoration: underline; font-size:80pt">t</span></span>'],
            point=QPointF(50, 200))

    def testHtmlMixedMetricFormattingVertical(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setColor(QColor(0, 255, 0))
        format.setAllowHtmlFormatting(True)
        format.setOrientation(QgsTextFormat.VerticalOrientation)
        assert self.checkRenderPoint(format, 'text_html_mixed_metric_formatting_vertical', None, text=[
            '<i>t</i><b style="font-size: 30pt">e</b><p><span style="color: red">s<span style="color: rgba(255,0,0,0.5); text-decoration: underline; font-size:80pt">t</span></span>'],
            point=QPointF(50, 200))

    def testHtmlMixedMetricFormattingBufferVertical(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setColor(QColor(0, 255, 0))
        format.setAllowHtmlFormatting(True)
        format.buffer().setEnabled(True)
        format.buffer().setSize(5)
        format.buffer().setColor(QColor(50, 150, 200))
        format.setOrientation(QgsTextFormat.VerticalOrientation)
        assert self.checkRenderPoint(format, 'text_html_mixed_metric_formatting_buffer_vertical', None, text=[
            '<i>t</i><b style="font-size: 30pt">e</b><p><span style="color: red">s<span style="color: rgba(255,0,0,0.5); text-decoration: underline; font-size:80pt">t</span></span>'],
            point=QPointF(50, 200))

    def testHtmlMixedMetricFormattingShadowVertical(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setColor(QColor(0, 255, 0))
        format.setAllowHtmlFormatting(True)
        format.shadow().setEnabled(True)
        format.shadow().setOffsetDistance(5)
        format.shadow().setBlurRadius(0)
        format.shadow().setColor(QColor(50, 150, 200))
        format.setOrientation(QgsTextFormat.VerticalOrientation)
        assert self.checkRenderPoint(format, 'text_html_mixed_metric_formatting_shadow_vertical', None, text=[
            '<i>t</i><b style="font-size: 30pt">e</b><p><span style="color: red">s<span style="color: rgba(255,0,0,0.5); text-decoration: underline; font-size:80pt">t</span></span>'],
            point=QPointF(50, 200))

    def testHtmlMixedMetricFormattingBufferShadowVertical(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setColor(QColor(0, 255, 0))
        format.setAllowHtmlFormatting(True)
        format.buffer().setEnabled(True)
        format.buffer().setSize(5)
        format.buffer().setColor(QColor(200, 50, 150))
        format.shadow().setEnabled(True)
        format.shadow().setOffsetDistance(5)
        format.shadow().setBlurRadius(0)
        format.shadow().setColor(QColor(50, 150, 200))
        format.setOrientation(QgsTextFormat.VerticalOrientation)
        assert self.checkRenderPoint(format, 'text_html_mixed_metric_formatting_buffer_shadow_vertical', None, text=[
            '<i>t</i><b style="font-size: 30pt">e</b><p><span style="color: red">s<span style="color: rgba(255,0,0,0.5); text-decoration: underline; font-size:80pt">t</span></span>'],
            point=QPointF(50, 200))

    def testHtmlSuperSubscript(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setColor(QColor(255, 0, 0))
        format.setAllowHtmlFormatting(True)
        assert self.checkRenderPoint(format, 'text_html_supersubscript', None, text=[
            '<sub>sub</sub>N<sup>sup</sup>'],
            point=QPointF(50, 200))

    def testHtmlSuperSubscriptFixedFontSize(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setColor(QColor(255, 0, 0))
        format.setAllowHtmlFormatting(True)
        assert self.checkRenderPoint(format, 'text_html_supersubscript_fixed_font_size', None, text=[
            '<sub style="font-size:80pt">s<span style="font-size:30pt">u</span></sub>N<sup style="font-size:40pt">s<span style="font-size: 20pt">up</span></sup>'],
            point=QPointF(50, 200))

    def testHtmlSuperSubscriptBuffer(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setColor(QColor(255, 0, 0))
        format.setAllowHtmlFormatting(True)
        format.buffer().setEnabled(True)
        format.buffer().setSize(5)
        format.buffer().setColor(QColor(50, 150, 200))
        assert self.checkRenderPoint(format, 'text_html_supersubscript_buffer', None, text=[
            '<sub>sub</sub>N<sup>sup</sup>'],
            point=QPointF(50, 200))

    def testHtmlSuperSubscriptShadow(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setColor(QColor(0, 255, 0))
        format.setAllowHtmlFormatting(True)
        format.shadow().setEnabled(True)
        format.shadow().setOffsetDistance(5)
        format.shadow().setBlurRadius(0)
        format.shadow().setColor(QColor(50, 150, 200))
        assert self.checkRenderPoint(format, 'text_html_supersubscript_shadow', None, text=[
            '<sub>sub</sub>N<sup>sup</sup>'],
            point=QPointF(50, 200))

    def testHtmlSuperSubscriptBufferShadow(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setColor(QColor(0, 255, 0))
        format.setAllowHtmlFormatting(True)
        format.buffer().setEnabled(True)
        format.buffer().setSize(5)
        format.buffer().setColor(QColor(200, 50, 150))
        format.shadow().setEnabled(True)
        format.shadow().setOffsetDistance(5)
        format.shadow().setBlurRadius(0)
        format.shadow().setColor(QColor(50, 150, 200))
        assert self.checkRenderPoint(format, 'text_html_supersubscript_buffer_shadow', None, text=[
            '<sub>sub</sub>N<sup>sup</sup>'],
            point=QPointF(50, 200))

    def testTextRenderFormat(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)

        filename = '{}/test_render_text.svg'.format(QDir.tempPath())
        svg = QSvgGenerator()
        svg.setFileName(filename)
        svg.setSize(QSize(400, 400))
        svg.setResolution(600)

        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 50, 50))
        ms.setOutputSize(QSize(400, 400))
        context = QgsRenderContext.fromMapSettings(ms)

        # test with ALWAYS TEXT mode
        context.setTextRenderFormat(QgsRenderContext.TextFormatAlwaysText)
        painter = QPainter()
        context.setPainter(painter)

        context.setScaleFactor(96 / 25.4)  # 96 DPI

        painter.begin(svg)

        painter.setBrush(QBrush(QColor(182, 239, 255)))
        painter.setPen(Qt.NoPen)

        QgsTextRenderer.drawText(QPointF(0, 30),
                                 0,
                                 QgsTextRenderer.AlignLeft,
                                 ['my test text'],
                                 context,
                                 format)

        painter.end()

        # expect svg to contain a text object with the label
        with open(filename, 'r') as f:
            lines = ''.join(f.readlines())
        self.assertIn('<text', lines)
        self.assertIn('>my test text<', lines)

        os.unlink(filename)

        # test with ALWAYS CURVES mode
        context = QgsRenderContext.fromMapSettings(ms)
        context.setTextRenderFormat(QgsRenderContext.TextFormatAlwaysOutlines)
        painter = QPainter()
        context.setPainter(painter)

        context.setScaleFactor(96 / 25.4)  # 96 DPI

        svg = QSvgGenerator()
        svg.setFileName(filename)
        svg.setSize(QSize(400, 400))
        svg.setResolution(600)
        painter.begin(svg)

        painter.setBrush(QBrush(QColor(182, 239, 255)))
        painter.setPen(Qt.NoPen)

        QgsTextRenderer.drawText(QPointF(0, 30),
                                 0,
                                 QgsTextRenderer.AlignLeft,
                                 ['my test text'],
                                 context,
                                 format)

        painter.end()

        # expect svg to contain a text object with the label
        with open(filename, 'r') as f:
            lines = ''.join(f.readlines())
        self.assertNotIn('<text', lines)
        self.assertNotIn('>my test text<', lines)

    def testDrawTextVerticalRectMode(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setOrientation(QgsTextFormat.VerticalOrientation)
        assert self.checkRender(format, 'text_vertical_rect_mode', QgsTextRenderer.Text, text=['1234'],
                                rect=QRectF(40, 20, 350, 350))

    def testDrawTextVerticalRectModeCenterAligned(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setOrientation(QgsTextFormat.VerticalOrientation)
        assert self.checkRender(format, 'text_vertical_rect_mode_center_aligned', QgsTextRenderer.Text,
                                text=['1234', '5678'], rect=QRectF(40, 20, 350, 350),
                                alignment=QgsTextRenderer.AlignCenter)

    def testDrawTextVerticalRectModeRightAligned(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setOrientation(QgsTextFormat.VerticalOrientation)
        assert self.checkRender(format, 'text_vertical_rect_mode_right_aligned', QgsTextRenderer.Text,
                                text=['1234', '5678'], rect=QRectF(40, 20, 350, 350),
                                alignment=QgsTextRenderer.AlignRight)

    def testDrawTextVerticalPointMode(self):
        format = QgsTextFormat()
        format.setFont(getTestFont('bold'))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderPoints)
        format.setOrientation(QgsTextFormat.VerticalOrientation)
        assert self.checkRenderPoint(format, 'text_vertical_point_mode', QgsTextRenderer.Text, text=['1234', '5678'],
                                     point=QPointF(40, 380))


if __name__ == '__main__':
    unittest.main()
