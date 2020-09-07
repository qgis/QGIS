# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsMapBoxGlStyleConverter.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2020 by Nyall Dawson'
__date__ = '29/07/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import qgis  # NOQA

from qgis.PyQt.QtGui import (QColor)
from qgis.core import (QgsMapBoxGlStyleConverter,
                       QgsMapBoxGlStyleConversionContext,
                       QgsEffectStack
                       )

from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsMapBoxGlStyleConverter(unittest.TestCase):
    maxDiff = 100000

    def testNoLayer(self):
        c = QgsMapBoxGlStyleConverter()
        self.assertEqual(c.convert({'x': 'y'}), QgsMapBoxGlStyleConverter.NoLayerList)
        self.assertEqual(c.errorMessage(), 'Could not find layers list in JSON')
        self.assertIsNone(c.renderer())
        self.assertIsNone(c.labeling())

    def testInterpolateExpression(self):
        self.assertEqual(QgsMapBoxGlStyleConverter.interpolateExpression(5, 13, 27, 29, 1),
                         '27 + 2 * (1^(@zoom_level-5)-1)/(1^(13-5)-1)')
        self.assertEqual(QgsMapBoxGlStyleConverter.interpolateExpression(5, 13, 27, 29, 1.5),
                         '27 + 2 * (1.5^(@zoom_level-5)-1)/(1.5^(13-5)-1)')

    def testColorAsHslaComponents(self):
        self.assertEqual(QgsMapBoxGlStyleConverter.colorAsHslaComponents(QColor.fromHsl(30, 50, 70)), (30, 19, 27, 255))

    def testParseInterpolateColorByZoom(self):
        conversion_context = QgsMapBoxGlStyleConversionContext()
        props, default_col = QgsMapBoxGlStyleConverter.parseInterpolateColorByZoom({}, conversion_context)
        self.assertEqual(props.isActive(),
                         False)
        props, default_col = QgsMapBoxGlStyleConverter.parseInterpolateColorByZoom({'base': 1,
                                                                                    'stops': [[0, '#f1f075'],
                                                                                              [150, '#b52e3e'],
                                                                                              [250, '#e55e5e']]
                                                                                    },
                                                                                   conversion_context)
        self.assertEqual(props.expressionString(),
                         'CASE WHEN @zoom_level >= 0 AND @zoom_level < 150 THEN color_hsla(scale_linear(@zoom_level, 0, 150, 59, 352), scale_linear(@zoom_level, 0, 150, 81, 59), scale_linear(@zoom_level, 0, 150, 70, 44), scale_linear(@zoom_level, 0, 150, 255, 255)) WHEN @zoom_level >= 150 AND @zoom_level < 250 THEN color_hsla(scale_linear(@zoom_level, 150, 250, 352, 0), scale_linear(@zoom_level, 150, 250, 59, 72), scale_linear(@zoom_level, 150, 250, 44, 63), scale_linear(@zoom_level, 150, 250, 255, 255)) WHEN @zoom_level >= 250 THEN color_hsla(0, 72, 63, 255) ELSE color_hsla(0, 72, 63, 255) END')
        self.assertEqual(default_col.name(), '#f1f075')
        props, default_col = QgsMapBoxGlStyleConverter.parseInterpolateColorByZoom({'base': 2,
                                                                                    'stops': [[0, '#f1f075'],
                                                                                              [150, '#b52e3e'],
                                                                                              [250, '#e55e5e']]
                                                                                    },
                                                                                   conversion_context)
        self.assertEqual(props.expressionString(),
                         'CASE WHEN @zoom_level >= 0 AND @zoom_level < 150 THEN color_hsla(59 + 293 * (2^(@zoom_level-0)-1)/(2^(150-0)-1), 81 + -22 * (2^(@zoom_level-0)-1)/(2^(150-0)-1), 70 + -26 * (2^(@zoom_level-0)-1)/(2^(150-0)-1), 255 + 0 * (2^(@zoom_level-0)-1)/(2^(150-0)-1)) WHEN @zoom_level >= 150 AND @zoom_level < 250 THEN color_hsla(352 + -352 * (2^(@zoom_level-150)-1)/(2^(250-150)-1), 59 + 13 * (2^(@zoom_level-150)-1)/(2^(250-150)-1), 44 + 19 * (2^(@zoom_level-150)-1)/(2^(250-150)-1), 255 + 0 * (2^(@zoom_level-150)-1)/(2^(250-150)-1)) WHEN @zoom_level >= 250 THEN color_hsla(0, 72, 63, 255) ELSE color_hsla(0, 72, 63, 255) END')
        self.assertEqual(default_col.name(), '#f1f075')

    def testParseStops(self):
        conversion_context = QgsMapBoxGlStyleConversionContext()
        self.assertEqual(QgsMapBoxGlStyleConverter.parseStops(1, [[1, 10], [2, 20], [5, 100]], 1, conversion_context),
                         'CASE WHEN @zoom_level > 1 AND @zoom_level <= 2 THEN scale_linear(@zoom_level, 1, 2, 10, 20) * 1 WHEN @zoom_level > 2 AND @zoom_level <= 5 THEN scale_linear(@zoom_level, 2, 5, 20, 100) * 1 END')
        self.assertEqual(QgsMapBoxGlStyleConverter.parseStops(1.5, [[1, 10], [2, 20], [5, 100]], 1, conversion_context),
                         'CASE WHEN @zoom_level > 1 AND @zoom_level <= 2 THEN 10 + 10 * (1.5^(@zoom_level-1)-1)/(1.5^(2-1)-1) * 1 WHEN @zoom_level > 2 AND @zoom_level <= 5 THEN 20 + 80 * (1.5^(@zoom_level-2)-1)/(1.5^(5-2)-1) * 1 END')
        self.assertEqual(QgsMapBoxGlStyleConverter.parseStops(1, [[1, 10], [2, 20], [5, 100]], 8, conversion_context),
                         'CASE WHEN @zoom_level > 1 AND @zoom_level <= 2 THEN scale_linear(@zoom_level, 1, 2, 10, 20) * 8 WHEN @zoom_level > 2 AND @zoom_level <= 5 THEN scale_linear(@zoom_level, 2, 5, 20, 100) * 8 END')

    def testInterpolateByZoom(self):
        conversion_context = QgsMapBoxGlStyleConversionContext()
        prop, default_val = QgsMapBoxGlStyleConverter.parseInterpolateByZoom({'base': 1,
                                                                              'stops': [[0, 11],
                                                                                        [150, 15],
                                                                                        [250, 22]]
                                                                              }, conversion_context)
        self.assertEqual(prop.expressionString(),
                         'CASE WHEN @zoom_level > 0 AND @zoom_level <= 150 THEN scale_linear(@zoom_level, 0, 150, 11, 15) * 1 WHEN @zoom_level > 150 AND @zoom_level <= 250 THEN scale_linear(@zoom_level, 150, 250, 15, 22) * 1 END')
        self.assertEqual(default_val, 11.0)
        prop, default_val = QgsMapBoxGlStyleConverter.parseInterpolateByZoom({'base': 1,
                                                                              'stops': [[0, 11],
                                                                                        [150, 15]]
                                                                              }, conversion_context)
        self.assertEqual(prop.expressionString(),
                         'scale_linear(@zoom_level, 0, 150, 11, 15) * 1')
        self.assertEqual(default_val, 11.0)
        prop, default_val = QgsMapBoxGlStyleConverter.parseInterpolateByZoom({'base': 2,
                                                                              'stops': [[0, 11],
                                                                                        [150, 15]]
                                                                              }, conversion_context)
        self.assertEqual(prop.expressionString(),
                         '11 + 4 * (2^(@zoom_level-0)-1)/(2^(150-0)-1)* 1')
        self.assertEqual(default_val, 11.0)

    def testInterpolateOpacityByZoom(self):
        self.assertEqual(QgsMapBoxGlStyleConverter.parseInterpolateOpacityByZoom({'base': 1,
                                                                                  'stops': [[0, 0.1],
                                                                                            [150, 0.15],
                                                                                            [250, 0.2]]
                                                                                  }).expressionString(),
                         "CASE WHEN @zoom_level < 0 THEN set_color_part(@symbol_color, 'alpha', 0.1 * 255) WHEN @zoom_level >= 0 AND @zoom_level < 150 THEN set_color_part(@symbol_color, 'alpha', scale_linear(@zoom_level, 0, 150, 0.1 * 255, 0.15 * 255))  WHEN @zoom_level >= 150 AND @zoom_level < 250 THEN set_color_part(@symbol_color, 'alpha', scale_linear(@zoom_level, 150, 250, 0.15 * 255, 0.2 * 255)) WHEN @zoom_level >= 250 THEN set_color_part(@symbol_color, 'alpha', 0.2) END")
        self.assertEqual(QgsMapBoxGlStyleConverter.parseInterpolateOpacityByZoom({'base': 1,
                                                                                  'stops': [[0, 0.1],
                                                                                            [150, 0.15]]
                                                                                  }).expressionString(),
                         "set_color_part(@symbol_color, 'alpha', scale_linear(@zoom_level, 0, 150, 25.5, 38.25))")
        self.assertEqual(QgsMapBoxGlStyleConverter.parseInterpolateOpacityByZoom({'base': 2,
                                                                                  'stops': [[0, 0.1],
                                                                                            [150, 0.15]]
                                                                                  }).expressionString(),
                         "set_color_part(@symbol_color, 'alpha', 25.5 + 0 * (2^(@zoom_level-0)-1)/(2^(150-0)-1))")

    def testInterpolateListByZoom(self):
        conversion_context = QgsMapBoxGlStyleConversionContext()
        prop, default_color, default_val = QgsMapBoxGlStyleConverter.parseInterpolateListByZoom([
            "interpolate",
            ["linear"],
            ["zoom"],
            10,
            0.1,
            15,
            0.3,
            18,
            0.6
        ], QgsMapBoxGlStyleConverter.Opacity, conversion_context, 2)
        self.assertEqual(prop.expressionString(),
                         "CASE WHEN @zoom_level < 10 THEN set_color_part(@symbol_color, 'alpha', 0.1 * 255) WHEN @zoom_level >= 10 AND @zoom_level < 15 THEN set_color_part(@symbol_color, 'alpha', scale_linear(@zoom_level, 10, 15, 0.1 * 255, 0.3 * 255))  WHEN @zoom_level >= 15 AND @zoom_level < 18 THEN set_color_part(@symbol_color, 'alpha', scale_linear(@zoom_level, 15, 18, 0.3 * 255, 0.6 * 255)) WHEN @zoom_level >= 18 THEN set_color_part(@symbol_color, 'alpha', 0.6) END")

        prop, default_color, default_val = QgsMapBoxGlStyleConverter.parseInterpolateListByZoom([
            "interpolate",
            ["linear"],
            ["zoom"],
            10,
            0.1,
            15,
            0.3,
            18,
            0.6
        ], QgsMapBoxGlStyleConverter.Numeric, conversion_context, 2)
        self.assertEqual(prop.expressionString(),
                         "CASE WHEN @zoom_level > 10 AND @zoom_level <= 15 THEN scale_linear(@zoom_level, 10, 15, 0.1, 0.3) * 2 WHEN @zoom_level > 15 AND @zoom_level <= 18 THEN scale_linear(@zoom_level, 15, 18, 0.3, 0.6) * 2 END")
        self.assertEqual(default_val, 0.2)

    def testParseExpression(self):
        conversion_context = QgsMapBoxGlStyleConversionContext()
        self.assertEqual(QgsMapBoxGlStyleConverter.parseExpression([
            "all",
            ["==", ["get", "level"], 0],
            ["match", ["get", "type"], ["Restricted"], True, False]
        ], conversion_context),
            '''(level IS 0) AND ("type" = 'Restricted')''')

        self.assertEqual(QgsMapBoxGlStyleConverter.parseExpression([
            "match", ["get", "type"], ["Restricted"], True, False
        ], conversion_context),
            '''"type" = 'Restricted\'''')

        self.assertEqual(QgsMapBoxGlStyleConverter.parseExpression([
            "match", ["get", "type"], ["Restricted"], "r", ["Local"], "l", ["Secondary", "Main"], "m", "n"
        ], conversion_context),
            '''CASE WHEN "type" = 'Restricted' THEN 'r' WHEN "type" = 'Local' THEN 'l' WHEN "type" IN ('Secondary', 'Main') THEN 'm' ELSE 'n' END''')

        self.assertEqual(QgsMapBoxGlStyleConverter.parseExpression([
            "all",
            ["==", ["get", "level"], 0],
            ["match", ["get", "type"], ["Restricted", "Temporary"], True, False]
        ], conversion_context),
            '''(level IS 0) AND ("type" IN ('Restricted', 'Temporary'))''')
        self.assertEqual(QgsMapBoxGlStyleConverter.parseExpression([
            "any",
            ["match", ["get", "level"], [1], True, False],
            ["match", ["get", "type"], ["Local"], True, False]
        ], conversion_context),
            '''("level" = 1) OR ("type" = 'Local')''')
        self.assertEqual(QgsMapBoxGlStyleConverter.parseExpression([
            "none",
            ["match", ["get", "level"], [1], True, False],
            ["match", ["get", "type"], ["Local"], True, False]
        ], conversion_context),
            '''NOT ("level" = 1) AND NOT ("type" = 'Local')''')
        self.assertEqual(QgsMapBoxGlStyleConverter.parseExpression([
            "match",
            ["get", "type"],
            ["Primary", "Motorway"],
            False,
            True
        ], conversion_context),
            '''CASE WHEN "type" IN ('Primary', 'Motorway') THEN FALSE ELSE TRUE END''')
        self.assertEqual(QgsMapBoxGlStyleConverter.parseExpression(["==", "_symbol", 0], conversion_context),
                         '''"_symbol" IS 0''')

        self.assertEqual(QgsMapBoxGlStyleConverter.parseExpression(["all", ["==", "_symbol", 8], ["!in", "Viz", 3]],
                                                                   conversion_context),
                         '''("_symbol" IS 8) AND (("Viz" IS NULL OR "Viz" NOT IN (3)))''')

        self.assertEqual(QgsMapBoxGlStyleConverter.parseExpression(["get", "name"],
                                                                   conversion_context),
                         '''"name"''')

        self.assertEqual(QgsMapBoxGlStyleConverter.parseExpression(["to-string", ["get", "name"]],
                                                                   conversion_context),
                         '''to_string("name")''')

    def testConvertLabels(self):
        context = QgsMapBoxGlStyleConversionContext()
        style = {
            "layout": {
                "text-field": "{name_en}",
                "text-font": [
                    "Open Sans Semibold",
                    "Arial Unicode MS Bold"
                ],
                "text-max-width": 8,
                "text-anchor": "top",
                "text-size": 11,
                "icon-size": 1
            },
            "type": "symbol",
            "id": "poi_label",
            "paint": {
                "text-color": "#666",
                "text-halo-width": 1.5,
                "text-halo-color": "rgba(255,255,255,0.95)",
                "text-halo-blur": 1
            },
            "source-layer": "poi_label"
        }
        renderer, has_renderer, labeling, has_labeling = QgsMapBoxGlStyleConverter.parseSymbolLayer(style, context)
        self.assertFalse(has_renderer)
        self.assertTrue(has_labeling)
        self.assertEqual(labeling.labelSettings().fieldName, 'name_en')
        self.assertFalse(labeling.labelSettings().isExpression)

        style = {
            "layout": {
                "text-field": "name_en",
                "text-font": [
                    "Open Sans Semibold",
                    "Arial Unicode MS Bold"
                ],
                "text-max-width": 8,
                "text-anchor": "top",
                "text-size": 11,
                "icon-size": 1
            },
            "type": "symbol",
            "id": "poi_label",
            "paint": {
                "text-color": "#666",
                "text-halo-width": 1.5,
                "text-halo-color": "rgba(255,255,255,0.95)"
            },
            "source-layer": "poi_label"
        }
        renderer, has_renderer, labeling, has_labeling = QgsMapBoxGlStyleConverter.parseSymbolLayer(style, context)
        self.assertFalse(has_renderer)
        self.assertTrue(has_labeling)
        self.assertEqual(labeling.labelSettings().fieldName, 'name_en')
        self.assertFalse(labeling.labelSettings().isExpression)

        style = {
            "layout": {
                "text-field": ["format",
                               "foo", {"font-scale": 1.2},
                               "bar", {"font-scale": 0.8}
                               ],
                "text-font": [
                    "Open Sans Semibold",
                    "Arial Unicode MS Bold"
                ],
                "text-max-width": 8,
                "text-anchor": "top",
                "text-size": 11,
                "icon-size": 1
            },
            "type": "symbol",
            "id": "poi_label",
            "paint": {
                "text-color": "#666",
                "text-halo-width": 1.5,
                "text-halo-color": "rgba(255,255,255,0.95)",
                "text-halo-blur": 1
            },
            "source-layer": "poi_label"
        }
        renderer, has_renderer, labeling, has_labeling = QgsMapBoxGlStyleConverter.parseSymbolLayer(style, context)
        self.assertFalse(has_renderer)
        self.assertTrue(has_labeling)
        self.assertEqual(labeling.labelSettings().fieldName, 'concat("foo","bar")')
        self.assertTrue(labeling.labelSettings().isExpression)

        style = {
            "layout": {
                "text-field": "{name_en} - {name_fr}",
                "text-font": [
                    "Open Sans Semibold",
                    "Arial Unicode MS Bold"
                ],
                "text-max-width": 8,
                "text-anchor": "top",
                "text-size": 11,
                "icon-size": 1
            },
            "type": "symbol",
            "id": "poi_label",
            "paint": {
                "text-color": "#666",
                "text-halo-width": 1.5,
                "text-halo-color": "rgba(255,255,255,0.95)",
                "text-halo-blur": 1
            },
            "source-layer": "poi_label"
        }
        renderer, has_renderer, labeling, has_labeling = QgsMapBoxGlStyleConverter.parseSymbolLayer(style, context)
        self.assertFalse(has_renderer)
        self.assertTrue(has_labeling)
        self.assertEqual(labeling.labelSettings().fieldName, '''concat("name_en",' - ',"name_fr")''')
        self.assertTrue(labeling.labelSettings().isExpression)

        style = {
            "layout": {
                "text-field": ["format",
                               "{name_en} - {name_fr}", {"font-scale": 1.2},
                               "bar", {"font-scale": 0.8}
                               ],
                "text-font": [
                    "Open Sans Semibold",
                    "Arial Unicode MS Bold"
                ],
                "text-max-width": 8,
                "text-anchor": "top",
                "text-size": 11,
                "icon-size": 1
            },
            "type": "symbol",
            "id": "poi_label",
            "paint": {
                "text-color": "#666",
                "text-halo-width": 1.5,
                "text-halo-color": "rgba(255,255,255,0.95)",
                "text-halo-blur": 1
            },
            "source-layer": "poi_label"
        }
        renderer, has_renderer, labeling, has_labeling = QgsMapBoxGlStyleConverter.parseSymbolLayer(style, context)
        self.assertFalse(has_renderer)
        self.assertTrue(has_labeling)
        self.assertEqual(labeling.labelSettings().fieldName, '''concat(concat("name_en",' - ',"name_fr"),"bar")''')
        self.assertTrue(labeling.labelSettings().isExpression)

        style = {
            "layout": {
                "text-field": ["to-string", ["get", "name"]],
                "text-font": [
                    "Open Sans Semibold",
                    "Arial Unicode MS Bold"
                ],
                "text-max-width": 8,
                "text-anchor": "top",
                "text-size": 11,
                "icon-size": 1
            },
            "type": "symbol",
            "id": "poi_label",
            "paint": {
                "text-color": "#666",
                "text-halo-width": 1.5,
                "text-halo-color": "rgba(255,255,255,0.95)",
                "text-halo-blur": 1
            },
            "source-layer": "poi_label"
        }
        renderer, has_renderer, labeling, has_labeling = QgsMapBoxGlStyleConverter.parseSymbolLayer(style, context)
        self.assertFalse(has_renderer)
        self.assertTrue(has_labeling)
        self.assertEqual(labeling.labelSettings().fieldName, '''to_string("name")''')
        self.assertTrue(labeling.labelSettings().isExpression)

        # text-transform

        style = {
            "layout": {
                "text-field": "name_en",
                "text-font": [
                    "Open Sans Semibold",
                    "Arial Unicode MS Bold"
                ],
                "text-transform": "uppercase",
                "text-max-width": 8,
                "text-anchor": "top",
                "text-size": 11,
                "icon-size": 1
            },
            "type": "symbol",
            "id": "poi_label",
            "paint": {
                "text-color": "#666",
                "text-halo-width": 1.5,
                "text-halo-color": "rgba(255,255,255,0.95)",
                "text-halo-blur": 1
            },
            "source-layer": "poi_label"
        }
        renderer, has_renderer, labeling, has_labeling = QgsMapBoxGlStyleConverter.parseSymbolLayer(style, context)
        self.assertFalse(has_renderer)
        self.assertTrue(has_labeling)
        self.assertEqual(labeling.labelSettings().fieldName, 'upper("name_en")')
        self.assertTrue(labeling.labelSettings().isExpression)

        style = {
            "layout": {
                "text-field": ["format",
                               "{name_en} - {name_fr}", {"font-scale": 1.2},
                               "bar", {"font-scale": 0.8}
                               ],
                "text-font": [
                    "Open Sans Semibold",
                    "Arial Unicode MS Bold"
                ],
                "text-transform": "lowercase",
                "text-max-width": 8,
                "text-anchor": "top",
                "text-size": 11,
                "icon-size": 1
            },
            "type": "symbol",
            "id": "poi_label",
            "paint": {
                "text-color": "#666",
                "text-halo-width": 1.5,
                "text-halo-color": "rgba(255,255,255,0.95)",
                "text-halo-blur": 1
            },
            "source-layer": "poi_label"
        }
        renderer, has_renderer, labeling, has_labeling = QgsMapBoxGlStyleConverter.parseSymbolLayer(style, context)
        self.assertFalse(has_renderer)
        self.assertTrue(has_labeling)
        self.assertEqual(labeling.labelSettings().fieldName, '''lower(concat(concat("name_en",' - ',"name_fr"),"bar"))''')
        self.assertTrue(labeling.labelSettings().isExpression)


if __name__ == '__main__':
    unittest.main()
