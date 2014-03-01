# -*- coding: utf-8 -*-
"""QGIS unit tests for QgsPalLabeling: label rendering to composer

From build dir: ctest -R PyQgsPalLabelingComposer -V
Set the following env variables when manually running tests:
  PAL_SUITE to run specific tests (define in __main__)
  PAL_VERBOSE to output individual test summary
  PAL_CONTROL_IMAGE to trigger building of new control images
  PAL_REPORT to open any failed image check reports in web browser

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = 'Larry Shaffer'
__date__ = '2014/02/21'
__copyright__ = 'Copyright 2013, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import sys
import os
import subprocess
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4.QtSvg import QSvgRenderer, QSvgGenerator

from qgis.core import *

from utilities import (
    unittest,
    expectedFailure,
    getTempfilePath,
    getExecutablePath,
)

from test_qgspallabeling_base import TestQgsPalLabeling, runSuite
from test_qgspallabeling_tests import (
    TestPointBase,
    suiteTests
)

# look for Poppler, then muPDF PDF-to-image utility
for util in ['pdftoppm', 'mudraw']:
    PDFUTIL = getExecutablePath(util)
    if PDFUTIL:
        break


def skip_if_not_pdf_util():  # skip test class decorator
    if PDFUTIL:
        return lambda func: func
    return unittest.skip('\nPDF-to-image utility not found on PATH\n'
                         'Install Poppler or muPDF utilities\n\n')


# output kind enum
# noinspection PyClassHasNoInit
class OutputKind():
    Img, Svg, Pdf = range(3)


# noinspection PyShadowingNames
class TestComposerBase(TestQgsPalLabeling):

    layer = None
    """:type: QgsVectorLayer"""

    @classmethod
    def setUpClass(cls):
        if not cls._BaseSetup:
            TestQgsPalLabeling.setUpClass()
        # the blue background (set via layer style) to match renderchecker's
        TestQgsPalLabeling.loadFeatureLayer('background', True)
        cls._CheckMismatch = 0  # mismatch expected for crosscheck
        cls._TestImage = ''
        cls._TestKind = ''  # img|svg|pdf
        cls._ImgW = 600
        cls._ImgH = 400
        cls._ImgDpi = 72

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        TestQgsPalLabeling.tearDownClass()
        cls._MapRegistry.removeMapLayer(cls.layer.id())
        cls.layer = None

    def setUp(self):
        """Run before each test."""
        TestQgsPalLabeling.setDefaultEngineSettings()
        self.lyr = self.defaultLayerSettings()
        self._TestImage = ''

    # noinspection PyUnusedLocal
    def checkTest(self, **kwargs):
        self.lyr.writeToLayer(self.layer)
        res_m, self._TestImage = self.get_composer_output(self._TestKind)
        self.saveContolImage(self._TestImage)
        self.assertTrue(res_m, 'Failed to retrieve/save output from composer')
        self.assertTrue(*self.renderCheck(mismatch=self._CheckMismatch,
                                          imgpath=self._TestImage))

    def _set_up_composition(self, width, height, dpi):
        # set up composition and add map
        # TODO: how to keep embedded map from being anti-aliased twice?
        # self._MapSettings.setFlag(QgsMapSettings.Antialiasing, False)
        self._c = QgsComposition(self._MapSettings)
        """:type: QgsComposition"""
        self._c.setPrintResolution(dpi)
        # 600 x 400 px = 211.67 x 141.11 mm @ 72 dpi
        # TODO: figure out why this doesn't work and needs fudging
        #       probably need sets of fudgyness per dpi group (72, 150, 300)?
        paperw = round((width * 25.4 / dpi) + 0.05, 0)
        paperh = round((height * 25.4 / dpi) + 0.05, 1)
        self._c.setPaperSize(paperw, paperh)
        self._cmap = QgsComposerMap(
            self._c, 0, 0, self._c.paperWidth(), self._c.paperHeight())
        """:type: QgsComposerMap"""
        self._cmap.setFrameEnabled(False)
        self._c.addComposerMap(self._cmap)
        self._cmap.setNewExtent(self.aoiExtent())
        self._c.setPlotStyle(QgsComposition.Print)

    # noinspection PyUnusedLocal
    def _get_composer_image(self, width, height, dpi):
        # dpi = self._c.printResolution()  # why did I add this before?
        dpmm = dpi / 25.4
        img_width = int(dpmm * self._c.paperWidth())
        img_height = int(dpmm * self._c.paperHeight())

        # create output image and initialize it
        image = QImage(QSize(img_width, img_height), QImage.Format_ARGB32)
        image.setDotsPerMeterX(dpmm * 1000)
        image.setDotsPerMeterY(dpmm * 1000)
        image.fill(QColor(152, 219, 249).rgb())

        # render the composition
        p = QPainter(image)
        p.setRenderHint(QPainter.HighQualityAntialiasing, False)
        p.setRenderHint(QPainter.SmoothPixmapTransform, False)
        p.setRenderHint(QPainter.Antialiasing, False)
        p.setRenderHint(QPainter.TextAntialiasing, False)
        src = QRectF(0, 0, self._c.paperWidth(), self._c.paperHeight())
        trgt = QRectF(0, 0, img_width, img_height)
        self._c.render(p, trgt, src)
        p.end()

        filepath = getTempfilePath('png')
        res = image.save(filepath, 'png')
        if not res:
            os.unlink(filepath)
            filepath = ''

        return res, filepath

    def _get_composer_svg_image(self, width, height, dpi):
        # from qgscomposer.cpp, QgsComposer::on_mActionExportAsSVG_triggered,
        # line 1909, near end of function
        svgpath = getTempfilePath('svg')
        temp_size = os.path.getsize(svgpath)
        svg_g = QSvgGenerator()
        # noinspection PyArgumentList
        svg_g.setTitle(QgsProject.instance().title())
        svg_g.setFileName(svgpath)
        # width and height in pixels
        svg_w = int(self._c.paperWidth() * self._c.printResolution() / 25.4)
        svg_h = int(self._c.paperHeight() * self._c.printResolution() / 25.4)
        svg_g.setSize(QSize(svg_w, svg_h))
        svg_g.setViewBox(QRect(0, 0, svg_w, svg_h))
        # because the rendering is done in mm, convert the dpi
        svg_g.setResolution(self._c.printResolution())

        sp = QPainter(svg_g)
        self._c.renderPage(sp, 0)
        sp.end()

        if temp_size == os.path.getsize(svgpath):
            # something went pear-shaped
            return False, ''

        svgr = QSvgRenderer(svgpath)
        image = QImage(width, height, QImage.Format_ARGB32)
        image.fill(QColor(152, 219, 249).rgb())
        image.setDotsPerMeterX(dpi/25.4 * 1000)
        image.setDotsPerMeterY(dpi/25.4 * 1000)
        p = QPainter(image)
        svgr.render(p)
        p.end()

        filepath = getTempfilePath('png')
        res = image.save(filepath, 'png')
        if not res:
            os.unlink(filepath)
            filepath = ''
        # TODO: remove .svg file as well?

        return res, filepath

    def _get_composer_pdf_image(self, width, height, dpi):
        pdfpath = getTempfilePath('pdf')
        temp_size = os.path.getsize(pdfpath)
        p = QPrinter()
        p.setOutputFormat(QPrinter.PdfFormat)
        p.setOutputFileName(pdfpath)
        p.setPaperSize(QSizeF(self._c.paperWidth(), self._c.paperHeight()),
                       QPrinter.Millimeter)
        p.setFullPage(True)
        p.setColorMode(QPrinter.Color)
        p.setResolution(self._c.printResolution())

        pdf_p = QPainter(p)
        page_mm = p.pageRect(QPrinter.Millimeter)
        page_px = p.pageRect(QPrinter.DevicePixel)
        self._c.render(pdf_p, page_px, page_mm)
        pdf_p.end()

        if temp_size == os.path.getsize(pdfpath):
            # something went pear-shaped
            return False, ''

        filepath = getTempfilePath('png')
        filebase = os.path.join(os.path.dirname(filepath),
                                os.path.splitext(os.path.basename(filepath))[0])
        # pdftoppm -singlefile -r 72 -x 0 -y 0 -W 600 -H 400 -png in.pdf pngbase
        # mudraw -o out.png -r 72 -w 600 -h 400 -c rgb[a] in.pdf
        if PDFUTIL == 'pdftoppm':
            call = [
                'pdftoppm', '-singlefile',
                '-r', str(dpi),
                '-x', str(0), '-y', str(0),
                '-W', str(width), '-H', str(height),
                '-png', pdfpath, filebase
            ]
        elif PDFUTIL == 'mudraw':
            call = [
                'mudraw'
                '-o', filepath,
                '-c', 'rgba',
                '-r', str(dpi),
                '-w', str(width), '-h', str(height),
                pdfpath
            ]
        else:
            return False, ''

        res = subprocess.check_call(call)

        if not res:
            os.unlink(filepath)
            filepath = ''

        return res, filepath

    def get_composer_output(self, kind):
        width, height, dpi = self._ImgW, self._ImgH, self._ImgDpi
        self._set_up_composition(width, height, dpi)
        if kind == OutputKind.Svg:
            return self._get_composer_svg_image(width, height, dpi)
        elif kind == OutputKind.Pdf:
            return self._get_composer_pdf_image(width, height, dpi)
        else:  # 'img'
            return self._get_composer_image(width, height, dpi)


class TestComposerPointBase(TestComposerBase, TestPointBase):

    @classmethod
    def setUpClass(cls):
        TestComposerBase.setUpClass()
        cls.layer = TestQgsPalLabeling.loadFeatureLayer('point')


class TestComposerImagePoint(TestComposerPointBase):

    def setUp(self):
        """Run before each test."""
        super(TestComposerImagePoint, self).setUp()
        self._TestKind = OutputKind.Img
        self.configTest('pal_composer', 'sp_img')
        # TODO: due to double antialiasing?
        self._CheckMismatch = 2700  # comment to PAL_REPORT difference


class TestComposerImageVsCanvasPoint(TestComposerPointBase):

    def setUp(self):
        """Run before each test."""
        super(TestComposerImageVsCanvasPoint, self).setUp()
        self._TestKind = OutputKind.Img
        self.configTest('pal_canvas', 'sp')
        # TODO: due to double antialiasing?
        if 'test_background_svg' in self.id():
            self._CheckMismatch = 3600
            # self._CheckMismatch = 0  # uncomment to PAL_REPORT difference


if __name__ == '__main__':
    # NOTE: unless PAL_SUITE env var is set all test class methods will be run
    # SEE: test_qgspallabeling_tests.suiteTests() to define suite
    st = suiteTests()
    sp_i = ['TestComposerImagePoint.' + t for t in st['sp_suite']]
    sp_ivs = ['TestComposerImageVsCanvasPoint.' + t for t in st['sp_vs_suite']]
    suite = []
    # extended separately for finer control of PAL_SUITE (comment-out undesired)
    suite.extend(sp_i)
    suite.extend(sp_ivs)
    res = runSuite(sys.modules[__name__], suite)
    sys.exit(not res.wasSuccessful())
