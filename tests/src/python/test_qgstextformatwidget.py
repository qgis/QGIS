# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsTextFormatWidget.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '2016-09'
__copyright__ = 'Copyright 2016, The QGIS Project'

import qgis  # NOQA

from qgis.core import (QgsTextBufferSettings,
                       QgsTextMaskSettings,
                       QgsTextBackgroundSettings,
                       QgsTextShadowSettings,
                       QgsTextFormat,
                       QgsUnitTypes,
                       QgsMapUnitScale,
                       QgsBlurEffect,
                       QgsMarkerSymbol,
                       QgsSymbolLayerReference,
                       QgsSymbolLayerId)
from qgis.gui import (QgsTextFormatWidget, QgsTextFormatDialog)
from qgis.PyQt.QtGui import (QColor, QPainter)
from qgis.PyQt.QtCore import (Qt, QSizeF, QPointF)
from qgis.testing import unittest, start_app
from utilities import getTestFont

start_app()


class PyQgsTextFormatWidget(unittest.TestCase):

    def createBufferSettings(self):
        s = QgsTextBufferSettings()
        s.setEnabled(True)
        s.setSize(5)
        s.setSizeUnit(QgsUnitTypes.RenderPixels)
        s.setSizeMapUnitScale(QgsMapUnitScale(1, 2))
        s.setColor(QColor(255, 0, 0))
        s.setFillBufferInterior(True)
        s.setOpacity(0.5)
        s.setJoinStyle(Qt.RoundJoin)
        s.setBlendMode(QPainter.CompositionMode_Difference)
        s.setPaintEffect(QgsBlurEffect.create({'blur_level': '2.0', 'blur_unit': QgsUnitTypes.encodeUnit(QgsUnitTypes.RenderMillimeters), 'enabled': '1'}))
        return s

    def checkBufferSettings(self, s):
        """ test QgsTextBufferSettings """
        self.assertTrue(s.enabled())
        self.assertEqual(s.size(), 5)
        self.assertEqual(s.sizeUnit(), QgsUnitTypes.RenderPixels)
        self.assertEqual(s.sizeMapUnitScale(), QgsMapUnitScale(1, 2))
        self.assertEqual(s.color(), QColor(255, 0, 0))
        self.assertTrue(s.fillBufferInterior())
        self.assertEqual(s.opacity(), 0.5)
        self.assertEqual(s.joinStyle(), Qt.RoundJoin)
        self.assertEqual(s.blendMode(), QPainter.CompositionMode_Difference)
        self.assertTrue(s.paintEffect())
        self.assertEqual(s.paintEffect().blurLevel(), 2.0)

    def createMaskSettings(self):
        s = QgsTextMaskSettings()
        s.setEnabled(True)
        s.setSize(5)
        s.setSizeUnit(QgsUnitTypes.RenderPixels)
        s.setSizeMapUnitScale(QgsMapUnitScale(1, 2))
        s.setOpacity(0.5)
        s.setJoinStyle(Qt.RoundJoin)
        s.setPaintEffect(QgsBlurEffect.create({'blur_level': '2.0', 'blur_unit': QgsUnitTypes.encodeUnit(QgsUnitTypes.RenderMillimeters), 'enabled': '1'}))
        s.setMaskedSymbolLayers([QgsSymbolLayerReference("layerid1", QgsSymbolLayerId("symbol", 1)),
                                 QgsSymbolLayerReference("layerid2", QgsSymbolLayerId("symbol2", 2))])
        return s

    def checkMaskSettings(self, s):
        """ test QgsTextMaskSettings """
        self.assertTrue(s.enabled())
        self.assertEqual(s.size(), 5)
        self.assertEqual(s.sizeUnit(), QgsUnitTypes.RenderPixels)
        self.assertEqual(s.sizeMapUnitScale(), QgsMapUnitScale(1, 2))
        self.assertEqual(s.opacity(), 0.5)
        self.assertEqual(s.joinStyle(), Qt.RoundJoin)
        self.assertTrue(s.paintEffect())
        self.assertEqual(s.paintEffect().blurLevel(), 2.0)
        # Always return an empty list because there is no project (and thus no layers) defined
        self.assertEqual(s.maskedSymbolLayers(), [])

    def createBackgroundSettings(self):
        s = QgsTextBackgroundSettings()
        s.setEnabled(True)
        s.setType(QgsTextBackgroundSettings.ShapeEllipse)
        s.setSvgFile('svg.svg')
        s.setSizeType(QgsTextBackgroundSettings.SizeFixed)
        s.setSize(QSizeF(1, 2))
        s.setSizeUnit(QgsUnitTypes.RenderPixels)
        s.setSizeMapUnitScale(QgsMapUnitScale(1, 2))
        s.setRotationType(QgsTextBackgroundSettings.RotationFixed)
        s.setRotation(45)
        s.setOffset(QPointF(3, 4))
        s.setOffsetUnit(QgsUnitTypes.RenderMapUnits)
        s.setOffsetMapUnitScale(QgsMapUnitScale(5, 6))
        s.setRadii(QSizeF(11, 12))
        s.setRadiiUnit(QgsUnitTypes.RenderPixels)
        s.setRadiiMapUnitScale(QgsMapUnitScale(15, 16))
        s.setFillColor(QColor(255, 0, 0))
        s.setStrokeColor(QColor(0, 255, 0))
        s.setOpacity(0.5)
        s.setJoinStyle(Qt.RoundJoin)
        s.setBlendMode(QPainter.CompositionMode_Difference)
        s.setStrokeWidth(7)
        s.setStrokeWidthUnit(QgsUnitTypes.RenderMapUnits)
        s.setStrokeWidthMapUnitScale(QgsMapUnitScale(QgsMapUnitScale(25, 26)))
        s.setPaintEffect(QgsBlurEffect.create({'blur_level': '6.0', 'blur_unit': QgsUnitTypes.encodeUnit(QgsUnitTypes.RenderMillimeters), 'enabled': '1'}))

        marker = QgsMarkerSymbol()
        marker.setColor(QColor(100, 112, 134))
        s.setMarkerSymbol(marker)

        return s

    def checkBackgroundSettings(self, s):
        """ test QgsTextBackgroundSettings """
        self.assertTrue(s.enabled())
        self.assertEqual(s.type(), QgsTextBackgroundSettings.ShapeEllipse)
        self.assertEqual(s.svgFile(), 'svg.svg')
        self.assertEqual(s.sizeType(), QgsTextBackgroundSettings.SizeFixed)
        self.assertEqual(s.size(), QSizeF(1, 2))
        self.assertEqual(s.sizeUnit(), QgsUnitTypes.RenderPixels)
        self.assertEqual(s.sizeMapUnitScale(), QgsMapUnitScale(1, 2))
        self.assertEqual(s.rotationType(), QgsTextBackgroundSettings.RotationFixed)
        self.assertEqual(s.rotation(), 45)
        self.assertEqual(s.offset(), QPointF(3, 4))
        self.assertEqual(s.offsetUnit(), QgsUnitTypes.RenderMapUnits)
        self.assertEqual(s.offsetMapUnitScale(), QgsMapUnitScale(5, 6))
        self.assertEqual(s.radii(), QSizeF(11, 12))
        self.assertEqual(s.radiiUnit(), QgsUnitTypes.RenderPixels)
        self.assertEqual(s.radiiMapUnitScale(), QgsMapUnitScale(15, 16))
        self.assertEqual(s.fillColor(), QColor(255, 0, 0))
        self.assertEqual(s.strokeColor(), QColor(0, 255, 0))
        self.assertEqual(s.opacity(), 0.5)
        self.assertEqual(s.joinStyle(), Qt.RoundJoin)
        self.assertEqual(s.blendMode(), QPainter.CompositionMode_Difference)
        self.assertEqual(s.strokeWidth(), 7)
        self.assertEqual(s.strokeWidthUnit(), QgsUnitTypes.RenderMapUnits)
        self.assertEqual(s.strokeWidthMapUnitScale(), QgsMapUnitScale(25, 26))
        self.assertTrue(s.paintEffect())
        self.assertEqual(s.paintEffect().blurLevel(), 6.0)

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
        s.setBlurRadiusUnit(QgsUnitTypes.RenderMapUnits)
        s.setBlurRadiusMapUnitScale(QgsMapUnitScale(15, 16))
        s.setBlurAlphaOnly(True)
        s.setColor(QColor(255, 0, 0))
        s.setOpacity(0.5)
        s.setScale(123)
        s.setBlendMode(QPainter.CompositionMode_Difference)
        return s

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
        self.assertEqual(s.blurRadiusUnit(), QgsUnitTypes.RenderMapUnits)
        self.assertEqual(s.blurRadiusMapUnitScale(), QgsMapUnitScale(15, 16))
        self.assertTrue(s.blurAlphaOnly())
        self.assertEqual(s.color(), QColor(255, 0, 0))
        self.assertEqual(s.opacity(), 0.5)
        self.assertEqual(s.scale(), 123)
        self.assertEqual(s.blendMode(), QPainter.CompositionMode_Difference)

    def createFormatSettings(self):
        s = QgsTextFormat()
        s.setBuffer(self.createBufferSettings())
        s.setMask(self.createMaskSettings())
        s.setBackground(self.createBackgroundSettings())
        s.setShadow(self.createShadowSettings())
        font = getTestFont()
        font.setKerning(False)
        s.setFont(font)
        s.setNamedStyle('Roman')
        s.setSize(5)
        s.setSizeUnit(QgsUnitTypes.RenderPoints)
        s.setSizeMapUnitScale(QgsMapUnitScale(1, 2))
        s.setColor(QColor(255, 0, 0))
        s.setOpacity(0.5)
        s.setBlendMode(QPainter.CompositionMode_Difference)
        s.setLineHeight(5)
        s.setOrientation(QgsTextFormat.VerticalOrientation)
        s.setPreviewBackgroundColor(QColor(100, 150, 200))
        return s

    def checkTextFormat(self, s):
        """ test QgsTextFormat """
        self.checkBufferSettings(s.buffer())
        self.checkMaskSettings(s.mask())
        self.checkShadowSettings(s.shadow())
        self.checkBackgroundSettings(s.background())
        self.assertEqual(s.font().family(), 'QGIS Vera Sans')
        self.assertFalse(s.font().kerning())
        self.assertEqual(s.namedStyle(), 'Roman')
        self.assertEqual(s.size(), 5)
        self.assertEqual(s.sizeUnit(), QgsUnitTypes.RenderPoints)
        self.assertEqual(s.sizeMapUnitScale(), QgsMapUnitScale(1, 2))
        self.assertEqual(s.color(), QColor(255, 0, 0))
        self.assertEqual(s.opacity(), 0.5)
        self.assertEqual(s.blendMode(), QPainter.CompositionMode_Difference)
        self.assertEqual(s.lineHeight(), 5)
        self.assertEqual(s.orientation(), QgsTextFormat.VerticalOrientation)
        self.assertEqual(s.previewBackgroundColor().name(), '#6496c8')

    def testSettings(self):
        # test that widget correctly sets and returns matching settings
        s = self.createFormatSettings()
        w = QgsTextFormatWidget(s)
        self.checkTextFormat(w.format())

    def testDialogSettings(self):
        # test that dialog correctly sets and returns matching settings
        s = self.createFormatSettings()
        d = QgsTextFormatDialog(s)
        self.checkTextFormat(d.format())


if __name__ == '__main__':
    unittest.main()
