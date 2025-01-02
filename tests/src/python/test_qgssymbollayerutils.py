"""QGIS Unit tests for QgsSymbolLayerUtils.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "2016-09"
__copyright__ = "Copyright 2016, The QGIS Project"

import math

from qgis.PyQt.QtCore import QDir, QMimeData, QPointF, QSize, QSizeF, Qt, QRectF
from qgis.PyQt.QtXml import QDomDocument, QDomElement
from qgis.PyQt.QtGui import QColor, QImage, QPolygonF
from qgis.core import (
    Qgis,
    QgsAnimatedMarkerSymbolLayer,
    QgsArrowSymbolLayer,
    QgsFillSymbol,
    QgsGradientColorRamp,
    QgsLinePatternFillSymbolLayer,
    QgsMapUnitScale,
    QgsMarkerLineSymbolLayer,
    QgsMarkerSymbol,
    QgsProperty,
    QgsReadWriteContext,
    QgsShapeburstFillSymbolLayer,
    QgsSimpleFillSymbolLayer,
    QgsSimpleLineSymbolLayer,
    QgsSingleSymbolRenderer,
    QgsSymbol,
    QgsSymbolLayer,
    QgsSymbolLayerUtils,
    QgsUnitTypes,
    QgsVectorLayer,
    QgsRenderContext,
    QgsGeometry,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class PyQgsSymbolLayerUtils(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "symbol_layer_utils"

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
        s2 = QgsSymbolLayerUtils.decodeSize("")
        self.assertEqual(s2, QSizeF(0, 0))

    def testToSize(self):
        s2, ok = QgsSymbolLayerUtils.toSize(None)
        self.assertFalse(ok)

        s2, ok = QgsSymbolLayerUtils.toSize(4)
        self.assertFalse(ok)

        s2, ok = QgsSymbolLayerUtils.toSize("4")
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

        s2, ok = QgsSymbolLayerUtils.toSize(["4", "5"])
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
        s2, ok = QgsSymbolLayerUtils.toSize("")
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
        s2 = QgsSymbolLayerUtils.decodePoint("")
        self.assertEqual(s2, QPointF())

    def testEncodeDecodeCoordinateReference(self):
        items = {
            "feature": Qgis.SymbolCoordinateReference.Feature,
            "viewport": Qgis.SymbolCoordinateReference.Viewport,
        }
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

        s2, ok = QgsSymbolLayerUtils.toPoint("4")
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

        s2, ok = QgsSymbolLayerUtils.toPoint(["4", "5"])
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
        s2, ok = QgsSymbolLayerUtils.toPoint("")
        self.assertFalse(ok)
        self.assertEqual(s2, QPointF())

    def testDecodeArrowHeadType(self):
        type, ok = QgsSymbolLayerUtils.decodeArrowHeadType(0)
        self.assertTrue(ok)
        self.assertEqual(type, QgsArrowSymbolLayer.HeadType.HeadSingle)
        type, ok = QgsSymbolLayerUtils.decodeArrowHeadType("single")
        self.assertTrue(ok)
        self.assertEqual(type, QgsArrowSymbolLayer.HeadType.HeadSingle)
        type, ok = QgsSymbolLayerUtils.decodeArrowHeadType("   SINGLE   ")
        self.assertTrue(ok)
        self.assertEqual(type, QgsArrowSymbolLayer.HeadType.HeadSingle)
        type, ok = QgsSymbolLayerUtils.decodeArrowHeadType(1)
        self.assertTrue(ok)
        self.assertEqual(type, QgsArrowSymbolLayer.HeadType.HeadReversed)
        type, ok = QgsSymbolLayerUtils.decodeArrowHeadType("reversed")
        self.assertTrue(ok)
        self.assertEqual(type, QgsArrowSymbolLayer.HeadType.HeadReversed)
        type, ok = QgsSymbolLayerUtils.decodeArrowHeadType(2)
        self.assertTrue(ok)
        self.assertEqual(type, QgsArrowSymbolLayer.HeadType.HeadDouble)
        type, ok = QgsSymbolLayerUtils.decodeArrowHeadType("double")
        self.assertTrue(ok)
        self.assertEqual(type, QgsArrowSymbolLayer.HeadType.HeadDouble)
        type, ok = QgsSymbolLayerUtils.decodeArrowHeadType("xxxxx")
        self.assertFalse(ok)
        type, ok = QgsSymbolLayerUtils.decodeArrowHeadType(34)
        self.assertFalse(ok)

    def testDecodeArrowType(self):
        type, ok = QgsSymbolLayerUtils.decodeArrowType(0)
        self.assertTrue(ok)
        self.assertEqual(type, QgsArrowSymbolLayer.ArrowType.ArrowPlain)
        type, ok = QgsSymbolLayerUtils.decodeArrowType("plain")
        self.assertTrue(ok)
        self.assertEqual(type, QgsArrowSymbolLayer.ArrowType.ArrowPlain)
        type, ok = QgsSymbolLayerUtils.decodeArrowType("   PLAIN   ")
        self.assertTrue(ok)
        self.assertEqual(type, QgsArrowSymbolLayer.ArrowType.ArrowPlain)
        type, ok = QgsSymbolLayerUtils.decodeArrowType(1)
        self.assertTrue(ok)
        self.assertEqual(type, QgsArrowSymbolLayer.ArrowType.ArrowLeftHalf)
        type, ok = QgsSymbolLayerUtils.decodeArrowType("lefthalf")
        self.assertTrue(ok)
        self.assertEqual(type, QgsArrowSymbolLayer.ArrowType.ArrowLeftHalf)
        type, ok = QgsSymbolLayerUtils.decodeArrowType(2)
        self.assertTrue(ok)
        self.assertEqual(type, QgsArrowSymbolLayer.ArrowType.ArrowRightHalf)
        type, ok = QgsSymbolLayerUtils.decodeArrowType("righthalf")
        self.assertTrue(ok)
        self.assertEqual(type, QgsArrowSymbolLayer.ArrowType.ArrowRightHalf)
        type, ok = QgsSymbolLayerUtils.decodeArrowType("xxxxx")
        self.assertFalse(ok)
        type, ok = QgsSymbolLayerUtils.decodeArrowType(34)
        self.assertFalse(ok)

    def test_decode_marker_clip(self):
        """
        Test decode marker clip
        """
        self.assertEqual(
            QgsSymbolLayerUtils.decodeMarkerClipMode(""),
            (Qgis.MarkerClipMode.Shape, False),
        )
        self.assertEqual(
            QgsSymbolLayerUtils.decodeMarkerClipMode("xxx"),
            (Qgis.MarkerClipMode.Shape, False),
        )
        self.assertEqual(
            QgsSymbolLayerUtils.decodeMarkerClipMode(" no   "),
            (Qgis.MarkerClipMode.NoClipping, True),
        )
        self.assertEqual(
            QgsSymbolLayerUtils.decodeMarkerClipMode(" NO   "),
            (Qgis.MarkerClipMode.NoClipping, True),
        )
        self.assertEqual(
            QgsSymbolLayerUtils.decodeMarkerClipMode(" shape   "),
            (Qgis.MarkerClipMode.Shape, True),
        )
        self.assertEqual(
            QgsSymbolLayerUtils.decodeMarkerClipMode(" Shape   "),
            (Qgis.MarkerClipMode.Shape, True),
        )
        self.assertEqual(
            QgsSymbolLayerUtils.decodeMarkerClipMode(" centroid_within   "),
            (Qgis.MarkerClipMode.CentroidWithin, True),
        )
        self.assertEqual(
            QgsSymbolLayerUtils.decodeMarkerClipMode(" Centroid_Within   "),
            (Qgis.MarkerClipMode.CentroidWithin, True),
        )
        self.assertEqual(
            QgsSymbolLayerUtils.decodeMarkerClipMode(" completely_within   "),
            (Qgis.MarkerClipMode.CompletelyWithin, True),
        )
        self.assertEqual(
            QgsSymbolLayerUtils.decodeMarkerClipMode(" Completely_Within   "),
            (Qgis.MarkerClipMode.CompletelyWithin, True),
        )

    def test_encode_marker_clip(self):
        """
        Test encode marker clip
        """
        self.assertEqual(
            QgsSymbolLayerUtils.encodeMarkerClipMode(Qgis.MarkerClipMode.Shape), "shape"
        )
        self.assertEqual(
            QgsSymbolLayerUtils.encodeMarkerClipMode(Qgis.MarkerClipMode.NoClipping),
            "no",
        )
        self.assertEqual(
            QgsSymbolLayerUtils.encodeMarkerClipMode(
                Qgis.MarkerClipMode.CentroidWithin
            ),
            "centroid_within",
        )
        self.assertEqual(
            QgsSymbolLayerUtils.encodeMarkerClipMode(
                Qgis.MarkerClipMode.CompletelyWithin
            ),
            "completely_within",
        )

    def test_decode_line_clip(self):
        """
        Test decode line clip
        """
        self.assertEqual(
            QgsSymbolLayerUtils.decodeLineClipMode(""),
            (Qgis.LineClipMode.ClipPainterOnly, False),
        )
        self.assertEqual(
            QgsSymbolLayerUtils.decodeLineClipMode("xxx"),
            (Qgis.LineClipMode.ClipPainterOnly, False),
        )
        self.assertEqual(
            QgsSymbolLayerUtils.decodeLineClipMode(" no   "),
            (Qgis.LineClipMode.NoClipping, True),
        )
        self.assertEqual(
            QgsSymbolLayerUtils.decodeLineClipMode(" NO   "),
            (Qgis.LineClipMode.NoClipping, True),
        )
        self.assertEqual(
            QgsSymbolLayerUtils.decodeLineClipMode(" during_render   "),
            (Qgis.LineClipMode.ClipPainterOnly, True),
        )
        self.assertEqual(
            QgsSymbolLayerUtils.decodeLineClipMode(" DURING_Render   "),
            (Qgis.LineClipMode.ClipPainterOnly, True),
        )
        self.assertEqual(
            QgsSymbolLayerUtils.decodeLineClipMode(" before_render   "),
            (Qgis.LineClipMode.ClipToIntersection, True),
        )
        self.assertEqual(
            QgsSymbolLayerUtils.decodeLineClipMode(" BEFORE_REnder   "),
            (Qgis.LineClipMode.ClipToIntersection, True),
        )

    def test_encode_line_clip(self):
        """
        Test encode line clip
        """
        self.assertEqual(
            QgsSymbolLayerUtils.encodeLineClipMode(Qgis.LineClipMode.ClipPainterOnly),
            "during_render",
        )
        self.assertEqual(
            QgsSymbolLayerUtils.encodeLineClipMode(Qgis.LineClipMode.NoClipping), "no"
        )
        self.assertEqual(
            QgsSymbolLayerUtils.encodeLineClipMode(
                Qgis.LineClipMode.ClipToIntersection
            ),
            "before_render",
        )

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
        encode = QgsSymbolLayerUtils.encodeSldUom(
            QgsUnitTypes.RenderUnit.RenderMillimeters
        )
        self.assertTupleEqual(encode, ("", 3.571428571428571))

        # mapunits
        encode = None
        encode = QgsSymbolLayerUtils.encodeSldUom(
            QgsUnitTypes.RenderUnit.RenderMapUnits
        )
        self.assertTupleEqual(
            encode, ("http://www.opengeospatial.org/se/units/metre", 0.001)
        )

        # meters at scale
        encode = None
        encode = QgsSymbolLayerUtils.encodeSldUom(
            QgsUnitTypes.RenderUnit.RenderMetersInMapUnits
        )
        self.assertTupleEqual(
            encode, ("http://www.opengeospatial.org/se/units/metre", 1.0)
        )

    def testDecodeSldUom(self):
        """
        Test Decodes a SLD unit of measure string to a render unit
        """

        # meter
        decode = None
        decode = QgsSymbolLayerUtils.decodeSldUom(
            "http://www.opengeospatial.org/se/units/metre"
        )
        self.assertEqual(decode, (QgsUnitTypes.RenderUnit.RenderMetersInMapUnits, 1.0))

        # foot
        decode = None
        decode = QgsSymbolLayerUtils.decodeSldUom(
            "http://www.opengeospatial.org/se/units/foot"
        )
        self.assertEqual(
            decode, (QgsUnitTypes.RenderUnit.RenderMetersInMapUnits, 0.3048)
        )

        # pixel
        decode = None
        decode = QgsSymbolLayerUtils.decodeSldUom(
            "http://www.opengeospatial.org/se/units/pixel"
        )
        self.assertEqual(decode, (QgsUnitTypes.RenderUnit.RenderPixels, 1.0))

    def testPolylineLength(self):
        """
        Test QgsSymbolLayerUtils.polylineLength
        """
        self.assertEqual(QgsSymbolLayerUtils.polylineLength(QPolygonF()), 0.0)
        self.assertEqual(
            QgsSymbolLayerUtils.polylineLength(
                QPolygonF([QPointF(11, 12), QPointF(11, 12)])
            ),
            0.0,
        )
        self.assertEqual(
            QgsSymbolLayerUtils.polylineLength(
                QPolygonF([QPointF(11, 2), QPointF(11, 12), QPointF(11, 12)])
            ),
            10.0,
        )
        self.assertEqual(
            QgsSymbolLayerUtils.polylineLength(
                QPolygonF([QPointF(11, 2), QPointF(11, 12), QPointF(111, 12)])
            ),
            110.0,
        )

    def testPolylineSubstring(self):
        res = QgsSymbolLayerUtils.polylineSubstring(QPolygonF(), 1, 2)  # no crash
        self.assertFalse(res)

        res = QgsSymbolLayerUtils.polylineSubstring(QPolygonF(), -1, 2)  # no crash
        self.assertFalse(res)

        res = QgsSymbolLayerUtils.polylineSubstring(QPolygonF(), 1, -2)  # no crash
        self.assertFalse(res)

        res = QgsSymbolLayerUtils.polylineSubstring(QPolygonF(), -1, -2)  # no crash
        self.assertFalse(res)

        res = QgsSymbolLayerUtils.polylineSubstring(
            QPolygonF([QPointF(11, 2), QPointF(11, 12), QPointF(111, 12)]), 0, -110
        )
        self.assertEqual([p for p in res], [])

        res = QgsSymbolLayerUtils.polylineSubstring(
            QPolygonF([QPointF(11, 2), QPointF(11, 12), QPointF(111, 12)]), 0, 110
        )
        self.assertEqual(
            [p for p in res], [QPointF(11, 2), QPointF(11, 12), QPointF(111, 12)]
        )

        res = QgsSymbolLayerUtils.polylineSubstring(
            QPolygonF([QPointF(11, 2), QPointF(11, 12), QPointF(111, 12)]), -1, -1000
        )
        self.assertFalse([p for p in res])

        res = QgsSymbolLayerUtils.polylineSubstring(
            QPolygonF([QPointF(11, 2), QPointF(11, 12), QPointF(111, 12)]), 1, -1000
        )
        self.assertFalse([p for p in res])

        res = QgsSymbolLayerUtils.polylineSubstring(
            QPolygonF([QPointF(11, 2), QPointF(11, 12), QPointF(111, 12)]), -1, 1000
        )
        self.assertEqual([p for p in res], [QPointF(110.0, 12.0), QPointF(111.0, 12.0)])

        res = QgsSymbolLayerUtils.polylineSubstring(
            QPolygonF([QPointF(11, 2), QPointF(11, 12), QPointF(111, 12)]),
            100000,
            -10000,
        )
        self.assertFalse([p for p in res])

        res = QgsSymbolLayerUtils.polylineSubstring(
            QPolygonF([QPointF(11, 2), QPointF(11, 12), QPointF(111, 12)]), 1, -109
        )
        self.assertEqual([p for p in res], [])

        res = QgsSymbolLayerUtils.polylineSubstring(
            QPolygonF([QPointF(11, 2), QPointF(11, 12), QPointF(111, 12)]), 1, 109
        )
        self.assertEqual(
            [p for p in res],
            [QPointF(11.0, 3.0), QPointF(11.0, 12.0), QPointF(110.0, 12.0)],
        )

        res = QgsSymbolLayerUtils.polylineSubstring(
            QPolygonF([QPointF(11, 2), QPointF(11, 12), QPointF(111, 12)]), -109, 109
        )
        self.assertEqual(
            [p for p in res],
            [QPointF(11.0, 3.0), QPointF(11.0, 12.0), QPointF(110.0, 12.0)],
        )

        res = QgsSymbolLayerUtils.polylineSubstring(
            QPolygonF([QPointF(11, 2), QPointF(11, 12), QPointF(111, 12)]), 1, -1000
        )
        self.assertEqual([p for p in res], [])

        res = QgsSymbolLayerUtils.polylineSubstring(
            QPolygonF([QPointF(11, 2), QPointF(11, 12), QPointF(111, 12)]), 1, 10
        )
        self.assertEqual([p for p in res], [QPointF(11, 3), QPointF(11, 12)])

        res = QgsSymbolLayerUtils.polylineSubstring(
            QPolygonF([QPointF(11, 2), QPointF(11, 12), QPointF(111, 12)]), 1, 0
        )
        self.assertEqual(
            [p for p in res], [QPointF(11, 3), QPointF(11, 12), QPointF(111, 12)]
        )

        res = QgsSymbolLayerUtils.polylineSubstring(
            QPolygonF([QPointF(11, 2), QPointF(11, 12), QPointF(111, 12)]), 1, -90
        )
        self.assertEqual(
            [p for p in res], [QPointF(11, 3), QPointF(11, 12), QPointF(21, 12)]
        )

    def testAppendPolyline(self):
        line = QPolygonF([QPointF(11, 2), QPointF(11, 12), QPointF(111, 12)])
        line2 = QPolygonF(
            [QPointF(111, 12), QPointF(111, 12), QPointF(111, 14), QPointF(111, 15)]
        )
        QgsSymbolLayerUtils.appendPolyline(line, line2)
        self.assertEqual(
            [p for p in line],
            [
                QPointF(11.0, 2.0),
                QPointF(11.0, 12.0),
                QPointF(111.0, 12.0),
                QPointF(111.0, 14.0),
                QPointF(111.0, 15.0),
            ],
        )

        line = QPolygonF([QPointF(11, 2), QPointF(11, 12), QPointF(111, 12)])
        line2 = QPolygonF([QPointF(111, 14), QPointF(111, 15)])
        QgsSymbolLayerUtils.appendPolyline(line, line2)
        self.assertEqual(
            [p for p in line],
            [
                QPointF(11.0, 2.0),
                QPointF(11.0, 12.0),
                QPointF(111.0, 12.0),
                QPointF(111.0, 14.0),
                QPointF(111.0, 15.0),
            ],
        )

    def testColorFromMimeData(self):
        data = QMimeData()
        color, has_alpha = QgsSymbolLayerUtils.colorFromMimeData(data)
        self.assertFalse(color.isValid())

        # color data
        data.setColorData(QColor(255, 0, 255))
        color, has_alpha = QgsSymbolLayerUtils.colorFromMimeData(data)
        self.assertTrue(color.isValid())
        self.assertEqual(color.name(), "#ff00ff")
        # should be true regardless of the actual color's opacity -- a QColor object has innate knowledge of the alpha,
        # so our input color HAS an alpha of 255
        self.assertTrue(has_alpha)
        self.assertEqual(color.alpha(), 255)

        data.setColorData(QColor(255, 0, 255, 100))
        color, has_alpha = QgsSymbolLayerUtils.colorFromMimeData(data)
        self.assertTrue(color.isValid())
        self.assertEqual(color.name(), "#ff00ff")
        self.assertEqual(color.alpha(), 100)
        self.assertTrue(has_alpha)

        # text data
        data = QMimeData()
        data.setText("#ff00ff")
        color, has_alpha = QgsSymbolLayerUtils.colorFromMimeData(data)
        self.assertTrue(color.isValid())
        self.assertEqual(color.name(), "#ff00ff")
        # should be False -- no alpha was specified
        self.assertFalse(has_alpha)
        self.assertEqual(color.alpha(), 255)

        data.setText("#ff00ff66")
        color, has_alpha = QgsSymbolLayerUtils.colorFromMimeData(data)
        self.assertTrue(color.isValid())
        self.assertEqual(color.name(), "#ff00ff")
        self.assertTrue(has_alpha)
        self.assertEqual(color.alpha(), 102)

        # "#" is optional
        data.setText("ff00ff66")
        color, has_alpha = QgsSymbolLayerUtils.colorFromMimeData(data)
        self.assertTrue(color.isValid())
        self.assertEqual(color.name(), "#ff00ff")
        self.assertTrue(has_alpha)
        self.assertEqual(color.alpha(), 102)

        data.setText("255,0,255")
        color, has_alpha = QgsSymbolLayerUtils.colorFromMimeData(data)
        self.assertTrue(color.isValid())
        self.assertEqual(color.name(), "#ff00ff")
        self.assertFalse(has_alpha)
        self.assertEqual(color.alpha(), 255)

        data.setText("255,0,255,0.5")
        color, has_alpha = QgsSymbolLayerUtils.colorFromMimeData(data)
        self.assertTrue(color.isValid())
        self.assertEqual(color.name(), "#ff00ff")
        self.assertTrue(has_alpha)
        self.assertEqual(color.alpha(), 128)

        data.setText("rgba(255,0,255,0.5)")
        color, has_alpha = QgsSymbolLayerUtils.colorFromMimeData(data)
        self.assertTrue(color.isValid())
        self.assertEqual(color.name(), "#ff00ff")
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
        self.assertTrue(
            self.image_check("color_ramp_horizontal", "color_ramp_horizontal", img)
        )

    def testPreviewColorRampHorizontalNoCheckboard(self):
        r = QgsGradientColorRamp(QColor(200, 0, 0, 200), QColor(0, 200, 0, 255))

        pix = QgsSymbolLayerUtils.colorRampPreviewPixmap(
            r, QSize(200, 100), drawTransparentBackground=False
        )
        img = QImage(pix)
        self.assertTrue(
            self.image_check("color_ramp_no_check", "color_ramp_no_check", img)
        )

    def testPreviewColorRampHorizontalFlipped(self):
        r = QgsGradientColorRamp(QColor(200, 0, 0, 200), QColor(0, 200, 0, 255))

        pix = QgsSymbolLayerUtils.colorRampPreviewPixmap(
            r, QSize(200, 100), flipDirection=True
        )
        img = QImage(pix)
        self.assertTrue(
            self.image_check(
                "color_ramp_horizontal_flipped", "color_ramp_horizontal_flipped", img
            )
        )

    def testPreviewColorRampVertical(self):
        r = QgsGradientColorRamp(QColor(200, 0, 0, 200), QColor(0, 200, 0, 255))

        pix = QgsSymbolLayerUtils.colorRampPreviewPixmap(
            r, QSize(100, 200), direction=Qt.Orientation.Vertical
        )
        img = QImage(pix)
        self.assertTrue(
            self.image_check("color_ramp_vertical", "color_ramp_vertical", img)
        )

    def testPreviewColorRampVerticalFlipped(self):
        r = QgsGradientColorRamp(QColor(200, 0, 0, 200), QColor(0, 200, 0, 255))

        pix = QgsSymbolLayerUtils.colorRampPreviewPixmap(
            r, QSize(100, 200), direction=Qt.Orientation.Vertical, flipDirection=True
        )
        img = QImage(pix)
        self.assertTrue(
            self.image_check(
                "color_ramp_vertical_flipped", "color_ramp_vertical_flipped", img
            )
        )

    def testCondenseFillAndOutline(self):
        """
        Test QgsSymbolLayerUtils.condenseFillAndOutline
        """
        self.assertFalse(QgsSymbolLayerUtils.condenseFillAndOutline(None, None))

        # not simple fill or line
        self.assertFalse(
            QgsSymbolLayerUtils.condenseFillAndOutline(
                QgsShapeburstFillSymbolLayer(), QgsSimpleLineSymbolLayer()
            )
        )
        self.assertFalse(
            QgsSymbolLayerUtils.condenseFillAndOutline(
                QgsSimpleFillSymbolLayer(), QgsMarkerLineSymbolLayer()
            )
        )

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
        line.setRingFilter(QgsSimpleLineSymbolLayer.RenderRingFilter.ExteriorRingOnly)
        self.assertFalse(QgsSymbolLayerUtils.condenseFillAndOutline(fill, line))

        line = QgsSimpleLineSymbolLayer()
        line.setOffset(1)
        self.assertFalse(QgsSymbolLayerUtils.condenseFillAndOutline(fill, line))

        line = QgsSimpleLineSymbolLayer()
        line.setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyTrimEnd, QgsProperty.fromValue(4)
        )
        self.assertFalse(QgsSymbolLayerUtils.condenseFillAndOutline(fill, line))

        # compatible!
        line = QgsSimpleLineSymbolLayer()
        line.setColor(QColor(255, 0, 0))
        line.setWidth(1.2)
        line.setWidthUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        line.setWidthMapUnitScale(QgsMapUnitScale(1, 2))
        line.setPenJoinStyle(Qt.PenJoinStyle.MiterJoin)
        line.setPenStyle(Qt.PenStyle.DashDotDotLine)
        self.assertTrue(QgsSymbolLayerUtils.condenseFillAndOutline(fill, line))

        self.assertEqual(fill.strokeColor(), QColor(255, 0, 0))
        self.assertEqual(fill.strokeWidth(), 1.2)
        self.assertEqual(fill.strokeWidthUnit(), QgsUnitTypes.RenderUnit.RenderPoints)
        self.assertEqual(fill.strokeWidthMapUnitScale(), QgsMapUnitScale(1, 2))
        self.assertEqual(fill.penJoinStyle(), Qt.PenJoinStyle.MiterJoin)
        self.assertEqual(fill.strokeStyle(), Qt.PenStyle.DashDotDotLine)

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

    def testTileSize(self):

        test_data = [
            # First quadrant
            [10, 20, 0, 10, 20, 0],
            [10, 20, math.pi, 10, 20, math.pi],
            [10, 10, math.pi / 4, 10 * math.sqrt(2), 10 * math.sqrt(2), math.pi / 4],
            [10, 20, math.pi / 2, 20, 10, math.pi / 2],
            [10, 20, math.pi / 4, 20 * math.sqrt(2), 20 * math.sqrt(2), math.pi / 4],
            [10, 20, math.pi / 6, 36, 72, 0.5880031703261417],  # Angle approx
            # Second quadrant
            [
                10,
                20,
                math.pi / 2 + math.pi / 6,
                72,
                36,
                math.pi / 2 + 0.5880031703261417,
            ],  # Angle approx
            [
                10,
                10,
                math.pi / 2 + math.pi / 4,
                10 * math.sqrt(2),
                10 * math.sqrt(2),
                math.pi / 2 + math.pi / 4,
            ],
            [10, 20, math.pi / 2 + math.pi / 2, 10, 20, math.pi / 2 + math.pi / 2],
            [
                10,
                20,
                math.pi / 2 + math.pi / 4,
                20 * math.sqrt(2),
                20 * math.sqrt(2),
                math.pi / 2 + math.pi / 4,
            ],
            # Third quadrant
            [
                10,
                20,
                math.pi + math.pi / 6,
                36,
                72,
                math.pi + 0.5880031703261417,
            ],  # Angle approx
            [
                10,
                10,
                math.pi + math.pi / 4,
                10 * math.sqrt(2),
                10 * math.sqrt(2),
                math.pi + math.pi / 4,
            ],
            [10, 20, math.pi + math.pi / 2, 20, 10, math.pi + math.pi / 2],
            [
                10,
                20,
                math.pi + math.pi / 4,
                20 * math.sqrt(2),
                20 * math.sqrt(2),
                math.pi + math.pi / 4,
            ],
            # Fourth quadrant
            [
                10,
                20,
                math.pi + math.pi / 2 + math.pi / 6,
                72,
                36,
                math.pi + math.pi / 2 + 0.5880031703261417,
            ],  # Angle approx
            [
                10,
                10,
                math.pi + math.pi / 2 + math.pi / 4,
                10 * math.sqrt(2),
                10 * math.sqrt(2),
                math.pi + math.pi / 2 + math.pi / 4,
            ],
            [
                10,
                20,
                math.pi + math.pi / 2 + math.pi / 4,
                20 * math.sqrt(2),
                20 * math.sqrt(2),
                math.pi + math.pi / 2 + math.pi / 4,
            ],
            # Test out of range angles > 2 PI
            # First quadrant
            [
                10,
                10,
                math.pi * 2 + math.pi / 4,
                10 * math.sqrt(2),
                10 * math.sqrt(2),
                math.pi / 4,
            ],
            [10, 20, math.pi * 2 + math.pi / 2, 20, 10, math.pi / 2],
            [
                10,
                20,
                math.pi * 2 + math.pi / 4,
                20 * math.sqrt(2),
                20 * math.sqrt(2),
                math.pi / 4,
            ],
            [
                10,
                20,
                math.pi * 2 + math.pi / 6,
                36,
                72,
                0.5880031703261417,
            ],  # Angle approx
            # Second quadrant
            [
                10,
                20,
                math.pi * 2 + math.pi / 2 + math.pi / 6,
                72,
                36,
                math.pi / 2 + 0.5880031703261417,
            ],  # Angle approx
            [
                10,
                10,
                math.pi * 2 + math.pi / 2 + math.pi / 4,
                10 * math.sqrt(2),
                10 * math.sqrt(2),
                math.pi / 2 + math.pi / 4,
            ],
            [
                10,
                20,
                math.pi * 2 + math.pi / 2 + math.pi / 2,
                10,
                20,
                math.pi / 2 + math.pi / 2,
            ],
            [
                10,
                20,
                math.pi * 2 + math.pi / 2 + math.pi / 4,
                20 * math.sqrt(2),
                20 * math.sqrt(2),
                math.pi / 2 + math.pi / 4,
            ],
            # Third quadrant
            [
                10,
                20,
                math.pi * 2 + math.pi + math.pi / 6,
                36,
                72,
                math.pi + 0.5880031703261417,
            ],  # Angle approx
            [
                10,
                10,
                math.pi * 2 + math.pi + math.pi / 4,
                10 * math.sqrt(2),
                10 * math.sqrt(2),
                math.pi + math.pi / 4,
            ],
            [
                10,
                20,
                math.pi * 2 + math.pi + math.pi / 2,
                20,
                10,
                math.pi + math.pi / 2,
            ],
            [
                10,
                20,
                math.pi * 2 + math.pi + math.pi / 4,
                20 * math.sqrt(2),
                20 * math.sqrt(2),
                math.pi + math.pi / 4,
            ],
            # Fourth quadrant
            [
                10,
                20,
                math.pi * 2 + math.pi + math.pi / 2 + math.pi / 6,
                72,
                36,
                math.pi + math.pi / 2 + 0.5880031703261417,
            ],  # Angle approx
            [
                10,
                10,
                math.pi * 2 + math.pi + math.pi / 2 + math.pi / 4,
                10 * math.sqrt(2),
                10 * math.sqrt(2),
                math.pi + math.pi / 2 + math.pi / 4,
            ],
            [
                10,
                20,
                math.pi * 2 + math.pi + math.pi / 2 + math.pi / 4,
                20 * math.sqrt(2),
                20 * math.sqrt(2),
                math.pi + math.pi / 2 + math.pi / 4,
            ],
            # Test out of range angles < 0
            # First quadrant
            [
                10,
                10,
                -math.pi * 2 + math.pi / 4,
                10 * math.sqrt(2),
                10 * math.sqrt(2),
                math.pi / 4,
            ],
            [10, 20, -math.pi * 2 + math.pi / 2, 20, 10, math.pi / 2],
            [
                10,
                20,
                -math.pi * 2 + math.pi / 4,
                20 * math.sqrt(2),
                20 * math.sqrt(2),
                math.pi / 4,
            ],
            [
                10,
                20,
                -math.pi * 2 + math.pi / 6,
                36,
                72,
                0.5880031703261417,
            ],  # Angle approx
            # Second quadrant
            [
                10,
                20,
                -math.pi * 2 + math.pi / 2 + math.pi / 6,
                72,
                36,
                math.pi / 2 + 0.5880031703261417,
            ],  # Angle approx
            [
                10,
                10,
                -math.pi * 2 + math.pi / 2 + math.pi / 4,
                10 * math.sqrt(2),
                10 * math.sqrt(2),
                math.pi / 2 + math.pi / 4,
            ],
            [
                10,
                20,
                -math.pi * 2 + math.pi / 2 + math.pi / 2,
                10,
                20,
                math.pi / 2 + math.pi / 2,
            ],
            [
                10,
                20,
                -math.pi * 2 + math.pi / 2 + math.pi / 4,
                20 * math.sqrt(2),
                20 * math.sqrt(2),
                math.pi / 2 + math.pi / 4,
            ],
            # Third quadrant
            [
                10,
                20,
                -math.pi * 2 + math.pi + math.pi / 6,
                36,
                72,
                math.pi + 0.5880031703261417,
            ],  # Angle approx
            [
                10,
                10,
                -math.pi * 2 + math.pi + math.pi / 4,
                10 * math.sqrt(2),
                10 * math.sqrt(2),
                math.pi + math.pi / 4,
            ],
            [
                10,
                20,
                -math.pi * 2 + math.pi + math.pi / 2,
                20,
                10,
                math.pi + math.pi / 2,
            ],
            [
                10,
                20,
                -math.pi * 2 + math.pi + math.pi / 4,
                20 * math.sqrt(2),
                20 * math.sqrt(2),
                math.pi + math.pi / 4,
            ],
            # Fourth quadrant
            [
                10,
                20,
                -math.pi * 2 + math.pi + math.pi / 2 + math.pi / 6,
                72,
                36,
                math.pi + math.pi / 2 + 0.5880031703261417,
            ],  # Angle approx
            [
                10,
                10,
                -math.pi * 2 + math.pi + math.pi / 2 + math.pi / 4,
                10 * math.sqrt(2),
                10 * math.sqrt(2),
                math.pi + math.pi / 2 + math.pi / 4,
            ],
            [
                10,
                20,
                -math.pi * 2 + math.pi + math.pi / 2 + math.pi / 4,
                20 * math.sqrt(2),
                20 * math.sqrt(2),
                math.pi + math.pi / 2 + math.pi / 4,
            ],
        ]

        for width, height, angle, exp_width, exp_height, exp_angle in test_data:
            (res_size, res_angle) = QgsSymbolLayerUtils.tileSize(width, height, angle)
            self.assertEqual(res_size.height(), int(exp_height), angle)
            self.assertEqual(res_size.width(), int(exp_width))
            self.assertAlmostEqual(res_angle, exp_angle)

    def test_clear_symbollayer_ids(self):
        """
        Test we manage to clear all symbol layer ids on a symbol
        """

        source = QgsVectorLayer("Polygon?crs=EPSG:4326", "layer", "memory")
        self.assertTrue(source.isValid())

        layer = QgsLinePatternFillSymbolLayer()
        fill_symbol = QgsFillSymbol([layer])

        sub_renderer = QgsSingleSymbolRenderer(fill_symbol)
        source.setRenderer(sub_renderer)

        self.assertEqual(len(fill_symbol.symbolLayers()), 1)

        subsymbol = fill_symbol.symbolLayers()[0].subSymbol()
        self.assertTrue(subsymbol)
        self.assertEqual(len(subsymbol.symbolLayers()), 1)

        child_sl = subsymbol.symbolLayers()[0]
        self.assertTrue(child_sl)

        old_id = child_sl.id()
        self.assertTrue(child_sl.id())

        QgsSymbolLayerUtils.resetSymbolLayerIds(fill_symbol)

        self.assertTrue(child_sl.id())
        self.assertTrue(child_sl.id() != old_id)

        QgsSymbolLayerUtils.clearSymbolLayerIds(fill_symbol)
        self.assertFalse(child_sl.id())

    def test_font_marker_load(self):
        """
        Test the loading of font marker from XML
        """

        font_marker_xml_string = """<symbol clip_to_extent="1" type="marker" is_animated="0" alpha="1" name="symbol" frame_rate="10" force_rhr="0">
 <layer pass="0" id="{2aefc556-4eb1-4f56-b96b-e1dea6b58f69}" locked="0" class="FontMarker" enabled="1">
  <Option type="Map">
   <Option value="0" type="QString" name="angle"/>
   <Option value="~!_#!#_!~14~!_#!#_!~" type="QString" name="chr"/>
   <Option value="0,0,255,255" type="QString" name="color"/>
   <Option value="Arial" type="QString" name="font"/>
   <Option value="Italic" type="QString" name="font_style"/>
   <Option value="1" type="QString" name="horizontal_anchor_point"/>
   <Option value="miter" type="QString" name="joinstyle"/>
   <Option value="0,0" type="QString" name="offset"/>
   <Option value="3x:0,0,0,0,0,0" type="QString" name="offset_map_unit_scale"/>
   <Option value="Point" type="QString" name="offset_unit"/>
   <Option value="255,255,255,255" type="QString" name="outline_color"/>
   <Option value="0" type="QString" name="outline_width"/>
   <Option value="3x:0,0,0,0,0,0" type="QString" name="outline_width_map_unit_scale"/>
   <Option value="MM" type="QString" name="outline_width_unit"/>
   <Option value="42.4" type="QString" name="size"/>
   <Option value="3x:0,0,0,0,0,0" type="QString" name="size_map_unit_scale"/>
   <Option value="Point" type="QString" name="size_unit"/>
   <Option value="1" type="QString" name="vertical_anchor_point"/>
  </Option>
 </layer>
</symbol>"""

        doc = QDomDocument()
        elem = QDomElement()
        doc.setContent(font_marker_xml_string)
        elem = doc.documentElement()
        font_marker = QgsSymbolLayerUtils.loadSymbol(elem, QgsReadWriteContext())
        self.assertEqual(font_marker.symbolLayers()[0].character(), chr(14))

        font_marker_xml_string = """<symbol clip_to_extent="1" type="marker" is_animated="0" alpha="1" name="symbol" frame_rate="10" force_rhr="0">
 <layer pass="0" id="{2aefc556-4eb1-4f56-b96b-e1dea6b58f69}" locked="0" class="FontMarker" enabled="1">
  <Option type="Map">
   <Option value="0" type="QString" name="angle"/>
   <Option value="~!_#!#_!~40~!_#!#_!~~!_#!#_!~41~!_#!#_!~" type="QString" name="chr"/>
   <Option value="0,0,255,255" type="QString" name="color"/>
   <Option value="Arial" type="QString" name="font"/>
   <Option value="Italic" type="QString" name="font_style"/>
   <Option value="1" type="QString" name="horizontal_anchor_point"/>
   <Option value="miter" type="QString" name="joinstyle"/>
   <Option value="0,0" type="QString" name="offset"/>
   <Option value="3x:0,0,0,0,0,0" type="QString" name="offset_map_unit_scale"/>
   <Option value="Point" type="QString" name="offset_unit"/>
   <Option value="255,255,255,255" type="QString" name="outline_color"/>
   <Option value="0" type="QString" name="outline_width"/>
   <Option value="3x:0,0,0,0,0,0" type="QString" name="outline_width_map_unit_scale"/>
   <Option value="MM" type="QString" name="outline_width_unit"/>
   <Option value="42.4" type="QString" name="size"/>
   <Option value="3x:0,0,0,0,0,0" type="QString" name="size_map_unit_scale"/>
   <Option value="Point" type="QString" name="size_unit"/>
   <Option value="1" type="QString" name="vertical_anchor_point"/>
  </Option>
 </layer>
</symbol>"""

        doc.setContent(font_marker_xml_string)
        elem = doc.documentElement()
        font_marker = QgsSymbolLayerUtils.loadSymbol(elem, QgsReadWriteContext())
        self.assertEqual(font_marker.symbolLayers()[0].character(), "()")

    def test_extent_buffer_load(self):
        doc = QDomDocument()
        elem = QDomElement()

        extent_buffer_xml_string = """<symbol is_animated="0" name="0" type="fill" alpha="1" clip_to_extent="1" force_rhr="0" frame_rate="10" extent_buffer="1000" extent_buffer_unit="MM">
  <data_defined_properties>
  <Option type="Map">
   <Option value="" name="name" type="QString"/>
   <Option name="properties" type="Map">
    <Option name="extent_buffer" type="Map">
     <Option value="true" name="active" type="bool"/>
     <Option value="if(@map_scale &lt;= 25000, 5000, 10000)" name="expression" type="QString"/>
     <Option value="3" name="type" type="int"/>
    </Option>
   </Option>
   <Option value="collection" name="type" type="QString"/>
  </Option>
 </data_defined_properties>
 <layer pass="0" id="{2aefc556-4eb1-4f56-b96b-e1dea6b58f69}" locked="0" class="FontMarker" enabled="1">
  <Option type="Map">
   <Option value="0" type="QString" name="angle"/>
   <Option value="~!_#!#_!~40~!_#!#_!~~!_#!#_!~41~!_#!#_!~" type="QString" name="chr"/>
   <Option value="0,0,255,255" type="QString" name="color"/>
   <Option value="Arial" type="QString" name="font"/>
   <Option value="Italic" type="QString" name="font_style"/>
   <Option value="1" type="QString" name="horizontal_anchor_point"/>
   <Option value="miter" type="QString" name="joinstyle"/>
   <Option value="0,0" type="QString" name="offset"/>
   <Option value="3x:0,0,0,0,0,0" type="QString" name="offset_map_unit_scale"/>
   <Option value="Point" type="QString" name="offset_unit"/>
   <Option value="255,255,255,255" type="QString" name="outline_color"/>
   <Option value="0" type="QString" name="outline_width"/>
   <Option value="3x:0,0,0,0,0,0" type="QString" name="outline_width_map_unit_scale"/>
   <Option value="MM" type="QString" name="outline_width_unit"/>
   <Option value="42.4" type="QString" name="size"/>
   <Option value="3x:0,0,0,0,0,0" type="QString" name="size_map_unit_scale"/>
   <Option value="Point" type="QString" name="size_unit"/>
   <Option value="1" type="QString" name="vertical_anchor_point"/>
  </Option>
 </layer>
</symbol>"""

        doc.setContent(extent_buffer_xml_string)
        elem = doc.documentElement()
        symbol = QgsSymbolLayerUtils.loadSymbol(elem, QgsReadWriteContext())
        self.assertEqual(symbol.extentBuffer(), 1000)
        self.assertEqual(symbol.extentBufferSizeUnit(), Qgis.RenderUnit.Millimeters)

        property = symbol.dataDefinedProperties().property(
            QgsSymbol.Property.ExtentBuffer
        )

        self.assertTrue(property.isActive())
        self.assertEqual(
            property.expressionString(), "if(@map_scale <= 25000, 5000, 10000)"
        )

    def test_collect_symbol_layer_clip_geometries(self):
        """
        Test logic relating to symbol layer clip geometries.
        """
        rc = QgsRenderContext()
        self.assertFalse(
            QgsSymbolLayerUtils.collectSymbolLayerClipGeometries(
                rc, "x", QRectF(0, 0, 10, 10)
            )
        )
        rc.addSymbolLayerClipGeometry(
            "x", QgsGeometry.fromWkt("Polygon(( 0 0, 1 0 , 1 1 , 0 1, 0 0 ))")
        )
        self.assertFalse(
            QgsSymbolLayerUtils.collectSymbolLayerClipGeometries(
                rc, "y", QRectF(0, 0, 10, 10)
            )
        )
        self.assertCountEqual(
            [
                g.asWkt()
                for g in QgsSymbolLayerUtils.collectSymbolLayerClipGeometries(
                    rc, "x", QRectF(0, 0, 10, 10)
                )
            ],
            ["Polygon ((0 0, 1 0, 1 1, 0 1, 0 0))"],
        )
        rc.addSymbolLayerClipGeometry(
            "x", QgsGeometry.fromWkt("Polygon(( 20 0, 21 0 , 21 1 , 20 1, 20 0 ))")
        )
        self.assertCountEqual(
            [
                g.asWkt()
                for g in QgsSymbolLayerUtils.collectSymbolLayerClipGeometries(
                    rc, "x", QRectF(0, 0, 10, 10)
                )
            ],
            ["Polygon ((0 0, 1 0, 1 1, 0 1, 0 0))"],
        )
        self.assertCountEqual(
            [
                g.asWkt()
                for g in QgsSymbolLayerUtils.collectSymbolLayerClipGeometries(
                    rc, "x", QRectF(15, 0, 10, 10)
                )
            ],
            ["Polygon ((20 0, 21 0, 21 1, 20 1, 20 0))"],
        )
        self.assertCountEqual(
            [
                g.asWkt()
                for g in QgsSymbolLayerUtils.collectSymbolLayerClipGeometries(
                    rc, "x", QRectF(0, 0, 25, 10)
                )
            ],
            [
                "Polygon ((0 0, 1 0, 1 1, 0 1, 0 0))",
                "Polygon ((20 0, 21 0, 21 1, 20 1, 20 0))",
            ],
        )

        # null rect
        self.assertCountEqual(
            [
                g.asWkt()
                for g in QgsSymbolLayerUtils.collectSymbolLayerClipGeometries(
                    rc, "x", QRectF()
                )
            ],
            [
                "Polygon ((0 0, 1 0, 1 1, 0 1, 0 0))",
                "Polygon ((20 0, 21 0, 21 1, 20 1, 20 0))",
            ],
        )

        rc.addSymbolLayerClipGeometry(
            "y", QgsGeometry.fromWkt("Polygon(( 0 0, 2 0 , 2 1 , 0 1, 0 0 ))")
        )
        self.assertCountEqual(
            [
                g.asWkt()
                for g in QgsSymbolLayerUtils.collectSymbolLayerClipGeometries(
                    rc, "x", QRectF(0, 0, 25, 10)
                )
            ],
            [
                "Polygon ((0 0, 1 0, 1 1, 0 1, 0 0))",
                "Polygon ((20 0, 21 0, 21 1, 20 1, 20 0))",
            ],
        )
        self.assertCountEqual(
            [
                g.asWkt()
                for g in QgsSymbolLayerUtils.collectSymbolLayerClipGeometries(
                    rc, "y", QRectF(0, 0, 25, 10)
                )
            ],
            ["Polygon ((0 0, 2 0, 2 1, 0 1, 0 0))"],
        )

        # null rect
        self.assertCountEqual(
            [
                g.asWkt()
                for g in QgsSymbolLayerUtils.collectSymbolLayerClipGeometries(
                    rc, "x", QRectF()
                )
            ],
            [
                "Polygon ((0 0, 1 0, 1 1, 0 1, 0 0))",
                "Polygon ((20 0, 21 0, 21 1, 20 1, 20 0))",
            ],
        )
        self.assertCountEqual(
            [
                g.asWkt()
                for g in QgsSymbolLayerUtils.collectSymbolLayerClipGeometries(
                    rc, "y", QRectF()
                )
            ],
            ["Polygon ((0 0, 2 0, 2 1, 0 1, 0 0))"],
        )

    @staticmethod
    def polys_to_list(polys):
        return [
            [[[round(p.x(), 3), round(p.y(), 3)] for p in ring] for ring in poly]
            for poly in polys
        ]

    def test_to_qpolygonf(self):
        """
        Test conversion of QgsGeometry to QPolygonF
        """

        # points
        self.assertEqual(
            self.polys_to_list(
                QgsSymbolLayerUtils.toQPolygonF(
                    QgsGeometry.fromWkt("Point( 5 5 )"), Qgis.SymbolType.Marker
                )
            ),
            [[[[0, 0]]]],
        )
        self.assertEqual(
            self.polys_to_list(
                QgsSymbolLayerUtils.toQPolygonF(
                    QgsGeometry.fromWkt("Point( 5 5 )"), Qgis.SymbolType.Line
                )
            ),
            [],
        )
        self.assertEqual(
            self.polys_to_list(
                QgsSymbolLayerUtils.toQPolygonF(
                    QgsGeometry.fromWkt("Point( 5 5 )"), Qgis.SymbolType.Fill
                )
            ),
            [],
        )

        # multipoint
        self.assertEqual(
            self.polys_to_list(
                QgsSymbolLayerUtils.toQPolygonF(
                    QgsGeometry.fromWkt("MultiPoint((5 5), (1 2))"),
                    Qgis.SymbolType.Marker,
                )
            ),
            [[[[5.0, 5.0], [1.0, 2.0]]]],
        )
        self.assertEqual(
            self.polys_to_list(
                QgsSymbolLayerUtils.toQPolygonF(
                    QgsGeometry.fromWkt("MultiPoint((5 5), (1 2))"),
                    Qgis.SymbolType.Line,
                )
            ),
            [],
        )
        self.assertEqual(
            self.polys_to_list(
                QgsSymbolLayerUtils.toQPolygonF(
                    QgsGeometry.fromWkt("MultiPoint((5 5), (1 2))"),
                    Qgis.SymbolType.Fill,
                )
            ),
            [],
        )
        self.assertEqual(
            self.polys_to_list(
                QgsSymbolLayerUtils.toQPolygonF(
                    QgsGeometry.fromWkt("MultiPoint((5 5), (1 2), (4 3))"),
                    Qgis.SymbolType.Marker,
                )
            ),
            [[[[5.0, 5.0], [1.0, 2.0], [4.0, 3.0]]]],
        )

        # line
        self.assertEqual(
            self.polys_to_list(
                QgsSymbolLayerUtils.toQPolygonF(
                    QgsGeometry.fromWkt("LineString(1 5, 6 5)"), Qgis.SymbolType.Marker
                )
            ),
            [[[[0, 0]]]],
        )
        self.assertEqual(
            self.polys_to_list(
                QgsSymbolLayerUtils.toQPolygonF(
                    QgsGeometry.fromWkt("LineString(1 5, 6 5)"), Qgis.SymbolType.Line
                )
            ),
            [[[[1.0, 5.0], [6.0, 5.0]]]],
        )
        self.assertEqual(
            self.polys_to_list(
                QgsSymbolLayerUtils.toQPolygonF(
                    QgsGeometry.fromWkt("LineString(1 5, 6 5, 10 10)"),
                    Qgis.SymbolType.Line,
                )
            ),
            [[[[1.0, 5.0], [6.0, 5.0], [10, 10]]]],
        )
        self.assertEqual(
            self.polys_to_list(
                QgsSymbolLayerUtils.toQPolygonF(
                    QgsGeometry.fromWkt("LineString(1 5, 6 5)"), Qgis.SymbolType.Fill
                )
            ),
            [],
        )

        # circularstring
        self.assertEqual(
            self.polys_to_list(
                QgsSymbolLayerUtils.toQPolygonF(
                    QgsGeometry.fromWkt("CircularString(5 5, 1 2, 3 4)"),
                    Qgis.SymbolType.Line,
                )
            )[0][0][:5],
            [[5.0, 5.0], [5.131, 5.042], [5.263, 5.083], [5.396, 5.12], [5.529, 5.156]],
        )

        # multilinestring
        self.assertEqual(
            self.polys_to_list(
                QgsSymbolLayerUtils.toQPolygonF(
                    QgsGeometry.fromWkt("MultiLineString((5 5, 1 2),(3 6, 4 2))"),
                    Qgis.SymbolType.Line,
                )
            ),
            [[[[5.0, 5.0], [1.0, 2.0]]], [[[3.0, 6.0], [4.0, 2.0]]]],
        )

        # polygon
        self.assertEqual(
            self.polys_to_list(
                QgsSymbolLayerUtils.toQPolygonF(
                    QgsGeometry.fromWkt("Polygon((5 5, 1 2, 3 4, 5 5))"),
                    Qgis.SymbolType.Marker,
                )
            ),
            [[[[0.0, 0.0]]]],
        )
        self.assertEqual(
            self.polys_to_list(
                QgsSymbolLayerUtils.toQPolygonF(
                    QgsGeometry.fromWkt("Polygon((5 5, 1 2, 3 4, 5 5))"),
                    Qgis.SymbolType.Line,
                )
            ),
            [],
        )
        self.assertEqual(
            self.polys_to_list(
                QgsSymbolLayerUtils.toQPolygonF(
                    QgsGeometry.fromWkt("Polygon((5 5, 1 2, 3 4, 5 5))"),
                    Qgis.SymbolType.Fill,
                )
            ),
            [[[[5.0, 5.0], [1.0, 2.0], [3.0, 4.0], [5.0, 5.0]]]],
        )

        # rings
        self.assertEqual(
            self.polys_to_list(
                QgsSymbolLayerUtils.toQPolygonF(
                    QgsGeometry.fromWkt(
                        "Polygon((5 5, 1 2, 3 4, 5 5), (4.5 4.5, 4.4 4.4, 4.5 4.4, 4.5 4.5))"
                    ),
                    Qgis.SymbolType.Fill,
                )
            ),
            [
                [
                    [[5.0, 5.0], [1.0, 2.0], [3.0, 4.0], [5.0, 5.0]],
                    [[4.5, 4.5], [4.4, 4.4], [4.5, 4.4], [4.5, 4.5]],
                ]
            ],
        )

        # circular
        self.assertEqual(
            self.polys_to_list(
                QgsSymbolLayerUtils.toQPolygonF(
                    QgsGeometry.fromWkt(
                        "CurvePolygon(CircularString(5 5, 3 4, 1 2, 3 0, 5 5))"
                    ),
                    Qgis.SymbolType.Fill,
                )
            )[0][0][:5],
            [[5.0, 5.0], [4.87, 4.955], [4.741, 4.909], [4.612, 4.859], [4.485, 4.808]],
        )

        # multipolygon
        self.assertEqual(
            self.polys_to_list(
                QgsSymbolLayerUtils.toQPolygonF(
                    QgsGeometry.fromWkt(
                        "MultiPolygon(((5 5, 1 2, 3 4, 5 5), (4.5 4.5, 4.4 4.4, 4.5 4.4, 4.5 4.5)),((10 11, 11 11, 11 10, 10 11)))"
                    ),
                    Qgis.SymbolType.Fill,
                )
            ),
            [
                [
                    [[5.0, 5.0], [1.0, 2.0], [3.0, 4.0], [5.0, 5.0]],
                    [[4.5, 4.5], [4.4, 4.4], [4.5, 4.4], [4.5, 4.5]],
                ],
                [[[10.0, 11.0], [11.0, 11.0], [11.0, 10.0], [10.0, 11.0]]],
            ],
        )


if __name__ == "__main__":
    unittest.main()
