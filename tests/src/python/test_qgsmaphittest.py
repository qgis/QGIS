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


if __name__ == "__main__":
    unittest.main()
