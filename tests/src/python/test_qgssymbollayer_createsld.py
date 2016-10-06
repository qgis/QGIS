# -*- coding: utf-8 -*-

"""
***************************************************************************
    test_qgssymbollayer_createsld.py
    ---------------------
    Date                 : July 2016
    Copyright            : (C) 2016 by Andrea Aime
    Email                : andrea dot aime at geosolutions dot it
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *less
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Andrea Aime'
__date__ = 'July 2016'
__copyright__ = '(C) 2012, Andrea Aime'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

import os

from qgis.PyQt.QtCore import pyqtWrapperType, Qt, QDir, QFile, QIODevice, QPointF
from qgis.PyQt.QtXml import (
    QDomDocument, QDomElement, QDomNode, QDomNamedNodeMap)
from qgis.PyQt.QtGui import QColor

from qgis.core import (
    QgsSimpleMarkerSymbolLayer, QgsSimpleMarkerSymbolLayerBase, QgsUnitTypes, QgsSvgMarkerSymbolLayer,
    QgsFontMarkerSymbolLayer, QgsEllipseSymbolLayer, QgsSimpleLineSymbolLayer,
    QgsMarkerLineSymbolLayer, QgsMarkerSymbol, QgsSimpleFillSymbolLayer, QgsSVGFillSymbolLayer,
    QgsLinePatternFillSymbolLayer, QgsPointPatternFillSymbolLayer, QgsVectorLayer)
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

# Convenience instances in case you may need them
# not used in this test
start_app()


class TestQgsSymbolLayerCreateSld(unittest.TestCase):

    """
     This class tests the creation of SLD from QGis layers
     """

    def testSimpleMarkerRotation(self):
        symbol = QgsSimpleMarkerSymbolLayer(
            QgsSimpleMarkerSymbolLayerBase.Star, color=QColor(255, 0, 0), borderColor=QColor(0, 255, 0), size=10)
        symbol.setAngle(50)
        dom, root = self.symbolToSld(symbol)
        # print( "Simple marker rotation: " + root.ownerDocument().toString())

        self.assertStaticRotation(root, '50')

    def assertStaticRotation(self, root, expectedValue):
        # Check the rotation element is a literal, not a
        rotation = root.elementsByTagName('se:Rotation').item(0)
        literal = rotation.firstChild()
        self.assertEqual("ogc:Literal", literal.nodeName())
        self.assertEqual(expectedValue, literal.firstChild().nodeValue())

    def assertStaticDisplacement(self, root, expectedDispX, expectedDispY):
        displacement = root.elementsByTagName('se:Displacement').item(0)
        self.assertIsNotNone(displacement)
        dx = displacement.firstChild()
        self.assertIsNotNone(dx)
        self.assertEqual("se:DisplacementX", dx.nodeName())
        self.assertSldNumber(expectedDispX, dx.firstChild().nodeValue())
        dy = displacement.lastChild()
        self.assertIsNotNone(dy)
        self.assertEqual("se:DisplacementY", dy.nodeName())
        self.assertSldNumber(expectedDispY, dy.firstChild().nodeValue())

    def assertSldNumber(self, expected, stringValue):
        value = float(stringValue)
        self.assertFloatEquals(expected, value, 0.01)

    def assertFloatEquals(self, expected, actual, tol):
        self.assertLess(abs(expected - actual), tol)

    def testSimpleMarkerUnitDefault(self):
        symbol = QgsSimpleMarkerSymbolLayer(
            QgsSimpleMarkerSymbolLayerBase.Star, color=QColor(255, 0, 0), borderColor=QColor(0, 255, 0), size=10)
        symbol.setOutlineWidth(3)
        symbol.setOffset(QPointF(5, 10))
        dom, root = self.symbolToSld(symbol)
        # print("Simple marker unit mm: " + root.ownerDocument().toString())

        # Check the size has been rescaled to pixels
        self.assertStaticSize(root, '36')

        # Check the same happened to the outline width
        self.assertStrokeWidth(root, 2, 11)
        self.assertStaticDisplacement(root, 18, 36)

    def assertStrokeWidth(self, root, svgParameterIdx, expectedWidth):
        strokeWidth = root.elementsByTagName(
            'se:SvgParameter').item(svgParameterIdx)
        svgParameterName = strokeWidth.attributes().namedItem('name')
        self.assertEqual("stroke-width", svgParameterName.nodeValue())
        self.assertSldNumber(
            expectedWidth, strokeWidth.firstChild().nodeValue())

    def testSimpleMarkerUnitPixels(self):
        symbol = QgsSimpleMarkerSymbolLayer(
            QgsSimpleMarkerSymbolLayerBase.Star, color=QColor(255, 0, 0), borderColor=QColor(0, 255, 0), size=10)
        symbol.setOutlineWidth(3)
        symbol.setOffset(QPointF(5, 10))
        symbol.setOutputUnit(QgsUnitTypes.RenderPixels)
        dom, root = self.symbolToSld(symbol)
        # print("Marker unit mm: " + root.ownerDocument().toString())

        # Check the size has not been rescaled
        self.assertStaticSize(root, '10')

        # Check the same happened to the outline width
        self.assertStrokeWidth(root, 2, 3)
        self.assertStaticDisplacement(root, 5, 10)

    def testSvgMarkerUnitDefault(self):
        symbol = QgsSvgMarkerSymbolLayer('symbols/star.svg', 10, 90)
        symbol.setOffset(QPointF(5, 10))

        dom, root = self.symbolToSld(symbol)
        # print("Svg marker mm: " + dom.toString())

        # Check the size has been rescaled
        self.assertStaticSize(root, '36')

        # Check rotation for good measure
        self.assertStaticRotation(root, '90')
        self.assertStaticDisplacement(root, 18, 36)

    def testSvgMarkerUnitPixels(self):
        symbol = QgsSvgMarkerSymbolLayer('symbols/star.svg', 10, 0)
        symbol.setOffset(QPointF(5, 10))
        symbol.setOutputUnit(QgsUnitTypes.RenderPixels)
        dom, root = self.symbolToSld(symbol)
        # print("Svg marker unit px: " + dom.toString())

        # Check the size has not been rescaled
        self.assertStaticSize(root, '10')
        self.assertStaticDisplacement(root, 5, 10)

    def testFontMarkerUnitDefault(self):
        symbol = QgsFontMarkerSymbolLayer('sans', ',', 10, QColor('black'), 45)
        symbol.setOffset(QPointF(5, 10))
        dom, root = self.symbolToSld(symbol)
        # print "Font marker unit mm: " + dom.toString()

        # Check the size has been rescaled
        self.assertStaticSize(root, '36')
        self.assertStaticRotation(root, '45')
        self.assertStaticDisplacement(root, 18, 36)

    def testFontMarkerUnitPixel(self):
        symbol = QgsFontMarkerSymbolLayer('sans', ',', 10, QColor('black'), 45)
        symbol.setOffset(QPointF(5, 10))
        symbol.setOutputUnit(QgsUnitTypes.RenderPixels)
        dom, root = self.symbolToSld(symbol)
        # print ("Font marker unit mm: " + dom.toString())

        # Check the size has been rescaled
        self.assertStaticSize(root, '10')
        self.assertStaticRotation(root, '45')
        self.assertStaticDisplacement(root, 5, 10)

    def createEllipseSymbolLayer(self):
        # No way to build it programmatically...
        mTestName = 'QgsEllipseSymbolLayer'
        mFilePath = QDir.toNativeSeparators(
            '%s/symbol_layer/%s.sld' % (unitTestDataPath(), mTestName))

        mDoc = QDomDocument(mTestName)
        mFile = QFile(mFilePath)
        mFile.open(QIODevice.ReadOnly)
        mDoc.setContent(mFile, True)
        mFile.close()
        mSymbolLayer = QgsEllipseSymbolLayer.createFromSld(
            mDoc.elementsByTagName('PointSymbolizer').item(0).toElement())
        return mSymbolLayer

    def testEllipseMarkerUnitDefault(self):
        symbol = self.createEllipseSymbolLayer()
        symbol.setOffset(QPointF(5, 10))
        symbol.setOutputUnit(QgsUnitTypes.RenderMillimeters)
        dom, root = self.symbolToSld(symbol)
        # print ("Ellipse marker unit mm: " + dom.toString())

        # Check the size has been rescaled
        self.assertStaticSize(root, '25')
        # Check also the stroke width
        self.assertStrokeWidth(root, 2, 4)
        self.assertStaticDisplacement(root, 18, 36)

    def testEllipseMarkerUnitPixel(self):
        symbol = self.createEllipseSymbolLayer()
        symbol.setOffset(QPointF(5, 10))
        symbol.setOutputUnit(QgsUnitTypes.RenderPixels)
        dom, root = self.symbolToSld(symbol)
        # print ("Ellipse marker unit mm: " + dom.toString())

        # Check the size has been rescaled
        self.assertStaticSize(root, '7')
        # Check also the stroke width
        self.assertStrokeWidth(root, 2, 1)
        self.assertStaticDisplacement(root, 5, 10)

    def testSimpleLineUnitDefault(self):
        symbol = QgsSimpleLineSymbolLayer(QColor("black"), 1)
        symbol.setCustomDashVector([10, 10])
        symbol.setUseCustomDashPattern(True)
        symbol.setOffset(5)
        dom, root = self.symbolToSld(symbol)

        # print ("Simple line px: \n" + dom.toString())

        self.assertStrokeWidth(root, 1, 4)
        self.assertDashPattern(root, 4, '36 36')
        self.assertStaticPerpendicularOffset(root, '18')

    def testSimpleLineUnitPixel(self):
        symbol = QgsSimpleLineSymbolLayer(QColor("black"), 1)
        symbol.setCustomDashVector([10, 10])
        symbol.setUseCustomDashPattern(True)
        symbol.setOffset(5)
        symbol.setOutputUnit(QgsUnitTypes.RenderPixels)
        dom, root = self.symbolToSld(symbol)

        # print ("Simple line px: \n" + dom.toString())

        self.assertStrokeWidth(root, 1, 1)
        self.assertDashPattern(root, 4, '10 10')
        self.assertStaticPerpendicularOffset(root, '5')

    def testMarkLineUnitDefault(self):
        symbol = QgsMarkerLineSymbolLayer()
        symbol.setSubSymbol(
            QgsMarkerSymbol.createSimple({'color': '#ffffff', 'size': '3'}))
        symbol.setInterval(5)
        symbol.setOffset(5)
        dom, root = self.symbolToSld(symbol)

        # print ("Mark line mm: \n" + dom.toString())

        # size of the mark
        self.assertStaticSize(root, '11')
        # gap and offset
        self.assertStaticGap(root, '18')
        self.assertStaticPerpendicularOffset(root, '18')

    def testMarkLineUnitPixels(self):
        symbol = QgsMarkerLineSymbolLayer()
        symbol.setSubSymbol(
            QgsMarkerSymbol.createSimple({'color': '#ffffff', 'size': '3'}))
        symbol.setInterval(5)
        symbol.setOffset(5)
        symbol.setOutputUnit(QgsUnitTypes.RenderPixels)
        dom, root = self.symbolToSld(symbol)

        # print ("Mark line px: \n" + dom.toString())

        # size of the mark
        self.assertStaticSize(root, '3')
        # gap and offset
        self.assertStaticGap(root, '5')
        self.assertStaticPerpendicularOffset(root, '5')

    def testSimpleFillDefault(self):
        symbol = QgsSimpleFillSymbolLayer(
            QColor('red'), Qt.SolidPattern, QColor('green'), Qt.SolidLine, 5)
        symbol.setOffset(QPointF(5, 10))

        dom, root = self.symbolToSld(symbol)

        # print ("Simple fill mm: \n" + dom.toString())

        self.assertStrokeWidth(root, 2, 18)
        self.assertStaticDisplacement(root, 18, 36)

    def testSimpleFillPixels(self):
        symbol = QgsSimpleFillSymbolLayer(
            QColor('red'), Qt.SolidPattern, QColor('green'), Qt.SolidLine, 5)
        symbol.setOffset(QPointF(5, 10))
        symbol.setOutputUnit(QgsUnitTypes.RenderPixels)

        dom, root = self.symbolToSld(symbol)
        # print ( "Simple fill px: \n" + dom.toString())

        self.assertStrokeWidth(root, 2, 5)
        self.assertStaticDisplacement(root, 5, 10)

    def testSvgFillDefault(self):
        symbol = QgsSVGFillSymbolLayer('test/star.svg', 10, 45)
        symbol.setSvgOutlineWidth(3)

        dom, root = self.symbolToSld(symbol)
        # print ("Svg fill mm: \n" + dom.toString())

        self.assertStaticRotation(root, '45')
        self.assertStaticSize(root, '36')
        # width of the svg outline
        self.assertStrokeWidth(root, 1, 11)
        # width of the polygon outline
        self.assertStrokeWidth(root, 3, 1)

    def testSvgFillPixel(self):
        symbol = QgsSVGFillSymbolLayer('test/star.svg', 10, 45)
        symbol.setSvgOutlineWidth(3)
        symbol.setOutputUnit(QgsUnitTypes.RenderPixels)

        dom, root = self.symbolToSld(symbol)
        # print ("Svg fill px: \n" + dom.toString())

        self.assertStaticRotation(root, '45')
        self.assertStaticSize(root, '10')
        # width of the svg outline
        self.assertStrokeWidth(root, 1, 3)
        # width of the polygon outline
        self.assertStrokeWidth(root, 3, 0.26)

    def testLineFillDefault(self):
        symbol = QgsLinePatternFillSymbolLayer()
        symbol.setLineAngle(45)
        symbol.setLineWidth(1)
        symbol.setOffset(5)

        dom, root = self.symbolToSld(symbol)
        # print ("Line fill mm: \n" + dom.toString())

        self.assertStaticRotation(root, '45')
        self.assertStrokeWidth(root, 1, 4)
        self.assertStaticSize(root, '18')
        self.assertStaticDisplacement(root, 15, 9)

    def testLineFillPixels(self):
        symbol = QgsLinePatternFillSymbolLayer()
        symbol.setLineAngle(45)
        symbol.setLineWidth(1)
        symbol.setOffset(5)
        symbol.setOutputUnit(QgsUnitTypes.RenderPixels)

        dom, root = self.symbolToSld(symbol)
        # print ("Line fill px: \n" + dom.toString())

        self.assertStaticRotation(root, '45')
        self.assertStrokeWidth(root, 1, 1)
        self.assertStaticSize(root, '5')
        self.assertStaticDisplacement(root, 4.25, 2.63)

    def testPointFillDefault(self):
        symbol = QgsPointPatternFillSymbolLayer()
        dom, root = self.symbolToSld(symbol)
        # print ("Point fill mm: \n" + dom.toString())

        self.assertStaticSize(root, '7')

    def testPointFillpixels(self):
        symbol = QgsPointPatternFillSymbolLayer()
        symbol.setOutputUnit(QgsUnitTypes.RenderPixels)
        dom, root = self.symbolToSld(symbol)
        # print ("Point fill px: \n" + dom.toString())

        self.assertStaticSize(root, '2')

    def testSingleSymbolNoScaleDependencies(self):
        layer = QgsVectorLayer("Point", "addfeat", "memory")
        mFilePath = QDir.toNativeSeparators('%s/symbol_layer/%s.qml' % (unitTestDataPath(), "singleSymbol"))
        layer.loadNamedStyle(mFilePath)

        dom, root = self.layerToSld(layer)
        # print("No dep on single symbol:" + dom.toString())

        self.assertScaleDenominator(root, None, None)

    def testSingleSymbolScaleDependencies(self):
        layer = QgsVectorLayer("Point", "addfeat", "memory")
        mFilePath = QDir.toNativeSeparators('%s/symbol_layer/%s.qml' % (unitTestDataPath(), "singleSymbol"))
        layer.loadNamedStyle(mFilePath)
        layer.setMinimumScale(1000)
        layer.setMaximumScale(500000)
        layer.setScaleBasedVisibility(True)

        dom, root = self.layerToSld(layer)
        # print("Scale dep on single symbol:" + dom.toString())

        self.assertScaleDenominator(root, '1000', '500000')

    def testCategorizedNoScaleDependencies(self):
        layer = QgsVectorLayer("Polygon", "addfeat", "memory")
        mFilePath = QDir.toNativeSeparators('%s/symbol_layer/%s.qml' % (unitTestDataPath(), "categorized"))
        layer.loadNamedStyle(mFilePath)

        dom, root = self.layerToSld(layer)
        # print("Categorized no scale deps:" + dom.toString())

        ruleCount = root.elementsByTagName('se:Rule').size()
        for i in range(0, ruleCount):
            self.assertScaleDenominator(root, None, None, i)

    def testCategorizedWithScaleDependencies(self):
        layer = QgsVectorLayer("Polygon", "addfeat", "memory")
        mFilePath = QDir.toNativeSeparators('%s/symbol_layer/%s.qml' % (unitTestDataPath(), "categorized"))
        layer.loadNamedStyle(mFilePath)
        layer.setMinimumScale(1000)
        layer.setMaximumScale(500000)
        layer.setScaleBasedVisibility(True)

        dom, root = self.layerToSld(layer)
        # print("Categorized with scale deps:" + dom.toString())

        ruleCount = root.elementsByTagName('se:Rule').size()
        for i in range(0, ruleCount):
            self.assertScaleDenominator(root, '1000', '500000', i)

    def testGraduatedNoScaleDependencies(self):
        layer = QgsVectorLayer("Polygon", "addfeat", "memory")

        mFilePath = QDir.toNativeSeparators('%s/symbol_layer/%s.qml' % (unitTestDataPath(), "graduated"))
        status = layer.loadNamedStyle(mFilePath)

        dom, root = self.layerToSld(layer)
        # print("Graduated no scale deps:" + dom.toString())

        ruleCount = root.elementsByTagName('se:Rule').size()
        for i in range(0, ruleCount):
            self.assertScaleDenominator(root, None, None, i)

    def testRuleBasedNoRootScaleDependencies(self):
        layer = QgsVectorLayer("Polygon", "addfeat", "memory")

        mFilePath = QDir.toNativeSeparators('%s/symbol_layer/%s.qml' % (unitTestDataPath(), "ruleBased"))
        status = layer.loadNamedStyle(mFilePath)

        dom, root = self.layerToSld(layer)
        print(("Rule based, no root scale deps:" + dom.toString()))

        ruleCount = root.elementsByTagName('se:Rule').size()
        self.assertScaleDenominator(root, '1000', '40000000', 0)
        self.assertScaleDenominator(root, None, None, 1)

    def testRuleBasedNoRootScaleDependencies(self):
        layer = QgsVectorLayer("Polygon", "addfeat", "memory")

        mFilePath = QDir.toNativeSeparators('%s/symbol_layer/%s.qml' % (unitTestDataPath(), "ruleBased"))
        status = layer.loadNamedStyle(mFilePath)
        layer.setMinimumScale(5000)
        layer.setMaximumScale(50000000)
        layer.setScaleBasedVisibility(True)

        dom, root = self.layerToSld(layer)
        # print("Rule based, with root scale deps:" + dom.toString())

        ruleCount = root.elementsByTagName('se:Rule').size()
        self.assertScaleDenominator(root, '5000', '40000000', 0)
        self.assertScaleDenominator(root, '5000', '50000000', 1)

    def assertScaleDenominator(self, root, expectedMinScale, expectedMaxScale, index=0):
        rule = root.elementsByTagName('se:Rule').item(index).toElement()

        if expectedMinScale:
            minScale = rule.elementsByTagName('se:MinScaleDenominator').item(0)
            self.assertEqual(expectedMinScale, minScale.firstChild().nodeValue())
        else:
            self.assertEqual(0, root.elementsByTagName('se:MinScaleDenominator').size())

        if expectedMaxScale:
            maxScale = rule.elementsByTagName('se:MaxScaleDenominator').item(0)
            self.assertEqual(expectedMaxScale, maxScale.firstChild().nodeValue())
        else:
            self.assertEqual(0, root.elementsByTagName('se:MaxScaleDenominator').size())

    def assertDashPattern(self, root, svgParameterIdx, expectedPattern):
        strokeWidth = root.elementsByTagName(
            'se:SvgParameter').item(svgParameterIdx)
        svgParameterName = strokeWidth.attributes().namedItem('name')
        self.assertEqual("stroke-dasharray", svgParameterName.nodeValue())
        self.assertEqual(
            expectedPattern, strokeWidth.firstChild().nodeValue())

    def assertStaticGap(self, root, expectedValue):
        # Check the rotation element is a literal, not a
        rotation = root.elementsByTagName('se:Gap').item(0)
        literal = rotation.firstChild()
        self.assertEqual("ogc:Literal", literal.nodeName())
        self.assertEqual(expectedValue, literal.firstChild().nodeValue())

    def assertStaticSize(self, root, expectedValue):
        size = root.elementsByTagName('se:Size').item(0)
        self.assertEqual(expectedValue, size.firstChild().nodeValue())

    def assertStaticPerpendicularOffset(self, root, expectedValue):
        offset = root.elementsByTagName('se:PerpendicularOffset').item(0)
        self.assertEqual(expectedValue, offset.firstChild().nodeValue())

    def symbolToSld(self, symbolLayer):
        dom = QDomDocument()
        root = dom.createElement("FakeRoot")
        dom.appendChild(root)
        symbolLayer.toSld(dom, root, {})
        return dom, root

    def layerToSld(self, mapLayer):
        dom = QDomDocument()
        root = dom.createElement("FakeRoot")
        dom.appendChild(root)
        error = None
        mapLayer.writeSld(root, dom, error, {})
        return dom, root


if __name__ == '__main__':
    unittest.main()
