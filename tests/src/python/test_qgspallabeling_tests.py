"""QGIS Unit tests for QgsPalLabeling: base suite of render check tests

Class is meant to be inherited by classes that test different labeling outputs

See <qgis-src-dir>/tests/testdata/labeling/README.rst for description.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Larry Shaffer"
__date__ = "07/16/2013"
__copyright__ = "Copyright 2013, The QGIS Project"

import os

from qgis.PyQt.QtCore import QPointF, QSizeF, Qt
from qgis.PyQt.QtGui import QFont
from qgis.core import (
    QgsCoordinateReferenceSystem,
    QgsExpressionContext,
    QgsExpressionContextUtils,
    QgsLabelingEngineSettings,
    QgsPalLayerSettings,
    QgsProject,
    QgsTextBackgroundSettings,
    QgsUnitTypes,
)

from utilities import svgSymbolsPath


# noinspection PyPep8Naming
class TestPointBase:

    def __init__(self):
        """Dummy assignments, intended to be overridden in subclasses"""
        self.lyr = QgsPalLayerSettings()
        """:type: QgsPalLayerSettings"""
        # noinspection PyArgumentList
        self._TestFont = QFont()  # will become a standard test font
        # custom mismatches per group/test (should not mask any needed anomaly)
        # e.g. self._Mismatches['TestClassName'] = 300
        # check base output class's checkTest() or subclasses for any defaults
        self._Mismatches = dict()
        # custom color tolerances per group/test: 1 - 20 (0 default, 20 max)
        # (should not mask any needed anomaly)
        # e.g. self._ColorTols['TestClassName'] = 10
        # check base output class's checkTest() or subclasses for any defaults
        self._ColorTols = dict()

    # noinspection PyMethodMayBeStatic
    def checkTest(self, **kwargs):
        """Intended to be overridden in subclasses"""
        pass

    def test_default_label(self):
        # Default label placement, with text size in points
        self._Mismatches["TestCanvasPoint"] = 776
        self._ColorTols["TestComposerPdfPoint"] = 2
        self.checkTest()

    def test_text_size_map_unit(self):
        # Label text size in map units
        format = self.lyr.format()

        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)
        format.setSize(460)
        font = QFont(self._TestFont)
        format.setFont(font)
        self.lyr.setFormat(format)
        self._Mismatches["TestCanvasPoint"] = 776
        self._ColorTols["TestComposerPdfPoint"] = 2
        self.checkTest()

    def test_text_color(self):
        self._Mismatches["TestCanvasPoint"] = 774
        self._ColorTols["TestComposerPdfPoint"] = 2
        # Label color change
        format = self.lyr.format()
        format.setColor(Qt.GlobalColor.blue)
        self.lyr.setFormat(format)
        self.checkTest()

    def test_background_rect(self):
        self._Mismatches["TestComposerImageVsCanvasPoint"] = 800
        self._Mismatches["TestComposerImagePoint"] = 800
        format = self.lyr.format()
        format.background().setEnabled(True)
        self.lyr.setFormat(format)
        self._Mismatches["TestCanvasPoint"] = 776
        self._ColorTols["TestComposerPdfPoint"] = 1
        self.checkTest()

    def test_background_rect_w_offset(self):
        # Label rectangular background
        self._Mismatches["TestComposerImageVsCanvasPoint"] = 800
        self._Mismatches["TestComposerImagePoint"] = 800
        # verify fix for issues
        #   https://github.com/qgis/QGIS/issues/17705
        #   http://gis.stackexchange.com/questions/86900

        format = self.lyr.format()
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)
        format.setSize(460)
        font = QFont(self._TestFont)
        format.setFont(font)

        format.background().setEnabled(True)
        format.background().setOffsetUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)
        format.background().setOffset(QPointF(-2900.0, -450.0))

        self.lyr.setFormat(format)

        self._Mismatches["TestCanvasPoint"] = 774
        self._ColorTols["TestComposerPdfPoint"] = 2
        self.checkTest()

    def test_background_svg(self):
        # Label SVG background
        format = self.lyr.format()
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)
        format.setSize(460)
        font = QFont(self._TestFont)
        format.setFont(font)

        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeSVG)
        svg = os.path.join(svgSymbolsPath(), "backgrounds", "background_square.svg")
        format.background().setSvgFile(svg)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeBuffer)
        format.background().setSize(QSizeF(100.0, 0.0))
        self.lyr.setFormat(format)

        self._Mismatches["TestComposerPdfVsComposerPoint"] = 580
        self._Mismatches["TestCanvasPoint"] = 776
        self._ColorTols["TestComposerPdfPoint"] = 2
        self.checkTest()

    def test_background_svg_w_offset(self):
        # Label SVG background
        format = self.lyr.format()
        format.setSizeUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)
        format.setSize(460)
        font = QFont(self._TestFont)
        format.setFont(font)

        format.background().setEnabled(True)
        format.background().setType(QgsTextBackgroundSettings.ShapeType.ShapeSVG)
        svg = os.path.join(svgSymbolsPath(), "backgrounds", "background_square.svg")
        format.background().setSvgFile(svg)
        format.background().setSizeUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)
        format.background().setSizeType(QgsTextBackgroundSettings.SizeType.SizeBuffer)
        format.background().setSize(QSizeF(100.0, 0.0))
        format.background().setOffsetUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)
        format.background().setOffset(QPointF(-2850.0, 500.0))

        self.lyr.setFormat(format)

        self._Mismatches["TestComposerPdfVsComposerPoint"] = 760
        self._Mismatches["TestCanvasPoint"] = 776
        self._ColorTols["TestComposerPdfPoint"] = 2
        self.checkTest()

    def test_partials_labels_enabled(self):
        # Set Big font size
        format = self.lyr.format()
        font = QFont(self._TestFont)
        format.setFont(font)
        format.setSize(84)
        self.lyr.setFormat(format)
        # Enable partials labels
        engine_settings = QgsLabelingEngineSettings()
        engine_settings.setFlag(
            QgsLabelingEngineSettings.Flag.UsePartialCandidates, True
        )
        self._TestMapSettings.setLabelingEngineSettings(engine_settings)
        self._Mismatches["TestCanvasPoint"] = 779
        self._ColorTols["TestComposerPdfPoint"] = 2
        self.checkTest()

    def test_partials_labels_disabled(self):
        # Set Big font size
        format = self.lyr.format()
        font = QFont(self._TestFont)
        format.setFont(font)
        format.setSize(84)
        self.lyr.setFormat(format)
        # Disable partials labels
        engine_settings = QgsLabelingEngineSettings()
        engine_settings.setFlag(
            QgsLabelingEngineSettings.Flag.UsePartialCandidates, False
        )
        self._TestMapSettings.setLabelingEngineSettings(engine_settings)
        self.checkTest()

    def test_buffer(self):
        # Label with buffer
        format = self.lyr.format()
        format.buffer().setEnabled(True)
        format.buffer().setSize(2)
        self.lyr.setFormat(format)
        self.checkTest()

    def test_shadow(self):
        # Label with shadow
        format = self.lyr.format()
        format.shadow().setEnabled(True)
        format.shadow().setOffsetDistance(2)
        format.shadow().setOpacity(1)
        self.lyr.setFormat(format)
        self.checkTest()

    def test_letter_spacing(self):
        # Modified letter spacing
        format = self.lyr.format()
        font = QFont(self._TestFont)
        font.setLetterSpacing(QFont.SpacingType.AbsoluteSpacing, 3.5)
        format.setFont(font)
        format.setSize(30)
        self.lyr.setFormat(format)
        self.checkTest()

    def test_word_spacing(self):
        # Modified word spacing
        format = self.lyr.format()
        font = QFont(self._TestFont)
        font.setWordSpacing(20.5)
        format.setFont(font)
        format.setSize(30)
        self.lyr.setFormat(format)
        self.checkTest()


# noinspection PyPep8Naming


class TestLineBase:

    def __init__(self):
        """Dummy assignments, intended to be overridden in subclasses"""
        self.lyr = QgsPalLayerSettings()
        """:type: QgsPalLayerSettings"""
        # noinspection PyArgumentList
        self._TestFont = QFont()  # will become a standard test font
        self._Pal = None
        """:type: QgsPalLabeling"""
        # custom mismatches per group/test (should not mask any needed anomaly)
        # e.g. self._Mismatches['TestClassName'] = 300
        # check base output class's checkTest() or subclasses for any defaults
        self._Mismatches = dict()
        # custom color tolerances per group/test: 1 - 20 (0 default, 20 max)
        # (should not mask any needed anomaly)
        # e.g. self._ColorTols['TestClassName'] = 10
        # check base output class's checkTest() or subclasses for any defaults
        self._ColorTols = dict()

    # noinspection PyMethodMayBeStatic
    def checkTest(self, **kwargs):
        """Intended to be overridden in subclasses"""
        pass

    def test_line_placement_above_line_orientation(self):
        # Line placement, above, follow line orientation
        self.lyr.placement = QgsPalLayerSettings.Placement.Line
        self.lyr.placementFlags = QgsPalLayerSettings.LinePlacementFlags.AboveLine
        self.checkTest()

    def test_line_placement_online(self):
        # Line placement, on line
        self.lyr.placement = QgsPalLayerSettings.Placement.Line
        self.lyr.placementFlags = QgsPalLayerSettings.LinePlacementFlags.OnLine
        self.checkTest()

    def test_line_placement_below_line_orientation(self):
        # Line placement, below, follow line orientation
        self.lyr.placement = QgsPalLayerSettings.Placement.Line
        self.lyr.placementFlags = QgsPalLayerSettings.LinePlacementFlags.BelowLine
        self.checkTest()

    def test_line_placement_above_map_orientation(self):
        # Line placement, above, follow map orientation
        self.lyr.placement = QgsPalLayerSettings.Placement.Line
        self.lyr.placementFlags = (
            QgsPalLayerSettings.LinePlacementFlags.AboveLine
            | QgsPalLayerSettings.LinePlacementFlags.MapOrientation
        )
        self.checkTest()

    def test_line_placement_below_map_orientation(self):
        # Line placement, below, follow map orientation
        self.lyr.placement = QgsPalLayerSettings.Placement.Line
        self.lyr.placementFlags = (
            QgsPalLayerSettings.LinePlacementFlags.BelowLine
            | QgsPalLayerSettings.LinePlacementFlags.MapOrientation
        )
        self.checkTest()

    def test_curved_placement_online(self):
        # Curved placement, on line
        self.lyr.placement = QgsPalLayerSettings.Placement.Curved
        self.lyr.placementFlags = QgsPalLayerSettings.LinePlacementFlags.OnLine
        self.checkTest()

    def test_curved_placement_above(self):
        # Curved placement, on line
        self.lyr.placement = QgsPalLayerSettings.Placement.Curved
        self.lyr.placementFlags = (
            QgsPalLayerSettings.LinePlacementFlags.AboveLine
            | QgsPalLayerSettings.LinePlacementFlags.MapOrientation
        )
        self.checkTest()

    def test_curved_placement_below(self):
        # Curved placement, on line
        self.lyr.placement = QgsPalLayerSettings.Placement.Curved
        self.lyr.placementFlags = (
            QgsPalLayerSettings.LinePlacementFlags.BelowLine
            | QgsPalLayerSettings.LinePlacementFlags.MapOrientation
        )
        self.checkTest()

    def test_curved_placement_online_html(self):
        # Curved placement, on line
        self.lyr.placement = QgsPalLayerSettings.Placement.Curved
        self.lyr.placementFlags = QgsPalLayerSettings.LinePlacementFlags.OnLine
        format = self.lyr.format()
        format.setAllowHtmlFormatting(True)
        self.lyr.setFormat(format)
        self.lyr.fieldName = '\'<span style="color: red">aaa</span><s>aa</s><span style="text-decoration: overline">a</span>\''
        self.lyr.isExpression = True
        self.checkTest()

    def test_length_expression(self):
        # compare length using the ellipsoid in kms and the planimetric distance in meters
        self.lyr.fieldName = "round($length,5) || ' - ' || round(length($geometry),2)"
        self.lyr.isExpression = True

        QgsProject.instance().setCrs(QgsCoordinateReferenceSystem("EPSG:32613"))
        QgsProject.instance().setEllipsoid("WGS84")
        QgsProject.instance().setDistanceUnits(
            QgsUnitTypes.DistanceUnit.DistanceKilometers
        )

        ctxt = QgsExpressionContext()
        ctxt.appendScope(QgsExpressionContextUtils.projectScope(QgsProject.instance()))
        ctxt.appendScope(QgsExpressionContextUtils.layerScope(self.layer))
        self._TestMapSettings.setExpressionContext(ctxt)

        self.lyr.placement = QgsPalLayerSettings.Placement.Curved
        self.lyr.placementFlags = (
            QgsPalLayerSettings.LinePlacementFlags.AboveLine
            | QgsPalLayerSettings.LinePlacementFlags.MapOrientation
        )
        self.checkTest()


# noinspection PyPep8Naming


def suiteTests():
    """
    Use to define which tests are run when PAL_SUITE is set.
    Use sp_vs_suite for comparison of server and layout outputs to canvas
    """
    sp_suite = [
        # 'test_default_label',
        # 'test_text_size_map_unit',
        # 'test_text_color',
        # 'test_background_rect',
        # 'test_background_rect_w_offset',
        # 'test_background_svg',
        # 'test_background_svg_w_offset',
        # 'test_partials_labels_enabled',
        # 'test_partials_labels_disabled',
    ]
    sp_vs_suite = [
        # 'test_something_specific',
    ]
    # extended separately for finer control of PAL_SUITE (comment-out undesired)
    sp_vs_suite.extend(sp_suite)

    return {"sp_suite": sp_suite, "sp_vs_suite": sp_vs_suite}


if __name__ == "__main__":
    pass
