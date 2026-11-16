"""
***************************************************************************
    test_qgspainting.py
    ---------------------
    Date                 : August 2023
    Copyright            : (C) 2023 by Nyall Dawson
    Email                : nyall dot dawson at gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Nyall Dawson"
__date__ = "August 2023"
__copyright__ = "(C) 2023, Nyall Dawson"

import math
import unittest
from concurrent.futures import ThreadPoolExecutor

from qgis.core import QgsFontUtils, QgsImageOperation, QgsPicture
from qgis.PyQt.QtCore import QLine, QLineF, QPoint, QPointF, QRect, QRectF, Qt
from qgis.PyQt.QtGui import (
    QBrush,
    QColor,
    QConicalGradient,
    QImage,
    QLinearGradient,
    QPainter,
    QPainterPath,
    QPen,
    QPicture,
    QPixmap,
    QPolygon,
    QPolygonF,
    QRadialGradient,
    QRegion,
    QTransform,
)
from qgis.testing import QgisTestCase
from utilities import start_app, unitTestDataPath

TEST_DATA_DIR = unitTestDataPath()
start_app()


class TestQgsPicture(QgisTestCase):
    @classmethod
    def control_path_prefix(cls):
        return "painting"

    def test_empty_and_invalid(self):
        """
        Test empty QgsPicture
        """
        pic = QgsPicture()
        self.assertTrue(pic.isNull())
        self.assertTrue(pic.boundingRect().isEmpty())

        inactive_painter = QPainter()
        self.assertFalse(pic.play(inactive_painter))

        # render empty QgsPicture
        img = QImage(50, 50, QImage.Format.Format_ARGB32_Premultiplied)
        img.fill(Qt.GlobalColor.white)
        painter = QPainter(img)
        self.assertTrue(pic.play(painter))
        painter.end()
        self.assertTrue(QgsImageOperation.isSingleColor(img, Qt.GlobalColor.white))

    def test_detach(self):
        """
        Test detaching QgsPicture
        """
        pic1 = QgsPicture()
        self.assertTrue(pic1.isDetached())

        p = QPainter(pic1)
        p.setPen(Qt.PenStyle.NoPen)
        p.setBrush(QColor(255, 0, 0))
        p.drawRect(QRectF(10, 20, 30, 40))
        p.end()

        self.assertFalse(pic1.isNull())
        self.assertEqual(pic1.boundingRect(), QRect(10, 20, 30, 40))

        pic2 = QgsPicture(pic1)
        self.assertFalse(pic1.isDetached())
        self.assertFalse(pic2.isDetached())
        self.assertEqual(pic2.boundingRect(), QRect(10, 20, 30, 40))

        pic1.setBoundingRect(QRect(0, 0, 100, 100))
        self.assertTrue(pic1.isDetached())
        self.assertTrue(pic2.isDetached())
        self.assertEqual(pic1.boundingRect(), QRect(0, 0, 100, 100))
        self.assertEqual(pic2.boundingRect(), QRect(10, 20, 30, 40))

        p2 = QPainter(pic2)
        p2.setPen(Qt.PenStyle.NoPen)
        p2.drawLine(QLineF(0, 0, 50, 0))
        p2.end()
        self.assertEqual(pic1.boundingRect(), QRect(0, 0, 100, 100))
        self.assertEqual(pic2.boundingRect().width(), 50)

    def test_multithreaded_replay(self):
        """
        Test concurrent replay of QgsPicture on different threads
        """
        test_image = TestQgsPicture._create_test_image()
        test_pixmap = QPixmap.fromImage(test_image)

        pic = QgsPicture()
        p = QPainter(pic)
        p.setRenderHint(QPainter.RenderHint.Antialiasing, True)
        p.setBrush(QBrush(test_pixmap))
        p.setPen(QPen(QBrush(test_pixmap), 6))
        p.drawRoundedRect(QRectF(10, 10, 100, 80), 12, 12)
        # use a pixmap, because if we don't handle them correctly
        # (as direct renders, or as brush/pen textures)
        # we'll get a crash when drawing off the main thread (ie make sure that
        # pixmap handling is correctly implemented in QgsPicture)
        p.drawTiledPixmap(QRectF(20, 105, 90, 40), test_pixmap, QPointF(4, 4))
        p.drawPixmap(QPointF(120, 20), test_pixmap)
        font = QgsFontUtils.getStandardTestFont("Bold", 32)
        p.setFont(font)
        p.drawText(QPointF(20, 195), "QGIS")
        p.end()

        def render_worker(_idx: int) -> QImage:
            out = QImage(180, 200, QImage.Format.Format_ARGB32_Premultiplied)
            out.fill(Qt.GlobalColor.white)
            wp = QPainter(out)
            res = pic.play(wp)
            wp.end()
            self.assertTrue(res)
            return out

        with ThreadPoolExecutor(max_workers=8) as pool:
            results = list(pool.map(render_worker, range(1000)))

        reference_image = results[0]
        self.assertTrue(
            self.image_check(
                "qgspicture_threading", "qgspicture_threading", reference_image
            )
        )

        # don't check other images against the expected one (takes a long time!)
        # instead just directly check against the first image we already know
        # is correct
        for render_image in results[1:]:
            self.assertEqual(render_image, reference_image)

    @staticmethod
    def _create_test_image() -> QImage:
        img = QImage(32, 32, QImage.Format.Format_ARGB32_Premultiplied)
        img.fill(QColor(30, 120, 220, 220))
        p = QPainter(img)
        p.setPen(Qt.PenStyle.NoPen)
        p.setBrush(QColor(240, 200, 40, 240))
        p.drawRect(0, 0, 16, 16)
        p.setBrush(QColor(220, 60, 80, 240))
        p.drawEllipse(12, 12, 18, 18)
        p.end()
        return img

    @staticmethod
    def _create_star_polygon(
        cx: float, cy: float, outer_r: float, inner_r: float
    ) -> QPolygonF:
        pts = []
        for i in range(5):
            angle = math.radians(-90 + i * 144)
            pts.append(
                QPointF(cx + outer_r * math.cos(angle), cy + outer_r * math.sin(angle))
            )
        return QPolygonF(pts)

    @staticmethod
    def _draw_objects(painter: QPainter):
        """
        Utility function to draw a whole range of objects using all (hopefully!) available
        QPainter methods.

        May need to be extended in future if we've missed any here
        """

        # IMPORTANT -- when creating brushes/pens here, use the pixmap based constructors,
        # so that we can check that those are handled correctly
        test_image = TestQgsPicture._create_test_image()
        test_pixmap = QPixmap.fromImage(test_image)

        painter.save()

        painter.setPen(QPen(QColor(220, 20, 20), 4))
        painter.drawPoint(QPointF(25.5, 35.5))
        painter.drawPoint(QPoint(35, 35))
        painter.drawPoints(
            QPolygonF(
                [QPointF(25, 55), QPointF(35, 65), QPointF(45, 55), QPointF(55, 65)]
            )
        )
        painter.drawPoints(QPolygon([QPoint(25, 85), QPoint(40, 95), QPoint(55, 85)]))

        painter.setPen(QPen(QColor(20, 140, 40), 5))
        painter.drawLine(QLineF(80.5, 25.5, 140.5, 45.5))
        painter.drawLine(QLine(80, 55, 140, 55))
        painter.drawLines(
            [
                QLineF(80, 70, 140, 70),
                QLineF(80, 82, 140, 95),
                QLineF(80, 105, 140, 98),
            ]
        )
        painter.drawLines([QLine(80, 112, 110, 118), QLine(115, 118, 140, 112)])

        painter.setPen(QPen(QColor(20, 40, 180), 3))
        painter.setBrush(QColor(120, 160, 240, 180))
        painter.drawRect(QRectF(165.5, 25.5, 40, 35))
        painter.drawRect(QRect(215, 25, 40, 35))
        painter.drawRects([QRectF(165, 72, 25, 35), QRectF(195, 72, 25, 35)])
        painter.drawRects([QRect(225, 72, 15, 35), QRect(243, 72, 15, 35)])

        painter.setBrush(QColor(240, 160, 80, 200))
        painter.drawRoundedRect(
            QRectF(280, 25, 70, 40), 25, 25, Qt.SizeMode.RelativeSize
        )
        painter.drawRoundedRect(
            QRectF(280, 75, 70, 40), 10.0, 10.0, Qt.SizeMode.AbsoluteSize
        )

        painter.setBrush(QColor(160, 90, 220, 200))
        painter.drawEllipse(QRectF(375.5, 25.5, 65, 40))
        painter.drawEllipse(QRect(375, 75, 65, 40))
        painter.restore()

        painter.save()
        painter.setPen(QPen(QColor(180, 50, 50), 5))
        painter.setBrush(QColor(250, 200, 120, 200))
        painter.drawArc(QRectF(20, 145, 65, 65), 30 * 16, 240 * 16)
        painter.drawPie(QRectF(100, 145, 65, 65), 45 * 16, 250 * 16)
        painter.drawChord(QRectF(180, 145, 65, 65), 20 * 16, 200 * 16)

        painter.setBrush(Qt.BrushStyle.NoBrush)
        painter.setPen(QPen(QColor(20, 110, 130), 3.5))
        painter.drawPolyline(
            QPolygonF(
                [
                    QPointF(265, 200),
                    QPointF(285, 150),
                    QPointF(305, 200),
                    QPointF(325, 150),
                ]
            )
        )
        painter.drawPolyline(
            QPolygon([QPoint(265, 245), QPoint(295, 215), QPoint(325, 245)])
        )

        painter.setPen(QPen(QColor(40, 40, 40), 3.5))
        painter.setBrush(QColor(80, 200, 140, 210))
        painter.drawPolygon(
            TestQgsPicture._create_star_polygon(385, 185, 38, 15),
            Qt.FillRule.OddEvenFill,
        )
        painter.drawPolygon(
            TestQgsPicture._create_star_polygon(475, 185, 38, 15),
            Qt.FillRule.WindingFill,
        )
        painter.drawPolygon(
            QPolygon(
                [QPoint(530, 155), QPoint(580, 155), QPoint(565, 215), QPoint(530, 205)]
            )
        )
        painter.drawConvexPolygon(
            QPolygonF(
                [
                    QPointF(600, 165),
                    QPointF(635, 145),
                    QPointF(660, 175),
                    QPointF(645, 215),
                    QPointF(605, 205),
                ]
            )
        )

        path = QPainterPath()
        path.moveTo(685, 205)
        path.cubicTo(685, 145, 745, 145, 745, 205)
        path.quadTo(715, 235, 685, 205)
        painter.setPen(QPen(QColor(110, 20, 140), 4))
        painter.setBrush(QColor(210, 150, 240, 200))
        painter.drawPath(path)

        fill_only_path = QPainterPath()
        fill_only_path.addEllipse(QRectF(765, 150, 50, 50))
        painter.fillPath(fill_only_path, QBrush(QColor(240, 90, 90, 200)))
        painter.strokePath(fill_only_path, QPen(QColor(40, 40, 180), 4))

        painter.fillRect(QRectF(835, 150, 60, 55), QColor(60, 180, 220))
        painter.eraseRect(QRectF(850, 165, 30, 25))
        painter.restore()

        painter.save()
        pen_styles = [
            Qt.PenStyle.NoPen,
            Qt.PenStyle.SolidLine,
            Qt.PenStyle.DashLine,
            Qt.PenStyle.DotLine,
            Qt.PenStyle.DashDotLine,
            Qt.PenStyle.DashDotDotLine,
        ]
        for idx, style in enumerate(pen_styles):
            pen = QPen(QColor(30, 30, 30), 5, style)
            painter.setPen(pen)
            y = 275 + idx * 16
            painter.drawLine(QLineF(20, y, 160, y))

        custom_pen = QPen(QColor(200, 40, 120), 5)
        custom_pen.setDashPattern([6.0, 3.0, 1.0, 3.0])
        painter.setPen(custom_pen)
        painter.drawLine(QLineF(20, 372, 160, 372))

        cosmetic_pen = QPen(QColor(0, 120, 200), 0)
        painter.setPen(cosmetic_pen)
        painter.drawLine(QLineF(20, 384, 160, 384))

        for idx, cap in enumerate(
            [Qt.PenCapStyle.FlatCap, Qt.PenCapStyle.SquareCap, Qt.PenCapStyle.RoundCap]
        ):
            pen = QPen(QColor(20, 120, 60), 10)
            pen.setCapStyle(cap)
            painter.setPen(pen)
            y = 285 + idx * 35
            painter.drawLine(QLineF(200, y, 310, y))

        for idx, join in enumerate(
            [
                Qt.PenJoinStyle.MiterJoin,
                Qt.PenJoinStyle.BevelJoin,
                Qt.PenJoinStyle.RoundJoin,
            ]
        ):
            pen = QPen(QColor(180, 90, 20), 9)
            pen.setJoinStyle(join)
            painter.setPen(pen)
            x0 = 350 + idx * 95
            painter.drawPolyline(
                QPolygonF(
                    [QPointF(x0, 365), QPointF(x0 + 35, 285), QPointF(x0 + 70, 365)]
                )
            )

        tex_pen = QPen(QBrush(test_pixmap), 12)
        painter.setPen(tex_pen)
        painter.setBrush(Qt.BrushStyle.NoBrush)
        painter.drawRect(QRectF(650, 280, 140, 85))
        painter.restore()

        painter.save()
        painter.setPen(QPen(QColor(50, 50, 50), 4))
        brush_styles = [
            Qt.BrushStyle.NoBrush,
            Qt.BrushStyle.SolidPattern,
            Qt.BrushStyle.Dense4Pattern,
            Qt.BrushStyle.HorPattern,
            Qt.BrushStyle.VerPattern,
            Qt.BrushStyle.CrossPattern,
            Qt.BrushStyle.DiagCrossPattern,
        ]
        for idx, bstyle in enumerate(brush_styles):
            painter.setBrush(QBrush(QColor(40, 90, 180), bstyle))
            painter.drawRect(QRectF(20 + idx * 62, 415, 52, 45))

        painter.setBrush(QBrush(QColor(200, 30, 30), Qt.BrushStyle.DiagCrossPattern))
        painter.setBackground(QBrush(QColor(255, 245, 150)))
        painter.setBackgroundMode(Qt.BGMode.OpaqueMode)
        painter.drawRect(QRectF(20, 475, 90, 45))

        painter.setBackgroundMode(Qt.BGMode.TransparentMode)
        painter.drawRect(QRectF(125, 475, 90, 45))

        lin_grad = QLinearGradient(240, 475, 330, 520)
        lin_grad.setColorAt(0.0, QColor(255, 80, 80))
        lin_grad.setColorAt(1.0, QColor(80, 80, 255))
        painter.setBrush(QBrush(lin_grad))
        painter.drawRect(QRectF(240, 475, 90, 45))

        rad_grad = QRadialGradient(QPointF(395, 497), 35)
        rad_grad.setColorAt(0.0, QColor(255, 255, 100))
        rad_grad.setColorAt(1.0, QColor(20, 150, 80))
        painter.setBrush(QBrush(rad_grad))
        painter.drawEllipse(QRectF(350, 475, 90, 45))

        con_grad = QConicalGradient(QPointF(505, 497), 30)
        con_grad.setColorAt(0.0, QColor(255, 0, 200))
        con_grad.setColorAt(0.5, QColor(0, 220, 255))
        con_grad.setColorAt(1.0, QColor(255, 0, 200))
        painter.setBrush(QBrush(con_grad))
        painter.drawRect(QRectF(460, 475, 90, 45))

        img_brush = QBrush(test_image)
        painter.setBrush(img_brush)
        painter.setBrushOrigin(QPointF(575, 415))
        painter.drawRect(QRectF(575, 415, 110, 105))

        pix_brush = QBrush(test_pixmap)
        painter.setBrush(pix_brush)
        painter.setBrushOrigin(QPointF(715, 430))
        painter.drawRect(QRectF(705, 415, 110, 105))
        painter.restore()

        painter.save()
        painter.drawImage(QPointF(20, 555), test_image)
        painter.drawImage(QRectF(70, 555, 90, 90), test_image, QRectF(4, 4, 24, 24))

        painter.drawPixmap(QPointF(185, 555), test_pixmap)
        painter.drawPixmap(QRectF(235, 555, 90, 90), test_pixmap, QRectF(0, 0, 20, 20))

        painter.drawTiledPixmap(QRectF(350, 555, 160, 95), test_pixmap, QPointF(8, 12))

        # render picture in picture
        nested_qpic = QPicture()
        np = QPainter(nested_qpic)
        np.setPen(QPen(QColor(20, 20, 160), 4))
        np.setBrush(QColor(150, 230, 180))
        np.drawEllipse(QRectF(0, 0, 70, 50))
        np.drawLine(QLineF(0, 25, 70, 25))
        np.end()
        painter.drawPicture(QPointF(540, 570), nested_qpic)
        painter.restore()

        painter.save()
        painter.translate(60, 740)
        painter.rotate(25)
        painter.scale(1.3, 0.8)
        painter.setPen(QPen(QColor(180, 30, 90), 4))
        painter.setBrush(QColor(250, 180, 210))
        painter.drawRect(QRectF(-25, -20, 50, 40))
        painter.restore()

        painter.save()
        xform = QTransform()
        xform.translate(180, 720)
        xform.shear(0.35, 0.0)
        painter.setTransform(xform, True)
        painter.setPen(QPen(QColor(0, 0, 0), 4))
        painter.setBrush(QColor(120, 210, 240))
        painter.drawRect(QRectF(0, 0, 60, 45))
        painter.restore()

        # clip rects
        painter.save()
        painter.setClipRect(QRectF(290, 700, 60, 60))
        painter.setPen(QPen(QColor(0, 0, 0), 4))
        painter.setBrush(QColor(240, 100, 60))
        painter.drawEllipse(QRectF(280, 690, 80, 80))
        painter.restore()

        painter.save()
        rgn1 = QRegion(380, 700, 70, 70, QRegion.RegionType.Ellipse)
        rgn2 = QRegion(405, 700, 70, 70, QRegion.RegionType.Rectangle)
        painter.setClipRegion(rgn1, Qt.ClipOperation.ReplaceClip)
        painter.setClipRegion(rgn2, Qt.ClipOperation.IntersectClip)
        painter.setPen(QPen(QColor(0, 0, 0), 4))
        painter.setBrush(QColor(60, 160, 90))
        painter.drawRect(QRectF(375, 695, 110, 80))
        painter.restore()

        painter.save()
        clip_path = QPainterPath()
        clip_path.addRoundedRect(QRectF(500, 700, 75, 70), 20, 20)
        painter.setClipPath(clip_path, Qt.ClipOperation.ReplaceClip)
        painter.setBrush(QColor(90, 90, 220))
        painter.setPen(QPen(QColor(0, 0, 0), 4))
        painter.drawRect(QRectF(490, 690, 95, 90))
        painter.setClipping(False)
        painter.setBrush(QColor(220, 180, 40))
        painter.drawEllipse(QRectF(590, 710, 45, 45))
        painter.restore()

        painter.save()
        painter.setPen(Qt.PenStyle.NoPen)
        painter.setBrush(QColor(220, 30, 30))
        painter.drawRect(QRectF(20, 835, 55, 55))
        painter.setOpacity(0.45)
        painter.setBrush(QColor(30, 30, 220))
        painter.drawRect(QRectF(45, 855, 55, 55))
        painter.setOpacity(1.0)

        comp_modes = [
            QPainter.CompositionMode.CompositionMode_SourceOver,
            QPainter.CompositionMode.CompositionMode_Multiply,
            QPainter.CompositionMode.CompositionMode_Screen,
            QPainter.CompositionMode.CompositionMode_Plus,
        ]
        for idx, mode in enumerate(comp_modes):
            x0 = 135 + idx * 75
            painter.setCompositionMode(
                QPainter.CompositionMode.CompositionMode_SourceOver
            )
            painter.setBrush(QColor(220, 90, 40))
            painter.drawEllipse(QRectF(x0, 835, 45, 45))
            painter.setCompositionMode(mode)
            painter.setBrush(QColor(40, 130, 220))
            painter.drawEllipse(QRectF(x0 + 18, 852, 45, 45))
        painter.setCompositionMode(QPainter.CompositionMode.CompositionMode_SourceOver)

        painter.setPen(QPen(QColor(20, 20, 20), 4))
        painter.setBrush(QColor(180, 220, 120))
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)
        painter.drawEllipse(QRectF(450, 835, 65, 65))
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, False)
        painter.drawEllipse(QRectF(530, 835, 65, 65))

        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)
        font = QgsFontUtils.getStandardTestFont("Bold", 16)
        painter.setFont(font)
        painter.setPen(QColor(20, 20, 20))
        painter.drawText(QPointF(620, 855), "QgsPicture Text")
        painter.drawText(
            QRectF(620, 865, 170, 40),
            Qt.AlignmentFlag.AlignCenter | Qt.TextFlag.TextWordWrap,
            "Aligned Rect Text",
        )
        painter.restore()

    def test_draw_using_qgspicture(self):
        """
        Test QgsPicture
        """
        picture = QgsPicture()
        painter = QPainter(picture)

        # record drawing
        self._draw_objects(painter)

        painter.end()

        self.assertEqual(picture.boundingRect(), QRect(17, 23, 878, 887))

        # now draw picture
        render_image = QImage(1000, 1000, QImage.Format.Format_ARGB32)
        render_image.fill(Qt.GlobalColor.transparent)

        painter = QPainter(render_image)
        self.assertTrue(picture.play(painter))
        painter.end()

        self.assertTrue(self.image_check("qgspicture", "qgspicture", render_image))

    def test_draw_using_qimage(self):
        """
        Test drawing using QImage.

        (this double-checks that the result of rendering via all the paint
        objects from _draw_objects to QgsPicture then drawing QgsPicture to
        QImage is exactly the same as drawing to QImage directly)
        """
        render_image = QImage(1000, 1000, QImage.Format.Format_ARGB32)
        render_image.fill(Qt.GlobalColor.transparent)

        painter = QPainter(render_image)
        self._draw_objects(painter)
        painter.end()

        self.assertTrue(self.image_check("qgspicture", "qgspicture", render_image))

    def test_clip_logic(self):
        """
        Test QgsPicture clip rect handling
        """
        pic = QgsPicture()
        p = QPainter(pic)
        p.setClipRect(QRectF(0, 0, 100, 100), Qt.ClipOperation.ReplaceClip)
        p.setClipping(False)
        p.setPen(Qt.PenStyle.NoPen)
        p.setBrush(QColor(255, 0, 0))
        p.drawRect(QRectF(0, 0, 100, 100))
        p.end()

        image = QImage(100, 100, QImage.Format.Format_ARGB32_Premultiplied)
        image.fill(Qt.GlobalColor.white)

        rp = QPainter(image)
        rp.setClipRect(QRectF(30, 30, 40, 40))
        self.assertTrue(pic.play(rp))
        rp.end()

        self.assertTrue(self.image_check("qgspicture_clip", "qgspicture_clip", image))


if __name__ == "__main__":
    unittest.main()
