"""QGIS Unit tests for annotations.

From build dir, run: ctest -R PyQgsAnnotation -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "24/1/2017"
__copyright__ = "Copyright 2017, The QGIS Project"

import os
import tempfile

from qgis.PyQt.QtCore import QPointF, QRectF, QSize, QSizeF
from qgis.PyQt.QtGui import QColor, QImage, QPainter, QTextDocument
from qgis.core import (
    Qgis,
    QgsCoordinateReferenceSystem,
    QgsFeature,
    QgsFillSymbol,
    QgsHtmlAnnotation,
    QgsLayout,
    QgsLayoutItemMap,
    QgsMapSettings,
    QgsMargins,
    QgsPointXY,
    QgsProject,
    QgsRectangle,
    QgsRenderChecker,
    QgsRenderContext,
    QgsSvgAnnotation,
    QgsTextAnnotation,
    QgsVectorLayer,
    QgsAnnotationPictureItem,
    QgsAnnotationRectangleTextItem,
    QgsBalloonCallout,
)
from qgis.gui import QgsFormAnnotation
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsAnnotation(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "annotations"

    def testTextAnnotation(self):
        """test rendering a text annotation"""
        a = QgsTextAnnotation()
        a.fillSymbol().symbolLayer(0).setStrokeColor(QColor(0, 0, 0))
        a.markerSymbol().symbolLayer(0).setStrokeColor(QColor(0, 0, 0))
        a.setFrameSizeMm(QSizeF(300 / 3.7795275, 200 / 3.7795275))
        a.setFrameOffsetFromReferencePointMm(QPointF(40 / 3.7795275, 50 / 3.7795275))
        doc = QTextDocument()
        doc.setHtml(
            '<p style="font-family: arial; font-weight: bold; font-size: 40px;">test annotation</p>'
        )
        a.setDocument(doc)
        im = self.renderAnnotation(a, QPointF(20, 30))
        self.assertTrue(self.image_check("text_annotation", "text_annotation", im))

        # check clone
        clone = a.clone()
        im = self.renderAnnotation(clone, QPointF(20, 30))
        self.assertTrue(self.image_check("text_annotation", "text_annotation", im))

    def testTextAnnotationInLayout(self):
        """test rendering a text annotation"""
        a = QgsTextAnnotation()
        a.fillSymbol().symbolLayer(0).setStrokeColor(QColor(0, 0, 0))
        a.markerSymbol().symbolLayer(0).setStrokeColor(QColor(0, 0, 0))
        a.setFrameSizeMm(QSizeF(300 / 3.7795275, 200 / 3.7795275))
        a.setFrameOffsetFromReferencePointMm(QPointF(40 / 3.7795275, 50 / 3.7795275))
        doc = QTextDocument()
        doc.setHtml(
            '<p style="font-family: arial; font-weight: bold; font-size: 40px;">test annotation</p>'
        )
        a.setDocument(doc)
        self.assertTrue(self.renderAnnotationInLayout("text_annotation_in_layout", a))

    def test_svg_annotation_project_upgrade(self):
        """
        Test that svg annotations are upgraded to annotation layers when loading projects
        """
        a = QgsSvgAnnotation()
        a.fillSymbol().symbolLayer(0).setStrokeColor(QColor(0, 0, 0))
        a.markerSymbol().symbolLayer(0).setStrokeColor(QColor(0, 0, 0))
        a.setFrameSizeMm(QSizeF(300 / 3.7795275, 200 / 3.7795275))
        a.setHasFixedMapPosition(True)
        a.setMapPosition(QgsPointXY(QPointF(20, 30)))
        a.setMapPositionCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        a.setFrameOffsetFromReferencePointMm(QPointF(40 / 3.7795275, 50 / 3.7795275))
        svg = TEST_DATA_DIR + "/sample_svg.svg"
        a.setFilePath(svg)

        b = QgsSvgAnnotation()
        b.fillSymbol().symbolLayer(0).setStrokeColor(QColor(0, 0, 0))
        b.markerSymbol().symbolLayer(0).setStrokeColor(QColor(0, 0, 0))
        b.setFrameSizeMm(QSizeF(300 / 3.7795275, 200 / 3.7795275))
        b.setRelativePosition(QPointF(0.2, 0.7))
        b.setHasFixedMapPosition(False)
        svg = TEST_DATA_DIR + "/sample_svg.svg"
        b.setFilePath(svg)

        p = QgsProject()
        p.annotationManager().addAnnotation(a)
        p.annotationManager().addAnnotation(b)

        p2 = QgsProject()
        with tempfile.TemporaryDirectory() as temp_dir:
            path = os.path.join(temp_dir, "test_project.qgs")
            p.write(path)
            p2.read(path)

        # should be no annotations in upgraded project
        self.assertFalse(p2.annotationManager().annotations())
        # annotation layer should contain picture items
        items = p2.mainAnnotationLayer().items()
        self.assertEqual(len(items), 2)

        item_a = [i for _, i in items.items() if not i.calloutAnchor().isEmpty()][0]
        item_b = [i for _, i in items.items() if i.calloutAnchor().isEmpty()][0]
        self.assertIsInstance(item_a, QgsAnnotationPictureItem)
        self.assertIsInstance(item_b, QgsAnnotationPictureItem)

        self.assertEqual(item_a.calloutAnchor().asWkt(), "Point (20 30)")
        self.assertEqual(item_a.placementMode(), Qgis.AnnotationPlacementMode.FixedSize)
        self.assertIsInstance(item_a.callout(), QgsBalloonCallout)
        self.assertAlmostEqual(item_a.fixedSize().width(), 79.375, 1)
        self.assertAlmostEqual(item_a.fixedSize().height(), 52.9166, 1)
        self.assertAlmostEqual(item_a.offsetFromCallout().width(), 10.5833, 1)
        self.assertAlmostEqual(item_a.offsetFromCallout().height(), 13.229, 1)

        self.assertIsNone(item_b.callout())
        self.assertEqual(
            item_b.placementMode(), Qgis.AnnotationPlacementMode.RelativeToMapFrame
        )
        self.assertAlmostEqual(item_b.fixedSize().width(), 79.375, 1)
        self.assertAlmostEqual(item_b.fixedSize().height(), 52.9166, 1)
        self.assertAlmostEqual(item_b.bounds().center().x(), 0.2, 3)
        self.assertAlmostEqual(item_b.bounds().center().y(), 0.7, 3)

    def test_text_annotation_project_upgrade(self):
        """
        Test that text annotations are upgraded to annotation layers when loading projects
        """
        a = QgsTextAnnotation()
        a.fillSymbol().symbolLayer(0).setStrokeColor(QColor(0, 0, 0))
        a.markerSymbol().symbolLayer(0).setStrokeColor(QColor(0, 0, 0))
        a.setFrameSizeMm(QSizeF(300 / 3.7795275, 200 / 3.7795275))
        a.setHasFixedMapPosition(True)
        a.setMapPosition(QgsPointXY(QPointF(20, 30)))
        a.setMapPositionCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        a.setFrameOffsetFromReferencePointMm(QPointF(40 / 3.7795275, 50 / 3.7795275))
        a.document().setHtml("<p>test annotation</p>")

        b = QgsTextAnnotation()
        b.fillSymbol().symbolLayer(0).setStrokeColor(QColor(0, 0, 0))
        b.markerSymbol().symbolLayer(0).setStrokeColor(QColor(0, 0, 0))
        b.setFrameSizeMm(QSizeF(300 / 3.7795275, 200 / 3.7795275))
        b.setRelativePosition(QPointF(0.2, 0.7))
        b.setHasFixedMapPosition(False)
        b.document().setHtml("<p>test annotation</p>")

        p = QgsProject()
        p.annotationManager().addAnnotation(a)
        p.annotationManager().addAnnotation(b)

        p2 = QgsProject()
        with tempfile.TemporaryDirectory() as temp_dir:
            path = os.path.join(temp_dir, "test_project.qgs")
            p.write(path)
            p2.read(path)

        # should be no annotations in upgraded project
        self.assertFalse(p2.annotationManager().annotations())
        # annotation layer should contain text items
        items = p2.mainAnnotationLayer().items()
        self.assertEqual(len(items), 2)

        item_a = [i for _, i in items.items() if not i.calloutAnchor().isEmpty()][0]
        item_b = [i for _, i in items.items() if i.calloutAnchor().isEmpty()][0]
        self.assertIsInstance(item_a, QgsAnnotationRectangleTextItem)
        self.assertIsInstance(item_b, QgsAnnotationRectangleTextItem)

        self.assertEqual(item_a.calloutAnchor().asWkt(), "Point (20 30)")
        self.assertEqual(item_a.placementMode(), Qgis.AnnotationPlacementMode.FixedSize)
        self.assertIsInstance(item_a.callout(), QgsBalloonCallout)
        self.assertAlmostEqual(item_a.fixedSize().width(), 79.375, 1)
        self.assertAlmostEqual(item_a.fixedSize().height(), 52.9166, 1)
        self.assertAlmostEqual(item_a.offsetFromCallout().width(), 10.5833, 1)
        self.assertAlmostEqual(item_a.offsetFromCallout().height(), 13.229, 1)

        self.assertIsNone(item_b.callout())
        self.assertEqual(
            item_b.placementMode(), Qgis.AnnotationPlacementMode.RelativeToMapFrame
        )
        self.assertAlmostEqual(item_b.fixedSize().width(), 79.375, 1)
        self.assertAlmostEqual(item_b.fixedSize().height(), 52.9166, 1)
        self.assertAlmostEqual(item_b.bounds().center().x(), 0.2, 3)
        self.assertAlmostEqual(item_b.bounds().center().y(), 0.7, 3)

    def testSvgAnnotation(self):
        """test rendering a svg annotation"""
        a = QgsSvgAnnotation()
        a.fillSymbol().symbolLayer(0).setStrokeColor(QColor(0, 0, 0))
        a.markerSymbol().symbolLayer(0).setStrokeColor(QColor(0, 0, 0))
        a.setFrameSizeMm(QSizeF(300 / 3.7795275, 200 / 3.7795275))
        a.setFrameOffsetFromReferencePointMm(QPointF(40 / 3.7795275, 50 / 3.7795275))
        svg = TEST_DATA_DIR + "/sample_svg.svg"
        a.setFilePath(svg)
        im = self.renderAnnotation(a, QPointF(20, 30))
        self.assertTrue(self.image_check("svg_annotation", "svg_annotation", im))

        # check clone
        clone = a.clone()
        im = self.renderAnnotation(clone, QPointF(20, 30))
        self.assertTrue(self.image_check("svg_annotation", "svg_annotation", im))

    def testSvgAnnotationInLayout(self):
        """test rendering a svg annotation"""
        a = QgsSvgAnnotation()
        a.fillSymbol().symbolLayer(0).setStrokeColor(QColor(0, 0, 0))
        a.markerSymbol().symbolLayer(0).setStrokeColor(QColor(0, 0, 0))
        a.setFrameSizeMm(QSizeF(300 / 3.7795275, 200 / 3.7795275))
        a.setFrameOffsetFromReferencePointMm(QPointF(40 / 3.7795275, 50 / 3.7795275))
        svg = TEST_DATA_DIR + "/sample_svg.svg"
        a.setFilePath(svg)
        self.assertTrue(self.renderAnnotationInLayout("svg_annotation_in_layout", a))

    def testHtmlAnnotation(self):
        """test rendering a html annotation"""
        a = QgsHtmlAnnotation()
        a.fillSymbol().symbolLayer(0).setStrokeColor(QColor(0, 0, 0))
        a.markerSymbol().symbolLayer(0).setStrokeColor(QColor(0, 0, 0))
        a.setFrameSizeMm(QSizeF(400 / 3.7795275, 250 / 3.7795275))
        a.setFrameOffsetFromReferencePointMm(QPointF(70 / 3.7795275, 90 / 3.7795275))
        html = TEST_DATA_DIR + "/test_html.html"
        a.setSourceFile(html)
        im = self.renderAnnotation(a, QPointF(20, 30))
        self.assertTrue(self.image_check("html_annotation", "html_annotation", im))

        # check clone
        clone = a.clone()
        im = self.renderAnnotation(clone, QPointF(20, 30))
        self.assertTrue(self.image_check("html_annotation", "html_annotation", im))

    def testHtmlAnnotationSetHtmlSource(self):
        """test rendering html annotation where the html is set directly (not from file)"""
        a = QgsHtmlAnnotation()
        a.fillSymbol().symbolLayer(0).setStrokeColor(QColor(0, 0, 0))
        a.markerSymbol().symbolLayer(0).setStrokeColor(QColor(0, 0, 0))
        a.setFrameSizeMm(QSizeF(400 / 3.7795275, 250 / 3.7795275))
        a.setFrameOffsetFromReferencePointMm(QPointF(70 / 3.7795275, 90 / 3.7795275))
        with open(TEST_DATA_DIR + "/test_html.html") as f:
            htmlText = f.read()
        a.setHtmlSource(htmlText)
        im = self.renderAnnotation(a, QPointF(20, 30))
        self.assertTrue(
            self.image_check("html_annotation_html_source", "html_annotation", im)
        )

    def testHtmlAnnotationInLayout(self):
        """test rendering a svg annotation"""
        a = QgsHtmlAnnotation()
        a.fillSymbol().symbolLayer(0).setStrokeColor(QColor(0, 0, 0))
        a.markerSymbol().symbolLayer(0).setStrokeColor(QColor(0, 0, 0))
        a.setFrameSizeMm(QSizeF(400 / 3.7795275, 200 / 3.7795275))
        a.setFrameOffsetFromReferencePointMm(QPointF(70 / 3.7795275, 90 / 3.7795275))
        html = TEST_DATA_DIR + "/test_html.html"
        a.setSourceFile(html)
        self.assertTrue(self.renderAnnotationInLayout("html_annotation_in_layout", a))

    def testHtmlAnnotationWithFeature(self):
        """test rendering a html annotation with a feature"""
        layer = QgsVectorLayer(
            "Point?crs=EPSG:3111&field=station:string&field=suburb:string",
            "test",
            "memory",
        )

        a = QgsHtmlAnnotation()
        a.fillSymbol().symbolLayer(0).setStrokeColor(QColor(0, 0, 0))
        a.markerSymbol().symbolLayer(0).setStrokeColor(QColor(0, 0, 0))
        a.setFrameSizeMm(QSizeF(400 / 3.7795275, 250 / 3.7795275))
        a.setFrameOffsetFromReferencePointMm(QPointF(70 / 3.7795275, 90 / 3.7795275))
        a.setMapLayer(layer)
        html = TEST_DATA_DIR + "/test_html_feature.html"
        a.setSourceFile(html)
        im = self.renderAnnotation(a, QPointF(20, 30))
        self.assertTrue(self.image_check("html_nofeature", "html_nofeature", im))
        f = QgsFeature(layer.fields())
        f.setValid(True)
        f.setAttributes(["hurstbridge", "somewhere"])
        a.setAssociatedFeature(f)
        im = self.renderAnnotation(a, QPointF(20, 30))
        self.assertTrue(self.image_check("html_feature", "html_feature", im))

    def testFormAnnotation(self):
        """test rendering a form annotation"""
        a = QgsFormAnnotation()
        a.fillSymbol().symbolLayer(0).setStrokeColor(QColor(0, 0, 0))
        a.markerSymbol().symbolLayer(0).setStrokeColor(QColor(0, 0, 0))
        a.setFrameSizeMm(QSizeF(400 / 3.7795275, 250 / 3.7795275))
        a.setFrameOffsetFromReferencePointMm(QPointF(70 / 3.7795275, 90 / 3.7795275))
        ui = TEST_DATA_DIR + "/test_form.ui"
        a.setDesignerForm(ui)
        im = self.renderAnnotation(a, QPointF(20, 30))
        self.assertTrue(self.image_check("form_annotation", "form_annotation", im))

        # check clone
        clone = a.clone()
        im = self.renderAnnotation(clone, QPointF(20, 30))
        self.assertTrue(self.image_check("form_annotation", "form_annotation", im))

    def testFormAnnotationInLayout(self):
        """test rendering a form annotation"""
        a = QgsFormAnnotation()
        a.fillSymbol().symbolLayer(0).setStrokeColor(QColor(0, 0, 0))
        a.markerSymbol().symbolLayer(0).setStrokeColor(QColor(0, 0, 0))
        a.setFrameSizeMm(QSizeF(400 / 3.7795275, 250 / 3.7795275))
        a.setFrameOffsetFromReferencePointMm(QPointF(70 / 3.7795275, 90 / 3.7795275))
        ui = TEST_DATA_DIR + "/test_form.ui"
        a.setDesignerForm(ui)
        self.assertTrue(self.renderAnnotationInLayout("form_annotation_in_layout", a))

    def testRelativePosition(self):
        """test rendering an annotation without map point"""
        a = QgsHtmlAnnotation()
        a.fillSymbol().symbolLayer(0).setStrokeColor(QColor(0, 0, 0))
        a.setFrameSizeMm(QSizeF(400 / 3.7795275, 250 / 3.7795275))
        a.setHasFixedMapPosition(False)
        html = TEST_DATA_DIR + "/test_html.html"
        a.setSourceFile(html)
        im = self.renderAnnotation(a, QPointF(20, 30))
        self.assertTrue(self.image_check("relative_style", "relative_style", im))

    def testMargins(self):
        """test rendering an annotation with margins"""
        a = QgsHtmlAnnotation()
        a.fillSymbol().symbolLayer(0).setStrokeColor(QColor(0, 0, 0))
        a.setFrameSizeMm(QSizeF(400 / 3.7795275, 250 / 3.7795275))
        a.setHasFixedMapPosition(False)
        a.setContentsMargin(QgsMargins(15, 10, 30, 20))
        html = TEST_DATA_DIR + "/test_html.html"
        a.setSourceFile(html)
        im = self.renderAnnotation(a, QPointF(20, 30))
        self.assertTrue(
            self.image_check("annotation_margins", "annotation_margins", im)
        )

    def testFillSymbol(self):
        """test rendering an annotation with fill symbol"""
        a = QgsTextAnnotation()
        a.setFrameSizeMm(QSizeF(400 / 3.7795275, 250 / 3.7795275))
        a.setHasFixedMapPosition(False)
        a.setFillSymbol(
            QgsFillSymbol.createSimple(
                {"color": "blue", "width_border": "5", "outline_color": "black"}
            )
        )
        im = self.renderAnnotation(a, QPointF(20, 30))
        self.assertTrue(
            self.image_check("annotation_fillstyle", "annotation_fillstyle", im)
        )

    def renderAnnotation(self, annotation, offset):
        image = QImage(600, 400, QImage.Format.Format_RGB32)
        image.fill(QColor(0, 0, 0, 0))
        QgsRenderChecker.drawBackground(image)

        painter = QPainter()
        ms = QgsMapSettings()
        ms.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        extent = QgsRectangle(0, 5, 40, 30)

        ms.setExtent(extent)
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI

        painter.begin(image)
        painter.translate(offset.x(), offset.y())
        annotation.render(context)
        painter.end()
        return image

    def renderAnnotationInLayout(self, test_name, annotation):
        pr = QgsProject()
        l = QgsLayout(pr)
        l.initializeDefaults()
        map = QgsLayoutItemMap(l)
        map.attemptSetSceneRect(QRectF(20, 20, 200, 100))
        map.setFrameEnabled(True)
        rectangle = QgsRectangle(0, 0, 18, 8)
        map.setExtent(rectangle)
        l.addLayoutItem(map)

        annotation.setMapPosition(QgsPointXY(1, 7))
        annotation.setHasFixedMapPosition(True)
        pr.annotationManager().addAnnotation(annotation)

        return self.render_layout_check(
            test_name, layout=l, size=QSize(1122 * 2, 794 * 2)
        )


if __name__ == "__main__":
    unittest.main()
