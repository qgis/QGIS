"""QGIS Unit tests for QgsMapBoxGlStyleConverter.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "(C) 2020 by Nyall Dawson"
__date__ = "29/07/2020"
__copyright__ = "Copyright 2020, The QGIS Project"

import json

from qgis.PyQt.QtCore import (
    Qt,
    QCoreApplication,
    QSize,
    QSizeF,
)
from qgis.PyQt.QtGui import QColor, QImage
from qgis.core import (
    Qgis,
    QgsApplication,
    QgsMapBoxGlStyleConversionContext,
    QgsMapBoxGlStyleConverter,
    QgsMapBoxGlStyleRasterSource,
    QgsPalLayerSettings,
    QgsRasterLayer,
    QgsRasterPipe,
    QgsSettings,
    QgsSymbol,
    QgsSymbolLayer,
    QgsWkbTypes,
)
import unittest
from qgis.testing import start_app, QgisTestCase
from qgis.core import qgsDoubleNear

from utilities import getTestFont, unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsMapBoxGlStyleConverter(QgisTestCase):
    maxDiff = 100000

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("QGIS_TestQgsMapBoxGlStyleConverter.com")
        QCoreApplication.setApplicationName("QGIS_TestQgsMapBoxGlStyleConverter")
        QgsSettings().clear()
        start_app()

    def testNoLayer(self):
        c = QgsMapBoxGlStyleConverter()
        self.assertEqual(
            c.convert({"x": "y"}), QgsMapBoxGlStyleConverter.Result.NoLayerList
        )
        self.assertEqual(c.errorMessage(), "Could not find layers list in JSON")
        self.assertIsNone(c.renderer())
        self.assertIsNone(c.labeling())

    def testInterpolateExpression(self):
        self.assertEqual(
            QgsMapBoxGlStyleConverter.interpolateExpression(5, 13, 27, 29, 1),
            "scale_linear(@vector_tile_zoom,5,13,27,29)",
        )
        self.assertEqual(
            QgsMapBoxGlStyleConverter.interpolateExpression(5, 13, 27, 29, 1.5),
            "scale_exponential(@vector_tile_zoom,5,13,27,29,1.5)",
        )

        # same values, return nice and simple expression!
        self.assertEqual(
            QgsMapBoxGlStyleConverter.interpolateExpression(5, 13, 27, 27, 1.5), "27"
        )
        self.assertEqual(
            QgsMapBoxGlStyleConverter.interpolateExpression(5, 13, 27, 27, 1.5, 2), "54"
        )

    def testColorAsHslaComponents(self):
        self.assertEqual(
            QgsMapBoxGlStyleConverter.colorAsHslaComponents(QColor.fromHsl(30, 50, 70)),
            (30, 19, 27, 255),
        )

    def testParseInterpolateColorByZoom(self):
        conversion_context = QgsMapBoxGlStyleConversionContext()
        props, default_col = QgsMapBoxGlStyleConverter.parseInterpolateColorByZoom(
            {}, conversion_context
        )
        self.assertEqual(props.isActive(), False)
        props, default_col = QgsMapBoxGlStyleConverter.parseInterpolateColorByZoom(
            {"base": 1, "stops": [[0, "#f1f075"], [150, "#b52e3e"], [250, "#e55e5e"]]},
            conversion_context,
        )
        self.assertEqual(
            props.expressionString(),
            "CASE WHEN @vector_tile_zoom < 0 THEN color_hsla(59, 81, 70, 255) WHEN @vector_tile_zoom >= 0 AND @vector_tile_zoom < 150 THEN color_hsla(scale_linear(@vector_tile_zoom,0,150,59,352), scale_linear(@vector_tile_zoom,0,150,81,59), scale_linear(@vector_tile_zoom,0,150,70,44), 255) WHEN @vector_tile_zoom >= 150 AND @vector_tile_zoom < 250 THEN color_hsla(scale_linear(@vector_tile_zoom,150,250,352,0), scale_linear(@vector_tile_zoom,150,250,59,72), scale_linear(@vector_tile_zoom,150,250,44,63), 255) WHEN @vector_tile_zoom >= 250 THEN color_hsla(0, 72, 63, 255) ELSE color_hsla(0, 72, 63, 255) END",
        )
        self.assertEqual(default_col.name(), "#f1f075")
        props, default_col = QgsMapBoxGlStyleConverter.parseInterpolateColorByZoom(
            {"base": 2, "stops": [[0, "#f1f075"], [150, "#b52e3e"], [250, "#e55e5e"]]},
            conversion_context,
        )
        self.assertEqual(
            props.expressionString(),
            (
                "CASE WHEN @vector_tile_zoom < 0 THEN color_hsla(59, 81, 70, 255) "
                "WHEN @vector_tile_zoom >= 0 AND @vector_tile_zoom < 150 THEN color_hsla(scale_exponential(@vector_tile_zoom,0,150,59,352,2), scale_exponential(@vector_tile_zoom,0,150,81,59,2), scale_exponential(@vector_tile_zoom,0,150,70,44,2), 255) "
                "WHEN @vector_tile_zoom >= 150 AND @vector_tile_zoom < 250 THEN color_hsla(scale_exponential(@vector_tile_zoom,150,250,352,0,2), scale_exponential(@vector_tile_zoom,150,250,59,72,2), scale_exponential(@vector_tile_zoom,150,250,44,63,2), 255) "
                "WHEN @vector_tile_zoom >= 250 THEN color_hsla(0, 72, 63, 255) ELSE color_hsla(0, 72, 63, 255) END"
            ),
        )
        self.assertEqual(default_col.name(), "#f1f075")

        props, default_col = QgsMapBoxGlStyleConverter.parseInterpolateColorByZoom(
            {
                "base": 1,
                "stops": [
                    [
                        "9",
                        [
                            "match",
                            ["get", "class"],
                            ["motorway", "trunk"],
                            "rgb(255,230,160)",
                            "rgb(255,255,255)",
                        ],
                    ],
                    [
                        "15",
                        [
                            "match",
                            ["get", "class"],
                            ["motorway", "trunk"],
                            "rgb(255, 224, 138)",
                            "rgb(255,255,255)",
                        ],
                    ],
                ],
            },
            conversion_context,
        )
        self.assertEqual(
            props.expressionString(),
            "CASE WHEN @vector_tile_zoom >= 9 AND @vector_tile_zoom < 15 THEN color_hsla(scale_linear(@vector_tile_zoom,9,15,color_part(CASE WHEN \"class\" IN ('motorway', 'trunk') THEN color_rgba(255,230,160,255) ELSE color_rgba(255,255,255,255) END,'hsl_hue'),color_part(CASE WHEN \"class\" IN ('motorway', 'trunk') THEN color_rgba(255,224,138,255) ELSE color_rgba(255,255,255,255) END,'hsl_hue')), scale_linear(@vector_tile_zoom,9,15,color_part(CASE WHEN \"class\" IN ('motorway', 'trunk') THEN color_rgba(255,230,160,255) ELSE color_rgba(255,255,255,255) END,'hsl_saturation'),color_part(CASE WHEN \"class\" IN ('motorway', 'trunk') THEN color_rgba(255,224,138,255) ELSE color_rgba(255,255,255,255) END,'hsl_saturation')), scale_linear(@vector_tile_zoom,9,15,color_part(CASE WHEN \"class\" IN ('motorway', 'trunk') THEN color_rgba(255,230,160,255) ELSE color_rgba(255,255,255,255) END,'lightness'),color_part(CASE WHEN \"class\" IN ('motorway', 'trunk') THEN color_rgba(255,224,138,255) ELSE color_rgba(255,255,255,255) END,'lightness')), scale_linear(@vector_tile_zoom,9,15,color_part(CASE WHEN \"class\" IN ('motorway', 'trunk') THEN color_rgba(255,230,160,255) ELSE color_rgba(255,255,255,255) END,'alpha'),color_part(CASE WHEN \"class\" IN ('motorway', 'trunk') THEN color_rgba(255,224,138,255) ELSE color_rgba(255,255,255,255) END,'alpha'))) WHEN @vector_tile_zoom >= 15 THEN color_hsla(color_part(CASE WHEN \"class\" IN ('motorway', 'trunk') THEN color_rgba(255,224,138,255) ELSE color_rgba(255,255,255,255) END,'hsl_hue'), color_part(CASE WHEN \"class\" IN ('motorway', 'trunk') THEN color_rgba(255,224,138,255) ELSE color_rgba(255,255,255,255) END,'hsl_saturation'), color_part(CASE WHEN \"class\" IN ('motorway', 'trunk') THEN color_rgba(255,224,138,255) ELSE color_rgba(255,255,255,255) END,'lightness'), color_part(CASE WHEN \"class\" IN ('motorway', 'trunk') THEN color_rgba(255,224,138,255) ELSE color_rgba(255,255,255,255) END,'alpha')) ELSE color_hsla(color_part(CASE WHEN \"class\" IN ('motorway', 'trunk') THEN color_rgba(255,224,138,255) ELSE color_rgba(255,255,255,255) END,'hsl_hue'), color_part(CASE WHEN \"class\" IN ('motorway', 'trunk') THEN color_rgba(255,224,138,255) ELSE color_rgba(255,255,255,255) END,'hsl_saturation'), color_part(CASE WHEN \"class\" IN ('motorway', 'trunk') THEN color_rgba(255,224,138,255) ELSE color_rgba(255,255,255,255) END,'lightness'), color_part(CASE WHEN \"class\" IN ('motorway', 'trunk') THEN color_rgba(255,224,138,255) ELSE color_rgba(255,255,255,255) END,'alpha')) END",
        )

    def testParseStops(self):
        conversion_context = QgsMapBoxGlStyleConversionContext()
        self.assertEqual(
            QgsMapBoxGlStyleConverter.parseStops(
                1, [[1, 10], [2, 20], [5, 100]], 1, conversion_context
            ),
            "CASE WHEN @vector_tile_zoom >= 1 AND @vector_tile_zoom <= 2 THEN scale_linear(@vector_tile_zoom,1,2,10,20) WHEN @vector_tile_zoom > 2 AND @vector_tile_zoom <= 5 THEN scale_linear(@vector_tile_zoom,2,5,20,100) WHEN @vector_tile_zoom > 5 THEN 100 END",
        )
        self.assertEqual(
            QgsMapBoxGlStyleConverter.parseStops(
                1.5, [[1, 10], [2, 20], [5, 100]], 1, conversion_context
            ),
            "CASE WHEN @vector_tile_zoom >= 1 AND @vector_tile_zoom <= 2 THEN scale_exponential(@vector_tile_zoom,1,2,10,20,1.5) WHEN @vector_tile_zoom > 2 AND @vector_tile_zoom <= 5 THEN scale_exponential(@vector_tile_zoom,2,5,20,100,1.5) WHEN @vector_tile_zoom > 5 THEN 100 END",
        )
        self.assertEqual(
            QgsMapBoxGlStyleConverter.parseStops(
                1, [[1, 10], [2, 20], [5, 100]], 8, conversion_context
            ),
            "CASE WHEN @vector_tile_zoom >= 1 AND @vector_tile_zoom <= 2 THEN (scale_linear(@vector_tile_zoom,1,2,10,20)) * 8 WHEN @vector_tile_zoom > 2 AND @vector_tile_zoom <= 5 THEN (scale_linear(@vector_tile_zoom,2,5,20,100)) * 8 WHEN @vector_tile_zoom > 5 THEN 800 END",
        )
        self.assertEqual(
            QgsMapBoxGlStyleConverter.parseStops(
                1.5, [[1, 10], [2, 20], [5, 100]], 8, conversion_context
            ),
            (
                "CASE WHEN @vector_tile_zoom >= 1 AND @vector_tile_zoom <= 2 THEN (scale_exponential(@vector_tile_zoom,1,2,10,20,1.5)) * 8 "
                "WHEN @vector_tile_zoom > 2 AND @vector_tile_zoom <= 5 THEN (scale_exponential(@vector_tile_zoom,2,5,20,100,1.5)) * 8 "
                "WHEN @vector_tile_zoom > 5 THEN 800 END"
            ),
        )

    def testParseMatchList(self):
        conversion_context = QgsMapBoxGlStyleConversionContext()
        res, default_color, default_number = QgsMapBoxGlStyleConverter.parseMatchList(
            [
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
                "#e7e7e7",
            ],
            QgsMapBoxGlStyleConverter.PropertyType.Color,
            conversion_context,
            2.5,
            200,
        )
        self.assertEqual(
            res.asExpression(),
            "CASE WHEN \"type\" IN ('Air Transport','Airport') THEN '#e6e6e6' WHEN \"type\" IS 'Education' THEN '#f7eaca' WHEN \"type\" IS 'Medical Care' THEN '#f3d8e7' WHEN \"type\" IS 'Road Transport' THEN '#f7f3ca' WHEN \"type\" IS 'Water Transport' THEN '#d8e6f3' ELSE '#e7e7e7' END",
        )
        self.assertEqual(default_color.name(), "#e7e7e7")

        res, default_color, default_number = QgsMapBoxGlStyleConverter.parseMatchList(
            ["match", ["get", "type"], ["Normal"], 0.25, ["Index"], 0.5, 0.2],
            QgsMapBoxGlStyleConverter.PropertyType.Numeric,
            conversion_context,
            2.5,
            200,
        )
        self.assertEqual(
            res.asExpression(),
            "CASE WHEN \"type\" IS 'Normal' THEN 0.625 WHEN \"type\" IS 'Index' THEN 1.25 ELSE 0.5 END",
        )
        self.assertEqual(default_number, 0.5)

        res, default_color, default_number = QgsMapBoxGlStyleConverter.parseMatchList(
            [
                "match",
                ["get", "luminosity"],
                -15,
                "rgb(200,210,213)",
                -14,
                "rgb(203,213,216)",
                -13,
                "rgb(207,215,218)",
                -12,
                "rgb(210,218,221)",
                -11,
                "rgb(213,221,224)",
                -10,
                "rgb(217,224,226)",
                -9,
                "rgb(220,227,229)",
                -8,
                "rgb(224,230,231)",
                -7,
                "rgb(227,232,234)",
                -6,
                "rgb(231,235,237)",
                -5,
                "rgb(234,238,239)",
                -4,
                "rgb(238,241,242)",
                -3,
                "rgb(241,244,245)",
                -2,
                "rgb(245,247,247)",
                -1,
                "rgb(248,249,250)",
                "rgb(252, 252, 252)",
            ],
            QgsMapBoxGlStyleConverter.PropertyType.Color,
            conversion_context,
            2.5,
            200,
        )
        self.assertEqual(
            res.asExpression(),
            "CASE WHEN \"luminosity\" IS -15 THEN '#c8d2d5' WHEN \"luminosity\" IS -14 THEN '#cbd5d8' WHEN \"luminosity\" IS -13 THEN '#cfd7da' WHEN \"luminosity\" IS -12 THEN '#d2dadd' WHEN \"luminosity\" IS -11 THEN '#d5dde0' WHEN \"luminosity\" IS -10 THEN '#d9e0e2' WHEN \"luminosity\" IS -9 THEN '#dce3e5' WHEN \"luminosity\" IS -8 THEN '#e0e6e7' WHEN \"luminosity\" IS -7 THEN '#e3e8ea' WHEN \"luminosity\" IS -6 THEN '#e7ebed' WHEN \"luminosity\" IS -5 THEN '#eaeeef' WHEN \"luminosity\" IS -4 THEN '#eef1f2' WHEN \"luminosity\" IS -3 THEN '#f1f4f5' WHEN \"luminosity\" IS -2 THEN '#f5f7f7' WHEN \"luminosity\" IS -1 THEN '#f8f9fa' ELSE '#fcfcfc' END",
        )
        self.assertTrue(qgsDoubleNear(default_number, 0.0))

        res, default_color, default_number = QgsMapBoxGlStyleConverter.parseMatchList(
            [
                "match",
                ["get", "class"],
                "scree",
                "rgba(0, 0, 0, 1)",
                "hsl(35, 86%, 38%)",
            ],
            QgsMapBoxGlStyleConverter.PropertyType.Color,
            conversion_context,
            2.5,
            200,
        )
        self.assertEqual(
            res.asExpression(),
            """CASE WHEN "class" IS 'scree' THEN '#000000' ELSE '#b26e0e' END""",
        )
        self.assertTrue(qgsDoubleNear(default_number, 0.0))

    def testParseStepList(self):
        conversion_context = QgsMapBoxGlStyleConversionContext()
        res, default_color, default_number = QgsMapBoxGlStyleConverter.parseStepList(
            [
                "step",
                ["zoom"],
                0,
                7,
                ["match", ["get", "capital"], [2, 4], 1, 0],
                8,
                ["case", [">", 14, ["get", "rank"]], 1, 0],
                9,
                ["case", [">", 15, ["get", "rank"]], 1, 0],
                10,
                ["case", [">", 18, ["get", "rank"]], 1, 0],
                11,
                ["case", [">", 28, ["get", "rank"]], 1, 0],
                12,
                1,
                13,
                0,
            ],
            QgsMapBoxGlStyleConverter.PropertyType.Opacity,
            conversion_context,
            100,
            255,
        )
        self.assertEqual(
            res.asExpression(),
            'CASE  WHEN @vector_tile_zoom >= 13 THEN (0)  WHEN @vector_tile_zoom >= 12 THEN (255)  WHEN @vector_tile_zoom >= 11 THEN (CASE WHEN ("28" > "rank") THEN 1 ELSE 0 END)  WHEN @vector_tile_zoom >= 10 THEN (CASE WHEN ("18" > "rank") THEN 1 ELSE 0 END)  WHEN @vector_tile_zoom >= 9 THEN (CASE WHEN ("15" > "rank") THEN 1 ELSE 0 END)  WHEN @vector_tile_zoom >= 8 THEN (CASE WHEN ("14" > "rank") THEN 1 ELSE 0 END)  WHEN @vector_tile_zoom >= 7 THEN (CASE WHEN "capital" IN (2,4) THEN 255 ELSE 0 END) ELSE (0) END',
        )

    def testParseValueList(self):
        conversion_context = QgsMapBoxGlStyleConversionContext()
        res, default_color, default_number = QgsMapBoxGlStyleConverter.parseValueList(
            [
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
                "#e7e7e7",
            ],
            QgsMapBoxGlStyleConverter.PropertyType.Color,
            conversion_context,
            2.5,
            200,
        )
        self.assertEqual(
            res.asExpression(),
            "CASE WHEN \"type\" IN ('Air Transport','Airport') THEN '#e6e6e6' WHEN \"type\" IS 'Education' THEN '#f7eaca' WHEN \"type\" IS 'Medical Care' THEN '#f3d8e7' WHEN \"type\" IS 'Road Transport' THEN '#f7f3ca' WHEN \"type\" IS 'Water Transport' THEN '#d8e6f3' ELSE '#e7e7e7' END",
        )
        self.assertEqual(default_color.name(), "#e7e7e7")

        res, default_color, default_number = QgsMapBoxGlStyleConverter.parseValueList(
            ["interpolate", ["linear"], ["zoom"], 10, 0.1, 15, 0.3, 18, 0.6],
            QgsMapBoxGlStyleConverter.PropertyType.Numeric,
            conversion_context,
            2.5,
            200,
        )
        self.assertEqual(
            res.asExpression(),
            "CASE WHEN @vector_tile_zoom >= 10 AND @vector_tile_zoom <= 15 THEN (scale_linear(@vector_tile_zoom,10,15,0.1,0.3)) * 2.5 WHEN @vector_tile_zoom > 15 AND @vector_tile_zoom <= 18 THEN (scale_linear(@vector_tile_zoom,15,18,0.3,0.6)) * 2.5 WHEN @vector_tile_zoom > 18 THEN 1.5 END",
        )
        self.assertEqual(default_number, 0.25)

        # nested match list
        res, default_color, default_number = QgsMapBoxGlStyleConverter.parseValueList(
            [
                "match",
                ["get", "is_route"],
                [5, 10],
                "hsl(16,91%,80%)",
                [6, 7, 8],
                "hsl(55,91%,80%)",
                [
                    "match",
                    ["get", "class"],
                    [
                        "motorway",
                        "trunk",
                        "motorway_construction",
                        "trunk_construction",
                    ],
                    "hsl(41,93%,73%)",
                    [
                        "rail",
                        "rail_construction",
                        "path",
                        "path_construction",
                        "footway",
                        "footway_construction",
                        "track",
                        "track_construction",
                        "trail",
                        "trail_construction",
                    ],
                    [
                        "match",
                        ["get", "subclass"],
                        "covered_bridge",
                        "rgb(255,255,255)",
                        "rgb(238,238,240)",
                    ],
                    "rgba(255,255,255,1)",
                ],
            ],
            QgsMapBoxGlStyleConverter.PropertyType.Color,
            conversion_context,
            2.5,
            200,
        )
        self.assertEqual(
            res.asExpression(),
            """CASE WHEN "is_route" IN (5,10) THEN '#fab69e' WHEN "is_route" IN (6,7,8) THEN '#faf39e' ELSE CASE WHEN "class" IN ('motorway','trunk','motorway_construction','trunk_construction') THEN '#fad27a' WHEN "class" IN ('rail','rail_construction','path','path_construction','footway','footway_construction','track','track_construction','trail','trail_construction') THEN CASE WHEN "subclass" IS 'covered_bridge' THEN '#ffffff' ELSE '#eeeef0' END ELSE '#ffffff' END END""",
        )

    def testInterpolateByZoom(self):
        conversion_context = QgsMapBoxGlStyleConversionContext()
        prop, default_val = QgsMapBoxGlStyleConverter.parseInterpolateByZoom(
            {"base": 1, "stops": [[0, 11], [150, 15], [250, 22]]}, conversion_context
        )
        self.assertEqual(
            prop.expressionString(),
            "CASE WHEN @vector_tile_zoom >= 0 AND @vector_tile_zoom <= 150 THEN scale_linear(@vector_tile_zoom,0,150,11,15) WHEN @vector_tile_zoom > 150 AND @vector_tile_zoom <= 250 THEN scale_linear(@vector_tile_zoom,150,250,15,22) WHEN @vector_tile_zoom > 250 THEN 22 END",
        )
        self.assertEqual(default_val, 11.0)
        prop, default_val = QgsMapBoxGlStyleConverter.parseInterpolateByZoom(
            {"base": 1, "stops": [[0, 11], [150, 15]]}, conversion_context
        )
        self.assertEqual(
            prop.expressionString(), "scale_linear(@vector_tile_zoom,0,150,11,15)"
        )
        self.assertEqual(default_val, 11.0)
        prop, default_val = QgsMapBoxGlStyleConverter.parseInterpolateByZoom(
            {"base": 2, "stops": [[0, 11], [150, 15]]}, conversion_context
        )
        self.assertEqual(
            prop.expressionString(),
            "scale_exponential(@vector_tile_zoom,0,150,11,15,2)",
        )
        self.assertEqual(default_val, 11.0)

        prop, default_val = QgsMapBoxGlStyleConverter.parseInterpolateByZoom(
            {"base": 2, "stops": [[0, 11], [150, 15]]}, conversion_context, multiplier=5
        )
        self.assertEqual(
            prop.expressionString(),
            "(scale_exponential(@vector_tile_zoom,0,150,11,15,2)) * 5",
        )
        self.assertEqual(default_val, 55.0)

    def testInterpolateOpacityByZoom(self):
        conversion_context = QgsMapBoxGlStyleConversionContext()
        self.assertEqual(
            QgsMapBoxGlStyleConverter.parseInterpolateOpacityByZoom(
                {"base": 1, "stops": [[0, 0.1], [150, 0.15], [250, 0.2]]},
                255,
                conversion_context,
            ).expressionString(),
            "CASE WHEN @vector_tile_zoom < 0 THEN set_color_part(@symbol_color, 'alpha', 25.5) WHEN @vector_tile_zoom >= 0 AND @vector_tile_zoom < 150 THEN set_color_part(@symbol_color, 'alpha', scale_linear(@vector_tile_zoom,0,150,25.5,38.25)) WHEN @vector_tile_zoom >= 150 AND @vector_tile_zoom < 250 THEN set_color_part(@symbol_color, 'alpha', scale_linear(@vector_tile_zoom,150,250,38.25,51)) WHEN @vector_tile_zoom >= 250 THEN set_color_part(@symbol_color, 'alpha', 51) END",
        )
        self.assertEqual(
            QgsMapBoxGlStyleConverter.parseInterpolateOpacityByZoom(
                {"base": 1, "stops": [[0, 0.1], [150, 0.15], [250, 0.2]]},
                100,
                conversion_context,
            ).expressionString(),
            "CASE WHEN @vector_tile_zoom < 0 THEN set_color_part(@symbol_color, 'alpha', 10) WHEN @vector_tile_zoom >= 0 AND @vector_tile_zoom < 150 THEN set_color_part(@symbol_color, 'alpha', scale_linear(@vector_tile_zoom,0,150,10,15)) WHEN @vector_tile_zoom >= 150 AND @vector_tile_zoom < 250 THEN set_color_part(@symbol_color, 'alpha', scale_linear(@vector_tile_zoom,150,250,15,20)) WHEN @vector_tile_zoom >= 250 THEN set_color_part(@symbol_color, 'alpha', 20) END",
        )
        self.assertEqual(
            QgsMapBoxGlStyleConverter.parseInterpolateOpacityByZoom(
                {"base": 1, "stops": [[0, 0.1], [150, 0.15]]}, 255, conversion_context
            ).expressionString(),
            "set_color_part(@symbol_color, 'alpha', scale_linear(@vector_tile_zoom,0,150,25.5,38.25))",
        )
        self.assertEqual(
            QgsMapBoxGlStyleConverter.parseInterpolateOpacityByZoom(
                {"base": 2, "stops": [[0, 0.1], [150, 0.15]]}, 255, conversion_context
            ).expressionString(),
            "set_color_part(@symbol_color, 'alpha', scale_exponential(@vector_tile_zoom,0,150,25.5,38.25,2))",
        )
        self.assertEqual(
            QgsMapBoxGlStyleConverter.parseInterpolateOpacityByZoom(
                {"base": 2, "stops": [[0, 0.1], [150, 0.1]]}, 255, conversion_context
            ).expressionString(),
            "set_color_part(@symbol_color, 'alpha', 25.5)",
        )

        self.assertEqual(
            QgsMapBoxGlStyleConverter.parseInterpolateOpacityByZoom(
                {
                    "base": 2,
                    "stops": [
                        [10, 0],
                        [11, ["match", ["get", "class"], ["path"], 0.5, 0]],
                        [13, ["match", ["get", "class"], ["path"], 1, 0.5]],
                    ],
                },
                255,
                conversion_context,
            ).expressionString(),
            (
                """CASE WHEN @vector_tile_zoom < 10 THEN set_color_part(@symbol_color, 'alpha', 0) """
                """WHEN @vector_tile_zoom >= 10 AND @vector_tile_zoom < 11 THEN set_color_part(@symbol_color, 'alpha', scale_exponential(@vector_tile_zoom,10,11,(0) * 255,(CASE WHEN "class" = 'path' THEN 0.5 ELSE 0 END) * 255,2)) """
                """WHEN @vector_tile_zoom >= 11 AND @vector_tile_zoom < 13 THEN set_color_part(@symbol_color, 'alpha', scale_exponential(@vector_tile_zoom,11,13,(CASE WHEN "class" = 'path' THEN 0.5 ELSE 0 END) * 255,(CASE WHEN "class" = 'path' THEN 1 ELSE 0.5 END) * 255,2)) """
                """WHEN @vector_tile_zoom >= 13 THEN set_color_part(@symbol_color, 'alpha', (CASE WHEN "class" = 'path' THEN 1 ELSE 0.5 END) * 255) END"""
            ),
        )

    def testInterpolateListByZoom(self):
        conversion_context = QgsMapBoxGlStyleConversionContext()
        prop, default_color, default_val = (
            QgsMapBoxGlStyleConverter.parseInterpolateListByZoom(
                ["interpolate", ["linear"], ["zoom"], 10, 0.1, 15, 0.3, 18, 0.6],
                QgsMapBoxGlStyleConverter.PropertyType.Opacity,
                conversion_context,
                2,
            )
        )
        self.assertEqual(
            prop.expressionString(),
            "CASE WHEN @vector_tile_zoom < 10 THEN set_color_part(@symbol_color, 'alpha', 25.5) WHEN @vector_tile_zoom >= 10 AND @vector_tile_zoom < 15 THEN set_color_part(@symbol_color, 'alpha', scale_linear(@vector_tile_zoom,10,15,25.5,76.5)) WHEN @vector_tile_zoom >= 15 AND @vector_tile_zoom < 18 THEN set_color_part(@symbol_color, 'alpha', scale_linear(@vector_tile_zoom,15,18,76.5,153)) WHEN @vector_tile_zoom >= 18 THEN set_color_part(@symbol_color, 'alpha', 153) END",
        )

        prop, default_color, default_val = (
            QgsMapBoxGlStyleConverter.parseInterpolateListByZoom(
                ["interpolate", ["linear"], ["zoom"], 10, 0.1, 15, 0.3, 18, 0.6],
                QgsMapBoxGlStyleConverter.PropertyType.Numeric,
                conversion_context,
                2,
            )
        )
        self.assertEqual(
            prop.expressionString(),
            "CASE WHEN @vector_tile_zoom >= 10 AND @vector_tile_zoom <= 15 THEN (scale_linear(@vector_tile_zoom,10,15,0.1,0.3)) * 2 WHEN @vector_tile_zoom > 15 AND @vector_tile_zoom <= 18 THEN (scale_linear(@vector_tile_zoom,15,18,0.3,0.6)) * 2 WHEN @vector_tile_zoom > 18 THEN 1.2 END",
        )
        self.assertEqual(default_val, 0.2)

        prop, default_color, default_val = (
            QgsMapBoxGlStyleConverter.parseInterpolateListByZoom(
                [
                    "interpolate",
                    ["exponential", 1.5],
                    ["zoom"],
                    5,
                    0,
                    6,
                    ["match", ["get", "class"], ["ice", "glacier"], 0.3, 0],
                    10,
                    ["match", ["get", "class"], ["ice", "glacier"], 0.2, 0],
                    11,
                    ["match", ["get", "class"], ["ice", "glacier"], 0.2, 0.3],
                    14,
                    ["match", ["get", "class"], ["ice", "glacier"], 0, 0.3],
                ],
                QgsMapBoxGlStyleConverter.PropertyType.Numeric,
                conversion_context,
                2,
            )
        )
        self.assertEqual(
            prop.expressionString(),
            (
                """CASE WHEN @vector_tile_zoom >= 5 AND @vector_tile_zoom <= 6 THEN (scale_exponential(@vector_tile_zoom,5,6,0,CASE WHEN "class" IN ('ice', 'glacier') THEN 0.3 ELSE 0 END,1.5)) * 2 """
                """WHEN @vector_tile_zoom > 6 AND @vector_tile_zoom <= 10 THEN (scale_exponential(@vector_tile_zoom,6,10,CASE WHEN "class" IN ('ice', 'glacier') THEN 0.3 ELSE 0 END,CASE WHEN "class" IN ('ice', 'glacier') THEN 0.2 ELSE 0 END,1.5)) * 2 """
                """WHEN @vector_tile_zoom > 10 AND @vector_tile_zoom <= 11 THEN (scale_exponential(@vector_tile_zoom,10,11,CASE WHEN "class" IN ('ice', 'glacier') THEN 0.2 ELSE 0 END,CASE WHEN "class" IN ('ice', 'glacier') THEN 0.2 ELSE 0.3 END,1.5)) * 2 """
                """WHEN @vector_tile_zoom > 11 AND @vector_tile_zoom <= 14 THEN (scale_exponential(@vector_tile_zoom,11,14,CASE WHEN "class" IN ('ice', 'glacier') THEN 0.2 ELSE 0.3 END,CASE WHEN "class" IN ('ice', 'glacier') THEN 0 ELSE 0.3 END,1.5)) * 2 """
                """WHEN @vector_tile_zoom > 14 THEN ( ( CASE WHEN "class" IN ('ice', 'glacier') THEN 0 ELSE 0.3 END ) * 2 ) END"""
            ),
        )

        prop, default_col, default_val = (
            QgsMapBoxGlStyleConverter.parseInterpolateListByZoom(
                [
                    "interpolate",
                    ["exponential", 1],
                    ["zoom"],
                    12,
                    [
                        "case",
                        ["==", ["%", ["to-number", ["get", "ele"]], 100], 0],
                        0.75,
                        0,
                    ],
                    13,
                    [
                        "case",
                        ["==", ["%", ["to-number", ["get", "ele"]], 100], 0],
                        0.75,
                        0,
                    ],
                    14,
                    [
                        "case",
                        ["==", ["%", ["to-number", ["get", "ele"]], 100], 0],
                        1,
                        0,
                    ],
                    14.5,
                    [
                        "case",
                        ["==", ["%", ["to-number", ["get", "ele"]], 100], 0],
                        1.5,
                        [
                            "case",
                            ["==", ["%", ["to-number", ["get", "ele"]], 20], 0],
                            0.75,
                            0,
                        ],
                    ],
                    15,
                    [
                        "case",
                        ["==", ["%", ["to-number", ["get", "ele"]], 100], 0],
                        1.75,
                        [
                            "case",
                            ["==", ["%", ["to-number", ["get", "ele"]], 20], 0],
                            1,
                            0,
                        ],
                    ],
                    16.5,
                    [
                        "case",
                        ["==", ["%", ["to-number", ["get", "ele"]], 100], 0],
                        2,
                        [
                            "case",
                            ["==", ["%", ["to-number", ["get", "ele"]], 10], 0],
                            1,
                            0,
                        ],
                    ],
                ],
                QgsMapBoxGlStyleConverter.PropertyType.Numeric,
                conversion_context,
                0.264583,
            )
        )
        self.assertEqual(
            prop.expressionString(),
            'CASE WHEN @vector_tile_zoom >= 12 AND @vector_tile_zoom <= 13 THEN (CASE WHEN (to_real("ele") % 100 IS 0) THEN 0.75 ELSE 0 END) * 0.264583 WHEN @vector_tile_zoom > 13 AND @vector_tile_zoom <= 14 THEN (scale_linear(@vector_tile_zoom,13,14,CASE WHEN (to_real("ele") % 100 IS 0) THEN 0.75 ELSE 0 END,CASE WHEN (to_real("ele") % 100 IS 0) THEN 1 ELSE 0 END)) * 0.264583 WHEN @vector_tile_zoom > 14 AND @vector_tile_zoom <= 14.5 THEN (scale_linear(@vector_tile_zoom,14,14.5,CASE WHEN (to_real("ele") % 100 IS 0) THEN 1 ELSE 0 END,CASE WHEN (to_real("ele") % 100 IS 0) THEN 1.5 ELSE CASE WHEN (to_real("ele") % 20 IS 0) THEN 0.75 ELSE 0 END END)) * 0.264583 WHEN @vector_tile_zoom > 14.5 AND @vector_tile_zoom <= 15 THEN (scale_linear(@vector_tile_zoom,14.5,15,CASE WHEN (to_real("ele") % 100 IS 0) THEN 1.5 ELSE CASE WHEN (to_real("ele") % 20 IS 0) THEN 0.75 ELSE 0 END END,CASE WHEN (to_real("ele") % 100 IS 0) THEN 1.75 ELSE CASE WHEN (to_real("ele") % 20 IS 0) THEN 1 ELSE 0 END END)) * 0.264583 WHEN @vector_tile_zoom > 15 AND @vector_tile_zoom <= 16.5 THEN (scale_linear(@vector_tile_zoom,15,16.5,CASE WHEN (to_real("ele") % 100 IS 0) THEN 1.75 ELSE CASE WHEN (to_real("ele") % 20 IS 0) THEN 1 ELSE 0 END END,CASE WHEN (to_real("ele") % 100 IS 0) THEN 2 ELSE CASE WHEN (to_real("ele") % 10 IS 0) THEN 1 ELSE 0 END END)) * 0.264583 WHEN @vector_tile_zoom > 16.5 THEN ( ( CASE WHEN (to_real("ele") % 100 IS 0) THEN 2 ELSE CASE WHEN (to_real("ele") % 10 IS 0) THEN 1 ELSE 0 END END ) * 0.264583 ) END',
        )

    def testParseExpression(self):
        conversion_context = QgsMapBoxGlStyleConversionContext()
        self.assertEqual(
            QgsMapBoxGlStyleConverter.parseExpression(
                [
                    "all",
                    ["==", ["get", "level"], 0],
                    ["match", ["get", "type"], ["Restricted"], True, False],
                ],
                conversion_context,
            ),
            """(level IS 0) AND ("type" = 'Restricted')""",
        )

        self.assertEqual(
            QgsMapBoxGlStyleConverter.parseExpression(
                ["match", ["get", "type"], ["Restricted"], True, False],
                conversion_context,
            ),
            """"type" = 'Restricted\'""",
        )

        self.assertEqual(
            QgsMapBoxGlStyleConverter.parseExpression(
                [
                    "match",
                    ["get", "type"],
                    ["Restricted"],
                    "r",
                    ["Local"],
                    "l",
                    ["Secondary", "Main"],
                    "m",
                    "n",
                ],
                conversion_context,
            ),
            """CASE WHEN "type" = 'Restricted' THEN 'r' WHEN "type" = 'Local' THEN 'l' WHEN "type" IN ('Secondary', 'Main') THEN 'm' ELSE 'n' END""",
        )

        self.assertEqual(
            QgsMapBoxGlStyleConverter.parseExpression(
                [
                    "all",
                    ["==", ["get", "level"], 0],
                    [
                        "match",
                        ["get", "type"],
                        ["Restricted", "Temporary"],
                        True,
                        False,
                    ],
                ],
                conversion_context,
            ),
            """(level IS 0) AND ("type" IN ('Restricted', 'Temporary'))""",
        )
        self.assertEqual(
            QgsMapBoxGlStyleConverter.parseExpression(
                [
                    "any",
                    ["match", ["get", "level"], [1], True, False],
                    ["match", ["get", "type"], ["Local"], True, False],
                ],
                conversion_context,
            ),
            """("level" = 1) OR ("type" = 'Local')""",
        )
        self.assertEqual(
            QgsMapBoxGlStyleConverter.parseExpression(
                [
                    "none",
                    ["match", ["get", "level"], [1], True, False],
                    ["match", ["get", "type"], ["Local"], True, False],
                ],
                conversion_context,
            ),
            """NOT ("level" = 1) AND NOT ("type" = 'Local')""",
        )
        self.assertEqual(
            QgsMapBoxGlStyleConverter.parseExpression(
                ["match", ["get", "type"], ["Primary", "Motorway"], False, True],
                conversion_context,
            ),
            """CASE WHEN "type" IN ('Primary', 'Motorway') THEN FALSE ELSE TRUE END""",
        )
        self.assertEqual(
            QgsMapBoxGlStyleConverter.parseExpression(
                ["==", "_symbol", 0], conversion_context
            ),
            """"_symbol" IS 0""",
        )

        self.assertEqual(
            QgsMapBoxGlStyleConverter.parseExpression(
                ["all", ["==", "_symbol", 8], ["!in", "Viz", 3]], conversion_context
            ),
            """("_symbol" IS 8) AND (("Viz" IS NULL OR "Viz" NOT IN (3)))""",
        )

        self.assertEqual(
            QgsMapBoxGlStyleConverter.parseExpression(
                ["get", "name"], conversion_context
            ),
            '''"name"''',
        )

        self.assertEqual(
            QgsMapBoxGlStyleConverter.parseExpression(
                ["to-boolean", ["get", "name"]], conversion_context
            ),
            """to_bool("name")""",
        )
        self.assertEqual(
            QgsMapBoxGlStyleConverter.parseExpression(
                ["to-string", ["get", "name"]], conversion_context
            ),
            """to_string("name")""",
        )
        self.assertEqual(
            QgsMapBoxGlStyleConverter.parseExpression(
                ["to-number", ["get", "elevation"]], conversion_context
            ),
            """to_real("elevation")""",
        )
        self.assertEqual(
            QgsMapBoxGlStyleConverter.parseExpression(
                ["%", 100, 20], conversion_context
            ),
            """100 % 20""",
        )
        self.assertEqual(
            QgsMapBoxGlStyleConverter.parseExpression(
                [
                    "match",
                    ["get", "subclass"],
                    "funicular",
                    "rgba(243,243,246,0)",
                    "rgb(243,243,246)",
                ],
                conversion_context,
                True,
            ),
            """CASE WHEN ("subclass" = 'funicular') THEN color_rgba(243,243,246,0) ELSE color_rgba(243,243,246,255) END""",
        )

        self.assertEqual(
            QgsMapBoxGlStyleConverter.parseExpression(
                ["case", ["==", ["%", ["to-number", ["get", "ele"]], 100], 0], 0.75, 0],
                conversion_context,
                False,
            ),
            """CASE WHEN (to_real("ele") % 100 IS 0) THEN 0.75 ELSE 0 END""",
        )

        self.assertEqual(
            QgsMapBoxGlStyleConverter.parseExpression(
                ["concat", ["get", "numero"], ["get", "indice_de_repetition"]],
                conversion_context,
                False,
            ),
            """concat("numero", "indice_de_repetition")""",
        )

        self.assertEqual(
            QgsMapBoxGlStyleConverter.parseExpression(
                [
                    "in",
                    ["get", "subclass"],
                    ["literal", ["allotments", "forest", "glacier"]],
                ],
                conversion_context,
                True,
            ),
            """"subclass" IN ('allotments', 'forest', 'glacier')""",
        )

        # fix last (default) value of a match can be a match
        self.assertEqual(
            QgsMapBoxGlStyleConverter.parseExpression(
                [
                    "match",
                    ["get", "is_route"],
                    [5, 10],
                    "hsl(16,91%,80%)",
                    [6, 7, 8],
                    "hsl(55,91%,80%)",
                    [
                        "match",
                        ["get", "class"],
                        [
                            "motorway",
                            "trunk",
                            "motorway_construction",
                            "trunk_construction",
                        ],
                        "hsl(41,93%,73%)",
                        [
                            "rail",
                            "rail_construction",
                            "path",
                            "path_construction",
                            "footway",
                            "footway_construction",
                            "track",
                            "track_construction",
                            "trail",
                            "trail_construction",
                        ],
                        [
                            "match",
                            ["get", "subclass"],
                            "covered_bridge",
                            "rgb(255,255,255)",
                            "rgb(238,238,240)",
                        ],
                        "rgba(255,255,255,1)",
                    ],
                ],
                conversion_context,
                True,
            ),
            """CASE WHEN "is_route" IN (5, 10) THEN color_rgba(250,182,158,255) WHEN "is_route" IN (6, 7, 8) THEN color_rgba(250,243,158,255) ELSE CASE WHEN "class" IN ('motorway', 'trunk', 'motorway_construction', 'trunk_construction') THEN color_rgba(250,210,122,255) WHEN "class" IN ('rail', 'rail_construction', 'path', 'path_construction', 'footway', 'footway_construction', 'track', 'track_construction', 'trail', 'trail_construction') THEN CASE WHEN ("subclass" = 'covered_bridge') THEN color_rgba(255,255,255,255) ELSE color_rgba(238,238,240,255) END ELSE color_rgba(255,255,255,255) END END""",
        )

        self.assertEqual(
            QgsMapBoxGlStyleConverter.parseExpression(
                [
                    "step",
                    ["zoom"],
                    "",
                    16,
                    [
                        "case",
                        ["has", "flstnrnen"],
                        ["concat", ["get", "flstnrzae"], "/", ["get", "flstnrnen"]],
                        ["get", "flstnrzae"],
                    ],
                ],
                conversion_context,
                True,
            ),
            """CASE  WHEN @vector_tile_zoom >= 16 THEN (CASE WHEN ("flstnrnen" IS NOT NULL) THEN concat("flstnrzae", '/', "flstnrnen") ELSE "flstnrzae" END) ELSE ('') END""",
        )

    def testConvertLabels(self):
        context = QgsMapBoxGlStyleConversionContext()
        style = {
            "layout": {
                "text-field": "{name_en}",
                "text-font": ["Open Sans Semibold", "Arial Unicode MS Bold"],
                "text-max-width": 8,
                "text-anchor": "top",
                "text-size": 11,
                "icon-size": 1,
            },
            "type": "symbol",
            "id": "poi_label",
            "paint": {
                "text-color": "#666",
                "text-halo-width": 1.5,
                "text-halo-color": "rgba(255,255,255,0.95)",
                "text-halo-blur": 1,
            },
            "source-layer": "poi_label",
        }
        renderer, has_renderer, labeling, has_labeling = (
            QgsMapBoxGlStyleConverter.parseSymbolLayer(style, context)
        )
        self.assertFalse(has_renderer)
        self.assertTrue(has_labeling)
        self.assertEqual(labeling.labelSettings().fieldName, "name_en")
        self.assertFalse(labeling.labelSettings().isExpression)

        style = {
            "layout": {
                "text-field": "name_en",
                "text-font": ["Open Sans Semibold", "Arial Unicode MS Bold"],
                "text-max-width": 8,
                "text-anchor": "top",
                "text-size": 11,
                "icon-size": 1,
            },
            "type": "symbol",
            "id": "poi_label",
            "paint": {
                "text-color": "#666",
                "text-halo-width": 1.5,
                "text-halo-color": "rgba(255,255,255,0.95)",
            },
            "source-layer": "poi_label",
        }
        renderer, has_renderer, labeling, has_labeling = (
            QgsMapBoxGlStyleConverter.parseSymbolLayer(style, context)
        )
        self.assertFalse(has_renderer)
        self.assertTrue(has_labeling)
        self.assertEqual(labeling.labelSettings().fieldName, "name_en")
        self.assertFalse(labeling.labelSettings().isExpression)

        style = {
            "layout": {
                "text-field": [
                    "format",
                    "foo",
                    {"font-scale": 1.2},
                    "bar",
                    {"font-scale": 0.8},
                ],
                "text-font": ["Open Sans Semibold", "Arial Unicode MS Bold"],
                "text-max-width": 8,
                "text-anchor": "top",
                "text-size": 11,
                "icon-size": 1,
            },
            "type": "symbol",
            "id": "poi_label",
            "paint": {
                "text-color": "#666",
                "text-halo-width": 1.5,
                "text-halo-color": "rgba(255,255,255,0.95)",
                "text-halo-blur": 1,
            },
            "source-layer": "poi_label",
        }
        renderer, has_renderer, labeling, has_labeling = (
            QgsMapBoxGlStyleConverter.parseSymbolLayer(style, context)
        )
        self.assertFalse(has_renderer)
        self.assertTrue(has_labeling)
        self.assertEqual(labeling.labelSettings().fieldName, 'concat("foo","bar")')
        self.assertTrue(labeling.labelSettings().isExpression)

        style = {
            "layout": {
                "text-field": "{name_en} - {name_fr}",
                "text-font": ["Open Sans Semibold", "Arial Unicode MS Bold"],
                "text-max-width": 8,
                "text-anchor": "top",
                "text-size": 11,
                "icon-size": 1,
            },
            "type": "symbol",
            "id": "poi_label",
            "paint": {
                "text-color": "#666",
                "text-halo-width": 1.5,
                "text-halo-color": "rgba(255,255,255,0.95)",
                "text-halo-blur": 1,
            },
            "source-layer": "poi_label",
        }
        renderer, has_renderer, labeling, has_labeling = (
            QgsMapBoxGlStyleConverter.parseSymbolLayer(style, context)
        )
        self.assertFalse(has_renderer)
        self.assertTrue(has_labeling)
        self.assertEqual(
            labeling.labelSettings().fieldName, """concat("name_en",' - ',"name_fr")"""
        )
        self.assertTrue(labeling.labelSettings().isExpression)

        style = {
            "layout": {
                "text-field": [
                    "format",
                    "{name_en} - {name_fr}",
                    {"font-scale": 1.2},
                    "bar",
                    {"font-scale": 0.8},
                ],
                "text-font": ["Open Sans Semibold", "Arial Unicode MS Bold"],
                "text-max-width": 8,
                "text-anchor": "top",
                "text-size": 11,
                "icon-size": 1,
            },
            "type": "symbol",
            "id": "poi_label",
            "paint": {
                "text-color": "#666",
                "text-halo-width": 1.5,
                "text-halo-color": "rgba(255,255,255,0.95)",
                "text-halo-blur": 1,
            },
            "source-layer": "poi_label",
        }
        renderer, has_renderer, labeling, has_labeling = (
            QgsMapBoxGlStyleConverter.parseSymbolLayer(style, context)
        )
        self.assertFalse(has_renderer)
        self.assertTrue(has_labeling)
        self.assertEqual(
            labeling.labelSettings().fieldName,
            """concat(concat("name_en",' - ',"name_fr"),"bar")""",
        )
        self.assertTrue(labeling.labelSettings().isExpression)

        style = {
            "layout": {
                "text-field": ["to-string", ["get", "name"]],
                "text-font": ["Open Sans Semibold", "Arial Unicode MS Bold"],
                "text-max-width": 8,
                "text-anchor": "top",
                "text-size": 11,
                "icon-size": 1,
            },
            "type": "symbol",
            "id": "poi_label",
            "paint": {
                "text-color": "#666",
                "text-halo-width": 1.5,
                "text-halo-color": "rgba(255,255,255,0.95)",
                "text-halo-blur": 1,
            },
            "source-layer": "poi_label",
        }
        renderer, has_renderer, labeling, has_labeling = (
            QgsMapBoxGlStyleConverter.parseSymbolLayer(style, context)
        )
        self.assertFalse(has_renderer)
        self.assertTrue(has_labeling)
        self.assertEqual(labeling.labelSettings().fieldName, """to_string("name")""")
        self.assertTrue(labeling.labelSettings().isExpression)

        # text-transform
        style = {
            "layout": {
                "text-field": "name_en",
                "text-font": ["Open Sans Semibold", "Arial Unicode MS Bold"],
                "text-transform": "uppercase",
                "text-max-width": 8,
                "text-anchor": "top",
                "text-size": 11,
                "icon-size": 1,
            },
            "type": "symbol",
            "id": "poi_label",
            "paint": {
                "text-color": "#666",
                "text-halo-width": 1.5,
                "text-halo-color": "rgba(255,255,255,0.95)",
                "text-halo-blur": 1,
            },
            "source-layer": "poi_label",
        }
        renderer, has_renderer, labeling, has_labeling = (
            QgsMapBoxGlStyleConverter.parseSymbolLayer(style, context)
        )
        self.assertFalse(has_renderer)
        self.assertTrue(has_labeling)
        self.assertEqual(labeling.labelSettings().fieldName, 'upper("name_en")')
        self.assertTrue(labeling.labelSettings().isExpression)

        style = {
            "layout": {
                "text-field": [
                    "format",
                    "{name_en} - {name_fr}",
                    {"font-scale": 1.2},
                    "bar",
                    {"font-scale": 0.8},
                ],
                "text-font": ["Open Sans Semibold", "Arial Unicode MS Bold"],
                "text-transform": "lowercase",
                "text-max-width": 8,
                "text-anchor": "top",
                "text-size": 11,
                "icon-size": 1,
            },
            "type": "symbol",
            "id": "poi_label",
            "paint": {
                "text-color": "#666",
                "text-halo-width": 1.5,
                "text-halo-color": "rgba(255,255,255,0.95)",
                "text-halo-blur": 1,
            },
            "source-layer": "poi_label",
        }
        renderer, has_renderer, labeling, has_labeling = (
            QgsMapBoxGlStyleConverter.parseSymbolLayer(style, context)
        )
        self.assertFalse(has_renderer)
        self.assertTrue(has_labeling)
        self.assertEqual(
            labeling.labelSettings().fieldName,
            """lower(concat(concat("name_en",' - ',"name_fr"),"bar"))""",
        )
        self.assertTrue(labeling.labelSettings().isExpression)

    def testHaloMaxSize(self):
        # text-halo-width is max 1/4 of font-size
        # https://docs.mapbox.com/style-spec/reference/layers/#paint-symbol-text-halo-width
        context = QgsMapBoxGlStyleConversionContext()

        # the pixel based text buffers appear larger when rendered on the web - so automatically scale
        # them up when converting to a QGIS style
        BUFFER_SIZE_SCALE = 2.0

        # (text_size, halo_size, expected_size, expected_data_defined)
        data = (
            (16, 3, 3, None),
            (16, 5, 4, None),
            (12, ["get", "some_field_1"], None, 'min(24/4, "some_field_1")'),
            (
                ["get", "some_field_2"],
                4,
                None,
                f'min("some_field_2"*{BUFFER_SIZE_SCALE:.0f}/4, {BUFFER_SIZE_SCALE * 4:.0f})',
            ),
            (
                ["get", "some_field_3"],
                ["get", "some_field_4"],
                None,
                f'min("some_field_3"*{BUFFER_SIZE_SCALE:.0f}/4, "some_field_4")',
            ),
        )

        for text_size, halo_size, expected_size, expected_data_defined in data:
            style = {
                "layout": {
                    "text-field": "name_en",
                    "text-font": ["Open Sans Semibold", "Arial Unicode MS Bold"],
                    "text-transform": "uppercase",
                    "text-max-width": 8,
                    "text-anchor": "top",
                    "text-size": text_size,
                    "icon-size": 1,
                },
                "type": "symbol",
                "id": "poi_label",
                "paint": {
                    "text-color": "#666",
                    "text-halo-width": halo_size,
                    "text-halo-color": "rgba(255,255,255,0.95)",
                    "text-halo-blur": 1,
                },
                "source-layer": "poi_label",
            }
            renderer, has_renderer, labeling, has_labeling = (
                QgsMapBoxGlStyleConverter.parseSymbolLayer(style, context)
            )
            self.assertFalse(has_renderer)
            self.assertTrue(has_labeling)
            if expected_size:
                f = labeling.labelSettings().format()
                buffer = f.buffer()
                self.assertEqual(buffer.size(), expected_size * BUFFER_SIZE_SCALE)
            if expected_data_defined:
                ls = labeling.labelSettings()
                dd = ls.dataDefinedProperties()
                self.assertEqual(
                    dd.property(QgsPalLayerSettings.Property.BufferSize).asExpression(),
                    expected_data_defined,
                )

    def testFontFamilyReplacement(self):
        context = QgsMapBoxGlStyleConversionContext()
        style = {
            "layout": {
                "text-field": "{name_en}",
                "text-font": ["not a font", "also not a font"],
                "text-max-width": 8,
                "text-anchor": "top",
                "text-size": 11,
                "icon-size": 1,
            },
            "type": "symbol",
            "id": "poi_label",
            "paint": {
                "text-color": "#666",
                "text-halo-width": 1.5,
                "text-halo-color": "rgba(255,255,255,0.95)",
                "text-halo-blur": 1,
            },
            "source-layer": "poi_label",
        }
        renderer, has_renderer, labeling, has_labeling = (
            QgsMapBoxGlStyleConverter.parseSymbolLayer(style, context)
        )
        self.assertFalse(has_renderer)
        self.assertTrue(has_labeling)
        test_font = getTestFont()
        self.assertNotEqual(
            labeling.labelSettings().format().font().family(), test_font.family()
        )

        # with a font replacement
        QgsApplication.fontManager().addFontFamilyReplacement(
            "not a font", test_font.family()
        )
        renderer, has_renderer, labeling, has_labeling = (
            QgsMapBoxGlStyleConverter.parseSymbolLayer(style, context)
        )
        self.assertFalse(has_renderer)
        self.assertTrue(has_labeling)
        self.assertEqual(
            labeling.labelSettings().format().font().family(), test_font.family()
        )

    def testDataDefinedIconRotate(self):
        """Test icon-rotate property that depends on a data attribute"""
        context = QgsMapBoxGlStyleConversionContext()

        image = QImage(QSize(1, 1), QImage.Format.Format_ARGB32)
        context.setSprites(
            image, {"foo": {"x": 0, "y": 0, "width": 1, "height": 1, "pixelRatio": 1}}
        )
        style = {
            "layout": {
                "icon-image": "{foo}",
                "icon-rotate": ["get", "ROTATION"],
                "text-size": 11,
                "icon-size": 1,
            },
            "type": "symbol",
            "id": "poi_label",
            "source-layer": "poi_label",
        }
        renderer, has_renderer, labeling, has_labeling = (
            QgsMapBoxGlStyleConverter.parseSymbolLayer(style, context)
        )
        self.assertTrue(has_renderer)
        self.assertFalse(has_labeling)
        dd_props = renderer.symbol().symbolLayers()[0].dataDefinedProperties()
        prop = dd_props.property(QgsSymbolLayer.Property.PropertyAngle)
        self.assertEqual(prop.asExpression(), '"ROTATION"')

    def testScaledIcon(self):
        """Test icon-size property that depends on a data attribute"""
        context = QgsMapBoxGlStyleConversionContext()

        image = QImage(QSize(1, 1), QImage.Format.Format_ARGB32)
        context.setSprites(
            image, {"foo": {"x": 0, "y": 0, "width": 2, "height": 2, "pixelRatio": 1}}
        )
        style = {
            "layout": {"icon-image": "{foo}", "text-size": 11, "icon-size": 2},
            "type": "symbol",
            "id": "poi_label",
            "source-layer": "poi_label",
        }
        renderer, has_renderer, labeling, has_labeling = (
            QgsMapBoxGlStyleConverter.parseSymbolLayer(style, context)
        )
        self.assertTrue(has_renderer)
        self.assertFalse(has_labeling)
        size = renderer.symbol().symbolLayers()[0].size()
        self.assertEqual(size, 4)

        image = QImage(QSize(1, 1), QImage.Format.Format_ARGB32)
        context.setSprites(
            image, {"foo": {"x": 0, "y": 0, "width": 2, "height": 2, "pixelRatio": 1}}
        )
        style = {
            "id": "landcover_pt",
            "type": "symbol",
            "source": "base_v1.0.0",
            "source-layer": "landcover_pt",
            "minzoom": 14.0,
            "layout": {
                "icon-size": [
                    "interpolate",
                    ["exponential", 1.6],
                    ["zoom"],
                    14,
                    0.2,
                    18,
                    1,
                ],
                "text-font": [],
                "icon-image": "{foo}",
                "visibility": "visible",
                "icon-allow-overlap": False,
                "icon-pitch-alignment": "map",
                "icon-ignore-placement": False,
                "icon-rotation-alignment": "map",
            },
            "paint": {"icon-opacity": {"stops": [[14, 0.4], [18, 0.6]]}},
        }
        renderer, has_renderer, labeling, has_labeling = (
            QgsMapBoxGlStyleConverter.parseSymbolLayer(style, context)
        )
        self.assertTrue(has_renderer)
        dd_properties = renderer.symbol().symbolLayers()[0].dataDefinedProperties()
        self.assertEqual(
            dd_properties.property(
                QgsSymbolLayer.Property.PropertyWidth
            ).asExpression(),
            """with_variable('marker_size',CASE WHEN "foo" = 'foo' THEN 2 END,(scale_exponential(@vector_tile_zoom,14,18,0.2,1,1.6))*@marker_size)""",
        )

        image = QImage(QSize(1, 1), QImage.Format.Format_ARGB32)
        context.setSprites(
            image,
            {"arrow_blue": {"x": 0, "y": 0, "width": 2, "height": 2, "pixelRatio": 1}},
        )
        style = {
            "id": "contour_line_pt_100",
            "type": "symbol",
            "source": "base_v1.0.0",
            "source-layer": "contour_line_pt",
            "minzoom": 13.0,
            "layout": {
                "icon-size": [
                    "interpolate",
                    ["linear"],
                    ["zoom"],
                    13,
                    0.6,
                    14,
                    0.7,
                    16,
                    0.9,
                ],
                "text-font": ["Frutiger Neue Italic"],
                "text-size": [
                    "interpolate",
                    ["exponential", 2],
                    ["zoom"],
                    13,
                    10,
                    14,
                    10.5,
                    16,
                    14,
                ],
                "icon-image": ["case", ["has", "lake_depth"], "arrow_blue", ""],
                "text-field": [
                    "case",
                    ["has", "lake_depth"],
                    ["get", "lake_depth"],
                    ["get", "ele"],
                ],
                "visibility": "visible",
                "icon-anchor": "center",
                "icon-offset": [
                    "case",
                    ["has", "lake_depth"],
                    ["literal", [-20, 0]],
                    ["literal", [0, 0]],
                ],
                "icon-rotate": ["get", "direction"],
                "text-anchor": "center",
                "text-rotate": ["get", "direction"],
                "text-padding": {"stops": [[13, 10], [16, 2]]},
                "symbol-spacing": 250,
                "text-max-angle": 35,
                "icon-keep-upright": True,
                "text-keep-upright": True,
                "symbol-avoid-edges": True,
                "text-letter-spacing": 0.1,
                "icon-pitch-alignment": "map",
                "icon-rotation-alignment": "map",
                "text-rotation-alignment": "map",
            },
        }
        renderer, has_renderer, labeling, has_labeling = (
            QgsMapBoxGlStyleConverter.parseSymbolLayer(style, context)
        )
        self.assertTrue(has_renderer)
        dd_properties = renderer.symbol().symbolLayers()[0].dataDefinedProperties()
        self.assertEqual(
            dd_properties.property(
                QgsSymbolLayer.Property.PropertyWidth
            ).asExpression(),
            """with_variable('marker_size',CASE WHEN "lake_depth" IS NOT NULL THEN 2 ELSE 2 END,(CASE WHEN @vector_tile_zoom >= 13 AND @vector_tile_zoom <= 14 THEN scale_linear(@vector_tile_zoom,13,14,0.6,0.7) WHEN @vector_tile_zoom > 14 AND @vector_tile_zoom <= 16 THEN scale_linear(@vector_tile_zoom,14,16,0.7,0.9) WHEN @vector_tile_zoom > 16 THEN 0.9 END)*@marker_size)""",
        )

    def testScaledLabelShieldIcon(self):
        """Test icon-size property for label shields that depends on a data attribute"""
        context = QgsMapBoxGlStyleConversionContext()

        image = QImage(QSize(1, 1), QImage.Format.Format_ARGB32)
        context.setSprites(
            image, {"foo": {"x": 0, "y": 0, "width": 2, "height": 2, "pixelRatio": 1}}
        )
        style = {
            "layout": {
                "visibility": "visible",
                "symbol-placement": "line",
                "text-field": "{texte}",
                "text-size": 11,
                "text-rotation-alignment": "viewport",
                "icon-rotation-alignment": "viewport",
                "icon-image": "{foo}",
                "icon-size": {"stops": [[13, 0.25], [16, 0.45], [17, 0.7]]},
            },
            "paint": {
                "text-color": "rgba(47, 47, 47, 1)",
            },
            "type": "symbol",
            "id": "poi_label",
            "source-layer": "poi_label",
        }
        renderer, has_renderer, labeling, has_labeling = (
            QgsMapBoxGlStyleConverter.parseSymbolLayer(style, context)
        )
        self.assertTrue(has_renderer)
        size = renderer.symbol().symbolLayers()[0].size()
        self.assertEqual(size, 0.5)
        dd_properties = renderer.symbol().symbolLayers()[0].dataDefinedProperties()
        self.assertEqual(
            dd_properties.property(
                QgsSymbolLayer.Property.PropertyWidth
            ).asExpression(),
            "with_variable('marker_size',CASE WHEN \"foo\" = 'foo' THEN 2 END,(CASE WHEN @vector_tile_zoom >= 13 AND @vector_tile_zoom <= 16 THEN scale_linear(@vector_tile_zoom,13,16,0.25,0.45) WHEN @vector_tile_zoom > 16 AND @vector_tile_zoom <= 17 THEN scale_linear(@vector_tile_zoom,16,17,0.45,0.7) WHEN @vector_tile_zoom > 17 THEN 0.7 END)*@marker_size)",
        )
        self.assertTrue(has_labeling)
        ls = labeling.labelSettings()
        tf = ls.format()
        self.assertEqual(tf.background().size(), QSizeF(1, 1))
        self.assertEqual(
            ls.dataDefinedProperties()
            .property(QgsPalLayerSettings.Property.ShapeSizeX)
            .asExpression(),
            "with_variable('marker_size',CASE WHEN \"foo\" = 'foo' THEN 2 END,(CASE WHEN @vector_tile_zoom >= 13 AND @vector_tile_zoom <= 16 THEN scale_linear(@vector_tile_zoom,13,16,0.25,0.45) WHEN @vector_tile_zoom > 16 AND @vector_tile_zoom <= 17 THEN scale_linear(@vector_tile_zoom,16,17,0.45,0.7) WHEN @vector_tile_zoom > 17 THEN 0.7 END)*@marker_size)",
        )

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
                "circle-translate": [11, 22],
            },
        }
        has_renderer, rendererStyle = QgsMapBoxGlStyleConverter.parseCircleLayer(
            style, context
        )
        self.assertTrue(has_renderer)
        self.assertEqual(
            rendererStyle.geometryType(), QgsWkbTypes.GeometryType.PointGeometry
        )
        properties = rendererStyle.symbol().symbolLayers()[0].properties()
        expected_properties = {
            "angle": "0",
            "cap_style": "square",
            "color": "22,22,22,153,rgb:0.08627450980392157,0.08627450980392157,0.08627450980392157,0.59999999999999998",
            "horizontal_anchor_point": "1",
            "joinstyle": "bevel",
            "name": "circle",
            "offset": "11,22",
            "offset_map_unit_scale": "3x:0,0,0,0,0,0",
            "offset_unit": "Pixel",
            "outline_color": "46,46,46,128,rgb:0.1803921568627451,0.1803921568627451,0.1803921568627451,0.50000762951094835",
            "outline_style": "solid",
            "outline_width": "3",
            "outline_width_map_unit_scale": "3x:0,0,0,0,0,0",
            "outline_width_unit": "Pixel",
            "scale_method": "diameter",
            "size": "66",
            "size_map_unit_scale": "3x:0,0,0,0,0,0",
            "size_unit": "Pixel",
            "vertical_anchor_point": "1",
        }
        self.assertEqual(properties, expected_properties)

    def testParseArrayStops(self):
        conversion_context = QgsMapBoxGlStyleConversionContext()
        exp = QgsMapBoxGlStyleConverter.parseArrayStops({}, conversion_context, 1)
        self.assertEqual(exp, "")

        exp = QgsMapBoxGlStyleConverter.parseArrayStops(
            [[0, [0, 1]], [2, [3, 4]]], conversion_context, 1
        )
        self.assertEqual(
            exp,
            "CASE WHEN @vector_tile_zoom <= 2 THEN array(0,1) WHEN @vector_tile_zoom > 2 THEN array(3,4) END",
        )

        exp = QgsMapBoxGlStyleConverter.parseArrayStops(
            [[0, [0, 1]], [2, [3, 4]]], conversion_context, 2
        )
        self.assertEqual(
            exp,
            "CASE WHEN @vector_tile_zoom <= 2 THEN array(0,2) WHEN @vector_tile_zoom > 2 THEN array(6,8) END",
        )

    def testParseLineDashArray(self):
        conversion_context = QgsMapBoxGlStyleConversionContext()
        style = {
            "id": "water line (intermittent)/river",
            "type": "line",
            "source": "esri",
            "source-layer": "water line (intermittent)",
            "filter": ["==", "_symbol", 3],
            "minzoom": 10,
            "layout": {"line-join": "round"},
            "paint": {
                "line-color": "#aad3df",
                "line-dasharray": {"stops": [[10, [1, 1]], [17, [0.3, 0.2]]]},
                "line-width": {
                    "base": 1.2,
                    "stops": [
                        [10, 1.5],
                        [11, 2],
                        [12, 3],
                        [13, 5],
                        [14, 6],
                        [16, 10],
                        [17, 12],
                    ],
                },
            },
        }
        has_renderer, rendererStyle = QgsMapBoxGlStyleConverter.parseLineLayer(
            style, conversion_context
        )
        self.assertTrue(has_renderer)
        self.assertEqual(
            rendererStyle.geometryType(), QgsWkbTypes.GeometryType.LineGeometry
        )
        self.assertTrue(rendererStyle.symbol()[0].useCustomDashPattern())
        dd_properties = rendererStyle.symbol().symbolLayers()[0].dataDefinedProperties()
        self.assertEqual(
            dd_properties.property(
                QgsSymbolLayer.Property.PropertyStrokeWidth
            ).asExpression(),
            (
                "CASE WHEN @vector_tile_zoom >= 10 AND @vector_tile_zoom <= 11 THEN scale_exponential(@vector_tile_zoom,10,11,1.5,2,1.2) "
                "WHEN @vector_tile_zoom > 11 AND @vector_tile_zoom <= 12 THEN scale_exponential(@vector_tile_zoom,11,12,2,3,1.2) "
                "WHEN @vector_tile_zoom > 12 AND @vector_tile_zoom <= 13 THEN scale_exponential(@vector_tile_zoom,12,13,3,5,1.2) "
                "WHEN @vector_tile_zoom > 13 AND @vector_tile_zoom <= 14 THEN scale_exponential(@vector_tile_zoom,13,14,5,6,1.2) "
                "WHEN @vector_tile_zoom > 14 AND @vector_tile_zoom <= 16 THEN scale_exponential(@vector_tile_zoom,14,16,6,10,1.2) "
                "WHEN @vector_tile_zoom > 16 AND @vector_tile_zoom <= 17 THEN scale_exponential(@vector_tile_zoom,16,17,10,12,1.2) "
                "WHEN @vector_tile_zoom > 17 THEN 12 END"
            ),
        )
        self.assertEqual(
            dd_properties.property(
                QgsSymbolLayer.Property.PropertyCustomDash
            ).asExpression(),
            (
                "array_to_string(array_foreach("
                "CASE WHEN @vector_tile_zoom <= 17 THEN array(1,1) WHEN @vector_tile_zoom > 17 THEN array(0.3,0.2) END,"
                "@element * ("
                "CASE WHEN @vector_tile_zoom >= 10 AND @vector_tile_zoom <= 11 THEN scale_exponential(@vector_tile_zoom,10,11,1.5,2,1.2) "
                "WHEN @vector_tile_zoom > 11 AND @vector_tile_zoom <= 12 THEN scale_exponential(@vector_tile_zoom,11,12,2,3,1.2) "
                "WHEN @vector_tile_zoom > 12 AND @vector_tile_zoom <= 13 THEN scale_exponential(@vector_tile_zoom,12,13,3,5,1.2) "
                "WHEN @vector_tile_zoom > 13 AND @vector_tile_zoom <= 14 THEN scale_exponential(@vector_tile_zoom,13,14,5,6,1.2) "
                "WHEN @vector_tile_zoom > 14 AND @vector_tile_zoom <= 16 THEN scale_exponential(@vector_tile_zoom,14,16,6,10,1.2) "
                "WHEN @vector_tile_zoom > 16 AND @vector_tile_zoom <= 17 THEN scale_exponential(@vector_tile_zoom,16,17,10,12,1.2) "
                "WHEN @vector_tile_zoom > 17 THEN 12 END)), ';')"
            ),
        )

    def testParseLineDashArrayOddNumber(self):
        conversion_context = QgsMapBoxGlStyleConversionContext()
        style = {
            "id": "water line (intermittent)/river",
            "type": "line",
            "source": "esri",
            "source-layer": "water line (intermittent)",
            "filter": ["==", "_symbol", 3],
            "minzoom": 10,
            "layout": {"line-join": "round"},
            "paint": {
                "line-color": "#aad3df",
                "line-dasharray": [1, 2, 3],
                "line-width": {
                    "base": 1.2,
                    "stops": [
                        [10, 1.5],
                        [11, 2],
                        [12, 3],
                        [13, 5],
                        [14, 6],
                        [16, 10],
                        [17, 12],
                    ],
                },
            },
        }
        has_renderer, rendererStyle = QgsMapBoxGlStyleConverter.parseLineLayer(
            style, conversion_context
        )
        self.assertTrue(has_renderer)
        self.assertEqual(
            rendererStyle.geometryType(), QgsWkbTypes.GeometryType.LineGeometry
        )
        self.assertTrue(rendererStyle.symbol()[0].useCustomDashPattern())
        self.assertEqual(rendererStyle.symbol()[0].customDashVector(), [6.0, 3.0])
        dd_properties = rendererStyle.symbol().symbolLayers()[0].dataDefinedProperties()
        self.assertEqual(
            dd_properties.property(
                QgsSymbolLayer.Property.PropertyStrokeWidth
            ).asExpression(),
            (
                "CASE WHEN @vector_tile_zoom >= 10 AND @vector_tile_zoom <= 11 THEN scale_exponential(@vector_tile_zoom,10,11,1.5,2,1.2) "
                "WHEN @vector_tile_zoom > 11 AND @vector_tile_zoom <= 12 THEN scale_exponential(@vector_tile_zoom,11,12,2,3,1.2) "
                "WHEN @vector_tile_zoom > 12 AND @vector_tile_zoom <= 13 THEN scale_exponential(@vector_tile_zoom,12,13,3,5,1.2) "
                "WHEN @vector_tile_zoom > 13 AND @vector_tile_zoom <= 14 THEN scale_exponential(@vector_tile_zoom,13,14,5,6,1.2) "
                "WHEN @vector_tile_zoom > 14 AND @vector_tile_zoom <= 16 THEN scale_exponential(@vector_tile_zoom,14,16,6,10,1.2) "
                "WHEN @vector_tile_zoom > 16 AND @vector_tile_zoom <= 17 THEN scale_exponential(@vector_tile_zoom,16,17,10,12,1.2) "
                "WHEN @vector_tile_zoom > 17 THEN 12 END"
            ),
        )
        self.assertEqual(
            dd_properties.property(
                QgsSymbolLayer.Property.PropertyCustomDash
            ).asExpression(),
            (
                "array_to_string(array_foreach(array(4,2),@element * ("
                "CASE WHEN @vector_tile_zoom >= 10 AND @vector_tile_zoom <= 11 THEN scale_exponential(@vector_tile_zoom,10,11,1.5,2,1.2) "
                "WHEN @vector_tile_zoom > 11 AND @vector_tile_zoom <= 12 THEN scale_exponential(@vector_tile_zoom,11,12,2,3,1.2) "
                "WHEN @vector_tile_zoom > 12 AND @vector_tile_zoom <= 13 THEN scale_exponential(@vector_tile_zoom,12,13,3,5,1.2) "
                "WHEN @vector_tile_zoom > 13 AND @vector_tile_zoom <= 14 THEN scale_exponential(@vector_tile_zoom,13,14,5,6,1.2) "
                "WHEN @vector_tile_zoom > 14 AND @vector_tile_zoom <= 16 THEN scale_exponential(@vector_tile_zoom,14,16,6,10,1.2) "
                "WHEN @vector_tile_zoom > 16 AND @vector_tile_zoom <= 17 THEN scale_exponential(@vector_tile_zoom,16,17,10,12,1.2) "
                "WHEN @vector_tile_zoom > 17 THEN 12 END)), ';')"
            ),
        )

    def testParseLineDashArraySingleNumber(self):
        conversion_context = QgsMapBoxGlStyleConversionContext()
        style = {
            "id": "water line (intermittent)/river",
            "type": "line",
            "source": "esri",
            "source-layer": "water line (intermittent)",
            "filter": ["==", "_symbol", 3],
            "minzoom": 10,
            "layout": {"line-join": "round"},
            "paint": {
                "line-color": "#aad3df",
                "line-dasharray": [3],
                "line-width": {
                    "base": 1.2,
                    "stops": [
                        [10, 1.5],
                        [11, 2],
                        [12, 3],
                        [13, 5],
                        [14, 6],
                        [16, 10],
                        [17, 12],
                    ],
                },
            },
        }
        has_renderer, rendererStyle = QgsMapBoxGlStyleConverter.parseLineLayer(
            style, conversion_context
        )
        self.assertTrue(has_renderer)
        self.assertEqual(
            rendererStyle.geometryType(), QgsWkbTypes.GeometryType.LineGeometry
        )
        self.assertFalse(rendererStyle.symbol()[0].useCustomDashPattern())
        dd_properties = rendererStyle.symbol().symbolLayers()[0].dataDefinedProperties()
        self.assertEqual(
            dd_properties.property(
                QgsSymbolLayer.Property.PropertyStrokeWidth
            ).asExpression(),
            "CASE WHEN @vector_tile_zoom >= 10 AND @vector_tile_zoom <= 11 THEN scale_exponential(@vector_tile_zoom,10,11,1.5,2,1.2) WHEN @vector_tile_zoom > 11 AND @vector_tile_zoom <= 12 THEN scale_exponential(@vector_tile_zoom,11,12,2,3,1.2) WHEN @vector_tile_zoom > 12 AND @vector_tile_zoom <= 13 THEN scale_exponential(@vector_tile_zoom,12,13,3,5,1.2) WHEN @vector_tile_zoom > 13 AND @vector_tile_zoom <= 14 THEN scale_exponential(@vector_tile_zoom,13,14,5,6,1.2) WHEN @vector_tile_zoom > 14 AND @vector_tile_zoom <= 16 THEN scale_exponential(@vector_tile_zoom,14,16,6,10,1.2) WHEN @vector_tile_zoom > 16 AND @vector_tile_zoom <= 17 THEN scale_exponential(@vector_tile_zoom,16,17,10,12,1.2) WHEN @vector_tile_zoom > 17 THEN 12 END",
        )
        self.assertFalse(
            dd_properties.property(
                QgsSymbolLayer.Property.PropertyCustomDash
            ).isActive()
        )

    def testParseLineDashArrayLiteral(self):
        conversion_context = QgsMapBoxGlStyleConversionContext()
        style = {
            "id": "tunnel_public_transport",
            "type": "line",
            "source": "base_v1.0.0",
            "source-layer": "transportation",
            "minzoom": 8.0,
            "layout": {
                "line-cap": "butt",
                "line-join": "miter",
                "visibility": "visible",
            },
            "paint": {
                "line-blur": 0.4,
                "line-color": "hsl(0,80%,60%)",
                "line-width": [
                    "interpolate",
                    ["linear"],
                    ["zoom"],
                    8,
                    0.5,
                    10,
                    1.2,
                    12,
                    [
                        "match",
                        ["get", "class"],
                        ["rail"],
                        [
                            "match",
                            ["get", "subclass"],
                            ["rail", "narrow_gauge", "rack_rail"],
                            ["match", ["get", "service"], ["yard", "siding"], 0.25, 1],
                            1,
                        ],
                        1,
                    ],
                    14,
                    [
                        "match",
                        ["get", "class"],
                        ["rail", "rail_construction"],
                        [
                            "match",
                            ["get", "subclass"],
                            ["rail", "narrow_gauge", "rack_rail"],
                            [
                                "match",
                                ["get", "service"],
                                ["yard", "siding"],
                                0.25,
                                1.5,
                            ],
                            1.5,
                        ],
                        1.5,
                    ],
                    18,
                    [
                        "match",
                        ["get", "class"],
                        ["rail", "rail_construction"],
                        [
                            "match",
                            ["get", "subclass"],
                            ["rail", "narrow_gauge", "rack_rail"],
                            ["match", ["get", "service"], ["yard", "siding"], 1, 2],
                            1.5,
                        ],
                        1.5,
                    ],
                ],
                "line-opacity": [
                    "interpolate",
                    ["linear"],
                    ["zoom"],
                    8,
                    0,
                    8.5,
                    ["match", ["get", "class"], ["rail"], 1, 0],
                    13,
                    [
                        "match",
                        ["get", "subclass"],
                        ["rail", "subway", "funicular", "narrow_gauge", "rack_rail"],
                        ["match", ["get", "is_route"], 99, 1, 0],
                        0,
                    ],
                    14,
                    [
                        "match",
                        ["get", "class"],
                        ["rail_construction", "transit_construction"],
                        0.8,
                        [
                            "match",
                            ["get", "subclass"],
                            [
                                "rail",
                                "narrow_gauge",
                                "funicular",
                                "subway",
                                "rack_rail",
                            ],
                            ["match", ["get", "service"], ["yard", "siding"], 0, 1],
                            0,
                        ],
                    ],
                    14.5,
                    [
                        "match",
                        ["get", "class"],
                        ["rail_construction", "transit_construction"],
                        0.8,
                        1,
                    ],
                ],
                "line-dasharray": [
                    "step",
                    ["zoom"],
                    ["literal", [3, 1.875]],
                    14,
                    ["literal", [4, 2.5]],
                    15,
                    ["literal", [5, 3.125]],
                    16,
                    ["literal", [6, 3.75]],
                ],
            },
            "filter": [
                "all",
                ["==", ["get", "brunnel"], "tunnel"],
                [
                    "in",
                    ["get", "class"],
                    [
                        "literal",
                        [
                            "cable_car",
                            "gondola",
                            "rail",
                            "rail_construction",
                            "transit",
                            "transit_construction",
                        ],
                    ],
                ],
                ["==", ["geometry-type"], "LineString"],
            ],
        }
        has_renderer, rendererStyle = QgsMapBoxGlStyleConverter.parseLineLayer(
            style, conversion_context
        )
        self.assertTrue(has_renderer)
        self.assertEqual(
            rendererStyle.geometryType(), QgsWkbTypes.GeometryType.LineGeometry
        )
        self.assertFalse(rendererStyle.symbol()[0].useCustomDashPattern())
        dd_properties = rendererStyle.symbol().symbolLayers()[0].dataDefinedProperties()
        self.assertEqual(
            dd_properties.property(QgsSymbolLayer.Property.CustomDash).asExpression(),
            """array_to_string(array_foreach(CASE  WHEN @vector_tile_zoom >= 16 THEN (array(6,3.75))  WHEN @vector_tile_zoom >= 15 THEN (array(5,3.125))  WHEN @vector_tile_zoom >= 14 THEN (array(4,2.5)) ELSE (array(3,1.875)) END,@element * (CASE WHEN @vector_tile_zoom >= 8 AND @vector_tile_zoom <= 10 THEN scale_linear(@vector_tile_zoom,8,10,0.5,1.2) WHEN @vector_tile_zoom > 10 AND @vector_tile_zoom <= 12 THEN scale_linear(@vector_tile_zoom,10,12,1.2,CASE WHEN "class" = 'rail' THEN CASE WHEN "subclass" IN ('rail', 'narrow_gauge', 'rack_rail') THEN CASE WHEN "service" IN ('yard', 'siding') THEN 0.25 ELSE 1 END ELSE 1 END ELSE 1 END) WHEN @vector_tile_zoom > 12 AND @vector_tile_zoom <= 14 THEN scale_linear(@vector_tile_zoom,12,14,CASE WHEN "class" = 'rail' THEN CASE WHEN "subclass" IN ('rail', 'narrow_gauge', 'rack_rail') THEN CASE WHEN "service" IN ('yard', 'siding') THEN 0.25 ELSE 1 END ELSE 1 END ELSE 1 END,CASE WHEN "class" IN ('rail', 'rail_construction') THEN CASE WHEN "subclass" IN ('rail', 'narrow_gauge', 'rack_rail') THEN CASE WHEN "service" IN ('yard', 'siding') THEN 0.25 ELSE 1.5 END ELSE 1.5 END ELSE 1.5 END) WHEN @vector_tile_zoom > 14 AND @vector_tile_zoom <= 18 THEN scale_linear(@vector_tile_zoom,14,18,CASE WHEN "class" IN ('rail', 'rail_construction') THEN CASE WHEN "subclass" IN ('rail', 'narrow_gauge', 'rack_rail') THEN CASE WHEN "service" IN ('yard', 'siding') THEN 0.25 ELSE 1.5 END ELSE 1.5 END ELSE 1.5 END,CASE WHEN "class" IN ('rail', 'rail_construction') THEN CASE WHEN "subclass" IN ('rail', 'narrow_gauge', 'rack_rail') THEN CASE WHEN "service" IN ('yard', 'siding') THEN 1 ELSE 2 END ELSE 1.5 END ELSE 1.5 END) WHEN @vector_tile_zoom > 18 THEN ( ( CASE WHEN "class" IN ('rail', 'rail_construction') THEN CASE WHEN "subclass" IN ('rail', 'narrow_gauge', 'rack_rail') THEN CASE WHEN "service" IN ('yard', 'siding') THEN 1 ELSE 2 END ELSE 1.5 END ELSE 1.5 END ) * 1 ) END)), ';')""",
        )
        self.assertTrue(
            dd_properties.property(
                QgsSymbolLayer.Property.PropertyCustomDash
            ).isActive()
        )

        conversion_context = QgsMapBoxGlStyleConversionContext()
        style["paint"].pop("line-width")
        has_renderer, rendererStyle = QgsMapBoxGlStyleConverter.parseLineLayer(
            style, conversion_context
        )
        self.assertTrue(has_renderer)
        self.assertEqual(
            rendererStyle.geometryType(), QgsWkbTypes.GeometryType.LineGeometry
        )
        self.assertFalse(rendererStyle.symbol()[0].useCustomDashPattern())
        dd_properties = rendererStyle.symbol().symbolLayers()[0].dataDefinedProperties()
        self.assertEqual(
            dd_properties.property(
                QgsSymbolLayer.Property.PropertyStrokeWidth
            ).asExpression(),
            "",
        )
        self.assertEqual(
            dd_properties.property(QgsSymbolLayer.Property.CustomDash).asExpression(),
            """array_to_string(CASE  WHEN @vector_tile_zoom >= 16 THEN (array(6,3.75))  WHEN @vector_tile_zoom >= 15 THEN (array(5,3.125))  WHEN @vector_tile_zoom >= 14 THEN (array(4,2.5)) ELSE (array(3,1.875)) END, ';')""",
        )
        self.assertTrue(
            dd_properties.property(
                QgsSymbolLayer.Property.PropertyCustomDash
            ).isActive()
        )

    def testParseLineNoWidth(self):
        conversion_context = QgsMapBoxGlStyleConversionContext()
        style = {
            "id": "water line (intermittent)/river",
            "type": "line",
            "source": "esri",
            "source-layer": "water line (intermittent)",
            "paint": {
                "line-color": "#aad3df",
            },
        }
        has_renderer, rendererStyle = QgsMapBoxGlStyleConverter.parseLineLayer(
            style, conversion_context
        )
        self.assertTrue(has_renderer)
        self.assertEqual(
            rendererStyle.geometryType(), QgsWkbTypes.GeometryType.LineGeometry
        )
        self.assertEqual(rendererStyle.symbol()[0].width(), 1.0)

        conversion_context.setPixelSizeConversionFactor(0.5)
        has_renderer, rendererStyle = QgsMapBoxGlStyleConverter.parseLineLayer(
            style, conversion_context
        )
        self.assertTrue(has_renderer)
        self.assertEqual(
            rendererStyle.geometryType(), QgsWkbTypes.GeometryType.LineGeometry
        )
        self.assertEqual(rendererStyle.symbol()[0].width(), 0.5)

    def testLinePattern(self):
        """Test line-pattern property"""
        context = QgsMapBoxGlStyleConversionContext()

        image = QImage(QSize(1, 1), QImage.Format.Format_ARGB32)
        context.setSprites(
            image, {"foo": {"x": 0, "y": 0, "width": 1, "height": 1, "pixelRatio": 1}}
        )
        style = {
            "id": "mountain range/ridge",
            "type": "line",
            "source": "esri",
            "source-layer": "mountain range",
            "filter": ["==", "_symbol", 1],
            "minzoom": 13,
            "layout": {"line-join": "round"},
            "paint": {
                "line-pattern": {"stops": [[13, "foo"], [15, "foo"]]},
                "line-width": {"stops": [[14, 20], [15, 40]]},
            },
        }
        has_renderer, rendererStyle = QgsMapBoxGlStyleConverter.parseLineLayer(
            style, context
        )
        self.assertTrue(has_renderer)
        self.assertEqual(
            rendererStyle.geometryType(), QgsWkbTypes.GeometryType.LineGeometry
        )
        self.assertEqual(
            rendererStyle.symbol().symbolLayers()[0].layerType(), "RasterLine"
        )
        dd_props = rendererStyle.symbol().symbolLayers()[0].dataDefinedProperties()
        prop = dd_props.property(QgsSymbolLayer.Property.PropertyFile)
        self.assertTrue(prop.isActive())

    def testParseLineOpactity(self):
        conversion_context = QgsMapBoxGlStyleConversionContext()
        style = {
            "id": "contour_line",
            "type": "line",
            "source": "base_v1.0.0",
            "source-layer": "contour_line",
            "minzoom": 11.0,
            "layout": {"visibility": "visible"},
            "paint": {
                "line-blur": 0.25,
                "line-color": [
                    "match",
                    ["get", "class"],
                    "scree",
                    "rgba(0, 0, 0, 1)",
                    "hsl(35, 86%, 38%)",
                ],
                "line-width": [
                    "interpolate",
                    ["exponential", 1],
                    ["zoom"],
                    11,
                    [
                        "case",
                        ["==", ["%", ["to-number", ["get", "ele"]], 100], 0],
                        0.75,
                        0,
                    ],
                    12,
                    [
                        "case",
                        ["==", ["%", ["to-number", ["get", "ele"]], 100], 0],
                        1,
                        0,
                    ],
                    14,
                    [
                        "case",
                        ["==", ["%", ["to-number", ["get", "ele"]], 100], 0],
                        1.5,
                        [
                            "case",
                            ["==", ["%", ["to-number", ["get", "ele"]], 20], 0],
                            0.75,
                            0,
                        ],
                    ],
                    15,
                    [
                        "case",
                        ["==", ["%", ["to-number", ["get", "ele"]], 100], 0],
                        2,
                        [
                            "case",
                            ["==", ["%", ["to-number", ["get", "ele"]], 20], 0],
                            1,
                            0,
                        ],
                    ],
                    18,
                    [
                        "case",
                        ["==", ["%", ["to-number", ["get", "ele"]], 100], 0],
                        3,
                        [
                            "case",
                            ["==", ["%", ["to-number", ["get", "ele"]], 10], 0],
                            1.5,
                            0,
                        ],
                    ],
                ],
                "line-opacity": [
                    "interpolate",
                    ["exponential", 1],
                    ["zoom"],
                    11,
                    ["match", ["get", "class"], "scree", 0.2, 0.3],
                    16,
                    ["match", ["get", "class"], "scree", 0.2, 0.3],
                ],
            },
            "filter": ["all", ["!in", "class", "rock", "ice", "water"]],
        }

        has_renderer, rendererStyle = QgsMapBoxGlStyleConverter.parseLineLayer(
            style, conversion_context
        )
        self.assertTrue(has_renderer)
        self.assertEqual(
            rendererStyle.geometryType(), QgsWkbTypes.GeometryType.LineGeometry
        )
        symbol = rendererStyle.symbol()
        dd_properties_layer = symbol.symbolLayers()[0].dataDefinedProperties()
        self.assertEqual(
            dd_properties_layer.property(
                QgsSymbolLayer.Property.StrokeColor
            ).asExpression(),
            """CASE WHEN "class" IS 'scree' THEN '#000000' ELSE '#b26e0e' END""",
        )
        dd_properties = symbol.dataDefinedProperties()
        self.assertEqual(
            dd_properties.property(QgsSymbol.Property.Opacity).asExpression(),
            """(CASE WHEN ("class" = 'scree') THEN 0.2 ELSE 0.3 END) * 100""",
        )

    def testLabelWithLiteral(self):
        context = QgsMapBoxGlStyleConversionContext()
        style = {
            "layout": {
                "visibility": "visible",
                "text-field": "Quarry {substance}",
            },
            "paint": {
                "text-color": "rgba(47, 47, 47, 1)",
            },
            "type": "symbol",
        }
        rendererStyle, has_renderer, labeling_style, has_labeling = (
            QgsMapBoxGlStyleConverter.parseSymbolLayer(style, context)
        )
        self.assertTrue(has_labeling)
        self.assertTrue(labeling_style.labelSettings().isExpression)
        self.assertEqual(
            labeling_style.labelSettings().fieldName, "concat('Quarry ',\"substance\")"
        )

    def testLabelWithLiteral2(self):
        context = QgsMapBoxGlStyleConversionContext()
        style = {
            "layout": {
                "visibility": "visible",
                "text-field": "{substance} Quarry",
            },
            "paint": {
                "text-color": "rgba(47, 47, 47, 1)",
            },
            "type": "symbol",
        }
        rendererStyle, has_renderer, labeling_style, has_labeling = (
            QgsMapBoxGlStyleConverter.parseSymbolLayer(style, context)
        )
        self.assertTrue(has_labeling)
        self.assertTrue(labeling_style.labelSettings().isExpression)
        self.assertEqual(
            labeling_style.labelSettings().fieldName, "concat(\"substance\",' Quarry')"
        )

    def testLabelWithLiteral3(self):
        context = QgsMapBoxGlStyleConversionContext()
        style = {
            "layout": {
                "visibility": "visible",
                "text-field": "A {substance} Quarry",
            },
            "paint": {
                "text-color": "rgba(47, 47, 47, 1)",
            },
            "type": "symbol",
        }
        rendererStyle, has_renderer, labeling_style, has_labeling = (
            QgsMapBoxGlStyleConverter.parseSymbolLayer(style, context)
        )
        self.assertTrue(has_labeling)
        self.assertTrue(labeling_style.labelSettings().isExpression)
        self.assertEqual(
            labeling_style.labelSettings().fieldName,
            "concat('A ',\"substance\",' Quarry')",
        )

    def testLabelWithField(self):
        context = QgsMapBoxGlStyleConversionContext()
        style = {
            "layout": {
                "visibility": "visible",
                "text-field": "{substance}",
            },
            "paint": {
                "text-color": "rgba(47, 47, 47, 1)",
            },
            "type": "symbol",
        }
        rendererStyle, has_renderer, labeling_style, has_labeling = (
            QgsMapBoxGlStyleConverter.parseSymbolLayer(style, context)
        )
        self.assertTrue(has_labeling)
        self.assertFalse(labeling_style.labelSettings().isExpression)
        self.assertEqual(labeling_style.labelSettings().fieldName, "substance")

    def testLabelRotation(self):
        context = QgsMapBoxGlStyleConversionContext()
        style = {
            "layout": {
                "visibility": "visible",
                "text-field": "{substance}",
                "text-rotate": 123,
            },
            "paint": {
                "text-color": "rgba(47, 47, 47, 1)",
            },
            "type": "symbol",
        }
        rendererStyle, has_renderer, labeling_style, has_labeling = (
            QgsMapBoxGlStyleConverter.parseSymbolLayer(style, context)
        )
        self.assertTrue(has_labeling)
        self.assertEqual(labeling_style.labelSettings().angleOffset, 123)

        context = QgsMapBoxGlStyleConversionContext()
        style = {
            "layout": {
                "visibility": "visible",
                "text-field": "{substance}",
                "text-rotate": ["get", "direction"],
            },
            "paint": {
                "text-color": "rgba(47, 47, 47, 1)",
            },
            "type": "symbol",
        }
        rendererStyle, has_renderer, labeling_style, has_labeling = (
            QgsMapBoxGlStyleConverter.parseSymbolLayer(style, context)
        )
        self.assertTrue(has_labeling)
        ls = labeling_style.labelSettings()
        ddp = ls.dataDefinedProperties()
        self.assertEqual(
            ddp.property(QgsPalLayerSettings.Property.LabelRotation).asExpression(),
            '"direction"',
        )

        context = QgsMapBoxGlStyleConversionContext()
        style = {
            "layout": {
                "visibility": "visible",
                "text-field": "{substance}",
                "text-rotate": {
                    "property": "label_angle1",
                    "default": 0,
                    "type": "identity",
                },
            },
            "paint": {
                "text-color": "rgba(47, 47, 47, 1)",
            },
            "type": "symbol",
        }
        rendererStyle, has_renderer, labeling_style, has_labeling = (
            QgsMapBoxGlStyleConverter.parseSymbolLayer(style, context)
        )
        self.assertTrue(has_labeling)
        ls = labeling_style.labelSettings()
        ddp = ls.dataDefinedProperties()
        self.assertEqual(
            ddp.property(QgsPalLayerSettings.Property.LabelRotation).asExpression(),
            "label_angle1",
        )

    def test_parse_visibility(self):
        context = QgsMapBoxGlStyleConversionContext()
        style = {
            "sources": {"Basemaps": {"type": "vector", "url": "https://xxxxxx"}},
            "layers": [
                {
                    "id": "at-layout-level-true",
                    "source": "streets",
                    "source-layer": "water",
                    "type": "fill",
                    "paint": {"fill-color": "#00ffff"},
                    "layout": {
                        "visibility": "visible",
                        "text-field": "{substance}",
                    },
                },
                {
                    "id": "at-layout-level-other",
                    "source": "streets",
                    "source-layer": "water",
                    "type": "fill",
                    "paint": {"fill-color": "#00ffff"},
                    "layout": {
                        "visibility": "anOtherText",
                        "text-field": "{substance}",
                    },
                },
                {
                    "id": "at-layout-level-false",
                    "source": "streets",
                    "source-layer": "water",
                    "type": "fill",
                    "paint": {"fill-color": "#00ffff"},
                    "layout": {
                        "visibility": "none",
                        "text-field": "{substance}",
                    },
                },
                {
                    "id": "at-root-level-true",
                    "source": "streets",
                    "source-layer": "water",
                    "type": "fill",
                    "paint": {"fill-color": "#00ffff"},
                    "visibility": "visible",
                },
                {
                    "id": "at-root-level-other",
                    "source": "streets",
                    "source-layer": "water",
                    "type": "fill",
                    "paint": {"fill-color": "#00ffff"},
                    "visibility": "anOtherText",
                },
                {
                    "id": "at-root-level-false",
                    "source": "streets",
                    "source-layer": "water",
                    "type": "fill",
                    "paint": {"fill-color": "#00ffff"},
                    "visibility": "none",
                },
            ],
        }

        converter = QgsMapBoxGlStyleConverter()
        converter.convert(style, context)

        renderer = converter.renderer()
        style = renderer.style(0)
        self.assertEqual(style.isEnabled(), True)
        style = renderer.style(1)
        self.assertEqual(style.isEnabled(), True)
        style = renderer.style(2)
        self.assertEqual(style.isEnabled(), False)
        style = renderer.style(3)
        self.assertEqual(style.isEnabled(), True)
        style = renderer.style(4)
        self.assertEqual(style.isEnabled(), True)
        style = renderer.style(5)
        self.assertEqual(style.isEnabled(), False)

    def test_parse_zoom_levels(self):
        context = QgsMapBoxGlStyleConversionContext()
        style = {
            "sources": {"Basemaps": {"type": "vector", "url": "https://xxxxxx"}},
            "layers": [
                {
                    "id": "water",
                    "source": "streets",
                    "source-layer": "water",
                    "minzoom": 3,
                    "maxzoom": 11,
                    "type": "fill",
                    "paint": {"fill-color": "#00ffff"},
                },
                {
                    "layout": {
                        "text-field": "{name_en}",
                        "text-font": ["Open Sans Semibold", "Arial Unicode MS Bold"],
                        "text-max-width": 8,
                        "text-anchor": "top",
                        "text-size": 11,
                        "icon-size": 1,
                    },
                    "type": "symbol",
                    "id": "poi_label",
                    "minzoom": 3,
                    "maxzoom": 11,
                    "paint": {
                        "text-color": "#666",
                        "text-halo-width": 1.5,
                        "text-halo-color": "rgba(255,255,255,0.95)",
                        "text-halo-blur": 1,
                    },
                    "source-layer": "poi_label",
                },
            ],
        }

        converter = QgsMapBoxGlStyleConverter()
        converter.convert(style, context)

        renderer = converter.renderer()
        style = renderer.style(0)
        self.assertEqual(style.minZoomLevel(), 3)
        # This differs from the handling of the max zoom as defined
        # in the MapBox Style, since in MapBox styles the style is rendered
        # only if the zoom level is less than the maximum zoom but in QGIS
        # styles the style is rendered if the zoom level is less than OR EQUAL TO
        # the maximum zoom
        self.assertEqual(style.maxZoomLevel(), 10)

        labeling = converter.labeling()
        style = labeling.style(0)
        self.assertEqual(style.minZoomLevel(), 3)
        self.assertEqual(style.maxZoomLevel(), 10)

    def test_parse_raster_source(self):
        context = QgsMapBoxGlStyleConversionContext()
        style = {
            "sources": {
                "Basemaps": {"type": "vector", "url": "https://xxxxxx"},
                "Texture-Relief": {
                    "tiles": [
                        "https://yyyyyy/v1/tiles/texturereliefshade/EPSG:3857/{z}/{x}/{y}.webp"
                    ],
                    "type": "raster",
                    "minzoom": 3,
                    "maxzoom": 20,
                    "tileSize": 256,
                    "attribution": "© 2022",
                },
            },
            "layers": [
                {
                    "layout": {"visibility": "visible"},
                    "paint": {
                        "raster-brightness-min": 0,
                        "raster-opacity": {
                            "stops": [
                                [1, 0.35],
                                [7, 0.35],
                                [8, 0.65],
                                [15, 0.65],
                                [16, 0.3],
                            ]
                        },
                        "raster-resampling": "nearest",
                        "raster-contrast": 0,
                    },
                    "id": "texture-relief-combined",
                    "source": "Texture-Relief",
                    "type": "raster",
                },
            ],
        }

        converter = QgsMapBoxGlStyleConverter()
        converter.convert(style, context)

        sources = converter.sources()
        self.assertEqual(len(sources), 1)

        raster_source = sources[0]
        self.assertIsInstance(raster_source, QgsMapBoxGlStyleRasterSource)

        self.assertEqual(raster_source.name(), "Texture-Relief")
        self.assertEqual(raster_source.type(), Qgis.MapBoxGlStyleSourceType.Raster)
        self.assertEqual(raster_source.attribution(), "© 2022")
        self.assertEqual(raster_source.minimumZoom(), 3)
        self.assertEqual(raster_source.maximumZoom(), 20)
        self.assertEqual(raster_source.tileSize(), 256)
        self.assertEqual(
            raster_source.tiles(),
            ["https://yyyyyy/v1/tiles/texturereliefshade/EPSG:3857/{z}/{x}/{y}.webp"],
        )

        # convert to raster layer
        rl = raster_source.toRasterLayer()
        self.assertIsInstance(rl, QgsRasterLayer)
        self.assertEqual(
            rl.source(),
            "tilePixelRation=1&type=xyz&url=https://yyyyyy/v1/tiles/texturereliefshade/EPSG:3857/%7Bz%7D/%7Bx%7D/%7By%7D.webp&zmax=20&zmin=3",
        )
        self.assertEqual(rl.providerType(), "wms")

        # raster sublayers
        sub_layers = converter.createSubLayers()
        self.assertEqual(len(sub_layers), 1)
        raster_layer = sub_layers[0]
        self.assertIsInstance(raster_layer, QgsRasterLayer)
        self.assertEqual(raster_layer.name(), "Texture-Relief")
        self.assertEqual(
            raster_layer.source(),
            "tilePixelRation=1&type=xyz&url=https://yyyyyy/v1/tiles/texturereliefshade/EPSG:3857/%7Bz%7D/%7Bx%7D/%7By%7D.webp&zmax=20&zmin=3",
        )
        self.assertEqual(
            raster_layer.pipe()
            .dataDefinedProperties()
            .property(QgsRasterPipe.Property.RendererOpacity)
            .asExpression(),
            "CASE WHEN @vector_tile_zoom >= 1 AND @vector_tile_zoom <= 7 THEN 35 WHEN @vector_tile_zoom > 7 AND @vector_tile_zoom <= 8 THEN (scale_linear(@vector_tile_zoom,7,8,0.35,0.65)) * 100 WHEN @vector_tile_zoom > 8 AND @vector_tile_zoom <= 15 THEN 65 WHEN @vector_tile_zoom > 15 AND @vector_tile_zoom <= 16 THEN (scale_linear(@vector_tile_zoom,15,16,0.65,0.3)) * 100 WHEN @vector_tile_zoom > 16 THEN 30 END",
        )

    def testLabelWithStops(self):
        context = QgsMapBoxGlStyleConversionContext()
        style = {
            "layout": {
                "visibility": "visible",
                "text-field": {"stops": [[6, ""], [15, "my {class} and {stuff}"]]},
            },
            "paint": {
                "text-color": "rgba(47, 47, 47, 1)",
            },
            "type": "symbol",
        }
        rendererStyle, has_renderer, labeling_style, has_labeling = (
            QgsMapBoxGlStyleConverter.parseSymbolLayer(style, context)
        )
        self.assertTrue(has_labeling)
        self.assertTrue(labeling_style.labelSettings().isExpression)
        self.assertEqual(
            labeling_style.labelSettings().fieldName,
            "CASE WHEN @vector_tile_zoom > 6 AND @vector_tile_zoom < 15 THEN concat('my ',\"class\",' and ',\"stuff\") WHEN @vector_tile_zoom >= 15 THEN concat('my ',\"class\",' and ',\"stuff\") ELSE '' END",
        )

    def testFillStroke(self):
        context = QgsMapBoxGlStyleConversionContext()
        style = {
            "id": "Land/Not ice",
            "type": "fill",
            "source": "esri",
            "source-layer": "Land",
            "layout": {},
            "paint": {
                "fill-color": "rgb(71,179,18)",
            },
        }
        has_renderer, renderer = QgsMapBoxGlStyleConverter.parseFillLayer(
            style, context
        )
        self.assertTrue(has_renderer)

        # mapbox fill strokes are always 1 px wide
        self.assertEqual(renderer.symbol()[0].strokeWidth(), 0)

        self.assertEqual(renderer.symbol()[0].strokeStyle(), Qt.PenStyle.SolidLine)
        # if "fill-outline-color" is not specified, then MapBox specs state the
        # stroke color matches the value of fill-color if unspecified.
        self.assertEqual(renderer.symbol()[0].strokeColor().name(), "#47b312")
        self.assertEqual(renderer.symbol()[0].strokeColor().alpha(), 255)

        # explicit outline color
        style = {
            "id": "Land/Not ice",
            "type": "fill",
            "source": "esri",
            "source-layer": "Land",
            "layout": {},
            "paint": {
                "fill-color": "rgb(71,179,18)",
                "fill-outline-color": "rgb(255,0,0)",
            },
        }
        has_renderer, renderer = QgsMapBoxGlStyleConverter.parseFillLayer(
            style, context
        )
        self.assertTrue(has_renderer)

        self.assertEqual(renderer.symbol()[0].strokeStyle(), Qt.PenStyle.SolidLine)
        self.assertEqual(renderer.symbol()[0].strokeColor().name(), "#ff0000")
        self.assertEqual(renderer.symbol()[0].strokeColor().alpha(), 255)

        # semi-transparent fill color
        style = {
            "id": "Land/Not ice",
            "type": "fill",
            "source": "esri",
            "source-layer": "Land",
            "layout": {},
            "paint": {
                "fill-color": "rgb(71,179,18,0.25)",
            },
        }
        has_renderer, renderer = QgsMapBoxGlStyleConverter.parseFillLayer(
            style, context
        )
        self.assertTrue(has_renderer)
        # if the outline color is semi-transparent, then drawing the default 1px stroke
        # will result in a double rendering of strokes for adjacent polygons,
        # resulting in visible seams between tiles. Accordingly, we only
        # set the stroke color if it's a completely different color to the
        # fill if the stroke color is opaque and the double-rendering artifacts aren't an issue
        self.assertEqual(renderer.symbol()[0].strokeStyle(), Qt.PenStyle.NoPen)

    def testFillOpacityWithStops(self):
        context = QgsMapBoxGlStyleConversionContext()
        style = {
            "id": "Land/Not ice",
            "type": "fill",
            "source": "esri",
            "source-layer": "Land",
            "filter": ["==", "_symbol", 0],
            "minzoom": 0,
            "layout": {},
            "paint": {
                "fill-opacity": {
                    "stops": [[0, 0.1], [8, 0.2], [14, 0.32], [15, 0.6], [17, 0.8]]
                },
                "fill-color": {
                    "stops": [
                        [0, "#e1e3d0"],
                        [8, "#e1e3d0"],
                        [14, "#E1E3D0"],
                        [15, "#ecede3"],
                        [17, "#f1f2ea"],
                    ]
                },
            },
        }
        has_renderer, renderer = QgsMapBoxGlStyleConverter.parseFillLayer(
            style, context
        )
        self.assertTrue(has_renderer)
        dd_props = renderer.symbol().dataDefinedProperties()
        prop = dd_props.property(QgsSymbol.Property.PropertyOpacity)
        self.assertEqual(
            prop.asExpression(),
            "CASE WHEN @vector_tile_zoom >= 0 AND @vector_tile_zoom <= 8 THEN (scale_linear(@vector_tile_zoom,0,8,0.1,0.2)) * 100 WHEN @vector_tile_zoom > 8 AND @vector_tile_zoom <= 14 THEN (scale_linear(@vector_tile_zoom,8,14,0.2,0.32)) * 100 WHEN @vector_tile_zoom > 14 AND @vector_tile_zoom <= 15 THEN (scale_linear(@vector_tile_zoom,14,15,0.32,0.6)) * 100 WHEN @vector_tile_zoom > 15 AND @vector_tile_zoom <= 17 THEN (scale_linear(@vector_tile_zoom,15,17,0.6,0.8)) * 100 WHEN @vector_tile_zoom > 17 THEN 80 END",
        )

        dd_props = renderer.symbol()[0].dataDefinedProperties()
        prop = dd_props.property(QgsSymbolLayer.Property.PropertyFillColor)
        self.assertEqual(
            prop.asExpression(),
            "CASE WHEN @vector_tile_zoom < 0 THEN color_hsla(66, 25, 85, 255) WHEN @vector_tile_zoom >= 0 AND @vector_tile_zoom < 8 THEN color_hsla(66, 25, 85, 255) WHEN @vector_tile_zoom >= 8 AND @vector_tile_zoom < 14 THEN color_hsla(66, 25, 85, 255) WHEN @vector_tile_zoom >= 14 AND @vector_tile_zoom < 15 THEN color_hsla(66, scale_linear(@vector_tile_zoom,14,15,25,21), scale_linear(@vector_tile_zoom,14,15,85,90), 255) WHEN @vector_tile_zoom >= 15 AND @vector_tile_zoom < 17 THEN color_hsla(scale_linear(@vector_tile_zoom,15,17,66,67), scale_linear(@vector_tile_zoom,15,17,21,23), scale_linear(@vector_tile_zoom,15,17,90,93), 255) WHEN @vector_tile_zoom >= 17 THEN color_hsla(67, 23, 93, 255) ELSE color_hsla(67, 23, 93, 255) END",
        )

    def testFillColorDDHasBrush(self):
        context = QgsMapBoxGlStyleConversionContext()
        # from https://vectortiles.geo.admin.ch/styles/ch.swisstopo.basemap.vt/style.json
        style = {
            "id": "building_fill",
            "type": "fill",
            "source": "base_v1.0.0",
            "source-layer": "building",
            "minzoom": 14.0,
            "layout": {"visibility": "visible"},
            "paint": {
                "fill-color": [
                    "interpolate",
                    ["linear"],
                    ["zoom"],
                    14,
                    [
                        "match",
                        ["get", "class"],
                        ["roof", "cooling_tower"],
                        "rgb(210, 210, 214)",
                        "rgba(184, 184, 188, 1)",
                    ],
                    16,
                    [
                        "match",
                        ["get", "class"],
                        ["roof", "cooling_tower"],
                        "rgb(210, 210, 214)",
                        "rgba(184, 184, 188, 1)",
                    ],
                ],
                "fill-opacity": 1,
            },
            "filter": ["all", ["!=", "class", "covered_bridge"]],
        }
        has_renderer, renderer = QgsMapBoxGlStyleConverter.parseFillLayer(
            style, context
        )
        self.assertTrue(has_renderer)
        self.assertEqual(renderer.symbol()[0].brushStyle(), Qt.BrushStyle.SolidPattern)
        dd_props = renderer.symbol()[0].dataDefinedProperties()
        prop = dd_props.property(QgsSymbolLayer.Property.PropertyFillColor)
        self.assertEqual(
            prop.asExpression(),
            "CASE WHEN @vector_tile_zoom >= 14 AND @vector_tile_zoom < 16 THEN color_hsla(color_part(CASE WHEN \"class\" IN ('roof', 'cooling_tower') THEN color_rgba(210,210,214,255) ELSE color_rgba(184,184,188,255) END,'hsl_hue'), color_part(CASE WHEN \"class\" IN ('roof', 'cooling_tower') THEN color_rgba(210,210,214,255) ELSE color_rgba(184,184,188,255) END,'hsl_saturation'), color_part(CASE WHEN \"class\" IN ('roof', 'cooling_tower') THEN color_rgba(210,210,214,255) ELSE color_rgba(184,184,188,255) END,'lightness'), color_part(CASE WHEN \"class\" IN ('roof', 'cooling_tower') THEN color_rgba(210,210,214,255) ELSE color_rgba(184,184,188,255) END,'alpha')) WHEN @vector_tile_zoom >= 16 THEN color_hsla(color_part(CASE WHEN \"class\" IN ('roof', 'cooling_tower') THEN color_rgba(210,210,214,255) ELSE color_rgba(184,184,188,255) END,'hsl_hue'), color_part(CASE WHEN \"class\" IN ('roof', 'cooling_tower') THEN color_rgba(210,210,214,255) ELSE color_rgba(184,184,188,255) END,'hsl_saturation'), color_part(CASE WHEN \"class\" IN ('roof', 'cooling_tower') THEN color_rgba(210,210,214,255) ELSE color_rgba(184,184,188,255) END,'lightness'), color_part(CASE WHEN \"class\" IN ('roof', 'cooling_tower') THEN color_rgba(210,210,214,255) ELSE color_rgba(184,184,188,255) END,'alpha')) ELSE color_hsla(color_part(CASE WHEN \"class\" IN ('roof', 'cooling_tower') THEN color_rgba(210,210,214,255) ELSE color_rgba(184,184,188,255) END,'hsl_hue'), color_part(CASE WHEN \"class\" IN ('roof', 'cooling_tower') THEN color_rgba(210,210,214,255) ELSE color_rgba(184,184,188,255) END,'hsl_saturation'), color_part(CASE WHEN \"class\" IN ('roof', 'cooling_tower') THEN color_rgba(210,210,214,255) ELSE color_rgba(184,184,188,255) END,'lightness'), color_part(CASE WHEN \"class\" IN ('roof', 'cooling_tower') THEN color_rgba(210,210,214,255) ELSE color_rgba(184,184,188,255) END,'alpha')) END",
        )

    def testRetrieveSprite(self):
        context = QgsMapBoxGlStyleConversionContext()
        sprite_image_file = (
            f"{TEST_DATA_DIR}/vector_tile/sprites/swisstopo-sprite@2x.png"
        )
        with open(sprite_image_file, "rb") as f:
            sprite_image = QImage()
            sprite_image.loadFromData(f.read())
        sprite_definition_file = (
            f"{TEST_DATA_DIR}/vector_tile/sprites/swisstopo-sprite@2x.json"
        )
        with open(sprite_definition_file) as f:
            sprite_definition = json.load(f)
        context.setSprites(sprite_image, sprite_definition)

        # swisstopo - lightbasemap - sinkhole
        icon_image = [
            "match",
            ["get", "class"],
            "sinkhole",
            "arrow_brown",
            ["sinkhole_rock", "sinkhole_scree"],
            "arrow_grey",
            ["sinkhole_ice", "sinkhole_water"],
            "arrow_blue",
            "",
        ]
        sprite, size, sprite_property, sprite_size_property = (
            QgsMapBoxGlStyleConverter.retrieveSpriteAsBase64WithProperties(
                icon_image, context
            )
        )
        self.assertEqual(
            sprite_property,
            "CASE WHEN \"class\" IN ('sinkhole') THEN 'base64:iVBORw0KGgoAAAANSUhEUgAAAAwAAAAgCAYAAAAmG5mqAAAACXBIWXMAAA9hAAAPYQGoP6dpAAAArklEQVQ4jWNmQAPxrgr1EzIMDiS4KjQwMDAwXLz34SCyPBO6BkJgVMOohlEN9NTA2JWit8NUTcidGMWnb73bybT60NM+Yk1ffehpH9PpW293nb71bicxpp++9XYXE0wnMaYzMEA9TcgWmOlwDYRsQZaDa8BlC7LpKBpw2YIuhqIB3RZ00zE0oJuIzUYWTDcjbEE3nYGBgYEZXYCBgYHhw5c/r649/Hz82dvvd9HlANtvdC5jaNf5AAAAAElFTkSuQmCC' WHEN \"class\" IN ('sinkhole_rock','sinkhole_scree') THEN 'base64:iVBORw0KGgoAAAANSUhEUgAAAAwAAAAgCAYAAAAmG5mqAAAACXBIWXMAAA9hAAAPYQGoP6dpAAAAoUlEQVQ4je2PMRLCIBBFv8QzcAbOwNB7LD1BzmNSaeUl8AJ/GDoKOhvDJGsyUFmxJct7DwaIMcZcnXMPY8wNAEIIz/VeSaA2HehAB/4JnKy1d631peUyyUl578dWu/d+VCRnklOLneSsFrLFDnw/Xass9gLUKutdAY4qa/sGOKrIsw0gK9L+A0jjXvG88+ZSkXYAGOQBAOScGWN8pZTecvcBJ6N45xp02+cAAAAASUVORK5CYII=' WHEN \"class\" IN ('sinkhole_ice','sinkhole_water') THEN 'base64:iVBORw0KGgoAAAANSUhEUgAAAAwAAAAgCAYAAAAmG5mqAAAACXBIWXMAAA9hAAAPYQGoP6dpAAAAq0lEQVQ4jWNgQAOSXmn1xlPO/jeecva/pFdaPbo8E7oAITCqYVTDqAZ6amBUzZ6yg0/T0p0YxZ+uH9/J9HLf0j5iTX+5b2kf06frx3d9un58JzGmf7p+fBcTTCcxpjMwQD1NyBaY6XANhGxBloNrwGULsukoGnDZgi6GogHdFnTTMTSgm4jNRhYsbobbgm46AwMDAzO6AAMDA8OfL+9ffb1/+fjPN0/uossBAN+ec6mo5jjFAAAAAElFTkSuQmCC' ELSE '' END",
        )

        # swisstopo - lightbasemap - place_village
        icon_image = [
            "step",
            ["zoom"],
            "circle_dark_grey_4",
            6,
            "circle_dark_grey_4",
            8,
            "circle_dark_grey_6",
            10,
            "circle_dark_grey_8",
            12,
            "circle_dark_grey_10",
        ]
        sprite, size, sprite_property, sprite_size_property = (
            QgsMapBoxGlStyleConverter.retrieveSpriteAsBase64WithProperties(
                icon_image, context
            )
        )
        self.assertEqual(
            sprite_property,
            "CASE WHEN @vector_tile_zoom >= 12 THEN 'base64:iVBORw0KGgoAAAANSUhEUgAAABQAAAAUCAYAAACNiR0NAAAACXBIWXMAAA9hAAAPYQGoP6dpAAACDUlEQVQ4jZ2VsaraYBTH/8mFTAfJ4NBBJBAHcdCMHTLkES7oA6Rv0D5BvU9gQRyKQ/MCgt3qpGBcdImLyRAhg4hgwIDHxSUd2s/eek24+ptCkvP7zjk5nEjIoFgsPhNRQ1VVi4gMAGBmL0mSyel08vb7/c9bcdL1DSIyqtXqDyHJgpm9IAg+MbOXKdQ0ra1p2lcA0HUdpmnCMAxUKhUAQBiG8DwPrutivV4DAKIoakdR9PLmRE3T2pZlpZZlpd1uNz0ej2kWx+Mx7Xa7qXi/VCp9flOmeDidTjNF10yn04tUtOgJAOr1+i9FUT40m020Wq281v1HuVwGM8P3fRQKhY/b7fa7LL6mruuwbfvdMoFt29B1HURkFIvFZ1mkapomiOhuIRHBNE1xbciqqloAYBi5U5KLiFVV1bpkKEbjEUQsETXkhy23kWQx6WEYPmwRsczsyUmSTADA87y8mFxEbJIkk0uGs9kMzHy3jJkxGo3+ZRjH8ZCZl2EYwnGcu4WO42C324GZl3EcD2UACILABoDBYADXdd8tc10Xg8EArx1PAHA+n3cAJFVVrfF4jNPphFqtBkVRMsvs9/vo9XoAgDAMv8RxPLwIgT8NFdLVaoXFYoHD4QBJkkBEOJ/P8H0fo9EInU4H8/n8IttsNt+EJ2vBOkTUyCuXmZdBENi5C/Y1f5eGcesXwMyeKPGa3yGWS8B8xv1iAAAAAElFTkSuQmCC'  WHEN @vector_tile_zoom >= 10 THEN 'base64:iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAACXBIWXMAAA9hAAAPYQGoP6dpAAABs0lEQVQ4jY2TMW/iMBiG37iM5hQhDwyZUIYTQmruF+AtkTKU+yeMt9Dh/sDdDezlH9At3liQbksGJJYE3ZAhSBbKQaQiS8Rd6opSaHk3W8/7+vtsfxZORCn1GGN3tm1zSqkHAFVVJWVZzqSUj1VVJce8dbxwHGfouu6v01AjrfU2y7L7PM//vAvo9XpTxtgdAPi+jyAI4LouACBNU0RRBCEEAEBKOV0sFt/fnMw512EY6jiO9SXFcazDMNScc+04zvC1Z8655px/aD4O4Zzrfr//n1LqEcbYwJTted6l9l/leR5834dlWV8YYwNi2zYHgCAIPjUbGda2bU7MU5kLu0aGpZTekqtd52UR8zHSNL3aZdiqqhJSluUMAKIoujrAsGVZzoiUclrX9U4IgSRJPrECSZJACIG6rndFUTzcKKWKw+Hw1Gq1gvl8jm63i3a7fdE8Go2glEKWZT82m010AwDb7fYvpfRbo9H4KoTAer1Gs9kEpRRKKSyXS0wmE4zHYyilIKV8TNN0CJwZpk6n85MQ0jxXQV3Xu9VqdZ/n+W+zZ51CL+M8ODfORVE87Pf7f8f8M97/C1rlJ2QfAAAAAElFTkSuQmCC'  WHEN @vector_tile_zoom >= 8 THEN 'base64:iVBORw0KGgoAAAANSUhEUgAAAAwAAAAMCAYAAABWdVznAAAACXBIWXMAAA9hAAAPYQGoP6dpAAABEElEQVQokXWSMW7CQBBFnx1EtcVKNFOmcIkU9xQsN0gk99kbkJ4TcIvNARA5AtxgkFyafkW1xZZITpEYUdiv/l8z8/8UPCEiXkQ+jTE1QM5ZY4whxvg9aAqA2Wxml8vl0VrrGCGldGrb9uN+v6cCoK7rk7V2vVqtaJqGqqowxqCqhBC4XC6klE6qukFEvHOu3+12/RTb7bZ3zvUi4ksR8QBN04xtA4D3/nFjORxY1/WkoaoqAIwxb+WkapyizDkrgKpOqrquA/5iLmOMAeBwOEwaQggAxBjDS85ZrbWb2+32er1eWSwWGGOYz+eoKvv9foj13HXd13NxP9ba9diElNK5bdv3R3ED/6/hR14jDJpfqBeVnGzOJRAAAAAASUVORK5CYII='  WHEN @vector_tile_zoom >= 6 THEN 'base64:iVBORw0KGgoAAAANSUhEUgAAAAgAAAAICAYAAADED76LAAAACXBIWXMAAA9hAAAPYQGoP6dpAAAAv0lEQVQYlYXOIQqDYBiH8b/6IW8wOPiCAxFXTIZ1i97AbFsw7wbCbjHwAMZdwcU11wwG0aLtRcQgwlYXhP0O8PAoACCljG3bvhqGcRZCmMxctm17Y+ZSkVLGvu8/sKOqqkjzPO9ORG4QBMiyDEmSYBgG9H0PIjopYRh+AKAoCliWBQBomgZpmmLbNlb30j8UzTTNiIjccRzhOA7WdUWe5+i6DtM0vf5PLstSz/P81nX9KIQ4qKpKzPys6/rCzOUX/GBLMm760HoAAAAASUVORK5CYII=' ELSE 'base64:iVBORw0KGgoAAAANSUhEUgAAAAgAAAAICAYAAADED76LAAAACXBIWXMAAA9hAAAPYQGoP6dpAAAAv0lEQVQYlYXOIQqDYBiH8b/6IW8wOPiCAxFXTIZ1i97AbFsw7wbCbjHwAMZdwcU11wwG0aLtRcQgwlYXhP0O8PAoACCljG3bvhqGcRZCmMxctm17Y+ZSkVLGvu8/sKOqqkjzPO9ORG4QBMiyDEmSYBgG9H0PIjopYRh+AKAoCliWBQBomgZpmmLbNlb30j8UzTTNiIjccRzhOA7WdUWe5+i6DtM0vf5PLstSz/P81nX9KIQ4qKpKzPys6/rCzOUX/GBLMm760HoAAAAASUVORK5CYII=' END",
        )

        # swisstopo - lightbasemap - lake_elevation
        icon_image = [
            "case",
            ["has", "lake_depth"],
            "arrow_line_blue",
            ["==", ["length", ["to-string", ["get", "ele"]]], 3],
            "line_blue_short",
            "line_blue_long",
        ]
        sprite, size, sprite_property, sprite_size_property = (
            QgsMapBoxGlStyleConverter.retrieveSpriteAsBase64WithProperties(
                icon_image, context
            )
        )
        self.assertEqual(
            sprite_property,
            "CASE WHEN \"lake_depth\" IS NOT NULL THEN 'base64:iVBORw0KGgoAAAANSUhEUgAAADAAAAAmCAYAAACCjRgBAAAACXBIWXMAAA9hAAAPYQGoP6dpAAAA3UlEQVRYhe3WMQrCMBgF4Nfo7C0cnDt5Ar2LnsWpHsE72A6umTo5ZBBxcVEEIeggCHVxCKWtLUj/BN63tfzD/16gTYSW4iQv2s72SUkvQERERER/FCd54evNs0rwt1EGkMYA0hhAGgNIYwBpDCAtGi+S7WgynTUNWaPTw3o572upLtRlt1n9GmozI0VZozNrdFo3YI1OrdFZn0t1oYDmhn1uH/gGqDsF39sHnK9QVdO+tw84AcqnEEL7QOk/4DYeQvsAMHQf3FMIoX0AGJRfvB/36/O016/b+SixUFcfqsZi6d4Ghu0AAAAASUVORK5CYII='  WHEN length(to_string(\"ele\")) IS 3 THEN 'base64:iVBORw0KGgoAAAANSUhEUgAAACQAAAAECAYAAADmrJ2uAAAACXBIWXMAAA9hAAAPYQGoP6dpAAAAGElEQVQokWM0nnL2P8MgAkwD7YBRQCoAANWHApf/BqmbAAAAAElFTkSuQmCC' ELSE 'base64:iVBORw0KGgoAAAANSUhEUgAAADAAAAAECAYAAADI6bw8AAAACXBIWXMAAA9hAAAPYQGoP6dpAAAAGElEQVQokWM0nnL2P8MQBkwD7YBRMNQBAMaFApc4aKj/AAAAAElFTkSuQmCC' END",
        )
        self.assertEqual(
            sprite_size_property,
            'CASE WHEN "lake_depth" IS NOT NULL THEN 24  WHEN length(to_string("ele")) IS 3 THEN 18 ELSE 24 END',
        )


if __name__ == "__main__":
    unittest.main()
