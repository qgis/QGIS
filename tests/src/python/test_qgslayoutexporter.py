# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsLayoutExporter

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '11/12/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'

import qgis  # NOQA
from qgis.PyQt import sip
import tempfile
import shutil
import os
import subprocess
from xml.dom import minidom
from osgeo import gdal

from qgis.core import (QgsMultiRenderChecker,
                       QgsLayoutExporter,
                       QgsLayout,
                       QgsProject,
                       QgsMargins,
                       QgsLayoutItemShape,
                       QgsLayoutItemLabel,
                       QgsLayoutGuide,
                       QgsRectangle,
                       QgsLayoutItemPage,
                       QgsLayoutItemMap,
                       QgsLayoutItemScaleBar,
                       QgsLayoutPoint,
                       QgsLayoutMeasurement,
                       QgsUnitTypes,
                       QgsSimpleFillSymbolLayer,
                       QgsFillSymbol,
                       QgsVectorLayer,
                       QgsCoordinateReferenceSystem,
                       QgsPrintLayout,
                       QgsSingleSymbolRenderer,
                       QgsRenderContext,
                       QgsReport,
                       QgsPalLayerSettings,
                       QgsFeature,
                       QgsGeometry,
                       QgsPointXY,
                       QgsVectorLayerSimpleLabeling)
from qgis.PyQt.QtCore import QSize, QSizeF, QDir, QRectF, Qt, QDateTime, QDate, QTime, QTimeZone
from qgis.PyQt.QtGui import QImage, QPainter
from qgis.PyQt.QtPrintSupport import QPrinter
from qgis.PyQt.QtSvg import QSvgRenderer, QSvgGenerator

from qgis.testing import start_app, unittest

from utilities import getExecutablePath, unitTestDataPath

TEST_DATA_DIR = unitTestDataPath()

# PDF-to-image utility
# look for Poppler w/ Cairo, then muPDF
# * Poppler w/ Cairo renders correctly
# * Poppler w/o Cairo does not always correctly render vectors in PDF to image
# * muPDF renders correctly, but slightly shifts colors
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


def pdfToPng(pdf_file_path, rendered_file_path, page, dpi=96):
    if PDFUTIL.strip().endswith('pdftocairo'):
        filebase = os.path.join(
            os.path.dirname(rendered_file_path),
            os.path.splitext(os.path.basename(rendered_file_path))[0]
        )
        call = [
            PDFUTIL, '-png', '-singlefile', '-r', str(dpi),
            '-x', '0', '-y', '0', '-f', str(page), '-l', str(page),
            pdf_file_path, filebase
        ]
    elif PDFUTIL.strip().endswith('mudraw'):
        call = [
            PDFUTIL, '-c', 'rgba',
            '-r', str(dpi), '-f', str(page), '-l', str(page),
            # '-b', '8',
            '-o', rendered_file_path, pdf_file_path
        ]
    else:
        return False, ''

    print("exportToPdf call: {0}".format(' '.join(call)))
    try:
        subprocess.check_call(call)
    except subprocess.CalledProcessError as e:
        assert False, ("exportToPdf failed!\n"
                       "cmd: {0}\n"
                       "returncode: {1}\n"
                       "message: {2}".format(e.cmd, e.returncode, e.message))


def svgToPng(svg_file_path, rendered_file_path, width):
    svgr = QSvgRenderer(svg_file_path)

    height = int(width / svgr.viewBoxF().width() * svgr.viewBoxF().height())

    image = QImage(width, height, QImage.Format_ARGB32)
    image.fill(Qt.transparent)

    p = QPainter(image)
    p.setRenderHint(QPainter.Antialiasing, False)
    svgr.render(p)
    p.end()

    res = image.save(rendered_file_path, 'png')
    if not res:
        os.unlink(rendered_file_path)


start_app()


class TestQgsLayoutExporter(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        cls.basetestpath = tempfile.mkdtemp()
        cls.dots_per_meter = int(96 / 25.4 * 1000)

    def setUp(self):
        self.report = "<h1>Python QgsLayoutExporter Tests</h1>\n"

    def tearDown(self):
        report_file_path = "%s/qgistest.html" % QDir.tempPath()
        with open(report_file_path, 'a') as report_file:
            report_file.write(self.report)

    def checkImage(self, name, reference_image, rendered_image, size_tolerance=0):
        checker = QgsMultiRenderChecker()
        checker.setControlPathPrefix("layout_exporter")
        checker.setControlName("expected_layoutexporter_" + reference_image)
        checker.setRenderedImage(rendered_image)
        checker.setColorTolerance(2)
        checker.setSizeTolerance(size_tolerance, size_tolerance)
        result = checker.runTest(name, 20)
        self.report += checker.report()
        print((self.report))
        return result

    def testRenderPage(self):
        l = QgsLayout(QgsProject.instance())
        l.initializeDefaults()

        # add some items
        item1 = QgsLayoutItemShape(l)
        item1.attemptSetSceneRect(QRectF(10, 20, 100, 150))
        fill = QgsSimpleFillSymbolLayer()
        fill_symbol = QgsFillSymbol()
        fill_symbol.changeSymbolLayer(0, fill)
        fill.setColor(Qt.green)
        fill.setStrokeStyle(Qt.NoPen)
        item1.setSymbol(fill_symbol)
        l.addItem(item1)

        # get width/height, create image and render the composition to it
        size = QSize(1122, 794)
        output_image = QImage(size, QImage.Format_RGB32)

        output_image.setDotsPerMeterX(self.dots_per_meter)
        output_image.setDotsPerMeterY(self.dots_per_meter)
        QgsMultiRenderChecker.drawBackground(output_image)
        painter = QPainter(output_image)
        exporter = QgsLayoutExporter(l)

        # valid page
        exporter.renderPage(painter, 0)
        painter.end()

        rendered_file_path = os.path.join(self.basetestpath, 'test_renderpage.png')
        output_image.save(rendered_file_path, "PNG")
        self.assertTrue(self.checkImage('renderpage', 'renderpage', rendered_file_path))

    def testRenderPageToImage(self):
        l = QgsLayout(QgsProject.instance())
        l.initializeDefaults()

        # add some items
        item1 = QgsLayoutItemShape(l)
        item1.attemptSetSceneRect(QRectF(10, 20, 100, 150))
        fill = QgsSimpleFillSymbolLayer()
        fill_symbol = QgsFillSymbol()
        fill_symbol.changeSymbolLayer(0, fill)
        fill.setColor(Qt.green)
        fill.setStrokeStyle(Qt.NoPen)
        item1.setSymbol(fill_symbol)
        l.addItem(item1)

        exporter = QgsLayoutExporter(l)
        size = QSize(1122, 794)

        # bad page numbers
        image = exporter.renderPageToImage(-1, size)
        self.assertTrue(image.isNull())
        image = exporter.renderPageToImage(1, size)
        self.assertTrue(image.isNull())

        # good page
        image = exporter.renderPageToImage(0, size)
        self.assertFalse(image.isNull())

        rendered_file_path = os.path.join(self.basetestpath, 'test_rendertoimagepage.png')
        image.save(rendered_file_path, "PNG")
        self.assertTrue(self.checkImage('rendertoimagepage', 'rendertoimagepage', rendered_file_path))

    def testRenderRegion(self):
        l = QgsLayout(QgsProject.instance())
        l.initializeDefaults()

        # add a guide, to ensure it is not included in export
        g1 = QgsLayoutGuide(Qt.Horizontal, QgsLayoutMeasurement(15, QgsUnitTypes.LayoutMillimeters), l.pageCollection().page(0))
        l.guides().addGuide(g1)

        # add some items
        item1 = QgsLayoutItemShape(l)
        item1.attemptSetSceneRect(QRectF(10, 20, 100, 150))
        fill = QgsSimpleFillSymbolLayer()
        fill_symbol = QgsFillSymbol()
        fill_symbol.changeSymbolLayer(0, fill)
        fill.setColor(Qt.green)
        fill.setStrokeStyle(Qt.NoPen)
        item1.setSymbol(fill_symbol)
        l.addItem(item1)

        # get width/height, create image and render the composition to it
        size = QSize(560, 509)
        output_image = QImage(size, QImage.Format_RGB32)

        output_image.setDotsPerMeterX(self.dots_per_meter)
        output_image.setDotsPerMeterY(self.dots_per_meter)
        QgsMultiRenderChecker.drawBackground(output_image)
        painter = QPainter(output_image)
        exporter = QgsLayoutExporter(l)

        exporter.renderRegion(painter, QRectF(5, 10, 110, 100))
        painter.end()

        rendered_file_path = os.path.join(self.basetestpath, 'test_renderregion.png')
        output_image.save(rendered_file_path, "PNG")
        self.assertTrue(self.checkImage('renderregion', 'renderregion', rendered_file_path))

    def testRenderRegionToImage(self):
        l = QgsLayout(QgsProject.instance())
        l.initializeDefaults()

        # add some items
        item1 = QgsLayoutItemShape(l)
        item1.attemptSetSceneRect(QRectF(10, 20, 100, 150))
        fill = QgsSimpleFillSymbolLayer()
        fill_symbol = QgsFillSymbol()
        fill_symbol.changeSymbolLayer(0, fill)
        fill.setColor(Qt.green)
        fill.setStrokeStyle(Qt.NoPen)
        item1.setSymbol(fill_symbol)
        l.addItem(item1)

        exporter = QgsLayoutExporter(l)
        size = QSize(560, 509)

        image = exporter.renderRegionToImage(QRectF(5, 10, 110, 100), size)
        self.assertFalse(image.isNull())

        rendered_file_path = os.path.join(self.basetestpath, 'test_rendertoimageregionsize.png')
        image.save(rendered_file_path, "PNG")
        self.assertTrue(self.checkImage('rendertoimageregionsize', 'rendertoimageregionsize', rendered_file_path))

        # using layout dpi
        l.renderContext().setDpi(40)
        image = exporter.renderRegionToImage(QRectF(5, 10, 110, 100))
        self.assertFalse(image.isNull())

        rendered_file_path = os.path.join(self.basetestpath, 'test_rendertoimageregiondpi.png')
        image.save(rendered_file_path, "PNG")
        self.assertTrue(self.checkImage('rendertoimageregiondpi', 'rendertoimageregiondpi', rendered_file_path))

        # overriding dpi
        image = exporter.renderRegionToImage(QRectF(5, 10, 110, 100), QSize(), 80)
        self.assertFalse(image.isNull())

        rendered_file_path = os.path.join(self.basetestpath, 'test_rendertoimageregionoverridedpi.png')
        image.save(rendered_file_path, "PNG")
        self.assertTrue(self.checkImage('rendertoimageregionoverridedpi', 'rendertoimageregionoverridedpi', rendered_file_path))

    def testExportToImage(self):
        md = QgsProject.instance().metadata()
        md.setTitle('proj title')
        md.setAuthor('proj author')
        md.setCreationDateTime(QDateTime(QDate(2011, 5, 3), QTime(9, 4, 5), QTimeZone(36000)))
        md.setIdentifier('proj identifier')
        md.setAbstract('proj abstract')
        md.setKeywords({'kw': ['kw1', 'kw2'], 'KWx': ['kw3', 'kw4']})
        QgsProject.instance().setMetadata(md)
        l = QgsLayout(QgsProject.instance())
        l.initializeDefaults()

        # add a second page
        page2 = QgsLayoutItemPage(l)
        page2.setPageSize('A5')
        l.pageCollection().addPage(page2)

        # add some items
        item1 = QgsLayoutItemShape(l)
        item1.attemptSetSceneRect(QRectF(10, 20, 100, 150))
        fill = QgsSimpleFillSymbolLayer()
        fill_symbol = QgsFillSymbol()
        fill_symbol.changeSymbolLayer(0, fill)
        fill.setColor(Qt.green)
        fill.setStrokeStyle(Qt.NoPen)
        item1.setSymbol(fill_symbol)
        l.addItem(item1)

        item2 = QgsLayoutItemShape(l)
        item2.attemptSetSceneRect(QRectF(10, 20, 100, 150))
        item2.attemptMove(QgsLayoutPoint(10, 20), page=1)
        fill = QgsSimpleFillSymbolLayer()
        fill_symbol = QgsFillSymbol()
        fill_symbol.changeSymbolLayer(0, fill)
        fill.setColor(Qt.cyan)
        fill.setStrokeStyle(Qt.NoPen)
        item2.setSymbol(fill_symbol)
        l.addItem(item2)

        exporter = QgsLayoutExporter(l)
        # setup settings
        settings = QgsLayoutExporter.ImageExportSettings()
        settings.dpi = 80

        rendered_file_path = os.path.join(self.basetestpath, 'test_exporttoimagedpi.png')
        self.assertEqual(exporter.exportToImage(rendered_file_path, settings), QgsLayoutExporter.Success)

        self.assertTrue(self.checkImage('exporttoimagedpi_page1', 'exporttoimagedpi_page1', rendered_file_path))
        page2_path = os.path.join(self.basetestpath, 'test_exporttoimagedpi_2.png')
        self.assertTrue(self.checkImage('exporttoimagedpi_page2', 'exporttoimagedpi_page2', page2_path))

        for f in (rendered_file_path, page2_path):
            d = gdal.Open(f)
            metadata = d.GetMetadata()
            self.assertEqual(metadata['Author'], 'proj author')
            self.assertEqual(metadata['Created'], '2011-05-03T09:04:05+10:00')
            self.assertEqual(metadata['Keywords'], 'KWx: kw3,kw4;kw: kw1,kw2')
            self.assertEqual(metadata['Subject'], 'proj abstract')
            self.assertEqual(metadata['Title'], 'proj title')

        # crop to contents
        settings.cropToContents = True
        settings.cropMargins = QgsMargins(10, 20, 30, 40)

        rendered_file_path = os.path.join(self.basetestpath, 'test_exporttoimagecropped.png')
        self.assertEqual(exporter.exportToImage(rendered_file_path, settings), QgsLayoutExporter.Success)

        self.assertTrue(self.checkImage('exporttoimagecropped_page1', 'exporttoimagecropped_page1', rendered_file_path))
        page2_path = os.path.join(self.basetestpath, 'test_exporttoimagecropped_2.png')
        self.assertTrue(self.checkImage('exporttoimagecropped_page2', 'exporttoimagecropped_page2', page2_path))

        # specific pages
        settings.cropToContents = False
        settings.pages = [1]

        rendered_file_path = os.path.join(self.basetestpath, 'test_exporttoimagepages.png')
        self.assertEqual(exporter.exportToImage(rendered_file_path, settings), QgsLayoutExporter.Success)

        self.assertFalse(os.path.exists(rendered_file_path))
        page2_path = os.path.join(self.basetestpath, 'test_exporttoimagepages_2.png')
        self.assertTrue(self.checkImage('exporttoimagedpi_page2', 'exporttoimagedpi_page2', page2_path))

        # image size
        settings.imageSize = QSize(600, 851)
        rendered_file_path = os.path.join(self.basetestpath, 'test_exporttoimagesize.png')
        self.assertEqual(exporter.exportToImage(rendered_file_path, settings), QgsLayoutExporter.Success)
        self.assertFalse(os.path.exists(rendered_file_path))
        page2_path = os.path.join(self.basetestpath, 'test_exporttoimagesize_2.png')
        self.assertTrue(self.checkImage('exporttoimagesize_page2', 'exporttoimagesize_page2', page2_path))

        # image size with incorrect aspect ratio
        # this can happen as a result of data defined page sizes
        settings.imageSize = QSize(851, 600)
        rendered_file_path = os.path.join(self.basetestpath, 'test_exporttoimagesizebadaspect.png')
        self.assertEqual(exporter.exportToImage(rendered_file_path, settings), QgsLayoutExporter.Success)

        page2_path = os.path.join(self.basetestpath, 'test_exporttoimagesizebadaspect_2.png')
        im = QImage(page2_path)
        self.assertTrue(self.checkImage('exporttoimagesize_badaspect', 'exporttoimagedpi_page2', page2_path), '{}x{}'.format(im.width(), im.height()))

    def testExportToPdf(self):
        md = QgsProject.instance().metadata()
        md.setTitle('proj title')
        md.setAuthor('proj author')
        md.setCreationDateTime(QDateTime(QDate(2011, 5, 3), QTime(9, 4, 5), QTimeZone(36000)))
        md.setIdentifier('proj identifier')
        md.setAbstract('proj abstract')
        md.setKeywords({'kw': ['kw1', 'kw2'], 'KWx': ['kw3', 'kw4']})
        QgsProject.instance().setMetadata(md)

        l = QgsLayout(QgsProject.instance())
        l.initializeDefaults()

        # add a second page
        page2 = QgsLayoutItemPage(l)
        page2.setPageSize('A5')
        l.pageCollection().addPage(page2)

        # add some items
        item1 = QgsLayoutItemShape(l)
        item1.attemptSetSceneRect(QRectF(10, 20, 100, 150))
        fill = QgsSimpleFillSymbolLayer()
        fill_symbol = QgsFillSymbol()
        fill_symbol.changeSymbolLayer(0, fill)
        fill.setColor(Qt.green)
        fill.setStrokeStyle(Qt.NoPen)
        item1.setSymbol(fill_symbol)
        l.addItem(item1)

        item2 = QgsLayoutItemShape(l)
        item2.attemptSetSceneRect(QRectF(10, 20, 100, 150))
        item2.attemptMove(QgsLayoutPoint(10, 20), page=1)
        fill = QgsSimpleFillSymbolLayer()
        fill_symbol = QgsFillSymbol()
        fill_symbol.changeSymbolLayer(0, fill)
        fill.setColor(Qt.cyan)
        fill.setStrokeStyle(Qt.NoPen)
        item2.setSymbol(fill_symbol)
        l.addItem(item2)

        exporter = QgsLayoutExporter(l)
        # setup settings
        settings = QgsLayoutExporter.PdfExportSettings()
        settings.dpi = 80
        settings.rasterizeWholeImage = False
        settings.forceVectorOutput = False
        settings.exportMetadata = True

        pdf_file_path = os.path.join(self.basetestpath, 'test_exporttopdfdpi.pdf')
        self.assertEqual(exporter.exportToPdf(pdf_file_path, settings), QgsLayoutExporter.Success)
        self.assertTrue(os.path.exists(pdf_file_path))

        rendered_page_1 = os.path.join(self.basetestpath, 'test_exporttopdfdpi.png')
        dpi = 80
        pdfToPng(pdf_file_path, rendered_page_1, dpi=dpi, page=1)
        rendered_page_2 = os.path.join(self.basetestpath, 'test_exporttopdfdpi2.png')
        pdfToPng(pdf_file_path, rendered_page_2, dpi=dpi, page=2)

        self.assertTrue(self.checkImage('exporttopdfdpi_page1', 'exporttopdfdpi_page1', rendered_page_1, size_tolerance=1))
        self.assertTrue(self.checkImage('exporttopdfdpi_page2', 'exporttopdfdpi_page2', rendered_page_2, size_tolerance=1))

        d = gdal.Open(pdf_file_path)
        metadata = d.GetMetadata()
        self.assertEqual(metadata['AUTHOR'], 'proj author')
        self.assertEqual(metadata['CREATION_DATE'], "D:20110503090405+10'0'")
        self.assertEqual(metadata['KEYWORDS'], 'KWx: kw3,kw4;kw: kw1,kw2')
        self.assertEqual(metadata['SUBJECT'], 'proj abstract')
        self.assertEqual(metadata['TITLE'], 'proj title')

    def testExportToPdfGeoreference(self):
        md = QgsProject.instance().metadata()
        md.setTitle('proj title')
        md.setAuthor('proj author')
        md.setCreationDateTime(QDateTime(QDate(2011, 5, 3), QTime(9, 4, 5), QTimeZone(36000)))
        md.setIdentifier('proj identifier')
        md.setAbstract('proj abstract')
        md.setKeywords({'kw': ['kw1', 'kw2'], 'KWx': ['kw3', 'kw4']})
        QgsProject.instance().setMetadata(md)

        l = QgsLayout(QgsProject.instance())
        l.initializeDefaults()

        # add some items
        map = QgsLayoutItemMap(l)
        map.attemptSetSceneRect(QRectF(30, 60, 200, 100))
        extent = QgsRectangle(333218, 1167809, 348781, 1180875)
        map.setCrs(QgsCoordinateReferenceSystem('EPSG:3148'))
        map.setExtent(extent)
        l.addLayoutItem(map)

        exporter = QgsLayoutExporter(l)
        # setup settings
        settings = QgsLayoutExporter.PdfExportSettings()
        settings.dpi = 96
        settings.rasterizeWholeImage = False
        settings.forceVectorOutput = False
        settings.appendGeoreference = True
        settings.exportMetadata = False

        pdf_file_path = os.path.join(self.basetestpath, 'test_exporttopdf_georeference.pdf')
        self.assertEqual(exporter.exportToPdf(pdf_file_path, settings), QgsLayoutExporter.Success)
        self.assertTrue(os.path.exists(pdf_file_path))

        d = gdal.Open(pdf_file_path)

        # check if georeferencing was successful
        geoTransform = d.GetGeoTransform()
        self.assertAlmostEqual(geoTransform[0], 330883.5499999996, 4)
        self.assertAlmostEqual(geoTransform[1], 13.184029109934016, 4)
        self.assertAlmostEqual(geoTransform[2], 0.0, 4)
        self.assertAlmostEqual(geoTransform[3], 1185550.768915511, 4)
        self.assertAlmostEqual(geoTransform[4], 0.0, 4)
        self.assertAlmostEqual(geoTransform[5], -13.183886222186642, 4)

        # check that the metadata has _not_ been added to the exported PDF
        metadata = d.GetMetadata()
        self.assertFalse('AUTHOR' in metadata)

        exporter = QgsLayoutExporter(l)
        # setup settings
        settings = QgsLayoutExporter.PdfExportSettings()
        settings.dpi = 96
        settings.rasterizeWholeImage = False
        settings.forceVectorOutput = False
        settings.appendGeoreference = False
        settings.exportMetadata = False

        pdf_file_path = os.path.join(self.basetestpath, 'test_exporttopdf_nogeoreference.pdf')
        self.assertEqual(exporter.exportToPdf(pdf_file_path, settings), QgsLayoutExporter.Success)
        self.assertTrue(os.path.exists(pdf_file_path))

        d = gdal.Open(pdf_file_path)
        # check that georeference information has _not_ been added to the exported PDF
        self.assertEqual(d.GetGeoTransform(), (0.0, 1.0, 0.0, 0.0, 0.0, 1.0))

    def testExportToPdfSkipFirstPage(self):
        l = QgsLayout(QgsProject.instance())
        l.initializeDefaults()

        # page 1 is excluded from export
        page1 = l.pageCollection().page(0)
        page1.setExcludeFromExports(True)

        # add a second page
        page2 = QgsLayoutItemPage(l)
        page2.setPageSize('A5')
        l.pageCollection().addPage(page2)

        item2 = QgsLayoutItemShape(l)
        item2.attemptSetSceneRect(QRectF(10, 20, 100, 150))
        item2.attemptMove(QgsLayoutPoint(10, 20), page=1)
        fill = QgsSimpleFillSymbolLayer()
        fill_symbol = QgsFillSymbol()
        fill_symbol.changeSymbolLayer(0, fill)
        fill.setColor(Qt.cyan)
        fill.setStrokeStyle(Qt.NoPen)
        item2.setSymbol(fill_symbol)
        l.addItem(item2)

        exporter = QgsLayoutExporter(l)
        # setup settings
        settings = QgsLayoutExporter.PdfExportSettings()
        settings.dpi = 80
        settings.rasterizeWholeImage = False
        settings.forceVectorOutput = False
        settings.exportMetadata = True

        pdf_file_path = os.path.join(self.basetestpath, 'test_exporttopdfdpi_skip_first.pdf')
        self.assertEqual(exporter.exportToPdf(pdf_file_path, settings), QgsLayoutExporter.Success)
        self.assertTrue(os.path.exists(pdf_file_path))

        rendered_page_1 = os.path.join(self.basetestpath, 'test_exporttopdfdpi_skip_first.png')
        dpi = 80
        pdfToPng(pdf_file_path, rendered_page_1, dpi=dpi, page=1)

        self.assertTrue(self.checkImage('test_exporttopdfdpi_skip_first', 'exporttopdfdpi_page2', rendered_page_1, size_tolerance=1))

    def testExportToSvg(self):
        md = QgsProject.instance().metadata()
        md.setTitle('proj title')
        md.setAuthor('proj author')
        md.setCreationDateTime(QDateTime(QDate(2011, 5, 3), QTime(9, 4, 5), QTimeZone(36000)))
        md.setIdentifier('proj identifier')
        md.setAbstract('proj abstract')
        md.setKeywords({'kw': ['kw1', 'kw2']})
        QgsProject.instance().setMetadata(md)
        l = QgsLayout(QgsProject.instance())
        l.initializeDefaults()

        # add a second page
        page2 = QgsLayoutItemPage(l)
        page2.setPageSize('A5')
        l.pageCollection().addPage(page2)

        # add some items
        item1 = QgsLayoutItemShape(l)
        item1.attemptSetSceneRect(QRectF(10, 20, 100, 150))
        fill = QgsSimpleFillSymbolLayer()
        fill_symbol = QgsFillSymbol()
        fill_symbol.changeSymbolLayer(0, fill)
        fill.setColor(Qt.green)
        fill.setStrokeStyle(Qt.NoPen)
        item1.setSymbol(fill_symbol)
        l.addItem(item1)

        item2 = QgsLayoutItemShape(l)
        item2.attemptSetSceneRect(QRectF(10, 20, 100, 150))
        item2.attemptMove(QgsLayoutPoint(10, 20), page=1)
        fill = QgsSimpleFillSymbolLayer()
        fill_symbol = QgsFillSymbol()
        fill_symbol.changeSymbolLayer(0, fill)
        fill.setColor(Qt.cyan)
        fill.setStrokeStyle(Qt.NoPen)
        item2.setSymbol(fill_symbol)
        l.addItem(item2)

        exporter = QgsLayoutExporter(l)
        # setup settings
        settings = QgsLayoutExporter.SvgExportSettings()
        settings.dpi = 80
        settings.forceVectorOutput = False
        settings.exportMetadata = True

        svg_file_path = os.path.join(self.basetestpath, 'test_exporttosvgdpi.svg')
        svg_file_path_2 = os.path.join(self.basetestpath, 'test_exporttosvgdpi_2.svg')
        self.assertEqual(exporter.exportToSvg(svg_file_path, settings), QgsLayoutExporter.Success)
        self.assertTrue(os.path.exists(svg_file_path))
        self.assertTrue(os.path.exists(svg_file_path_2))

        # metadata
        def checkMetadata(f, expected):
            # ideally we'd check the path too - but that's very complex given that
            # the output from Qt svg generator isn't valid XML, and no Python standard library
            # xml parser handles invalid xml...
            self.assertEqual('proj title' in open(f).read(), expected)
            self.assertEqual('proj author' in open(f).read(), expected)
            self.assertEqual('proj identifier' in open(f).read(), expected)
            self.assertEqual('2011-05-03' in open(f).read(), expected)
            self.assertEqual('proj abstract' in open(f).read(), expected)
            self.assertEqual('kw1' in open(f).read(), expected)
            self.assertEqual('kw2' in open(f).read(), expected)
            self.assertEqual('xmlns:cc="http://creativecommons.org/ns#"' in open(f).read(), expected)

        for f in [svg_file_path, svg_file_path_2]:
            checkMetadata(f, True)

        rendered_page_1 = os.path.join(self.basetestpath, 'test_exporttosvgdpi.png')
        svgToPng(svg_file_path, rendered_page_1, width=936)
        rendered_page_2 = os.path.join(self.basetestpath, 'test_exporttosvgdpi2.png')
        svgToPng(svg_file_path_2, rendered_page_2, width=467)

        self.assertTrue(self.checkImage('exporttosvgdpi_page1', 'exporttopdfdpi_page1', rendered_page_1, size_tolerance=1))
        self.assertTrue(self.checkImage('exporttosvgdpi_page2', 'exporttopdfdpi_page2', rendered_page_2, size_tolerance=1))

        # no metadata
        settings.exportMetadata = False
        self.assertEqual(exporter.exportToSvg(svg_file_path, settings), QgsLayoutExporter.Success)
        for f in [svg_file_path, svg_file_path_2]:
            checkMetadata(f, False)

        # layered
        settings.exportAsLayers = True
        settings.exportMetadata = True

        svg_file_path = os.path.join(self.basetestpath, 'test_exporttosvglayered.svg')
        svg_file_path_2 = os.path.join(self.basetestpath, 'test_exporttosvglayered_2.svg')
        self.assertEqual(exporter.exportToSvg(svg_file_path, settings), QgsLayoutExporter.Success)
        self.assertTrue(os.path.exists(svg_file_path))
        self.assertTrue(os.path.exists(svg_file_path_2))

        rendered_page_1 = os.path.join(self.basetestpath, 'test_exporttosvglayered.png')
        svgToPng(svg_file_path, rendered_page_1, width=936)
        rendered_page_2 = os.path.join(self.basetestpath, 'test_exporttosvglayered2.png')
        svgToPng(svg_file_path_2, rendered_page_2, width=467)

        self.assertTrue(self.checkImage('exporttosvglayered_page1', 'exporttopdfdpi_page1', rendered_page_1, size_tolerance=1))
        self.assertTrue(self.checkImage('exporttosvglayered_page2', 'exporttopdfdpi_page2', rendered_page_2, size_tolerance=1))

        for f in [svg_file_path, svg_file_path_2]:
            checkMetadata(f, True)

        # layered no metadata
        settings.exportAsLayers = True
        settings.exportMetadata = False
        self.assertEqual(exporter.exportToSvg(svg_file_path, settings), QgsLayoutExporter.Success)
        for f in [svg_file_path, svg_file_path_2]:
            checkMetadata(f, False)

    def testExportToSvgTextRenderFormat(self):
        l = QgsLayout(QgsProject.instance())
        l.initializeDefaults()

        # add a map and scalebar
        mapitem = QgsLayoutItemMap(l)
        mapitem.attemptSetSceneRect(QRectF(110, 120, 200, 250))
        mapitem.zoomToExtent(QgsRectangle(1, 1, 10, 10))
        mapitem.setScale(666)  # unlikely to appear in the SVG by accident... unless... oh no! RUN!
        l.addItem(mapitem)

        item1 = QgsLayoutItemScaleBar(l)
        item1.attemptSetSceneRect(QRectF(10, 20, 100, 150))
        item1.setLinkedMap(mapitem)
        item1.setStyle('Numeric')
        l.addItem(item1)

        exporter = QgsLayoutExporter(l)
        # setup settings
        settings = QgsLayoutExporter.SvgExportSettings()
        settings.dpi = 80
        settings.forceVectorOutput = False
        settings.exportMetadata = True
        settings.textRenderFormat = QgsRenderContext.TextFormatAlwaysText

        svg_file_path = os.path.join(self.basetestpath, 'test_exporttosvgtextformattext.svg')
        self.assertEqual(exporter.exportToSvg(svg_file_path, settings), QgsLayoutExporter.Success)
        self.assertTrue(os.path.exists(svg_file_path))

        # expect svg to contain a text object with the scale
        with open(svg_file_path, 'r') as f:
            lines = ''.join(f.readlines())
        self.assertIn('<text', lines)
        self.assertIn('>1:666<', lines)

        # force use of outlines
        os.unlink(svg_file_path)
        settings.textRenderFormat = QgsRenderContext.TextFormatAlwaysOutlines
        self.assertEqual(exporter.exportToSvg(svg_file_path, settings), QgsLayoutExporter.Success)
        self.assertTrue(os.path.exists(svg_file_path))

        # expect svg NOT to contain a text object with the scale
        with open(svg_file_path, 'r') as f:
            lines = ''.join(f.readlines())
        self.assertNotIn('<text', lines)
        self.assertNotIn('>1:666<', lines)

    def testPrint(self):
        l = QgsLayout(QgsProject.instance())
        l.initializeDefaults()

        # add a second page
        page2 = QgsLayoutItemPage(l)
        page2.setPageSize('A5')
        l.pageCollection().addPage(page2)

        # add some items
        item1 = QgsLayoutItemShape(l)
        item1.attemptSetSceneRect(QRectF(10, 20, 100, 150))
        fill = QgsSimpleFillSymbolLayer()
        fill_symbol = QgsFillSymbol()
        fill_symbol.changeSymbolLayer(0, fill)
        fill.setColor(Qt.green)
        fill.setStrokeStyle(Qt.NoPen)
        item1.setSymbol(fill_symbol)
        l.addItem(item1)

        item2 = QgsLayoutItemShape(l)
        item2.attemptSetSceneRect(QRectF(10, 20, 100, 150))
        item2.attemptMove(QgsLayoutPoint(10, 20), page=1)
        fill = QgsSimpleFillSymbolLayer()
        fill_symbol = QgsFillSymbol()
        fill_symbol.changeSymbolLayer(0, fill)
        fill.setColor(Qt.cyan)
        fill.setStrokeStyle(Qt.NoPen)
        item2.setSymbol(fill_symbol)
        l.addItem(item2)

        exporter = QgsLayoutExporter(l)
        # setup settings
        settings = QgsLayoutExporter.PrintExportSettings()
        settings.dpi = 80
        settings.rasterizeWholeImage = False

        pdf_file_path = os.path.join(self.basetestpath, 'test_printdpi.pdf')
        # make a qprinter directed to pdf
        printer = QPrinter()
        printer.setOutputFileName(pdf_file_path)
        printer.setOutputFormat(QPrinter.PdfFormat)

        self.assertEqual(exporter.print(printer, settings), QgsLayoutExporter.Success)
        self.assertTrue(os.path.exists(pdf_file_path))

        rendered_page_1 = os.path.join(self.basetestpath, 'test_exporttopdfdpi.png')
        dpi = 80
        pdfToPng(pdf_file_path, rendered_page_1, dpi=dpi, page=1)
        rendered_page_2 = os.path.join(self.basetestpath, 'test_exporttopdfdpi2.png')
        pdfToPng(pdf_file_path, rendered_page_2, dpi=dpi, page=2)

        self.assertTrue(self.checkImage('printdpi_page1', 'exporttopdfdpi_page1', rendered_page_1, size_tolerance=1))
        self.assertTrue(self.checkImage('printdpi_page2', 'exporttopdfdpi_page2', rendered_page_2, size_tolerance=1))

    def testExportWorldFile(self):
        l = QgsLayout(QgsProject.instance())
        l.initializeDefaults()

        # add some items
        map = QgsLayoutItemMap(l)
        map.attemptSetSceneRect(QRectF(30, 60, 200, 100))
        extent = QgsRectangle(2000, 2800, 2500, 2900)
        map.setExtent(extent)
        l.addLayoutItem(map)

        exporter = QgsLayoutExporter(l)
        # setup settings
        settings = QgsLayoutExporter.ImageExportSettings()
        settings.dpi = 80
        settings.generateWorldFile = False

        rendered_file_path = os.path.join(self.basetestpath, 'test_exportwithworldfile.png')
        world_file_path = os.path.join(self.basetestpath, 'test_exportwithworldfile.pgw')
        self.assertEqual(exporter.exportToImage(rendered_file_path, settings), QgsLayoutExporter.Success)
        self.assertTrue(os.path.exists(rendered_file_path))
        self.assertFalse(os.path.exists(world_file_path))

        # with world file
        settings.generateWorldFile = True
        rendered_file_path = os.path.join(self.basetestpath, 'test_exportwithworldfile.png')
        self.assertEqual(exporter.exportToImage(rendered_file_path, settings), QgsLayoutExporter.Success)
        self.assertTrue(os.path.exists(rendered_file_path))
        self.assertTrue(os.path.exists(world_file_path))

        lines = tuple(open(world_file_path, 'r'))
        values = [float(f) for f in lines]
        self.assertAlmostEqual(values[0], 0.794117647059, 2)
        self.assertAlmostEqual(values[1], 0.0, 2)
        self.assertAlmostEqual(values[2], 0.0, 2)
        self.assertAlmostEqual(values[3], -0.794251134644, 2)
        self.assertAlmostEqual(values[4], 1925.000000000000, 2)
        self.assertAlmostEqual(values[5], 3050.000000000000, 2)

    def testExcludePagesImage(self):
        l = QgsLayout(QgsProject.instance())
        l.initializeDefaults()
        # add a second page
        page2 = QgsLayoutItemPage(l)
        page2.setPageSize('A5')
        l.pageCollection().addPage(page2)

        exporter = QgsLayoutExporter(l)
        # setup settings
        settings = QgsLayoutExporter.ImageExportSettings()
        settings.dpi = 80
        settings.generateWorldFile = False

        rendered_file_path = os.path.join(self.basetestpath, 'test_exclude_export.png')
        details = QgsLayoutExporter.PageExportDetails()
        details.directory = self.basetestpath
        details.baseName = 'test_exclude_export'
        details.extension = 'png'
        details.page = 0

        self.assertEqual(exporter.exportToImage(rendered_file_path, settings), QgsLayoutExporter.Success)
        self.assertTrue(os.path.exists(exporter.generateFileName(details)))
        details.page = 1
        self.assertTrue(os.path.exists(exporter.generateFileName(details)))

        # exclude a page
        l.pageCollection().page(0).setExcludeFromExports(True)
        rendered_file_path = os.path.join(self.basetestpath, 'test_exclude_export_excluded.png')
        details.baseName = 'test_exclude_export_excluded'
        details.page = 0
        self.assertEqual(exporter.exportToImage(rendered_file_path, settings), QgsLayoutExporter.Success)
        self.assertFalse(os.path.exists(exporter.generateFileName(details)))
        details.page = 1
        self.assertTrue(os.path.exists(exporter.generateFileName(details)))

        # exclude second page
        l.pageCollection().page(1).setExcludeFromExports(True)
        rendered_file_path = os.path.join(self.basetestpath, 'test_exclude_export_excluded_all.png')
        details.baseName = 'test_exclude_export_excluded_all'
        details.page = 0
        self.assertEqual(exporter.exportToImage(rendered_file_path, settings), QgsLayoutExporter.Success)
        self.assertFalse(os.path.exists(exporter.generateFileName(details)))
        details.page = 1
        self.assertFalse(os.path.exists(exporter.generateFileName(details)))

    def testPageFileName(self):
        l = QgsLayout(QgsProject.instance())
        exporter = QgsLayoutExporter(l)
        details = QgsLayoutExporter.PageExportDetails()
        details.directory = '/tmp/output'
        details.baseName = 'my_maps'
        details.extension = 'png'
        details.page = 0
        self.assertEqual(exporter.generateFileName(details), '/tmp/output/my_maps.png')
        details.page = 1
        self.assertEqual(exporter.generateFileName(details), '/tmp/output/my_maps_2.png')
        details.page = 2
        self.assertEqual(exporter.generateFileName(details), '/tmp/output/my_maps_3.png')

    def prepareIteratorLayout(self):
        layer_path = os.path.join(TEST_DATA_DIR, 'france_parts.shp')
        layer = QgsVectorLayer(layer_path, 'test', "ogr")

        project = QgsProject()
        project.addMapLayers([layer])
        # select epsg:2154
        crs = QgsCoordinateReferenceSystem('epsg:2154')
        project.setCrs(crs)

        layout = QgsPrintLayout(project)
        layout.initializeDefaults()

        # fix the renderer, fill with green
        props = {"color": "0,127,0", "outline_width": "4", "outline_color": '255,255,255'}
        fillSymbol = QgsFillSymbol.createSimple(props)
        renderer = QgsSingleSymbolRenderer(fillSymbol)
        layer.setRenderer(renderer)

        # the atlas map
        atlas_map = QgsLayoutItemMap(layout)
        atlas_map.attemptSetSceneRect(QRectF(20, 20, 130, 130))
        atlas_map.setFrameEnabled(True)
        atlas_map.setLayers([layer])
        layout.addLayoutItem(atlas_map)

        # the atlas
        atlas = layout.atlas()
        atlas.setCoverageLayer(layer)
        atlas.setEnabled(True)

        atlas_map.setExtent(
            QgsRectangle(332719.06221504929, 6765214.5887386119, 560957.85090677091, 6993453.3774303338))

        atlas_map.setAtlasDriven(True)
        atlas_map.setAtlasScalingMode(QgsLayoutItemMap.Auto)
        atlas_map.setAtlasMargin(0.10)

        return project, layout

    def testIteratorToImages(self):
        project, layout = self.prepareIteratorLayout()
        atlas = layout.atlas()
        atlas.setFilenameExpression("'test_exportiteratortoimage_' || \"NAME_1\"")

        # setup settings
        settings = QgsLayoutExporter.ImageExportSettings()
        settings.dpi = 80

        result, error = QgsLayoutExporter.exportToImage(atlas, self.basetestpath + '/', 'png', settings)
        self.assertEqual(result, QgsLayoutExporter.Success, error)

        page1_path = os.path.join(self.basetestpath, 'test_exportiteratortoimage_Basse-Normandie.png')
        self.assertTrue(self.checkImage('iteratortoimage1', 'iteratortoimage1', page1_path))
        page2_path = os.path.join(self.basetestpath, 'test_exportiteratortoimage_Bretagne.png')
        self.assertTrue(self.checkImage('iteratortoimage2', 'iteratortoimage2', page2_path))
        page3_path = os.path.join(self.basetestpath, 'test_exportiteratortoimage_Centre.png')
        self.assertTrue(os.path.exists(page3_path))
        page4_path = os.path.join(self.basetestpath, 'test_exportiteratortoimage_Pays de la Loire.png')
        self.assertTrue(os.path.exists(page4_path))

    def testIteratorToSvgs(self):
        project, layout = self.prepareIteratorLayout()
        atlas = layout.atlas()
        atlas.setFilenameExpression("'test_exportiteratortosvg_' || \"NAME_1\"")

        # setup settings
        settings = QgsLayoutExporter.SvgExportSettings()
        settings.dpi = 80
        settings.forceVectorOutput = False

        result, error = QgsLayoutExporter.exportToSvg(atlas, self.basetestpath + '/', settings)
        self.assertEqual(result, QgsLayoutExporter.Success, error)

        page1_path = os.path.join(self.basetestpath, 'test_exportiteratortosvg_Basse-Normandie.svg')
        rendered_page_1 = os.path.join(self.basetestpath, 'test_exportiteratortosvg_Basse-Normandie.png')
        svgToPng(page1_path, rendered_page_1, width=935)
        self.assertTrue(self.checkImage('iteratortosvg1', 'iteratortoimage1', rendered_page_1, size_tolerance=2))
        page2_path = os.path.join(self.basetestpath, 'test_exportiteratortosvg_Bretagne.svg')
        rendered_page_2 = os.path.join(self.basetestpath, 'test_exportiteratortosvg_Bretagne.png')
        svgToPng(page2_path, rendered_page_2, width=935)
        self.assertTrue(self.checkImage('iteratortosvg2', 'iteratortoimage2', rendered_page_2, size_tolerance=2))
        page3_path = os.path.join(self.basetestpath, 'test_exportiteratortosvg_Centre.svg')
        self.assertTrue(os.path.exists(page3_path))
        page4_path = os.path.join(self.basetestpath, 'test_exportiteratortosvg_Pays de la Loire.svg')
        self.assertTrue(os.path.exists(page4_path))

    def testIteratorToPdfs(self):
        project, layout = self.prepareIteratorLayout()
        atlas = layout.atlas()
        atlas.setFilenameExpression("'test_exportiteratortopdf_' || \"NAME_1\"")

        # setup settings
        settings = QgsLayoutExporter.PdfExportSettings()
        settings.dpi = 80
        settings.rasterizeWholeImage = False
        settings.forceVectorOutput = False

        result, error = QgsLayoutExporter.exportToPdfs(atlas, self.basetestpath + '/', settings)
        self.assertEqual(result, QgsLayoutExporter.Success, error)

        page1_path = os.path.join(self.basetestpath, 'test_exportiteratortopdf_Basse-Normandie.pdf')
        rendered_page_1 = os.path.join(self.basetestpath, 'test_exportiteratortopdf_Basse-Normandie.png')
        pdfToPng(page1_path, rendered_page_1, dpi=80, page=1)
        self.assertTrue(self.checkImage('iteratortopdf1', 'iteratortoimage1', rendered_page_1, size_tolerance=2))
        page2_path = os.path.join(self.basetestpath, 'test_exportiteratortopdf_Bretagne.pdf')
        rendered_page_2 = os.path.join(self.basetestpath, 'test_exportiteratortopdf_Bretagne.png')
        pdfToPng(page2_path, rendered_page_2, dpi=80, page=1)
        self.assertTrue(self.checkImage('iteratortopdf2', 'iteratortoimage2', rendered_page_2, size_tolerance=2))
        page3_path = os.path.join(self.basetestpath, 'test_exportiteratortopdf_Centre.pdf')
        self.assertTrue(os.path.exists(page3_path))
        page4_path = os.path.join(self.basetestpath, 'test_exportiteratortopdf_Pays de la Loire.pdf')
        self.assertTrue(os.path.exists(page4_path))

    def testIteratorToPdf(self):
        project, layout = self.prepareIteratorLayout()
        atlas = layout.atlas()

        # setup settings
        settings = QgsLayoutExporter.PdfExportSettings()
        settings.dpi = 80
        settings.rasterizeWholeImage = False
        settings.forceVectorOutput = False

        pdf_path = os.path.join(self.basetestpath, 'test_exportiteratortopdf_single.pdf')
        result, error = QgsLayoutExporter.exportToPdf(atlas, pdf_path, settings)
        self.assertEqual(result, QgsLayoutExporter.Success, error)

        rendered_page_1 = os.path.join(self.basetestpath, 'test_exportiteratortopdf_single1.png')
        pdfToPng(pdf_path, rendered_page_1, dpi=80, page=1)
        self.assertTrue(self.checkImage('iteratortopdfsingle1', 'iteratortoimage1', rendered_page_1, size_tolerance=2))

        rendered_page_2 = os.path.join(self.basetestpath, 'test_exportiteratortopdf_single2.png')
        pdfToPng(pdf_path, rendered_page_2, dpi=80, page=2)
        self.assertTrue(self.checkImage('iteratortopdfsingle2', 'iteratortoimage2', rendered_page_2, size_tolerance=2))

        rendered_page_3 = os.path.join(self.basetestpath, 'test_exportiteratortopdf_single3.png')
        pdfToPng(pdf_path, rendered_page_3, dpi=80, page=3)
        self.assertTrue(os.path.exists(rendered_page_3))
        rendered_page_4 = os.path.join(self.basetestpath, 'test_exportiteratortopdf_single4.png')
        pdfToPng(pdf_path, rendered_page_4, dpi=80, page=4)
        self.assertTrue(os.path.exists(rendered_page_4))

    def testPrintIterator(self):
        project, layout = self.prepareIteratorLayout()
        atlas = layout.atlas()

        # setup settings
        settings = QgsLayoutExporter.PrintExportSettings()
        settings.dpi = 80
        settings.rasterizeWholeImage = False

        pdf_path = os.path.join(self.basetestpath, 'test_printiterator.pdf')
        # make a qprinter directed to pdf
        printer = QPrinter()
        printer.setOutputFileName(pdf_path)
        printer.setOutputFormat(QPrinter.PdfFormat)

        result, error = QgsLayoutExporter.print(atlas, printer, settings)
        self.assertEqual(result, QgsLayoutExporter.Success, error)

        rendered_page_1 = os.path.join(self.basetestpath, 'test_printiterator1.png')
        pdfToPng(pdf_path, rendered_page_1, dpi=80, page=1)
        self.assertTrue(self.checkImage('printeriterator1', 'iteratortoimage1', rendered_page_1, size_tolerance=2))

        rendered_page_2 = os.path.join(self.basetestpath, 'test_printiterator2.png')
        pdfToPng(pdf_path, rendered_page_2, dpi=80, page=2)
        self.assertTrue(self.checkImage('printiterator2', 'iteratortoimage2', rendered_page_2, size_tolerance=2))

        rendered_page_3 = os.path.join(self.basetestpath, 'test_printiterator3.png')
        pdfToPng(pdf_path, rendered_page_3, dpi=80, page=3)
        self.assertTrue(os.path.exists(rendered_page_3))
        rendered_page_4 = os.path.join(self.basetestpath, 'test_printiterator4.png')
        pdfToPng(pdf_path, rendered_page_4, dpi=80, page=4)
        self.assertTrue(os.path.exists(rendered_page_4))

    def testExportReport(self):
        p = QgsProject()
        r = QgsReport(p)

        # add a header
        r.setHeaderEnabled(True)
        report_header = QgsLayout(p)
        report_header.initializeDefaults()
        item1 = QgsLayoutItemShape(report_header)
        item1.attemptSetSceneRect(QRectF(10, 20, 100, 150))
        fill = QgsSimpleFillSymbolLayer()
        fill_symbol = QgsFillSymbol()
        fill_symbol.changeSymbolLayer(0, fill)
        fill.setColor(Qt.green)
        fill.setStrokeStyle(Qt.NoPen)
        item1.setSymbol(fill_symbol)
        report_header.addItem(item1)

        r.setHeader(report_header)

        # add a footer
        r.setFooterEnabled(True)
        report_footer = QgsLayout(p)
        report_footer.initializeDefaults()
        item2 = QgsLayoutItemShape(report_footer)
        item2.attemptSetSceneRect(QRectF(10, 20, 100, 150))
        item2.attemptMove(QgsLayoutPoint(10, 20))
        fill = QgsSimpleFillSymbolLayer()
        fill_symbol = QgsFillSymbol()
        fill_symbol.changeSymbolLayer(0, fill)
        fill.setColor(Qt.cyan)
        fill.setStrokeStyle(Qt.NoPen)
        item2.setSymbol(fill_symbol)
        report_footer.addItem(item2)

        r.setFooter(report_footer)

        # setup settings
        settings = QgsLayoutExporter.ImageExportSettings()
        settings.dpi = 80

        report_path = os.path.join(self.basetestpath, 'test_report')
        result, error = QgsLayoutExporter.exportToImage(r, report_path, 'png', settings)
        self.assertEqual(result, QgsLayoutExporter.Success, error)

        page1_path = os.path.join(self.basetestpath, 'test_report_0001.png')
        self.assertTrue(self.checkImage('report_page1', 'report_page1', page1_path))
        page2_path = os.path.join(self.basetestpath, 'test_report_0002.png')
        self.assertTrue(self.checkImage('report_page2', 'report_page2', page2_path))

    def testRequiresRasterization(self):
        """
        Test QgsLayoutExporter.requiresRasterization
        """
        l = QgsLayout(QgsProject.instance())
        l.initializeDefaults()

        # add an item
        label = QgsLayoutItemLabel(l)
        label.attemptSetSceneRect(QRectF(30, 60, 200, 100))
        l.addLayoutItem(label)

        self.assertFalse(QgsLayoutExporter.requiresRasterization(l))

        # an item with a blend mode will force the whole layout to be rasterized
        label.setBlendMode(QPainter.CompositionMode_Overlay)
        self.assertTrue(QgsLayoutExporter.requiresRasterization(l))

        # but if the item is NOT visible, it won't affect the output in any way..
        label.setVisibility(False)
        self.assertFalse(QgsLayoutExporter.requiresRasterization(l))

    def testContainsAdvancedEffects(self):
        """
        Test QgsLayoutExporter.containsAdvancedEffects
        """
        l = QgsLayout(QgsProject.instance())
        l.initializeDefaults()

        # add an item
        label = QgsLayoutItemLabel(l)
        label.attemptSetSceneRect(QRectF(30, 60, 200, 100))
        l.addLayoutItem(label)

        self.assertFalse(QgsLayoutExporter.containsAdvancedEffects(l))

        # an item with transparency will force it to be individually rasterized
        label.setItemOpacity(0.5)
        self.assertTrue(QgsLayoutExporter.containsAdvancedEffects(l))

        # but if the item is NOT visible, it won't affect the output in any way..
        label.setVisibility(False)
        self.assertFalse(QgsLayoutExporter.containsAdvancedEffects(l))

    def testLabelingResults(self):
        """
        Test QgsLayoutExporter.labelingResults()
        """
        settings = QgsPalLayerSettings()
        settings.fieldName = "\"id\""
        settings.isExpression = True
        settings.placement = QgsPalLayerSettings.OverPoint
        settings.priority = 10
        settings.displayAll = True

        vl = QgsVectorLayer("Point?crs=epsg:4326&field=id:integer", "vl", "memory")
        f = QgsFeature()
        f.setAttributes([1])
        f.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(-6.250851540391068, 53.335006994584944)))
        self.assertTrue(vl.dataProvider().addFeature(f))
        f.setAttributes([8888])
        f.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(-21.950014487179544, 64.150023619739216)))
        self.assertTrue(vl.dataProvider().addFeature(f))
        f.setAttributes([33333])
        f.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(-0.118667702475932, 51.5019405883275)))
        self.assertTrue(vl.dataProvider().addFeature(f))
        vl.updateExtents()

        p = QgsProject()
        p.addMapLayer(vl)

        vl.setLabeling(QgsVectorLayerSimpleLabeling(settings))
        vl.setLabelsEnabled(True)

        l = QgsLayout(p)
        l.initializeDefaults()

        map = QgsLayoutItemMap(l)
        map.attemptSetSceneRect(QRectF(20, 20, 200, 100))
        map.setFrameEnabled(False)
        map.setBackgroundEnabled(False)
        map.setCrs(vl.crs())
        map.zoomToExtent(vl.extent())
        map.setLayers([vl])
        l.addLayoutItem(map)

        exporter = QgsLayoutExporter(l)
        self.assertEqual(exporter.labelingResults(), {})
        # setup settings
        settings = QgsLayoutExporter.ImageExportSettings()
        settings.dpi = 80

        rendered_file_path = os.path.join(self.basetestpath, 'test_exportlabelresults.png')
        self.assertEqual(exporter.exportToImage(rendered_file_path, settings), QgsLayoutExporter.Success)

        results = exporter.labelingResults()
        self.assertEqual(len(results), 1)

        labels = results[map.uuid()].allLabels()
        self.assertEqual(len(labels), 3)
        self.assertCountEqual([l.labelText for l in labels], ['1', '33333', '8888'])


if __name__ == '__main__':
    unittest.main()
