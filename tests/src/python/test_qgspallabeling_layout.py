"""QGIS unit tests for QgsPalLabeling: label rendering output via layout

From build dir, run: ctest -R PyQgsPalLabelingLayout -V

See <qgis-src-dir>/tests/testdata/labeling/README.rst for description.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Larry Shaffer"
__date__ = "2014/02/21"
__copyright__ = "Copyright 2013, The QGIS Project"

import os
import subprocess
import sys

from qgis.PyQt.QtCore import QRect, QRectF, QSize, qDebug
from qgis.PyQt.QtGui import QColor, QImage, QPainter
from qgis.PyQt.QtSvg import QSvgGenerator, QSvgRenderer
from qgis.core import (
    QgsLabelingEngineSettings,
    QgsLayout,
    QgsLayoutExporter,
    QgsLayoutItemMap,
    QgsLayoutItemPage,
    QgsLayoutSize,
    QgsMapSettings,
    QgsProject,
    QgsVectorLayerSimpleLabeling,
)

from test_qgspallabeling_base import TestQgsPalLabeling, runSuite
from test_qgspallabeling_tests import TestLineBase, TestPointBase, suiteTests
from utilities import getExecutablePath, getTempfilePath, mapSettingsString

# PDF-to-image utility
# look for Poppler w/ Cairo, then muPDF
# * Poppler w/ Cairo renders correctly
# * Poppler w/o Cairo does not always correctly render vectors in PDF to image
# * muPDF renders correctly, but slightly shifts colors
for util in [
    "pdftocairo",
    # 'mudraw',
]:
    PDFUTIL = getExecutablePath(util)
    if PDFUTIL:
        break

# noinspection PyUnboundLocalVariable
if not PDFUTIL:
    raise Exception(
        "PDF-to-image utility not found on PATH: " "install Poppler (with Cairo)"
    )


# output kind enum
# noinspection PyClassHasNoInit
class OutputKind:
    Img, Svg, Pdf = list(range(3))


# noinspection PyShadowingNames
class TestLayoutBase(TestQgsPalLabeling):

    layer = None
    """:type: QgsVectorLayer"""

    @classmethod
    def setUpClass(cls):
        if not cls._BaseSetup:
            TestQgsPalLabeling.setUpClass()
        # the blue background (set via layer style) to match renderchecker's
        TestQgsPalLabeling.loadFeatureLayer("background", True)
        cls._TestKind = 0  # OutputKind.(Img|Svg|Pdf)
        cls._test_base_name = ""

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        super().tearDownClass()
        cls.removeMapLayer(cls.layer)
        cls.layer = None

    def setUp(self):
        """Run before each test."""
        super().setUp()
        # ensure per test map settings stay encapsulated
        self._TestMapSettings = self.cloneMapSettings(self._MapSettings)

    def _set_up_composition(self, width, height, dpi, engine_settings):
        # set up layout and add map
        self._c = QgsLayout(QgsProject.instance())
        """:type: QgsLayout"""
        # self._c.setUseAdvancedEffects(False)
        self._c.renderContext().setDpi(dpi)
        # 600 x 400 px = 211.67 x 141.11 mm @ 72 dpi
        paperw = width * 25.4 / dpi
        paperh = height * 25.4 / dpi
        page = QgsLayoutItemPage(self._c)
        page.attemptResize(QgsLayoutSize(paperw, paperh))
        self._c.pageCollection().addPage(page)
        # NOTE: do not use QgsLayoutItemMap(self._c, 0, 0, paperw, paperh) since
        # it only takes integers as parameters and the composition will grow
        # larger based upon union of item scene rectangles and a slight buffer
        #   see end of QgsComposition::compositionBounds()
        # add map as small graphics item first, then set its scene QRectF later
        self._cmap = QgsLayoutItemMap(self._c)
        self._cmap.attemptSetSceneRect(QRectF(10, 10, 10, 10))
        """:type: QgsLayoutItemMap"""
        self._cmap.setFrameEnabled(False)
        self._cmap.setLayers(self._TestMapSettings.layers())
        if (
            self._TestMapSettings.labelingEngineSettings().flags()
            & QgsLabelingEngineSettings.Flag.UsePartialCandidates
        ):
            self._cmap.setMapFlags(QgsLayoutItemMap.MapItemFlag.ShowPartialLabels)
        self._c.addLayoutItem(self._cmap)
        # now expand map to fill page and set its extent
        self._cmap.attemptSetSceneRect(QRectF(0, 0, paperw, paperw))
        self._cmap.setExtent(self.aoiExtent())
        # self._cmap.updateCachedImage()
        # composition takes labeling engine settings from project
        QgsProject.instance().setLabelingEngineSettings(engine_settings)

    # noinspection PyUnusedLocal
    def _get_layout_image(self, width, height, dpi):
        image = QImage(QSize(width, height), self._TestMapSettings.outputImageFormat())
        image.fill(QColor(152, 219, 249).rgb())
        image.setDotsPerMeterX(int(dpi / 25.4 * 1000))
        image.setDotsPerMeterY(int(dpi / 25.4 * 1000))

        p = QPainter(image)
        p.setRenderHint(
            QPainter.RenderHint.Antialiasing,
            self._TestMapSettings.testFlag(QgsMapSettings.Flag.Antialiasing),
        )
        exporter = QgsLayoutExporter(self._c)
        exporter.renderPage(p, 0)
        p.end()

        # image = self._c.printPageAsRaster(0)
        # """:type: QImage"""

        return image

    def _get_layout_svg_image(self, width, height, dpi):
        svgpath = getTempfilePath("svg")
        temp_size = os.path.getsize(svgpath)

        svg_g = QSvgGenerator()
        # noinspection PyArgumentList
        svg_g.setTitle(QgsProject.instance().title())
        svg_g.setFileName(svgpath)
        svg_g.setSize(QSize(width, height))
        svg_g.setViewBox(QRect(0, 0, width, height))
        svg_g.setResolution(int(dpi))

        sp = QPainter(svg_g)
        exporter = QgsLayoutExporter(self._c)
        exporter.renderPage(sp, 0)
        sp.end()

        if temp_size == os.path.getsize(svgpath):
            return False, ""

        image = QImage(width, height, self._TestMapSettings.outputImageFormat())
        image.fill(QColor(152, 219, 249).rgb())
        image.setDotsPerMeterX(int(dpi / 25.4 * 1000))
        image.setDotsPerMeterY(int(dpi / 25.4 * 1000))

        svgr = QSvgRenderer(svgpath)
        p = QPainter(image)
        p.setRenderHint(
            QPainter.RenderHint.Antialiasing,
            self._TestMapSettings.testFlag(QgsMapSettings.Flag.Antialiasing),
        )
        p.setRenderHint(QPainter.RenderHint.TextAntialiasing)
        svgr.render(p)
        p.end()

        return image

    def _get_layout_pdf_image(self, width, height, dpi):
        pdfpath = getTempfilePath("pdf")
        temp_size = os.path.getsize(pdfpath)

        exporter = QgsLayoutExporter(self._c)
        settings = QgsLayoutExporter.PdfExportSettings()
        settings.dpi = int(self._c.renderContext().dpi())
        exporter.exportToPdf(pdfpath, settings)

        if temp_size == os.path.getsize(pdfpath):
            return False, ""

        filepath = getTempfilePath("png")
        # Poppler (pdftocairo or pdftoppm):
        # PDFUTIL -png -singlefile -r 72 -x 0 -y 0 -W 420 -H 280 in.pdf pngbase
        # muPDF (mudraw):
        # PDFUTIL -c rgb[a] -r 72 -w 420 -h 280 -o out.png in.pdf
        if PDFUTIL.strip().endswith("pdftocairo"):
            filebase = os.path.join(
                os.path.dirname(filepath),
                os.path.splitext(os.path.basename(filepath))[0],
            )
            call = [
                PDFUTIL,
                "-png",
                "-singlefile",
                "-r",
                str(dpi),
                "-x",
                "0",
                "-y",
                "0",
                "-W",
                str(width),
                "-H",
                str(height),
                pdfpath,
                filebase,
            ]
        elif PDFUTIL.strip().endswith("mudraw"):
            call = [
                PDFUTIL,
                "-c",
                "rgba",
                "-r",
                str(dpi),
                "-w",
                str(width),
                "-h",
                str(height),
                # '-b', '8',
                "-o",
                filepath,
                pdfpath,
            ]
        else:
            return False, ""

        qDebug(f"_get_layout_pdf_image call: {' '.join(call)}")
        res = False
        try:
            subprocess.check_call(call)
            res = True
        except subprocess.CalledProcessError as e:
            qDebug(
                "_get_layout_pdf_image failed!\n"
                "cmd: {}\n"
                "returncode: {}\n"
                "message: {}".format(e.cmd, e.returncode, e.message)
            )

        if not res:
            os.unlink(filepath)
            filepath = ""

        return QImage(filepath)

    def get_layout_output(self, kind) -> QImage:
        ms = self._TestMapSettings
        osize = ms.outputSize()
        width, height, dpi = osize.width(), osize.height(), ms.outputDpi()
        self._set_up_composition(width, height, dpi, ms.labelingEngineSettings())
        if kind == OutputKind.Svg:
            return self._get_layout_svg_image(width, height, dpi)
        elif kind == OutputKind.Pdf:
            return self._get_layout_pdf_image(width, height, dpi)
        else:  # OutputKind.Img
            return self._get_layout_image(width, height, dpi)

    # noinspection PyUnusedLocal
    def checkTest(self, **kwargs):
        self.layer.setLabeling(QgsVectorLayerSimpleLabeling(self.lyr))

        image = self.get_layout_output(self._TestKind)

        self.assertTrue(
            self.image_check(
                f"{self._test_base_name}{self._TestGroupPrefix}_{self._Test}",
                self._Test,
                image,
                self._Test,
                color_tolerance=0,
                allowed_mismatch=0,
                control_path_prefix="expected_" + self._TestGroupPrefix,
            )
        )


class TestLayoutPointBase(TestLayoutBase):

    @classmethod
    def setUpClass(cls):
        TestLayoutBase.setUpClass()
        cls.layer = TestQgsPalLabeling.loadFeatureLayer("point")


class TestLayoutImagePoint(TestLayoutPointBase, TestPointBase):

    def setUp(self):
        """Run before each test."""
        super().setUp()
        self._TestKind = OutputKind.Img
        self._test_base_name = "layout_image"
        self.configTest("pal_composer", "sp_img")


class TestLayoutImageVsCanvasPoint(TestLayoutPointBase, TestPointBase):

    def setUp(self):
        """Run before each test."""
        super().setUp()
        self._TestKind = OutputKind.Img
        self._test_base_name = "layout_image_v_canvas"
        self.configTest("pal_canvas", "sp")


class TestLayoutSvgPoint(TestLayoutPointBase, TestPointBase):

    def setUp(self):
        """Run before each test."""
        super().setUp()
        self._TestKind = OutputKind.Svg
        self._test_base_name = "layout_svg"
        self.configTest("pal_composer", "sp_svg")


class TestLayoutSvgVsLayoutPoint(TestLayoutPointBase, TestPointBase):
    """
    Compare only to layout image, which is already compared to canvas point
    """

    def setUp(self):
        """Run before each test."""
        super().setUp()
        self._TestKind = OutputKind.Svg
        self._test_base_name = "layout_svg_v_img"
        self.configTest("pal_composer", "sp_img")


class TestLayoutPdfPoint(TestLayoutPointBase, TestPointBase):

    def setUp(self):
        """Run before each test."""
        super().setUp()
        self._test_base_name = "layout_pdf"
        self._TestKind = OutputKind.Pdf
        self.configTest("pal_composer", "sp_pdf")


class TestLayoutPdfVsLayoutPoint(TestLayoutPointBase, TestPointBase):
    """
    Compare only to layout image, which is already compared to canvas point
    """

    def setUp(self):
        """Run before each test."""
        super().setUp()
        self._test_base_name = "layout_pdf_v_img"
        self._TestKind = OutputKind.Pdf
        self.configTest("pal_composer", "sp_img")


class TestLayoutLineBase(TestLayoutBase):

    @classmethod
    def setUpClass(cls):
        TestLayoutBase.setUpClass()
        cls.layer = TestQgsPalLabeling.loadFeatureLayer("line")


class TestLayoutImageLine(TestLayoutLineBase, TestLineBase):

    def setUp(self):
        """Run before each test."""
        super().setUp()
        self._test_base_name = "layout_img"
        self._TestKind = OutputKind.Img
        self.configTest("pal_composer_line", "sp_img")


class TestLayoutImageVsCanvasLine(TestLayoutLineBase, TestLineBase):

    def setUp(self):
        """Run before each test."""
        super().setUp()
        self._test_base_name = "layout_img_v_canvas"
        self._TestKind = OutputKind.Img
        self.configTest("pal_canvas_line", "sp")


class TestLayoutSvgLine(TestLayoutLineBase, TestLineBase):

    def setUp(self):
        """Run before each test."""
        super().setUp()
        self._test_base_name = "layout_svg"
        self._TestKind = OutputKind.Svg
        self.configTest("pal_composer_line", "sp_svg")


class TestLayoutSvgVsLayoutLine(TestLayoutLineBase, TestLineBase):
    """
    Compare only to layout image, which is already compared to canvas line
    """

    def setUp(self):
        """Run before each test."""
        super().setUp()
        self._test_base_name = "layout_svg_v_img"
        self._TestKind = OutputKind.Svg
        self.configTest("pal_composer_line", "sp_img")


class TestLayoutPdfLine(TestLayoutLineBase, TestLineBase):

    def setUp(self):
        """Run before each test."""
        super().setUp()
        self._test_base_name = "layout_pdf"
        self._TestKind = OutputKind.Pdf
        self.configTest("pal_composer_line", "sp_pdf")


class TestLayoutPdfVsLayoutLine(TestLayoutLineBase, TestLineBase):
    """
    Compare only to layout image, which is already compared to canvas line
    """

    def setUp(self):
        """Run before each test."""
        super().setUp()
        self._TestKind = OutputKind.Pdf
        self._test_base_name = "layout_pdf_v_img"
        self.configTest("pal_composer_line", "sp_img")


if __name__ == "__main__":
    # NOTE: unless PAL_SUITE env var is set all test class methods will be run
    # SEE: test_qgspallabeling_tests.suiteTests() to define suite
    st = suiteTests()
    sp_i = ["TestLayoutImagePoint." + t for t in st["sp_suite"]]
    sp_ivs = ["TestLayoutImageVsCanvasPoint." + t for t in st["sp_vs_suite"]]
    sp_s = ["TestLayoutSvgPoint." + t for t in st["sp_suite"]]
    sp_svs = ["TestLayoutSvgVsLayoutPoint." + t for t in st["sp_vs_suite"]]
    sp_p = ["TestLayoutPdfPoint." + t for t in st["sp_suite"]]
    sp_pvs = ["TestLayoutPdfVsLayoutPoint." + t for t in st["sp_vs_suite"]]
    suite = []

    # extended separately for finer control of PAL_SUITE (comment-out undesired)
    suite.extend(sp_i)
    suite.extend(sp_ivs)
    suite.extend(sp_s)
    suite.extend(sp_svs)
    suite.extend(sp_p)
    suite.extend(sp_pvs)

    res = runSuite(sys.modules[__name__], suite)
    sys.exit(not res.wasSuccessful())
