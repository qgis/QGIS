# -*- coding: utf-8 -*-
"""QGIS unit tests for QgsPalLabeling: label rendering output via composer

From build dir, run: ctest -R PyQgsPalLabelingComposer -V

See <qgis-src-dir>/tests/testdata/labeling/README.rst for description.

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

import qgis
import sys
import os
import subprocess

from PyQt4.QtCore import QRect, QRectF, QSize, QSizeF, qDebug
from PyQt4.QtGui import QImage, QColor, QPainter, QPrinter
from PyQt4.QtSvg import QSvgRenderer, QSvgGenerator

from qgis.core import QgsComposition, QgsMapSettings, QgsProject, QgsComposerMap

from utilities import (
    getTempfilePath,
    getExecutablePath,
    mapSettingsString
)

from test_qgspallabeling_base import TestQgsPalLabeling, runSuite
from test_qgspallabeling_tests import (
    TestPointBase,
    suiteTests
)

# PDF-to-image utility
# look for Poppler w/ Cairo, then muPDF
# * Poppler w/ Cairo renders correctly
# * Poppler w/o Cairo does not always correctly render vectors in PDF to image
# * muPDF renders correctly, but sightly shifts colors
for util in [
    'pdftocairo',
    # 'mudraw',
]:
    PDFUTIL = getExecutablePath(util)
    if PDFUTIL:
        break

# noinspection PyUnboundLocalVariable
if not PDFUTIL:
    raise Exception('PDF-to-image utility not found on PATH: '
                    'install Poppler (with Cairo)')


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
        cls._TestKind = 0  # OutputKind.(Img|Svg|Pdf)

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        TestQgsPalLabeling.tearDownClass()
        cls.removeMapLayer(cls.layer)
        cls.layer = None

    def setUp(self):
        """Run before each test."""
        super(TestComposerBase, self).setUp()
        self._TestImage = ''
        # ensure per test map settings stay encapsulated
        self._TestMapSettings = self.cloneMapSettings(self._MapSettings)
        self._Mismatch = 0
        self._ColorTol = 0
        self._Mismatches.clear()
        self._ColorTols.clear()

    def _set_up_composition(self, width, height, dpi):
        # set up composition and add map
        self._c = QgsComposition(self._TestMapSettings)
        """:type: QgsComposition"""
        # self._c.setUseAdvancedEffects(False)
        self._c.setPrintResolution(dpi)
        # 600 x 400 px = 211.67 x 141.11 mm @ 72 dpi
        paperw = width * 25.4 / dpi
        paperh = height * 25.4 / dpi
        self._c.setPaperSize(paperw, paperh)
        # NOTE: do not use QgsComposerMap(self._c, 0, 0, paperw, paperh) since
        # it only takes integers as parameters and the composition will grow
        # larger based upon union of item scene rectangles and a slight buffer
        #   see end of QgsComposition::compositionBounds()
        # add map as small graphics item first, then set its scene QRectF later
        self._cmap = QgsComposerMap(self._c, 10, 10, 10, 10)
        """:type: QgsComposerMap"""
        self._cmap.setPreviewMode(QgsComposerMap.Render)
        self._cmap.setFrameEnabled(False)
        self._c.addComposerMap(self._cmap)
        # now expand map to fill page and set its extent
        self._cmap.setSceneRect(QRectF(0, 0, paperw, paperw))
        self._cmap.setNewExtent(self.aoiExtent())
        # self._cmap.updateCachedImage()
        self._c.setPlotStyle(QgsComposition.Print)

    # noinspection PyUnusedLocal
    def _get_composer_image(self, width, height, dpi):
        image = QImage(QSize(width, height),
                       self._TestMapSettings.outputImageFormat())
        image.fill(QColor(152, 219, 249).rgb())
        image.setDotsPerMeterX(dpi / 25.4 * 1000)
        image.setDotsPerMeterY(dpi / 25.4 * 1000)

        p = QPainter(image)
        p.setRenderHint(
            QPainter.Antialiasing,
            self._TestMapSettings.testFlag(QgsMapSettings.Antialiasing)
        )
        self._c.renderPage(p, 0)
        p.end()

        # image = self._c.printPageAsRaster(0)
        # """:type: QImage"""

        if image.isNull():
            return False, ''

        filepath = getTempfilePath('png')
        res = image.save(filepath, 'png')
        if not res:
            os.unlink(filepath)
            filepath = ''

        return res, filepath

    def _get_composer_svg_image(self, width, height, dpi):
        # from qgscomposer.cpp, QgsComposer::on_mActionExportAsSVG_triggered,
        # near end of function
        svgpath = getTempfilePath('svg')
        temp_size = os.path.getsize(svgpath)

        svg_g = QSvgGenerator()
        # noinspection PyArgumentList
        svg_g.setTitle(QgsProject.instance().title())
        svg_g.setFileName(svgpath)
        svg_g.setSize(QSize(width, height))
        svg_g.setViewBox(QRect(0, 0, width, height))
        svg_g.setResolution(dpi)

        sp = QPainter(svg_g)
        self._c.renderPage(sp, 0)
        sp.end()

        if temp_size == os.path.getsize(svgpath):
            return False, ''

        image = QImage(width, height, self._TestMapSettings.outputImageFormat())
        image.fill(QColor(152, 219, 249).rgb())
        image.setDotsPerMeterX(dpi / 25.4 * 1000)
        image.setDotsPerMeterY(dpi / 25.4 * 1000)

        svgr = QSvgRenderer(svgpath)
        p = QPainter(image)
        p.setRenderHint(
            QPainter.Antialiasing,
            self._TestMapSettings.testFlag(QgsMapSettings.Antialiasing)
        )
        p.setRenderHint(QPainter.TextAntialiasing)
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
        # page_mm = p.pageRect(QPrinter.Millimeter)
        # page_px = p.pageRect(QPrinter.DevicePixel)
        # self._c.render(pdf_p, page_px, page_mm)
        self._c.renderPage(pdf_p, 0)
        pdf_p.end()

        if temp_size == os.path.getsize(pdfpath):
            return False, ''

        filepath = getTempfilePath('png')
        # Poppler (pdftocairo or pdftoppm):
        # PDFUTIL -png -singlefile -r 72 -x 0 -y 0 -W 420 -H 280 in.pdf pngbase
        # muPDF (mudraw):
        # PDFUTIL -c rgb[a] -r 72 -w 420 -h 280 -o out.png in.pdf
        if PDFUTIL.strip().endswith('pdftocairo'):
            filebase = os.path.join(
                os.path.dirname(filepath),
                os.path.splitext(os.path.basename(filepath))[0]
            )
            call = [
                PDFUTIL, '-png', '-singlefile', '-r', str(dpi),
                '-x', '0', '-y', '0', '-W', str(width), '-H', str(height),
                pdfpath, filebase
            ]
        elif PDFUTIL.strip().endswith('mudraw'):
            call = [
                PDFUTIL,  '-c', 'rgba',
                '-r', str(dpi), '-w', str(width), '-h', str(height),
                # '-b', '8',
                '-o', filepath, pdfpath
            ]
        else:
            return False, ''

        qDebug("_get_composer_pdf_image call: {0}".format(' '.join(call)))
        res = False
        try:
            subprocess.check_call(call)
            res = True
        except subprocess.CalledProcessError as e:
            qDebug("_get_composer_pdf_image failed!\n"
                   "cmd: {0}\n"
                   "returncode: {1}\n"
                   "message: {2}".format(e.cmd, e.returncode, e.message))

        if not res:
            os.unlink(filepath)
            filepath = ''

        return res, filepath

    def get_composer_output(self, kind):
        ms = self._TestMapSettings
        osize = ms.outputSize()
        width, height, dpi = osize.width(), osize.height(), ms.outputDpi()
        self._set_up_composition(width, height, dpi)
        if kind == OutputKind.Svg:
            return self._get_composer_svg_image(width, height, dpi)
        elif kind == OutputKind.Pdf:
            return self._get_composer_pdf_image(width, height, dpi)
        else:  # OutputKind.Img
            return self._get_composer_image(width, height, dpi)

    # noinspection PyUnusedLocal
    def checkTest(self, **kwargs):
        self.lyr.writeToLayer(self.layer)

        ms = self._MapSettings  # class settings
        settings_type = 'Class'
        if self._TestMapSettings is not None:
            ms = self._TestMapSettings  # per test settings
            settings_type = 'Test'
        if 'PAL_VERBOSE' in os.environ:
            qDebug('MapSettings type: {0}'.format(settings_type))
            qDebug(mapSettingsString(ms))

        res_m, self._TestImage = self.get_composer_output(self._TestKind)
        self.assertTrue(res_m, 'Failed to retrieve/save output from composer')
        self.saveControlImage(self._TestImage)
        mismatch = 0
        if 'PAL_NO_MISMATCH' not in os.environ:
            # some mismatch expected
            mismatch = self._Mismatch if self._Mismatch else 20
            if self._TestGroup in self._Mismatches:
                mismatch = self._Mismatches[self._TestGroup]
        colortol = 0
        if 'PAL_NO_COLORTOL' not in os.environ:
            colortol = self._ColorTol if self._ColorTol else 0
            if self._TestGroup in self._ColorTols:
                colortol = self._ColorTols[self._TestGroup]
        self.assertTrue(*self.renderCheck(mismatch=mismatch,
                                          colortol=colortol,
                                          imgpath=self._TestImage))


class TestComposerPointBase(TestComposerBase):

    @classmethod
    def setUpClass(cls):
        TestComposerBase.setUpClass()
        cls.layer = TestQgsPalLabeling.loadFeatureLayer('point')


class TestComposerImagePoint(TestComposerPointBase, TestPointBase):

    def setUp(self):
        """Run before each test."""
        super(TestComposerImagePoint, self).setUp()
        self._TestKind = OutputKind.Img
        self.configTest('pal_composer', 'sp_img')


class TestComposerImageVsCanvasPoint(TestComposerPointBase, TestPointBase):

    def setUp(self):
        """Run before each test."""
        super(TestComposerImageVsCanvasPoint, self).setUp()
        self._TestKind = OutputKind.Img
        self.configTest('pal_canvas', 'sp')


class TestComposerSvgPoint(TestComposerPointBase, TestPointBase):

    def setUp(self):
        """Run before each test."""
        super(TestComposerSvgPoint, self).setUp()
        self._TestKind = OutputKind.Svg
        self.configTest('pal_composer', 'sp_svg')


class TestComposerSvgVsComposerPoint(TestComposerPointBase, TestPointBase):
    """
    Compare only to composer image, which is already compared to canvas point
    """
    def setUp(self):
        """Run before each test."""
        super(TestComposerSvgVsComposerPoint, self).setUp()
        self._TestKind = OutputKind.Svg
        self.configTest('pal_composer', 'sp_img')
        self._ColorTol = 4


class TestComposerPdfPoint(TestComposerPointBase, TestPointBase):

    def setUp(self):
        """Run before each test."""
        super(TestComposerPdfPoint, self).setUp()
        self._TestKind = OutputKind.Pdf
        self.configTest('pal_composer', 'sp_pdf')


class TestComposerPdfVsComposerPoint(TestComposerPointBase, TestPointBase):
    """
    Compare only to composer image, which is already compared to canvas point
    """
    def setUp(self):
        """Run before each test."""
        super(TestComposerPdfVsComposerPoint, self).setUp()
        self._TestKind = OutputKind.Pdf
        self.configTest('pal_composer', 'sp_img')
        self._Mismatch = 50
        self._ColorTol = 18


if __name__ == '__main__':
    # NOTE: unless PAL_SUITE env var is set all test class methods will be run
    # SEE: test_qgspallabeling_tests.suiteTests() to define suite
    st = suiteTests()
    sp_i = ['TestComposerImagePoint.' + t for t in st['sp_suite']]
    sp_ivs = ['TestComposerImageVsCanvasPoint.' + t for t in st['sp_vs_suite']]
    sp_s = ['TestComposerSvgPoint.' + t for t in st['sp_suite']]
    sp_svs = ['TestComposerSvgVsComposerPoint.' + t for t in st['sp_vs_suite']]
    sp_p = ['TestComposerPdfPoint.' + t for t in st['sp_suite']]
    sp_pvs = ['TestComposerPdfVsComposerPoint.' + t for t in st['sp_vs_suite']]
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
