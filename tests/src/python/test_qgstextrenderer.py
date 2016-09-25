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
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.core import (QgsTextBufferSettings,
                       QgsTextBackgroundSettings,
                       QgsTextShadowSettings,
                       QgsTextFormat,
                       QgsUnitTypes,
                       QgsMapUnitScale,
                       QgsVectorLayer)
from qgis.PyQt.QtGui import (QColor, QPainter, QFont)
from qgis.PyQt.QtCore import (Qt, QSizeF, QPointF)
from qgis.PyQt.QtXml import (QDomDocument, QDomElement)
from qgis.testing import unittest, start_app
from utilities import getTestFont

start_app()


def createEmptyLayer():
    layer = QgsVectorLayer("Point", "addfeat", "memory")
    assert layer.isValid()
    return layer


class PyQgsTextRenderer(unittest.TestCase):

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

    def testBufferReadWriteLayer(self):
        """test writing and retrieving settings from a layer"""
        layer = createEmptyLayer()
        s = self.createBufferSettings()
        s.writeToLayer(layer)
        t = QgsTextBufferSettings()
        t.readFromLayer(layer)
        self.checkBufferSettings(t)

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
        s.setBorderColor(QColor(0, 255, 0))
        s.setOpacity(0.5)
        s.setJoinStyle(Qt.RoundJoin)
        s.setBlendMode(QPainter.CompositionMode_DestinationAtop)
        s.setBorderWidth(7)
        s.setBorderWidthUnit(QgsUnitTypes.RenderPoints)
        s.setBorderWidthMapUnitScale(QgsMapUnitScale(QgsMapUnitScale(25, 26)))
        return s

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
        self.assertEqual(s.borderColor(), QColor(0, 255, 0))
        self.assertEqual(s.opacity(), 0.5)
        self.assertEqual(s.joinStyle(), Qt.RoundJoin)
        self.assertEqual(s.blendMode(), QPainter.CompositionMode_DestinationAtop)
        self.assertEqual(s.borderWidth(), 7)
        self.assertEqual(s.borderWidthUnit(), QgsUnitTypes.RenderPoints)
        self.assertEqual(s.borderWidthMapUnitScale(), QgsMapUnitScale(25, 26))

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

    def testBackgroundReadWriteLayer(self):
        """test writing and retrieving settings from a layer"""
        layer = createEmptyLayer()
        s = self.createBackgroundSettings()
        s.writeToLayer(layer)
        t = QgsTextBackgroundSettings()
        t.readFromLayer(layer)
        self.checkBackgroundSettings(t)

    def testBackgroundReadWriteXml(self):
        """test saving and restoring state of a background to xml"""
        doc = QDomDocument("testdoc")
        s = self.createBackgroundSettings()
        elem = s.writeXml(doc)
        parent = doc.createElement("settings")
        parent.appendChild(elem)
        t = QgsTextBackgroundSettings()
        t.readXml(parent)
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

    def testShadowReadWriteLayer(self):
        """test writing and retrieving settings from a layer"""
        layer = createEmptyLayer()
        s = self.createShadowSettings()
        s.writeToLayer(layer)
        t = QgsTextShadowSettings()
        t.readFromLayer(layer)
        self.checkShadowSettings(t)

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
        s.background().setEnabled(True)
        s.background().setSvgFile('test.svg')
        s.shadow().setEnabled(True)
        s.shadow().setOffsetAngle(223)
        s.setFont(getTestFont())
        s.setNamedStyle('Italic')
        s.setSize(5)
        s.setSizeUnit(QgsUnitTypes.RenderPoints)
        s.setSizeMapUnitScale(QgsMapUnitScale(1, 2))
        s.setColor(QColor(255, 0, 0))
        s.setOpacity(0.5)
        s.setBlendMode(QPainter.CompositionMode_DestinationAtop)
        s.setLineHeight(5)
        return s

    def checkTextFormat(self, s):
        """ test QgsTextFormat """
        self.assertTrue(s.buffer().enabled())
        self.assertEqual(s.buffer().size(), 25)
        self.assertTrue(s.background().enabled())
        self.assertEqual(s.background().svgFile(), 'test.svg')
        self.assertTrue(s.shadow().enabled())
        self.assertEqual(s.shadow().offsetAngle(), 223)
        self.assertEqual(s.font().family(), 'QGIS Vera Sans')
        self.assertEqual(s.namedStyle(), 'Italic')
        self.assertEqual(s.size(), 5)
        self.assertEqual(s.sizeUnit(), QgsUnitTypes.RenderPoints)
        self.assertEqual(s.sizeMapUnitScale(), QgsMapUnitScale(1, 2))
        self.assertEqual(s.color(), QColor(255, 0, 0))
        self.assertEqual(s.opacity(), 0.5)
        self.assertEqual(s.blendMode(), QPainter.CompositionMode_DestinationAtop)
        self.assertEqual(s.lineHeight(), 5)

    def testFormatGettersSetters(self):
        s = self.createFormatSettings()
        self.checkTextFormat(s)

    def testFormatCopy(self):
        s = self.createFormatSettings()
        s2 = s
        self.checkTextFormat(s2)
        s3 = QgsTextFormat(s)
        self.checkTextFormat(s3)

    def testFormatReadWriteLayer(self):
        """test writing and retrieving settings from a layer"""
        layer = createEmptyLayer()
        s = self.createFormatSettings()
        s.writeToLayer(layer)
        t = QgsTextFormat()
        t.readFromLayer(layer)
        self.checkTextFormat(t)

    def testFormatReadWriteXml(self):
        """test saving and restoring state of a shadow to xml"""
        doc = QDomDocument("testdoc")
        s = self.createFormatSettings()
        elem = s.writeXml(doc)
        parent = doc.createElement("settings")
        parent.appendChild(elem)
        t = QgsTextFormat()
        t.readXml(parent)
        self.checkTextFormat(t)

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
        elem = f.writeXml(doc)
        elem.setAttribute('fontFamily', 'asdfasdfsadf')
        parent = doc.createElement("parent")
        parent.appendChild(elem)

        f.readXml(parent)
        self.assertFalse(f.fontFound())

        font = getTestFont()
        elem.setAttribute('fontFamily', font.family())
        f.readXml(parent)
        self.assertTrue(f.fontFound())


if __name__ == '__main__':
    unittest.main()
