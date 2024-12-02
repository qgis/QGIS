"""QGIS Unit tests for QgsVirtualLayerDefinition

From build dir, run: ctest -R PyQgsSelectiveMasking -V

QGIS_PREFIX_PATH=/home/hme/src/QGIS/build_ninja/output PYTHONPATH=/home/hme/src/QGIS/build_ninja/output/python/:/home/hme/src/QGIS/build_ninja/output/python/plugins:/home/hme/src/QGIS/tests/src/python python3 ~/src/QGIS/tests/src/python/test_selective_masking.py

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Hugo Mercier / Oslandia"
__date__ = "28/06/2019"

import os
import subprocess
import tempfile

from qgis.PyQt.QtCore import QRectF, QSize, Qt, QUuid, QCoreApplication
from qgis.PyQt.QtGui import QColor, QImage, QPainter
from qgis.core import (
    Qgis,
    QgsCoordinateReferenceSystem,
    QgsLayout,
    QgsLayoutExporter,
    QgsLayoutItemMap,
    QgsLayoutItemPage,
    QgsLayoutSize,
    QgsLineSymbol,
    QgsMapRendererCache,
    QgsMapRendererCustomPainterJob,
    QgsMapRendererParallelJob,
    QgsMapRendererSequentialJob,
    QgsMapSettings,
    QgsMarkerLineSymbolLayer,
    QgsMarkerSymbol,
    QgsMaskMarkerSymbolLayer,
    QgsNullSymbolRenderer,
    QgsOuterGlowEffect,
    QgsPalLayerSettings,
    QgsPathResolver,
    QgsProject,
    QgsProjectFileTransform,
    QgsProperty,
    QgsRectangle,
    QgsRenderContext,
    QgsSingleSymbolRenderer,
    QgsSvgMarkerSymbolLayer,
    QgsSymbolLayerId,
    QgsSymbolLayerReference,
    QgsSymbolLayerUtils,
    QgsUnitTypes,
    QgsWkbTypes,
    QgsFontUtils,
    QgsSettings,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import getTempfilePath, getTestFont, unitTestDataPath

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


class TestSelectiveMasking(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "selective_masking"

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("SelectiveMaskingTestBase.com")
        QCoreApplication.setApplicationName("SelectiveMaskingTestBase")
        QgsSettings().clear()

        start_app()

    def setUp(self):
        self.map_settings = QgsMapSettings()
        crs = QgsCoordinateReferenceSystem("epsg:4326")
        extent = QgsRectangle(-123.0, 22.7, -76.4, 46.9)
        self.map_settings.setBackgroundColor(QColor(152, 219, 249))
        self.map_settings.setOutputSize(QSize(420, 280))
        self.map_settings.setOutputDpi(72)
        self.map_settings.setFlag(QgsMapSettings.Flag.Antialiasing, True)
        self.map_settings.setFlag(QgsMapSettings.Flag.UseAdvancedEffects, False)
        self.map_settings.setDestinationCrs(crs)
        self.map_settings.setExtent(extent)

        # load a predefined QGIS project
        self.assertTrue(
            QgsProject.instance().read(
                os.path.join(unitTestDataPath(), "selective_masking.qgs")
            )
        )

        self.points_layer = QgsProject.instance().mapLayersByName("points")[0]
        self.lines_layer = QgsProject.instance().mapLayersByName("lines")[0]
        # line layer with subsymbols
        self.lines_layer2 = QgsProject.instance().mapLayersByName("lines2")[0]
        # line layer with labels
        self.lines_with_labels = QgsProject.instance().mapLayersByName(
            "lines_with_labels"
        )[0]

        self.polys_layer = QgsProject.instance().mapLayersByName("polys")[0]
        # polygon layer with a rule based labeling
        self.polys_layer2 = QgsProject.instance().mapLayersByName("polys2")[0]

        self.raster_layer = QgsProject.instance().mapLayersByName("raster_layer")[0]

        # try to fix the font for where labels are defined
        # in order to have more stable image comparison tests
        for layer in [self.polys_layer, self.lines_with_labels, self.polys_layer2]:
            for provider in layer.labeling().subProviders():
                settings = layer.labeling().settings(provider)
                font = getTestFont()
                font.setPointSize(32)
                fmt = settings.format()
                fmt.setFont(font)
                fmt.setNamedStyle("Roman")
                fmt.setSize(32)
                fmt.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
                settings.setFormat(fmt)
                if layer.geometryType == QgsWkbTypes.GeometryType.PolygonGeometry:
                    settings.placement = QgsPalLayerSettings.Placement.OverPoint
                layer.labeling().setSettings(settings, provider)

        # order layers for rendering
        self.map_settings.setLayers(
            [self.points_layer, self.lines_layer, self.polys_layer]
        )

    def get_symbollayer(self, layer, ruleId, symbollayer_ids):
        """
        Returns the symbol layer according to given layer, ruleId (None if no rule) and the path
        to symbol layer id (for instance [0, 1])
        """
        renderer = layer.renderer()
        symbol = None
        if renderer.type() == "categorizedSymbol":
            i = renderer.categoryIndexForValue(ruleId)
            cat = renderer.categories()[i]
            symbol = cat.symbol()
        elif renderer.type() == "singleSymbol":
            symbol = renderer.symbol()

        symbollayer = symbol.symbolLayer(symbollayer_ids[0])
        for i in range(1, len(symbollayer_ids)):
            symbol = symbollayer.subSymbol()
            symbollayer = symbol.symbolLayer(symbollayer_ids[i])

        return symbollayer

    def get_symbollayer_ref(self, layer, ruleId, symbollayer_ids):
        """
        Returns the symbol layer according to given layer, ruleId (None if no rule) and the path
        to symbol layer id (for instance [0, 1])
        """
        symbollayer = self.get_symbollayer(layer, ruleId, symbollayer_ids)
        return QgsSymbolLayerReference(layer.id(), symbollayer.id())

    def check_renderings(
        self, map_settings, control_name, test_parallel_rendering: bool = True
    ):
        """Test a rendering with different configurations:
        - parallel rendering, no cache
        - sequential rendering, no cache
        - parallel rendering, with cache (rendered two times)
        - sequential rendering, with cache (rendered two times)
        """
        if test_parallel_rendering:
            parallel_tests = [False, True]
        else:
            parallel_tests = [False]

        for do_parallel in parallel_tests:
            for use_cache in [False, True]:
                print("=== parallel", do_parallel, "cache", use_cache)
                cache = None
                if use_cache:
                    cache = QgsMapRendererCache()
                    # render a first time to fill the cache
                    renderMapToImageWithTime(
                        map_settings, parallel=do_parallel, cache=cache
                    )
                img, t = renderMapToImageWithTime(
                    map_settings, parallel=do_parallel, cache=cache
                )

                suffix = ("_parallel" if do_parallel else "_sequential") + (
                    "_cache" if use_cache else "_nocache"
                )
                res = self.image_check(
                    control_name + suffix,
                    control_name,
                    img,
                    control_name,
                    allowed_mismatch=0,
                    color_tolerance=0,
                )

                self.assertTrue(res)

                print(f"=== Rendering took {float(t) / 1000.0}s")

    def check_layout_export(
        self, control_name, expected_nb_raster, layers=None, dpiTarget=None, extent=None
    ):
        """
        Generate a PDF layout export and control the output matches expected_filename
        """

        with tempfile.TemporaryDirectory() as temp_dir:
            # generate vector file
            layout = QgsLayout(QgsProject.instance())
            page = QgsLayoutItemPage(layout)
            page.setPageSize(QgsLayoutSize(50, 33))
            layout.pageCollection().addPage(page)

            map = QgsLayoutItemMap(layout)
            map.attemptSetSceneRect(QRectF(1, 1, 48, 32))
            map.setFrameEnabled(True)
            layout.addLayoutItem(map)
            map.setExtent(extent if extent is not None else self.lines_layer.extent())
            map.setLayers(
                layers
                if layers is not None
                else [self.points_layer, self.lines_layer, self.polys_layer]
            )

            settings = QgsLayoutExporter.PdfExportSettings()

            if dpiTarget is not None:
                settings.dpi = dpiTarget

            exporter = QgsLayoutExporter(layout)
            result_filename = os.path.join(temp_dir, "export.pdf")
            exporter.exportToPdf(result_filename, settings)
            self.assertTrue(os.path.exists(result_filename))

            # Generate a readable PDF file so we count raster in it
            result_txt = os.path.join(temp_dir, "export.txt")
            subprocess.run(
                [
                    "qpdf",
                    "--qdf",
                    "--object-streams=disable",
                    result_filename,
                    result_txt,
                ]
            )
            self.assertTrue(os.path.exists(result_txt))

            result = open(result_txt, "rb")
            result_lines = [l.decode("iso-8859-1") for l in result.readlines()]
            result.close()
            nb_raster = len([l for l in result_lines if "/Subtype /Image" in l])
            self.assertEqual(nb_raster, expected_nb_raster)

            # Generate an image from pdf to compare with expected control image
            # keep PDF DPI resolution (300)
            image_result_filename = os.path.join(temp_dir, "export.png")
            subprocess.run(
                [
                    "pdftoppm",
                    result_filename,
                    os.path.splitext(image_result_filename)[0],
                    "-png",
                    "-r",
                    "300",
                    "-singlefile",
                ]
            )

            rendered_image = QImage(image_result_filename)
            res = self.image_check(
                control_name,
                control_name,
                rendered_image,
                control_name,
                allowed_mismatch=0,
                color_tolerance=0,
            )

            self.assertTrue(res)

    def test_save_restore_references(self):
        """
        Test saving and restoring symbol layer references
        """

        # simple ids
        mask_layer = QgsMaskMarkerSymbolLayer()
        mask_layer.setMasks(
            [
                self.get_symbollayer_ref(self.lines_layer, "", [0]),
                self.get_symbollayer_ref(self.lines_layer2, "some_id", [1, 0]),
                self.get_symbollayer_ref(self.polys_layer, "some_other_id", [0]),
            ]
        )

        props = mask_layer.properties()

        print(f"props={props}")

        mask_layer2 = QgsMaskMarkerSymbolLayer.create(props)
        self.assertEqual(
            mask_layer2.masks(),
            [
                self.get_symbollayer_ref(self.lines_layer, "", [0]),
                self.get_symbollayer_ref(self.lines_layer2, "some_id", [1, 0]),
                self.get_symbollayer_ref(self.polys_layer, "some_other_id", [0]),
            ],
        )

    def test_migrate_old_references(self):
        """
        Since QGIS 3.30, QgsSymbolLayerReference has change its definition, so we test we can migrate
        old reference to new ones
        """

        # test label mask
        label_settings = self.polys_layer.labeling().settings()
        fmt = label_settings.format()
        # enable a mask
        fmt.mask().setEnabled(True)
        fmt.mask().setSize(4.0)
        # and mask other symbol layers underneath
        oldMaskRefs = [
            # the black part of roads
            QgsSymbolLayerReference(
                self.lines_layer2.id(), QgsSymbolLayerId("", [1, 0])
            ),
            # the black jets
            QgsSymbolLayerReference(
                self.points_layer.id(), QgsSymbolLayerId("B52", [0])
            ),
            QgsSymbolLayerReference(
                self.points_layer.id(), QgsSymbolLayerId("Jet", [0])
            ),
        ]
        fmt.mask().setMaskedSymbolLayers(oldMaskRefs)

        label_settings.setFormat(fmt)
        self.polys_layer.labeling().setSettings(label_settings)

        self.assertEqual(
            [
                slRef.symbolLayerIdV2()
                for slRef in self.polys_layer.labeling()
                .settings()
                .format()
                .mask()
                .maskedSymbolLayers()
            ],
            ["", "", ""],
        )
        self.assertEqual(
            [
                slRef.symbolLayerId()
                for slRef in self.polys_layer.labeling()
                .settings()
                .format()
                .mask()
                .maskedSymbolLayers()
            ],
            [slRef.symbolLayerId() for slRef in oldMaskRefs],
        )

        QgsProjectFileTransform.fixOldSymbolLayerReferences(
            QgsProject.instance().mapLayers()
        )

        self.assertEqual(
            [
                QUuid(slRef.symbolLayerIdV2()).isNull()
                for slRef in self.polys_layer.labeling()
                .settings()
                .format()
                .mask()
                .maskedSymbolLayers()
            ],
            [False, False, False],
        )
        self.assertEqual(
            [
                slRef.symbolLayerIdV2()
                for slRef in self.polys_layer.labeling()
                .settings()
                .format()
                .mask()
                .maskedSymbolLayers()
            ],
            [
                self.get_symbollayer(self.lines_layer2, "", [1, 0]).id(),
                self.get_symbollayer(self.points_layer, "B52", [0]).id(),
                self.get_symbollayer(self.points_layer, "Jet", [0]).id(),
            ],
        )
        self.assertEqual(
            [
                slRef.symbolLayerId()
                for slRef in self.polys_layer.labeling()
                .settings()
                .format()
                .mask()
                .maskedSymbolLayers()
            ],
            [QgsSymbolLayerId(), QgsSymbolLayerId(), QgsSymbolLayerId()],
        )

        # test symbol layer masks
        p = QgsMarkerSymbol.createSimple({"color": "#fdbf6f", "size": "7"})
        self.points_layer.setRenderer(QgsSingleSymbolRenderer(p))

        circle_symbol = QgsMarkerSymbol.createSimple({"size": "10"})
        mask_layer = QgsMaskMarkerSymbolLayer()
        mask_layer.setSubSymbol(circle_symbol)
        oldMaskRefs = [
            QgsSymbolLayerReference(
                self.lines_layer2.id(), QgsSymbolLayerId("", [1, 0])
            )
        ]
        mask_layer.setMasks(oldMaskRefs)

        # add this mask layer to the point layer
        self.points_layer.renderer().symbol().appendSymbolLayer(mask_layer)

        self.assertEqual(
            [
                slRef.symbolLayerIdV2()
                for slRef in self.points_layer.renderer()
                .symbol()
                .symbolLayers()[1]
                .masks()
            ],
            [""],
        )
        self.assertEqual(
            [
                slRef.symbolLayerId()
                for slRef in self.points_layer.renderer()
                .symbol()
                .symbolLayers()[1]
                .masks()
            ],
            [slRef.symbolLayerId() for slRef in oldMaskRefs],
        )

        QgsProjectFileTransform.fixOldSymbolLayerReferences(
            QgsProject.instance().mapLayers()
        )

        self.assertEqual(
            [
                QUuid(slRef.symbolLayerIdV2()).isNull()
                for slRef in self.points_layer.renderer()
                .symbol()
                .symbolLayers()[1]
                .masks()
            ],
            [False],
        )
        self.assertEqual(
            [
                slRef.symbolLayerIdV2()
                for slRef in self.points_layer.renderer()
                .symbol()
                .symbolLayers()[1]
                .masks()
            ],
            [self.get_symbollayer(self.lines_layer2, "", [1, 0]).id()],
        )
        self.assertEqual(
            [
                slRef.symbolLayerId()
                for slRef in self.points_layer.renderer()
                .symbol()
                .symbolLayers()[1]
                .masks()
            ],
            [QgsSymbolLayerId()],
        )

    def test_label_mask(self):
        # modify labeling settings
        label_settings = self.polys_layer.labeling().settings()
        fmt = label_settings.format()
        # enable a mask
        fmt.mask().setEnabled(True)
        fmt.mask().setSize(4.0)
        # and mask other symbol layers underneath
        fmt.mask().setMaskedSymbolLayers(
            [
                # the black part of roads
                self.get_symbollayer_ref(self.lines_layer, "", [0]),
                # the black jets
                self.get_symbollayer_ref(self.points_layer, "B52", [0]),
                self.get_symbollayer_ref(self.points_layer, "Jet", [0]),
            ]
        )

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
        fmt.mask().setMaskedSymbolLayers(
            [
                # the black part of roads
                self.get_symbollayer_ref(self.lines_with_labels, "", [0]),
                # the black jets
                self.get_symbollayer_ref(self.points_layer, "B52", [0]),
                self.get_symbollayer_ref(self.points_layer, "Jet", [0]),
            ]
        )

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
        fmt.mask().setMaskedSymbolLayers(
            [
                # polygons
                self.get_symbollayer_ref(self.polys_layer, "", [0]),
            ]
        )
        label_settings.setFormat(fmt)
        self.lines_with_labels.labeling().setSettings(label_settings)

        # new map settings with a line symbology that has labels
        self.map_settings.setLayers(
            [self.points_layer, self.lines_with_labels, self.polys_layer]
        )
        self.check_renderings(self.map_settings, "multiple_label_masks_different_sets")
        # restore map settings
        self.map_settings.setLayers(
            [self.points_layer, self.lines_layer, self.polys_layer]
        )

    def test_multiple_label_masks_same_set(self):
        # modify labeling settings of the polys layer
        label_settings = self.polys_layer.labeling().settings()
        fmt = label_settings.format()
        # enable a mask
        fmt.mask().setEnabled(True)
        fmt.mask().setSize(4.0)
        # and mask other symbol layers underneath
        fmt.mask().setMaskedSymbolLayers(
            [
                # the black part of roads
                self.get_symbollayer_ref(self.lines_with_labels, "", [0]),
            ]
        )

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
        fmt.mask().setMaskedSymbolLayers(
            [
                # the black part of roads
                self.get_symbollayer_ref(self.lines_with_labels, "", [0]),
            ]
        )
        label_settings.setFormat(fmt)
        self.lines_with_labels.labeling().setSettings(label_settings)

        # new map settings with a line symbology that has labels
        self.map_settings.setLayers(
            [self.points_layer, self.lines_with_labels, self.polys_layer]
        )
        self.check_renderings(self.map_settings, "multiple_label_masks_same_set")
        # restore map settings
        self.map_settings.setLayers(
            [self.points_layer, self.lines_layer, self.polys_layer]
        )

    def test_label_mask_subsymbol(self):
        # new map settings with a line symbology that has sub symbols
        self.map_settings.setLayers(
            [self.points_layer, self.lines_layer2, self.polys_layer]
        )

        # modify labeling settings
        label_settings = self.polys_layer.labeling().settings()
        fmt = label_settings.format()
        # enable a mask
        fmt.mask().setEnabled(True)
        fmt.mask().setSize(4.0)
        # and mask other symbol layers underneath
        fmt.mask().setMaskedSymbolLayers(
            [
                # mask only vertical segments of "roads"
                self.get_symbollayer_ref(self.lines_layer2, "", [1, 0]),
                # the black jets
                self.get_symbollayer_ref(self.points_layer, "B52", [0]),
                self.get_symbollayer_ref(self.points_layer, "Jet", [0]),
            ]
        )

        label_settings.setFormat(fmt)
        self.polys_layer.labeling().setSettings(label_settings)

        format = self.polys_layer.labeling().settings().format()
        self.assertTrue(format.mask().enabled())

        self.check_renderings(self.map_settings, "label_mask_subsymbol")

        # restore original map settings
        self.map_settings.setLayers(
            [self.points_layer, self.lines_layer, self.polys_layer]
        )

    def test_label_mask_dd(self):
        """test label mask with data defined properties"""
        label_settings = self.polys_layer.labeling().settings()
        fmt = label_settings.format()
        fmt.mask().setEnabled(False)
        fmt.mask().setSize(1.0)
        fmt.mask().setOpacity(0.42)
        # mask other symbol layers underneath
        fmt.mask().setMaskedSymbolLayers(
            [
                # the black part of roads
                self.get_symbollayer_ref(self.lines_layer, "", [0]),
                # the black jets
                self.get_symbollayer_ref(self.points_layer, "B52", [0]),
                self.get_symbollayer_ref(self.points_layer, "Jet", [0]),
            ]
        )

        # overwrite with data-defined properties
        fmt.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.MaskEnabled, QgsProperty.fromExpression("1")
        )
        fmt.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.MaskBufferSize,
            QgsProperty.fromExpression("4.0"),
        )
        fmt.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.MaskOpacity,
            QgsProperty.fromExpression("100.0"),
        )

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
        self.map_settings.setLayers(
            [self.points_layer, self.lines_layer, self.polys_layer2]
        )

        # modify labeling settings of one rule
        for child in self.polys_layer2.labeling().rootRule().children():
            if child.description() == "Tadam":
                break
        label_settings = child.settings()
        label_settings.priority = 3
        fmt = label_settings.format()
        # enable a mask
        fmt.mask().setEnabled(True)
        fmt.mask().setSize(4.0)
        # and mask other symbol layers underneath
        fmt.mask().setMaskedSymbolLayers(
            [
                # the black part of roads
                self.get_symbollayer_ref(self.lines_layer, "", [0]),
                # the black jets
                self.get_symbollayer_ref(self.points_layer, "B52", [0]),
                self.get_symbollayer_ref(self.points_layer, "Jet", [0]),
            ]
        )

        label_settings.setFormat(fmt)
        child.setSettings(label_settings)

        # modify labeling settings of another rule
        for child in self.polys_layer2.labeling().rootRule().children():
            if child.description() != "Tadam":
                break
        label_settings = child.settings()
        fmt = label_settings.format()
        # enable a mask
        fmt.mask().setEnabled(True)
        fmt.mask().setSize(4.0)
        # and mask other symbol layers underneath
        fmt.mask().setMaskedSymbolLayers(
            [
                # the polygons
                self.get_symbollayer_ref(self.polys_layer2, "", [0]),
            ]
        )
        label_settings.setFormat(fmt)
        child.setSettings(label_settings)

        self.check_renderings(self.map_settings, "rule_label_mask")

        # restore map settings
        self.map_settings.setLayers(
            [self.points_layer, self.lines_layer, self.polys_layer]
        )

    def test_label_mask_symbol_levels(self):
        # modify labeling settings
        label_settings = self.polys_layer.labeling().settings()
        fmt = label_settings.format()
        # enable a mask
        fmt.mask().setEnabled(True)
        fmt.mask().setSize(4.0)
        # and mask other symbol layers underneath
        fmt.mask().setMaskedSymbolLayers(
            [
                # the black part of roads
                self.get_symbollayer_ref(self.lines_layer, "", [0]),
                # the black jets
                self.get_symbollayer_ref(self.points_layer, "B52", [0]),
                self.get_symbollayer_ref(self.points_layer, "Jet", [0]),
            ]
        )

        label_settings.setFormat(fmt)
        self.polys_layer.labeling().setSettings(label_settings)

        format = self.polys_layer.labeling().settings().format()
        self.assertTrue(format.mask().enabled())

        # enable symbol levels
        self.lines_layer.renderer().setUsingSymbolLevels(True)

        self.check_renderings(self.map_settings, "label_mask_symbol_levels")

    def test_symbol_layer_mask(self):
        p = QgsMarkerSymbol.createSimple({"color": "#fdbf6f", "size": "7"})
        self.points_layer.setRenderer(QgsSingleSymbolRenderer(p))

        circle_symbol = QgsMarkerSymbol.createSimple({"size": "10"})
        mask_layer = QgsMaskMarkerSymbolLayer()
        mask_layer.setSubSymbol(circle_symbol)
        mask_layer.setMasks(
            [
                # the black part of roads
                self.get_symbollayer_ref(self.lines_layer, "", [0]),
            ]
        )
        # add this mask layer to the point layer
        self.points_layer.renderer().symbol().appendSymbolLayer(mask_layer)

        self.check_renderings(self.map_settings, "sl_mask")

    def test_multiple_masks_same_symbol_layer(self):
        """Test multiple masks that occlude the same symbol layer"""
        #
        # 1. a symbol layer mask
        #
        p = QgsMarkerSymbol.createSimple({"color": "#fdbf6f", "size": "7"})
        self.points_layer.setRenderer(QgsSingleSymbolRenderer(p))

        circle_symbol = QgsMarkerSymbol.createSimple({"size": "10"})
        mask_layer = QgsMaskMarkerSymbolLayer()
        mask_layer.setSubSymbol(circle_symbol)
        mask_layer.setMasks(
            [
                # the black part of roads
                self.get_symbollayer_ref(self.lines_layer, "", [0]),
            ]
        )
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
        fmt.mask().setMaskedSymbolLayers(
            [
                # the black part of roads
                self.get_symbollayer_ref(self.lines_layer, "", [0])
            ]
        )
        label_settings.setFormat(fmt)
        self.polys_layer.labeling().setSettings(label_settings)

        self.check_renderings(self.map_settings, "multiple_masks_same_sl")

    def test_multiple_masks_different_symbol_layers_same_layer(self):
        """Test multiple masks that occlude different symbol layers of the same layer.
        The UI should disallow this settings. We test here that only one mask is retained
        """
        #
        # 1. a symbol layer mask
        #
        p = QgsMarkerSymbol.createSimple({"color": "#fdbf6f", "size": "7"})
        self.points_layer.setRenderer(QgsSingleSymbolRenderer(p))

        circle_symbol = QgsMarkerSymbol.createSimple({"size": "10"})
        mask_layer = QgsMaskMarkerSymbolLayer()
        mask_layer.setSubSymbol(circle_symbol)
        mask_layer.setMasks(
            [
                # the yellow part of roads
                self.get_symbollayer_ref(self.lines_layer, "", [1]),
            ]
        )
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
        fmt.mask().setMaskedSymbolLayers(
            [
                # the black part of roads
                self.get_symbollayer_ref(self.lines_layer, "", [0])
            ]
        )
        label_settings.setFormat(fmt)
        self.polys_layer.labeling().setSettings(label_settings)

        self.check_renderings(self.map_settings, "multiple_masks_different_sl")

    def test_multiple_masks_different_symbol_layers_same_layer2(self):
        """Test multiple masks that occlude different symbol layers of the same layer - 2nd possible order
        The UI should disallow this settings. We test here that only one mask is retained
        """
        #
        # 1. a symbol layer mask
        #
        p = QgsMarkerSymbol.createSimple({"color": "#fdbf6f", "size": "7"})
        self.points_layer.setRenderer(QgsSingleSymbolRenderer(p))

        circle_symbol = QgsMarkerSymbol.createSimple({"size": "10"})
        mask_layer = QgsMaskMarkerSymbolLayer()
        mask_layer.setSubSymbol(circle_symbol)
        mask_layer.setMasks(
            [
                # the black part of roads
                self.get_symbollayer_ref(self.lines_layer, "", [0]),
            ]
        )
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
        fmt.mask().setMaskedSymbolLayers(
            [
                # the yellow part of roads
                self.get_symbollayer_ref(self.lines_layer, "", [1])
            ]
        )
        label_settings.setFormat(fmt)
        self.polys_layer.labeling().setSettings(label_settings)

        self.check_renderings(self.map_settings, "multiple_masks_different_sl2")

    def test_mask_symbollayer_preview(self):
        #
        # Masks should be visible in previews
        #
        p = QgsMarkerSymbol.createSimple({"color": "#fdbf6f", "size": "7"})

        circle_symbol = QgsMarkerSymbol.createSimple({"size": "10"})
        mask_layer = QgsMaskMarkerSymbolLayer()
        mask_layer.setSubSymbol(circle_symbol)
        p.insertSymbolLayer(0, mask_layer)

        for control_name, render_function in [
            ("as_image", lambda: p.asImage(QSize(64, 64)).save(tmp)),
            ("as_big_preview", lambda: p.bigSymbolPreviewImage().save(tmp)),
            (
                "sl_preview",
                lambda: QgsSymbolLayerUtils.symbolLayerPreviewIcon(
                    mask_layer, QgsUnitTypes.RenderUnit.RenderPixels, QSize(64, 64)
                )
                .pixmap(QSize(64, 64))
                .save(tmp),
            ),
        ]:
            with tempfile.TemporaryDirectory() as temp_dir:
                tmp = os.path.join(temp_dir, "render.png")
                render_function()

                rendered_image = QImage(tmp)

                res = self.image_check(
                    control_name,
                    control_name,
                    rendered_image,
                    control_name,
                    allowed_mismatch=90,
                    color_tolerance=0,
                )

                self.assertTrue(res)

    def test_mask_with_effect(self):
        p = QgsMarkerSymbol.createSimple({"color": "#fdbf6f", "size": "7"})
        self.points_layer.setRenderer(QgsSingleSymbolRenderer(p))

        circle_symbol = QgsMarkerSymbol.createSimple({"size": "12"})
        mask_layer = QgsMaskMarkerSymbolLayer()
        mask_layer.setSubSymbol(circle_symbol)
        mask_layer.setMasks(
            [
                # the yellow part of roads
                self.get_symbollayer_ref(self.lines_layer, "", [1]),
            ]
        )
        # add an outer glow effect to the mask layer
        blur = QgsOuterGlowEffect.create(
            {
                "enabled": "1",
                "blur_level": "6.445",
                "blur_unit": "MM",
                "opacity": "1",
                "spread": "0.6",
                "spread_unit": "MM",
                "color1": "0,0,255,255",
                "draw_mode": "2",
            }
        )
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
        fmt.mask().setMaskedSymbolLayers(
            [
                # the black part of roads
                self.get_symbollayer_ref(self.lines_layer, "", [0]),
                # the black jets
                self.get_symbollayer_ref(self.points_layer, "B52", [0]),
                self.get_symbollayer_ref(self.points_layer, "Jet", [0]),
            ]
        )

        # add an outer glow effect to the mask
        blur = QgsOuterGlowEffect.create(
            {
                "enabled": "1",
                "blur_level": "6.445",
                "blur_unit": "MM",
                "opacity": "1",
                "spread": "0.6",
                "spread_unit": "MM",
                "color1": "0,0,255,255",
                "draw_mode": "2",
            }
        )
        fmt.mask().setPaintEffect(blur)

        label_settings.setFormat(fmt)
        self.polys_layer.labeling().setSettings(label_settings)

        format = self.polys_layer.labeling().settings().format()
        self.assertTrue(format.mask().enabled())

        self.check_renderings(self.map_settings, "label_mask_with_effect")

        # test that force vector output has no impact on the result
        self.map_settings.setFlag(Qgis.MapSettingsFlag.ForceVectorOutput, True)
        # skip parallel rendering for this check, as force vector output is ignored when parallel rendering
        # is used
        self.check_renderings(
            self.map_settings, "label_mask_with_effect", test_parallel_rendering=False
        )

    def test_different_dpi_target(self):
        """Test with raster layer and a target dpi"""

        # modify labeling settings
        label_settings = self.polys_layer.labeling().settings()
        fmt = label_settings.format()
        # enable a mask
        fmt.mask().setEnabled(True)
        fmt.mask().setSize(4.0)
        # and mask other symbol layers underneath
        fmt.mask().setMaskedSymbolLayers(
            [
                # the black part of roads
                self.get_symbollayer_ref(self.lines_layer, "", [0])
            ]
        )

        label_settings.setFormat(fmt)
        self.polys_layer.labeling().setSettings(label_settings)

        self.map_settings.setLayers(
            [self.lines_layer, self.polys_layer, self.raster_layer]
        )
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
        fmt.mask().setMaskedSymbolLayers(
            [
                # the black part of roads
                self.get_symbollayer_ref(self.lines_layer, "", [0]),
                # the black jets
                self.get_symbollayer_ref(self.points_layer, "B52", [0]),
                self.get_symbollayer_ref(self.points_layer, "Jet", [0]),
            ]
        )

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
        fmt.mask().setMaskedSymbolLayers(
            [
                # the black part of roads
                self.get_symbollayer_ref(self.lines_layer, "", [0]),
                # the black jets
                self.get_symbollayer_ref(self.points_layer, "B52", [0]),
                self.get_symbollayer_ref(self.points_layer, "Jet", [0]),
            ]
        )

        # add an outer glow effect to the mask
        blur = QgsOuterGlowEffect.create(
            {
                "enabled": "1",
                "blur_level": "3.445",
                "blur_unit": "MM",
                "opacity": "1",
                "spread": "0.06",
                "spread_unit": "MM",
                "color1": "0,0,255,255",
                "draw_mode": "2",
            }
        )
        fmt.mask().setPaintEffect(blur)

        label_settings.setFormat(fmt)
        self.polys_layer.labeling().setSettings(label_settings)

        # 4 rasters : Image and its mask for masked point and lines layer
        self.check_layout_export("layout_export_w_effects", 4)

    def test_layout_export_marker_masking(self):
        """Test mask effects in a layout export with a marker symbol masking"""

        p = QgsMarkerSymbol.createSimple({"color": "#fdbf6f", "size": "3"})
        self.points_layer.setRenderer(QgsSingleSymbolRenderer(p))

        circle_symbol = QgsMarkerSymbol.createSimple({"size": "6"})
        mask_layer = QgsMaskMarkerSymbolLayer()
        mask_layer.setSubSymbol(circle_symbol)
        mask_layer.setMasks(
            [
                # the black part of roads
                self.get_symbollayer_ref(self.lines_layer, "", [0]),
            ]
        )
        # add this mask layer to the point layer
        self.points_layer.renderer().symbol().appendSymbolLayer(mask_layer)

        self.check_layout_export("layout_export_marker_masking", 0)

    def test_layout_export_marker_masking_w_effects(self):
        """Test mask effects in a layout export with a marker symbol masking"""

        p = QgsMarkerSymbol.createSimple({"color": "#fdbf6f", "size": "3"})
        self.points_layer.setRenderer(QgsSingleSymbolRenderer(p))

        circle_symbol = QgsMarkerSymbol.createSimple({"size": "6"})
        mask_layer = QgsMaskMarkerSymbolLayer()
        mask_layer.setSubSymbol(circle_symbol)
        mask_layer.setMasks(
            [
                # the black part of roads
                self.get_symbollayer_ref(self.lines_layer, "", [0]),
            ]
        )

        # add an outer glow effect to the mask
        blur = QgsOuterGlowEffect.create(
            {
                "enabled": "1",
                "blur_level": "3.445",
                "blur_unit": "MM",
                "opacity": "1",
                "spread": "0.06",
                "spread_unit": "MM",
                "color1": "0,0,255,255",
                "draw_mode": "2",
            }
        )

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
        fmt.mask().setMaskedSymbolLayers(
            [
                # the black part of roads
                self.get_symbollayer_ref(self.lines_layer, "", [0]),
                # the black jets
                self.get_symbollayer_ref(self.points_layer, "B52", [0]),
                self.get_symbollayer_ref(self.points_layer, "Jet", [0]),
            ]
        )

        label_settings.setFormat(fmt)
        self.polys_layer.labeling().setSettings(label_settings)

        # 1 raster : the raster layer
        self.check_layout_export(
            "layout_export_w_raster",
            1,
            [self.lines_layer, self.polys_layer, self.raster_layer],
        )

    def test_layout_export_w_force_raster_render(self):
        """
        Test layout export with a marker symbol masking forced to be render as raster
        We expect the lines to be masked and the whole output needs to be vector except
        the marker layer forced as raster
        """

        p = QgsMarkerSymbol.createSimple({"color": "#fdbf6f", "size": "3"})
        self.points_layer.setRenderer(QgsSingleSymbolRenderer(p))

        circle_symbol = QgsMarkerSymbol.createSimple({"size": "6"})
        mask_layer = QgsMaskMarkerSymbolLayer()
        mask_layer.setSubSymbol(circle_symbol)
        mask_layer.setMasks(
            [
                # the black part of roads
                self.get_symbollayer_ref(self.lines_layer, "", [0]),
            ]
        )
        # add this mask layer to the point layer
        self.points_layer.renderer().symbol().appendSymbolLayer(mask_layer)
        self.points_layer.renderer().setForceRasterRender(True)

        # 2 rasters : Image and its mask for the points layer
        self.check_layout_export(
            "layout_export_force_raster_render",
            2,
            [self.points_layer, self.lines_layer],
        )

    def test_layout_export_marker_masking_w_transparency(self):
        """Test layout export with a marker symbol masking which has an opacity lower than 1"""

        p = QgsMarkerSymbol.createSimple({"color": "#fdbf6f", "size": "3"})
        self.points_layer.setRenderer(QgsSingleSymbolRenderer(p))

        circle_symbol = QgsMarkerSymbol.createSimple({"size": "6"})
        circle_symbol.setOpacity(0.5)
        mask_layer = QgsMaskMarkerSymbolLayer()
        mask_layer.setSubSymbol(circle_symbol)
        mask_layer.setMasks(
            [
                # the black part of roads
                self.get_symbollayer_ref(self.lines_layer, "", [0]),
            ]
        )
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
        fmt.mask().setMaskedSymbolLayers(
            [
                # the black part of roads
                self.get_symbollayer_ref(self.lines_layer, "", [0]),
                # the black jets
                self.get_symbollayer_ref(self.points_layer, "B52", [0]),
                self.get_symbollayer_ref(self.points_layer, "Jet", [0]),
            ]
        )

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
        fmt.mask().setMaskedSymbolLayers(
            [
                # the black part of roads
                self.get_symbollayer_ref(self.lines_layer, "", [0])
            ]
        )

        label_settings.setFormat(fmt)
        self.polys_layer.labeling().setSettings(label_settings)

        self.map_settings.setLayers(
            [self.lines_layer, self.polys_layer, self.raster_layer]
        )
        self.map_settings.setOutputDpi(81)
        self.map_settings.setDpiTarget(300)
        self.map_settings.setFlag(Qgis.MapSettingsFlag.ForceVectorOutput, True)

        image = QImage(
            self.map_settings.deviceOutputSize(), self.map_settings.outputImageFormat()
        )
        image.setDevicePixelRatio(self.map_settings.devicePixelRatio())
        image.setDotsPerMeterX(int(1000 * self.map_settings.outputDpi() / 25.4))
        image.setDotsPerMeterY(int(1000 * self.map_settings.outputDpi() / 25.4))
        image.fill(Qt.GlobalColor.transparent)
        pImg = QPainter()
        pImg.begin(image)
        job = QgsMapRendererCustomPainterJob(self.map_settings, pImg)
        job.start()
        job.waitForFinished()
        pImg.end()

        control_name = "different_dpi_target_vector"
        res = self.image_check(
            control_name,
            control_name,
            image,
            control_name,
            allowed_mismatch=0,
            color_tolerance=0,
        )
        self.assertTrue(res)

        # Same test with high dpi
        self.map_settings.setDevicePixelRatio(2)
        image = QImage(
            self.map_settings.deviceOutputSize(), self.map_settings.outputImageFormat()
        )
        image.setDevicePixelRatio(self.map_settings.devicePixelRatio())
        image.setDotsPerMeterX(int(1000 * self.map_settings.outputDpi() / 25.4))
        image.setDotsPerMeterY(int(1000 * self.map_settings.outputDpi() / 25.4))
        image.fill(Qt.GlobalColor.transparent)
        pImg = QPainter()
        pImg.begin(image)
        job = QgsMapRendererCustomPainterJob(self.map_settings, pImg)
        job.start()
        job.waitForFinished()
        pImg.end()

        control_name = "different_dpi_target_vector_hdpi"
        res = self.image_check(
            control_name,
            control_name,
            image,
            control_name,
            allowed_mismatch=0,
            color_tolerance=0,
        )
        self.assertTrue(res)

    def test_layout_export_2_sources_masking(self):
        """Test masking with 2 different sources"""

        # mask with points layer circles...
        p = QgsMarkerSymbol.createSimple({"color": "#fdbf6f", "size": "3"})
        self.points_layer.setRenderer(QgsSingleSymbolRenderer(p))

        circle_symbol = QgsMarkerSymbol.createSimple({"size": "6"})
        mask_layer = QgsMaskMarkerSymbolLayer()
        mask_layer.setSubSymbol(circle_symbol)
        mask_layer.setMasks(
            [
                # the black part of roads
                self.get_symbollayer_ref(self.lines_layer, "", [0]),
            ]
        )
        self.points_layer.renderer().symbol().appendSymbolLayer(mask_layer)

        # ...and with text
        label_settings = self.polys_layer.labeling().settings()
        fmt = label_settings.format()

        fmt.font().setPointSize(4)
        fmt.mask().setEnabled(True)
        fmt.mask().setSize(1.0)
        # and mask other symbol layers underneath
        fmt.mask().setMaskedSymbolLayers(
            [
                # the black part of roads
                self.get_symbollayer_ref(self.lines_layer, "", [0])
            ]
        )

        label_settings.setFormat(fmt)
        self.polys_layer.labeling().setSettings(label_settings)

        self.check_layout_export("layout_export_2_sources_masking", 0)

    def test_raster_line_pattern_fill(self):
        """
        Test raster rendering and masking when a line pattern fill symbol layer is involved
        """
        self.assertTrue(
            QgsProject.instance().read(
                os.path.join(
                    unitTestDataPath(), "selective_masking_fill_symbollayer.qgz"
                )
            )
        )

        layer = QgsProject.instance().mapLayersByName("line_pattern_fill")[0]
        self.assertTrue(layer)

        self.assertTrue(len(layer.labeling().subProviders()), 1)
        settings = layer.labeling().settings()
        fmt = settings.format()
        fmt.setFont(QgsFontUtils.getStandardTestFont("Bold"))
        fmt.setSize(30)
        fmt.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        settings.setFormat(fmt)
        layer.labeling().setSettings(settings)

        map_settings = QgsMapSettings()
        crs = QgsCoordinateReferenceSystem("epsg:4326")
        extent = QgsRectangle(0, -1, 0.5, 0.8)
        map_settings.setBackgroundColor(QColor(152, 219, 249))
        map_settings.setOutputSize(QSize(420, 280))
        map_settings.setOutputDpi(72)
        map_settings.setFlag(QgsMapSettings.Flag.Antialiasing, True)
        map_settings.setFlag(QgsMapSettings.Flag.UseAdvancedEffects, False)
        map_settings.setDestinationCrs(crs)
        map_settings.setExtent(extent)

        map_settings.setLayers([layer])

        self.check_renderings(map_settings, "line_pattern_fill")

    def test_vector_line_pattern_fill(self):
        """
        Test vector rendering and masking when a line pattern fill symbol layer is involved
        """
        self.assertTrue(
            QgsProject.instance().read(
                os.path.join(
                    unitTestDataPath(), "selective_masking_fill_symbollayer.qgz"
                )
            )
        )

        layer = QgsProject.instance().mapLayersByName("line_pattern_fill")[0]
        self.assertTrue(layer)

        self.assertTrue(len(layer.labeling().subProviders()), 1)
        settings = layer.labeling().settings()
        fmt = settings.format()
        fmt.setFont(QgsFontUtils.getStandardTestFont("Bold"))
        fmt.setSize(9)
        fmt.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        settings.setFormat(fmt)
        layer.labeling().setSettings(settings)

        map_settings = QgsMapSettings()
        crs = QgsCoordinateReferenceSystem("epsg:4326")
        extent = QgsRectangle(
            -1.0073971192118132,
            -0.7875782447946843,
            0.87882587741257345,
            0.51640826470600099,
        )
        map_settings.setBackgroundColor(QColor(152, 219, 249))
        map_settings.setOutputSize(QSize(420, 280))
        map_settings.setOutputDpi(72)
        map_settings.setFlag(QgsMapSettings.Flag.Antialiasing, True)
        map_settings.setFlag(QgsMapSettings.Flag.UseAdvancedEffects, False)
        map_settings.setDestinationCrs(crs)

        map_settings.setLayers([layer])

        self.check_layout_export(
            "layout_export_line_pattern_fill", 0, [layer], extent=extent
        )

    def test_vector_point_pattern_fill(self):
        """
        Test vector rendering and masking when a point pattern fill symbol layer is involved
        """
        self.assertTrue(
            QgsProject.instance().read(
                os.path.join(
                    unitTestDataPath(), "selective_masking_fill_symbollayer.qgz"
                )
            )
        )

        layer = QgsProject.instance().mapLayersByName("point_pattern_fill")[0]
        self.assertTrue(layer)

        self.assertTrue(len(layer.labeling().subProviders()), 1)
        settings = layer.labeling().settings()
        fmt = settings.format()
        fmt.setFont(QgsFontUtils.getStandardTestFont("Bold"))
        fmt.setSize(9)
        fmt.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        settings.setFormat(fmt)
        layer.labeling().setSettings(settings)

        map_settings = QgsMapSettings()
        crs = QgsCoordinateReferenceSystem("epsg:4326")
        extent = QgsRectangle(
            -1.0073971192118132,
            -0.7875782447946843,
            0.87882587741257345,
            0.51640826470600099,
        )
        map_settings.setBackgroundColor(QColor(152, 219, 249))
        map_settings.setOutputSize(QSize(420, 280))
        map_settings.setOutputDpi(72)
        map_settings.setFlag(QgsMapSettings.Flag.Antialiasing, True)
        map_settings.setFlag(QgsMapSettings.Flag.UseAdvancedEffects, False)
        map_settings.setDestinationCrs(crs)
        map_settings.setExtent(extent)

        map_settings.setLayers([layer])

        self.check_layout_export(
            "layout_export_point_pattern_fill", 0, [layer], extent=extent
        )

    def test_vector_centroid_fill(self):
        """
        Test masking when a centroid fill symbol layer is involved
        """
        self.assertTrue(
            QgsProject.instance().read(
                os.path.join(
                    unitTestDataPath(), "selective_masking_fill_symbollayer.qgz"
                )
            )
        )

        layer = QgsProject.instance().mapLayersByName("centroid_fill")[0]
        self.assertTrue(layer)

        self.assertTrue(len(layer.labeling().subProviders()), 1)
        settings = layer.labeling().settings()
        fmt = settings.format()
        fmt.setFont(QgsFontUtils.getStandardTestFont("Bold"))
        fmt.setSize(9)
        fmt.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        settings.setFormat(fmt)
        layer.labeling().setSettings(settings)

        map_settings = QgsMapSettings()
        crs = QgsCoordinateReferenceSystem("epsg:4326")
        extent = QgsRectangle(
            -1.0073971192118132,
            -0.7875782447946843,
            0.87882587741257345,
            0.51640826470600099,
        )
        map_settings.setBackgroundColor(QColor(152, 219, 249))
        map_settings.setOutputSize(QSize(420, 280))
        map_settings.setOutputDpi(72)
        map_settings.setFlag(QgsMapSettings.Flag.Antialiasing, True)
        map_settings.setFlag(QgsMapSettings.Flag.UseAdvancedEffects, False)
        map_settings.setDestinationCrs(crs)

        map_settings.setLayers([layer])

        self.check_layout_export(
            "layout_export_centroid_fill", 0, [layer], extent=extent
        )

    def test_vector_random_generator_fill(self):
        """
        Test masking when a random generator fill symbol layer is involved
        """
        self.assertTrue(
            QgsProject.instance().read(
                os.path.join(
                    unitTestDataPath(), "selective_masking_fill_symbollayer.qgz"
                )
            )
        )

        layer = QgsProject.instance().mapLayersByName("random_generator_fill")[0]
        self.assertTrue(layer)

        self.assertTrue(len(layer.labeling().subProviders()), 1)
        settings = layer.labeling().settings()
        fmt = settings.format()
        fmt.setFont(QgsFontUtils.getStandardTestFont("Bold"))
        fmt.setSize(9)
        fmt.setSizeUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        settings.setFormat(fmt)
        layer.labeling().setSettings(settings)

        map_settings = QgsMapSettings()
        crs = QgsCoordinateReferenceSystem("epsg:4326")
        extent = QgsRectangle(
            -1.0073971192118132,
            -0.7875782447946843,
            0.87882587741257345,
            0.51640826470600099,
        )
        map_settings.setBackgroundColor(QColor(152, 219, 249))
        map_settings.setOutputSize(QSize(420, 280))
        map_settings.setOutputDpi(72)
        map_settings.setFlag(QgsMapSettings.Flag.Antialiasing, True)
        map_settings.setFlag(QgsMapSettings.Flag.UseAdvancedEffects, False)
        map_settings.setDestinationCrs(crs)

        map_settings.setLayers([layer])

        self.check_layout_export(
            "layout_export_random_generator_fill", 0, [layer], extent=extent
        )

    def test_layout_export_svg_marker_masking(self):
        """Test layout export with a svg marker symbol masking"""

        svgPath = QgsSymbolLayerUtils.svgSymbolNameToPath(
            "gpsicons/plane.svg", QgsPathResolver()
        )

        sl = QgsSvgMarkerSymbolLayer(svgPath, 5)
        sl.setFillColor(QColor("blue"))
        sl.setStrokeColor(QColor("white"))
        sl.setStrokeWidth(0)

        p = QgsMarkerSymbol()
        p.changeSymbolLayer(0, sl)

        self.points_layer.setRenderer(QgsSingleSymbolRenderer(p))

        mask_layer = QgsMaskMarkerSymbolLayer()
        maskSl = QgsSvgMarkerSymbolLayer(svgPath, 8)
        pSl = QgsMarkerSymbol()
        pSl.changeSymbolLayer(0, maskSl)
        mask_layer.setSubSymbol(pSl)
        mask_layer.setMasks(
            [
                # the black part of roads
                self.get_symbollayer_ref(self.lines_layer, "", [0]),
            ]
        )
        # add this mask layer to the point layer
        self.points_layer.renderer().symbol().appendSymbolLayer(mask_layer)

        # no rasters
        self.check_layout_export(
            "layout_export_svg_marker_masking", 0, [self.points_layer, self.lines_layer]
        )

    def test_markerline_masked(self):
        """
        Test a layout export where a QgsMarkerLineSymbolLayer is masked
        """

        sl = QgsMarkerLineSymbolLayer(True, 7)
        circle_symbol = QgsMarkerSymbol.createSimple({"size": "3"})
        sl.setSubSymbol(circle_symbol)

        symbol = QgsLineSymbol.createSimple({})
        symbol.changeSymbolLayer(0, sl)
        self.lines_layer.setRenderer(QgsSingleSymbolRenderer(symbol))
        self.polys_layer.setRenderer(QgsNullSymbolRenderer())

        label_settings = self.polys_layer.labeling().settings()
        fmt = label_settings.format()
        # enable a mask
        fmt.mask().setEnabled(True)
        fmt.mask().setSize(4.0)
        # and mask other symbol layers underneath
        fmt.mask().setMaskedSymbolLayers(
            [QgsSymbolLayerReference(self.lines_layer.id(), sl.id())]
        )
        label_settings.setFormat(fmt)
        self.polys_layer.labeling().setSettings(label_settings)

        self.check_layout_export(
            "layout_export_markerline_masked", 0, [self.polys_layer, self.lines_layer]
        )


if __name__ == "__main__":
    unittest.main()
