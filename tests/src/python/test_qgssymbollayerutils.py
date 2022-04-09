# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsSymbolLayerUtils.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '2016-09'
__copyright__ = 'Copyright 2016, The QGIS Project'

import qgis  # NOQA
from qgis.PyQt.QtCore import (
    QSizeF,
    QPointF,
    QMimeData,
    QDir,
    QSize,
    Qt
)
from qgis.PyQt.QtGui import (
    QColor,
    QPolygonF,
    QImage
)
from qgis.core import (
    Qgis,
    QgsSymbolLayerUtils,
    QgsMarkerSymbol,
    QgsArrowSymbolLayer,
    QgsUnitTypes,
    QgsRenderChecker,
    QgsGradientColorRamp,
    QgsShapeburstFillSymbolLayer,
    QgsMarkerLineSymbolLayer,
    QgsSimpleLineSymbolLayer,
    QgsSimpleFillSymbolLayer,
    QgsSymbolLayer,
    QgsProperty,
    QgsMapUnitScale,
    Qgis,
    QgsSingleSymbolRenderer,
    QgsAnimatedMarkerSymbolLayer,
    QgsSimpleMarkerSymbolLayer
)
from qgis.testing import unittest, start_app

start_app()


class PyQgsSymbolLayerUtils(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.report = "<h1>Python QgsPointCloudRgbRenderer Tests</h1>\n"

    @classmethod
    def tearDownClass(cls):
        report_file_path = "%s/qgistest.html" % QDir.tempPath()
        with open(report_file_path, 'a') as report_file:
            report_file.write(cls.report)

    def testEncodeDecodeSize(self):
        s = QSizeF()
        string = QgsSymbolLayerUtils.encodeSize(s)
        s2 = QgsSymbolLayerUtils.decodeSize(string)
        self.assertEqual(s2, s)
        s = QSizeF(1.5, 2.5)
        string = QgsSymbolLayerUtils.encodeSize(s)
        s2 = QgsSymbolLayerUtils.decodeSize(string)
        self.assertEqual(s2, s)

        # bad string
        s2 = QgsSymbolLayerUtils.decodeSize('')
        self.assertEqual(s2, QSizeF(0, 0))

    def testToSize(self):
        s2, ok = QgsSymbolLayerUtils.toSize(None)
        self.assertFalse(ok)

        s2, ok = QgsSymbolLayerUtils.toSize(4)
        self.assertFalse(ok)

        s2, ok = QgsSymbolLayerUtils.toSize('4')
        self.assertFalse(ok)

        # arrays
        s2, ok = QgsSymbolLayerUtils.toSize([4])
        self.assertFalse(ok)

        s2, ok = QgsSymbolLayerUtils.toSize([])
        self.assertFalse(ok)

        s2, ok = QgsSymbolLayerUtils.toSize([4, 5, 6])
        self.assertFalse(ok)

        s2, ok = QgsSymbolLayerUtils.toSize([4, 5])
        self.assertTrue(ok)
        self.assertEqual(s2, QSizeF(4, 5))

        s2, ok = QgsSymbolLayerUtils.toSize(['4', '5'])
        self.assertTrue(ok)
        self.assertEqual(s2, QSizeF(4, 5))

        # string values
        s = QSizeF()
        string = QgsSymbolLayerUtils.encodeSize(s)
        s2, ok = QgsSymbolLayerUtils.toSize(string)
        self.assertTrue(ok)
        self.assertEqual(s2, s)
        s = QSizeF(1.5, 2.5)
        string = QgsSymbolLayerUtils.encodeSize(s)
        s2, ok = QgsSymbolLayerUtils.toSize(string)
        self.assertTrue(ok)
        self.assertEqual(s2, s)

        # bad string
        s2, ok = QgsSymbolLayerUtils.toSize('')
        self.assertFalse(ok)
        self.assertEqual(s2, QSizeF())

    def testEncodeDecodePoint(self):
        s = QPointF()
        string = QgsSymbolLayerUtils.encodePoint(s)
        s2 = QgsSymbolLayerUtils.decodePoint(string)
        self.assertEqual(s2, s)
        s = QPointF(1.5, 2.5)
        string = QgsSymbolLayerUtils.encodePoint(s)
        s2 = QgsSymbolLayerUtils.decodePoint(string)
        self.assertEqual(s2, s)

        # bad string
        s2 = QgsSymbolLayerUtils.decodePoint('')
        self.assertEqual(s2, QPointF())

    def testEncodeDecodeCoordinateReference(self):
        items = {'feature': Qgis.SymbolCoordinateReference.Feature, 'viewport': Qgis.SymbolCoordinateReference.Viewport}
        for item in items.keys():
            encoded = QgsSymbolLayerUtils.encodeCoordinateReference(items[item])
            self.assertEqual(item, encoded)
            decoded, ok = QgsSymbolLayerUtils.decodeCoordinateReference(encoded)
            self.assertEqual(items[item], decoded)

    def testToPoint(self):
        s2, ok = QgsSymbolLayerUtils.toPoint(None)
        self.assertFalse(ok)

        s2, ok = QgsSymbolLayerUtils.toPoint(4)
        self.assertFalse(ok)

        s2, ok = QgsSymbolLayerUtils.toPoint('4')
        self.assertFalse(ok)

        # arrays
        s2, ok = QgsSymbolLayerUtils.toPoint([4])
        self.assertFalse(ok)

        s2, ok = QgsSymbolLayerUtils.toPoint([])
        self.assertFalse(ok)

        s2, ok = QgsSymbolLayerUtils.toPoint([4, 5, 6])
        self.assertFalse(ok)

        s2, ok = QgsSymbolLayerUtils.toPoint([4, 5])
        self.assertTrue(ok)
        self.assertEqual(s2, QPointF(4, 5))

        s2, ok = QgsSymbolLayerUtils.toPoint(['4', '5'])
        self.assertTrue(ok)
        self.assertEqual(s2, QPointF(4, 5))

        # string values
        s = QPointF()
        string = QgsSymbolLayerUtils.encodePoint(s)
        s2, ok = QgsSymbolLayerUtils.toPoint(string)
        self.assertTrue(ok)
        self.assertEqual(s2, s)
        s = QPointF(1.5, 2.5)
        string = QgsSymbolLayerUtils.encodePoint(s)
        s2, ok = QgsSymbolLayerUtils.toPoint(string)
        self.assertTrue(ok)
        self.assertEqual(s2, s)

        # bad string
        s2, ok = QgsSymbolLayerUtils.toPoint('')
        self.assertFalse(ok)
        self.assertEqual(s2, QPointF())

    def testDecodeArrowHeadType(self):
        type, ok = QgsSymbolLayerUtils.decodeArrowHeadType(0)
        self.assertTrue(ok)
        self.assertEqual(type, QgsArrowSymbolLayer.HeadSingle)
        type, ok = QgsSymbolLayerUtils.decodeArrowHeadType('single')
        self.assertTrue(ok)
        self.assertEqual(type, QgsArrowSymbolLayer.HeadSingle)
        type, ok = QgsSymbolLayerUtils.decodeArrowHeadType('   SINGLE   ')
        self.assertTrue(ok)
        self.assertEqual(type, QgsArrowSymbolLayer.HeadSingle)
        type, ok = QgsSymbolLayerUtils.decodeArrowHeadType(1)
        self.assertTrue(ok)
        self.assertEqual(type, QgsArrowSymbolLayer.HeadReversed)
        type, ok = QgsSymbolLayerUtils.decodeArrowHeadType('reversed')
        self.assertTrue(ok)
        self.assertEqual(type, QgsArrowSymbolLayer.HeadReversed)
        type, ok = QgsSymbolLayerUtils.decodeArrowHeadType(2)
        self.assertTrue(ok)
        self.assertEqual(type, QgsArrowSymbolLayer.HeadDouble)
        type, ok = QgsSymbolLayerUtils.decodeArrowHeadType('double')
        self.assertTrue(ok)
        self.assertEqual(type, QgsArrowSymbolLayer.HeadDouble)
        type, ok = QgsSymbolLayerUtils.decodeArrowHeadType('xxxxx')
        self.assertFalse(ok)
        type, ok = QgsSymbolLayerUtils.decodeArrowHeadType(34)
        self.assertFalse(ok)

    def testDecodeArrowType(self):
        type, ok = QgsSymbolLayerUtils.decodeArrowType(0)
        self.assertTrue(ok)
        self.assertEqual(type, QgsArrowSymbolLayer.ArrowPlain)
        type, ok = QgsSymbolLayerUtils.decodeArrowType('plain')
        self.assertTrue(ok)
        self.assertEqual(type, QgsArrowSymbolLayer.ArrowPlain)
        type, ok = QgsSymbolLayerUtils.decodeArrowType('   PLAIN   ')
        self.assertTrue(ok)
        self.assertEqual(type, QgsArrowSymbolLayer.ArrowPlain)
        type, ok = QgsSymbolLayerUtils.decodeArrowType(1)
        self.assertTrue(ok)
        self.assertEqual(type, QgsArrowSymbolLayer.ArrowLeftHalf)
        type, ok = QgsSymbolLayerUtils.decodeArrowType('lefthalf')
        self.assertTrue(ok)
        self.assertEqual(type, QgsArrowSymbolLayer.ArrowLeftHalf)
        type, ok = QgsSymbolLayerUtils.decodeArrowType(2)
        self.assertTrue(ok)
        self.assertEqual(type, QgsArrowSymbolLayer.ArrowRightHalf)
        type, ok = QgsSymbolLayerUtils.decodeArrowType('righthalf')
        self.assertTrue(ok)
        self.assertEqual(type, QgsArrowSymbolLayer.ArrowRightHalf)
        type, ok = QgsSymbolLayerUtils.decodeArrowType('xxxxx')
        self.assertFalse(ok)
        type, ok = QgsSymbolLayerUtils.decodeArrowType(34)
        self.assertFalse(ok)

    def test_decode_marker_clip(self):
        """
        Test decode marker clip
        """
        self.assertEqual(QgsSymbolLayerUtils.decodeMarkerClipMode(''), (Qgis.MarkerClipMode.Shape, False))
        self.assertEqual(QgsSymbolLayerUtils.decodeMarkerClipMode('xxx'), (Qgis.MarkerClipMode.Shape, False))
        self.assertEqual(QgsSymbolLayerUtils.decodeMarkerClipMode(' no   '), (Qgis.MarkerClipMode.NoClipping, True))
        self.assertEqual(QgsSymbolLayerUtils.decodeMarkerClipMode(' NO   '), (Qgis.MarkerClipMode.NoClipping, True))
        self.assertEqual(QgsSymbolLayerUtils.decodeMarkerClipMode(' shape   '), (Qgis.MarkerClipMode.Shape, True))
        self.assertEqual(QgsSymbolLayerUtils.decodeMarkerClipMode(' Shape   '), (Qgis.MarkerClipMode.Shape, True))
        self.assertEqual(QgsSymbolLayerUtils.decodeMarkerClipMode(' centroid_within   '), (Qgis.MarkerClipMode.CentroidWithin, True))
        self.assertEqual(QgsSymbolLayerUtils.decodeMarkerClipMode(' Centroid_Within   '),
                         (Qgis.MarkerClipMode.CentroidWithin, True))
        self.assertEqual(QgsSymbolLayerUtils.decodeMarkerClipMode(' completely_within   '),
                         (Qgis.MarkerClipMode.CompletelyWithin, True))
        self.assertEqual(QgsSymbolLayerUtils.decodeMarkerClipMode(' Completely_Within   '),
                         (Qgis.MarkerClipMode.CompletelyWithin, True))

    def test_encode_marker_clip(self):
        """
        Test encode marker clip
        """
        self.assertEqual(QgsSymbolLayerUtils.encodeMarkerClipMode(Qgis.MarkerClipMode.Shape), 'shape')
        self.assertEqual(QgsSymbolLayerUtils.encodeMarkerClipMode(Qgis.MarkerClipMode.NoClipping), 'no')
        self.assertEqual(QgsSymbolLayerUtils.encodeMarkerClipMode(Qgis.MarkerClipMode.CentroidWithin), 'centroid_within')
        self.assertEqual(QgsSymbolLayerUtils.encodeMarkerClipMode(Qgis.MarkerClipMode.CompletelyWithin), 'completely_within')

    def test_decode_line_clip(self):
        """
        Test decode line clip
        """
        self.assertEqual(QgsSymbolLayerUtils.decodeLineClipMode(''), (Qgis.LineClipMode.ClipPainterOnly, False))
        self.assertEqual(QgsSymbolLayerUtils.decodeLineClipMode('xxx'), (Qgis.LineClipMode.ClipPainterOnly, False))
        self.assertEqual(QgsSymbolLayerUtils.decodeLineClipMode(' no   '), (Qgis.LineClipMode.NoClipping, True))
        self.assertEqual(QgsSymbolLayerUtils.decodeLineClipMode(' NO   '), (Qgis.LineClipMode.NoClipping, True))
        self.assertEqual(QgsSymbolLayerUtils.decodeLineClipMode(' during_render   '), (Qgis.LineClipMode.ClipPainterOnly, True))
        self.assertEqual(QgsSymbolLayerUtils.decodeLineClipMode(' DURING_Render   '), (Qgis.LineClipMode.ClipPainterOnly, True))
        self.assertEqual(QgsSymbolLayerUtils.decodeLineClipMode(' before_render   '), (Qgis.LineClipMode.ClipToIntersection, True))
        self.assertEqual(QgsSymbolLayerUtils.decodeLineClipMode(' BEFORE_REnder   '),
                         (Qgis.LineClipMode.ClipToIntersection, True))

    def test_encode_line_clip(self):
        """
        Test encode line clip
        """
        self.assertEqual(QgsSymbolLayerUtils.encodeLineClipMode(Qgis.LineClipMode.ClipPainterOnly), 'during_render')
        self.assertEqual(QgsSymbolLayerUtils.encodeLineClipMode(Qgis.LineClipMode.NoClipping), 'no')
        self.assertEqual(QgsSymbolLayerUtils.encodeLineClipMode(Qgis.LineClipMode.ClipToIntersection), 'before_render')

    def testSymbolToFromMimeData(self):
        """
        Test converting symbols to and from mime data
        """
        symbol = QgsMarkerSymbol.createSimple({})
        symbol.setColor(QColor(255, 0, 255))
        self.assertFalse(QgsSymbolLayerUtils.symbolFromMimeData(None))
        self.assertFalse(QgsSymbolLayerUtils.symbolToMimeData(None))
        mime = QgsSymbolLayerUtils.symbolToMimeData(symbol)
        self.assertTrue(mime is not None)
        symbol2 = QgsSymbolLayerUtils.symbolFromMimeData(mime)
        self.assertTrue(symbol2 is not None)
        self.assertEqual(symbol2.color().name(), symbol.color().name())

    def testEncodeSldUom(self):
        """
        Test Encodes a SLD unit of measure string to a render unit
        """

        # millimeter
        encode = None
        encode = QgsSymbolLayerUtils.encodeSldUom(QgsUnitTypes.RenderMillimeters)
        self.assertTupleEqual(encode, ('', 3.571428571428571))

        # mapunits
        encode = None
        encode = QgsSymbolLayerUtils.encodeSldUom(QgsUnitTypes.RenderMapUnits)
        self.assertTupleEqual(encode, ('http://www.opengeospatial.org/se/units/metre', 0.001))

    def testDecodeSldUom(self):
        """
        Test Decodes a SLD unit of measure string to a render unit
        """

        # meter
        decode = None
        decode = QgsSymbolLayerUtils.decodeSldUom("http://www.opengeospatial.org/se/units/metre")
        self.assertEqual(decode, (QgsUnitTypes.RenderMapUnits, 1000.0))

        # foot
        decode = None
        decode = QgsSymbolLayerUtils.decodeSldUom("http://www.opengeospatial.org/se/units/foot")
        self.assertEqual(decode, (QgsUnitTypes.RenderMapUnits, 304.8))

        # pixel
        decode = None
        decode = QgsSymbolLayerUtils.decodeSldUom("http://www.opengeospatial.org/se/units/pixel")
        self.assertEqual(decode, (QgsUnitTypes.RenderPixels, 1.0))

    def testPolylineLength(self):
        """
        Test QgsSymbolLayerUtils.polylineLength
        """
        self.assertEqual(QgsSymbolLayerUtils.polylineLength(QPolygonF()), 0.0)
        self.assertEqual(
            QgsSymbolLayerUtils.polylineLength(QPolygonF([QPointF(11, 12), QPointF(11, 12)])), 0.0)
        self.assertEqual(
            QgsSymbolLayerUtils.polylineLength(QPolygonF([QPointF(11, 2), QPointF(11, 12), QPointF(11, 12)])), 10.0)
        self.assertEqual(QgsSymbolLayerUtils.polylineLength(QPolygonF([QPointF(11, 2), QPointF(11, 12), QPointF(111, 12)])), 110.0)

    def testPolylineSubstring(self):
        res = QgsSymbolLayerUtils.polylineSubstring(QPolygonF(), 1, 2)  # no crash
        self.assertFalse(res)

        res = QgsSymbolLayerUtils.polylineSubstring(QPolygonF(), -1, 2)  # no crash
        self.assertFalse(res)

        res = QgsSymbolLayerUtils.polylineSubstring(QPolygonF(), 1, -2)  # no crash
        self.assertFalse(res)

        res = QgsSymbolLayerUtils.polylineSubstring(QPolygonF(), -1, -2)  # no crash
        self.assertFalse(res)

        res = QgsSymbolLayerUtils.polylineSubstring(QPolygonF([QPointF(11, 2), QPointF(11, 12), QPointF(111, 12)]), 0,
                                                    -110)
        self.assertEqual([p for p in res], [])

        res = QgsSymbolLayerUtils.polylineSubstring(QPolygonF([QPointF(11, 2), QPointF(11, 12), QPointF(111, 12)]), 0,
                                                    110)
        self.assertEqual([p for p in res], [QPointF(11, 2), QPointF(11, 12), QPointF(111, 12)])

        res = QgsSymbolLayerUtils.polylineSubstring(QPolygonF([QPointF(11, 2), QPointF(11, 12), QPointF(111, 12)]), -1,
                                                    -1000)
        self.assertFalse([p for p in res])

        res = QgsSymbolLayerUtils.polylineSubstring(QPolygonF([QPointF(11, 2), QPointF(11, 12), QPointF(111, 12)]), 1,
                                                    -1000)
        self.assertFalse([p for p in res])

        res = QgsSymbolLayerUtils.polylineSubstring(QPolygonF([QPointF(11, 2), QPointF(11, 12), QPointF(111, 12)]), -1,
                                                    1000)
        self.assertEqual([p for p in res], [QPointF(110.0, 12.0), QPointF(111.0, 12.0)])

        res = QgsSymbolLayerUtils.polylineSubstring(QPolygonF([QPointF(11, 2), QPointF(11, 12), QPointF(111, 12)]),
                                                    100000, -10000)
        self.assertFalse([p for p in res])

        res = QgsSymbolLayerUtils.polylineSubstring(QPolygonF([QPointF(11, 2), QPointF(11, 12), QPointF(111, 12)]), 1,
                                                    -109)
        self.assertEqual([p for p in res], [])

        res = QgsSymbolLayerUtils.polylineSubstring(QPolygonF([QPointF(11, 2), QPointF(11, 12), QPointF(111, 12)]), 1,
                                                    109)
        self.assertEqual([p for p in res], [QPointF(11.0, 3.0), QPointF(11.0, 12.0), QPointF(110.0, 12.0)])

        res = QgsSymbolLayerUtils.polylineSubstring(QPolygonF([QPointF(11, 2), QPointF(11, 12), QPointF(111, 12)]),
                                                    -109, 109)
        self.assertEqual([p for p in res], [QPointF(11.0, 3.0), QPointF(11.0, 12.0), QPointF(110.0, 12.0)])

        res = QgsSymbolLayerUtils.polylineSubstring(QPolygonF([QPointF(11, 2), QPointF(11, 12), QPointF(111, 12)]), 1,
                                                    -1000)
        self.assertEqual([p for p in res], [])

        res = QgsSymbolLayerUtils.polylineSubstring(QPolygonF([QPointF(11, 2), QPointF(11, 12), QPointF(111, 12)]), 1,
                                                    10)
        self.assertEqual([p for p in res], [QPointF(11, 3), QPointF(11, 12)])

        res = QgsSymbolLayerUtils.polylineSubstring(QPolygonF([QPointF(11, 2), QPointF(11, 12), QPointF(111, 12)]), 1,
                                                    0)
        self.assertEqual([p for p in res], [QPointF(11, 3), QPointF(11, 12), QPointF(111, 12)])

        res = QgsSymbolLayerUtils.polylineSubstring(QPolygonF([QPointF(11, 2), QPointF(11, 12), QPointF(111, 12)]), 1,
                                                    -90)
        self.assertEqual([p for p in res], [QPointF(11, 3), QPointF(11, 12), QPointF(21, 12)])

    def testAppendPolyline(self):
        line = QPolygonF([QPointF(11, 2), QPointF(11, 12), QPointF(111, 12)])
        line2 = QPolygonF([QPointF(111, 12), QPointF(111, 12), QPointF(111, 14), QPointF(111, 15)])
        QgsSymbolLayerUtils.appendPolyline(line, line2)
        self.assertEqual([p for p in line],
                         [QPointF(11.0, 2.0), QPointF(11.0, 12.0), QPointF(111.0, 12.0), QPointF(111.0, 14.0),
                          QPointF(111.0, 15.0)])

        line = QPolygonF([QPointF(11, 2), QPointF(11, 12), QPointF(111, 12)])
        line2 = QPolygonF([QPointF(111, 14), QPointF(111, 15)])
        QgsSymbolLayerUtils.appendPolyline(line, line2)
        self.assertEqual([p for p in line],
                         [QPointF(11.0, 2.0), QPointF(11.0, 12.0), QPointF(111.0, 12.0), QPointF(111.0, 14.0),
                          QPointF(111.0, 15.0)])

    def testColorFromMimeData(self):
        data = QMimeData()
        color, has_alpha = QgsSymbolLayerUtils.colorFromMimeData(data)
        self.assertFalse(color.isValid())

        # color data
        data.setColorData(QColor(255, 0, 255))
        color, has_alpha = QgsSymbolLayerUtils.colorFromMimeData(data)
        self.assertTrue(color.isValid())
        self.assertEqual(color.name(), '#ff00ff')
        # should be true regardless of the actual color's opacity -- a QColor object has innate knowledge of the alpha,
        # so our input color HAS an alpha of 255
        self.assertTrue(has_alpha)
        self.assertEqual(color.alpha(), 255)

        data.setColorData(QColor(255, 0, 255, 100))
        color, has_alpha = QgsSymbolLayerUtils.colorFromMimeData(data)
        self.assertTrue(color.isValid())
        self.assertEqual(color.name(), '#ff00ff')
        self.assertEqual(color.alpha(), 100)
        self.assertTrue(has_alpha)

        # text data
        data = QMimeData()
        data.setText('#ff00ff')
        color, has_alpha = QgsSymbolLayerUtils.colorFromMimeData(data)
        self.assertTrue(color.isValid())
        self.assertEqual(color.name(), '#ff00ff')
        # should be False -- no alpha was specified
        self.assertFalse(has_alpha)
        self.assertEqual(color.alpha(), 255)

        data.setText('#ff00ff66')
        color, has_alpha = QgsSymbolLayerUtils.colorFromMimeData(data)
        self.assertTrue(color.isValid())
        self.assertEqual(color.name(), '#ff00ff')
        self.assertTrue(has_alpha)
        self.assertEqual(color.alpha(), 102)

        # "#" is optional
        data.setText('ff00ff66')
        color, has_alpha = QgsSymbolLayerUtils.colorFromMimeData(data)
        self.assertTrue(color.isValid())
        self.assertEqual(color.name(), '#ff00ff')
        self.assertTrue(has_alpha)
        self.assertEqual(color.alpha(), 102)

        data.setText('255,0,255')
        color, has_alpha = QgsSymbolLayerUtils.colorFromMimeData(data)
        self.assertTrue(color.isValid())
        self.assertEqual(color.name(), '#ff00ff')
        self.assertFalse(has_alpha)
        self.assertEqual(color.alpha(), 255)

        data.setText('255,0,255,0.5')
        color, has_alpha = QgsSymbolLayerUtils.colorFromMimeData(data)
        self.assertTrue(color.isValid())
        self.assertEqual(color.name(), '#ff00ff')
        self.assertTrue(has_alpha)
        self.assertEqual(color.alpha(), 128)

        data.setText('rgba(255,0,255,0.5)')
        color, has_alpha = QgsSymbolLayerUtils.colorFromMimeData(data)
        self.assertTrue(color.isValid())
        self.assertEqual(color.name(), '#ff00ff')
        self.assertTrue(has_alpha)
        self.assertEqual(color.alpha(), 128)

        # wrong data type
        data = QMimeData()
        data.setImageData(QImage())
        color, has_alpha = QgsSymbolLayerUtils.colorFromMimeData(data)
        self.assertFalse(color.isValid())

    def testPreviewColorRampHorizontal(self):
        r = QgsGradientColorRamp(QColor(200, 0, 0, 200), QColor(0, 200, 0, 255))

        pix = QgsSymbolLayerUtils.colorRampPreviewPixmap(r, QSize(200, 100))
        img = QImage(pix)
        self.assertTrue(self.imageCheck('color_ramp_horizontal', 'color_ramp_horizontal', img))

    def testPreviewColorRampHorizontalNoCheckboard(self):
        r = QgsGradientColorRamp(QColor(200, 0, 0, 200), QColor(0, 200, 0, 255))

        pix = QgsSymbolLayerUtils.colorRampPreviewPixmap(r, QSize(200, 100), drawTransparentBackground=False)
        img = QImage(pix)
        self.assertTrue(self.imageCheck('color_ramp_no_check', 'color_ramp_no_check', img))

    def testPreviewColorRampHorizontalFlipped(self):
        r = QgsGradientColorRamp(QColor(200, 0, 0, 200), QColor(0, 200, 0, 255))

        pix = QgsSymbolLayerUtils.colorRampPreviewPixmap(r, QSize(200, 100), flipDirection=True)
        img = QImage(pix)
        self.assertTrue(self.imageCheck('color_ramp_horizontal_flipped', 'color_ramp_horizontal_flipped', img))

    def testPreviewColorRampVertical(self):
        r = QgsGradientColorRamp(QColor(200, 0, 0, 200), QColor(0, 200, 0, 255))

        pix = QgsSymbolLayerUtils.colorRampPreviewPixmap(r, QSize(100, 200), direction=Qt.Vertical)
        img = QImage(pix)
        self.assertTrue(self.imageCheck('color_ramp_vertical', 'color_ramp_vertical', img))

    def testPreviewColorRampVerticalFlipped(self):
        r = QgsGradientColorRamp(QColor(200, 0, 0, 200), QColor(0, 200, 0, 255))

        pix = QgsSymbolLayerUtils.colorRampPreviewPixmap(r, QSize(100, 200), direction=Qt.Vertical, flipDirection=True)
        img = QImage(pix)
        self.assertTrue(self.imageCheck('color_ramp_vertical_flipped', 'color_ramp_vertical_flipped', img))

    def testCondenseFillAndOutline(self):
        """
        Test QgsSymbolLayerUtils.condenseFillAndOutline
        """
        self.assertFalse(QgsSymbolLayerUtils.condenseFillAndOutline(None, None))

        # not simple fill or line
        self.assertFalse(QgsSymbolLayerUtils.condenseFillAndOutline(QgsShapeburstFillSymbolLayer(), QgsSimpleLineSymbolLayer()))
        self.assertFalse(QgsSymbolLayerUtils.condenseFillAndOutline(QgsSimpleFillSymbolLayer(), QgsMarkerLineSymbolLayer()))

        # simple fill/line
        fill = QgsSimpleFillSymbolLayer()
        line = QgsSimpleLineSymbolLayer()

        # set incompatible settings on outline
        line.setUseCustomDashPattern(True)
        self.assertFalse(QgsSymbolLayerUtils.condenseFillAndOutline(fill, line))

        line = QgsSimpleLineSymbolLayer()
        line.setDashPatternOffset(1)
        self.assertFalse(QgsSymbolLayerUtils.condenseFillAndOutline(fill, line))

        line = QgsSimpleLineSymbolLayer()
        line.setAlignDashPattern(True)
        self.assertFalse(QgsSymbolLayerUtils.condenseFillAndOutline(fill, line))

        line = QgsSimpleLineSymbolLayer()
        line.setTweakDashPatternOnCorners(True)
        self.assertFalse(QgsSymbolLayerUtils.condenseFillAndOutline(fill, line))

        line = QgsSimpleLineSymbolLayer()
        line.setTrimDistanceStart(1)
        self.assertFalse(QgsSymbolLayerUtils.condenseFillAndOutline(fill, line))

        line = QgsSimpleLineSymbolLayer()
        line.setTrimDistanceEnd(1)
        self.assertFalse(QgsSymbolLayerUtils.condenseFillAndOutline(fill, line))

        line = QgsSimpleLineSymbolLayer()
        line.setDrawInsidePolygon(True)
        self.assertFalse(QgsSymbolLayerUtils.condenseFillAndOutline(fill, line))

        line = QgsSimpleLineSymbolLayer()
        line.setRingFilter(QgsSimpleLineSymbolLayer.ExteriorRingOnly)
        self.assertFalse(QgsSymbolLayerUtils.condenseFillAndOutline(fill, line))

        line = QgsSimpleLineSymbolLayer()
        line.setOffset(1)
        self.assertFalse(QgsSymbolLayerUtils.condenseFillAndOutline(fill, line))

        line = QgsSimpleLineSymbolLayer()
        line.setDataDefinedProperty(QgsSymbolLayer.PropertyTrimEnd, QgsProperty.fromValue(4))
        self.assertFalse(QgsSymbolLayerUtils.condenseFillAndOutline(fill, line))

        # compatible!
        line = QgsSimpleLineSymbolLayer()
        line.setColor(QColor(255, 0, 0))
        line.setWidth(1.2)
        line.setWidthUnit(QgsUnitTypes.RenderPoints)
        line.setWidthMapUnitScale(QgsMapUnitScale(1, 2))
        line.setPenJoinStyle(Qt.MiterJoin)
        line.setPenStyle(Qt.DashDotDotLine)
        self.assertTrue(QgsSymbolLayerUtils.condenseFillAndOutline(fill, line))

        self.assertEqual(fill.strokeColor(), QColor(255, 0, 0))
        self.assertEqual(fill.strokeWidth(), 1.2)
        self.assertEqual(fill.strokeWidthUnit(), QgsUnitTypes.RenderPoints)
        self.assertEqual(fill.strokeWidthMapUnitScale(), QgsMapUnitScale(1, 2))
        self.assertEqual(fill.penJoinStyle(), Qt.MiterJoin)
        self.assertEqual(fill.strokeStyle(), Qt.DashDotDotLine)

    def test_renderer_frame_rate(self):
        # renderer without an animated symbol
        marker_symbol = QgsMarkerSymbol.createSimple({})
        renderer = QgsSingleSymbolRenderer(marker_symbol)
        self.assertEqual(QgsSymbolLayerUtils.rendererFrameRate(renderer), -1)

        # renderer with an animated symbol
        marker_symbol = QgsMarkerSymbol()
        animated_marker = QgsAnimatedMarkerSymbolLayer()
        animated_marker.setFrameRate(30)
        marker_symbol.appendSymbolLayer(animated_marker)
        renderer = QgsSingleSymbolRenderer(marker_symbol)
        self.assertEqual(QgsSymbolLayerUtils.rendererFrameRate(renderer), 30)

        # renderer with two animated symbol layers
        marker_symbol = QgsMarkerSymbol()
        animated_marker = QgsAnimatedMarkerSymbolLayer()
        animated_marker.setFrameRate(30)
        marker_symbol.appendSymbolLayer(animated_marker)
        animated_marker = QgsAnimatedMarkerSymbolLayer()
        animated_marker.setFrameRate(60)
        marker_symbol.appendSymbolLayer(animated_marker)
        renderer = QgsSingleSymbolRenderer(marker_symbol)
        self.assertEqual(QgsSymbolLayerUtils.rendererFrameRate(renderer), 60)

        s = QgsMarkerSymbol()
        renderer = QgsSingleSymbolRenderer(s.clone())
        self.assertEqual(QgsSymbolLayerUtils.rendererFrameRate(renderer), -1)
        s.animationSettings().setIsAnimated(True)
        s.animationSettings().setFrameRate(30)
        renderer = QgsSingleSymbolRenderer(s.clone())
        self.assertEqual(QgsSymbolLayerUtils.rendererFrameRate(renderer), 30)

    def imageCheck(self, name, reference_image, image):
        self.report += "<h2>Render {}</h2>\n".format(name)
        temp_dir = QDir.tempPath() + '/'
        file_name = temp_dir + name + ".png"
        image.save(file_name, "PNG")
        checker = QgsRenderChecker()
        checker.setControlPathPrefix("symbol_layer_utils")
        checker.setControlName("expected_" + reference_image)
        checker.setRenderedImage(file_name)
        checker.setColorTolerance(2)
        result = checker.compareImages(name, 20)
        PyQgsSymbolLayerUtils.report += checker.report()
        return result


if __name__ == '__main__':
    unittest.main()
