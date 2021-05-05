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
                       QgsWkbTypes,
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
                         'scale_linear(@vector_tile_zoom,5,13,27,29)')
        self.assertEqual(QgsMapBoxGlStyleConverter.interpolateExpression(5, 13, 27, 29, 1.5),
                         'scale_exp(@vector_tile_zoom,5,13,27,29,1.5)')
        self.assertEqual(QgsMapBoxGlStyleConverter.interpolateExpression(5, 13, 27, 29, 1.5),
                         'scale_exp(@vector_tile_zoom,5,13,27,29,1.5)')

        # same values, return nice and simple expression!
        self.assertEqual(QgsMapBoxGlStyleConverter.interpolateExpression(5, 13, 27, 27, 1.5),
                         '27')
        self.assertEqual(QgsMapBoxGlStyleConverter.interpolateExpression(5, 13, 27, 27, 1.5, 2),
                         '54')

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
                         'CASE WHEN @vector_tile_zoom >= 0 AND @vector_tile_zoom < 150 THEN color_hsla(scale_linear(@vector_tile_zoom,0,150,59,352), scale_linear(@vector_tile_zoom,0,150,81,59), scale_linear(@vector_tile_zoom,0,150,70,44), 255) WHEN @vector_tile_zoom >= 150 AND @vector_tile_zoom < 250 THEN color_hsla(scale_linear(@vector_tile_zoom,150,250,352,0), scale_linear(@vector_tile_zoom,150,250,59,72), scale_linear(@vector_tile_zoom,150,250,44,63), 255) WHEN @vector_tile_zoom >= 250 THEN color_hsla(0, 72, 63, 255) ELSE color_hsla(0, 72, 63, 255) END')
        self.assertEqual(default_col.name(), '#f1f075')
        props, default_col = QgsMapBoxGlStyleConverter.parseInterpolateColorByZoom({'base': 2,
                                                                                    'stops': [[0, '#f1f075'],
                                                                                              [150, '#b52e3e'],
                                                                                              [250, '#e55e5e']]
                                                                                    },
                                                                                   conversion_context)
        self.assertEqual(props.expressionString(),
                         'CASE WHEN @vector_tile_zoom >= 0 AND @vector_tile_zoom < 150 THEN color_hsla(scale_exp(@vector_tile_zoom,0,150,59,352,2), scale_exp(@vector_tile_zoom,0,150,81,59,2), scale_exp(@vector_tile_zoom,0,150,70,44,2), 255) WHEN @vector_tile_zoom >= 150 AND @vector_tile_zoom < 250 THEN color_hsla(scale_exp(@vector_tile_zoom,150,250,352,0,2), scale_exp(@vector_tile_zoom,150,250,59,72,2), scale_exp(@vector_tile_zoom,150,250,44,63,2), 255) WHEN @vector_tile_zoom >= 250 THEN color_hsla(0, 72, 63, 255) ELSE color_hsla(0, 72, 63, 255) END')
        self.assertEqual(default_col.name(), '#f1f075')

    def testParseStops(self):
        conversion_context = QgsMapBoxGlStyleConversionContext()
        self.assertEqual(QgsMapBoxGlStyleConverter.parseStops(1, [[1, 10], [2, 20], [5, 100]], 1, conversion_context),
                         'CASE WHEN @vector_tile_zoom > 1 AND @vector_tile_zoom <= 2 THEN scale_linear(@vector_tile_zoom,1,2,10,20) WHEN @vector_tile_zoom > 2 AND @vector_tile_zoom <= 5 THEN scale_linear(@vector_tile_zoom,2,5,20,100) WHEN @vector_tile_zoom > 5 THEN 100 END')
        self.assertEqual(QgsMapBoxGlStyleConverter.parseStops(1.5, [[1, 10], [2, 20], [5, 100]], 1, conversion_context),
                         'CASE WHEN @vector_tile_zoom > 1 AND @vector_tile_zoom <= 2 THEN scale_exp(@vector_tile_zoom,1,2,10,20,1.5) WHEN @vector_tile_zoom > 2 AND @vector_tile_zoom <= 5 THEN scale_exp(@vector_tile_zoom,2,5,20,100,1.5) WHEN @vector_tile_zoom > 5 THEN 100 END')
        self.assertEqual(QgsMapBoxGlStyleConverter.parseStops(1, [[1, 10], [2, 20], [5, 100]], 8, conversion_context),
                         'CASE WHEN @vector_tile_zoom > 1 AND @vector_tile_zoom <= 2 THEN scale_linear(@vector_tile_zoom,1,2,10,20) * 8 WHEN @vector_tile_zoom > 2 AND @vector_tile_zoom <= 5 THEN scale_linear(@vector_tile_zoom,2,5,20,100) * 8 WHEN @vector_tile_zoom > 5 THEN 800 END')
        self.assertEqual(QgsMapBoxGlStyleConverter.parseStops(1.5, [[1, 10], [2, 20], [5, 100]], 8, conversion_context),
                         'CASE WHEN @vector_tile_zoom > 1 AND @vector_tile_zoom <= 2 THEN scale_exp(@vector_tile_zoom,1,2,10,20,1.5) * 8 WHEN @vector_tile_zoom > 2 AND @vector_tile_zoom <= 5 THEN scale_exp(@vector_tile_zoom,2,5,20,100,1.5) * 8 WHEN @vector_tile_zoom > 5 THEN 800 END')

    def testParseMatchList(self):
        conversion_context = QgsMapBoxGlStyleConversionContext()
        res, default_color, default_number = QgsMapBoxGlStyleConverter.parseMatchList([
            "match",
            ["get", "type"],
            ["Air Transport", "Airport"],
            "#e6e6e6",
            ["Education"],
            "#f7eaca",
            ["Medical Care"],
            "#f3d8e7",
            ["Road Transport"],
            "#f7f3ca",
            ["Water Transport"],
            "#d8e6f3",
            "#e7e7e7"
        ], QgsMapBoxGlStyleConverter.Color, conversion_context, 2.5, 200)
        self.assertEqual(res.asExpression(),
                         'CASE WHEN "type" IN (\'Air Transport\',\'Airport\') THEN \'#e6e6e6\' WHEN "type" IN (\'Education\') THEN \'#f7eaca\' WHEN "type" IN (\'Medical Care\') THEN \'#f3d8e7\' WHEN "type" IN (\'Road Transport\') THEN \'#f7f3ca\' WHEN "type" IN (\'Water Transport\') THEN \'#d8e6f3\' ELSE \'#e7e7e7\' END')
        self.assertEqual(default_color.name(), '#e7e7e7')

        res, default_color, default_number = QgsMapBoxGlStyleConverter.parseMatchList([
            "match",
            ["get", "type"],
            ["Normal"],
            0.25,
            ["Index"],
            0.5,
            0.2
        ], QgsMapBoxGlStyleConverter.Numeric, conversion_context, 2.5, 200)
        self.assertEqual(res.asExpression(),
                         'CASE WHEN "type" IN (\'Normal\') THEN 0.625 WHEN "type" IN (\'Index\') THEN 1.25 ELSE 0.5 END')
        self.assertEqual(default_number, 0.5)

    def testParseValueList(self):
        conversion_context = QgsMapBoxGlStyleConversionContext()
        res, default_color, default_number = QgsMapBoxGlStyleConverter.parseValueList([
            "match",
            ["get", "type"],
            ["Air Transport", "Airport"],
            "#e6e6e6",
            ["Education"],
            "#f7eaca",
            ["Medical Care"],
            "#f3d8e7",
            ["Road Transport"],
            "#f7f3ca",
            ["Water Transport"],
            "#d8e6f3",
            "#e7e7e7"
        ], QgsMapBoxGlStyleConverter.Color, conversion_context, 2.5, 200)
        self.assertEqual(res.asExpression(),
                         'CASE WHEN "type" IN (\'Air Transport\',\'Airport\') THEN \'#e6e6e6\' WHEN "type" IN (\'Education\') THEN \'#f7eaca\' WHEN "type" IN (\'Medical Care\') THEN \'#f3d8e7\' WHEN "type" IN (\'Road Transport\') THEN \'#f7f3ca\' WHEN "type" IN (\'Water Transport\') THEN \'#d8e6f3\' ELSE \'#e7e7e7\' END')
        self.assertEqual(default_color.name(), '#e7e7e7')

        res, default_color, default_number = QgsMapBoxGlStyleConverter.parseValueList([
            "interpolate",
            ["linear"],
            ["zoom"],
            10,
            0.1,
            15,
            0.3,
            18,
            0.6
        ], QgsMapBoxGlStyleConverter.Numeric, conversion_context, 2.5, 200)
        self.assertEqual(res.asExpression(),
                         'CASE WHEN @vector_tile_zoom > 10 AND @vector_tile_zoom <= 15 THEN scale_linear(@vector_tile_zoom,10,15,0.1,0.3) * 2.5 WHEN @vector_tile_zoom > 15 AND @vector_tile_zoom <= 18 THEN scale_linear(@vector_tile_zoom,15,18,0.3,0.6) * 2.5 WHEN @vector_tile_zoom > 18 THEN 1.5 END')
        self.assertEqual(default_number, 0.25)

    def testInterpolateByZoom(self):
        conversion_context = QgsMapBoxGlStyleConversionContext()
        prop, default_val = QgsMapBoxGlStyleConverter.parseInterpolateByZoom({'base': 1,
                                                                              'stops': [[0, 11],
                                                                                        [150, 15],
                                                                                        [250, 22]]
                                                                              }, conversion_context)
        self.assertEqual(prop.expressionString(),
                         'CASE WHEN @vector_tile_zoom > 0 AND @vector_tile_zoom <= 150 THEN scale_linear(@vector_tile_zoom,0,150,11,15) WHEN @vector_tile_zoom > 150 AND @vector_tile_zoom <= 250 THEN scale_linear(@vector_tile_zoom,150,250,15,22) WHEN @vector_tile_zoom > 250 THEN 22 END')
        self.assertEqual(default_val, 11.0)
        prop, default_val = QgsMapBoxGlStyleConverter.parseInterpolateByZoom({'base': 1,
                                                                              'stops': [[0, 11],
                                                                                        [150, 15]]
                                                                              }, conversion_context)
        self.assertEqual(prop.expressionString(),
                         'scale_linear(@vector_tile_zoom,0,150,11,15)')
        self.assertEqual(default_val, 11.0)
        prop, default_val = QgsMapBoxGlStyleConverter.parseInterpolateByZoom({'base': 2,
                                                                              'stops': [[0, 11],
                                                                                        [150, 15]]
                                                                              }, conversion_context)
        self.assertEqual(prop.expressionString(),
                         'scale_exp(@vector_tile_zoom,0,150,11,15,2)')
        self.assertEqual(default_val, 11.0)

        prop, default_val = QgsMapBoxGlStyleConverter.parseInterpolateByZoom({'base': 2,
                                                                              'stops': [[0, 11],
                                                                                        [150, 15]]
                                                                              }, conversion_context, multiplier=5)
        self.assertEqual(prop.expressionString(),
                         'scale_exp(@vector_tile_zoom,0,150,11,15,2) * 5')
        self.assertEqual(default_val, 55.0)

    def testInterpolateOpacityByZoom(self):
        self.assertEqual(QgsMapBoxGlStyleConverter.parseInterpolateOpacityByZoom({'base': 1,
                                                                                  'stops': [[0, 0.1],
                                                                                            [150, 0.15],
                                                                                            [250, 0.2]]
                                                                                  }, 255).expressionString(),
                         "CASE WHEN @vector_tile_zoom < 0 THEN set_color_part(@symbol_color, 'alpha', 25.5) WHEN @vector_tile_zoom >= 0 AND @vector_tile_zoom < 150 THEN set_color_part(@symbol_color, 'alpha', scale_linear(@vector_tile_zoom,0,150,25.5,38.25)) WHEN @vector_tile_zoom >= 150 AND @vector_tile_zoom < 250 THEN set_color_part(@symbol_color, 'alpha', scale_linear(@vector_tile_zoom,150,250,38.25,51)) WHEN @vector_tile_zoom >= 250 THEN set_color_part(@symbol_color, 'alpha', 51) END")
        self.assertEqual(QgsMapBoxGlStyleConverter.parseInterpolateOpacityByZoom({'base': 1,
                                                                                  'stops': [[0, 0.1],
                                                                                            [150, 0.15],
                                                                                            [250, 0.2]]
                                                                                  }, 100).expressionString(),
                         "CASE WHEN @vector_tile_zoom < 0 THEN set_color_part(@symbol_color, 'alpha', 10) WHEN @vector_tile_zoom >= 0 AND @vector_tile_zoom < 150 THEN set_color_part(@symbol_color, 'alpha', scale_linear(@vector_tile_zoom,0,150,10,15)) WHEN @vector_tile_zoom >= 150 AND @vector_tile_zoom < 250 THEN set_color_part(@symbol_color, 'alpha', scale_linear(@vector_tile_zoom,150,250,15,20)) WHEN @vector_tile_zoom >= 250 THEN set_color_part(@symbol_color, 'alpha', 20) END")
        self.assertEqual(QgsMapBoxGlStyleConverter.parseInterpolateOpacityByZoom({'base': 1,
                                                                                  'stops': [[0, 0.1],
                                                                                            [150, 0.15]]
                                                                                  }, 255).expressionString(),
                         "set_color_part(@symbol_color, 'alpha', scale_linear(@vector_tile_zoom,0,150,25.5,38.25))")
        self.assertEqual(QgsMapBoxGlStyleConverter.parseInterpolateOpacityByZoom({'base': 2,
                                                                                  'stops': [[0, 0.1],
                                                                                            [150, 0.15]]
                                                                                  }, 255).expressionString(),
                         "set_color_part(@symbol_color, 'alpha', scale_exp(@vector_tile_zoom,0,150,25.5,38.25,2))")
        self.assertEqual(QgsMapBoxGlStyleConverter.parseInterpolateOpacityByZoom({'base': 2,
                                                                                  'stops': [[0, 0.1],
                                                                                            [150, 0.1]]
                                                                                  }, 255).expressionString(),
                         "set_color_part(@symbol_color, 'alpha', 25.5)")

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
                         "CASE WHEN @vector_tile_zoom < 10 THEN set_color_part(@symbol_color, 'alpha', 25.5) WHEN @vector_tile_zoom >= 10 AND @vector_tile_zoom < 15 THEN set_color_part(@symbol_color, 'alpha', scale_linear(@vector_tile_zoom,10,15,25.5,76.5)) WHEN @vector_tile_zoom >= 15 AND @vector_tile_zoom < 18 THEN set_color_part(@symbol_color, 'alpha', scale_linear(@vector_tile_zoom,15,18,76.5,153)) WHEN @vector_tile_zoom >= 18 THEN set_color_part(@symbol_color, 'alpha', 153) END")

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
                         "CASE WHEN @vector_tile_zoom > 10 AND @vector_tile_zoom <= 15 THEN scale_linear(@vector_tile_zoom,10,15,0.1,0.3) * 2 WHEN @vector_tile_zoom > 15 AND @vector_tile_zoom <= 18 THEN scale_linear(@vector_tile_zoom,15,18,0.3,0.6) * 2 WHEN @vector_tile_zoom > 18 THEN 1.2 END")
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

    def testCircleLayer(self):
        context = QgsMapBoxGlStyleConversionContext()
        style = {
            "id": "cicle_layer",
            "type": "circle",
            "paint": {
                "circle-stroke-color": "rgba(46, 46, 46, 1)",
                "circle-stroke-opacity": 0.5,
                "circle-stroke-width": 3,
                "circle-color": "rgba(22, 22, 22, 1)",
                "circle-opacity": 0.6,
                "circle-radius": 33,
                "circle-translate": [11, 22]
            }
        }
        has_renderer, rendererStyle = QgsMapBoxGlStyleConverter.parseCircleLayer(style, context)
        self.assertTrue(has_renderer)
        self.assertEqual(rendererStyle.geometryType(), QgsWkbTypes.PointGeometry)
        properties = rendererStyle.symbol().symbolLayers()[0].properties()
        expected_properties = {
            'angle': '0',
            'cap_style': 'square',
            'color': '22,22,22,153',
            'horizontal_anchor_point': '1',
            'joinstyle': 'bevel',
            'name': 'circle',
            'offset': '11,22',
            'offset_map_unit_scale': '3x:0,0,0,0,0,0',
            'offset_unit': 'Pixel',
            'outline_color': '46,46,46,128',
            'outline_style': 'solid',
            'outline_width': '3',
            'outline_width_map_unit_scale': '3x:0,0,0,0,0,0',
            'outline_width_unit': 'Pixel',
            'scale_method': 'diameter',
            'size': '66',
            'size_map_unit_scale': '3x:0,0,0,0,0,0',
            'size_unit': 'Pixel',
            'vertical_anchor_point': '1'}
        self.assertEqual(properties, expected_properties)


if __name__ == '__main__':
    unittest.main()
