"""QGIS Unit tests for QgsTextRenderer.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "2016-09"
__copyright__ = "Copyright 2016, The QGIS Project"

import os
from typing import Optional

from qgis.PyQt.QtSvg import QSvgGenerator
from qgis.PyQt.QtCore import QT_VERSION_STR, QDir, QPointF, QRectF, QSize, QSizeF, Qt
from qgis.PyQt.QtGui import QBrush, QColor, QImage, QPainter, QPen, QPolygonF
from qgis.core import (
    Qgis,
    QgsBlurEffect,
    QgsFillSymbol,
    QgsFontUtils,
    QgsMapSettings,
    QgsMarkerSymbol,
    QgsPalLayerSettings,
    QgsProperty,
    QgsRectangle,
    QgsRenderContext,
    QgsSimpleFillSymbolLayer,
    QgsTextBackgroundSettings,
    QgsTextDocument,
    QgsTextDocumentMetrics,
    QgsTextFormat,
    QgsTextRenderer,
    QgsTextShadowSettings,
    QgsUnitTypes,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import getTestFont, svgSymbolsPath, unitTestDataPath

start_app()


class PyQgsTextRenderer(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        QgsFontUtils.loadStandardTestFonts(["Bold", "Oblique"])

    @classmethod
    def control_path_prefix(cls):
        return "text_renderer"

    def testAlignmentConversion(self):
        self.assertEqual(
            QgsTextRenderer.convertQtHAlignment(Qt.AlignmentFlag.AlignLeft),
            QgsTextRenderer.HAlignment.AlignLeft,
        )
        self.assertEqual(
            QgsTextRenderer.convertQtHAlignment(Qt.AlignmentFlag.AlignRight),
            QgsTextRenderer.HAlignment.AlignRight,
        )
        self.assertEqual(
            QgsTextRenderer.convertQtHAlignment(Qt.AlignmentFlag.AlignHCenter),
            QgsTextRenderer.HAlignment.AlignCenter,
        )
        self.assertEqual(
            QgsTextRenderer.convertQtHAlignment(Qt.AlignmentFlag.AlignJustify),
            QgsTextRenderer.HAlignment.AlignJustify,
        )
        # not supported, should fallback to left
        self.assertEqual(
            QgsTextRenderer.convertQtHAlignment(Qt.AlignmentFlag.AlignAbsolute),
            QgsTextRenderer.HAlignment.AlignLeft,
        )

        self.assertEqual(
            QgsTextRenderer.convertQtVAlignment(Qt.AlignmentFlag.AlignTop),
            QgsTextRenderer.VAlignment.AlignTop,
        )
        self.assertEqual(
            QgsTextRenderer.convertQtVAlignment(Qt.AlignmentFlag.AlignBottom),
            QgsTextRenderer.VAlignment.AlignBottom,
        )
        self.assertEqual(
            QgsTextRenderer.convertQtVAlignment(Qt.AlignmentFlag.AlignVCenter),
            QgsTextRenderer.VAlignment.AlignVCenter,
        )
        # note supported, should fallback to bottom
        self.assertEqual(
            QgsTextRenderer.convertQtVAlignment(Qt.AlignmentFlag.AlignBaseline),
            QgsTextRenderer.VAlignment.AlignBottom,
        )

    def testFontMetrics(self):
        """
        Test calculating font metrics from scaled text formats
        """
        s = QgsTextFormat()
        f = getTestFont()
        s.setFont(f)
        s.setSize(12)
        s.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)

        string = "xxxxxxxxxxxxxxxxxxxxxx"

        image = QImage(400, 400, QImage.Format.Format_RGB32)
        painter = QPainter(image)
        context = QgsRenderContext.fromQPainter(painter)
        context.setScaleFactor(1)
        metrics = QgsTextRenderer.fontMetrics(context, s)
        context.setScaleFactor(2)
        metrics2 = QgsTextRenderer.fontMetrics(context, s)
        painter.end()

        self.assertAlmostEqual(metrics.horizontalAdvance(string), 51.9, 1)
        self.assertAlmostEqual(metrics2.horizontalAdvance(string), 104.15, 1)

    def checkRender(
        self,
        format,
        name,
        part=None,
        angle=0,
        alignment=QgsTextRenderer.HAlignment.AlignLeft,
        text=["test"],
        rect=QRectF(100, 100, 50, 250),
        vAlignment=QgsTextRenderer.VAlignment.AlignTop,
        flags=Qgis.TextRendererFlags(),
        image_size=400,
        mode=Qgis.TextLayoutMode.Rectangle,
        reference_scale: Optional[float] = None,
        renderer_scale: Optional[float] = None,
    ):

        image = QImage(image_size, image_size, QImage.Format.Format_RGB32)

        painter = QPainter()
        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 50, 50))
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI
        context.setFlag(
            QgsRenderContext.Flag.ApplyScalingWorkaroundForTextRendering, True
        )
        if renderer_scale:
            context.setRendererScale(renderer_scale)
        if reference_scale:
            context.setSymbologyReferenceScale(reference_scale)

        for render_format in (
            Qgis.TextRenderFormat.AlwaysText,
            Qgis.TextRenderFormat.AlwaysOutlines,
            Qgis.TextRenderFormat.PreferText,
        ):

            context.setTextRenderFormat(render_format)
            painter.begin(image)
            painter.setRenderHint(QPainter.RenderHint.Antialiasing)
            image.fill(QColor(152, 219, 249))

            painter.setBrush(QBrush(QColor(182, 239, 255)))
            painter.setPen(Qt.PenStyle.NoPen)
            # to highlight rect on image
            # painter.drawRect(rect)

            if part is not None:
                QgsTextRenderer.drawPart(
                    rect, angle, alignment, text, context, format, part
                )
            else:
                QgsTextRenderer.drawText(
                    rect,
                    angle,
                    alignment,
                    text,
                    context,
                    format,
                    vAlignment=vAlignment,
                    flags=flags,
                    mode=mode,
                )

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
            if not self.image_check(name, name, image, control_name=name):
                return False
        return True

    def checkRenderPoint(
        self,
        format,
        name,
        part=None,
        angle=0,
        alignment=QgsTextRenderer.HAlignment.AlignLeft,
        text=["test"],
        point=QPointF(100, 200),
        image_size=400,
        enable_scale_workaround=False,
        render_mask=False,
    ):
        image = QImage(image_size, image_size, QImage.Format.Format_RGB32)

        painter = QPainter()
        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 50, 50))
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI
        if render_mask:
            context.setIsGuiPreview(True)

        context.setFlag(
            QgsRenderContext.Flag.ApplyScalingWorkaroundForTextRendering,
            enable_scale_workaround,
        )

        for render_format in (
            Qgis.TextRenderFormat.AlwaysText,
            Qgis.TextRenderFormat.AlwaysOutlines,
            Qgis.TextRenderFormat.PreferText,
        ):
            context.setTextRenderFormat(render_format)
            painter.begin(image)
            painter.setRenderHint(QPainter.RenderHint.Antialiasing)
            image.fill(QColor(152, 219, 249))

            painter.setBrush(QBrush(QColor(182, 239, 255)))
            painter.setPen(Qt.PenStyle.NoPen)
            # to highlight point on image
            # painter.drawRect(QRectF(point.x() - 5, point.y() - 5, 10, 10))

            if part is not None:
                QgsTextRenderer.drawPart(
                    point, angle, alignment, text, context, format, part
                )
            else:
                QgsTextRenderer.drawText(point, angle, alignment, text, context, format)

            painter.setFont(format.scaledFont(context))
            painter.setPen(QPen(QColor(255, 0, 255, 200)))
            # For comparison with QPainter's methods:
            # painter.drawText(point, '\n'.join(text))

            painter.end()
            if not self.image_check(name, name, image, control_name=name):
                return False
        return True

    def testDrawMassiveFont(self):
        """
        Test that we aren't bitten by https://bugreports.qt.io/browse/QTBUG-98778

        This test should pass when there's a correct WORD space between the 'a' and 't' characters, or fail when
        the spacing between these characters is nill or close to a letter spacing
        """
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(1100)
        self.assertTrue(
            self.checkRender(
                format,
                "massive_font",
                rect=QRectF(-800, -600, 1000, 1000),
                text=["a t"],
                image_size=800,
            )
        )

    def testDrawRectMixedHtml(self):
        """
        Test drawing text in rect mode with mixed html fonts
        """
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setAllowHtmlFormatting(True)
        format.setSize(30)
        self.assertTrue(
            self.checkRender(
                format,
                "rect_html",
                rect=QRectF(100, 100, 100, 100),
                text=[
                    'first <span style="font-size:50pt">line</span>',
                    'second <span style="font-size:50pt">line</span>',
                    "third line",
                ],
            )
        )

    def testDrawDocumentRect(self):
        """
        Test drawing text document in rect mode
        """
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setAllowHtmlFormatting(True)
        format.setSize(30)

        image = QImage(400, 400, QImage.Format.Format_RGB32)

        painter = QPainter()
        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 50, 50))
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI
        context.setFlag(
            QgsRenderContext.Flag.ApplyScalingWorkaroundForTextRendering, True
        )

        painter.begin(image)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)
        image.fill(QColor(152, 219, 249))

        painter.setBrush(QBrush(QColor(182, 239, 255)))
        painter.setPen(Qt.PenStyle.NoPen)

        doc = QgsTextDocument.fromHtml(
            [
                'first <span style="font-size:50pt">line</span>',
                'second <span style="font-size:50pt">line</span>',
                "third line",
            ]
        )

        metrics = QgsTextDocumentMetrics.calculateMetrics(
            doc,
            format,
            context,
            QgsTextRenderer.calculateScaleFactorForFormat(context, format),
        )

        QgsTextRenderer.drawDocument(
            QRectF(100, 100, 100, 100),
            format,
            doc,
            metrics,
            context,
            mode=Qgis.TextLayoutMode.Rectangle,
        )

        painter.end()

        self.assertTrue(
            self.image_check(
                "draw_document_rect", "draw_document_rect", image, "draw_document_rect"
            )
        )

    def testDrawRectHtmlBackground(self):
        """
        Test drawing html with backgrounds in rect mode
        """
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setAllowHtmlFormatting(True)
        format.setSize(16)
        self.assertTrue(
            self.checkRender(
                format,
                "html_background_rect",
                rect=QRectF(10, 100, 300, 100),
                text=[
                    '<div style="background-color: blue"><span style="background-color: rgba(255,0,0,0.5);">red</span> <span style="font-size: 10pt; background-color: yellow;">yellow</span> outside span</div><div style="background-color: pink; text-align: right;">no span <span style="background-color: yellow;">yel</span> no bg</div>'
                ],
            )
        )

    def testDrawPointHtmlBackground(self):
        """
        Test drawing html with backgrounds in point mode
        """
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setAllowHtmlFormatting(True)
        format.setSize(16)
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "html_background_point",
                point=QPointF(10, 100),
                text=[
                    '<div style="background-color: blue"><span style="background-color: rgba(255,0,0,0.5);">red</span> <span style="font-size: 10pt; background-color: yellow;">yellow</span> outside span</div><div style="background-color: pink; ext-align: right;">no span <span style="background-color: yellow;">yel</span> no bg</div>'
                ],
            )
        )

    def testDrawRectHtmlBackgroundImage(self):
        """
        Test drawing html with background image in rect mode
        """
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setAllowHtmlFormatting(True)
        format.setSize(36)
        image_url = unitTestDataPath() + "/raster_brush.png"
        self.assertTrue(
            self.checkRender(
                format,
                "html_background_image_rect",
                rect=QRectF(10, 100, 300, 100),
                text=[
                    f'<div style="background-image: url({image_url})">test text</div>'
                ],
            )
        )

    def testDrawPointHtmlBackgroundImage(self):
        """
        Test drawing html with backgrounds in point mode
        """
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setAllowHtmlFormatting(True)
        format.setSize(36)
        image_url = unitTestDataPath() + "/raster_brush.png"
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "html_background_image_point",
                point=QPointF(10, 100),
                text=[
                    f'<div style="background-image: url({image_url})">test text</div>'
                ],
            )
        )

    def testDrawRectCapHeightMode(self):
        """
        Test drawing text in rect mode with cap height based line heights
        """
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        self.assertTrue(
            self.checkRender(
                format,
                "rect_cap_height_mode",
                rect=QRectF(100, 100, 100, 100),
                text=["first line", "second line", "third line"],
                mode=Qgis.TextLayoutMode.RectangleCapHeightBased,
            )
        )

    def testDrawRectCapHeightModeMixedHtml(self):
        """
        Test drawing text in rect mode with cap height based line heights
        """
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setAllowHtmlFormatting(True)
        format.setSize(30)
        self.assertTrue(
            self.checkRender(
                format,
                "rect_cap_height_mode_html",
                rect=QRectF(100, 100, 100, 100),
                text=[
                    'first <span style="font-size:50pt">line</span>',
                    'second <span style="font-size:50pt">line</span>',
                    "third line",
                ],
                mode=Qgis.TextLayoutMode.RectangleCapHeightBased,
            )
        )

    def testDrawDocumentRectCapHeightMode(self):
        """
        Test drawing text document in rect cap height mode
        """
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setAllowHtmlFormatting(True)
        format.setSize(30)

        image = QImage(400, 400, QImage.Format.Format_RGB32)

        painter = QPainter()
        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 50, 50))
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI
        context.setFlag(
            QgsRenderContext.Flag.ApplyScalingWorkaroundForTextRendering, True
        )

        painter.begin(image)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)
        image.fill(QColor(152, 219, 249))

        painter.setBrush(QBrush(QColor(182, 239, 255)))
        painter.setPen(Qt.PenStyle.NoPen)

        doc = QgsTextDocument.fromHtml(
            [
                'first <span style="font-size:50pt">line</span>',
                'second <span style="font-size:50pt">line</span>',
                "third line",
            ]
        )

        metrics = QgsTextDocumentMetrics.calculateMetrics(
            doc,
            format,
            context,
            QgsTextRenderer.calculateScaleFactorForFormat(context, format),
        )

        QgsTextRenderer.drawDocument(
            QRectF(100, 100, 100, 100),
            format,
            doc,
            metrics,
            context,
            mode=Qgis.TextLayoutMode.RectangleCapHeightBased,
        )

        painter.end()

        self.assertTrue(
            self.image_check(
                "draw_document_rect_cap_height",
                "draw_document_rect_cap_height",
                image,
                "draw_document_rect_cap_height",
            )
        )

    def testDrawRectAscentMode(self):
        """
        Test drawing text in rect mode with cap height based line heights
        """
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        self.assertTrue(
            self.checkRender(
                format,
                "rect_ascent_mode",
                rect=QRectF(100, 100, 100, 100),
                text=["first line", "second line", "third line"],
                mode=Qgis.TextLayoutMode.RectangleAscentBased,
            )
        )

    def testDrawRectAscentModeMixedHtml(self):
        """
        Test drawing text in rect mode with ascent based line heights
        """
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setAllowHtmlFormatting(True)
        format.setSize(30)
        self.assertTrue(
            self.checkRender(
                format,
                "rect_ascent_mode_html",
                rect=QRectF(100, 100, 100, 100),
                text=[
                    'first <span style="font-size:50pt">line</span>',
                    'second <span style="font-size:50pt">line</span>',
                    "third line",
                ],
                mode=Qgis.TextLayoutMode.RectangleAscentBased,
            )
        )

    def testDrawDocumentRectAscentMode(self):
        """
        Test drawing text document in rect ascent mode
        """
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setAllowHtmlFormatting(True)
        format.setSize(30)

        image = QImage(400, 400, QImage.Format.Format_RGB32)

        painter = QPainter()
        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 50, 50))
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI
        context.setFlag(
            QgsRenderContext.Flag.ApplyScalingWorkaroundForTextRendering, True
        )

        painter.begin(image)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)
        image.fill(QColor(152, 219, 249))

        painter.setBrush(QBrush(QColor(182, 239, 255)))
        painter.setPen(Qt.PenStyle.NoPen)

        doc = QgsTextDocument.fromHtml(
            [
                'first <span style="font-size:50pt">line</span>',
                'second <span style="font-size:50pt">line</span>',
                "third line",
            ]
        )

        metrics = QgsTextDocumentMetrics.calculateMetrics(
            doc,
            format,
            context,
            QgsTextRenderer.calculateScaleFactorForFormat(context, format),
        )

        QgsTextRenderer.drawDocument(
            QRectF(100, 100, 100, 100),
            format,
            doc,
            metrics,
            context,
            mode=Qgis.TextLayoutMode.RectangleAscentBased,
        )

        painter.end()

        self.assertTrue(
            self.image_check(
                "draw_document_rect_ascent",
                "draw_document_rect_ascent",
                image,
                "draw_document_rect_ascent",
            )
        )

    def testDrawDocumentShadowPlacement(self):
        """
        Test drawing text document with shadow placement lowest
        """
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setAllowHtmlFormatting(True)
        format.setSize(30)
        format.setColor(QColor(255, 255, 255))

        format.shadow().setEnabled(True)
        format.shadow().setShadowPlacement(
            QgsTextShadowSettings.ShadowPlacement.ShadowLowest
        )
        format.shadow().setOpacity(1.0)
        format.shadow().setBlurRadius(0)
        format.shadow().setOffsetDistance(5)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)

        image = QImage(400, 400, QImage.Format.Format_RGB32)

        painter = QPainter()
        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 50, 50))
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI
        context.setFlag(
            QgsRenderContext.Flag.ApplyScalingWorkaroundForTextRendering, True
        )

        painter.begin(image)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)
        image.fill(QColor(152, 219, 249))

        painter.setBrush(QBrush(QColor(182, 239, 255)))
        painter.setPen(Qt.PenStyle.NoPen)

        doc = QgsTextDocument.fromHtml(
            [
                'first <span style="font-size:50pt">line</span>',
                'second <span style="font-size:50pt">line</span>',
                "third line",
            ]
        )

        metrics = QgsTextDocumentMetrics.calculateMetrics(
            doc,
            format,
            context,
            QgsTextRenderer.calculateScaleFactorForFormat(context, format),
        )

        QgsTextRenderer.drawDocument(
            QRectF(100, 100, 100, 100),
            format,
            doc,
            metrics,
            context,
            mode=Qgis.TextLayoutMode.RectangleAscentBased,
        )

        painter.end()

        self.assertTrue(
            self.image_check(
                "draw_document_shadow_lowest",
                "draw_document_shadow_lowest",
                image,
                "draw_document_shadow_lowest",
            )
        )

    def testDrawForcedItalic(self):
        """
        Test drawing with forced italic
        """
        format = QgsTextFormat()
        format.setFont(getTestFont())
        format.setSize(30)
        format.setForcedItalic(True)
        self.assertTrue(
            self.checkRender(format, "forced_italic", text=["Forced italic"])
        )

    def testDrawRTL(self):
        """
        Test drawing with simple rtl text
        """
        format = QgsTextFormat()
        format.setFont(getTestFont("Deja bold"))
        format.setSize(30)

        self.assertTrue(
            self.checkRenderPoint(
                format, "rtl", text=["مرحبا بالعالم"], point=QPointF(5, 200)
            )
        )

    def testDrawRTLHTML(self):
        """
        Test drawing with rtl text with HTML formatting
        """
        format = QgsTextFormat()
        format.setFont(getTestFont("Deja bold"))
        format.setSize(30)
        format.setAllowHtmlFormatting(True)
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "rtl_html",
                text=['<span style="font-size:50pt; color:green">بالعالم</span> مرحبا'],
                point=QPointF(5, 200),
            )
        )

    def testDrawRTLRightAlign(self):
        """
        Test drawing with rtl text with right align
        """
        format = QgsTextFormat()
        format.setFont(getTestFont("Deja bold"))
        format.setSize(30)

        self.assertTrue(
            self.checkRender(
                format,
                "rtl_right_align",
                text=["مرحبا بالعالم"],
                rect=QRectF(5, 100, 350, 250),
                alignment=QgsTextRenderer.HAlignment.AlignRight,
            )
        )

    def testDrawRTLBuffer(self):
        """
        Test drawing with right to left text with buffer
        """
        format = QgsTextFormat()
        format.setFont(getTestFont("Deja bold"))
        format.setSize(30)
        format.buffer().setEnabled(True)
        format.buffer().setSize(2)
        format.buffer().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)

        self.assertTrue(
            self.checkRenderPoint(
                format, "rtl_buffer", text=["مرحبا بالعالم"], point=QPointF(5, 200)
            )
        )

    def testDrawRTLShadow(self):
        """
        Test drawing with right to left text with shadow
        """
        format = QgsTextFormat()
        format.setFont(getTestFont("Deja bold"))
        format.setSize(30)
        format.setColor(QColor(255, 255, 0))
        format.shadow().setEnabled(True)
        format.shadow().setShadowPlacement(
            QgsTextShadowSettings.ShadowPlacement.ShadowText
        )
        format.shadow().setOpacity(1.0)
        format.shadow().setBlurRadius(0)
        format.shadow().setOffsetDistance(5)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)

        self.assertTrue(
            self.checkRenderPoint(
                format, "rtl_shadow", text=["مرحبا بالعالم"], point=QPointF(5, 200)
            )
        )

    @unittest.skip("broken")
    def testDrawTextOnLineRTL(self):
        """
        Test drawing right to left text on line
        """
        format = QgsTextFormat()
        format.setFont(getTestFont("Deja bold"))
        format.setSize(25)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)

        image = QImage(400, 400, QImage.Format.Format_RGB32)

        painter = QPainter()
        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 50, 50))
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI
        context.setFlag(
            QgsRenderContext.Flag.ApplyScalingWorkaroundForTextRendering, True
        )

        painter.begin(image)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)
        image.fill(QColor(152, 219, 249))

        painter.setBrush(Qt.BrushStyle.NoBrush)
        painter.setPen(QPen(QColor(0, 0, 0)))

        line = QPolygonF([QPointF(50, 200), QPointF(350, 200)])
        painter.drawPolygon(line)

        painter.setBrush(QBrush(QColor(182, 239, 255)))
        painter.setPen(Qt.PenStyle.NoPen)

        QgsTextRenderer.drawTextOnLine(line, "مرحبا بالعالم", context, format, 0)

        painter.end()
        self.assertTrue(
            self.image_check(
                "text_on_line_at_start",
                "text_on_line_at_start",
                image,
                "text_on_line_at_start",
            )
        )

    def testDrawRTLLTRMixed(self):
        """
        Test drawing with mixed right to left and left to right text
        """
        format = QgsTextFormat()
        format.setFont(getTestFont("Deja bold"))
        format.setSize(30)

        self.assertTrue(
            self.checkRenderPoint(
                format, "rtl_mixed", text=["hello באמת abc"], point=QPointF(5, 200)
            )
        )

    def testDrawRTLLTRMixedHtml(self):
        """
        Test drawing with right to left marker, html mode
        """
        format = QgsTextFormat()
        format.setFont(getTestFont("Deja bold"))
        format.setSize(30)
        format.setAllowHtmlFormatting(True)

        self.assertTrue(
            self.checkRenderPoint(
                format,
                "rtl_mixed_html",
                text=["<i>hello באמת </i>abc"],
                point=QPointF(5, 200),
            )
        )

    def testDrawRTLLTRMixedRect(self):
        """
        Test drawing with right to left marker in rect mode, right aligned
        """
        format = QgsTextFormat()
        format.setFont(getTestFont("Deja bold"))
        format.setSize(30)

        self.assertTrue(
            self.checkRender(
                format,
                "rtl_mixed_right_align",
                text=["hello באמת abc"],
                rect=QRectF(5, 100, 350, 250),
                alignment=QgsTextRenderer.HAlignment.AlignRight,
            )
        )

    def testDrawRTLLTRMixedBuffer(self):
        """
        Test drawing with right to left marker with buffer
        """
        format = QgsTextFormat()
        format.setFont(getTestFont("Deja bold"))
        format.setSize(30)
        format.buffer().setEnabled(True)
        format.buffer().setSize(2)
        format.buffer().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)

        self.assertTrue(
            self.checkRenderPoint(
                format,
                "rtl_mixed_buffer",
                text=["hello באמת abc"],
                point=QPointF(5, 200),
            )
        )

    def testDrawRTLLTRMixedShadow(self):
        """
        Test drawing with right to left marker with shadow
        """
        format = QgsTextFormat()
        format.setFont(getTestFont("Deja bold"))
        format.setSize(30)
        format.setColor(QColor(255, 255, 0))
        format.shadow().setEnabled(True)
        format.shadow().setShadowPlacement(
            QgsTextShadowSettings.ShadowPlacement.ShadowText
        )
        format.shadow().setOpacity(1.0)
        format.shadow().setBlurRadius(0)
        format.shadow().setOffsetDistance(5)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)

        self.assertTrue(
            self.checkRenderPoint(
                format,
                "rtl_mixed_shadow",
                text=["hello באמת abc"],
                point=QPointF(5, 200),
            )
        )

    @unittest.skipIf(
        int(QT_VERSION_STR.split(".")[0]) < 6
        or (
            int(QT_VERSION_STR.split(".")[0]) == 6
            and int(QT_VERSION_STR.split(".")[1]) < 3
        ),
        "Too old Qt",
    )
    def testDrawSmallCaps(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setCapitalization(Qgis.Capitalization.SmallCaps)
        format.setSize(30)
        self.assertTrue(
            self.checkRender(format, "mixed_small_caps", text=["Small Caps"])
        )

    @unittest.skipIf(
        int(QT_VERSION_STR.split(".")[0]) < 6
        or (
            int(QT_VERSION_STR.split(".")[0]) == 6
            and int(QT_VERSION_STR.split(".")[1]) < 3
        ),
        "Too old Qt",
    )
    def testDrawAllSmallCaps(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setCapitalization(Qgis.Capitalization.AllSmallCaps)
        self.assertTrue(self.checkRender(format, "all_small_caps", text=["Small Caps"]))

    @unittest.skipIf(
        int(QT_VERSION_STR.split(".")[0]) < 6
        or (
            int(QT_VERSION_STR.split(".")[0]) == 6
            and int(QT_VERSION_STR.split(".")[1]) < 3
        ),
        "Too old Qt",
    )
    def testDrawStretch(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setStretchFactor(150)
        self.assertTrue(self.checkRender(format, "stretch_expand"))

    @unittest.skipIf(
        int(QT_VERSION_STR.split(".")[0]) < 6
        or (
            int(QT_VERSION_STR.split(".")[0]) == 6
            and int(QT_VERSION_STR.split(".")[1]) < 3
        ),
        "Too old Qt",
    )
    def testDrawStretchCondense(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setStretchFactor(50)
        self.assertTrue(self.checkRender(format, "stretch_condense"))

    def testDrawBackgroundDisabled(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.background().setEnabled(False)
        self.assertTrue(
            self.checkRender(
                format, "background_disabled", QgsTextRenderer.TextPart.Background
            )
        )

    def testDrawBackgroundRectangleFixedSizeMapUnits(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(20, 10))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)
        self.assertTrue(
            self.checkRender(
                format, "background_rect_mapunits", QgsTextRenderer.TextPart.Background
            )
        )

    def testDrawBackgroundRectangleFixedSizeWithRotatedText(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(40)
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(20, 20))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        self.assertTrue(
            self.checkRenderPoint(
                format, "background_rect_fixed_rotated_text", angle=3.141 / 4
            )
        )

    def testDrawBackgroundRectangleBufferSizeWithRotatedText(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(40)
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(2, 3))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeBuffer)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        self.assertTrue(
            self.checkRenderPoint(
                format, "background_rect_buffer_rotated_text", angle=3.141 / 4
            )
        )

    def testDrawBackgroundRectangleMultilineFixedSizeMapUnits(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(20, 10))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)
        self.assertTrue(
            self.checkRender(
                format,
                "background_rect_multiline_mapunits",
                QgsTextRenderer.TextPart.Background,
                text=["test", "multi", "line"],
            )
        )

    def testDrawBackgroundPointMultilineFixedSizeMapUnits(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(20, 10))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "background_point_multiline_mapunits",
                QgsTextRenderer.TextPart.Background,
                text=["test", "multi", "line"],
            )
        )

    def testDrawBackgroundRectangleMultilineBufferMapUnits(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(4, 2))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeBuffer)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)
        self.assertTrue(
            self.checkRender(
                format,
                "background_rect_multiline_buffer_mapunits",
                QgsTextRenderer.TextPart.Background,
                text=["test", "multi", "line"],
            )
        )

    def testDrawBackgroundPointMultilineBufferMapUnits(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(4, 2))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeBuffer)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "background_point_multiline_buffer_mapunits",
                QgsTextRenderer.TextPart.Background,
                text=["test", "multi", "line"],
            )
        )

    def testDrawBackgroundPointFixedSizeMapUnits(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(20, 10))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "background_point_mapunits",
                QgsTextRenderer.TextPart.Background,
                text=["Testy"],
            )
        )

    def testDrawBackgroundRectangleCenterAlignFixedSizeMapUnits(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(20, 10))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)
        self.assertTrue(
            self.checkRender(
                format,
                "background_rect_center_mapunits",
                QgsTextRenderer.TextPart.Background,
                alignment=QgsTextRenderer.HAlignment.AlignCenter,
            )
        )

    def testDrawBackgroundPointCenterAlignFixedSizeMapUnits(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(20, 10))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "background_point_center_mapunits",
                QgsTextRenderer.TextPart.Background,
                alignment=QgsTextRenderer.HAlignment.AlignCenter,
            )
        )

    def testDrawBackgroundRectangleRightAlignFixedSizeMapUnits(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(20, 10))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)
        self.assertTrue(
            self.checkRender(
                format,
                "background_rect_right_mapunits",
                QgsTextRenderer.TextPart.Background,
                alignment=QgsTextRenderer.HAlignment.AlignRight,
            )
        )

    def testDrawBackgroundPointRightAlignFixedSizeMapUnits(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(20, 10))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "background_point_right_mapunits",
                QgsTextRenderer.TextPart.Background,
                alignment=QgsTextRenderer.HAlignment.AlignRight,
            )
        )

    def testDrawBackgroundRectangleFixedSizeMM(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(30, 20))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        self.assertTrue(
            self.checkRender(
                format, "background_rect_mm", QgsTextRenderer.TextPart.Background
            )
        )

    def testDrawBackgroundRectangleFixedSizePixels(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(60, 80))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderPixels)
        self.assertTrue(
            self.checkRender(
                format, "background_rect_pixels", QgsTextRenderer.TextPart.Background
            )
        )

    def testDrawBackgroundRectBufferPixels(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.background().setEnabled(True)
        svg = os.path.join(svgSymbolsPath(), "backgrounds", "background_square.svg")
        format.background().setSvgFile(svg)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(30, 50))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeBuffer)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderPixels)
        self.assertTrue(
            self.checkRender(
                format,
                "background_rect_buffer_pixels",
                QgsTextRenderer.TextPart.Background,
                rect=QRectF(100, 100, 100, 100),
            )
        )

    def testDrawBackgroundRectRightAlignBufferPixels(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.background().setEnabled(True)
        svg = os.path.join(svgSymbolsPath(), "backgrounds", "background_square.svg")
        format.background().setSvgFile(svg)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(30, 50))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeBuffer)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderPixels)
        self.assertTrue(
            self.checkRender(
                format,
                "background_rect_right_buffer_pixels",
                QgsTextRenderer.TextPart.Background,
                alignment=QgsTextRenderer.HAlignment.AlignRight,
                rect=QRectF(100, 100, 100, 100),
            )
        )

    def testDrawBackgroundRectCenterAlignBufferPixels(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.background().setEnabled(True)
        svg = os.path.join(svgSymbolsPath(), "backgrounds", "background_square.svg")
        format.background().setSvgFile(svg)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(30, 50))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeBuffer)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderPixels)
        self.assertTrue(
            self.checkRender(
                format,
                "background_rect_center_buffer_pixels",
                QgsTextRenderer.TextPart.Background,
                alignment=QgsTextRenderer.HAlignment.AlignCenter,
                rect=QRectF(100, 100, 100, 100),
            )
        )

    def testDrawBackgroundPointBufferPixels(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.background().setEnabled(True)
        svg = os.path.join(svgSymbolsPath(), "backgrounds", "background_square.svg")
        format.background().setSvgFile(svg)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(30, 50))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeBuffer)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderPixels)
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "background_point_buffer_pixels",
                QgsTextRenderer.TextPart.Background,
                point=QPointF(100, 100),
            )
        )

    def testDrawBackgroundPointRightAlignBufferPixels(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.background().setEnabled(True)
        svg = os.path.join(svgSymbolsPath(), "backgrounds", "background_square.svg")
        format.background().setSvgFile(svg)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(30, 50))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeBuffer)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderPixels)
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "background_point_right_buffer_pixels",
                QgsTextRenderer.TextPart.Background,
                alignment=QgsTextRenderer.HAlignment.AlignRight,
                point=QPointF(100, 100),
            )
        )

    def testDrawBackgroundPointCenterAlignBufferPixels(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.background().setEnabled(True)
        svg = os.path.join(svgSymbolsPath(), "backgrounds", "background_square.svg")
        format.background().setSvgFile(svg)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(30, 50))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeBuffer)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderPixels)
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "background_point_center_buffer_pixels",
                QgsTextRenderer.TextPart.Background,
                alignment=QgsTextRenderer.HAlignment.AlignCenter,
                point=QPointF(100, 100),
            )
        )

    def testDrawBackgroundRectBufferMapUnits(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.background().setEnabled(True)
        svg = os.path.join(svgSymbolsPath(), "backgrounds", "background_square.svg")
        format.background().setSvgFile(svg)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(4, 6))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeBuffer)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)
        self.assertTrue(
            self.checkRender(
                format,
                "background_rect_buffer_mapunits",
                QgsTextRenderer.TextPart.Background,
                rect=QRectF(100, 100, 100, 100),
            )
        )

    def testDrawBackgroundRectBufferMM(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.background().setEnabled(True)
        svg = os.path.join(svgSymbolsPath(), "backgrounds", "background_square.svg")
        format.background().setSvgFile(svg)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(10, 16))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeBuffer)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        self.assertTrue(
            self.checkRender(
                format,
                "background_rect_buffer_mm",
                QgsTextRenderer.TextPart.Background,
                rect=QRectF(100, 100, 100, 100),
            )
        )

    def testDrawBackgroundEllipse(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeEllipse)
        format.background().setSize(QSizeF(60, 80))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderPixels)
        self.assertTrue(
            self.checkRender(
                format, "background_ellipse_pixels", QgsTextRenderer.TextPart.Background
            )
        )

    def testDrawBackgroundSvgFixedPixels(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.background().setEnabled(True)
        svg = os.path.join(svgSymbolsPath(), "backgrounds", "background_square.svg")
        format.background().setSvgFile(svg)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeSVG)
        format.background().setSize(QSizeF(60, 80))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderPixels)
        self.assertTrue(
            self.checkRender(
                format,
                "background_svg_fixed_pixels",
                QgsTextRenderer.TextPart.Background,
            )
        )

    def testDrawBackgroundSvgFixedMapUnits(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.background().setEnabled(True)
        svg = os.path.join(svgSymbolsPath(), "backgrounds", "background_square.svg")
        format.background().setSvgFile(svg)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeSVG)
        format.background().setSize(QSizeF(20, 20))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)
        self.assertTrue(
            self.checkRender(
                format,
                "background_svg_fixed_mapunits",
                QgsTextRenderer.TextPart.Background,
            )
        )

    def testDrawBackgroundSvgFixedMM(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.background().setEnabled(True)
        svg = os.path.join(svgSymbolsPath(), "backgrounds", "background_square.svg")
        format.background().setSvgFile(svg)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeSVG)
        format.background().setSize(QSizeF(30, 30))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        self.assertTrue(
            self.checkRender(
                format, "background_svg_fixed_mm", QgsTextRenderer.TextPart.Background
            )
        )

    def testDrawBackgroundRotationSynced(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(30, 20))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        format.background().setRotation(45)  # should be ignored
        format.background().setRotationType(
            QgsTextBackgroundSettings.RotationType.RotationSync
        )
        self.assertTrue(
            self.checkRender(
                format,
                "background_rotation_sync",
                QgsTextRenderer.TextPart.Background,
                angle=20,
            )
        )

    def testDrawBackgroundSvgBufferPixels(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.background().setEnabled(True)
        svg = os.path.join(svgSymbolsPath(), "backgrounds", "background_square.svg")
        format.background().setSvgFile(svg)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeSVG)
        format.background().setSize(QSizeF(30, 30))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeBuffer)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderPixels)
        self.assertTrue(
            self.checkRender(
                format,
                "background_svg_buffer_pixels",
                QgsTextRenderer.TextPart.Background,
                rect=QRectF(100, 100, 100, 100),
            )
        )

    def testDrawBackgroundSvgBufferMapUnits(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.background().setEnabled(True)
        svg = os.path.join(svgSymbolsPath(), "backgrounds", "background_square.svg")
        format.background().setSvgFile(svg)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeSVG)
        format.background().setSize(QSizeF(4, 4))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeBuffer)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)
        self.assertTrue(
            self.checkRender(
                format,
                "background_svg_buffer_mapunits",
                QgsTextRenderer.TextPart.Background,
                rect=QRectF(100, 100, 100, 100),
            )
        )

    def testDrawBackgroundSvgBufferMM(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.background().setEnabled(True)
        svg = os.path.join(svgSymbolsPath(), "backgrounds", "background_square.svg")
        format.background().setSvgFile(svg)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeSVG)
        format.background().setSize(QSizeF(10, 10))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeBuffer)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        self.assertTrue(
            self.checkRender(
                format,
                "background_svg_buffer_mm",
                QgsTextRenderer.TextPart.Background,
                rect=QRectF(100, 100, 100, 100),
            )
        )

    def testDrawBackgroundMarkerFixedPixels(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.background().setEnabled(True)
        format.background().setMarkerSymbol(
            QgsMarkerSymbol.createSimple(
                {
                    "color": "#ffffff",
                    "size": "3",
                    "outline_color": "red",
                    "outline_width": "3",
                }
            )
        )
        format.background().setType(
            QgsTextBackgroundSettings.ShapeType.ShapeMarkerSymbol
        )
        format.background().setSize(QSizeF(60, 80))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderPixels)
        self.assertTrue(
            self.checkRender(
                format,
                "background_marker_fixed_pixels",
                QgsTextRenderer.TextPart.Background,
            )
        )

    def testDrawBackgroundMarkerFixedReferenceScale(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(16)
        format.background().setEnabled(True)
        format.background().setMarkerSymbol(
            QgsMarkerSymbol.createSimple(
                {
                    "color": "#ffffff",
                    "size": "3",
                    "outline_color": "red",
                    "outline_width": "3",
                }
            )
        )
        format.background().setType(
            QgsTextBackgroundSettings.ShapeType.ShapeMarkerSymbol
        )
        format.background().setSize(QSizeF(6, 8))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        self.assertTrue(
            self.checkRender(
                format,
                "background_marker_fixed_reference_scale",
                reference_scale=10000,
                renderer_scale=5000,
            )
        )

    def testDrawBackgroundMarkerFixedMapUnits(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.background().setEnabled(True)
        format.background().setMarkerSymbol(
            QgsMarkerSymbol.createSimple(
                {
                    "color": "#ffffff",
                    "size": "3",
                    "outline_color": "red",
                    "outline_width": "3",
                }
            )
        )
        format.background().setType(
            QgsTextBackgroundSettings.ShapeType.ShapeMarkerSymbol
        )
        format.background().setSize(QSizeF(20, 20))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)
        self.assertTrue(
            self.checkRender(
                format,
                "background_marker_fixed_mapunits",
                QgsTextRenderer.TextPart.Background,
            )
        )

    def testDrawBackgroundMarkerFixedMM(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.background().setEnabled(True)
        format.background().setMarkerSymbol(
            QgsMarkerSymbol.createSimple(
                {
                    "color": "#ffffff",
                    "size": "3",
                    "outline_color": "red",
                    "outline_width": "3",
                }
            )
        )
        format.background().setType(
            QgsTextBackgroundSettings.ShapeType.ShapeMarkerSymbol
        )
        format.background().setSize(QSizeF(30, 30))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        self.assertTrue(
            self.checkRender(
                format,
                "background_marker_fixed_mm",
                QgsTextRenderer.TextPart.Background,
            )
        )

    def testDrawBackgroundMarkerBufferPixels(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.background().setEnabled(True)
        format.background().setMarkerSymbol(
            QgsMarkerSymbol.createSimple(
                {
                    "color": "#ffffff",
                    "size": "3",
                    "outline_color": "red",
                    "outline_width": "3",
                }
            )
        )
        format.background().setType(
            QgsTextBackgroundSettings.ShapeType.ShapeMarkerSymbol
        )
        format.background().setSize(QSizeF(30, 30))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeBuffer)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderPixels)
        self.assertTrue(
            self.checkRender(
                format,
                "background_marker_buffer_pixels",
                QgsTextRenderer.TextPart.Background,
                rect=QRectF(100, 100, 100, 100),
            )
        )

    def testDrawBackgroundMarkerBufferMapUnits(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.background().setEnabled(True)
        format.background().setMarkerSymbol(
            QgsMarkerSymbol.createSimple(
                {
                    "color": "#ffffff",
                    "size": "3",
                    "outline_color": "red",
                    "outline_width": "3",
                }
            )
        )
        format.background().setType(
            QgsTextBackgroundSettings.ShapeType.ShapeMarkerSymbol
        )
        format.background().setSize(QSizeF(4, 4))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeBuffer)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)
        self.assertTrue(
            self.checkRender(
                format,
                "background_marker_buffer_mapunits",
                QgsTextRenderer.TextPart.Background,
                rect=QRectF(100, 100, 100, 100),
            )
        )

    def testDrawBackgroundMarkerBufferMM(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.background().setEnabled(True)
        format.background().setMarkerSymbol(
            QgsMarkerSymbol.createSimple(
                {
                    "color": "#ffffff",
                    "size": "3",
                    "outline_color": "red",
                    "outline_width": "3",
                }
            )
        )
        format.background().setType(
            QgsTextBackgroundSettings.ShapeType.ShapeMarkerSymbol
        )
        format.background().setSize(QSizeF(10, 10))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeBuffer)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        self.assertTrue(
            self.checkRender(
                format,
                "background_marker_buffer_mm",
                QgsTextRenderer.TextPart.Background,
                rect=QRectF(100, 100, 100, 100),
            )
        )

    def testDrawBackgroundRotationFixed(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(30, 20))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        format.background().setRotation(45)
        format.background().setRotationType(
            QgsTextBackgroundSettings.RotationType.RotationFixed
        )
        self.assertTrue(
            self.checkRender(
                format,
                "background_rotation_fixed",
                QgsTextRenderer.TextPart.Background,
                angle=20,
            )
        )

    def testDrawRotationOffset(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(30, 20))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        format.background().setRotation(45)
        format.background().setRotationType(
            QgsTextBackgroundSettings.RotationType.RotationOffset
        )
        self.assertTrue(
            self.checkRender(
                format,
                "background_rotation_offset",
                QgsTextRenderer.TextPart.Background,
                angle=20,
            )
        )

    def testDrawBackgroundOffsetMM(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(30, 20))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        format.background().setOffset(QPointF(30, 20))
        format.background().setOffsetUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        self.assertTrue(
            self.checkRender(
                format, "background_offset_mm", QgsTextRenderer.TextPart.Background
            )
        )

    def testDrawBackgroundOffsetMapUnits(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(30, 20))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        format.background().setOffset(QPointF(10, 5))
        format.background().setOffsetUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)
        self.assertTrue(
            self.checkRender(
                format,
                "background_offset_mapunits",
                QgsTextRenderer.TextPart.Background,
            )
        )

    def testDrawBackgroundRadiiMM(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(30, 20))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        format.background().setRadii(QSizeF(6, 4))
        format.background().setRadiiUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        self.assertTrue(
            self.checkRender(
                format, "background_radii_mm", QgsTextRenderer.TextPart.Background
            )
        )

    def testDrawBackgroundRadiiMapUnits(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(30, 20))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        format.background().setRadii(QSizeF(3, 2))
        format.background().setRadiiUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)
        self.assertTrue(
            self.checkRender(
                format, "background_radii_mapunits", QgsTextRenderer.TextPart.Background
            )
        )

    def testDrawBackgroundOpacity(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(30, 20))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeFixed)
        format.background().setOpacity(0.6)
        self.assertTrue(
            self.checkRender(
                format, "background_opacity", QgsTextRenderer.TextPart.Background
            )
        )

    def testDrawBackgroundFillColor(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(30, 20))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeFixed)
        format.background().setFillColor(QColor(50, 100, 50))
        self.assertTrue(
            self.checkRender(
                format, "background_fillcolor", QgsTextRenderer.TextPart.Background
            )
        )

    def testDrawBackgroundFillSymbol(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(30, 20))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeFixed)
        format.background().setFillColor(QColor(255, 0, 0))
        fill = QgsSimpleFillSymbolLayer()
        fill_symbol = QgsFillSymbol()
        fill_symbol.changeSymbolLayer(0, fill)
        fill.setColor(QColor(0, 255, 0, 25))
        fill.setStrokeColor(QColor(0, 0, 255))
        fill.setStrokeWidth(6)
        format.background().setFillSymbol(fill_symbol)

        self.assertTrue(
            self.checkRender(
                format, "background_fillsymbol", QgsTextRenderer.TextPart.Background
            )
        )

    def testDrawBackgroundStroke(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(30, 20))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeFixed)
        format.background().setStrokeColor(QColor(50, 100, 50))
        format.background().setStrokeWidth(3)
        format.background().setStrokeWidthUnit(
            QgsUnitTypes.RenderUnit.RenderMillimeters
        )
        self.assertTrue(
            self.checkRender(
                format, "background_outline", QgsTextRenderer.TextPart.Background
            )
        )

    def testDrawBackgroundEffect(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(30, 20))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeFixed)
        format.background().setPaintEffect(
            QgsBlurEffect.create({"blur_level": "10", "enabled": "1"})
        )
        self.assertTrue(
            self.checkRender(
                format,
                "background_effect",
                QgsTextRenderer.TextPart.Background,
                text=["test"],
            )
        )

    def testDrawText(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        self.assertTrue(
            self.checkRender(
                format, "text_bold", QgsTextRenderer.TextPart.Text, text=["test"]
            )
        )

    def testDrawTextPoint(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        self.assertTrue(
            self.checkRenderPoint(
                format, "text_point_bold", QgsTextRenderer.TextPart.Text, text=["test"]
            )
        )

    def testDrawTextNamedStyle(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        # need to call getTestFont to make sure font style is installed and ready to go
        temp_font = getTestFont("Bold Oblique")  # NOQA
        format.setFont(getTestFont())
        format.setNamedStyle("Bold Oblique")
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        self.assertTrue(
            self.checkRender(
                format, "text_named_style", QgsTextRenderer.TextPart.Text, text=["test"]
            )
        )

    def testDrawTextColor(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(0, 255, 0))
        self.assertTrue(
            self.checkRender(
                format, "text_color", QgsTextRenderer.TextPart.Text, text=["test"]
            )
        )

    def testDrawTextOpacity(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setOpacity(0.7)
        self.assertTrue(
            self.checkRender(
                format, "text_opacity", QgsTextRenderer.TextPart.Text, text=["test"]
            )
        )

    def testDrawTextBlendMode(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(100, 100, 100))
        format.setBlendMode(QPainter.CompositionMode.CompositionMode_Difference)
        self.assertTrue(
            self.checkRender(
                format, "text_blend_mode", QgsTextRenderer.TextPart.Text, text=["test"]
            )
        )

    def testDrawTextAngle(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        self.assertTrue(
            self.checkRender(
                format,
                "text_angled",
                QgsTextRenderer.TextPart.Text,
                angle=90 / 180 * 3.141,
                text=["test"],
            )
        )

    def testDrawTextMapUnits(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(5)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)
        self.assertTrue(
            self.checkRender(
                format, "text_mapunits", QgsTextRenderer.TextPart.Text, text=["test"]
            )
        )

    def testDrawTextPixels(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(50)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPixels)
        self.assertTrue(
            self.checkRender(
                format, "text_pixels", QgsTextRenderer.TextPart.Text, text=["test"]
            )
        )

    def testDrawMultiLineText(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        self.assertTrue(
            self.checkRender(
                format,
                "text_multiline",
                QgsTextRenderer.TextPart.Text,
                text=["test", "multi", "line"],
            )
        )

    def testDrawMultiLineTextPoint(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "text_point_multiline",
                QgsTextRenderer.TextPart.Text,
                text=["test", "multi", "line"],
            )
        )

    def testDrawLineHeightText(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setLineHeight(1.5)
        self.assertTrue(
            self.checkRender(
                format,
                "text_line_height",
                QgsTextRenderer.TextPart.Text,
                text=["test", "multi", "line"],
            )
        )

    def testDrawLineHeightAbsolutePoints(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setLineHeight(20)
        format.setLineHeightUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        self.assertTrue(
            self.checkRender(
                format,
                "text_line_absolute_height",
                QgsTextRenderer.TextPart.Text,
                text=["test", "multi", "line"],
            )
        )

    def testDrawLineHeightAbsoluteMillimeters(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setLineHeight(20)
        format.setLineHeightUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        self.assertTrue(
            self.checkRender(
                format,
                "text_line_absolute_mm_height",
                QgsTextRenderer.TextPart.Text,
                text=["test", "multi", "line"],
            )
        )

    def testDrawBufferSizeMM(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.buffer().setEnabled(True)
        format.buffer().setSize(2)
        format.buffer().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        self.assertTrue(
            self.checkRender(
                format, "text_buffer_mm", QgsTextRenderer.TextPart.Buffer, text=["test"]
            )
        )

    def testDrawBufferDisabled(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.buffer().setEnabled(False)
        self.assertTrue(
            self.checkRender(
                format,
                "text_disabled_buffer",
                QgsTextRenderer.TextPart.Buffer,
                text=["test"],
            )
        )

    def testDrawBufferSizeMapUnits(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.buffer().setEnabled(True)
        format.buffer().setSize(2)
        format.buffer().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)
        self.assertTrue(
            self.checkRender(
                format,
                "text_buffer_mapunits",
                QgsTextRenderer.TextPart.Buffer,
                text=["test"],
            )
        )

    def testDrawBufferSizePixels(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.buffer().setEnabled(True)
        format.buffer().setSize(10)
        format.buffer().setSizeUnit(QgsUnitTypes.RenderUnit.RenderPixels)
        self.assertTrue(
            self.checkRender(
                format,
                "text_buffer_pixels",
                QgsTextRenderer.TextPart.Buffer,
                text=["test"],
            )
        )

    def testDrawBufferSizePercentage(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.buffer().setEnabled(True)
        format.buffer().setSize(10)
        format.buffer().setSizeUnit(QgsUnitTypes.RenderUnit.RenderPercentage)
        self.assertTrue(
            self.checkRender(
                format,
                "text_buffer_percentage",
                QgsTextRenderer.TextPart.Buffer,
                text=["test"],
            )
        )

    def testDrawBufferColor(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.buffer().setEnabled(True)
        format.buffer().setSize(2)
        format.buffer().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        format.buffer().setColor(QColor(0, 255, 0))
        self.assertTrue(
            self.checkRender(
                format,
                "text_buffer_color",
                QgsTextRenderer.TextPart.Buffer,
                text=["test"],
            )
        )

    def testDrawBufferOpacity(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.buffer().setEnabled(True)
        format.buffer().setSize(2)
        format.buffer().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        format.buffer().setOpacity(0.5)
        self.assertTrue(
            self.checkRender(
                format,
                "text_buffer_opacity",
                QgsTextRenderer.TextPart.Buffer,
                text=["test"],
            )
        )

    def testDrawBufferFillInterior(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.buffer().setEnabled(True)
        format.buffer().setSize(2)
        format.buffer().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        format.buffer().setFillBufferInterior(True)
        self.assertTrue(
            self.checkRender(
                format,
                "text_buffer_interior",
                QgsTextRenderer.TextPart.Buffer,
                text=["test"],
            )
        )

    def testDrawBufferEffect(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.buffer().setEnabled(True)
        format.buffer().setSize(2)
        format.buffer().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        format.buffer().setPaintEffect(
            QgsBlurEffect.create({"blur_level": "10", "enabled": "1"})
        )
        self.assertTrue(
            self.checkRender(
                format,
                "text_buffer_effect",
                QgsTextRenderer.TextPart.Buffer,
                text=["test"],
            )
        )

    def testDrawShadow(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(255, 255, 255))
        format.shadow().setEnabled(True)
        format.shadow().setShadowPlacement(
            QgsTextShadowSettings.ShadowPlacement.ShadowText
        )
        format.shadow().setOpacity(1.0)
        format.shadow().setBlurRadius(0)
        format.shadow().setOffsetDistance(5)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        self.assertTrue(self.checkRender(format, "shadow_enabled", None, text=["test"]))

    def testDrawShadowOffsetAngle(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(255, 255, 255))
        format.shadow().setEnabled(True)
        format.shadow().setShadowPlacement(
            QgsTextShadowSettings.ShadowPlacement.ShadowText
        )
        format.shadow().setOpacity(1.0)
        format.shadow().setBlurRadius(0)
        format.shadow().setOffsetDistance(5)
        format.shadow().setOffsetAngle(0)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        self.assertTrue(
            self.checkRender(format, "shadow_offset_angle", None, text=["test"])
        )

    def testDrawShadowOffsetMapUnits(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(255, 255, 255))
        format.shadow().setEnabled(True)
        format.shadow().setShadowPlacement(
            QgsTextShadowSettings.ShadowPlacement.ShadowText
        )
        format.shadow().setOpacity(1.0)
        format.shadow().setBlurRadius(0)
        format.shadow().setOffsetDistance(10)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)
        self.assertTrue(
            self.checkRender(format, "shadow_offset_mapunits", None, text=["test"])
        )

    def testDrawShadowOffsetPixels(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(255, 255, 255))
        format.shadow().setEnabled(True)
        format.shadow().setShadowPlacement(
            QgsTextShadowSettings.ShadowPlacement.ShadowText
        )
        format.shadow().setOpacity(1.0)
        format.shadow().setBlurRadius(0)
        format.shadow().setOffsetDistance(10)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderUnit.RenderPixels)
        self.assertTrue(
            self.checkRender(format, "shadow_offset_pixels", None, text=["test"])
        )

    def testDrawShadowOffsetPercentage(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(255, 255, 255))
        format.shadow().setEnabled(True)
        format.shadow().setShadowPlacement(
            QgsTextShadowSettings.ShadowPlacement.ShadowText
        )
        format.shadow().setOpacity(1.0)
        format.shadow().setBlurRadius(0)
        format.shadow().setOffsetDistance(10)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderUnit.RenderPercentage)
        self.assertTrue(
            self.checkRender(format, "shadow_offset_percentage", None, text=["test"])
        )

    def testDrawShadowBlurRadiusMM(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(255, 255, 255))
        format.shadow().setEnabled(True)
        format.shadow().setShadowPlacement(
            QgsTextShadowSettings.ShadowPlacement.ShadowText
        )
        format.shadow().setOpacity(1.0)
        format.shadow().setOffsetDistance(5)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        format.shadow().setBlurRadius(1)
        format.shadow().setBlurRadiusUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        self.assertTrue(
            self.checkRender(format, "shadow_radius_mm", None, text=["test"])
        )

    def testDrawShadowBlurRadiusMapUnits(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(255, 255, 255))
        format.shadow().setEnabled(True)
        format.shadow().setShadowPlacement(
            QgsTextShadowSettings.ShadowPlacement.ShadowText
        )
        format.shadow().setOpacity(1.0)
        format.shadow().setOffsetDistance(5)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        format.shadow().setBlurRadius(3)
        format.shadow().setBlurRadiusUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)
        self.assertTrue(
            self.checkRender(format, "shadow_radius_mapunits", None, text=["test"])
        )

    def testDrawShadowBlurRadiusPixels(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(255, 255, 255))
        format.shadow().setEnabled(True)
        format.shadow().setShadowPlacement(
            QgsTextShadowSettings.ShadowPlacement.ShadowText
        )
        format.shadow().setOpacity(1.0)
        format.shadow().setOffsetDistance(5)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        format.shadow().setBlurRadius(3)
        format.shadow().setBlurRadiusUnit(QgsUnitTypes.RenderUnit.RenderPixels)
        self.assertTrue(
            self.checkRender(format, "shadow_radius_pixels", None, text=["test"])
        )

    def testDrawShadowBlurRadiusPercentage(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(255, 255, 255))
        format.shadow().setEnabled(True)
        format.shadow().setShadowPlacement(
            QgsTextShadowSettings.ShadowPlacement.ShadowText
        )
        format.shadow().setOpacity(1.0)
        format.shadow().setOffsetDistance(5)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        format.shadow().setBlurRadius(5)
        format.shadow().setBlurRadiusUnit(QgsUnitTypes.RenderUnit.RenderPercentage)
        self.assertTrue(
            self.checkRender(format, "shadow_radius_percentage", None, text=["test"])
        )

    def testDrawShadowOpacity(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(255, 255, 255))
        format.shadow().setEnabled(True)
        format.shadow().setShadowPlacement(
            QgsTextShadowSettings.ShadowPlacement.ShadowText
        )
        format.shadow().setOpacity(0.5)
        format.shadow().setBlurRadius(0)
        format.shadow().setOffsetDistance(5)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        self.assertTrue(self.checkRender(format, "shadow_opacity", None, text=["test"]))

    def testDrawShadowColor(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(255, 255, 255))
        format.shadow().setEnabled(True)
        format.shadow().setShadowPlacement(
            QgsTextShadowSettings.ShadowPlacement.ShadowText
        )
        format.shadow().setColor(QColor(255, 255, 0))
        format.shadow().setBlurRadius(0)
        format.shadow().setOffsetDistance(5)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        self.assertTrue(self.checkRender(format, "shadow_color", None, text=["test"]))

    def testDrawShadowWithJustifyAlign(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.shadow().setEnabled(True)
        format.shadow().setShadowPlacement(
            QgsTextShadowSettings.ShadowPlacement.ShadowText
        )
        format.shadow().setOpacity(0.5)
        format.shadow().setBlurRadius(0)
        format.shadow().setOffsetDistance(5)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        self.assertTrue(
            self.checkRender(
                format,
                "text_justify_aligned_with_shadow",
                text=["a t est", "off", "justification", "align"],
                alignment=QgsTextRenderer.HAlignment.AlignJustify,
                rect=QRectF(100, 100, 200, 100),
            )
        )

    def testDrawShadowScale(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(255, 255, 255))
        format.shadow().setEnabled(True)
        format.shadow().setShadowPlacement(
            QgsTextShadowSettings.ShadowPlacement.ShadowText
        )
        format.shadow().setScale(50)
        format.shadow().setBlurRadius(0)
        format.shadow().setOffsetDistance(5)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        self.assertTrue(
            self.checkRender(format, "shadow_scale_50", None, text=["test"])
        )

    def testDrawShadowScaleUp(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(255, 255, 255))
        format.shadow().setEnabled(True)
        format.shadow().setShadowPlacement(
            QgsTextShadowSettings.ShadowPlacement.ShadowText
        )
        format.shadow().setScale(150)
        format.shadow().setBlurRadius(0)
        format.shadow().setOffsetDistance(5)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        self.assertTrue(
            self.checkRender(format, "shadow_scale_150", None, text=["test"])
        )

    def testDrawShadowBackgroundPlacement(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(255, 255, 255))
        format.shadow().setEnabled(True)
        format.shadow().setShadowPlacement(
            QgsTextShadowSettings.ShadowPlacement.ShadowShape
        )
        format.shadow().setBlurRadius(0)
        format.shadow().setOffsetDistance(5)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(20, 10))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)
        self.assertTrue(
            self.checkRender(format, "shadow_placement_background", None, text=["test"])
        )

    def testDrawShadowBufferPlacement(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(255, 255, 255))
        format.shadow().setEnabled(True)
        format.shadow().setShadowPlacement(
            QgsTextShadowSettings.ShadowPlacement.ShadowBuffer
        )
        format.shadow().setBlurRadius(0)
        format.shadow().setOffsetDistance(5)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        format.buffer().setEnabled(True)
        format.buffer().setSize(4)
        format.buffer().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        self.assertTrue(
            self.checkRender(format, "shadow_placement_buffer", None, text=["test"])
        )

    def testDrawTextWithBuffer(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.buffer().setEnabled(True)
        format.buffer().setSize(4)
        format.buffer().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        self.assertTrue(
            self.checkRender(
                format,
                "text_with_buffer",
                text=["test"],
                rect=QRectF(100, 100, 200, 100),
            )
        )

    def testDrawTextWithBufferBlendMode(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(20, 10))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)
        format.background().setFillColor(QColor(200, 100, 150))
        format.buffer().setEnabled(True)
        format.buffer().setSize(4)
        format.buffer().setColor(QColor(100, 255, 100))
        format.buffer().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        format.buffer().setBlendMode(QPainter.CompositionMode.CompositionMode_Multiply)
        self.assertTrue(
            self.checkRender(
                format,
                "text_with_buffer_blend_mode",
                text=["test"],
                rect=QRectF(100, 100, 200, 100),
            )
        )

    def testDrawTextWithBackground(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(20, 10))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)
        self.assertTrue(
            self.checkRender(
                format,
                "text_with_background",
                text=["test"],
                rect=QRectF(100, 100, 200, 100),
            )
        )

    def testDrawTextWithBufferAndBackground(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(20, 10))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)
        format.buffer().setEnabled(True)
        format.buffer().setSize(4)
        format.buffer().setColor(QColor(100, 255, 100))
        format.buffer().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        self.assertTrue(
            self.checkRender(
                format,
                "text_with_buffer_and_background",
                text=["test"],
                rect=QRectF(100, 100, 200, 100),
            )
        )

    def testDrawTextWithShadowAndBuffer(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.shadow().setEnabled(True)
        format.shadow().setOpacity(1.0)
        format.shadow().setBlurRadius(0)
        format.shadow().setOffsetDistance(5)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        format.shadow().setColor(QColor(255, 100, 100))
        format.buffer().setEnabled(True)
        format.buffer().setSize(4)
        format.buffer().setColor(QColor(100, 255, 100))
        format.buffer().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        self.assertTrue(
            self.checkRender(
                format,
                "text_with_shadow_and_buffer",
                text=["test"],
                rect=QRectF(100, 100, 200, 100),
            )
        )

    def testDrawTextWithShadowBelowTextAndBuffer(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.shadow().setEnabled(True)
        format.shadow().setOpacity(1.0)
        format.shadow().setBlurRadius(0)
        format.shadow().setOffsetDistance(5)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        format.shadow().setColor(QColor(255, 100, 100))
        format.shadow().setShadowPlacement(
            QgsTextShadowSettings.ShadowPlacement.ShadowText
        )
        format.buffer().setEnabled(True)
        format.buffer().setSize(4)
        format.buffer().setColor(QColor(100, 255, 100))
        format.buffer().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        self.assertTrue(
            self.checkRender(
                format,
                "text_with_shadow_below_text_and_buffer",
                text=["test"],
                rect=QRectF(100, 100, 200, 100),
            )
        )

    def testDrawTextWithBackgroundAndShadow(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.shadow().setEnabled(True)
        format.shadow().setOpacity(1.0)
        format.shadow().setBlurRadius(0)
        format.shadow().setOffsetDistance(5)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        format.shadow().setColor(QColor(255, 100, 100))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(20, 10))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)
        self.assertTrue(
            self.checkRender(
                format,
                "text_with_shadow_and_background",
                text=["test"],
                rect=QRectF(100, 100, 200, 100),
            )
        )

    def testDrawTextWithShadowBelowTextAndBackground(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.shadow().setEnabled(True)
        format.shadow().setOpacity(1.0)
        format.shadow().setBlurRadius(0)
        format.shadow().setOffsetDistance(5)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        format.shadow().setColor(QColor(255, 100, 100))
        format.shadow().setShadowPlacement(
            QgsTextShadowSettings.ShadowPlacement.ShadowText
        )
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(20, 10))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)
        self.assertTrue(
            self.checkRender(
                format,
                "text_with_shadow_below_text_and_background",
                text=["test"],
                rect=QRectF(100, 100, 200, 100),
            )
        )

    def testDrawTextWithBackgroundBufferAndShadow(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.shadow().setEnabled(True)
        format.shadow().setOpacity(1.0)
        format.shadow().setBlurRadius(0)
        format.shadow().setOffsetDistance(5)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        format.shadow().setColor(QColor(255, 100, 100))
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(20, 10))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)
        format.buffer().setEnabled(True)
        format.buffer().setSize(4)
        format.buffer().setColor(QColor(100, 255, 100))
        format.buffer().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        self.assertTrue(
            self.checkRender(
                format,
                "text_with_shadow_buffer_and_background",
                text=["test"],
                rect=QRectF(100, 100, 200, 100),
            )
        )

    def testDrawTextWithBackgroundBufferAndShadowBelowText(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.shadow().setEnabled(True)
        format.shadow().setOpacity(1.0)
        format.shadow().setBlurRadius(0)
        format.shadow().setOffsetDistance(5)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        format.shadow().setColor(QColor(255, 100, 100))
        format.shadow().setShadowPlacement(
            QgsTextShadowSettings.ShadowPlacement.ShadowText
        )
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(20, 10))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)
        format.buffer().setEnabled(True)
        format.buffer().setSize(4)
        format.buffer().setColor(QColor(100, 255, 100))
        format.buffer().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        self.assertTrue(
            self.checkRender(
                format,
                "text_with_shadow_below_text_buffer_and_background",
                text=["test"],
                rect=QRectF(100, 100, 200, 100),
            )
        )

    def testDrawTextWithBackgroundBufferAndShadowBelowBuffer(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.shadow().setEnabled(True)
        format.shadow().setOpacity(1.0)
        format.shadow().setBlurRadius(0)
        format.shadow().setOffsetDistance(5)
        format.shadow().setOffsetUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        format.shadow().setColor(QColor(255, 100, 100))
        format.shadow().setShadowPlacement(
            QgsTextShadowSettings.ShadowPlacement.ShadowBuffer
        )
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(20, 10))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeFixed)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)
        format.buffer().setEnabled(True)
        format.buffer().setSize(4)
        format.buffer().setColor(QColor(100, 255, 100))
        format.buffer().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        self.assertTrue(
            self.checkRender(
                format,
                "text_with_shadow_below_buffer_and_background",
                text=["test"],
                rect=QRectF(100, 100, 200, 100),
            )
        )

    def testDrawTextRectMultilineRightAlign(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        self.assertTrue(
            self.checkRender(
                format,
                "text_rect_multiline_right_aligned",
                text=["test", "right", "aligned"],
                alignment=QgsTextRenderer.HAlignment.AlignRight,
                rect=QRectF(100, 100, 200, 100),
            )
        )

    def testDrawTextRectRightAlign(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        self.assertTrue(
            self.checkRender(
                format,
                "text_rect_right_aligned",
                text=["test"],
                alignment=QgsTextRenderer.HAlignment.AlignRight,
                rect=QRectF(100, 100, 200, 100),
            )
        )

    def testDrawTextRectMultilineJustifyAlign(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.buffer().setEnabled(True)
        format.buffer().setSize(4)
        format.buffer().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        self.assertTrue(
            self.checkRender(
                format,
                "text_rect_multiline_justify_aligned",
                text=["a t est", "off", "justification", "align"],
                alignment=QgsTextRenderer.HAlignment.AlignJustify,
                rect=QRectF(100, 100, 200, 100),
            )
        )

    def testDrawTextRectJustifyAlign(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        self.assertTrue(
            self.checkRender(
                format,
                "text_rect_justify_aligned",
                text=["test"],
                alignment=QgsTextRenderer.HAlignment.AlignJustify,
                rect=QRectF(100, 100, 200, 100),
            )
        )

    def testDrawTextRectMultiparagraphJustifyAlign(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.buffer().setEnabled(True)
        format.buffer().setSize(4)
        format.buffer().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        self.assertTrue(
            self.checkRender(
                format,
                "text_rect_multiparagraph_justify_aligned",
                text=["a t est", "of justify", "", "with two", "pgraphs"],
                alignment=QgsTextRenderer.HAlignment.AlignJustify,
                rect=QRectF(50, 100, 250, 100),
            )
        )

    def testDrawTextRectWordWrapSingleLine(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        painter = QPainter()
        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 50, 50))
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI
        context.setFlag(
            QgsRenderContext.Flag.ApplyScalingWorkaroundForTextRendering, True
        )

        self.assertTrue(
            QgsTextRenderer.textRequiresWrapping(
                context, "a test of word wrap", 100, format
            )
        )
        self.assertTrue(
            QgsTextRenderer.textRequiresWrapping(
                context, "a test of word wrap", 200, format
            )
        )
        self.assertTrue(
            QgsTextRenderer.textRequiresWrapping(
                context, "a test of word wrap", 400, format
            )
        )
        self.assertFalse(
            QgsTextRenderer.textRequiresWrapping(
                context, "a test of word wrap", 500, format
            )
        )

        self.assertEqual(
            QgsTextRenderer.wrappedText(context, "a test of word wrap", 50, format),
            ["a", "test", "of", "word", "wrap"],
        )
        self.assertEqual(
            QgsTextRenderer.wrappedText(context, "a test of word wrap", 200, format),
            ["a test of", "word", "wrap"],
        )
        self.assertEqual(
            QgsTextRenderer.wrappedText(context, "a test of word wrap", 400, format),
            ["a test of word", "wrap"],
        )
        self.assertEqual(
            QgsTextRenderer.wrappedText(context, "a test of word wrap", 500, format),
            ["a test of word wrap"],
        )

        # text height should account for wrapping
        self.assertGreater(
            QgsTextRenderer.textHeight(
                context,
                format,
                ["a test of word wrap"],
                mode=QgsTextRenderer.DrawMode.Rect,
                flags=Qgis.TextRendererFlag.WrapLines,
                maxLineWidth=200,
            ),
            QgsTextRenderer.textHeight(
                context,
                format,
                ["a test of word wrap"],
                mode=QgsTextRenderer.DrawMode.Rect,
            )
            * 2.75,
        )

        self.assertTrue(
            self.checkRender(
                format,
                "text_rect_word_wrap_single_line",
                text=["a test of word wrap"],
                alignment=QgsTextRenderer.HAlignment.AlignLeft,
                rect=QRectF(100, 100, 200, 100),
                flags=Qgis.TextRendererFlag.WrapLines,
            )
        )

    def testWordWrapSingleLineStabilityAtSmallScaling(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)

        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 50, 50))
        painter = QPainter()
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setFlag(
            QgsRenderContext.Flag.ApplyScalingWorkaroundForTextRendering, True
        )

        for i in range(1, 3000, 5):
            adjustment = i / 100
            context.setScaleFactor(96 / 25.4 * adjustment)

            self.assertEqual(
                QgsTextRenderer.wrappedText(
                    context, "a test of word wrap", 50 * adjustment, format
                ),
                ["a", "test", "of", "word", "wrap"],
            )
            self.assertEqual(
                QgsTextRenderer.wrappedText(
                    context, "a test of word wrap", 200 * adjustment, format
                ),
                ["a test of", "word", "wrap"],
            )
            self.assertEqual(
                QgsTextRenderer.wrappedText(
                    context, "a test of word wrap", 400 * adjustment, format
                ),
                ["a test of word", "wrap"],
            )
            self.assertEqual(
                QgsTextRenderer.wrappedText(
                    context, "a test of word wrap", 500 * adjustment, format
                ),
                ["a test of word wrap"],
            )

        format.setSize(60)
        for i in range(1, 3000, 5):
            adjustment = i / 100
            context.setScaleFactor(96 / 25.4 * adjustment)

            self.assertEqual(
                QgsTextRenderer.wrappedText(
                    context, "a test of word wrap", 50 * adjustment, format
                ),
                ["a", "test", "of", "word", "wrap"],
            )
            self.assertEqual(
                QgsTextRenderer.wrappedText(
                    context, "a test of word wrap", 200 * adjustment, format
                ),
                ["a", "test", "of", "word", "wrap"],
            )
            self.assertEqual(
                QgsTextRenderer.wrappedText(
                    context, "a test of word wrap", 400 * adjustment, format
                ),
                ["a test of", "word", "wrap"],
            )
            self.assertEqual(
                QgsTextRenderer.wrappedText(
                    context, "a test of word wrap", 500 * adjustment, format
                ),
                ["a test of", "word wrap"],
            )
            self.assertEqual(
                QgsTextRenderer.wrappedText(
                    context, "a test of word wrap", 650 * adjustment, format
                ),
                ["a test of word", "wrap"],
            )
            self.assertEqual(
                QgsTextRenderer.wrappedText(
                    context, "a test of word wrap", 900 * adjustment, format
                ),
                ["a test of word wrap"],
            )

        format.setSize(10)
        for i in range(1, 3000, 5):
            adjustment = i / 100
            context.setScaleFactor(96 / 25.4 * adjustment)

            self.assertEqual(
                QgsTextRenderer.wrappedText(
                    context, "a test of word wrap", 10 * adjustment, format
                ),
                ["a", "test", "of", "word", "wrap"],
            )
            self.assertEqual(
                QgsTextRenderer.wrappedText(
                    context, "a test of word wrap", 70 * adjustment, format
                ),
                ["a test of", "word", "wrap"],
            )
            self.assertEqual(
                QgsTextRenderer.wrappedText(
                    context, "a test of word wrap", 120 * adjustment, format
                ),
                ["a test of word", "wrap"],
            )
            self.assertEqual(
                QgsTextRenderer.wrappedText(
                    context, "a test of word wrap", 150 * adjustment, format
                ),
                ["a test of word wrap"],
            )

    def testDrawTextRectWordWrapMultiLine(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        painter = QPainter()
        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 50, 50))
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI
        context.setFlag(
            QgsRenderContext.Flag.ApplyScalingWorkaroundForTextRendering, True
        )

        # text height should account for wrapping
        self.assertGreater(
            QgsTextRenderer.textHeight(
                context,
                format,
                ["a test of word wrap", "with bit more"],
                mode=QgsTextRenderer.DrawMode.Rect,
                flags=Qgis.TextRendererFlag.WrapLines,
                maxLineWidth=200,
            ),
            QgsTextRenderer.textHeight(
                context,
                format,
                ["a test of word wrap with with bit more"],
                mode=QgsTextRenderer.DrawMode.Rect,
            )
            * 4.75,
        )

        self.assertTrue(
            self.checkRender(
                format,
                "text_rect_word_wrap_multi_line",
                text=["a test of word wrap", "with bit more"],
                alignment=QgsTextRenderer.HAlignment.AlignLeft,
                rect=QRectF(100, 100, 200, 100),
                flags=Qgis.TextRendererFlag.WrapLines,
            )
        )

    def testDrawTextRectWordWrapWithJustify(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        self.assertTrue(
            self.checkRender(
                format,
                "text_rect_word_wrap_justify",
                text=["a test of word wrap"],
                alignment=QgsTextRenderer.HAlignment.AlignJustify,
                rect=QRectF(100, 100, 200, 100),
                flags=Qgis.TextRendererFlag.WrapLines,
            )
        )

    def testDrawTextRectWordWrapHtml1(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setAllowHtmlFormatting(True)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        painter = QPainter()
        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 50, 50))
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI
        context.setFlag(
            QgsRenderContext.Flag.ApplyScalingWorkaroundForTextRendering, True
        )

        self.assertTrue(
            self.checkRender(
                format,
                "html_rect_wrapped1",
                text=[
                    'some text <span style="font-size: 60pt">more text</span> and more'
                ],
                alignment=QgsTextRenderer.HAlignment.AlignLeft,
                rect=QRectF(50, 100, 300, 100),
                flags=Qgis.TextRendererFlag.WrapLines,
            )
        )

    def testDrawTextRectWordWrapHtml2(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setAllowHtmlFormatting(True)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        painter = QPainter()
        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 50, 50))
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI
        context.setFlag(
            QgsRenderContext.Flag.ApplyScalingWorkaroundForTextRendering, True
        )

        self.assertTrue(
            self.checkRender(
                format,
                "html_rect_wrapped2",
                text=[
                    'thiswordistoolong but <span style="font-size: 60pt">this is</span> not'
                ],
                alignment=QgsTextRenderer.HAlignment.AlignLeft,
                rect=QRectF(50, 100, 300, 100),
                flags=Qgis.TextRendererFlag.WrapLines,
            )
        )

    def testDrawTextRectWordWrapHtmlImage1(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setAllowHtmlFormatting(True)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        painter = QPainter()
        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 50, 50))
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI
        context.setFlag(
            QgsRenderContext.Flag.ApplyScalingWorkaroundForTextRendering, True
        )

        self.assertTrue(
            self.checkRender(
                format,
                "html_img_wrapping",
                text=[
                    f'this img <img src="{unitTestDataPath()}/small_sample_image.png" width="80" height="50"> should wrap'
                ],
                alignment=QgsTextRenderer.HAlignment.AlignLeft,
                rect=QRectF(50, 130, 300, 100),
                flags=Qgis.TextRendererFlag.WrapLines,
            )
        )

    def testDrawTextRectWordWrapTab(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setAllowHtmlFormatting(True)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setTabStopDistance(5)
        painter = QPainter()
        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 50, 50))
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI
        context.setFlag(
            QgsRenderContext.Flag.ApplyScalingWorkaroundForTextRendering, True
        )

        self.assertTrue(
            self.checkRender(
                format,
                "tab_wrapping",
                text=["this\ttab\tshould\twrap"],
                alignment=QgsTextRenderer.HAlignment.AlignLeft,
                rect=QRectF(50, 130, 350, 100),
                flags=Qgis.TextRendererFlag.WrapLines,
            )
        )

    def testDrawTextRectWordWrapTabPositions(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setAllowHtmlFormatting(True)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setTabPositions([QgsTextFormat.Tab(5), QgsTextFormat.Tab(8)])
        format.setTabStopDistance(5)
        painter = QPainter()
        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 50, 50))
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI
        context.setFlag(
            QgsRenderContext.Flag.ApplyScalingWorkaroundForTextRendering, True
        )

        self.assertTrue(
            self.checkRender(
                format,
                "tab_wrapping",
                text=["this\ttab\tshould\twrap"],
                alignment=QgsTextRenderer.HAlignment.AlignLeft,
                rect=QRectF(50, 130, 350, 100),
                flags=Qgis.TextRendererFlag.WrapLines,
            )
        )

    def testDrawTextRectMultilineBottomAlign(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)

        self.assertTrue(
            self.checkRender(
                format,
                "text_rect_multiline_bottom_aligned",
                text=["test", "bottom", "aligned"],
                alignment=QgsTextRenderer.HAlignment.AlignLeft,
                rect=QRectF(100, 100, 200, 100),
                vAlignment=QgsTextRenderer.VAlignment.AlignBottom,
            )
        )

    def testDrawTextRectBottomAlign(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)

        self.assertTrue(
            self.checkRender(
                format,
                "text_rect_bottom_aligned",
                text=["bottom aligned"],
                alignment=QgsTextRenderer.HAlignment.AlignLeft,
                rect=QRectF(100, 100, 200, 100),
                vAlignment=QgsTextRenderer.VAlignment.AlignBottom,
            )
        )

    def testDrawTextRectMultilineVCenterAlign(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)

        self.assertTrue(
            self.checkRender(
                format,
                "text_rect_multiline_vcenter_aligned",
                text=["test", "center", "aligned"],
                alignment=QgsTextRenderer.HAlignment.AlignLeft,
                rect=QRectF(100, 100, 200, 100),
                vAlignment=QgsTextRenderer.VAlignment.AlignVCenter,
            )
        )

    def testDrawTextRectVCenterAlign(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)

        self.assertTrue(
            self.checkRender(
                format,
                "text_rect_vcenter_aligned",
                text=["center aligned"],
                alignment=QgsTextRenderer.HAlignment.AlignLeft,
                rect=QRectF(100, 100, 200, 100),
                vAlignment=QgsTextRenderer.VAlignment.AlignVCenter,
            )
        )

    def testDrawTextRectMultilineCenterAlign(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        self.assertTrue(
            self.checkRender(
                format,
                "text_rect_multiline_center_aligned",
                text=["test", "c", "aligned"],
                alignment=QgsTextRenderer.HAlignment.AlignCenter,
                rect=QRectF(100, 100, 200, 100),
            )
        )

    def testDrawTextRectCenterAlign(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        self.assertTrue(
            self.checkRender(
                format,
                "text_rect_center_aligned",
                text=["test"],
                alignment=QgsTextRenderer.HAlignment.AlignCenter,
                rect=QRectF(100, 100, 200, 100),
            )
        )

    def testDrawTextPointMultilineRightAlign(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "text_point_right_multiline_aligned",
                text=["test", "right", "aligned"],
                alignment=QgsTextRenderer.HAlignment.AlignRight,
                point=QPointF(300, 200),
            )
        )

    def testDrawTextPointMultilineCenterAlign(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "text_point_center_multiline_aligned",
                text=["test", "center", "aligned"],
                alignment=QgsTextRenderer.HAlignment.AlignCenter,
                point=QPointF(200, 200),
            )
        )

    def testDrawTextPointRightAlign(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "text_point_right_aligned",
                text=["test"],
                alignment=QgsTextRenderer.HAlignment.AlignRight,
                point=QPointF(300, 200),
            )
        )

    def testDrawTextPointJustifyAlign(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "text_point_justify_aligned",
                text=["test"],
                alignment=QgsTextRenderer.HAlignment.AlignJustify,
                point=QPointF(100, 200),
            )
        )

    def testDrawTextPointMultilineJustifyAlign(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "text_point_justify_multiline_aligned",
                text=["a t est", "off", "justification", "align"],
                alignment=QgsTextRenderer.HAlignment.AlignJustify,
                point=QPointF(100, 200),
            )
        )

    def testDrawTextPointCenterAlign(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "text_point_center_aligned",
                text=["test"],
                alignment=QgsTextRenderer.HAlignment.AlignCenter,
                point=QPointF(200, 200),
            )
        )

    def testDrawTextDataDefinedColorPoint(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(0, 255, 0))
        format.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.Color, QgsProperty.fromExpression("'#bb00cc'")
        )
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "text_dd_color_point",
                None,
                text=["test"],
                point=QPointF(50, 200),
            )
        )

    def testDrawTextDataDefinedColorRect(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(0, 255, 0))
        format.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.Color, QgsProperty.fromExpression("'#bb00cc'")
        )
        self.assertTrue(
            self.checkRender(
                format,
                "text_dd_color_rect",
                None,
                text=["test"],
                alignment=QgsTextRenderer.HAlignment.AlignCenter,
                rect=QRectF(100, 100, 100, 100),
            )
        )

    def testDrawTextDataDefinedBufferColorPoint(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(0, 255, 0))
        format.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.BufferColor,
            QgsProperty.fromExpression("'#bb00cc'"),
        )
        format.buffer().setEnabled(True)
        format.buffer().setSize(5)
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "text_dd_buffer_color",
                None,
                text=["test"],
                point=QPointF(50, 200),
            )
        )

    def testDrawTabPercent(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(20)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setTabStopDistance(4)
        format.setTabStopDistanceUnit(Qgis.RenderUnit.Percentage)
        self.assertTrue(
            self.checkRender(format, "text_tab_percentage", text=["with\ttabs", "a\tb"])
        )

    def testDrawTabPositionsPercent(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(20)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setTabPositions([QgsTextFormat.Tab(3.1), QgsTextFormat.Tab(8)])
        format.setTabStopDistanceUnit(Qgis.RenderUnit.Percentage)
        self.assertTrue(
            self.checkRender(
                format,
                "text_tab_positions_percentage",
                text=["with\tmany\ttabs", "a\tb\tc"],
            )
        )

    def testDrawTabFixedSize(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(20)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setTabStopDistance(40)
        format.setTabStopDistanceUnit(Qgis.RenderUnit.Millimeters)
        self.assertTrue(
            self.checkRender(format, "text_tab_fixed_size", text=["with\ttabs", "a\tb"])
        )

    def testDrawTabPositionsFixedSize(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(20)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setTabPositions([QgsTextFormat.Tab(20), QgsTextFormat.Tab(50)])
        format.setTabStopDistanceUnit(Qgis.RenderUnit.Millimeters)
        self.assertTrue(
            self.checkRender(
                format,
                "text_tab_positions_fixed_size",
                text=["with\tmany\ttabs", "a\tb\tc"],
            )
        )

    def testDrawTabPositionsFixedSizeMoreTabsThanPositions(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(20)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setTabPositions([QgsTextFormat.Tab(20), QgsTextFormat.Tab(50)])
        format.setTabStopDistanceUnit(Qgis.RenderUnit.Millimeters)
        self.assertTrue(
            self.checkRender(
                format,
                "text_tab_positions_fixed_size_more_tabs",
                text=["with\tmany\ttabs", "a\tb\tc\td\te"],
            )
        )

    def testDrawTabPositionsFixedSizeLongText(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(20)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setTabPositions([QgsTextFormat.Tab(15), QgsTextFormat.Tab(50)])
        format.setTabStopDistanceUnit(Qgis.RenderUnit.Millimeters)
        self.assertTrue(
            self.checkRender(
                format,
                "text_tab_positions_fixed_size_long_text",
                text=["with long\ttext\t2", "a\tb\tc"],
            )
        )

    def testHtmlFormatting(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(0, 255, 0))
        format.setAllowHtmlFormatting(True)
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "text_html_formatting",
                None,
                text=[
                    '<s>t</s><span style="text-decoration: overline">e</span><span style="color: red">s<span style="color: rgba(255,0,0,0.5); text-decoration: underline">t</span></span>'
                ],
                point=QPointF(50, 200),
            )
        )

    def testHtmlTabPercent(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(20)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setTabStopDistance(4)
        format.setTabStopDistanceUnit(Qgis.RenderUnit.Percentage)
        format.setAllowHtmlFormatting(True)
        self.assertTrue(
            self.checkRender(
                format,
                "text_tab_percentage_html",
                text=['<span style="font-size: 15pt">with</span>\ttabs', " a\tb"],
            )
        )

    def testHtmlTabPositionsPercent(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(20)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setTabPositions([QgsTextFormat.Tab(3), QgsTextFormat.Tab(8)])
        format.setTabStopDistanceUnit(Qgis.RenderUnit.Percentage)
        format.setAllowHtmlFormatting(True)
        self.assertTrue(
            self.checkRender(
                format,
                "text_tab_positions_percentage_html",
                text=[
                    '<span style="font-size: 15pt">with</span>\tmany\ttabs',
                    " a\tb\tc",
                ],
            )
        )

    def testHtmlTabFixedSize(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(20)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setTabStopDistance(40)
        format.setTabStopDistanceUnit(Qgis.RenderUnit.Millimeters)
        format.setAllowHtmlFormatting(True)
        self.assertTrue(
            self.checkRender(
                format,
                "text_tab_fixed_size_html",
                text=['<span style="font-size: 15pt">with</span>\ttabs', " a\tb"],
            )
        )

    def testHtmlTabPositionsFixedSize(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(20)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setTabPositions([QgsTextFormat.Tab(25), QgsTextFormat.Tab(60)])
        format.setTabStopDistanceUnit(Qgis.RenderUnit.Millimeters)
        format.setAllowHtmlFormatting(True)
        self.assertTrue(
            self.checkRender(
                format,
                "text_tab_positions_fixed_size_html",
                text=[
                    '<span style="font-size: 15pt">with</span>\tmany\ttabs',
                    " a\tb\tc",
                ],
            )
        )

    def testHtmlFormattingBuffer(self):
        """
        Test drawing HTML with buffer
        """
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(0, 255, 0))
        format.setAllowHtmlFormatting(True)
        format.buffer().setEnabled(True)
        format.buffer().setSize(5)
        format.buffer().setColor(QColor(50, 150, 200))
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "text_html_formatting_buffer",
                None,
                text=[
                    '<s>t</s><span style="text-decoration: overline">e</span><span style="color: red">s<span style="text-decoration: underline">t</span></span>'
                ],
                point=QPointF(50, 200),
            )
        )

    def testHtmlFormattingBufferScaleFactor(self):
        """
        Test drawing HTML with scale factor workaround
        """
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        # font sizes < 50 pixel trigger the scale factor workaround
        format.setSize(49)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPixels)
        format.setColor(QColor(0, 255, 0))
        format.setAllowHtmlFormatting(True)
        format.buffer().setEnabled(True)
        format.buffer().setSize(5)
        format.buffer().setColor(QColor(50, 150, 200))
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "text_html_formatting_buffer_scale_workaround",
                None,
                text=[
                    't <span style="font-size:60pt">e</span> <span style="color: red">s</span>'
                ],
                point=QPointF(50, 200),
                enable_scale_workaround=True,
            )
        )

    def testHtmlFormattingMask(self):
        """
        Test drawing HTML with mask
        """
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(0, 255, 0))
        format.setAllowHtmlFormatting(True)
        format.mask().setEnabled(True)
        format.mask().setSize(5)
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "text_html_formatting_mask",
                None,
                text=[
                    't <span style="font-size:60pt">e</span> <span style="color: red">s</span>'
                ],
                point=QPointF(50, 200),
                render_mask=True,
            )
        )

    def testHtmlFormattingMaskScaleFactor(self):
        """
        Test drawing HTML with mask with scale factor workaround
        """
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        # font sizes < 50 pixel trigger the scale factor workaround
        format.setSize(49)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPixels)
        format.setColor(QColor(0, 255, 0))
        format.setAllowHtmlFormatting(True)
        format.mask().setEnabled(True)
        format.mask().setSize(5)
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "text_html_formatting_mask_scale_workaround",
                None,
                text=[
                    't <span style="font-size:60pt">e</span> <span style="color: red">s</span>'
                ],
                point=QPointF(50, 200),
                render_mask=True,
                enable_scale_workaround=True,
            )
        )

    def testHtmlFormattingShadow(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(0, 255, 0))
        format.setAllowHtmlFormatting(True)
        format.shadow().setEnabled(True)
        format.shadow().setOffsetDistance(5)
        format.shadow().setBlurRadius(0)
        format.shadow().setColor(QColor(50, 150, 200))
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "text_html_formatting_shadow",
                None,
                text=[
                    '<s>t</s><span style="text-decoration: overline">e</span><span style="color: red">s<span style="text-decoration: underline">t</span></span>'
                ],
                point=QPointF(50, 200),
            )
        )

    def testHtmlFormattingBufferShadow(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(0, 255, 0))
        format.setAllowHtmlFormatting(True)
        format.buffer().setEnabled(True)
        format.buffer().setSize(5)
        format.buffer().setColor(QColor(200, 50, 150))
        format.shadow().setEnabled(True)
        format.shadow().setOffsetDistance(5)
        format.shadow().setBlurRadius(0)
        format.shadow().setColor(QColor(50, 150, 200))
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "text_html_formatting_buffer_shadow",
                None,
                text=[
                    '<s>t</s><span style="text-decoration: overline">e</span><span style="color: red">s<span style="text-decoration: underline">t</span></span>'
                ],
                point=QPointF(50, 200),
            )
        )

    def testHtmlFormattingVertical(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(0, 255, 0))
        format.setAllowHtmlFormatting(True)
        format.setOrientation(QgsTextFormat.TextOrientation.VerticalOrientation)
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "text_html_formatting_vertical",
                None,
                text=[
                    '<s>t</s><span style="text-decoration: overline">e</span><span style="color: red">s<span style="text-decoration: underline">t</span></span>'
                ],
                point=QPointF(50, 200),
            )
        )

    def testHtmlFormattingBufferVertical(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(0, 255, 0))
        format.setAllowHtmlFormatting(True)
        format.buffer().setEnabled(True)
        format.buffer().setSize(5)
        format.buffer().setColor(QColor(50, 150, 200))
        format.setOrientation(QgsTextFormat.TextOrientation.VerticalOrientation)
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "text_html_formatting_buffer_vertical",
                None,
                text=[
                    '<s>t</s><span style="text-decoration: overline">e</span><span style="color: red">s<span style="text-decoration: underline">t</span></span>'
                ],
                point=QPointF(50, 200),
            )
        )

    def testHtmlMixedMetricFormatting(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("regular"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(0, 255, 0))
        format.setAllowHtmlFormatting(True)
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "text_html_mixed_metric_formatting",
                None,
                text=[
                    '<i>t</i><b style="font-size: 30pt">e</b><p><span style="color: red">s<span style="color: rgba(255,0,0,0.5); text-decoration: underline; font-size:80pt">t</span></span>'
                ],
                point=QPointF(50, 200),
            )
        )

    def testHtmlMixedMetricLineHeight(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("regular"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(0, 255, 0))
        format.setAllowHtmlFormatting(True)
        format.setLineHeight(0.5)
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "text_html_mixed_metric_formatting_line_height",
                None,
                text=[
                    '<i>t</i><b style="font-size: 30pt">e</b><p><span style="color: red">s<span style="color: rgba(255,0,0,0.5); text-decoration: underline; font-size:80pt">t</span></span>'
                ],
                point=QPointF(50, 200),
            )
        )

    def testHtmlMixedMetricLineHeightCss(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("regular"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(0, 0, 0))
        format.setAllowHtmlFormatting(True)
        format.setLineHeight(0.5)
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "text_html_mixed_metric_formatting_line_height_css",
                None,
                text=[
                    '<i>t</i><b style="font-size: 30pt">e</b><p style="line-height: 75pt"><span style="color: red">s<span style="color: rgba(255,0,0,0.5); font-size:80pt">t</span></span></p><p style="line-height: 50%">third line</p>'
                ],
                point=QPointF(50, 300),
            )
        )

    def testHtmlMixedMetricFormattingBuffer(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(0, 255, 0))
        format.setAllowHtmlFormatting(True)
        format.buffer().setEnabled(True)
        format.buffer().setSize(5)
        format.buffer().setColor(QColor(50, 150, 200))
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "text_html_mixed_metric_formatting_buffer",
                None,
                text=[
                    '<i>t</i><b style="font-size: 30pt">e</b><p><span style="color: red">s<span style="color: rgba(255,0,0,0.5); text-decoration: underline; font-size:80pt">t</span></span>'
                ],
                point=QPointF(50, 200),
            )
        )

    def testHtmlMixedMetricFormattingShadow(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(0, 255, 0))
        format.setAllowHtmlFormatting(True)
        format.shadow().setEnabled(True)
        format.shadow().setOffsetDistance(5)
        format.shadow().setBlurRadius(0)
        format.shadow().setColor(QColor(50, 150, 200))
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "text_html_mixed_metric_formatting_shadow",
                None,
                text=[
                    '<i>t</i><b style="font-size: 30pt">e</b><p><span style="color: red">s<span style="color: rgba(255,0,0,0.5); text-decoration: underline; font-size:80pt">t</span></span>'
                ],
                point=QPointF(50, 200),
            )
        )

    def testHtmlMixedMetricFormattingBufferShadow(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(0, 255, 0))
        format.setAllowHtmlFormatting(True)
        format.buffer().setEnabled(True)
        format.buffer().setSize(5)
        format.buffer().setColor(QColor(200, 50, 150))
        format.shadow().setEnabled(True)
        format.shadow().setOffsetDistance(5)
        format.shadow().setBlurRadius(0)
        format.shadow().setColor(QColor(50, 150, 200))
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "text_html_mixed_metric_formatting_buffer_shadow",
                None,
                text=[
                    '<i>t</i><b style="font-size: 30pt">e</b><p><span style="color: red">s<span style="color: rgba(255,0,0,0.5); text-decoration: underline; font-size:80pt">t</span></span>'
                ],
                point=QPointF(50, 200),
            )
        )

    def testHtmlMixedMetricFormattingVertical(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(0, 255, 0))
        format.setAllowHtmlFormatting(True)
        format.setOrientation(QgsTextFormat.TextOrientation.VerticalOrientation)
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "text_html_mixed_metric_formatting_vertical",
                None,
                text=[
                    '<i>t</i><b style="font-size: 30pt">e</b><p><span style="color: red">s<span style="color: rgba(255,0,0,0.5); text-decoration: underline; font-size:80pt">t</span></span>'
                ],
                point=QPointF(50, 200),
            )
        )

    def testHtmlMixedMetricFormattingBufferVertical(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(0, 255, 0))
        format.setAllowHtmlFormatting(True)
        format.buffer().setEnabled(True)
        format.buffer().setSize(5)
        format.buffer().setColor(QColor(50, 150, 200))
        format.setOrientation(QgsTextFormat.TextOrientation.VerticalOrientation)
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "text_html_mixed_metric_formatting_buffer_vertical",
                None,
                text=[
                    '<i>t</i><b style="font-size: 30pt">e</b><p><span style="color: red">s<span style="color: rgba(255,0,0,0.5); text-decoration: underline; font-size:80pt">t</span></span>'
                ],
                point=QPointF(50, 200),
            )
        )

    def testHtmlMixedMetricFormattingShadowVertical(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(0, 255, 0))
        format.setAllowHtmlFormatting(True)
        format.shadow().setEnabled(True)
        format.shadow().setOffsetDistance(5)
        format.shadow().setBlurRadius(0)
        format.shadow().setColor(QColor(50, 150, 200))
        format.setOrientation(QgsTextFormat.TextOrientation.VerticalOrientation)
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "text_html_mixed_metric_formatting_shadow_vertical",
                None,
                text=[
                    '<i>t</i><b style="font-size: 30pt">e</b><p><span style="color: red">s<span style="color: rgba(255,0,0,0.5); text-decoration: underline; font-size:80pt">t</span></span>'
                ],
                point=QPointF(50, 200),
            )
        )

    def testHtmlMixedMetricFormattingBufferShadowVertical(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(0, 255, 0))
        format.setAllowHtmlFormatting(True)
        format.buffer().setEnabled(True)
        format.buffer().setSize(5)
        format.buffer().setColor(QColor(200, 50, 150))
        format.shadow().setEnabled(True)
        format.shadow().setOffsetDistance(5)
        format.shadow().setBlurRadius(0)
        format.shadow().setColor(QColor(50, 150, 200))
        format.setOrientation(QgsTextFormat.TextOrientation.VerticalOrientation)
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "text_html_mixed_metric_formatting_buffer_shadow_vertical",
                None,
                text=[
                    '<i>t</i><b style="font-size: 30pt">e</b><p><span style="color: red">s<span style="color: rgba(255,0,0,0.5); text-decoration: underline; font-size:80pt">t</span></span>'
                ],
                point=QPointF(50, 200),
            )
        )

    def testHtmlHeadings(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(255, 0, 0))
        format.setAllowHtmlFormatting(True)
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "html_headings",
                None,
                text=[
                    "<h1>h1</h1><h2>h2</h2><h3>h3</h3><h4>h4</h4><h5>h5</h5><h6>h6</h6>"
                ],
                point=QPointF(10, 300),
            )
        )

    def testHtmlHeadingsLargerFont(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(40)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(255, 0, 0))
        format.setAllowHtmlFormatting(True)
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "html_headings_larger",
                None,
                text=[
                    "<h1>h1</h1><h2>h2</h2><h3>h3</h3><h4>h4</h4><h5>h5</h5><h6>h6</h6>"
                ],
                point=QPointF(10, 350),
            )
        )

    def testHtmlAlignmentLeftBase(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(255, 0, 0))
        format.setAllowHtmlFormatting(True)
        self.assertTrue(
            self.checkRender(
                format,
                "html_align_rect_left_base",
                None,
                text=[
                    '<p>Test some text</p><p>Short</p><p style="text-align: right">test</p><p align="center">test</p><center>center</center>'
                ],
                rect=QRectF(10, 10, 300, 300),
            )
        )

    def testHtmlAlignmentRightBase(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(255, 0, 0))
        format.setAllowHtmlFormatting(True)
        self.assertTrue(
            self.checkRender(
                format,
                "html_align_rect_right_base",
                None,
                text=[
                    '<p>Test some text</p><p>Short</p><p style="text-align: right">test</p><p align="center">test</p><center>center</center>'
                ],
                rect=QRectF(10, 10, 300, 300),
                alignment=Qgis.TextHorizontalAlignment.Right,
            )
        )

    def testHtmlAlignmentCenterBase(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(255, 0, 0))
        format.setAllowHtmlFormatting(True)
        self.assertTrue(
            self.checkRender(
                format,
                "html_align_rect_center_base",
                None,
                text=[
                    '<p>Test some text</p><p>Short</p><p style="text-align: right">test</p><p align="left">test</p><center>center</center>'
                ],
                rect=QRectF(10, 10, 300, 300),
                alignment=Qgis.TextHorizontalAlignment.Center,
            )
        )

    def testHtmlMarginsRectLeftBase(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(255, 0, 0))
        format.setAllowHtmlFormatting(True)
        self.assertTrue(
            self.checkRender(
                format,
                "margins_rect_left",
                None,
                text=[
                    '<p style="margin: 15pt 20pt 20pt 25pt">Test some text</p><p style="margin: 35pt 40pt 10pt 35pt">Short</p><p style="text-align: right; margin: 5pt 20pt 10pt 35pt">test</p><p align="center" style="margin: 5pt 20pt 10pt 35pt">test</p><center>center</center>'
                ],
                rect=QRectF(10, 10, 300, 300),
            )
        )

    def testHtmlMarginsRectJustifyBase(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(255, 0, 0))
        format.setAllowHtmlFormatting(True)
        self.assertTrue(
            self.checkRender(
                format,
                "margins_rect_justify",
                None,
                alignment=QgsTextRenderer.HAlignment.AlignJustify,
                text=[
                    '<p style="margin: 15pt 20pt 20pt 25pt">Test some text</p><p style="margin: 35pt 40pt 10pt 35pt">Short</p><p style="text-align: right; margin: 5pt 20pt 10pt 35pt">test</p><p align="center" style="margin: 5pt 20pt 10pt 35pt">test</p><center>center</center>'
                ],
                rect=QRectF(10, 10, 300, 300),
            )
        )

    def testHtmlMarginsRectRightBase(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(255, 0, 0))
        format.setAllowHtmlFormatting(True)
        self.assertTrue(
            self.checkRender(
                format,
                "margins_rect_right",
                None,
                alignment=QgsTextRenderer.HAlignment.AlignRight,
                text=[
                    '<p style="margin: 15pt 20pt 20pt 25pt">Test some text</p><p style="margin: 35pt 40pt 10pt 35pt">Short</p><p style="text-align: right; margin: 5pt 20pt 10pt 35pt">test</p><p align="center" style="margin: 5pt 20pt 10pt 35pt">test</p><center>center</center>'
                ],
                rect=QRectF(10, 10, 350, 300),
            )
        )

    def testHtmlMarginsRectCenterBase(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(255, 0, 0))
        format.setAllowHtmlFormatting(True)
        self.assertTrue(
            self.checkRender(
                format,
                "margins_rect_center",
                None,
                alignment=QgsTextRenderer.HAlignment.AlignCenter,
                text=[
                    '<p style="margin: 15pt 20pt 20pt 25pt">Test some text</p><p style="margin: 35pt 40pt 10pt 35pt">Short</p><p style="text-align: right; margin: 5pt 20pt 10pt 35pt">test</p><p align="center" style="margin: 5pt 20pt 10pt 35pt">test</p><center>center</center>'
                ],
                rect=QRectF(10, 10, 350, 300),
            )
        )

    def testHtmlMarginsPointLeftBase(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(255, 0, 0))
        format.setAllowHtmlFormatting(True)
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "margins_point_left",
                None,
                text=[
                    '<p style="margin: 15pt 20pt 20pt 25pt">Test some text</p><p style="margin: 35pt 40pt 10pt 35pt">Short</p><p style="margin: 5pt 20pt 10pt 35pt">test</p><p style="margin: 5pt 20pt 10pt 35pt">test</p>'
                ],
                point=QPointF(10, 350),
            )
        )

    def testHtmlMarginsPointJustifyBase(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(255, 0, 0))
        format.setAllowHtmlFormatting(True)
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "margins_point_justify",
                None,
                alignment=QgsTextRenderer.HAlignment.AlignJustify,
                text=[
                    '<p style="margin: 15pt 20pt 20pt 25pt">Test some text</p><p style="margin: 35pt 40pt 10pt 35pt">Short</p><p style="margin: 5pt 20pt 10pt 35pt">test</p><p style="margin: 5pt 20pt 10pt 35pt">test</p>'
                ],
                point=QPointF(10, 350),
            )
        )

    def testHtmlMarginsPointRightBase(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(255, 0, 0))
        format.setAllowHtmlFormatting(True)
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "margins_point_right",
                None,
                alignment=QgsTextRenderer.HAlignment.AlignRight,
                text=[
                    '<p style="margin: 15pt 20pt 20pt 25pt">Test some text</p><p style="margin: 35pt 40pt 10pt 35pt">Short</p><p style="margin: 5pt 20pt 10pt 35pt">test</p><p style="margin: 5pt 20pt 10pt 35pt">test</p>'
                ],
                point=QPointF(400, 350),
            )
        )

    def testHtmlMarginsPointCenterBase(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(255, 0, 0))
        format.setAllowHtmlFormatting(True)
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "margins_point_center",
                None,
                alignment=QgsTextRenderer.HAlignment.AlignCenter,
                text=[
                    '<p style="margin: 15pt 20pt 20pt 25pt">Test some text</p><p style="margin: 35pt 40pt 10pt 35pt">Short</p><p style="margin: 5pt 20pt 10pt 35pt">test</p><p style="margin: 5pt 20pt 10pt 35pt">test</p>'
                ],
                point=QPointF(200, 350),
            )
        )

    def testHtmlMarginsRectMiddleBase(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(255, 0, 0))
        format.setAllowHtmlFormatting(True)
        self.assertTrue(
            self.checkRender(
                format,
                "margins_rect_middle",
                None,
                vAlignment=QgsTextRenderer.VAlignment.AlignVCenter,
                text=[
                    '<p style="margin: 15pt 20pt 20pt 25pt">Test some text</p><p style="margin: 35pt 40pt 10pt 35pt">Short</p><p style="text-align: right; margin: 5pt 20pt 10pt 35pt">test</p><p align="center" style="margin: 5pt 20pt 10pt 35pt">test</p><center>center</center>'
                ],
                rect=QRectF(10, 10, 300, 300),
            )
        )

    def testHtmlMarginsRectBottomBase(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(255, 0, 0))
        format.setAllowHtmlFormatting(True)
        self.assertTrue(
            self.checkRender(
                format,
                "margins_rect_bottom",
                None,
                vAlignment=QgsTextRenderer.VAlignment.AlignBottom,
                text=[
                    '<p style="margin: 15pt 20pt 20pt 25pt">Test some text</p><p style="margin: 35pt 40pt 10pt 35pt">Short</p><p style="text-align: right; margin: 5pt 20pt 10pt 35pt">test</p><p align="center" style="margin: 5pt 20pt 10pt 35pt">test</p><center>center</center>'
                ],
                rect=QRectF(10, 10, 300, 300),
            )
        )

    def testHtmlImageAutoSize(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(255, 0, 0))
        format.setAllowHtmlFormatting(True)
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(0, 0))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeBuffer)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        format.background().setFillColor(QColor(255, 255, 255))

        self.assertTrue(
            self.checkRender(
                format,
                "image_autosize",
                None,
                text=[
                    f'<p>Test <img src="{unitTestDataPath()}/small_sample_image.png">test</p>'
                ],
                rect=QRectF(10, 10, 300, 300),
                alignment=Qgis.TextHorizontalAlignment.Center,
            )
        )

    def testHtmlImageAutoWidth(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(255, 0, 0))
        format.setAllowHtmlFormatting(True)
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(0, 0))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeBuffer)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        format.background().setFillColor(QColor(255, 255, 255))

        self.assertTrue(
            self.checkRender(
                format,
                "image_autowidth",
                None,
                text=[
                    f'<p>Test <img src="{unitTestDataPath()}/small_sample_image.png" height="80">test</p>'
                ],
                rect=QRectF(10, 10, 300, 300),
                alignment=Qgis.TextHorizontalAlignment.Center,
            )
        )

    def testHtmlImageAutoHeight(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(255, 0, 0))
        format.setAllowHtmlFormatting(True)
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(0, 0))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeBuffer)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        format.background().setFillColor(QColor(255, 255, 255))

        self.assertTrue(
            self.checkRender(
                format,
                "image_autoheight",
                None,
                text=[
                    f'<p>Test <img src="{unitTestDataPath()}/small_sample_image.png" width="80">test</p>'
                ],
                rect=QRectF(10, 10, 300, 300),
                alignment=Qgis.TextHorizontalAlignment.Center,
            )
        )

    def testHtmlImageFixedSize(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(255, 0, 0))
        format.setAllowHtmlFormatting(True)
        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(0, 0))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeBuffer)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        format.background().setFillColor(QColor(255, 255, 255))

        self.assertTrue(
            self.checkRender(
                format,
                "image_fixed_size",
                None,
                text=[
                    f'<p>Test <img src="{unitTestDataPath()}/small_sample_image.png" width="80" height="200">test</p>'
                ],
                rect=QRectF(10, 10, 300, 300),
                alignment=Qgis.TextHorizontalAlignment.Center,
            )
        )

    def testHtmlSuperSubscript(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(255, 0, 0))
        format.setAllowHtmlFormatting(True)
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "text_html_supersubscript",
                None,
                text=["<sub>sub</sub>N<sup>sup</sup>"],
                point=QPointF(50, 200),
            )
        )

    def testHtmlSuperSubscriptFixedFontSize(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(255, 0, 0))
        format.setAllowHtmlFormatting(True)
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "text_html_supersubscript_fixed_font_size",
                None,
                text=[
                    '<sub style="font-size:80pt">s<span style="font-size:30pt">u</span></sub>N<sup style="font-size:40pt">s<span style="font-size: 20pt">up</span></sup>'
                ],
                point=QPointF(50, 200),
            )
        )

    def testHtmlSuperSubscriptBuffer(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(255, 0, 0))
        format.setAllowHtmlFormatting(True)
        format.buffer().setEnabled(True)
        format.buffer().setSize(5)
        format.buffer().setColor(QColor(50, 150, 200))
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "text_html_supersubscript_buffer",
                None,
                text=["<sub>sub</sub>N<sup>sup</sup>"],
                point=QPointF(50, 200),
            )
        )

    def testHtmlSuperSubscriptShadow(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(0, 255, 0))
        format.setAllowHtmlFormatting(True)
        format.shadow().setEnabled(True)
        format.shadow().setOffsetDistance(5)
        format.shadow().setBlurRadius(0)
        format.shadow().setColor(QColor(50, 150, 200))
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "text_html_supersubscript_shadow",
                None,
                text=["<sub>sub</sub>N<sup>sup</sup>"],
                point=QPointF(50, 200),
            )
        )

    def testHtmlSuperSubscriptBufferShadow(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(0, 255, 0))
        format.setAllowHtmlFormatting(True)
        format.buffer().setEnabled(True)
        format.buffer().setSize(5)
        format.buffer().setColor(QColor(200, 50, 150))
        format.shadow().setEnabled(True)
        format.shadow().setOffsetDistance(5)
        format.shadow().setBlurRadius(0)
        format.shadow().setColor(QColor(50, 150, 200))
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "text_html_supersubscript_buffer_shadow",
                None,
                text=["<sub>sub</sub>N<sup>sup</sup>"],
                point=QPointF(50, 200),
            )
        )

    def testHtmlWordSpacing(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(255, 0, 0))
        format.setAllowHtmlFormatting(True)
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "html_word_spacing",
                None,
                text=['test of <span style="word-spacing: 20.5">wo space</span>'],
                point=QPointF(10, 200),
            )
        )

    def testHtmlWordSpacingPx(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(255, 0, 0))
        format.setAllowHtmlFormatting(True)
        # unit should be ignored, we always treat it as pt as pixels don't
        # scale
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "html_word_spacing",
                None,
                text=['test of <span style="word-spacing: 20.5px">wo space</span>'],
                point=QPointF(10, 200),
            )
        )

    def testHtmlWordSpacingNegative(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setColor(QColor(255, 0, 0))
        format.setAllowHtmlFormatting(True)
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "html_word_spacing_negative",
                None,
                text=['test of <span style="word-spacing: -20.5">wo space</span>'],
                point=QPointF(10, 200),
            )
        )

    def testTextRenderFormat(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(30)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)

        filename = f"{QDir.tempPath()}/test_render_text.svg"
        svg = QSvgGenerator()
        svg.setFileName(filename)
        svg.setSize(QSize(400, 400))
        svg.setResolution(600)

        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 50, 50))
        ms.setOutputSize(QSize(400, 400))
        context = QgsRenderContext.fromMapSettings(ms)

        # test with ALWAYS TEXT mode
        context.setTextRenderFormat(
            QgsRenderContext.TextRenderFormat.TextFormatAlwaysText
        )
        painter = QPainter()
        context.setPainter(painter)

        context.setScaleFactor(96 / 25.4)  # 96 DPI

        painter.begin(svg)

        painter.setBrush(QBrush(QColor(182, 239, 255)))
        painter.setPen(Qt.PenStyle.NoPen)

        QgsTextRenderer.drawText(
            QPointF(0, 30),
            0,
            QgsTextRenderer.HAlignment.AlignLeft,
            ["my test text"],
            context,
            format,
        )

        painter.end()

        # expect svg to contain a text object with the label
        with open(filename) as f:
            lines = "".join(f.readlines())
        self.assertIn("<text", lines)
        self.assertIn(">my test text<", lines)

        os.unlink(filename)

        # test with ALWAYS CURVES mode
        context = QgsRenderContext.fromMapSettings(ms)
        context.setTextRenderFormat(
            QgsRenderContext.TextRenderFormat.TextFormatAlwaysOutlines
        )
        painter = QPainter()
        context.setPainter(painter)

        context.setScaleFactor(96 / 25.4)  # 96 DPI

        svg = QSvgGenerator()
        svg.setFileName(filename)
        svg.setSize(QSize(400, 400))
        svg.setResolution(600)
        painter.begin(svg)

        painter.setBrush(QBrush(QColor(182, 239, 255)))
        painter.setPen(Qt.PenStyle.NoPen)

        QgsTextRenderer.drawText(
            QPointF(0, 30),
            0,
            QgsTextRenderer.HAlignment.AlignLeft,
            ["my test text"],
            context,
            format,
        )

        painter.end()

        # expect svg to contain a text object with the label
        with open(filename) as f:
            lines = "".join(f.readlines())
        self.assertNotIn("<text", lines)
        self.assertNotIn(">my test text<", lines)

    def testDrawTextVerticalRectMode(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setOrientation(QgsTextFormat.TextOrientation.VerticalOrientation)
        self.assertTrue(
            self.checkRender(
                format,
                "text_vertical_rect_mode",
                QgsTextRenderer.TextPart.Text,
                text=["1234"],
                rect=QRectF(40, 20, 350, 350),
            )
        )

    def testDrawTextVerticalRectModeCenterAligned(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setOrientation(QgsTextFormat.TextOrientation.VerticalOrientation)
        self.assertTrue(
            self.checkRender(
                format,
                "text_vertical_rect_mode_center_aligned",
                QgsTextRenderer.TextPart.Text,
                text=["1234", "5678"],
                rect=QRectF(40, 20, 350, 350),
                alignment=QgsTextRenderer.HAlignment.AlignCenter,
            )
        )

    def testDrawTextVerticalRectModeRightAligned(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setOrientation(QgsTextFormat.TextOrientation.VerticalOrientation)
        self.assertTrue(
            self.checkRender(
                format,
                "text_vertical_rect_mode_right_aligned",
                QgsTextRenderer.TextPart.Text,
                text=["1234", "5678"],
                rect=QRectF(40, 20, 350, 350),
                alignment=QgsTextRenderer.HAlignment.AlignRight,
            )
        )

    def testDrawTextVerticalPointMode(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(60)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.setOrientation(QgsTextFormat.TextOrientation.VerticalOrientation)
        self.assertTrue(
            self.checkRenderPoint(
                format,
                "text_vertical_point_mode",
                QgsTextRenderer.TextPart.Text,
                text=["1234", "5678"],
                point=QPointF(40, 380),
            )
        )

    def testDrawTextOnLineAtStart(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(16)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)

        image = QImage(400, 400, QImage.Format.Format_RGB32)

        painter = QPainter()
        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 50, 50))
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI
        context.setFlag(
            QgsRenderContext.Flag.ApplyScalingWorkaroundForTextRendering, True
        )

        painter.begin(image)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)
        image.fill(QColor(152, 219, 249))

        painter.setBrush(Qt.BrushStyle.NoBrush)
        painter.setPen(QPen(QColor(0, 0, 0)))

        line = QPolygonF([QPointF(50, 200), QPointF(350, 200)])
        painter.drawPolygon(line)

        painter.setBrush(QBrush(QColor(182, 239, 255)))
        painter.setPen(Qt.PenStyle.NoPen)

        QgsTextRenderer.drawTextOnLine(line, "my curved text", context, format, 0)

        painter.end()
        self.assertTrue(
            self.image_check(
                "text_on_line_at_start",
                "text_on_line_at_start",
                image,
                "text_on_line_at_start",
            )
        )

    def testDrawTextOnLineZeroWidthChar(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(16)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.background().setEnabled(True)

        image = QImage(400, 400, QImage.Format.Format_RGB32)

        painter = QPainter()
        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 50, 50))
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI
        context.setFlag(
            QgsRenderContext.Flag.ApplyScalingWorkaroundForTextRendering, True
        )

        painter.begin(image)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)
        image.fill(QColor(152, 219, 249))

        painter.setBrush(Qt.BrushStyle.NoBrush)
        painter.setPen(QPen(QColor(0, 0, 0)))

        line = QPolygonF([QPointF(50, 200), QPointF(350, 200)])
        painter.drawPolygon(line)

        painter.setBrush(QBrush(QColor(182, 239, 255)))
        painter.setPen(Qt.PenStyle.NoPen)

        QgsTextRenderer.drawTextOnLine(line, "b\r\na", context, format, 0)

        painter.end()
        self.assertTrue(
            self.image_check(
                "text_on_curved_line_zero_width_char",
                "text_on_curved_line_zero_width_char",
                image,
                "text_on_curved_line_zero_width_char",
            )
        )

    def testDrawTextOnLineAtOffset(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(16)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)

        image = QImage(400, 400, QImage.Format.Format_RGB32)

        painter = QPainter()
        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 50, 50))
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI
        context.setFlag(
            QgsRenderContext.Flag.ApplyScalingWorkaroundForTextRendering, True
        )

        painter.begin(image)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)
        image.fill(QColor(152, 219, 249))

        painter.setBrush(Qt.BrushStyle.NoBrush)
        painter.setPen(QPen(QColor(0, 0, 0)))

        line = QPolygonF([QPointF(50, 200), QPointF(350, 200)])
        painter.drawPolygon(line)

        painter.setBrush(QBrush(QColor(182, 239, 255)))
        painter.setPen(Qt.PenStyle.NoPen)

        QgsTextRenderer.drawTextOnLine(line, "my curved text", context, format, 100)

        painter.end()
        self.assertTrue(
            self.image_check(
                "text_on_line_at_offset",
                "text_on_line_at_offset",
                image,
                "text_on_line_at_offset",
            )
        )

    def testDrawTextOnCurvedLine(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(16)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.buffer().setEnabled(True)
        format.buffer().setSize(2)
        format.buffer().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)

        image = QImage(400, 400, QImage.Format.Format_RGB32)

        painter = QPainter()
        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 50, 50))
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI
        context.setFlag(
            QgsRenderContext.Flag.ApplyScalingWorkaroundForTextRendering, True
        )

        painter.begin(image)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)
        image.fill(QColor(152, 219, 249))

        painter.setBrush(Qt.BrushStyle.NoBrush)
        painter.setPen(QPen(QColor(0, 0, 0)))

        line = QPolygonF(
            [QPointF(50, 200), QPointF(100, 230), QPointF(150, 235), QPointF(350, 200)]
        )
        painter.drawPolyline(line)

        painter.setBrush(QBrush(QColor(182, 239, 255)))
        painter.setPen(Qt.PenStyle.NoPen)

        format.setAllowHtmlFormatting(True)
        QgsTextRenderer.drawTextOnLine(
            line,
            'm<sup>y</sup> <span style="font-size: 29pt; color: red;">curv<sup style="font-size: 10pt">ed</sup></span> te<sub>xt</sub>',
            context,
            format,
            20,
            0,
        )

        painter.end()
        self.assertTrue(
            self.image_check(
                "text_on_curved_line",
                "text_on_curved_line",
                image,
                "text_on_curved_line",
            )
        )

    def testDrawTextOnCurvedLineUpsideDown(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(16)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        format.buffer().setEnabled(True)
        format.buffer().setSize(2)
        format.buffer().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)

        image = QImage(400, 400, QImage.Format.Format_RGB32)

        painter = QPainter()
        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 50, 50))
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI
        context.setFlag(
            QgsRenderContext.Flag.ApplyScalingWorkaroundForTextRendering, True
        )

        painter.begin(image)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)
        image.fill(QColor(152, 219, 249))

        painter.setBrush(Qt.BrushStyle.NoBrush)
        painter.setPen(QPen(QColor(0, 0, 0)))

        line = QPolygonF(
            reversed(
                [
                    QPointF(50, 200),
                    QPointF(100, 230),
                    QPointF(150, 235),
                    QPointF(350, 200),
                ]
            )
        )
        painter.drawPolyline(line)

        painter.setBrush(QBrush(QColor(182, 239, 255)))
        painter.setPen(Qt.PenStyle.NoPen)

        format.setAllowHtmlFormatting(True)
        QgsTextRenderer.drawTextOnLine(
            line,
            'm<sup>y</sup> <span style="font-size: 29pt; color: red;">curv<sup style="font-size: 10pt">ed</sup></span> te<sub>xt</sub>',
            context,
            format,
            20,
            0,
        )

        painter.end()
        self.assertTrue(
            self.image_check(
                "text_on_curved_line_upside_down",
                "text_on_curved_line_upside_down",
                image,
                "text_on_curved_line_upside_down",
            )
        )

    def testDrawTextOnCurvedLineBackground(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(16)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)

        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeRectangle)
        format.background().setSize(QSizeF(2, 2))
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeBuffer)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMillimeters)
        format.background().setFillColor(QColor(255, 255, 255))

        image = QImage(400, 400, QImage.Format.Format_RGB32)

        painter = QPainter()
        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 50, 50))
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI
        context.setFlag(
            QgsRenderContext.Flag.ApplyScalingWorkaroundForTextRendering, True
        )

        painter.begin(image)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)
        image.fill(QColor(152, 219, 249))

        painter.setBrush(Qt.BrushStyle.NoBrush)
        painter.setPen(QPen(QColor(0, 0, 0)))

        line = QPolygonF(
            [QPointF(50, 200), QPointF(100, 230), QPointF(150, 235), QPointF(350, 200)]
        )
        painter.drawPolyline(line)

        painter.setBrush(QBrush(QColor(182, 239, 255)))
        painter.setPen(Qt.PenStyle.NoPen)

        QgsTextRenderer.drawTextOnLine(line, "my curved text", context, format, 20, 0)

        painter.end()
        self.assertTrue(
            self.image_check(
                "text_on_curved_line_background",
                "text_on_curved_line_background",
                image,
                "text_on_curved_line_background",
            )
        )

    def testDrawTextOnCurvedLineOffsetFromLine(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(16)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)

        image = QImage(400, 400, QImage.Format.Format_RGB32)

        painter = QPainter()
        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 50, 50))
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI
        context.setFlag(
            QgsRenderContext.Flag.ApplyScalingWorkaroundForTextRendering, True
        )

        painter.begin(image)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)
        image.fill(QColor(152, 219, 249))

        painter.setBrush(Qt.BrushStyle.NoBrush)
        painter.setPen(QPen(QColor(0, 0, 0)))

        line = QPolygonF(
            [QPointF(50, 200), QPointF(100, 230), QPointF(150, 235), QPointF(350, 200)]
        )
        painter.drawPolyline(line)

        painter.setBrush(QBrush(QColor(182, 239, 255)))
        painter.setPen(Qt.PenStyle.NoPen)

        format.setAllowHtmlFormatting(True)
        QgsTextRenderer.drawTextOnLine(line, "my curved text", context, format, 20, -20)

        painter.end()
        self.assertTrue(
            self.image_check(
                "text_on_curved_line_offset_line",
                "text_on_curved_line_offset_line",
                image,
                "text_on_curved_line_offset_line",
            )
        )

    def testDrawTextOnCurvedLineOffsetFromLinePositive(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(16)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)

        image = QImage(400, 400, QImage.Format.Format_RGB32)

        painter = QPainter()
        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 50, 50))
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI
        context.setFlag(
            QgsRenderContext.Flag.ApplyScalingWorkaroundForTextRendering, True
        )

        painter.begin(image)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)
        image.fill(QColor(152, 219, 249))

        painter.setBrush(Qt.BrushStyle.NoBrush)
        painter.setPen(QPen(QColor(0, 0, 0)))

        line = QPolygonF(
            [QPointF(50, 200), QPointF(100, 230), QPointF(150, 235), QPointF(350, 200)]
        )
        painter.drawPolyline(line)

        painter.setBrush(QBrush(QColor(182, 239, 255)))
        painter.setPen(Qt.PenStyle.NoPen)

        format.setAllowHtmlFormatting(True)
        QgsTextRenderer.drawTextOnLine(line, "my curved text", context, format, 20, 20)

        painter.end()
        self.assertTrue(
            self.image_check(
                "text_on_curved_line_offset_line_positive",
                "text_on_curved_line_offset_line_positive",
                image,
                "text_on_curved_line_offset_line_positive",
            )
        )

    def testDrawTextDataDefinedProperties(self):
        format = QgsTextFormat()
        format.setFont(getTestFont("bold"))
        format.setSize(16)
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)

        format.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.Size, QgsProperty.fromExpression("90*1.5")
        )

        self.assertTrue(
            self.checkRender(
                format,
                "datadefined_render",
                None,
                text=["1234", "5678"],
                rect=QRectF(40, 20, 350, 350),
                alignment=QgsTextRenderer.HAlignment.AlignRight,
            )
        )


if __name__ == "__main__":
    unittest.main()
