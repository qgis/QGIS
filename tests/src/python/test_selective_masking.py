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

from qgis.PyQt.QtCore import (
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
    QgsMapSettings,
    QgsCoordinateReferenceSystem,
    QgsRectangle,
    QgsProject,
    QgsSymbolLayerReference,
    QgsMapRendererParallelJob,
    QgsMapRendererSequentialJob,
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
                suffix = "_parallel" if do_parallel else "_sequential"
                res = self.checker.compareImages(control_name + suffix)
                self.report += self.checker.report()
                self.assertTrue(res)

                print("=== Rendering took {}s".format(float(t) / 1000.0))

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

    def test_layout_exports(self):
        """Test mask effects in a layout export at 300 dpi"""
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

        layout = QgsLayout(QgsProject.instance())
        page = QgsLayoutItemPage(layout)
        page.setPageSize(QgsLayoutSize(50, 33))
        layout.pageCollection().addPage(page)

        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(1, 1, 48, 32))
        map.setFrameEnabled(True)
        layout.addLayoutItem(map)
        map.setExtent(self.lines_layer.extent())
        map.setLayers([self.points_layer, self.lines_layer, self.polys_layer])

        image = QImage(591, 591, QImage.Format_RGB32)
        image.setDotsPerMeterX(int(300 / 25.3 * 1000))
        image.setDotsPerMeterY(int(300 / 25.3 * 1000))
        image.fill(0)
        p = QPainter(image)
        exporter = QgsLayoutExporter(layout)
        exporter.renderPage(p, 0)
        p.end()

        tmp = getTempfilePath('png')
        image.save(tmp)

        control_name = "layout_export"
        self.checker.setControlName(control_name)
        self.checker.setRenderedImage(tmp)
        res = self.checker.compareImages(control_name)
        self.report += self.checker.report()
        self.assertTrue(res)

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


if __name__ == '__main__':
    start_app()
    unittest.main()
