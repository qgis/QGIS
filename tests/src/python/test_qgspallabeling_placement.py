# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsPalLabeling: base suite of render check tests

Class is meant to be inherited by classes that test different labeling outputs

See <qgis-src-dir>/tests/testdata/labeling/README.rst for description.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '2015-08-24'
__copyright__ = 'Copyright 2015, The QGIS Project'

import qgis  # NOQA

import os
import sys

from qgis.PyQt.QtCore import QThreadPool, qDebug

from qgis.core import (QgsLabelingEngineSettings,
                       QgsPalLayerSettings,
                       QgsSingleSymbolRenderer,
                       QgsMarkerSymbol,
                       QgsProperty,
                       QgsVectorLayerSimpleLabeling)
from utilities import getTempfilePath, renderMapToImage, mapSettingsString

from test_qgspallabeling_base import TestQgsPalLabeling, runSuite


# noinspection PyPep8Naming
class TestPlacementBase(TestQgsPalLabeling):

    @classmethod
    def setUpClass(cls):
        if not cls._BaseSetup:
            TestQgsPalLabeling.setUpClass()

    @classmethod
    def tearDownClass(cls):
        TestQgsPalLabeling.tearDownClass()

    def setUp(self):
        """Run before each test."""
        super(TestPlacementBase, self).setUp()
        self.removeAllLayers()
        self.configTest('pal_placement', 'sp')
        self._TestImage = ''

        self._Mismatch = 0
        self._ColorTol = 0
        self._Mismatches.clear()
        self._ColorTols.clear()

        # render only rectangles of the placed labels
        engine_settings = QgsLabelingEngineSettings()
        engine_settings.setFlag(QgsLabelingEngineSettings.DrawLabelRectOnly)
        self._MapSettings.setLabelingEngineSettings(engine_settings)

    def checkTest(self, **kwargs):
        if kwargs.get('apply_simple_labeling', True):
            self.layer.setLabeling(QgsVectorLayerSimpleLabeling(self.lyr))

        ms = self._MapSettings  # class settings
        settings_type = 'Class'
        if self._TestMapSettings is not None:
            ms = self._TestMapSettings  # per test settings
            settings_type = 'Test'
        if 'PAL_VERBOSE' in os.environ:
            qDebug('MapSettings type: {0}'.format(settings_type))
            qDebug(mapSettingsString(ms))

        img = renderMapToImage(ms, parallel=False)
        self._TestImage = getTempfilePath('png')
        if not img.save(self._TestImage, 'png'):
            os.unlink(self._TestImage)
            raise OSError('Failed to save output from map render job')
        self.saveControlImage(self._TestImage)

        mismatch = 0
        if 'PAL_NO_MISMATCH' not in os.environ:
            # some mismatch expected
            mismatch = self._Mismatch if self._Mismatch else 0
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

# noinspection PyPep8Naming


class TestPointPlacement(TestPlacementBase):

    @classmethod
    def setUpClass(cls):
        TestPlacementBase.setUpClass()
        cls.layer = None

    def test_point_placement_around(self):
        # Default point label placement
        self.layer = TestQgsPalLabeling.loadFeatureLayer('point')
        self._TestMapSettings = self.cloneMapSettings(self._MapSettings)
        self.checkTest()
        self.removeMapLayer(self.layer)
        self.layer = None

    def test_point_placement_around_obstacle(self):
        # Default point label placement with obstacle
        self.layer = TestQgsPalLabeling.loadFeatureLayer('point2')
        self._TestMapSettings = self.cloneMapSettings(self._MapSettings)
        self.checkTest()
        self.removeMapLayer(self.layer)
        self.layer = None

    def test_point_placement_narrow_polygon_obstacle(self):
        # Default point label placement with narrow polygon obstacle
        self.layer = TestQgsPalLabeling.loadFeatureLayer('point')
        polyLayer = TestQgsPalLabeling.loadFeatureLayer('narrow_polygon')
        self._TestMapSettings = self.cloneMapSettings(self._MapSettings)
        self.checkTest()
        self.removeMapLayer(self.layer)
        self.removeMapLayer(polyLayer)
        self.layer = None

    def test_point_placement_around_obstacle_large_symbol(self):
        # Default point label placement with obstacle and large symbols
        self.layer = TestQgsPalLabeling.loadFeatureLayer('point3')
        self._TestMapSettings = self.cloneMapSettings(self._MapSettings)
        self.checkTest()
        self.removeMapLayer(self.layer)
        self.layer = None

    def test_polygon_placement_with_hole(self):
        # Horizontal label placement for polygon with hole
        # Note for this test, the mask is used to check only pixels outside of the polygon.
        # We don't care where in the polygon the label is, just that it
        # is INSIDE the polygon
        self.layer = TestQgsPalLabeling.loadFeatureLayer('polygon_with_hole')
        self._TestMapSettings = self.cloneMapSettings(self._MapSettings)
        self.lyr.placement = QgsPalLayerSettings.Horizontal
        self.checkTest()
        self.removeMapLayer(self.layer)
        self.layer = None

    def test_polygon_placement_with_hole_and_point(self):
        # Testing that hole from a feature is not treated as an obstacle for other feature's labels
        self.layer = TestQgsPalLabeling.loadFeatureLayer('point')
        polyLayer = TestQgsPalLabeling.loadFeatureLayer('polygon_with_hole')
        self._TestMapSettings = self.cloneMapSettings(self._MapSettings)
        self.checkTest()
        self.removeMapLayer(self.layer)
        self.removeMapLayer(polyLayer)
        self.layer = None

    def test_polygon_multiple_labels(self):
        # Horizontal label placement for polygon with hole
        # Note for this test, the mask is used to check only pixels outside of the polygon.
        # We don't care where in the polygon the label is, just that it
        # is INSIDE the polygon
        self.layer = TestQgsPalLabeling.loadFeatureLayer('polygon_rule_based')
        self._TestMapSettings = self.cloneMapSettings(self._MapSettings)
        self.checkTest(apply_simple_labeling=False)
        self.removeMapLayer(self.layer)
        self.layer = None

    def test_multipolygon_obstacle(self):
        # Test that all parts of multipolygon are used as an obstacle
        self.layer = TestQgsPalLabeling.loadFeatureLayer('point')
        polyLayer = TestQgsPalLabeling.loadFeatureLayer('multi_polygon')
        self._TestMapSettings = self.cloneMapSettings(self._MapSettings)
        self.checkTest()
        self.removeMapLayer(self.layer)
        self.removeMapLayer(polyLayer)
        self.layer = None

    def test_point_ordered_placement1(self):
        # Test ordered placements for point
        self.layer = TestQgsPalLabeling.loadFeatureLayer('point_ordered_placement')
        self._TestMapSettings = self.cloneMapSettings(self._MapSettings)
        self.lyr.placement = QgsPalLayerSettings.OrderedPositionsAroundPoint
        self.lyr.dist = 2
        self.checkTest()
        self.removeMapLayer(self.layer)
        self.layer = None

    def test_point_ordered_placement2(self):
        # Test ordered placements for point (1 obstacle)
        self.layer = TestQgsPalLabeling.loadFeatureLayer('point_ordered_placement')
        obstacleLayer = TestQgsPalLabeling.loadFeatureLayer('point_ordered_obstacle1')
        self._TestMapSettings = self.cloneMapSettings(self._MapSettings)
        self.lyr.placement = QgsPalLayerSettings.OrderedPositionsAroundPoint
        self.lyr.dist = 2
        self.checkTest()
        self.removeMapLayer(obstacleLayer)
        self.removeMapLayer(self.layer)
        self.layer = None

    def test_point_ordered_placement3(self):
        # Test ordered placements for point (2 obstacle)
        self.layer = TestQgsPalLabeling.loadFeatureLayer('point_ordered_placement')
        obstacleLayer = TestQgsPalLabeling.loadFeatureLayer('point_ordered_obstacle2')
        self._TestMapSettings = self.cloneMapSettings(self._MapSettings)
        self.lyr.placement = QgsPalLayerSettings.OrderedPositionsAroundPoint
        self.lyr.dist = 2
        self.checkTest()
        self.removeMapLayer(obstacleLayer)
        self.removeMapLayer(self.layer)
        self.layer = None

    def test_point_ordered_placement4(self):
        # Test ordered placements for point (3 obstacle)
        self.layer = TestQgsPalLabeling.loadFeatureLayer('point_ordered_placement')
        obstacleLayer = TestQgsPalLabeling.loadFeatureLayer('point_ordered_obstacle3')
        self._TestMapSettings = self.cloneMapSettings(self._MapSettings)
        self.lyr.placement = QgsPalLayerSettings.OrderedPositionsAroundPoint
        self.lyr.dist = 2
        self.checkTest()
        self.removeMapLayer(obstacleLayer)
        self.removeMapLayer(self.layer)
        self.layer = None

    def test_point_dd_ordered_placement(self):
        # Test ordered placements for point with data defined order
        self.layer = TestQgsPalLabeling.loadFeatureLayer('point_ordered_placement')
        self._TestMapSettings = self.cloneMapSettings(self._MapSettings)
        self.lyr.placement = QgsPalLayerSettings.OrderedPositionsAroundPoint
        self.lyr.dist = 2
        self.lyr.dataDefinedProperties().setProperty(QgsPalLayerSettings.PredefinedPositionOrder, QgsProperty.fromExpression("'T,B'"))
        self.checkTest()
        self.removeMapLayer(self.layer)
        self.lyr.dataDefinedProperties().setProperty(QgsPalLayerSettings.PredefinedPositionOrder, QgsProperty())
        self.layer = None

    def test_point_dd_ordered_placement1(self):
        # Test ordered placements for point with data defined order and obstacle
        self.layer = TestQgsPalLabeling.loadFeatureLayer('point_ordered_placement')
        obstacleLayer = TestQgsPalLabeling.loadFeatureLayer('point_ordered_obstacle_top')
        self._TestMapSettings = self.cloneMapSettings(self._MapSettings)
        self.lyr.placement = QgsPalLayerSettings.OrderedPositionsAroundPoint
        self.lyr.dist = 2
        self.lyr.dataDefinedProperties().setProperty(QgsPalLayerSettings.PredefinedPositionOrder, QgsProperty.fromExpression("'T,B'"))
        self.checkTest()
        self.removeMapLayer(obstacleLayer)
        self.removeMapLayer(self.layer)
        self.lyr.dataDefinedProperties().setProperty(QgsPalLayerSettings.PredefinedPositionOrder, QgsProperty())
        self.layer = None

    def test_point_ordered_symbol_bound_offset(self):
        # Test ordered placements for point using symbol bounds offset
        self.layer = TestQgsPalLabeling.loadFeatureLayer('point_ordered_placement')
        # Make a big symbol
        symbol = QgsMarkerSymbol.createSimple({'color': '31,120,180,255',
                                               'outline_color': '0,0,0,0',
                                               'outline_style': 'solid',
                                               'size': '10',
                                               'name': 'rectangle',
                                               'size_unit': 'MM'})
        renderer = QgsSingleSymbolRenderer(symbol)
        self.layer.setRenderer(renderer)
        self._TestMapSettings = self.cloneMapSettings(self._MapSettings)
        self.lyr.placement = QgsPalLayerSettings.OrderedPositionsAroundPoint
        self.lyr.dist = 2
        self.lyr.offsetType = QgsPalLayerSettings.FromSymbolBounds
        self.checkTest()
        self.removeMapLayer(self.layer)
        self.layer = None

    def test_polygon_placement_perimeter(self):
        # Default polygon perimeter placement
        self.layer = TestQgsPalLabeling.loadFeatureLayer('polygon_perimeter')
        self._TestMapSettings = self.cloneMapSettings(self._MapSettings)
        self.lyr.placement = QgsPalLayerSettings.Line
        self.lyr.placementFlags = QgsPalLayerSettings.AboveLine
        self.checkTest()
        self.removeMapLayer(self.layer)
        self.layer = None

    def test_small_polygon_placement_perimeter(self):
        # Default polygon perimeter placement for small polygon
        self.layer = TestQgsPalLabeling.loadFeatureLayer('polygon_small')
        self._TestMapSettings = self.cloneMapSettings(self._MapSettings)
        self.lyr.placement = QgsPalLayerSettings.Line
        self.checkTest()
        self.removeMapLayer(self.layer)
        self.layer = None

    def test_small_polygon_perimeter_only_fit(self):
        # Polygon perimeter placement for small polygon when set to only show labels which fit in polygon
        self.layer = TestQgsPalLabeling.loadFeatureLayer('polygon_small')
        self._TestMapSettings = self.cloneMapSettings(self._MapSettings)
        self.lyr.placement = QgsPalLayerSettings.Line
        self.lyr.fitInPolygonOnly = True
        self.checkTest()
        self.removeMapLayer(self.layer)
        self.layer = None

    def test_small_polygon_curvedperimeter_only_fit(self):
        # Polygon perimeter placement for small polygon when set to only show labels which fit in polygon
        self.layer = TestQgsPalLabeling.loadFeatureLayer('polygon_small')
        self._TestMapSettings = self.cloneMapSettings(self._MapSettings)
        self.lyr.placement = QgsPalLayerSettings.PerimeterCurved
        self.lyr.fitInPolygonOnly = True
        self.checkTest()
        self.removeMapLayer(self.layer)
        self.layer = None

    def test_small_polygon_over_point_only_fit(self):
        # Polygon over point placement for small polygon when set to only show labels which fit in polygon
        self.layer = TestQgsPalLabeling.loadFeatureLayer('polygon_small')
        self._TestMapSettings = self.cloneMapSettings(self._MapSettings)
        self.lyr.placement = QgsPalLayerSettings.OverPoint
        self.lyr.fitInPolygonOnly = True
        self.checkTest()
        self.removeMapLayer(self.layer)
        self.layer = None

    def test_prefer_line_curved_above_instead_of_below(self):
        # Test that labeling a line using curved labels when both above and below placement are allowed that above
        # is preferred
        self.layer = TestQgsPalLabeling.loadFeatureLayer('line')
        self._TestMapSettings = self.cloneMapSettings(self._MapSettings)
        self.lyr.placement = QgsPalLayerSettings.Curved
        self.lyr.placementFlags = QgsPalLayerSettings.AboveLine | QgsPalLayerSettings.BelowLine | QgsPalLayerSettings.MapOrientation
        self.checkTest()
        self.removeMapLayer(self.layer)
        self.layer = None

    def test_prefer_line_curved_above_instead_of_online(self):
        # Test that labeling a line using curved labels when both above and online placement are allowed that above
        # is preferred
        self.layer = TestQgsPalLabeling.loadFeatureLayer('line')
        self._TestMapSettings = self.cloneMapSettings(self._MapSettings)
        self.lyr.placement = QgsPalLayerSettings.Curved
        self.lyr.placementFlags = QgsPalLayerSettings.AboveLine | QgsPalLayerSettings.OnLine | QgsPalLayerSettings.MapOrientation
        self.checkTest()
        self.removeMapLayer(self.layer)
        self.layer = None

    def test_prefer_line_curved_below_instead_of_online(self):
        # Test that labeling a line using curved labels when both below and online placement are allowed that below
        # is preferred
        self.layer = TestQgsPalLabeling.loadFeatureLayer('line')
        self._TestMapSettings = self.cloneMapSettings(self._MapSettings)
        self.lyr.placement = QgsPalLayerSettings.Curved
        self.lyr.placementFlags = QgsPalLayerSettings.BelowLine | QgsPalLayerSettings.OnLine | QgsPalLayerSettings.MapOrientation
        self.checkTest()
        self.removeMapLayer(self.layer)
        self.layer = None

    def test_prefer_line_above_instead_of_below(self):
        # Test that labeling a line using parallel labels when both above and below placement are allowed that above
        # is preferred
        self.layer = TestQgsPalLabeling.loadFeatureLayer('line')
        self._TestMapSettings = self.cloneMapSettings(self._MapSettings)
        self.lyr.placement = QgsPalLayerSettings.Line
        self.lyr.placementFlags = QgsPalLayerSettings.AboveLine | QgsPalLayerSettings.BelowLine | QgsPalLayerSettings.MapOrientation
        self.checkTest()
        self.removeMapLayer(self.layer)
        self.layer = None

    def test_prefer_line_above_instead_of_online(self):
        # Test that labeling a line using parallel labels when both above and online placement are allowed that above
        # is preferred
        self.layer = TestQgsPalLabeling.loadFeatureLayer('line')
        self._TestMapSettings = self.cloneMapSettings(self._MapSettings)
        self.lyr.placement = QgsPalLayerSettings.Line
        self.lyr.placementFlags = QgsPalLayerSettings.AboveLine | QgsPalLayerSettings.OnLine | QgsPalLayerSettings.MapOrientation
        self.checkTest()
        self.removeMapLayer(self.layer)
        self.layer = None

    def test_prefer_line_below_instead_of_online(self):
        # Test that labeling a line using parallel labels when both below and online placement are allowed that below
        # is preferred
        self.layer = TestQgsPalLabeling.loadFeatureLayer('line')
        self._TestMapSettings = self.cloneMapSettings(self._MapSettings)
        self.lyr.placement = QgsPalLayerSettings.Line
        self.lyr.placementFlags = QgsPalLayerSettings.BelowLine | QgsPalLayerSettings.OnLine | QgsPalLayerSettings.MapOrientation
        self.checkTest()
        self.removeMapLayer(self.layer)
        self.layer = None

    def test_prefer_longer_lines_over_shorter(self):
        # Test that labeling a line using parallel labels will tend to place the labels over the longer straight parts of
        # the line
        self.layer = TestQgsPalLabeling.loadFeatureLayer('line_placement_1')
        self._TestMapSettings = self.cloneMapSettings(self._MapSettings)
        self.lyr.placement = QgsPalLayerSettings.Line
        self.checkTest()
        self.removeMapLayer(self.layer)
        self.layer = None

    def test_prefer_more_horizontal_lines(self):
        # Test that labeling a line using parallel labels will tend to place the labels over more horizontal sections
        self.layer = TestQgsPalLabeling.loadFeatureLayer('line_placement_2')
        self._TestMapSettings = self.cloneMapSettings(self._MapSettings)
        self.lyr.placement = QgsPalLayerSettings.Line
        self.checkTest()
        self.removeMapLayer(self.layer)
        self.layer = None

    def test_label_line_over_small_angles(self):
        # Test that labeling a line using parallel labels will place labels near center of straightish line
        self.layer = TestQgsPalLabeling.loadFeatureLayer('line_placement_3')
        self._TestMapSettings = self.cloneMapSettings(self._MapSettings)
        self.lyr.placement = QgsPalLayerSettings.Line
        self.checkTest()
        self.removeMapLayer(self.layer)
        self.layer = None

    def test_label_line_toward_center(self):
        # Test that labeling a line using parallel labels will try to place labels as close to center of line as possible
        self.layer = TestQgsPalLabeling.loadFeatureLayer('line_placement_4')
        self._TestMapSettings = self.cloneMapSettings(self._MapSettings)
        self.lyr.placement = QgsPalLayerSettings.Line
        self.checkTest()
        self.removeMapLayer(self.layer)
        self.layer = None

    def test_label_line_avoid_jaggy(self):
        # Test that labeling a line using parallel labels won't place labels over jaggy bits of line
        self.layer = TestQgsPalLabeling.loadFeatureLayer('line_placement_5')
        self._TestMapSettings = self.cloneMapSettings(self._MapSettings)
        self.lyr.placement = QgsPalLayerSettings.Line
        self.checkTest()
        self.removeMapLayer(self.layer)
        self.layer = None

    def test_label_curved_zero_width_char(self):
        # Test that curved label work with zero-width characters
        self.layer = TestQgsPalLabeling.loadFeatureLayer('line')
        self._TestMapSettings = self.cloneMapSettings(self._MapSettings)
        self.lyr.placement = QgsPalLayerSettings.Curved
        self.lyr.placementFlags = QgsPalLayerSettings.OnLine
        self.lyr.fieldName = "'invisible​space'"
        self.lyr.isExpression = True
        self.checkTest()
        self.removeMapLayer(self.layer)
        self.layer = None


if __name__ == '__main__':
    # NOTE: unless PAL_SUITE env var is set all test class methods will be run
    # SEE: test_qgspallabeling_tests.suiteTests() to define suite
    suite = ('TestPointPlacement')
    res = runSuite(sys.modules[__name__], suite)
    sys.exit(not res.wasSuccessful())
