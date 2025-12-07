"""QGIS Unit tests for QgsMapHitTest.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "(C) 2023 by Nyall Dawson"
__date__ = "08/03/2023"
__copyright__ = "Copyright 2023, The QGIS Project"

import os

from qgis.PyQt.QtCore import QSize
from qgis.core import (
    QgsCoordinateReferenceSystem,
    QgsMapHitTest,
    QgsMapSettings,
    QgsMarkerSymbol,
    QgsRectangle,
    QgsRuleBasedRenderer,
    QgsApplication,
    QgsVectorLayer,
    QgsMapHitTestTask,
    QgsLayerTreeFilterSettings,
    QgsRasterLayer,
    QgsSingleBandPseudoColorRenderer,
    QgsMeshLayer,
    Qgis,
)
import unittest
from qgis.testing import start_app, QgisTestCase
from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsMapHitTest(QgisTestCase):

    def test_hit_test(self):
        point_path = os.path.join(TEST_DATA_DIR, "points.shp")
        point_layer = QgsVectorLayer(point_path, "points", "ogr")

        root_rule = QgsRuleBasedRenderer.Rule(None)
        marker_symbol = QgsMarkerSymbol.createSimple(
            {"color": "#ff0000", "outline_style": "no", "size": "8"}
        )

        less_than_two_rule = QgsRuleBasedRenderer.Rule(
            marker_symbol, filterExp='"Importance" <=2', label="lessthantwo"
        )
        root_rule.appendChild(less_than_two_rule)

        else_rule = QgsRuleBasedRenderer.Rule(None, elseRule=True)

        marker_symbol = QgsMarkerSymbol.createSimple(
            {"color": "#00ffff", "outline_style": "no", "size": "4"}
        )
        one_rule = QgsRuleBasedRenderer.Rule(
            marker_symbol, filterExp='"Pilots" = 1', label="1"
        )
        else_rule.appendChild(one_rule)
        marker_symbol = QgsMarkerSymbol.createSimple(
            {"color": "#ff8888", "outline_style": "no", "size": "4"}
        )
        two_rule = QgsRuleBasedRenderer.Rule(
            marker_symbol, filterExp='"Pilots" = 2', label="2"
        )
        else_rule.appendChild(two_rule)
        marker_symbol = QgsMarkerSymbol.createSimple(
            {"color": "#8888ff", "outline_style": "no", "size": "4"}
        )
        three_rule = QgsRuleBasedRenderer.Rule(
            marker_symbol, filterExp='"Pilots" = 3', label="3"
        )
        else_rule.appendChild(three_rule)

        root_rule.appendChild(else_rule)

        renderer = QgsRuleBasedRenderer(root_rule)
        point_layer.setRenderer(renderer)

        map_settings = QgsMapSettings()
        map_settings.setOutputDpi(96)
        map_settings.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:3857"))
        map_settings.setOutputSize(QSize(1221, 750))
        map_settings.setExtent(QgsRectangle(-12360166, 3146940, -11269206, 3816372))
        map_settings.setLayers([point_layer])

        map_hit_test = QgsMapHitTest(map_settings)
        map_hit_test.run()
        self.assertEqual(list(map_hit_test.results().keys()), [point_layer.id()])
        self.assertCountEqual(
            map_hit_test.results()[point_layer.id()],
            [
                one_rule.ruleKey(),
                three_rule.ruleKey(),
                else_rule.ruleKey(),
                root_rule.ruleKey(),
            ],
        )

        map_settings.setExtent(QgsRectangle(-11226365, 4873483, -10573781, 5273920))

        map_hit_test = QgsMapHitTest(map_settings)
        map_hit_test.run()
        self.assertEqual(list(map_hit_test.results().keys()), [point_layer.id()])
        self.assertCountEqual(
            map_hit_test.results()[point_layer.id()],
            [two_rule.ruleKey(), else_rule.ruleKey(), root_rule.ruleKey()],
        )

    def test_hit_test_task(self):
        point_path = os.path.join(TEST_DATA_DIR, "points.shp")
        point_layer = QgsVectorLayer(point_path, "points", "ogr")

        root_rule = QgsRuleBasedRenderer.Rule(None)
        marker_symbol = QgsMarkerSymbol.createSimple(
            {"color": "#ff0000", "outline_style": "no", "size": "8"}
        )

        less_than_two_rule = QgsRuleBasedRenderer.Rule(
            marker_symbol, filterExp='"Importance" <=2', label="lessthantwo"
        )
        root_rule.appendChild(less_than_two_rule)

        else_rule = QgsRuleBasedRenderer.Rule(None, elseRule=True)

        marker_symbol = QgsMarkerSymbol.createSimple(
            {"color": "#00ffff", "outline_style": "no", "size": "4"}
        )
        one_rule = QgsRuleBasedRenderer.Rule(
            marker_symbol, filterExp='"Pilots" = 1', label="1"
        )
        else_rule.appendChild(one_rule)
        marker_symbol = QgsMarkerSymbol.createSimple(
            {"color": "#ff8888", "outline_style": "no", "size": "4"}
        )
        two_rule = QgsRuleBasedRenderer.Rule(
            marker_symbol, filterExp='"Pilots" = 2', label="2"
        )
        else_rule.appendChild(two_rule)
        marker_symbol = QgsMarkerSymbol.createSimple(
            {"color": "#8888ff", "outline_style": "no", "size": "4"}
        )
        three_rule = QgsRuleBasedRenderer.Rule(
            marker_symbol, filterExp='"Pilots" = 3', label="3"
        )
        else_rule.appendChild(three_rule)

        root_rule.appendChild(else_rule)

        renderer = QgsRuleBasedRenderer(root_rule)
        point_layer.setRenderer(renderer)

        map_settings = QgsMapSettings()
        map_settings.setOutputDpi(96)
        map_settings.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:3857"))
        map_settings.setOutputSize(QSize(1221, 750))
        map_settings.setExtent(QgsRectangle(-12360166, 3146940, -11269206, 3816372))
        map_settings.setLayers([point_layer])

        filter_settings = QgsLayerTreeFilterSettings(map_settings)
        map_hit_test_task = QgsMapHitTestTask(filter_settings)

        def catch_results():
            TestQgsMapHitTest.results = map_hit_test_task.results()

        map_hit_test_task.taskCompleted.connect(catch_results)
        QgsApplication.taskManager().addTask(map_hit_test_task)
        map_hit_test_task.waitForFinished()

        self.assertEqual(list(TestQgsMapHitTest.results.keys()), [point_layer.id()])
        self.assertCountEqual(
            TestQgsMapHitTest.results[point_layer.id()],
            [
                one_rule.ruleKey(),
                three_rule.ruleKey(),
                else_rule.ruleKey(),
                root_rule.ruleKey(),
            ],
        )

    def test_hittest_raster(self):

        # load the same for testing with different settings
        # dem3 should be ignored by MapHitTest as it doesn't have proper settings
        path_dem = os.path.join(TEST_DATA_DIR, "raster/dem.tif")
        dem_layer_1 = QgsRasterLayer(path_dem, "dem1", "gdal")
        dem_layer_2 = QgsRasterLayer(path_dem, "dem2", "gdal")
        dem_layer_3 = QgsRasterLayer(path_dem, "dem3", "gdal")

        # renderers
        renderer = dem_layer_1.renderer().clone()
        min_max_origin = renderer.minMaxOrigin()
        min_max_origin.setExtent(Qgis.RasterRangeExtent.UpdatedCanvas)
        renderer.setMinMaxOrigin(min_max_origin)
        dem_layer_1.setRenderer(renderer.clone())

        renderer_color = QgsSingleBandPseudoColorRenderer(dem_layer_2.dataProvider())
        renderer_color.setInputBand(1)
        renderer_color.setMinMaxOrigin(min_max_origin)
        dem_layer_2.setRenderer(renderer_color.clone())

        # map settings
        map_settings = QgsMapSettings()
        map_settings.setOutputDpi(96)
        map_settings.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:3857"))
        map_settings.setOutputSize(QSize(1221, 750))
        map_settings.setExtent(QgsRectangle(2079632, 5747751, 2081188, 5749009))
        map_settings.setLayers([dem_layer_1, dem_layer_2, dem_layer_3])

        # hit test
        map_hit_test = QgsMapHitTest(map_settings)
        map_hit_test.run()
        result = map_hit_test.resultsRenderersUpdatedCanvas()

        # results
        self.assertIsInstance(result, dict)
        self.assertEqual(len(result), 2)
        self.assertEqual(list(result.keys()), [dem_layer_1.id(), dem_layer_2.id()])

        # values for dem layer 1
        self.assertAlmostEqual(result[dem_layer_1.id()][0], 86.199, places=2)
        self.assertAlmostEqual(result[dem_layer_1.id()][1], 243.0, places=1)

    def test_hittest_raster_task(self):

        # load the same for testing with different settings
        # dem3 should be ignored by MapHitTest as it doesn't have proper settings
        path_dem = os.path.join(TEST_DATA_DIR, "raster/dem.tif")
        dem_layer_1 = QgsRasterLayer(path_dem, "dem1", "gdal")
        dem_layer_2 = QgsRasterLayer(path_dem, "dem2", "gdal")
        dem_layer_3 = QgsRasterLayer(path_dem, "dem3", "gdal")

        # renderers
        renderer = dem_layer_1.renderer().clone()
        min_max_origin = renderer.minMaxOrigin()
        min_max_origin.setExtent(Qgis.RasterRangeExtent.UpdatedCanvas)
        renderer.setMinMaxOrigin(min_max_origin)
        dem_layer_1.setRenderer(renderer.clone())

        renderer_color = QgsSingleBandPseudoColorRenderer(dem_layer_2.dataProvider())
        renderer_color.setInputBand(1)
        renderer_color.setMinMaxOrigin(min_max_origin)
        dem_layer_2.setRenderer(renderer_color.clone())

        # map settings
        map_settings = QgsMapSettings()
        map_settings.setOutputDpi(96)
        map_settings.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:3857"))
        map_settings.setOutputSize(QSize(1221, 750))
        map_settings.setExtent(QgsRectangle(2079632, 5747751, 2081188, 5749009))
        map_settings.setLayers([dem_layer_1, dem_layer_2, dem_layer_3])

        # hit test task
        filter_settings = QgsLayerTreeFilterSettings(map_settings)
        map_hit_test_task = QgsMapHitTestTask(filter_settings)

        TestQgsMapHitTest.resultsRenderersUpdatedCanvas = None

        def catch_results():
            TestQgsMapHitTest.resultsRenderersUpdatedCanvas = (
                map_hit_test_task.resultsRenderersUpdatedCanvas()
            )

        map_hit_test_task.taskCompleted.connect(catch_results)
        QgsApplication.taskManager().addTask(map_hit_test_task)
        map_hit_test_task.waitForFinished()

        # results
        self.assertIsInstance(TestQgsMapHitTest.resultsRenderersUpdatedCanvas, dict)
        self.assertEqual(len(TestQgsMapHitTest.resultsRenderersUpdatedCanvas), 2)
        self.assertEqual(
            list(TestQgsMapHitTest.resultsRenderersUpdatedCanvas.keys()),
            [dem_layer_1.id(), dem_layer_2.id()],
        )

        # values for dem layer 1
        self.assertAlmostEqual(
            TestQgsMapHitTest.resultsRenderersUpdatedCanvas[dem_layer_1.id()][0],
            86.199,
            places=2,
        )
        self.assertAlmostEqual(
            TestQgsMapHitTest.resultsRenderersUpdatedCanvas[dem_layer_1.id()][1],
            243.0,
            places=1,
        )

    def test_hittest_mesh(self):

        # load the same for testing with different settings, mesh3 does not have proper settings so it should be ignored by MapHitTest
        path_dem = os.path.join(TEST_DATA_DIR, "mesh/manzese_1d2d_small_map.nc")
        mesh_layer_1 = QgsMeshLayer(f'Ugrid:"{path_dem}":mesh2d', "mesh1", "mdal")
        mesh_layer_1.updateTriangularMesh()
        mesh_layer_2 = QgsMeshLayer(f'Ugrid:"{path_dem}":mesh2d', "mesh2", "mdal")
        mesh_layer_2.updateTriangularMesh()
        mesh_layer_3 = QgsMeshLayer(f'Ugrid:"{path_dem}":mesh2d', "mesh3", "mdal")
        mesh_layer_3.updateTriangularMesh()

        # renderer settings
        renderer_settings = mesh_layer_1.rendererSettings()

        # mesh layer 1 - group 0
        group_index = 0

        renderer_settings.setActiveScalarDatasetGroup(group_index)
        scalarRendererSettings = renderer_settings.scalarSettings(group_index)
        scalarRendererSettings.setLimits(Qgis.MeshRangeLimit.MinimumMaximum)
        scalarRendererSettings.setExtent(Qgis.MeshRangeExtent.UpdatedCanvas)
        renderer_settings.setScalarSettings(group_index, scalarRendererSettings)
        mesh_layer_1.setRendererSettings(renderer_settings)

        # mesh layer 2 - group 4
        group_index = 4

        renderer_settings.setActiveScalarDatasetGroup(group_index)
        scalarRendererSettings = renderer_settings.scalarSettings(group_index)
        scalarRendererSettings.setLimits(Qgis.MeshRangeLimit.MinimumMaximum)
        scalarRendererSettings.setExtent(Qgis.MeshRangeExtent.UpdatedCanvas)
        renderer_settings.setScalarSettings(group_index, scalarRendererSettings)
        mesh_layer_2.setRendererSettings(renderer_settings)

        # map settings
        map_settings = QgsMapSettings()
        map_settings.setOutputDpi(96)
        map_settings.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:3857"))
        map_settings.setOutputSize(QSize(1221, 750))
        map_settings.setExtent(QgsRectangle(525213, 9249013, 525679, 9249260))
        map_settings.setLayers([mesh_layer_1, mesh_layer_2, mesh_layer_3])

        # hit test
        map_hit_test = QgsMapHitTest(map_settings)
        map_hit_test.run()
        result = map_hit_test.resultsRenderersUpdatedCanvas()

        # results
        self.assertIsInstance(result, dict)
        self.assertEqual(len(result), 2)
        self.assertEqual(list(result.keys()), [mesh_layer_1.id(), mesh_layer_2.id()])

        # values for mesh layer 1
        self.assertAlmostEqual(result[mesh_layer_1.id()][0], 30.437649, places=5)
        self.assertAlmostEqual(result[mesh_layer_1.id()][1], 37.86449, places=4)

        # values for mesh layer 2
        self.assertAlmostEqual(result[mesh_layer_2.id()][0], 523.899, places=2)
        self.assertAlmostEqual(result[mesh_layer_2.id()][1], 625.0, places=1)

    def test_hittest_mesh_task(self):

        # load the same for testing with different settings, mesh3 does not have proper settings so it should be ignored by MapHitTest
        path_dem = os.path.join(TEST_DATA_DIR, "mesh/manzese_1d2d_small_map.nc")
        mesh_layer_1 = QgsMeshLayer(f'Ugrid:"{path_dem}":mesh2d', "mesh1", "mdal")
        mesh_layer_1.updateTriangularMesh()
        mesh_layer_2 = QgsMeshLayer(f'Ugrid:"{path_dem}":mesh2d', "mesh2", "mdal")
        mesh_layer_2.updateTriangularMesh()
        mesh_layer_3 = QgsMeshLayer(f'Ugrid:"{path_dem}":mesh2d', "mesh3", "mdal")
        mesh_layer_3.updateTriangularMesh()

        # renderer settings
        renderer_settings = mesh_layer_1.rendererSettings()

        # mesh layer 1 - group 0
        group_index = 0

        renderer_settings.setActiveScalarDatasetGroup(group_index)
        scalarRendererSettings = renderer_settings.scalarSettings(group_index)
        scalarRendererSettings.setLimits(Qgis.MeshRangeLimit.MinimumMaximum)
        scalarRendererSettings.setExtent(Qgis.MeshRangeExtent.UpdatedCanvas)
        renderer_settings.setScalarSettings(group_index, scalarRendererSettings)
        mesh_layer_1.setRendererSettings(renderer_settings)

        # mesh layer 2 - group 4
        group_index = 4

        renderer_settings.setActiveScalarDatasetGroup(group_index)
        scalarRendererSettings = renderer_settings.scalarSettings(group_index)
        scalarRendererSettings.setLimits(Qgis.MeshRangeLimit.MinimumMaximum)
        scalarRendererSettings.setExtent(Qgis.MeshRangeExtent.UpdatedCanvas)
        renderer_settings.setScalarSettings(group_index, scalarRendererSettings)
        mesh_layer_2.setRendererSettings(renderer_settings)

        # map settings
        map_settings = QgsMapSettings()
        map_settings.setOutputDpi(96)
        map_settings.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:3857"))
        map_settings.setOutputSize(QSize(1221, 750))
        map_settings.setExtent(QgsRectangle(525213, 9249013, 525679, 9249260))
        map_settings.setLayers([mesh_layer_1, mesh_layer_2, mesh_layer_3])

        # hit test task
        filter_settings = QgsLayerTreeFilterSettings(map_settings)
        map_hit_test_task = QgsMapHitTestTask(filter_settings)

        TestQgsMapHitTest.resultsRenderersUpdatedCanvas = None

        def catch_results():
            TestQgsMapHitTest.resultsRenderersUpdatedCanvas = (
                map_hit_test_task.resultsRenderersUpdatedCanvas()
            )

        map_hit_test_task.taskCompleted.connect(catch_results)
        QgsApplication.taskManager().addTask(map_hit_test_task)
        map_hit_test_task.waitForFinished()

        # results
        self.assertIsInstance(TestQgsMapHitTest.resultsRenderersUpdatedCanvas, dict)
        self.assertEqual(len(TestQgsMapHitTest.resultsRenderersUpdatedCanvas), 2)
        self.assertEqual(
            list(TestQgsMapHitTest.resultsRenderersUpdatedCanvas.keys()),
            [mesh_layer_1.id(), mesh_layer_2.id()],
        )

        # values for mesh layer 1
        self.assertAlmostEqual(
            TestQgsMapHitTest.resultsRenderersUpdatedCanvas[mesh_layer_1.id()][0],
            30.437649,
            places=5,
        )
        self.assertAlmostEqual(
            TestQgsMapHitTest.resultsRenderersUpdatedCanvas[mesh_layer_1.id()][1],
            37.86449,
            places=4,
        )
        # values for mesh layer 2
        self.assertAlmostEqual(
            TestQgsMapHitTest.resultsRenderersUpdatedCanvas[mesh_layer_2.id()][0],
            523.899,
            places=2,
        )
        self.assertAlmostEqual(
            TestQgsMapHitTest.resultsRenderersUpdatedCanvas[mesh_layer_2.id()][1],
            625.0,
            places=1,
        )


if __name__ == "__main__":
    unittest.main()
