# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsVirtualLayerDefinition

From build dir, run: ctest -R PyQgsSelectiveMasking -V

QGIS_PREFIX_PATH=/home/hme/src/QGIS/build_ninja/output PYTHONPATH=/home/hme/src/QGIS/build_ninja/output/python/:/home/hme/src/QGIS/build_ninja/output/python/plugins:/home/hme/src/QGIS/tests/src/python python3 ~/src/QGIS/tests/src/python/test_selective_masking.py

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Hugo Mercier / Oslandia'
__date__ = '28/06/2019'

import qgis  # NOQA
import os
import subprocess
import difflib

from qgis.PyQt.QtCore import (
    Qt,
    QSize,
    QRectF,
    QDir
)

from qgis.PyQt.QtGui import (
    QColor,
    QImage,
    QPainter
)

from qgis.testing import unittest, start_app

from utilities import (
    unitTestDataPath,
    getTempfilePath,
    renderMapToImage,
    loadTestFonts,
    getTestFont,
    openInBrowserTab
)

from qgis.core import (
    Qgis,
    QgsMapSettings,
    QgsCoordinateReferenceSystem,
    QgsRectangle,
    QgsProject,
    QgsSymbolLayerReference,
    QgsMapRendererParallelJob,
    QgsMapRendererSequentialJob,
    QgsMapRendererCustomPainterJob,
    QgsRenderChecker,
    QgsSimpleMarkerSymbolLayer,
    QgsSimpleMarkerSymbolLayerBase,
    QgsMarkerSymbol,
    QgsMaskMarkerSymbolLayer,
    QgsSingleSymbolRenderer,
    QgsSymbolLayerId,
    QgsSymbolLayerUtils,
    QgsMapRendererCache,
    QgsUnitTypes,
    QgsOuterGlowEffect,
    QgsPalLayerSettings,
    QgsRuleBasedLabeling,
    QgsPalLayerSettings,
    QgsProperty,
    QgsRenderContext,
    QgsVectorLayerSimpleLabeling,
    QgsLayout,
    QgsLayoutItemPage,
    QgsLayoutSize,
    QgsLayoutItemMap,
    QgsLayoutExporter,
    QgsWkbTypes,
)

TEST_DATA_DIR = unitTestDataPath()


def renderMapToImageWithTime(mapsettings, parallel=False, cache=None):
    """
    Render current map to an image, via multi-threaded renderer
    :param QgsMapSettings mapsettings:
    :param bool parallel: Do parallel or sequential render job
    :rtype: QImage
    """
    if parallel:
        job = QgsMapRendererParallelJob(mapsettings)
    else:
        job = QgsMapRendererSequentialJob(mapsettings)
    if cache:
        job.setCache(cache)
    job.start()
    job.waitForFinished()

    return (job.renderedImage(), job.renderingTime())


class TestSelectiveMasking(unittest.TestCase):

    def setUp(self):
        self.checker = QgsRenderChecker()
        self.checker.setControlPathPrefix("selective_masking")

        self.report = "<h1>Python Selective Masking Tests</h1>\n"

        self.map_settings = QgsMapSettings()
        crs = QgsCoordinateReferenceSystem('epsg:4326')
        extent = QgsRectangle(-123.0, 22.7, -76.4, 46.9)
        self.map_settings.setBackgroundColor(QColor(152, 219, 249))
        self.map_settings.setOutputSize(QSize(420, 280))
        self.map_settings.setOutputDpi(72)
        self.map_settings.setFlag(QgsMapSettings.Antialiasing, True)
        self.map_settings.setFlag(QgsMapSettings.UseAdvancedEffects, False)
        self.map_settings.setDestinationCrs(crs)
        self.map_settings.setExtent(extent)

        # load a predefined QGIS project
        self.assertTrue(QgsProject.instance().read(os.path.join(unitTestDataPath(), "selective_masking.qgs")))

        self.points_layer = QgsProject.instance().mapLayersByName('points')[0]
        self.lines_layer = QgsProject.instance().mapLayersByName('lines')[0]
        # line layer with subsymbols
        self.lines_layer2 = QgsProject.instance().mapLayersByName('lines2')[0]
        # line layer with labels
        self.lines_with_labels = QgsProject.instance().mapLayersByName('lines_with_labels')[0]

        self.polys_layer = QgsProject.instance().mapLayersByName('polys')[0]
        # polygon layer with a rule based labeling
        self.polys_layer2 = QgsProject.instance().mapLayersByName('polys2')[0]

        self.raster_layer = QgsProject.instance().mapLayersByName('raster_layer')[0]

        # try to fix the font for where labels are defined
        # in order to have more stable image comparison tests
        for layer in [self.polys_layer, self.lines_with_labels, self.polys_layer2]:
            for provider in layer.labeling().subProviders():
                settings = layer.labeling().settings(provider)
                font = getTestFont()
                font.setPointSize(32)
                fmt = settings.format()
                fmt.setFont(font)
                fmt.setNamedStyle('Roman')
                fmt.setSize(32)
                fmt.setSizeUnit(QgsUnitTypes.RenderPoints)
                settings.setFormat(fmt)
                if (layer.geometryType == QgsWkbTypes.PolygonGeometry):
                    settings.placement = QgsPalLayerSettings.OverPoint
                layer.labeling().setSettings(settings, provider)

        # order layers for rendering
        self.map_settings.setLayers([self.points_layer, self.lines_layer, self.polys_layer])

    def tearDown(self):
        report_file_path = "%s/qgistest.html" % QDir.tempPath()
        with open(report_file_path, 'a') as report_file:
            report_file.write(self.report)

    def check_renderings(self, map_settings, control_name):
        """Test a rendering with different configurations:
        - parallel rendering, no cache
        - sequential rendering, no cache
        - parallel rendering, with cache (rendered two times)
        - sequential rendering, with cache (rendered two times)
        """

        for do_parallel in [False, True]:
            for use_cache in [False, True]:
                print("=== parallel", do_parallel, "cache", use_cache)
                tmp = getTempfilePath('png')
                cache = None
                if use_cache:
                    cache = QgsMapRendererCache()
                    # render a first time to fill the cache
                    renderMapToImageWithTime(self.map_settings, parallel=do_parallel, cache=cache)
                img, t = renderMapToImageWithTime(self.map_settings, parallel=do_parallel, cache=cache)
                img.save(tmp)
                print("Image rendered in {}".format(tmp))

                self.checker.setControlName(control_name)
                self.checker.setRenderedImage(tmp)
                suffix = ("_parallel" if do_parallel else "_sequential") + ("_cache" if use_cache else "_nocache")
                res = self.checker.compareImages(control_name + suffix)
                self.report += self.checker.report()
                self.assertTrue(res)

                print("=== Rendering took {}s".format(float(t) / 1000.0))

    def check_layout_export(self, control_name, expected_nb_raster, layers=None, dpiTarget=None):
        """
        Generate a PDF layout export and control the output matches expected_filename
        """

        # generate vector file
        layout = QgsLayout(QgsProject.instance())
        page = QgsLayoutItemPage(layout)
        page.setPageSize(QgsLayoutSize(50, 33))
        layout.pageCollection().addPage(page)

        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(1, 1, 48, 32))
        map.setFrameEnabled(True)
        layout.addLayoutItem(map)
        map.setExtent(self.lines_layer.extent())
        map.setLayers(layers if layers is not None else [self.points_layer, self.lines_layer, self.polys_layer])

        settings = QgsLayoutExporter.PdfExportSettings()

        if dpiTarget is not None:
            settings.dpi = dpiTarget

        exporter = QgsLayoutExporter(layout)
        result_filename = getTempfilePath('pdf')
        exporter.exportToPdf(result_filename, settings)
        self.assertTrue(os.path.exists(result_filename))

        # Generate a readable PDF file so we count raster in it
        result_txt = getTempfilePath("txt")
        subprocess.run(["qpdf", "--qdf", "--object-streams=disable", result_filename, result_txt])
        self.assertTrue(os.path.exists(result_txt))

        # expected_file = os.path.join(TEST_DATA_DIR, "control_images/selective_masking/pdf_exports/{}".format(expected_filename))
        # self.assertTrue(os.path.exists(expected_file))

        result = open(result_txt, 'rb')
        result_lines = [l.decode('iso-8859-1') for l in result.readlines()]
        result.close()
        nb_raster = len([l for l in result_lines if "/Subtype /Image" in l])
        self.assertEqual(nb_raster, expected_nb_raster)

        # Generate an image from pdf to compare with expected control image
        # keep PDF DPI resolution (300)
        image_result_filename = getTempfilePath("png")
        subprocess.run(["pdftoppm", result_filename,
                        os.path.splitext(image_result_filename)[0],
                        "-png", "-r", "300", "-singlefile"])

        self.checker.setControlName(control_name)
        self.checker.setRenderedImage(image_result_filename)
        res = self.checker.compareImages(control_name)
        self.report += self.checker.report()
        self.assertTrue(res)

    def test_save_restore_references(self):
        """
        Test saving and restoring symbol layer references
        """

        # simple ids
        mask_layer = QgsMaskMarkerSymbolLayer()
        mask_layer.setMasks([
            QgsSymbolLayerReference(self.lines_layer.id(), QgsSymbolLayerId("", 0)),
            QgsSymbolLayerReference(self.lines_layer2.id(), QgsSymbolLayerId("some_id", [1, 3, 5, 19])),
            QgsSymbolLayerReference(self.polys_layer.id(), QgsSymbolLayerId("some_other_id", [4, 5])),
        ])

        props = mask_layer.properties()

        mask_layer2 = QgsMaskMarkerSymbolLayer.create(props)
        self.assertEqual(mask_layer2.masks(), [
            QgsSymbolLayerReference(self.lines_layer.id(), QgsSymbolLayerId("", 0)),
            QgsSymbolLayerReference(self.lines_layer2.id(), QgsSymbolLayerId("some_id", [1, 3, 5, 19])),
            QgsSymbolLayerReference(self.polys_layer.id(), QgsSymbolLayerId("some_other_id", [4, 5])),
        ])

        # complex ids
        mask_layer = QgsMaskMarkerSymbolLayer()
        mask_layer.setMasks([
            QgsSymbolLayerReference(self.lines_layer.id(), QgsSymbolLayerId("", 0)),
            QgsSymbolLayerReference(self.lines_layer2.id(), QgsSymbolLayerId("some id, #1", [1, 3, 5, 19])),
            QgsSymbolLayerReference(self.polys_layer.id(), QgsSymbolLayerId("some other id, like, this", [4, 5])),
        ])

        props = mask_layer.properties()

        mask_layer2 = QgsMaskMarkerSymbolLayer.create(props)
        self.assertEqual(mask_layer2.masks(), [
            QgsSymbolLayerReference(self.lines_layer.id(), QgsSymbolLayerId("", 0)),
            QgsSymbolLayerReference(self.lines_layer2.id(), QgsSymbolLayerId("some id, #1", [1, 3, 5, 19])),
            QgsSymbolLayerReference(self.polys_layer.id(), QgsSymbolLayerId("some other id, like, this", [4, 5])),
        ])

        # complex ids, v2
        mask_layer = QgsMaskMarkerSymbolLayer()
        mask_layer.setMasks([
            QgsSymbolLayerReference(self.lines_layer.id(), QgsSymbolLayerId("a string; with bits", 0)),
            QgsSymbolLayerReference(self.lines_layer2.id(), QgsSymbolLayerId("some; id, #1", [1, 3, 5, 19])),
            QgsSymbolLayerReference(self.polys_layer.id(), QgsSymbolLayerId("some other; id, lik;e, this", [4, 5])),
        ])

        props = mask_layer.properties()

        mask_layer2 = QgsMaskMarkerSymbolLayer.create(props)
        self.assertEqual(mask_layer2.masks(), [
            QgsSymbolLayerReference(self.lines_layer.id(), QgsSymbolLayerId("a string; with bits", 0)),
            QgsSymbolLayerReference(self.lines_layer2.id(), QgsSymbolLayerId("some; id, #1", [1, 3, 5, 19])),
            QgsSymbolLayerReference(self.polys_layer.id(), QgsSymbolLayerId("some other; id, lik;e, this", [4, 5])),
        ])

    def test_label_mask(self):
        # modify labeling settings
        label_settings = self.polys_layer.labeling().settings()
        fmt = label_settings.format()
        # enable a mask
        fmt.mask().setEnabled(True)
        fmt.mask().setSize(4.0)
        # and mask other symbol layers underneath
        fmt.mask().setMaskedSymbolLayers([
            # the black part of roads
            QgsSymbolLayerReference(self.lines_layer.id(), QgsSymbolLayerId("", 0)),
            # the black jets
            QgsSymbolLayerReference(self.points_layer.id(), QgsSymbolLayerId("B52", 0)),
            QgsSymbolLayerReference(self.points_layer.id(), QgsSymbolLayerId("Jet", 0))])

        label_settings.setFormat(fmt)
        self.polys_layer.labeling().setSettings(label_settings)

        format = self.polys_layer.labeling().settings().format()
        self.assertTrue(format.mask().enabled())

        self.check_renderings(self.map_settings, "label_mask")

    def test_multiple_label_masks_different_sets(self):
        # modify labeling settings of the polys layer
        label_settings = self.polys_layer.labeling().settings()
        fmt = label_settings.format()
        # enable a mask
        fmt.mask().setEnabled(True)
        fmt.mask().setSize(4.0)
        # and mask other symbol layers underneath
        fmt.mask().setMaskedSymbolLayers([
            # the black part of roads
            QgsSymbolLayerReference(self.lines_with_labels.id(), QgsSymbolLayerId("", 0)),
            # the black jets
            QgsSymbolLayerReference(self.points_layer.id(), QgsSymbolLayerId("B52", 0)),
            QgsSymbolLayerReference(self.points_layer.id(), QgsSymbolLayerId("Jet", 0))])

        label_settings.setFormat(fmt)
        self.polys_layer.labeling().setSettings(label_settings)

        format = self.polys_layer.labeling().settings().format()
        self.assertTrue(format.mask().enabled())

        # modify labeling settings of the lines layer
        label_settings = self.lines_with_labels.labeling().settings()
        fmt = label_settings.format()
        # enable a mask
        fmt.mask().setEnabled(True)
        fmt.mask().setSize(4.0)
        # and mask other symbol layers underneath
        fmt.mask().setMaskedSymbolLayers([
            # polygons
            QgsSymbolLayerReference(self.polys_layer.id(), QgsSymbolLayerId("", 0)),
        ])
        label_settings.setFormat(fmt)
        self.lines_with_labels.labeling().setSettings(label_settings)

        # new map settings with a line symbology that has labels
        self.map_settings.setLayers([self.points_layer, self.lines_with_labels, self.polys_layer])
        self.check_renderings(self.map_settings, "multiple_label_masks_different_sets")
        # restore map settings
        self.map_settings.setLayers([self.points_layer, self.lines_layer, self.polys_layer])

    def test_multiple_label_masks_same_set(self):
        # modify labeling settings of the polys layer
        label_settings = self.polys_layer.labeling().settings()
        fmt = label_settings.format()
        # enable a mask
        fmt.mask().setEnabled(True)
        fmt.mask().setSize(4.0)
        # and mask other symbol layers underneath
        fmt.mask().setMaskedSymbolLayers([
            # the black part of roads
            QgsSymbolLayerReference(self.lines_with_labels.id(), QgsSymbolLayerId("", 0)),
        ])

        label_settings.setFormat(fmt)
        self.polys_layer.labeling().setSettings(label_settings)

        format = self.polys_layer.labeling().settings().format()
        self.assertTrue(format.mask().enabled())

        # modify labeling settings of the lines layer
        label_settings = self.lines_with_labels.labeling().settings()
        fmt = label_settings.format()
        # enable a mask
        fmt.mask().setEnabled(True)
        fmt.mask().setSize(4.0)
        # and mask other symbol layers underneath
        fmt.mask().setMaskedSymbolLayers([
            # the black part of roads
            QgsSymbolLayerReference(self.lines_with_labels.id(), QgsSymbolLayerId("", 0)),
        ])
        label_settings.setFormat(fmt)
        self.lines_with_labels.labeling().setSettings(label_settings)

        # new map settings with a line symbology that has labels
        self.map_settings.setLayers([self.points_layer, self.lines_with_labels, self.polys_layer])
        self.check_renderings(self.map_settings, "multiple_label_masks_same_set")
        # restore map settings
        self.map_settings.setLayers([self.points_layer, self.lines_layer, self.polys_layer])

    def test_label_mask_subsymbol(self):
        # new map settings with a line symbology that has sub symbols
        self.map_settings.setLayers([self.points_layer, self.lines_layer2, self.polys_layer])

        # modify labeling settings
        label_settings = self.polys_layer.labeling().settings()
        fmt = label_settings.format()
        # enable a mask
        fmt.mask().setEnabled(True)
        fmt.mask().setSize(4.0)
        # and mask other symbol layers underneath
        fmt.mask().setMaskedSymbolLayers([
            # mask only vertical segments of "roads"
            QgsSymbolLayerReference(self.lines_layer2.id(), QgsSymbolLayerId("", [1, 0])),
            # the black jets
            QgsSymbolLayerReference(self.points_layer.id(), QgsSymbolLayerId("B52", 0)),
            QgsSymbolLayerReference(self.points_layer.id(), QgsSymbolLayerId("Jet", 0))])

        label_settings.setFormat(fmt)
        self.polys_layer.labeling().setSettings(label_settings)

        format = self.polys_layer.labeling().settings().format()
        self.assertTrue(format.mask().enabled())

        self.check_renderings(self.map_settings, "label_mask_subsymbol")

        # restore original map settings
        self.map_settings.setLayers([self.points_layer, self.lines_layer, self.polys_layer])

    def test_label_mask_dd(self):
        """ test label mask with data defined properties """
        label_settings = self.polys_layer.labeling().settings()
        fmt = label_settings.format()
        fmt.mask().setEnabled(False)
        fmt.mask().setSize(1.0)
        fmt.mask().setOpacity(0.42)
        # mask other symbol layers underneath
        fmt.mask().setMaskedSymbolLayers([
            # the black part of roads
            QgsSymbolLayerReference(self.lines_layer.id(), QgsSymbolLayerId("", 0)),
            # the black jets
            QgsSymbolLayerReference(self.points_layer.id(), QgsSymbolLayerId("B52", 0)),
            QgsSymbolLayerReference(self.points_layer.id(), QgsSymbolLayerId("Jet", 0))])

        # overwrite with data-defined properties
        fmt.dataDefinedProperties().setProperty(QgsPalLayerSettings.MaskEnabled, QgsProperty.fromExpression('1'))
        fmt.dataDefinedProperties().setProperty(QgsPalLayerSettings.MaskBufferSize, QgsProperty.fromExpression('4.0'))
        fmt.dataDefinedProperties().setProperty(QgsPalLayerSettings.MaskOpacity, QgsProperty.fromExpression('100.0'))

        context = QgsRenderContext()
        fmt.updateDataDefinedProperties(context)

        self.assertEqual(fmt.mask().enabled(), True)
        self.assertEqual(fmt.mask().size(), 4.0)
        self.assertEqual(fmt.mask().opacity(), 1.0)

        label_settings.setFormat(fmt)
        self.polys_layer.labeling().setSettings(label_settings)

        self.check_renderings(self.map_settings, "label_mask")

    def test_label_mask_rule_labeling(self):
        # new map settings with a rule based labeling
        self.map_settings.setLayers([self.points_layer, self.lines_layer, self.polys_layer2])

        # modify labeling settings of one rule
        for child in self.polys_layer2.labeling().rootRule().children():
            if child.description() == 'Tadam':
                break
        label_settings = child.settings()
        label_settings.priority = 3
        fmt = label_settings.format()
        # enable a mask
        fmt.mask().setEnabled(True)
        fmt.mask().setSize(4.0)
        # and mask other symbol layers underneath
        fmt.mask().setMaskedSymbolLayers([
            # the black part of roads
            QgsSymbolLayerReference(self.lines_layer.id(), QgsSymbolLayerId("", 0)),
            # the black jets
            QgsSymbolLayerReference(self.points_layer.id(), QgsSymbolLayerId("B52", 0)),
            QgsSymbolLayerReference(self.points_layer.id(), QgsSymbolLayerId("Jet", 0))])

        label_settings.setFormat(fmt)
        child.setSettings(label_settings)

        # modify labeling settings of another rule
        for child in self.polys_layer2.labeling().rootRule().children():
            if child.description() != 'Tadam':
                break
        label_settings = child.settings()
        fmt = label_settings.format()
        # enable a mask
        fmt.mask().setEnabled(True)
        fmt.mask().setSize(4.0)
        # and mask other symbol layers underneath
        fmt.mask().setMaskedSymbolLayers([
            # the polygons
            QgsSymbolLayerReference(self.polys_layer2.id(), QgsSymbolLayerId("", 0)),
        ])
        label_settings.setFormat(fmt)
        child.setSettings(label_settings)

        self.check_renderings(self.map_settings, "rule_label_mask")

        # restore map settings
        self.map_settings.setLayers([self.points_layer, self.lines_layer, self.polys_layer])

    def test_label_mask_symbol_levels(self):
        # modify labeling settings
        label_settings = self.polys_layer.labeling().settings()
        fmt = label_settings.format()
        # enable a mask
        fmt.mask().setEnabled(True)
        fmt.mask().setSize(4.0)
        # and mask other symbol layers underneath
        fmt.mask().setMaskedSymbolLayers([
            # the black part of roads
            QgsSymbolLayerReference(self.lines_layer.id(), QgsSymbolLayerId("", 0)),
            # the black jets
            QgsSymbolLayerReference(self.points_layer.id(), QgsSymbolLayerId("B52", 0)),
            QgsSymbolLayerReference(self.points_layer.id(), QgsSymbolLayerId("Jet", 0))])

        label_settings.setFormat(fmt)
        self.polys_layer.labeling().setSettings(label_settings)

        format = self.polys_layer.labeling().settings().format()
        self.assertTrue(format.mask().enabled())

        # enable symbol levels
        self.lines_layer.renderer().setUsingSymbolLevels(True)

        self.check_renderings(self.map_settings, "label_mask_symbol_levels")

    def test_symbol_layer_mask(self):
        p = QgsMarkerSymbol.createSimple({'color': '#fdbf6f', 'size': "7"})
        self.points_layer.setRenderer(QgsSingleSymbolRenderer(p))

        circle_symbol = QgsMarkerSymbol.createSimple({'size': '10'})
        mask_layer = QgsMaskMarkerSymbolLayer()
        mask_layer.setSubSymbol(circle_symbol)
        mask_layer.setMasks([
            # the black part of roads
            QgsSymbolLayerReference(self.lines_layer.id(), QgsSymbolLayerId("", 0)),
        ])
        # add this mask layer to the point layer
        self.points_layer.renderer().symbol().appendSymbolLayer(mask_layer)

        self.check_renderings(self.map_settings, "sl_mask")

    def test_multiple_masks_same_symbol_layer(self):
        """Test multiple masks that occlude the same symbol layer"""
        #
        # 1. a symbol layer mask
        #
        p = QgsMarkerSymbol.createSimple({'color': '#fdbf6f', 'size': "7"})
        self.points_layer.setRenderer(QgsSingleSymbolRenderer(p))

        circle_symbol = QgsMarkerSymbol.createSimple({'size': '10'})
        mask_layer = QgsMaskMarkerSymbolLayer()
        mask_layer.setSubSymbol(circle_symbol)
        mask_layer.setMasks([
            # the black part of roads
            QgsSymbolLayerReference(self.lines_layer.id(), QgsSymbolLayerId("", 0)),
        ])
        # add this mask layer to the point layer
        self.points_layer.renderer().symbol().appendSymbolLayer(mask_layer)

        #
        # 2. a label mask
        #

        # modify labeling settings
        label_settings = self.polys_layer.labeling().settings()
        fmt = label_settings.format()
        # enable a mask
        fmt.mask().setEnabled(True)
        fmt.mask().setSize(4.0)
        # and mask other symbol layers underneath
        fmt.mask().setMaskedSymbolLayers([
            # the black part of roads
            QgsSymbolLayerReference(self.lines_layer.id(), QgsSymbolLayerId("", 0))
        ])
        label_settings.setFormat(fmt)
        self.polys_layer.labeling().setSettings(label_settings)

        self.check_renderings(self.map_settings, "multiple_masks_same_sl")

    def test_multiple_masks_different_symbol_layers_same_layer(self):
        """Test multiple masks that occlude different symbol layers of the same layer.
        The UI should disallow this settings. We test here that only one mask is retained"""
        #
        # 1. a symbol layer mask
        #
        p = QgsMarkerSymbol.createSimple({'color': '#fdbf6f', 'size': "7"})
        self.points_layer.setRenderer(QgsSingleSymbolRenderer(p))

        circle_symbol = QgsMarkerSymbol.createSimple({'size': '10'})
        mask_layer = QgsMaskMarkerSymbolLayer()
        mask_layer.setSubSymbol(circle_symbol)
        mask_layer.setMasks([
            # the yellow part of roads
            QgsSymbolLayerReference(self.lines_layer.id(), QgsSymbolLayerId("", 1)),
        ])
        # add this mask layer to the point layer
        self.points_layer.renderer().symbol().appendSymbolLayer(mask_layer)

        #
        # 2. a label mask
        #

        # modify labeling settings
        label_settings = self.polys_layer.labeling().settings()
        fmt = label_settings.format()
        # enable a mask
        fmt.mask().setEnabled(True)
        fmt.mask().setSize(4.0)
        # and mask other symbol layers underneath
        fmt.mask().setMaskedSymbolLayers([
            # the black part of roads
            QgsSymbolLayerReference(self.lines_layer.id(), QgsSymbolLayerId("", 0))
        ])
        label_settings.setFormat(fmt)
        self.polys_layer.labeling().setSettings(label_settings)

        self.check_renderings(self.map_settings, "multiple_masks_different_sl")

    def test_multiple_masks_different_symbol_layers_same_layer2(self):
        """Test multiple masks that occlude different symbol layers of the same layer - 2nd possible order
        The UI should disallow this settings. We test here that only one mask is retained"""
        #
        # 1. a symbol layer mask
        #
        p = QgsMarkerSymbol.createSimple({'color': '#fdbf6f', 'size': "7"})
        self.points_layer.setRenderer(QgsSingleSymbolRenderer(p))

        circle_symbol = QgsMarkerSymbol.createSimple({'size': '10'})
        mask_layer = QgsMaskMarkerSymbolLayer()
        mask_layer.setSubSymbol(circle_symbol)
        mask_layer.setMasks([
            # the black part of roads
            QgsSymbolLayerReference(self.lines_layer.id(), QgsSymbolLayerId("", 0)),
        ])
        # add this mask layer to the point layer
        self.points_layer.renderer().symbol().appendSymbolLayer(mask_layer)

        #
        # 2. a label mask
        #

        # modify labeling settings
        label_settings = self.polys_layer.labeling().settings()
        fmt = label_settings.format()
        # enable a mask
        fmt.mask().setEnabled(True)
        fmt.mask().setSize(4.0)
        # and mask other symbol layers underneath
        fmt.mask().setMaskedSymbolLayers([
            # the yellow part of roads
            QgsSymbolLayerReference(self.lines_layer.id(), QgsSymbolLayerId("", 1))
        ])
        label_settings.setFormat(fmt)
        self.polys_layer.labeling().setSettings(label_settings)

        self.check_renderings(self.map_settings, "multiple_masks_different_sl2")

    def test_mask_symbollayer_preview(self):
        #
        # Masks should be visible in previews
        #
        p = QgsMarkerSymbol.createSimple({'color': '#fdbf6f', 'size': "7"})

        circle_symbol = QgsMarkerSymbol.createSimple({'size': '10'})
        mask_layer = QgsMaskMarkerSymbolLayer()
        mask_layer.setSubSymbol(circle_symbol)
        p.insertSymbolLayer(0, mask_layer)

        for control_name, render_function in [
                ("as_image", lambda: p.asImage(QSize(64, 64)).save(tmp)),
                ("as_big_preview", lambda: p.bigSymbolPreviewImage().save(tmp)),
                ("sl_preview", lambda:
                 QgsSymbolLayerUtils.symbolLayerPreviewIcon(mask_layer,
                                                            QgsUnitTypes.RenderPixels,
                                                            QSize(64, 64)).pixmap(QSize(64, 64)).save(tmp))
        ]:
            tmp = getTempfilePath('png')
            render_function()
            self.checker.setControlName(control_name)
            self.checker.setRenderedImage(tmp)
            res = self.checker.compareImages(control_name, 90)
            self.report += self.checker.report()
            self.assertTrue(res)

    def test_mask_with_effect(self):
        p = QgsMarkerSymbol.createSimple({'color': '#fdbf6f', 'size': "7"})
        self.points_layer.setRenderer(QgsSingleSymbolRenderer(p))

        circle_symbol = QgsMarkerSymbol.createSimple({'size': '12'})
        mask_layer = QgsMaskMarkerSymbolLayer()
        mask_layer.setSubSymbol(circle_symbol)
        mask_layer.setMasks([
            # the yellow part of roads
            QgsSymbolLayerReference(self.lines_layer.id(), QgsSymbolLayerId("", 1)),
        ])
        # add an outer glow effect to the mask layer
        blur = QgsOuterGlowEffect.create({"enabled": "1",
                                          "blur_level": "6.445",
                                          "blur_unit": "MM",
                                          "opacity": "1",
                                          "spread": "0.6",
                                          "spread_unit": "MM",
                                          "color1": "0,0,255,255",
                                          "draw_mode": "2"
                                          })
        mask_layer.setPaintEffect(blur)
        # add this mask layer to the point layer
        self.points_layer.renderer().symbol().appendSymbolLayer(mask_layer)

        self.check_renderings(self.map_settings, "mask_with_effect")

    def test_label_mask_with_effect(self):
        # modify labeling settings
        label_settings = self.polys_layer.labeling().settings()
        fmt = label_settings.format()
        # enable a mask
        fmt.mask().setEnabled(True)
        fmt.mask().setSize(4.0)
        # and mask other symbol layers underneath
        fmt.mask().setMaskedSymbolLayers([
            # the black part of roads
            QgsSymbolLayerReference(self.lines_layer.id(), QgsSymbolLayerId("", 0)),
            # the black jets
            QgsSymbolLayerReference(self.points_layer.id(), QgsSymbolLayerId("B52", 0)),
            QgsSymbolLayerReference(self.points_layer.id(), QgsSymbolLayerId("Jet", 0))])

        # add an outer glow effect to the mask
        blur = QgsOuterGlowEffect.create({"enabled": "1",
                                          "blur_level": "6.445",
                                          "blur_unit": "MM",
                                          "opacity": "1",
                                          "spread": "0.6",
                                          "spread_unit": "MM",
                                          "color1": "0,0,255,255",
                                          "draw_mode": "2"
                                          })
        fmt.mask().setPaintEffect(blur)

        label_settings.setFormat(fmt)
        self.polys_layer.labeling().setSettings(label_settings)

        format = self.polys_layer.labeling().settings().format()
        self.assertTrue(format.mask().enabled())

        self.check_renderings(self.map_settings, "label_mask_with_effect")

        # test that force vector output has no impact on the result
        self.map_settings.setFlag(Qgis.MapSettingsFlag.ForceVectorOutput, True)
        self.check_renderings(self.map_settings, "label_mask_with_effect")

    def test_different_dpi_target(self):
        """Test with raster layer and a target dpi"""

        # modify labeling settings
        label_settings = self.polys_layer.labeling().settings()
        fmt = label_settings.format()
        # enable a mask
        fmt.mask().setEnabled(True)
        fmt.mask().setSize(4.0)
        # and mask other symbol layers underneath
        fmt.mask().setMaskedSymbolLayers([
            # the black part of roads
            QgsSymbolLayerReference(self.lines_layer.id(), QgsSymbolLayerId("", 0))])

        label_settings.setFormat(fmt)
        self.polys_layer.labeling().setSettings(label_settings)

        self.map_settings.setLayers([self.lines_layer, self.polys_layer, self.raster_layer])
        self.map_settings.setDpiTarget(300)
        self.check_renderings(self.map_settings, "different_dpi_target")

        # test with high dpi screen
        self.map_settings.setDevicePixelRatio(2)
        self.check_renderings(self.map_settings, "different_dpi_target_hdpi")

    def test_layout_export(self):
        """Test mask effects in a layout export at 300 dpi"""
        # modify labeling settings
        label_settings = self.polys_layer.labeling().settings()
        fmt = label_settings.format()
        # enable a mask

        fmt.font().setPointSize(4)

        fmt.mask().setEnabled(True)
        fmt.mask().setSize(1.0)
        # and mask other symbol layers underneath
        fmt.mask().setMaskedSymbolLayers([
            # the black part of roads
            QgsSymbolLayerReference(self.lines_layer.id(), QgsSymbolLayerId("", 0)),
            # the black jets
            QgsSymbolLayerReference(self.points_layer.id(), QgsSymbolLayerId("B52", 0)),
            QgsSymbolLayerReference(self.points_layer.id(), QgsSymbolLayerId("Jet", 0))])

        label_settings.setFormat(fmt)
        self.polys_layer.labeling().setSettings(label_settings)

        self.check_layout_export("layout_export", 0)

    def test_layout_export_w_effects(self):
        """Test mask effects in a layout export at 300 dpi"""
        # modify labeling settings
        label_settings = self.polys_layer.labeling().settings()
        fmt = label_settings.format()
        # enable a mask

        fmt.font().setPointSize(4)

        fmt.mask().setEnabled(True)
        fmt.mask().setSize(1.0)
        # and mask other symbol layers underneath
        fmt.mask().setMaskedSymbolLayers([
            # the black part of roads
            QgsSymbolLayerReference(self.lines_layer.id(), QgsSymbolLayerId("", 0)),
            # the black jets
            QgsSymbolLayerReference(self.points_layer.id(), QgsSymbolLayerId("B52", 0)),
            QgsSymbolLayerReference(self.points_layer.id(), QgsSymbolLayerId("Jet", 0))])

        # add an outer glow effect to the mask
        blur = QgsOuterGlowEffect.create({"enabled": "1",
                                          "blur_level": "3.445",
                                          "blur_unit": "MM",
                                          "opacity": "1",
                                          "spread": "0.06",
                                          "spread_unit": "MM",
                                          "color1": "0,0,255,255",
                                          "draw_mode": "2"
                                          })
        fmt.mask().setPaintEffect(blur)

        label_settings.setFormat(fmt)
        self.polys_layer.labeling().setSettings(label_settings)

        # 4 rasters : Image and its mask for masked point and lines layer
        self.check_layout_export("layout_export_w_effects", 4)

    def test_layout_export_marker_masking(self):
        """Test mask effects in a layout export with a marker symbol masking"""

        p = QgsMarkerSymbol.createSimple({'color': '#fdbf6f', 'size': "3"})
        self.points_layer.setRenderer(QgsSingleSymbolRenderer(p))

        circle_symbol = QgsMarkerSymbol.createSimple({'size': '6'})
        mask_layer = QgsMaskMarkerSymbolLayer()
        mask_layer.setSubSymbol(circle_symbol)
        mask_layer.setMasks([
            # the black part of roads
            QgsSymbolLayerReference(self.lines_layer.id(), QgsSymbolLayerId("", 0)),
        ])
        # add this mask layer to the point layer
        self.points_layer.renderer().symbol().appendSymbolLayer(mask_layer)

        self.check_layout_export("layout_export_marker_masking", 0)

    def test_layout_export_marker_masking_w_effects(self):
        """Test mask effects in a layout export with a marker symbol masking"""

        p = QgsMarkerSymbol.createSimple({'color': '#fdbf6f', 'size': "3"})
        self.points_layer.setRenderer(QgsSingleSymbolRenderer(p))

        circle_symbol = QgsMarkerSymbol.createSimple({'size': '6'})
        mask_layer = QgsMaskMarkerSymbolLayer()
        mask_layer.setSubSymbol(circle_symbol)
        mask_layer.setMasks([
            # the black part of roads
            QgsSymbolLayerReference(self.lines_layer.id(), QgsSymbolLayerId("", 0)),
        ])

        # add an outer glow effect to the mask
        blur = QgsOuterGlowEffect.create({"enabled": "1",
                                          "blur_level": "3.445",
                                          "blur_unit": "MM",
                                          "opacity": "1",
                                          "spread": "0.06",
                                          "spread_unit": "MM",
                                          "color1": "0,0,255,255",
                                          "draw_mode": "2"
                                          })

        # TODO try to set the mask effect on p the marker symbol -> result should be the same
        mask_layer.setPaintEffect(blur)

        # add this mask layer to the point layer
        self.points_layer.renderer().symbol().appendSymbolLayer(mask_layer)

        # 2 rasters : Image and its mask for masked lines layer
        self.check_layout_export("layout_export_marker_masking_w_effects", 2)

    def test_layout_export_w_raster(self):
        """Test layout export with raster beneath the masked area"""

        # just decrease the yellow line so we see the raster on masked area
        self.lines_layer.renderer().symbol().symbolLayers()[1].setWidth(0.5)

        # modify labeling settings
        label_settings = self.polys_layer.labeling().settings()
        fmt = label_settings.format()
        # enable a mask

        fmt.font().setPointSize(4)

        fmt.mask().setEnabled(True)
        fmt.mask().setSize(1.0)
        # and mask other symbol layers underneath
        fmt.mask().setMaskedSymbolLayers([
            # the black part of roads
            QgsSymbolLayerReference(self.lines_layer.id(), QgsSymbolLayerId("", 0)),
            # the black jets
            QgsSymbolLayerReference(self.points_layer.id(), QgsSymbolLayerId("B52", 0)),
            QgsSymbolLayerReference(self.points_layer.id(), QgsSymbolLayerId("Jet", 0))])

        label_settings.setFormat(fmt)
        self.polys_layer.labeling().setSettings(label_settings)

        # 1 raster : the raster layer
        self.check_layout_export("layout_export_w_raster", 1, [self.lines_layer, self.polys_layer, self.raster_layer])

    def test_layout_export_w_force_raster_render(self):
        """
        Test layout export with a marker symbol masking forced to be render as raster
        We expect the lines to be masked and the whole output needs to be vector except
        the marker layer forced as raster
        """

        p = QgsMarkerSymbol.createSimple({'color': '#fdbf6f', 'size': "3"})
        self.points_layer.setRenderer(QgsSingleSymbolRenderer(p))

        circle_symbol = QgsMarkerSymbol.createSimple({'size': '6'})
        mask_layer = QgsMaskMarkerSymbolLayer()
        mask_layer.setSubSymbol(circle_symbol)
        mask_layer.setMasks([
            # the black part of roads
            QgsSymbolLayerReference(self.lines_layer.id(), QgsSymbolLayerId("", 0)),
        ])
        # add this mask layer to the point layer
        self.points_layer.renderer().symbol().appendSymbolLayer(mask_layer)
        self.points_layer.renderer().setForceRasterRender(True)

        # 2 rasters : Image and its mask for the points layer
        self.check_layout_export("layout_export_force_raster_render", 2, [self.points_layer, self.lines_layer])

    def test_layout_export_marker_masking_w_transparency(self):
        """Test layout export with a marker symbol masking which has an opacity lower than 1"""

        p = QgsMarkerSymbol.createSimple({'color': '#fdbf6f', 'size': "3"})
        self.points_layer.setRenderer(QgsSingleSymbolRenderer(p))

        circle_symbol = QgsMarkerSymbol.createSimple({'size': '6'})
        circle_symbol.setOpacity(0.5)
        mask_layer = QgsMaskMarkerSymbolLayer()
        mask_layer.setSubSymbol(circle_symbol)
        mask_layer.setMasks([
            # the black part of roads
            QgsSymbolLayerReference(self.lines_layer.id(), QgsSymbolLayerId("", 0)),
        ])
        # add this mask layer to the point layer
        self.points_layer.renderer().symbol().appendSymbolLayer(mask_layer)

        # 2 rasters (mask + image) because opacity force rasterization of the masked line layers
        self.check_layout_export("layout_export_marker_masking_w_transparency", 2)

    def test_layout_export_text_masking_w_transparency(self):
        """Test mask effects in a layout export at 300 dpi"""
        # modify labeling settings
        label_settings = self.polys_layer.labeling().settings()
        fmt = label_settings.format()
        # enable a mask

        fmt.font().setPointSize(4)

        fmt.mask().setEnabled(True)
        fmt.mask().setSize(1.0)
        fmt.mask().setOpacity(0.5)

        # and mask other symbol layers underneath
        fmt.mask().setMaskedSymbolLayers([
            # the black part of roads
            QgsSymbolLayerReference(self.lines_layer.id(), QgsSymbolLayerId("", 0)),
            # the black jets
            QgsSymbolLayerReference(self.points_layer.id(), QgsSymbolLayerId("B52", 0)),
            QgsSymbolLayerReference(self.points_layer.id(), QgsSymbolLayerId("Jet", 0))])

        label_settings.setFormat(fmt)
        self.polys_layer.labeling().setSettings(label_settings)

        # 4 rasters (mask+image per masked layer) because opacity force rasterization
        # of the masked line and point layers
        self.check_layout_export("layout_export_text_masking_w_transparency", 4)

    def test_different_dpi_target_vector(self):
        """Test rendering a raster layer with vector output and a target dpi
        Used when layout previewing
        """

        # modify labeling settings
        label_settings = self.polys_layer.labeling().settings()
        fmt = label_settings.format()
        # enable a mask
        fmt.mask().setEnabled(True)
        fmt.mask().setSize(4.0)
        # and mask other symbol layers underneath
        fmt.mask().setMaskedSymbolLayers([
            # the black part of roads
            QgsSymbolLayerReference(self.lines_layer.id(), QgsSymbolLayerId("", 0))])

        label_settings.setFormat(fmt)
        self.polys_layer.labeling().setSettings(label_settings)

        self.map_settings.setLayers([self.lines_layer, self.polys_layer, self.raster_layer])
        self.map_settings.setOutputDpi(81)
        self.map_settings.setDpiTarget(300)
        self.map_settings.setFlag(Qgis.MapSettingsFlag.ForceVectorOutput, True)

        image = QImage(self.map_settings.deviceOutputSize(), self.map_settings.outputImageFormat())
        image.setDevicePixelRatio(self.map_settings.devicePixelRatio())
        image.setDotsPerMeterX(int(1000 * self.map_settings.outputDpi() / 25.4))
        image.setDotsPerMeterY(int(1000 * self.map_settings.outputDpi() / 25.4))
        image.fill(Qt.transparent)
        pImg = QPainter()
        pImg.begin(image)
        job = QgsMapRendererCustomPainterJob(self.map_settings, pImg)
        job.start()
        job.waitForFinished()
        pImg.end()
        tmp = getTempfilePath('png')
        image.save(tmp)

        control_name = "different_dpi_target_vector"
        self.checker.setControlName(control_name)
        self.checker.setRenderedImage(tmp)
        res = self.checker.compareImages(control_name)
        self.report += self.checker.report()
        self.assertTrue(res)

        # Same test with high dpi
        self.map_settings.setDevicePixelRatio(2)
        image = QImage(self.map_settings.deviceOutputSize(), self.map_settings.outputImageFormat())
        image.setDevicePixelRatio(self.map_settings.devicePixelRatio())
        image.setDotsPerMeterX(int(1000 * self.map_settings.outputDpi() / 25.4))
        image.setDotsPerMeterY(int(1000 * self.map_settings.outputDpi() / 25.4))
        image.fill(Qt.transparent)
        pImg = QPainter()
        pImg.begin(image)
        job = QgsMapRendererCustomPainterJob(self.map_settings, pImg)
        job.start()
        job.waitForFinished()
        pImg.end()
        tmp = getTempfilePath('png')
        image.save(tmp)

        control_name = "different_dpi_target_vector_hdpi"
        self.checker.setControlName(control_name)
        self.checker.setRenderedImage(tmp)
        res = self.checker.compareImages(control_name)
        self.report += self.checker.report()
        self.assertTrue(res)

    def test_layout_export_2_sources_masking(self):
        """Test masking with 2 different sources"""

        # mask with points layer circles...
        p = QgsMarkerSymbol.createSimple({'color': '#fdbf6f', 'size': "3"})
        self.points_layer.setRenderer(QgsSingleSymbolRenderer(p))

        circle_symbol = QgsMarkerSymbol.createSimple({'size': '6'})
        mask_layer = QgsMaskMarkerSymbolLayer()
        mask_layer.setSubSymbol(circle_symbol)
        mask_layer.setMasks([
            # the black part of roads
            QgsSymbolLayerReference(self.lines_layer.id(), QgsSymbolLayerId("", 0)),
        ])
        self.points_layer.renderer().symbol().appendSymbolLayer(mask_layer)

        # ...and with text
        label_settings = self.polys_layer.labeling().settings()
        fmt = label_settings.format()

        fmt.font().setPointSize(4)
        fmt.mask().setEnabled(True)
        fmt.mask().setSize(1.0)
        # and mask other symbol layers underneath
        fmt.mask().setMaskedSymbolLayers([
            # the black part of roads
            QgsSymbolLayerReference(self.lines_layer.id(), QgsSymbolLayerId("", 0))])

        label_settings.setFormat(fmt)
        self.polys_layer.labeling().setSettings(label_settings)

        self.check_layout_export("layout_export_2_sources_masking", 0)


if __name__ == '__main__':
    start_app()
    unittest.main()
