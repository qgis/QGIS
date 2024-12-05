"""QGIS Unit tests for QgsHeatmapRenderer

From build dir, run: ctest -R PyQgsHeatmapRenderer -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import os

from qgis.PyQt.QtCore import QSize
from qgis.PyQt.QtGui import QColor
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    Qgis,
    QgsHeatmapRenderer,
    QgsGradientColorRamp,
    QgsReadWriteContext,
    QgsColorRampLegendNodeSettings,
    QgsProperty,
    QgsFeatureRenderer,
    QgsVectorLayer,
    QgsMapSettings,
    QgsExpressionContext,
    QgsExpressionContextScope,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()

TEST_DATA_DIR = unitTestDataPath()


class TestQgsHeatmapRenderer(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "heatmap_renderer"

    def test_clone(self):
        """
        Test cloning renderer
        """

        renderer = QgsHeatmapRenderer()
        renderer.setColorRamp(
            QgsGradientColorRamp(QColor(255, 0, 0), QColor(255, 200, 100))
        )

        legend_settings = QgsColorRampLegendNodeSettings()
        legend_settings.setMaximumLabel("my max")
        legend_settings.setMinimumLabel("my min")
        renderer.setLegendSettings(legend_settings)

        renderer.setDataDefinedProperty(
            QgsFeatureRenderer.Property.HeatmapRadius,
            QgsProperty.fromField("radius_field"),
        )
        renderer.setDataDefinedProperty(
            QgsFeatureRenderer.Property.HeatmapMaximum,
            QgsProperty.fromField("max_field"),
        )

        renderer2 = renderer.clone()
        self.assertEqual(renderer2.colorRamp().color1(), QColor(255, 0, 0))
        self.assertEqual(renderer2.colorRamp().color2(), QColor(255, 200, 100))
        self.assertEqual(renderer2.legendSettings().minimumLabel(), "my min")
        self.assertEqual(renderer2.legendSettings().maximumLabel(), "my max")
        self.assertEqual(
            renderer2.dataDefinedProperties()
            .property(QgsFeatureRenderer.Property.HeatmapRadius)
            .field(),
            "radius_field",
        )
        self.assertEqual(
            renderer2.dataDefinedProperties()
            .property(QgsFeatureRenderer.Property.HeatmapMaximum)
            .field(),
            "max_field",
        )

    def test_write_read_xml(self):
        """
        Test writing renderer to xml and restoring
        """

        renderer = QgsHeatmapRenderer()
        renderer.setColorRamp(
            QgsGradientColorRamp(QColor(255, 0, 0), QColor(255, 200, 100))
        )

        legend_settings = QgsColorRampLegendNodeSettings()
        legend_settings.setMaximumLabel("my max")
        legend_settings.setMinimumLabel("my min")
        renderer.setLegendSettings(legend_settings)

        renderer.setDataDefinedProperty(
            QgsFeatureRenderer.Property.HeatmapRadius,
            QgsProperty.fromField("radius_field"),
        )
        renderer.setDataDefinedProperty(
            QgsFeatureRenderer.Property.HeatmapMaximum,
            QgsProperty.fromField("max_field"),
        )

        doc = QDomDocument("testdoc")
        elem = renderer.save(doc, QgsReadWriteContext())

        renderer2 = QgsFeatureRenderer.load(elem, QgsReadWriteContext())
        self.assertEqual(renderer2.colorRamp().color1(), QColor(255, 0, 0))
        self.assertEqual(renderer2.colorRamp().color2(), QColor(255, 200, 100))
        self.assertEqual(renderer2.legendSettings().minimumLabel(), "my min")
        self.assertEqual(renderer2.legendSettings().maximumLabel(), "my max")

        self.assertEqual(
            renderer2.dataDefinedProperties()
            .property(QgsFeatureRenderer.Property.HeatmapRadius)
            .field(),
            "radius_field",
        )
        self.assertEqual(
            renderer2.dataDefinedProperties()
            .property(QgsFeatureRenderer.Property.HeatmapMaximum)
            .field(),
            "max_field",
        )

    def test_render(self):
        """
        Test heatmap rendering
        """
        layer = QgsVectorLayer(
            os.path.join(TEST_DATA_DIR, "points.shp"), "Points", "ogr"
        )
        self.assertTrue(layer.isValid())

        renderer = QgsHeatmapRenderer()
        renderer.setColorRamp(
            QgsGradientColorRamp(QColor(255, 0, 0), QColor(255, 200, 100))
        )
        renderer.setRadius(20)
        renderer.setRadiusUnit(Qgis.RenderUnit.Millimeters)
        layer.setRenderer(renderer)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setExtent(layer.extent())
        mapsettings.setLayers([layer])

        self.assertTrue(
            self.render_map_settings_check(
                "Render heatmap", "render_heatmap", mapsettings
            )
        )

    def test_data_defined_radius(self):
        """
        Test heatmap rendering with data defined radius
        """
        layer = QgsVectorLayer(
            os.path.join(TEST_DATA_DIR, "points.shp"), "Points", "ogr"
        )
        self.assertTrue(layer.isValid())

        renderer = QgsHeatmapRenderer()
        renderer.setColorRamp(
            QgsGradientColorRamp(QColor(255, 0, 0), QColor(255, 200, 100))
        )
        renderer.setRadius(20)
        renderer.setRadiusUnit(Qgis.RenderUnit.Millimeters)
        renderer.setDataDefinedProperty(
            QgsFeatureRenderer.Property.HeatmapRadius,
            QgsProperty.fromExpression("@my_var * 2"),
        )

        layer.setRenderer(renderer)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setExtent(layer.extent())
        mapsettings.setLayers([layer])
        scope = QgsExpressionContextScope()
        scope.setVariable("my_var", 20)
        context = QgsExpressionContext()
        context.appendScope(scope)
        mapsettings.setExpressionContext(context)

        self.assertTrue(
            self.render_map_settings_check(
                "Render heatmap with data defined radius",
                "data_defined_radius",
                mapsettings,
            )
        )

    def test_data_defined_maximum(self):
        """
        Test heatmap rendering with data defined maximum value
        """
        layer = QgsVectorLayer(
            os.path.join(TEST_DATA_DIR, "points.shp"), "Points", "ogr"
        )
        self.assertTrue(layer.isValid())

        renderer = QgsHeatmapRenderer()
        renderer.setColorRamp(
            QgsGradientColorRamp(QColor(255, 0, 0), QColor(255, 200, 100))
        )
        renderer.setRadius(20)
        renderer.setRadiusUnit(Qgis.RenderUnit.Millimeters)
        renderer.setDataDefinedProperty(
            QgsFeatureRenderer.Property.HeatmapMaximum,
            QgsProperty.fromExpression("@my_var * 2"),
        )

        layer.setRenderer(renderer)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setExtent(layer.extent())
        mapsettings.setLayers([layer])
        scope = QgsExpressionContextScope()
        scope.setVariable("my_var", 0.5)
        context = QgsExpressionContext()
        context.appendScope(scope)
        mapsettings.setExpressionContext(context)

        self.assertTrue(
            self.render_map_settings_check(
                "Render heatmap with data defined maximum",
                "data_defined_maximum",
                mapsettings,
            )
        )


if __name__ == "__main__":
    unittest.main()
