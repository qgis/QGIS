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

import qgis  # NOQA

from qgis.PyQt.QtCore import Qt, QDir, QFile, QIODevice, QPointF, QSizeF
from qgis.PyQt.QtXml import QDomDocument
from qgis.PyQt.QtGui import QColor, QFont

from qgis.core import (
    QgsSimpleMarkerSymbolLayer, QgsSimpleMarkerSymbolLayerBase, QgsUnitTypes, QgsSvgMarkerSymbolLayer,
    QgsFontMarkerSymbolLayer, QgsEllipseSymbolLayer, QgsSimpleLineSymbolLayer,
    QgsMarkerLineSymbolLayer, QgsMarkerSymbol, QgsSimpleFillSymbolLayer, QgsSVGFillSymbolLayer,
    QgsLinePatternFillSymbolLayer, QgsPointPatternFillSymbolLayer, QgsVectorLayer, QgsVectorLayerSimpleLabeling,
    QgsTextBufferSettings, QgsPalLayerSettings, QgsTextBackgroundSettings, QgsRuleBasedLabeling,
    QgsLineSymbol)
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
            QgsSimpleMarkerSymbolLayerBase.Star, color=QColor(255, 0, 0), strokeColor=QColor(0, 255, 0), size=10)
        symbol.setAngle(50)
        dom, root = self.symbolToSld(symbol)
        # print( "Simple marker rotation: " + root.ownerDocument().toString())

        self.assertStaticRotation(root, '50')

    def testSimpleMarkerUnitDefault(self):
        symbol = QgsSimpleMarkerSymbolLayer(
            QgsSimpleMarkerSymbolLayerBase.Star, color=QColor(255, 0, 0), strokeColor=QColor(0, 255, 0), size=10)
        symbol.setStrokeWidth(3)
        symbol.setOffset(QPointF(5, 10))
        dom, root = self.symbolToSld(symbol)
        # print("Simple marker unit mm: " + root.ownerDocument().toString())

        # Check the size has been rescaled to pixels
        self.assertStaticSize(root, '36')

        # Check the same happened to the stroke width
        self.assertStrokeWidth(root, 2, 11)
        self.assertStaticDisplacement(root, 18, 36)

    def testSimpleMarkerUnitPixels(self):
        symbol = QgsSimpleMarkerSymbolLayer(
            QgsSimpleMarkerSymbolLayerBase.Star, color=QColor(255, 0, 0), strokeColor=QColor(0, 255, 0), size=10)
        symbol.setStrokeWidth(3)
        symbol.setOffset(QPointF(5, 10))
        symbol.setOutputUnit(QgsUnitTypes.RenderPixels)
        dom, root = self.symbolToSld(symbol)
        # print("Marker unit mm: " + root.ownerDocument().toString())

        # Check the size has not been rescaled
        self.assertStaticSize(root, '10')

        # Check the same happened to the stroke width
        self.assertStrokeWidth(root, 2, 3)
        self.assertStaticDisplacement(root, 5, 10)

    def testSvgMarkerUnitDefault(self):
        symbol = QgsSvgMarkerSymbolLayer('symbols/star.svg', 10, 90)
        symbol.setFillColor(QColor("blue"))
        symbol.setStrokeWidth(1)
        symbol.setStrokeColor(QColor('red'))
        symbol.setPath('symbols/star.svg')
        symbol.setOffset(QPointF(5, 10))

        dom, root = self.symbolToSld(symbol)
        # print("Svg marker mm: " + dom.toString())

        self.assertExternalGraphic(root, 0,
                                   'symbols/star.svg?fill=%230000ff&fill-opacity=1&outline=%23ff0000&outline-opacity=1&outline-width=4',
                                   'image/svg+xml')
        self.assertExternalGraphic(root, 1,
                                   'symbols/star.svg', 'image/svg+xml')
        self.assertWellKnownMark(root, 0, 'square', '#0000ff', '#ff0000', 4)

        # Check the size has been rescaled
        self.assertStaticSize(root, '36')

        # Check rotation for good measure
        self.assertStaticRotation(root, '90')
        self.assertStaticDisplacement(root, 18, 36)

    def testSvgMarkerUnitPixels(self):
        symbol = QgsSvgMarkerSymbolLayer('symbols/star.svg', 10, 0)
        symbol.setFillColor(QColor("blue"))
        symbol.setStrokeWidth(1)
        symbol.setStrokeColor(QColor('red'))
        symbol.setPath('symbols/star.svg')
        symbol.setOffset(QPointF(5, 10))
        symbol.setOutputUnit(QgsUnitTypes.RenderPixels)
        dom, root = self.symbolToSld(symbol)
        # print("Svg marker unit px: " + dom.toString())

        self.assertExternalGraphic(root, 0,
                                   'symbols/star.svg?fill=%230000ff&fill-opacity=1&outline=%23ff0000&outline-opacity=1&outline-width=1',
                                   'image/svg+xml')
        self.assertExternalGraphic(root, 1,
                                   'symbols/star.svg', 'image/svg+xml')
        self.assertWellKnownMark(root, 0, 'square', '#0000ff', '#ff0000', 1)

        # Check the size has not been rescaled
        self.assertStaticSize(root, '10')
        self.assertStaticDisplacement(root, 5, 10)

    def testFontMarkerUnitDefault(self):
        symbol = QgsFontMarkerSymbolLayer('sans', ',', 10, QColor('black'), 45)
        symbol.setOffset(QPointF(5, 10))
        dom, root = self.symbolToSld(symbol)
        # print("Font marker unit mm: " + dom.toString())

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

    def testSimpleLineHairline(self):
        symbol = QgsSimpleLineSymbolLayer(QColor("black"), 0)
        dom, root = self.symbolToSld(symbol)

        # print ("Simple line px: \n" + dom.toString())

        # Hairline is turned into 0.5px
        self.assertStrokeWidth(root, 1, 0.5)

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
        symbol.setSvgFillColor(QColor('blue'))
        symbol.setSvgStrokeWidth(3)
        symbol.setSvgStrokeColor(QColor('yellow'))
        symbol.setSubSymbol(QgsLineSymbol())
        symbol.subSymbol().setWidth(10)

        dom, root = self.symbolToSld(symbol)
        # print ("Svg fill mm: \n" + dom.toString())

        self.assertExternalGraphic(root, 0,
                                   'test/star.svg?fill=%230000ff&fill-opacity=1&outline=%23ffff00&outline-opacity=1&outline-width=11',
                                   'image/svg+xml')
        self.assertExternalGraphic(root, 1,
                                   'test/star.svg', 'image/svg+xml')
        self.assertWellKnownMark(root, 0, 'square', '#0000ff', '#ffff00', 11)

        self.assertStaticRotation(root, '45')
        self.assertStaticSize(root, '36')
        # width of the polygon stroke
        lineSymbolizer = root.elementsByTagName('se:LineSymbolizer').item(0).toElement()
        self.assertStrokeWidth(lineSymbolizer, 1, 36)

    def testSvgFillPixel(self):
        symbol = QgsSVGFillSymbolLayer('test/star.svg', 10, 45)
        symbol.setSubSymbol(QgsLineSymbol())
        symbol.setSvgFillColor(QColor('blue'))
        symbol.setSvgStrokeWidth(3)
        symbol.setSvgStrokeColor(QColor('black'))
        symbol.setOutputUnit(QgsUnitTypes.RenderPixels)
        symbol.subSymbol().setWidth(10)

        dom, root = self.symbolToSld(symbol)
        # print ("Svg fill px: \n" + dom.toString())

        self.assertExternalGraphic(root, 0,
                                   'test/star.svg?fill=%230000ff&fill-opacity=1&outline=%23000000&outline-opacity=1&outline-width=3',
                                   'image/svg+xml')
        self.assertExternalGraphic(root, 1,
                                   'test/star.svg', 'image/svg+xml')
        self.assertWellKnownMark(root, 0, 'square', '#0000ff', '#000000', 3)

        self.assertStaticRotation(root, '45')
        self.assertStaticSize(root, '10')
        # width of the polygon stroke
        lineSymbolizer = root.elementsByTagName('se:LineSymbolizer').item(0).toElement()
        self.assertStrokeWidth(lineSymbolizer, 1, 10)

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
        layer.setMaximumScale(1000)
        layer.setMinimumScale(500000)
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
        layer.setMaximumScale(1000)
        layer.setMinimumScale(500000)
        layer.setScaleBasedVisibility(True)

        dom, root = self.layerToSld(layer)
        # print("Categorized with scale deps:" + dom.toString())

        ruleCount = root.elementsByTagName('se:Rule').size()
        for i in range(0, ruleCount):
            self.assertScaleDenominator(root, '1000', '500000', i)

    def testGraduatedNoScaleDependencies(self):
        layer = QgsVectorLayer("Polygon", "addfeat", "memory")

        mFilePath = QDir.toNativeSeparators('%s/symbol_layer/%s.qml' % (unitTestDataPath(), "graduated"))
        status = layer.loadNamedStyle(mFilePath)  # NOQA

        dom, root = self.layerToSld(layer)
        # print("Graduated no scale deps:" + dom.toString())

        ruleCount = root.elementsByTagName('se:Rule').size()
        for i in range(0, ruleCount):
            self.assertScaleDenominator(root, None, None, i)

    #    def testRuleBasedNoRootScaleDependencies(self):
    #        layer = QgsVectorLayer("Polygon", "addfeat", "memory")
    #
    #        mFilePath = QDir.toNativeSeparators('%s/symbol_layer/%s.qml' % (unitTestDataPath(), "ruleBased"))
    #        status = layer.loadNamedStyle(mFilePath)  # NOQA
    #
    #        dom, root = self.layerToSld(layer)
    #        print(("Rule based, no root scale deps:" + dom.toString()))
    #
    #        ruleCount = root.elementsByTagName('se:Rule').size()  # NOQA
    #        self.assertScaleDenominator(root, '1000', '40000000', 0)
    #        self.assertScaleDenominator(root, None, None, 1)

    def testRuleBasedNoRootScaleDependencies(self):
        layer = QgsVectorLayer("Polygon", "addfeat", "memory")

        mFilePath = QDir.toNativeSeparators('%s/symbol_layer/%s.qml' % (unitTestDataPath(), "ruleBased"))
        status = layer.loadNamedStyle(mFilePath)  # NOQA
        layer.setMaximumScale(5000)
        layer.setMinimumScale(50000000)
        layer.setScaleBasedVisibility(True)

        dom, root = self.layerToSld(layer)
        # print("Rule based, with root scale deps:" + dom.toString())

        ruleCount = root.elementsByTagName('se:Rule').size()  # NOQA
        self.assertScaleDenominator(root, '5000', '40000000', 0)
        self.assertScaleDenominator(root, '5000', '50000000', 1)

    def testCategorizedFunctionConflict(self):
        layer = QgsVectorLayer("Point", "addfeat", "memory")

        mFilePath = QDir.toNativeSeparators(
            '%s/symbol_layer/%s.qml' % (unitTestDataPath(), "categorizedFunctionConflict"))
        status = layer.loadNamedStyle(mFilePath)  # NOQA

        dom, root = self.layerToSld(layer)
        # print("Rule based, with root scale deps:" + dom.toString())

        ruleCount = root.elementsByTagName('se:Rule').size()  # NOQA
        self.assertEqual(7, ruleCount)
        self.assertRuleRangeFilter(root, 0, 'Area', '0', True, '500', True)
        self.assertRuleRangeFilter(root, 1, 'Area', '500', False, '1000', True)
        self.assertRuleRangeFilter(root, 2, 'Area', '1000', False, '5000', True)
        self.assertRuleRangeFilter(root, 3, 'Area', '5000', False, '10000', True)
        self.assertRuleRangeFilter(root, 4, 'Area', '10000', False, '50000', True)
        self.assertRuleRangeFilter(root, 5, 'Area', '50000', False, '100000', True)
        self.assertRuleRangeFilter(root, 6, 'Area', '100000', False, '200000', True)

    def assertRuleRangeFilter(self, root, index, attributeName, min, includeMin, max, includeMax):
        rule = root.elementsByTagName('se:Rule').item(index).toElement()
        filter = rule.elementsByTagName("Filter").item(0).firstChild()
        self.assertEqual("ogc:And", filter.nodeName())

        gt = filter.firstChild()
        expectedGtName = "ogc:PropertyIsGreaterThanOrEqualTo" if includeMin else "ogc:PropertyIsGreaterThan"
        self.assertEqual(expectedGtName, gt.nodeName())
        gtProperty = gt.firstChild()
        self.assertEqual("ogc:PropertyName", gtProperty.nodeName())
        self.assertEqual(attributeName, gtProperty.toElement().text())
        gtValue = gt.childNodes().item(1)
        self.assertEqual(min, gtValue.toElement().text())

        lt = filter.childNodes().item(1)
        expectedLtName = "ogc:PropertyIsLessThanOrEqualTo" if includeMax else "ogc:PropertyIsLessThan"
        self.assertEqual(expectedLtName, lt.nodeName())
        ltProperty = lt.firstChild()
        self.assertEqual("ogc:PropertyName", ltProperty.nodeName())
        self.assertEqual(attributeName, ltProperty.toElement().text())
        ltValue = lt.childNodes().item(1)
        self.assertEqual(max, ltValue.toElement().text())

    def testSimpleLabeling(self):
        layer = QgsVectorLayer("Point", "addfeat", "memory")
        self.loadStyleWithCustomProperties(layer, "simpleLabel")
        # Pick a local default font
        fontFamily = QFont().family()
        settings = layer.labeling().settings()
        format = settings.format()
        font = format.font()
        font.setFamily(fontFamily)
        font.setBold(False)
        font.setItalic(False)
        format.setFont(font)
        settings.setFormat(format)
        layer.setLabeling(QgsVectorLayerSimpleLabeling(settings))

        dom, root = self.layerToSld(layer)
        # print("Simple label text symbolizer" + dom.toString())

        ts = self.getTextSymbolizer(root, 1, 0)
        self.assertPropertyName(ts, 'se:Label', 'NAME')
        font = self.assertElement(ts, 'se:Font', 0)
        self.assertEqual(fontFamily, self.assertSvgParameter(font, 'font-family').text())
        self.assertEqual('11', self.assertSvgParameter(font, 'font-size').text())

        fill = self.assertElement(ts, 'se:Fill', 0)
        self.assertEqual('#000000', self.assertSvgParameter(fill, "fill").text())
        self.assertIsNone(self.assertSvgParameter(fill, "fill-opacity", True))

    def testLabelingUomMillimeter(self):
        layer = QgsVectorLayer("Point", "addfeat", "memory")
        self.loadStyleWithCustomProperties(layer, "simpleLabel")
        self.updateLayerLabelingUnit(layer, QgsUnitTypes.RenderMillimeters)

        dom, root = self.layerToSld(layer)
        # print("Label sized in mm " + dom.toString())

        ts = self.getTextSymbolizer(root, 1, 0)
        font = self.assertElement(ts, 'se:Font', 0)
        self.assertEqual('32', self.assertSvgParameter(font, 'font-size').text())

    def testLabelingUomPixels(self):
        layer = QgsVectorLayer("Point", "addfeat", "memory")
        self.loadStyleWithCustomProperties(layer, "simpleLabel")
        self.updateLayerLabelingUnit(layer, QgsUnitTypes.RenderPixels)

        dom, root = self.layerToSld(layer)
        # print("Label sized in pixels " + dom.toString())

        ts = self.getTextSymbolizer(root, 1, 0)
        font = self.assertElement(ts, 'se:Font', 0)
        self.assertEqual('9', self.assertSvgParameter(font, 'font-size').text())

    def testLabelingUomInches(self):
        layer = QgsVectorLayer("Point", "addfeat", "memory")
        self.loadStyleWithCustomProperties(layer, "simpleLabel")
        self.updateLayerLabelingUnit(layer, QgsUnitTypes.RenderInches)

        dom, root = self.layerToSld(layer)
        # print("Label sized in inches " + dom.toString())

        ts = self.getTextSymbolizer(root, 1, 0)
        font = self.assertElement(ts, 'se:Font', 0)
        self.assertEqual('816', self.assertSvgParameter(font, 'font-size').text())

    def testTextStyle(self):
        layer = QgsVectorLayer("Point", "addfeat", "memory")
        self.loadStyleWithCustomProperties(layer, "simpleLabel")

        # testing regular
        self.updateLayerLabelingFontStyle(layer, False, False)
        dom, root = self.layerToSld(layer)
        # print("Simple label italic text" + dom.toString())
        ts = self.getTextSymbolizer(root, 1, 0)
        font = self.assertElement(ts, 'se:Font', 0)
        self.assertIsNone(self.assertSvgParameter(font, 'font-weight', True))
        self.assertIsNone(self.assertSvgParameter(font, 'font-style', True))

        # testing bold
        self.updateLayerLabelingFontStyle(layer, True, False)
        dom, root = self.layerToSld(layer)
        # print("Simple label bold text" + dom.toString())
        ts = self.getTextSymbolizer(root, 1, 0)
        font = self.assertElement(ts, 'se:Font', 0)
        self.assertEqual('bold', self.assertSvgParameter(font, 'font-weight').text())
        self.assertIsNone(self.assertSvgParameter(font, 'font-style', True))

        # testing italic
        self.updateLayerLabelingFontStyle(layer, False, True)
        dom, root = self.layerToSld(layer)
        # print("Simple label italic text" + dom.toString())
        ts = self.getTextSymbolizer(root, 1, 0)
        font = self.assertElement(ts, 'se:Font', 0)
        self.assertEqual('italic', self.assertSvgParameter(font, 'font-style').text())
        self.assertIsNone(self.assertSvgParameter(font, 'font-weight', True))

        # testing bold italic
        self.updateLayerLabelingFontStyle(layer, True, True)
        dom, root = self.layerToSld(layer)
        # print("Simple label bold and italic text" + dom.toString())
        ts = self.getTextSymbolizer(root, 1, 0)
        font = self.assertElement(ts, 'se:Font', 0)
        self.assertEqual('italic', self.assertSvgParameter(font, 'font-style').text())
        self.assertEqual('bold', self.assertSvgParameter(font, 'font-weight').text())

        # testing underline and strikethrough vendor options
        self.updateLayerLabelingFontStyle(layer, False, False, True, True)
        dom, root = self.layerToSld(layer)
        # print("Simple label underline and strikethrough text" + dom.toString())
        ts = self.getTextSymbolizer(root, 1, 0)
        font = self.assertElement(ts, 'se:Font', 0)
        self.assertEqual('true', self.assertVendorOption(ts, 'underlineText').text())
        self.assertEqual('true', self.assertVendorOption(ts, 'strikethroughText').text())

    def testTextMixedCase(self):
        self.assertCapitalizationFunction(QFont.MixedCase, None)

    def testTextUppercase(self):
        self.assertCapitalizationFunction(QFont.AllUppercase, "strToUpperCase")

    def testTextLowercase(self):
        self.assertCapitalizationFunction(QFont.AllLowercase, "strToLowerCase")

    def testTextCapitalcase(self):
        self.assertCapitalizationFunction(QFont.Capitalize, "strCapitalize")

    def assertCapitalizationFunction(self, capitalization, expectedFunction):
        layer = QgsVectorLayer("Point", "addfeat", "memory")
        self.loadStyleWithCustomProperties(layer, "simpleLabel")

        settings = layer.labeling().settings()
        format = settings.format()
        font = format.font()
        font.setCapitalization(capitalization)
        format.setFont(font)
        settings.setFormat(format)
        layer.setLabeling(QgsVectorLayerSimpleLabeling(settings))

        dom, root = self.layerToSld(layer)
        # print("Simple text with capitalization " + str(QFont.AllUppercase) + ": " + dom.toString())
        ts = self.getTextSymbolizer(root, 1, 0)
        label = self.assertElement(ts, "se:Label", 0)
        if expectedFunction is None:
            property = self.assertElement(label, "ogc:PropertyName", 0)
            self.assertEqual("NAME", property.text())
        else:
            function = self.assertElement(label, "ogc:Function", 0)
            self.assertEqual(expectedFunction, function.attribute("name"))
            property = self.assertElement(function, "ogc:PropertyName", 0)
            self.assertEqual("NAME", property.text())

    def testLabelingTransparency(self):
        layer = QgsVectorLayer("Point", "addfeat", "memory")
        self.loadStyleWithCustomProperties(layer, "simpleLabel")
        settings = layer.labeling().settings()
        format = settings.format()
        format.setOpacity(0.5)
        settings.setFormat(format)
        layer.setLabeling(QgsVectorLayerSimpleLabeling(settings))

        dom, root = self.layerToSld(layer)
        # print("Label with transparency  " + dom.toString())

        ts = self.getTextSymbolizer(root, 1, 0)
        fill = self.assertElement(ts, 'se:Fill', 0)
        self.assertEqual('#000000', self.assertSvgParameter(fill, "fill").text())
        self.assertEqual('0.5', self.assertSvgParameter(fill, "fill-opacity").text())

    def testLabelingBuffer(self):
        layer = QgsVectorLayer("Point", "addfeat", "memory")
        self.loadStyleWithCustomProperties(layer, "simpleLabel")
        buffer = QgsTextBufferSettings()
        buffer.setEnabled(True)
        buffer.setSize(10)
        buffer.setSizeUnit(QgsUnitTypes.RenderPixels)
        buffer.setColor(QColor("Black"))
        self.setLabelBufferSettings(layer, buffer)

        dom, root = self.layerToSld(layer)
        # print("Label with buffer 10 px  " + dom.toString())

        ts = self.getTextSymbolizer(root, 1, 0)
        halo = self.assertElement(ts, 'se:Halo', 0)
        # not full width, just radius here
        self.assertEqual('5', self.assertElement(ts, 'se:Radius', 0).text())
        haloFill = self.assertElement(halo, 'se:Fill', 0)
        self.assertEqual('#000000', self.assertSvgParameter(haloFill, "fill").text())

    def testLabelingBufferPointTranslucent(self):
        layer = QgsVectorLayer("Point", "addfeat", "memory")
        self.loadStyleWithCustomProperties(layer, "simpleLabel")
        buffer = QgsTextBufferSettings()
        buffer.setEnabled(True)
        buffer.setSize(10)
        buffer.setSizeUnit(QgsUnitTypes.RenderPoints)
        buffer.setColor(QColor("Red"))
        buffer.setOpacity(0.5)
        self.setLabelBufferSettings(layer, buffer)

        dom, root = self.layerToSld(layer)
        # print("Label with buffer 10 points, red 50% transparent  " + dom.toString())

        ts = self.getTextSymbolizer(root, 1, 0)
        halo = self.assertElement(ts, 'se:Halo', 0)
        # not full width, just radius here
        self.assertEqual('6.5', self.assertElement(ts, 'se:Radius', 0).text())
        haloFill = self.assertElement(halo, 'se:Fill', 0)
        self.assertEqual('#ff0000', self.assertSvgParameter(haloFill, "fill").text())
        self.assertEqual('0.5', self.assertSvgParameter(haloFill, "fill-opacity").text())

    def testLabelingLowPriority(self):
        self.assertLabelingPriority(0, 0, '0')

    def testLabelingDefaultPriority(self):
        self.assertLabelingPriority(0, 5, None)

    def testLabelingHighPriority(self):
        self.assertLabelingPriority(0, 10, '1000')

    def testLabelingZIndexLowPriority(self):
        self.assertLabelingPriority(1, 0, '1001')

    def testLabelingZIndexDefaultPriority(self):
        self.assertLabelingPriority(1, 5, "1500")

    def testLabelingZIndexHighPriority(self):
        self.assertLabelingPriority(1, 10, '2000')

    def assertLabelingPriority(self, zIndex, priority, expectedSldPriority):
        layer = QgsVectorLayer("Point", "addfeat", "memory")
        self.loadStyleWithCustomProperties(layer, "simpleLabel")
        settings = layer.labeling().settings()
        settings.zIndex = zIndex
        settings.priority = priority
        layer.setLabeling(QgsVectorLayerSimpleLabeling(settings))

        dom, root = self.layerToSld(layer)
        # print("Label with zIndex at " + str(zIndex) + " and priority at " + str(priority) + ": " + dom.toString())

        ts = self.getTextSymbolizer(root, 1, 0)
        priorityElement = self.assertElement(ts, "se:Priority", 0, True)
        if expectedSldPriority is None:
            self.assertIsNone(priorityElement)
        else:
            self.assertEqual(expectedSldPriority, priorityElement.text())

    def testLabelingPlacementOverPointOffsetRotation(self):
        layer = QgsVectorLayer("Point", "addfeat", "memory")
        self.loadStyleWithCustomProperties(layer, "simpleLabel")
        settings = layer.labeling().settings()
        settings.placement = QgsPalLayerSettings.OverPoint
        settings.xOffset = 5
        settings.yOffset = 10
        settings.offsetUnits = QgsUnitTypes.RenderMillimeters
        settings.quadOffset = QgsPalLayerSettings.QuadrantOver
        settings.angleOffset = 30
        layer.setLabeling(QgsVectorLayerSimpleLabeling(settings))

        dom, root = self.layerToSld(layer)
        # print("Label with 'over point' placement  " + dom.toString())

        ts = self.getTextSymbolizer(root, 1, 0)
        pointPlacement = self.assertPointPlacement(ts)
        self.assertStaticDisplacement(pointPlacement, 18, 36)
        self.assertStaticAnchorPoint(pointPlacement, 0.5, 0.5)

    def testPointPlacementAboveLeft(self):
        self.assertLabelQuadrant(QgsPalLayerSettings.QuadrantAboveLeft, "AboveLeft", 1, 0)

    def testPointPlacementAbove(self):
        self.assertLabelQuadrant(QgsPalLayerSettings.QuadrantAbove, "Above", 0.5, 0)

    def testPointPlacementAboveRight(self):
        self.assertLabelQuadrant(QgsPalLayerSettings.QuadrantAboveRight, "AboveRight", 0, 0)

    def testPointPlacementLeft(self):
        self.assertLabelQuadrant(QgsPalLayerSettings.QuadrantLeft, "Left", 1, 0.5)

    def testPointPlacementRight(self):
        self.assertLabelQuadrant(QgsPalLayerSettings.QuadrantRight, "Right", 0, 0.5)

    def testPointPlacementBelowLeft(self):
        self.assertLabelQuadrant(QgsPalLayerSettings.QuadrantBelowLeft, "BelowLeft", 1, 1)

    def testPointPlacementBelow(self):
        self.assertLabelQuadrant(QgsPalLayerSettings.QuadrantBelow, "Below", 0.5, 1)

    def testPointPlacementAboveRight(self):
        self.assertLabelQuadrant(QgsPalLayerSettings.QuadrantBelowRight, "BelowRight", 0, 1)

    def testPointPlacementCartoraphic(self):
        self.assertPointPlacementDistance(QgsPalLayerSettings.OrderedPositionsAroundPoint)

    def testPointPlacementCartoraphic(self):
        self.assertPointPlacementDistance(QgsPalLayerSettings.AroundPoint)

    def testLineParallelPlacement(self):
        layer = QgsVectorLayer("LineString", "addfeat", "memory")
        self.loadStyleWithCustomProperties(layer, "lineLabel")

        dom, root = self.layerToSld(layer)
        # print("Label with parallel line placement  " + dom.toString())
        linePlacement = self.assertLinePlacement(root)
        generalize = self.assertElement(linePlacement, 'se:GeneralizeLine', 0)
        self.assertEqual("true", generalize.text())

    def testLineParallelPlacementOffsetRepeat(self):
        layer = QgsVectorLayer("LineString", "addfeat", "memory")
        self.loadStyleWithCustomProperties(layer, "lineLabel")
        self.updateLinePlacementProperties(layer, QgsPalLayerSettings.Line, 2, 50)

        dom, root = self.layerToSld(layer)
        # print("Label with parallel line placement, perp. offset and repeat  " + dom.toString())
        ts = self.getTextSymbolizer(root, 1, 0)
        linePlacement = self.assertLinePlacement(ts)
        generalize = self.assertElement(linePlacement, 'se:GeneralizeLine', 0)
        self.assertEqual("true", generalize.text())
        offset = self.assertElement(linePlacement, 'se:PerpendicularOffset', 0)
        self.assertEqual("7", offset.text())
        repeat = self.assertElement(linePlacement, 'se:Repeat', 0)
        self.assertEqual("true", repeat.text())
        gap = self.assertElement(linePlacement, 'se:Gap', 0)
        self.assertEqual("179", gap.text())
        self.assertEqual("179", self.assertVendorOption(ts, "repeat").text())

    def testLineCurvePlacementOffsetRepeat(self):
        layer = QgsVectorLayer("LineString", "addfeat", "memory")
        self.loadStyleWithCustomProperties(layer, "lineLabel")
        self.updateLinePlacementProperties(layer, QgsPalLayerSettings.Curved, 2, 50, 30, 40)

        dom, root = self.layerToSld(layer)
        # print("Label with curved line placement  " + dom.toString())

        ts = self.getTextSymbolizer(root, 1, 0)
        linePlacement = self.assertLinePlacement(ts)
        generalize = self.assertElement(linePlacement, 'se:GeneralizeLine', 0)
        self.assertEqual("true", generalize.text())
        offset = self.assertElement(linePlacement, 'se:PerpendicularOffset', 0)
        self.assertEqual("7", offset.text())
        repeat = self.assertElement(linePlacement, 'se:Repeat', 0)
        self.assertEqual("true", repeat.text())
        gap = self.assertElement(linePlacement, 'se:Gap', 0)
        self.assertEqual("179", gap.text())
        self.assertEqual("179", self.assertVendorOption(ts, "repeat").text())
        self.assertEqual("true", self.assertVendorOption(ts, "followLine").text())
        self.assertEqual("30", self.assertVendorOption(ts, "maxAngleDelta").text())

    def testLineCurveMergeLines(self):
        layer = QgsVectorLayer("LineString", "addfeat", "memory")
        self.loadStyleWithCustomProperties(layer, "lineLabel")
        settings = layer.labeling().settings()
        settings.placement = QgsPalLayerSettings.Curved
        settings.mergeLines = True
        settings.labelPerPart = True
        layer.setLabeling(QgsVectorLayerSimpleLabeling(settings))

        dom, root = self.layerToSld(layer)
        # print("Label with curved line and line grouping  " + dom.toString())

        ts = self.getTextSymbolizer(root, 1, 0)
        self.assertEqual("yes", self.assertVendorOption(ts, "group").text())
        self.assertEqual("true", self.assertVendorOption(ts, "labelAllGroup").text())

    def testLabelingPolygonFree(self):
        layer = QgsVectorLayer("Polygon", "addfeat", "memory")
        self.loadStyleWithCustomProperties(layer, "polygonLabel")
        settings = layer.labeling().settings()
        settings.placement = QgsPalLayerSettings.Free
        layer.setLabeling(QgsVectorLayerSimpleLabeling(settings))

        dom, root = self.layerToSld(layer)
        # print("Polygon label with 'Free' placement  " + dom.toString())

        ts = self.getTextSymbolizer(root, 1, 0)
        pointPlacement = self.assertPointPlacement(ts)
        self.assertIsNone(self.assertElement(ts, "se:Displacement", 0, True))
        self.assertStaticAnchorPoint(pointPlacement, 0.5, 0.5)

    def testLabelingPolygonPerimeterCurved(self):
        layer = QgsVectorLayer("Polygon", "addfeat", "memory")
        self.loadStyleWithCustomProperties(layer, "polygonLabel")
        self.updateLinePlacementProperties(layer, QgsPalLayerSettings.PerimeterCurved, 2, 50, 30, -40)

        dom, root = self.layerToSld(layer)
        # print("Polygon Label with curved perimeter line placement  " + dom.toString())

        ts = self.getTextSymbolizer(root, 1, 0)
        linePlacement = self.assertLinePlacement(ts)
        generalize = self.assertElement(linePlacement, 'se:GeneralizeLine', 0)
        self.assertEqual("true", generalize.text())
        offset = self.assertElement(linePlacement, 'se:PerpendicularOffset', 0)
        self.assertEqual("7", offset.text())
        repeat = self.assertElement(linePlacement, 'se:Repeat', 0)
        self.assertEqual("true", repeat.text())
        gap = self.assertElement(linePlacement, 'se:Gap', 0)
        self.assertEqual("179", gap.text())
        self.assertEqual("179", self.assertVendorOption(ts, "repeat").text())
        self.assertEqual("true", self.assertVendorOption(ts, "followLine").text())
        self.assertEqual("30", self.assertVendorOption(ts, "maxAngleDelta").text())

    def testLabelScaleDependencies(self):
        layer = QgsVectorLayer("Polygon", "addfeat", "memory")
        self.loadStyleWithCustomProperties(layer, "polygonLabel")
        settings = layer.labeling().settings()
        settings.scaleVisibility = True
        # Careful: min scale -> large scale denomin
        settings.minimumScale = 10000000
        settings.maximumScale = 1000000
        layer.setLabeling(QgsVectorLayerSimpleLabeling(settings))

        dom, root = self.layerToSld(layer)
        # print("Labeling with scale dependencies  " + dom.toString())
        self.assertScaleDenominator(root, "1000000", "10000000", 1)

    def testLabelShowAll(self):
        layer = QgsVectorLayer("Polygon", "addfeat", "memory")
        self.loadStyleWithCustomProperties(layer, "polygonLabel")
        settings = layer.labeling().settings()
        settings.displayAll = True
        layer.setLabeling(QgsVectorLayerSimpleLabeling(settings))

        dom, root = self.layerToSld(layer)
        # print("Labeling, showing all labels  " + dom.toString())

        ts = self.getTextSymbolizer(root, 1, 0)
        self.assertVendorOption(ts, "conflictResolution", "false")

    def testLabelUpsideDown(self):
        layer = QgsVectorLayer("Polygon", "addfeat", "memory")
        self.loadStyleWithCustomProperties(layer, "polygonLabel")
        settings = layer.labeling().settings()
        settings.upsidedownLabels = QgsPalLayerSettings.ShowAll
        layer.setLabeling(QgsVectorLayerSimpleLabeling(settings))

        dom, root = self.layerToSld(layer)
        # print("Labeling, showing upside down labels on lines  " + dom.toString())

        ts = self.getTextSymbolizer(root, 1, 0)
        self.assertVendorOption(ts, "forceLeftToRight", "false")

    def testLabelBackgroundSquareResize(self):
        self.assertLabelBackground(QgsTextBackgroundSettings.ShapeSquare, 'square',
                                   QgsTextBackgroundSettings.SizeBuffer, 'proportional')

    def testLabelBackgroundRectangleResize(self):
        self.assertLabelBackground(QgsTextBackgroundSettings.ShapeRectangle, 'square',
                                   QgsTextBackgroundSettings.SizeBuffer, 'stretch')

    def testLabelBackgroundCircleResize(self):
        self.assertLabelBackground(QgsTextBackgroundSettings.ShapeCircle, 'circle',
                                   QgsTextBackgroundSettings.SizeBuffer, 'proportional')

    def testLabelBackgroundEllipseResize(self):
        self.assertLabelBackground(QgsTextBackgroundSettings.ShapeEllipse, 'circle',
                                   QgsTextBackgroundSettings.SizeBuffer, 'stretch')

    def testLabelBackgroundSquareAbsolute(self):
        self.assertLabelBackground(QgsTextBackgroundSettings.ShapeSquare, 'square',
                                   QgsTextBackgroundSettings.SizeFixed, None)

    def testLabelBackgroundRectangleAbsolute(self):
        self.assertLabelBackground(QgsTextBackgroundSettings.ShapeRectangle, 'square',
                                   QgsTextBackgroundSettings.SizeFixed, None)

    def testLabelBackgroundCircleAbsolute(self):
        self.assertLabelBackground(QgsTextBackgroundSettings.ShapeCircle, 'circle',
                                   QgsTextBackgroundSettings.SizeFixed, None)

    def testLabelBackgroundEllipseAbsolute(self):
        self.assertLabelBackground(QgsTextBackgroundSettings.ShapeEllipse, 'circle',
                                   QgsTextBackgroundSettings.SizeFixed, None)

    def assertLabelBackground(self, backgroundType, expectedMarkName, sizeType, expectedResize):
        layer = QgsVectorLayer("Polygon", "addfeat", "memory")
        self.loadStyleWithCustomProperties(layer, "polygonLabel")
        settings = layer.labeling().settings()
        background = QgsTextBackgroundSettings()
        background.setEnabled(True)
        background.setType(backgroundType)
        background.setFillColor(QColor('yellow'))
        background.setStrokeColor(QColor('black'))
        background.setStrokeWidth(2)
        background.setSize(QSizeF(10, 10))
        background.setSizeType(sizeType)
        format = settings.format()
        format.setBackground(background)
        settings.setFormat(format)
        layer.setLabeling(QgsVectorLayerSimpleLabeling(settings))

        dom, root = self.layerToSld(layer)
        # print("Labeling, with background type " + str(backgroundType) + " and size type " + str(sizeType) + ": " + dom.toString())

        ts = self.getTextSymbolizer(root, 1, 0)
        graphic = self.assertElement(ts, "se:Graphic", 0)
        self.assertEqual("36", self.assertElement(graphic, 'se:Size', 0).text())
        self.assertWellKnownMark(graphic, 0, expectedMarkName, '#ffff00', '#000000', 7)
        if expectedResize is None:
            self.assertIsNone(expectedResize, self.assertVendorOption(ts, 'graphic-resize', True))
        else:
            self.assertEqual(expectedResize, self.assertVendorOption(ts, 'graphic-resize').text())
        if sizeType == 0:
            # check extra padding for proportional ellipse
            if backgroundType == QgsTextBackgroundSettings.ShapeEllipse:
                self.assertEqual("42.5 49", self.assertVendorOption(ts, 'graphic-margin').text())
            else:
                self.assertEqual("36 36", self.assertVendorOption(ts, 'graphic-margin').text())
        else:
            self.assertIsNone(self.assertVendorOption(ts, 'graphic-margin', True))

    def testRuleBasedLabels(self):
        layer = QgsVectorLayer("Point", "addfeat", "memory")
        self.loadStyleWithCustomProperties(layer, "ruleLabel")

        dom, root = self.layerToSld(layer)
        # print("Rule based labeling: " + dom.toString())

        # three rules, one with the point symbol, one with the first rule based label,
        # one with the second rule based label
        rule1 = self.getRule(root, 0)
        self.assertElement(rule1, 'se:PointSymbolizer', 0)

        rule2 = self.getRule(root, 1)
        self.assertScaleDenominator(root, '100000', '10000000', 1)
        tsRule2 = self.assertElement(rule2, 'se:TextSymbolizer', 0)
        gt = rule2.elementsByTagName("Filter").item(0).firstChild()
        self.assertEqual("ogc:PropertyIsGreaterThan", gt.nodeName())
        gtProperty = gt.toElement().firstChild()
        self.assertEqual("ogc:PropertyName", gtProperty.nodeName())
        self.assertEqual("POP_MAX", gtProperty.toElement().text())
        gtValue = gt.childNodes().item(1)
        self.assertEqual("1000000", gtValue.toElement().text())

        rule3 = self.getRule(root, 2)
        tsRule3 = self.assertElement(rule3, 'se:TextSymbolizer', 0)
        lt = rule3.elementsByTagName("Filter").item(0).firstChild()
        self.assertEqual("ogc:PropertyIsLessThan", lt.nodeName())
        ltProperty = lt.toElement().firstChild()
        self.assertEqual("ogc:PropertyName", ltProperty.nodeName())
        self.assertEqual("POP_MAX", ltProperty.toElement().text())
        ltValue = gt.childNodes().item(1)
        self.assertEqual("1000000", gtValue.toElement().text())

        # check that adding a rule without settings does not segfault
        xml1 = dom.toString()
        layer.labeling().rootRule().appendChild(QgsRuleBasedLabeling.Rule(None))
        dom, root = self.layerToSld(layer)
        xml2 = dom.toString()
        self.assertEqual(xml1, xml2)

    def updateLinePlacementProperties(self, layer, linePlacement, distance, repeat, maxAngleInternal=25,
                                      maxAngleExternal=-25):
        settings = layer.labeling().settings()
        settings.placement = linePlacement
        settings.dist = distance
        settings.repeatDistance = repeat
        settings.maxCurvedCharAngleIn = maxAngleInternal
        settings.maxCurvedCharAngleOut = maxAngleExternal
        layer.setLabeling(QgsVectorLayerSimpleLabeling(settings))

    def assertPointPlacementDistance(self, placement):
        layer = QgsVectorLayer("Point", "addfeat", "memory")
        self.loadStyleWithCustomProperties(layer, "simpleLabel")

        settings = layer.labeling().settings()
        settings.placement = placement
        settings.xOffset = 0
        settings.yOffset = 0
        settings.dist = 2
        layer.setLabeling(QgsVectorLayerSimpleLabeling(settings))

        dom, root = self.layerToSld(layer)
        # print("Label with around point placement  " + dom.toString())
        ts = self.getTextSymbolizer(root, 1, 0)
        pointPlacement = self.assertPointPlacement(ts)
        self.assertStaticAnchorPoint(pointPlacement, 0, 0.5)
        self.assertStaticDisplacement(pointPlacement, 4.95, 4.95)

    def assertLabelQuadrant(self, quadrant, label, ax, ay):
        layer = QgsVectorLayer("Point", "addfeat", "memory")
        self.loadStyleWithCustomProperties(layer, "simpleLabel")

        settings = layer.labeling().settings()
        settings.placement = QgsPalLayerSettings.OverPoint
        settings.xOffset = 0
        settings.yOffset = 0
        settings.quadOffset = quadrant
        settings.angleOffset = 0
        layer.setLabeling(QgsVectorLayerSimpleLabeling(settings))

        dom, root = self.layerToSld(layer)
        # print("Label with " + label  + " placement  " + dom.toString())
        self.assertStaticAnchorPoint(root, ax, ay)

    def setLabelBufferSettings(self, layer, buffer):
        settings = layer.labeling().settings()
        format = settings.format()
        format.setBuffer(buffer)
        settings.setFormat(format)
        layer.setLabeling(QgsVectorLayerSimpleLabeling(settings))

    def updateLayerLabelingFontStyle(self, layer, bold, italic, underline=False, strikeout=False):
        settings = layer.labeling().settings()
        format = settings.format()
        font = format.font()
        font.setBold(bold)
        font.setItalic(italic)
        font.setUnderline(underline)
        font.setStrikeOut(strikeout)
        format.setFont(font)
        settings.setFormat(format)
        layer.setLabeling(QgsVectorLayerSimpleLabeling(settings))

    def updateLayerLabelingUnit(self, layer, unit):
        settings = layer.labeling().settings()
        format = settings.format()
        format.setSizeUnit(unit)
        settings.setFormat(format)
        layer.setLabeling(QgsVectorLayerSimpleLabeling(settings))

    def loadStyleWithCustomProperties(self, layer, qmlFileName):
        # load the style, only vector symbology
        path = QDir.toNativeSeparators('%s/symbol_layer/%s.qml' % (unitTestDataPath(), qmlFileName))

        # labeling is in custom properties, they need to be loaded separately
        status = layer.loadNamedStyle(path)
        doc = QDomDocument()
        file = QFile(path)
        file.open(QIODevice.ReadOnly)
        doc.setContent(file, True)
        file.close()
        flag = layer.readCustomProperties(doc.documentElement())

    def assertPointPlacement(self, textSymbolizer):
        labelPlacement = self.assertElement(textSymbolizer, 'se:LabelPlacement', 0)
        self.assertIsNone(self.assertElement(labelPlacement, 'se:LinePlacement', 0, True))
        pointPlacement = self.assertElement(labelPlacement, 'se:PointPlacement', 0)
        return pointPlacement

    def assertLinePlacement(self, textSymbolizer):
        labelPlacement = self.assertElement(textSymbolizer, 'se:LabelPlacement', 0)
        self.assertIsNone(self.assertElement(labelPlacement, 'se:PointPlacement', 0, True))
        linePlacement = self.assertElement(labelPlacement, 'se:LinePlacement', 0)
        return linePlacement

    def assertElement(self, container, elementName, index, allowMissing=False):
        list = container.elementsByTagName(elementName)
        if list.size() <= index:
            if allowMissing:
                return None
            else:
                self.fail('Expected to find at least ' + str(
                    index + 1) + ' ' + elementName + ' in ' + container.nodeName() + ' but found ' + str(list.size()))

        node = list.item(index)
        self.assertTrue(node.isElement(), 'Found node but it''s not an element')
        return node.toElement()

    def getRule(self, root, ruleIndex):
        rule = self.assertElement(root, 'se:Rule', ruleIndex)
        return rule

    def getTextSymbolizer(self, root, ruleIndex, textSymbolizerIndex):
        rule = self.assertElement(root, 'se:Rule', ruleIndex)
        textSymbolizer = self.assertElement(rule, 'se:TextSymbolizer', textSymbolizerIndex)
        return textSymbolizer

    def assertPropertyName(self, root, containerProperty, expectedAttributeName):
        container = root.elementsByTagName(containerProperty).item(0).toElement()
        property = container.elementsByTagName("ogc:PropertyName").item(0).toElement()
        self.assertEqual(expectedAttributeName, property.text())

    def assertSvgParameter(self, container, expectedName, allowMissing=False):
        list = container.elementsByTagName("se:SvgParameter")
        for i in range(0, list.size()):
            item = list.item(i)
            if item.isElement and item.isElement() and item.toElement().attribute('name') == expectedName:
                return item.toElement()
        if allowMissing:
            return None
        else:
            self.fail('Could not find a se:SvgParameter named ' + expectedName + ' in ' + container.nodeName())

    def assertVendorOption(self, container, expectedName, allowMissing=False):
        list = container.elementsByTagName("se:VendorOption")
        for i in range(0, list.size()):
            item = list.item(i)
            if item.isElement and item.isElement() and item.toElement().attribute('name') == expectedName:
                return item.toElement()
        if allowMissing:
            return None
        else:
            self.fail('Could not find a se:VendorOption named ' + expectedName + ' in ' + container.nodeName())

    def testRuleBaseEmptyFilter(self):
        layer = QgsVectorLayer("Point", "addfeat", "memory")

        mFilePath = QDir.toNativeSeparators('%s/symbol_layer/%s.qml' % (unitTestDataPath(), "categorizedEmptyValue"))
        status = layer.loadNamedStyle(mFilePath)  # NOQA

        dom, root = self.layerToSld(layer)
        # print("Rule based, with last rule checking against empty value:" + dom.toString())

        # get the third rule
        rule = root.elementsByTagName('se:Rule').item(2).toElement()
        filter = rule.elementsByTagName('Filter').item(0).toElement()
        filter = filter.firstChild().toElement()
        self.assertEqual("ogc:Or", filter.nodeName())
        self.assertEqual(1, filter.elementsByTagName('ogc:PropertyIsEqualTo').size())
        self.assertEqual(1, filter.elementsByTagName('ogc:PropertyIsNull').size())

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

    def assertExternalGraphic(self, root, index, expectedLink, expectedFormat):
        graphic = root.elementsByTagName('se:ExternalGraphic').item(index)
        onlineResource = graphic.firstChildElement('se:OnlineResource')
        self.assertEqual(expectedLink, onlineResource.attribute('xlink:href'))
        format = graphic.firstChildElement('se:Format')
        self.assertEqual(expectedFormat, format.firstChild().nodeValue())

    def assertStaticPerpendicularOffset(self, root, expectedValue):
        offset = root.elementsByTagName('se:PerpendicularOffset').item(0)
        self.assertEqual(expectedValue, offset.firstChild().nodeValue())

    def assertWellKnownMark(self, root, index, expectedName, expectedFill, expectedStroke, expectedStrokeWidth):
        mark = root.elementsByTagName('se:Mark').item(index)
        wkn = mark.firstChildElement('se:WellKnownName')
        self.assertEqual(expectedName, wkn.text())

        fill = mark.firstChildElement('se:Fill')
        if expectedFill is None:
            self.assertTrue(fill.isNull())
        else:
            parameter = fill.firstChildElement('se:SvgParameter')
            self.assertEqual('fill', parameter.attribute('name'))
            self.assertEqual(expectedFill, parameter.text())

        stroke = mark.firstChildElement('se:Stroke')
        if expectedStroke is None:
            self.assertTrue(stroke.isNull())
        else:
            parameter = stroke.firstChildElement('se:SvgParameter')
            self.assertEqual('stroke', parameter.attribute('name'))
            self.assertEqual(expectedStroke, parameter.text())
            parameter = parameter.nextSiblingElement('se:SvgParameter')
            self.assertEqual('stroke-width', parameter.attribute('name'))
            self.assertEqual(str(expectedStrokeWidth), parameter.text())

    def assertStaticRotation(self, root, expectedValue, index=0):
        # Check the rotation element is a literal, not a
        rotation = root.elementsByTagName('se:Rotation').item(index)
        literal = rotation.firstChild()
        self.assertEqual("ogc:Literal", literal.nodeName())
        self.assertEqual(expectedValue, literal.firstChild().nodeValue())

    def assertStaticDisplacement(self, root, expectedAnchorX, expectedAnchorY):
        displacement = root.elementsByTagName('se:Displacement').item(0)
        self.assertIsNotNone(displacement)
        dx = displacement.firstChild()
        self.assertIsNotNone(dx)
        self.assertEqual("se:DisplacementX", dx.nodeName())
        self.assertSldNumber(expectedAnchorX, dx.firstChild().nodeValue())
        dy = displacement.lastChild()
        self.assertIsNotNone(dy)
        self.assertEqual("se:DisplacementY", dy.nodeName())
        self.assertSldNumber(expectedAnchorY, dy.firstChild().nodeValue())

    def assertStaticAnchorPoint(self, root, expectedDispX, expectedDispY):
        anchor = root.elementsByTagName('se:AnchorPoint').item(0)
        self.assertIsNotNone(anchor)
        ax = anchor.firstChild()
        self.assertIsNotNone(ax)
        self.assertEqual("se:AnchorPointX", ax.nodeName())
        self.assertSldNumber(expectedDispX, ax.firstChild().nodeValue())
        ay = anchor.lastChild()
        self.assertIsNotNone(ay)
        self.assertEqual("se:AnchorPointY", ay.nodeName())
        self.assertSldNumber(expectedDispY, ay.firstChild().nodeValue())

    def assertSldNumber(self, expected, stringValue):
        value = float(stringValue)
        self.assertFloatEquals(expected, value, 0.01)

    def assertFloatEquals(self, expected, actual, tol):
        self.assertLess(abs(expected - actual), tol, 'Expected %d but was %d' % (expected, actual))

    def assertStrokeWidth(self, root, svgParameterIdx, expectedWidth):
        strokeWidth = root.elementsByTagName(
            'se:SvgParameter').item(svgParameterIdx)
        svgParameterName = strokeWidth.attributes().namedItem('name')
        self.assertEqual("stroke-width", svgParameterName.nodeValue())
        self.assertSldNumber(
            expectedWidth, strokeWidth.firstChild().nodeValue())

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
