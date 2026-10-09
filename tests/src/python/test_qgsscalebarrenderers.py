"""QGIS Unit tests for scale bar renderers

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import math
import unittest

from qgis.core import (
    Qgis,
    QgsFontUtils,
    QgsLineSymbol,
    QgsRenderContext,
    QgsScaleBarRenderer,
    QgsScaleBarSettings,
    QgsSingleBoxScaleBarRenderer,
    QgsTextFormat,
)
from qgis.PyQt.QtGui import QColor, QImage, QPainter
from qgis.testing import QgisTestCase, start_app

start_app()


class TestQgsScaleBarRenderers(QgisTestCase):
    @classmethod
    def control_path_prefix(cls):
        return "scalebars"

    def _render_scale_bar(
        self,
        renderer: QgsScaleBarRenderer,
        settings: QgsScaleBarSettings,
        scale_context: QgsScaleBarRenderer.ScaleBarContext | None = None,
    ):
        if scale_context is None:
            scale_context = QgsScaleBarRenderer.ScaleBarContext()
            scale_context.scale = 500
            scale_context.flags = renderer.flags()
            scale_context.segmentWidth = 40

        dpi = 96
        dpmm = dpi / 25.4
        render_context = QgsRenderContext()
        render_context.setScaleFactor(dpmm)

        box_size_mm = renderer.calculateBoxSize(render_context, settings, scale_context)
        scale_context.size = box_size_mm

        image_width = int(math.ceil(box_size_mm.width() * dpmm))
        image_height = int(math.ceil(box_size_mm.height() * dpmm))

        image = QImage(image_width, image_height, QImage.Format.Format_ARGB32)
        image.fill(QColor(255, 255, 255))

        painter = QPainter(image)
        render_context.setPainter(painter)

        renderer.draw(render_context, settings, scale_context)
        painter.end()

        return image

    def test_context(self):
        context = QgsScaleBarRenderer.ScaleBarContext()
        context.segmentWidth = 5
        self.assertTrue(context.isValid())
        context.segmentWidth = math.nan
        self.assertFalse(context.isValid())

    @staticmethod
    def get_default_settings() -> QgsScaleBarSettings:
        settings = QgsScaleBarSettings()
        settings.setUnitsPerSegment(100)
        settings.setNumberOfSegments(2)
        line_symbol = QgsLineSymbol.createSimple(
            {"outline_width": 1, "joinstyle": "miter"}
        )
        settings.setLineSymbol(line_symbol)
        settings.setUnitLabel("m")

        format = QgsTextFormat()
        format.setFont(QgsFontUtils.getStandardTestFont("Bold"))
        format.setSize(20)
        format.setNamedStyle("Bold")
        format.setColor(QColor(0, 0, 0))
        settings.setTextFormat(format)

        return settings

    def test_single_box_renderer(self):
        renderer = QgsSingleBoxScaleBarRenderer()
        settings = self.get_default_settings()
        image = self._render_scale_bar(renderer, settings)

        self.assertTrue(self.image_check("singlebox", "singlebox", image))

    def test_units_before_first(self):
        renderer = QgsSingleBoxScaleBarRenderer()
        settings = self.get_default_settings()
        settings.setUnitLabel("Mtrs")

        settings.setUnitLabelPlacements(
            Qgis.ScaleBarUnitLabelPlacement.BeforeFirstDistanceLabel
        )
        image = self._render_scale_bar(renderer, settings)

        self.assertTrue(
            self.image_check("units_before_first", "units_before_first", image)
        )

    def test_units_before_first_above_centered(self):
        renderer = QgsSingleBoxScaleBarRenderer()
        settings = self.get_default_settings()
        settings.setUnitLabel("Mtrs")
        settings.setLabelHorizontalPlacement(
            Qgis.ScaleBarDistanceLabelHorizontalPlacement.CenteredSegment
        )

        settings.setUnitLabelPlacements(
            Qgis.ScaleBarUnitLabelPlacement.BeforeFirstDistanceLabel
        )
        image = self._render_scale_bar(renderer, settings)

        self.assertTrue(
            self.image_check(
                "units_before_first_above_centered",
                "units_before_first_above_centered",
                image,
            )
        )

    def test_units_before_first_below(self):
        renderer = QgsSingleBoxScaleBarRenderer()
        settings = self.get_default_settings()
        settings.setUnitLabel("Mtrs")
        settings.setLabelVerticalPlacement(
            Qgis.ScaleBarDistanceLabelVerticalPlacement.BelowSegment
        )

        settings.setUnitLabelPlacements(
            Qgis.ScaleBarUnitLabelPlacement.BeforeFirstDistanceLabel
        )
        image = self._render_scale_bar(renderer, settings)

        self.assertTrue(
            self.image_check(
                "units_before_first_below",
                "units_before_first_below",
                image,
            )
        )

    def test_units_after_last(self):
        renderer = QgsSingleBoxScaleBarRenderer()
        settings = self.get_default_settings()
        settings.setUnitLabel("Mtrs")

        settings.setUnitLabelPlacements(
            Qgis.ScaleBarUnitLabelPlacement.AfterLastDistanceLabel
        )
        image = self._render_scale_bar(renderer, settings)

        self.assertTrue(self.image_check("units_after_last", "units_after_last", image))

    def test_units_after_last_above_centered(self):
        renderer = QgsSingleBoxScaleBarRenderer()
        settings = self.get_default_settings()
        settings.setUnitLabel("Mtrs")
        settings.setLabelHorizontalPlacement(
            Qgis.ScaleBarDistanceLabelHorizontalPlacement.CenteredSegment
        )

        settings.setUnitLabelPlacements(
            Qgis.ScaleBarUnitLabelPlacement.AfterLastDistanceLabel
        )
        image = self._render_scale_bar(renderer, settings)

        self.assertTrue(
            self.image_check(
                "units_after_last_above_centered",
                "units_after_last_above_centered",
                image,
            )
        )

    def test_units_before_distance(self):
        renderer = QgsSingleBoxScaleBarRenderer()
        settings = self.get_default_settings()
        settings.setUnitLabel("Mtrs")

        settings.setUnitLabelPlacements(
            Qgis.ScaleBarUnitLabelPlacement.BeforeEveryDistanceLabel
        )
        image = self._render_scale_bar(renderer, settings)

        self.assertTrue(
            self.image_check("units_before_every", "units_before_every", image)
        )

    def test_units_before_distance_above_centered(self):
        renderer = QgsSingleBoxScaleBarRenderer()
        settings = self.get_default_settings()
        settings.setUnitLabel("Mtrs")
        settings.setLabelHorizontalPlacement(
            Qgis.ScaleBarDistanceLabelHorizontalPlacement.CenteredSegment
        )

        settings.setUnitLabelPlacements(
            Qgis.ScaleBarUnitLabelPlacement.BeforeEveryDistanceLabel
        )
        image = self._render_scale_bar(renderer, settings)

        self.assertTrue(
            self.image_check(
                "units_before_every_above_centered",
                "units_before_every_above_centered",
                image,
            )
        )

    def test_units_after_distance(self):
        renderer = QgsSingleBoxScaleBarRenderer()
        settings = self.get_default_settings()
        settings.setUnitLabel("Mtrs")

        settings.setUnitLabelPlacements(
            Qgis.ScaleBarUnitLabelPlacement.AfterEveryDistanceLabel
        )
        image = self._render_scale_bar(renderer, settings)

        self.assertTrue(
            self.image_check("units_after_every", "units_after_every", image)
        )

    def test_units_after_distance_above_centered(self):
        renderer = QgsSingleBoxScaleBarRenderer()
        settings = self.get_default_settings()
        settings.setUnitLabel("Mtrs")
        settings.setLabelHorizontalPlacement(
            Qgis.ScaleBarDistanceLabelHorizontalPlacement.CenteredSegment
        )

        settings.setUnitLabelPlacements(
            Qgis.ScaleBarUnitLabelPlacement.AfterEveryDistanceLabel
        )
        image = self._render_scale_bar(renderer, settings)

        self.assertTrue(
            self.image_check(
                "units_after_every_above_centered",
                "units_after_every_above_centered",
                image,
            )
        )

    def test_units_before_and_after_distance(self):
        renderer = QgsSingleBoxScaleBarRenderer()
        settings = self.get_default_settings()
        settings.setUnitLabel("m")

        settings.setUnitLabelPlacements(
            Qgis.ScaleBarUnitLabelPlacement.BeforeEveryDistanceLabel
            | Qgis.ScaleBarUnitLabelPlacement.AfterEveryDistanceLabel
        )
        image = self._render_scale_bar(renderer, settings)

        self.assertTrue(
            self.image_check(
                "units_before_after_every", "units_before_after_every", image
            )
        )

    def test_units_before_bar(self):
        renderer = QgsSingleBoxScaleBarRenderer()
        settings = self.get_default_settings()
        settings.setUnitLabel("Mtrs")

        settings.setUnitLabelPlacements(Qgis.ScaleBarUnitLabelPlacement.BeforeBar)
        image = self._render_scale_bar(renderer, settings)

        self.assertTrue(self.image_check("units_before_bar", "units_before_bar", image))

    def test_units_before_bar_above_centered(self):
        renderer = QgsSingleBoxScaleBarRenderer()
        settings = self.get_default_settings()
        settings.setUnitLabel("Mtrs")
        settings.setLabelHorizontalPlacement(
            Qgis.ScaleBarDistanceLabelHorizontalPlacement.CenteredSegment
        )

        settings.setUnitLabelPlacements(Qgis.ScaleBarUnitLabelPlacement.BeforeBar)
        image = self._render_scale_bar(renderer, settings)

        self.assertTrue(
            self.image_check(
                "units_before_bar_above_centered",
                "units_before_bar_above_centered",
                image,
            )
        )

    def test_units_after_bar(self):
        renderer = QgsSingleBoxScaleBarRenderer()
        settings = self.get_default_settings()
        settings.setUnitLabel("Mtrs")

        settings.setUnitLabelPlacements(Qgis.ScaleBarUnitLabelPlacement.AfterBar)
        image = self._render_scale_bar(renderer, settings)

        self.assertTrue(self.image_check("units_after_bar", "units_after_bar", image))

    def test_units_after_bar_above_centered(self):
        renderer = QgsSingleBoxScaleBarRenderer()
        settings = self.get_default_settings()
        settings.setUnitLabel("Mtrs")
        settings.setLabelHorizontalPlacement(
            Qgis.ScaleBarDistanceLabelHorizontalPlacement.CenteredSegment
        )

        settings.setUnitLabelPlacements(Qgis.ScaleBarUnitLabelPlacement.AfterBar)
        image = self._render_scale_bar(renderer, settings)

        self.assertTrue(
            self.image_check(
                "units_after_bar_above_centered",
                "units_after_bar_above_centered",
                image,
            )
        )

    def test_units_below_bar_left(self):
        renderer = QgsSingleBoxScaleBarRenderer()
        settings = self.get_default_settings()
        settings.setUnitLabel("Mtrs")

        settings.setUnitLabelPlacements(Qgis.ScaleBarUnitLabelPlacement.LeftBelow)
        image = self._render_scale_bar(renderer, settings)

        self.assertTrue(self.image_check("units_below_left", "units_below_left", image))

    def test_units_below_bar_left_above_centered(self):
        renderer = QgsSingleBoxScaleBarRenderer()
        settings = self.get_default_settings()
        settings.setUnitLabel("Mtrs")
        settings.setLabelHorizontalPlacement(
            Qgis.ScaleBarDistanceLabelHorizontalPlacement.CenteredSegment
        )

        settings.setUnitLabelPlacements(Qgis.ScaleBarUnitLabelPlacement.LeftBelow)
        image = self._render_scale_bar(renderer, settings)

        self.assertTrue(
            self.image_check(
                "units_below_left_above_centered",
                "units_below_left_above_centered",
                image,
            )
        )

    def test_units_below_bar_center(self):
        renderer = QgsSingleBoxScaleBarRenderer()
        settings = self.get_default_settings()
        settings.setUnitLabel("Mtrs")

        settings.setUnitLabelPlacements(Qgis.ScaleBarUnitLabelPlacement.CenteredBelow)
        image = self._render_scale_bar(renderer, settings)

        self.assertTrue(
            self.image_check("units_below_centered", "units_below_centered", image)
        )

    def test_units_below_bar_center_above_centered(self):
        renderer = QgsSingleBoxScaleBarRenderer()
        settings = self.get_default_settings()
        settings.setUnitLabel("Mtrs")
        settings.setLabelHorizontalPlacement(
            Qgis.ScaleBarDistanceLabelHorizontalPlacement.CenteredSegment
        )

        settings.setUnitLabelPlacements(Qgis.ScaleBarUnitLabelPlacement.CenteredBelow)
        image = self._render_scale_bar(renderer, settings)

        self.assertTrue(
            self.image_check(
                "units_below_centered_above_centered",
                "units_below_centered_above_centered",
                image,
            )
        )

    def test_units_below_bar_center_long_string(self):
        renderer = QgsSingleBoxScaleBarRenderer()
        settings = self.get_default_settings()
        settings.setUnitLabel("This is a very long units string")

        settings.setUnitLabelPlacements(Qgis.ScaleBarUnitLabelPlacement.CenteredBelow)
        image = self._render_scale_bar(renderer, settings)

        self.assertTrue(
            self.image_check(
                "units_below_centered_long_string",
                "units_below_centered_long_string",
                image,
            )
        )

    def test_units_below_bar_right(self):
        renderer = QgsSingleBoxScaleBarRenderer()
        settings = self.get_default_settings()
        settings.setUnitLabel("Mtrs")

        settings.setUnitLabelPlacements(Qgis.ScaleBarUnitLabelPlacement.RightBelow)
        image = self._render_scale_bar(renderer, settings)

        self.assertTrue(
            self.image_check("units_below_right", "units_below_right", image)
        )

    def test_units_below_bar_right_above_centered(self):
        renderer = QgsSingleBoxScaleBarRenderer()
        settings = self.get_default_settings()
        settings.setUnitLabel("Mtrs")
        settings.setLabelHorizontalPlacement(
            Qgis.ScaleBarDistanceLabelHorizontalPlacement.CenteredSegment
        )

        settings.setUnitLabelPlacements(Qgis.ScaleBarUnitLabelPlacement.RightBelow)
        image = self._render_scale_bar(renderer, settings)

        self.assertTrue(
            self.image_check(
                "units_below_right_above_centered",
                "units_below_right_above_centered",
                image,
            )
        )

    def test_units_above_bar_left(self):
        renderer = QgsSingleBoxScaleBarRenderer()
        settings = self.get_default_settings()
        settings.setUnitLabel("Mtrs")

        settings.setUnitLabelPlacements(Qgis.ScaleBarUnitLabelPlacement.LeftAbove)
        image = self._render_scale_bar(renderer, settings)

        self.assertTrue(self.image_check("units_above_left", "units_above_left", image))

    def test_units_above_bar_left_above_centered(self):
        renderer = QgsSingleBoxScaleBarRenderer()
        settings = self.get_default_settings()
        settings.setUnitLabel("Mtrs")
        settings.setLabelHorizontalPlacement(
            Qgis.ScaleBarDistanceLabelHorizontalPlacement.CenteredSegment
        )

        settings.setUnitLabelPlacements(Qgis.ScaleBarUnitLabelPlacement.LeftAbove)
        image = self._render_scale_bar(renderer, settings)

        self.assertTrue(
            self.image_check(
                "units_above_left_above_centered",
                "units_above_left_above_centered",
                image,
            )
        )

    def test_units_above_bar_center(self):
        renderer = QgsSingleBoxScaleBarRenderer()
        settings = self.get_default_settings()
        settings.setUnitLabel("Mtrs")

        settings.setUnitLabelPlacements(Qgis.ScaleBarUnitLabelPlacement.CenteredAbove)
        image = self._render_scale_bar(renderer, settings)

        self.assertTrue(
            self.image_check("units_above_centered", "units_above_centered", image)
        )

    def test_units_above_bar_center_above_centered(self):
        renderer = QgsSingleBoxScaleBarRenderer()
        settings = self.get_default_settings()
        settings.setUnitLabel("Mtrs")
        settings.setLabelHorizontalPlacement(
            Qgis.ScaleBarDistanceLabelHorizontalPlacement.CenteredSegment
        )

        settings.setUnitLabelPlacements(Qgis.ScaleBarUnitLabelPlacement.CenteredAbove)
        image = self._render_scale_bar(renderer, settings)

        self.assertTrue(
            self.image_check(
                "units_above_centered_above_centered",
                "units_above_centered_above_centered",
                image,
            )
        )

    def test_units_above_bar_center_long_string(self):
        renderer = QgsSingleBoxScaleBarRenderer()
        settings = self.get_default_settings()
        settings.setUnitLabel("This is a very long units string")

        settings.setUnitLabelPlacements(Qgis.ScaleBarUnitLabelPlacement.CenteredAbove)
        image = self._render_scale_bar(renderer, settings)

        self.assertTrue(
            self.image_check(
                "units_above_centered_long_string",
                "units_above_centered_long_string",
                image,
            )
        )

    def test_units_above_bar_right(self):
        renderer = QgsSingleBoxScaleBarRenderer()
        settings = self.get_default_settings()
        settings.setUnitLabel("Mtrs")

        settings.setUnitLabelPlacements(Qgis.ScaleBarUnitLabelPlacement.RightAbove)
        image = self._render_scale_bar(renderer, settings)

        self.assertTrue(
            self.image_check("units_above_right", "units_above_right", image)
        )

    def test_units_above_bar_right_above_centered(self):
        renderer = QgsSingleBoxScaleBarRenderer()
        settings = self.get_default_settings()
        settings.setUnitLabel("Mtrs")
        settings.setLabelHorizontalPlacement(
            Qgis.ScaleBarDistanceLabelHorizontalPlacement.CenteredSegment
        )

        settings.setUnitLabelPlacements(Qgis.ScaleBarUnitLabelPlacement.RightAbove)
        image = self._render_scale_bar(renderer, settings)

        self.assertTrue(
            self.image_check(
                "units_above_right_above_centered",
                "units_above_right_above_centered",
                image,
            )
        )

    def test_units_on_bar(self):
        renderer = QgsSingleBoxScaleBarRenderer()
        settings = self.get_default_settings()
        settings.setUnitLabel("Mtrs")

        format = settings.textFormat()
        format.setColor(QColor(255, 0, 0))
        settings.setTextFormat(format)

        settings.setUnitLabelPlacements(
            Qgis.ScaleBarUnitLabelPlacement.OnBarAfterFirstDivision
        )
        image = self._render_scale_bar(renderer, settings)

        self.assertTrue(
            self.image_check(
                "units_on_bar_after_first",
                "units_on_bar_after_first",
                image,
            )
        )


if __name__ == "__main__":
    unittest.main()
