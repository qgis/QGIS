"""QGIS Unit tests for QgsMapBoxGlStyleConverter.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2020 by Nyall Dawson'
__date__ = '29/07/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

from qgis.PyQt.QtCore import (
    Qt,
    QCoreApplication,
    QSize
)
from qgis.PyQt.QtGui import QColor, QImage
from qgis.core import (
    Qgis,
    QgsApplication,
    QgsMapBoxGlStyleConversionContext,
    QgsMapBoxGlStyleConverter,
    QgsMapBoxGlStyleRasterSource,
    QgsRasterLayer,
    QgsRasterPipe,
    QgsSettings,
    QgsSymbol,
    QgsSymbolLayer,
    QgsWkbTypes,
)
import unittest
from qgis.testing import start_app, QgisTestCase

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
        self.assertEqual(c.convert({'x': 'y'}), QgsMapBoxGlStyleConverter.Result.NoLayerList)
        self.assertEqual(c.errorMessage(), 'Could not find layers list in JSON')
        self.assertIsNone(c.renderer())
        self.assertIsNone(c.labeling())

    def testInterpolateExpression(self):
        self.assertEqual(QgsMapBoxGlStyleConverter.interpolateExpression(5, 13, 27, 29, 1),
                         'scale_linear(@vector_tile_zoom,5,13,27,29)')
        self.assertEqual(QgsMapBoxGlStyleConverter.interpolateExpression(5, 13, 27, 29, 1.5),
                         '(27) + ((1.5^(@vector_tile_zoom - 5) - 1) / (1.5^(13 - 5) - 1)) * ((29) - (27))')
        # 'scale_exp(@vector_tile_zoom,5,13,27,29,1.5)')

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
                         'CASE WHEN @vector_tile_zoom < 0 THEN color_hsla(59, 81, 70, 255) WHEN @vector_tile_zoom >= 0 AND @vector_tile_zoom < 150 THEN color_hsla(scale_linear(@vector_tile_zoom,0,150,59,352), scale_linear(@vector_tile_zoom,0,150,81,59), scale_linear(@vector_tile_zoom,0,150,70,44), 255) WHEN @vector_tile_zoom >= 150 AND @vector_tile_zoom < 250 THEN color_hsla(scale_linear(@vector_tile_zoom,150,250,352,0), scale_linear(@vector_tile_zoom,150,250,59,72), scale_linear(@vector_tile_zoom,150,250,44,63), 255) WHEN @vector_tile_zoom >= 250 THEN color_hsla(0, 72, 63, 255) ELSE color_hsla(0, 72, 63, 255) END')
        self.assertEqual(default_col.name(), '#f1f075')
        props, default_col = QgsMapBoxGlStyleConverter.parseInterpolateColorByZoom({'base': 2,
                                                                                    'stops': [[0, '#f1f075'],
                                                                                              [150, '#b52e3e'],
                                                                                              [250, '#e55e5e']]
                                                                                    },
                                                                                   conversion_context)
        self.assertEqual(props.expressionString(),
                         'CASE WHEN @vector_tile_zoom < 0 THEN color_hsla(59, 81, 70, 255) WHEN @vector_tile_zoom >= 0 AND @vector_tile_zoom < 150 THEN color_hsla((59) + ((2^(@vector_tile_zoom - 0) - 1) / (2^(150 - 0) - 1)) * ((352) - (59)), (81) + ((2^(@vector_tile_zoom - 0) - 1) / (2^(150 - 0) - 1)) * ((59) - (81)), (70) + ((2^(@vector_tile_zoom - 0) - 1) / (2^(150 - 0) - 1)) * ((44) - (70)), 255) WHEN @vector_tile_zoom >= 150 AND @vector_tile_zoom < 250 THEN color_hsla((352) + ((2^(@vector_tile_zoom - 150) - 1) / (2^(250 - 150) - 1)) * ((0) - (352)), (59) + ((2^(@vector_tile_zoom - 150) - 1) / (2^(250 - 150) - 1)) * ((72) - (59)), (44) + ((2^(@vector_tile_zoom - 150) - 1) / (2^(250 - 150) - 1)) * ((63) - (44)), 255) WHEN @vector_tile_zoom >= 250 THEN color_hsla(0, 72, 63, 255) ELSE color_hsla(0, 72, 63, 255) END')
        self.assertEqual(default_col.name(), '#f1f075')

        props, default_col = QgsMapBoxGlStyleConverter.parseInterpolateColorByZoom({'base': 1,
                                                                                    "stops": [["9",
                                                                                               [
                                                                                                   "match",
                                                                                                   [
                                                                                                       "get",
                                                                                                       "class"
                                                                                                   ],
                                                                                                   [
                                                                                                       "motorway",
                                                                                                       "trunk"
                                                                                                   ],
                                                                                                   "rgb(255,230,160)",
                                                                                                   "rgb(255,255,255)"
                                                                                               ]
                                                                                               ],
                                                                                              [
                                                                                                  "15",
                                                                                                  [
                                                                                                      "match",
                                                                                                      [
                                                                                                          "get",
                                                                                                          "class"
                                                                                                      ],
                                                                                                      [
                                                                                                          "motorway",
                                                                                                          "trunk"
                                                                                                      ],
                                                                                                      "rgb(255, 224, 138)",
                                                                                                      "rgb(255,255,255)"
                                                                                                  ]
                                                                                    ]
                                                                                    ]
                                                                                    }, conversion_context)
        self.assertEqual(props.expressionString(),
                         "CASE WHEN @vector_tile_zoom >= 9 AND @vector_tile_zoom < 15 THEN color_hsla(scale_linear(@vector_tile_zoom,9,15,color_part(CASE WHEN \"class\" IN ('motorway', 'trunk') THEN color_rgba(255,230,160,255) ELSE color_rgba(255,255,255,255) END,'hsl_hue'),color_part(CASE WHEN \"class\" IN ('motorway', 'trunk') THEN color_rgba(255,224,138,255) ELSE color_rgba(255,255,255,255) END,'hsl_hue')), scale_linear(@vector_tile_zoom,9,15,color_part(CASE WHEN \"class\" IN ('motorway', 'trunk') THEN color_rgba(255,230,160,255) ELSE color_rgba(255,255,255,255) END,'hsl_saturation'),color_part(CASE WHEN \"class\" IN ('motorway', 'trunk') THEN color_rgba(255,224,138,255) ELSE color_rgba(255,255,255,255) END,'hsl_saturation')), scale_linear(@vector_tile_zoom,9,15,color_part(CASE WHEN \"class\" IN ('motorway', 'trunk') THEN color_rgba(255,230,160,255) ELSE color_rgba(255,255,255,255) END,'lightness'),color_part(CASE WHEN \"class\" IN ('motorway', 'trunk') THEN color_rgba(255,224,138,255) ELSE color_rgba(255,255,255,255) END,'lightness')), scale_linear(@vector_tile_zoom,9,15,color_part(CASE WHEN \"class\" IN ('motorway', 'trunk') THEN color_rgba(255,230,160,255) ELSE color_rgba(255,255,255,255) END,'alpha'),color_part(CASE WHEN \"class\" IN ('motorway', 'trunk') THEN color_rgba(255,224,138,255) ELSE color_rgba(255,255,255,255) END,'alpha'))) WHEN @vector_tile_zoom >= 15 THEN color_hsla(color_part(CASE WHEN \"class\" IN ('motorway', 'trunk') THEN color_rgba(255,224,138,255) ELSE color_rgba(255,255,255,255) END,'hsl_hue'), color_part(CASE WHEN \"class\" IN ('motorway', 'trunk') THEN color_rgba(255,224,138,255) ELSE color_rgba(255,255,255,255) END,'hsl_saturation'), color_part(CASE WHEN \"class\" IN ('motorway', 'trunk') THEN color_rgba(255,224,138,255) ELSE color_rgba(255,255,255,255) END,'lightness'), color_part(CASE WHEN \"class\" IN ('motorway', 'trunk') THEN color_rgba(255,224,138,255) ELSE color_rgba(255,255,255,255) END,'alpha')) ELSE color_hsla(color_part(CASE WHEN \"class\" IN ('motorway', 'trunk') THEN color_rgba(255,224,138,255) ELSE color_rgba(255,255,255,255) END,'hsl_hue'), color_part(CASE WHEN \"class\" IN ('motorway', 'trunk') THEN color_rgba(255,224,138,255) ELSE color_rgba(255,255,255,255) END,'hsl_saturation'), color_part(CASE WHEN \"class\" IN ('motorway', 'trunk') THEN color_rgba(255,224,138,255) ELSE color_rgba(255,255,255,255) END,'lightness'), color_part(CASE WHEN \"class\" IN ('motorway', 'trunk') THEN color_rgba(255,224,138,255) ELSE color_rgba(255,255,255,255) END,'alpha')) END")

    def testParseStops(self):
        conversion_context = QgsMapBoxGlStyleConversionContext()
        self.assertEqual(QgsMapBoxGlStyleConverter.parseStops(1, [[1, 10], [2, 20], [5, 100]], 1, conversion_context),
                         'CASE WHEN @vector_tile_zoom >= 1 AND @vector_tile_zoom <= 2 THEN scale_linear(@vector_tile_zoom,1,2,10,20) WHEN @vector_tile_zoom > 2 AND @vector_tile_zoom <= 5 THEN scale_linear(@vector_tile_zoom,2,5,20,100) WHEN @vector_tile_zoom > 5 THEN 100 END')
        self.assertEqual(QgsMapBoxGlStyleConverter.parseStops(1.5, [[1, 10], [2, 20], [5, 100]], 1, conversion_context),
                         'CASE WHEN @vector_tile_zoom >= 1 AND @vector_tile_zoom <= 2 THEN (10) + ((1.5^(@vector_tile_zoom - 1) - 1) / (1.5^(2 - 1) - 1)) * ((20) - (10)) WHEN @vector_tile_zoom > 2 AND @vector_tile_zoom <= 5 THEN (20) + ((1.5^(@vector_tile_zoom - 2) - 1) / (1.5^(5 - 2) - 1)) * ((100) - (20)) WHEN @vector_tile_zoom > 5 THEN 100 END')
        self.assertEqual(QgsMapBoxGlStyleConverter.parseStops(1, [[1, 10], [2, 20], [5, 100]], 8, conversion_context),
                         'CASE WHEN @vector_tile_zoom >= 1 AND @vector_tile_zoom <= 2 THEN (scale_linear(@vector_tile_zoom,1,2,10,20)) * 8 WHEN @vector_tile_zoom > 2 AND @vector_tile_zoom <= 5 THEN (scale_linear(@vector_tile_zoom,2,5,20,100)) * 8 WHEN @vector_tile_zoom > 5 THEN 800 END')
        self.assertEqual(QgsMapBoxGlStyleConverter.parseStops(1.5, [[1, 10], [2, 20], [5, 100]], 8, conversion_context),
                         'CASE WHEN @vector_tile_zoom >= 1 AND @vector_tile_zoom <= 2 THEN ((10) + ((1.5^(@vector_tile_zoom - 1) - 1) / (1.5^(2 - 1) - 1)) * ((20) - (10))) * 8 WHEN @vector_tile_zoom > 2 AND @vector_tile_zoom <= 5 THEN ((20) + ((1.5^(@vector_tile_zoom - 2) - 1) / (1.5^(5 - 2) - 1)) * ((100) - (20))) * 8 WHEN @vector_tile_zoom > 5 THEN 800 END')

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
        ], QgsMapBoxGlStyleConverter.PropertyType.Color, conversion_context, 2.5, 200)
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
        ], QgsMapBoxGlStyleConverter.PropertyType.Numeric, conversion_context, 2.5, 200)
        self.assertEqual(res.asExpression(),
                         'CASE WHEN "type" IN (\'Normal\') THEN 0.625 WHEN "type" IN (\'Index\') THEN 1.25 ELSE 0.5 END')
        self.assertEqual(default_number, 0.5)

        res, default_color, default_number = QgsMapBoxGlStyleConverter.parseMatchList([
            "match",
            [
                "get",
                "luminosity"
            ],
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
            "rgb(252, 252, 252)"
        ], QgsMapBoxGlStyleConverter.PropertyType.Numeric, conversion_context, 2.5, 200)
        self.assertEqual(res.asExpression(), 'CASE WHEN "luminosity" IN (-15) THEN 0 WHEN "luminosity" IN (-14) THEN 0 WHEN "luminosity" IN (-13) THEN 0 WHEN "luminosity" IN (-12) THEN 0 WHEN "luminosity" IN (-11) THEN 0 WHEN "luminosity" IN (-10) THEN 0 WHEN "luminosity" IN (-9) THEN 0 WHEN "luminosity" IN (-8) THEN 0 WHEN "luminosity" IN (-7) THEN 0 WHEN "luminosity" IN (-6) THEN 0 WHEN "luminosity" IN (-5) THEN 0 WHEN "luminosity" IN (-4) THEN 0 WHEN "luminosity" IN (-3) THEN 0 WHEN "luminosity" IN (-2) THEN 0 WHEN "luminosity" IN (-1) THEN 0 ELSE 0 END')
        self.assertEqual(default_number, 0.0)

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
        ], QgsMapBoxGlStyleConverter.PropertyType.Color, conversion_context, 2.5, 200)
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
        ], QgsMapBoxGlStyleConverter.PropertyType.Numeric, conversion_context, 2.5, 200)
        self.assertEqual(res.asExpression(),
                         'CASE WHEN @vector_tile_zoom >= 10 AND @vector_tile_zoom <= 15 THEN (scale_linear(@vector_tile_zoom,10,15,0.1,0.3)) * 2.5 WHEN @vector_tile_zoom > 15 AND @vector_tile_zoom <= 18 THEN (scale_linear(@vector_tile_zoom,15,18,0.3,0.6)) * 2.5 WHEN @vector_tile_zoom > 18 THEN 1.5 END')
        self.assertEqual(default_number, 0.25)

    def testInterpolateByZoom(self):
        conversion_context = QgsMapBoxGlStyleConversionContext()
        prop, default_val = QgsMapBoxGlStyleConverter.parseInterpolateByZoom({'base': 1,
                                                                              'stops': [[0, 11],
                                                                                        [150, 15],
                                                                                        [250, 22]]
                                                                              }, conversion_context)
        self.assertEqual(prop.expressionString(),
                         'CASE WHEN @vector_tile_zoom >= 0 AND @vector_tile_zoom <= 150 THEN scale_linear(@vector_tile_zoom,0,150,11,15) WHEN @vector_tile_zoom > 150 AND @vector_tile_zoom <= 250 THEN scale_linear(@vector_tile_zoom,150,250,15,22) WHEN @vector_tile_zoom > 250 THEN 22 END')
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
                         '(11) + ((2^(@vector_tile_zoom - 0) - 1) / (2^(150 - 0) - 1)) * ((15) - (11))')
        # 'scale_exponential(@vector_tile_zoom,0,150,11,15,2)')
        self.assertEqual(default_val, 11.0)

        prop, default_val = QgsMapBoxGlStyleConverter.parseInterpolateByZoom({'base': 2,
                                                                              'stops': [[0, 11],
                                                                                        [150, 15]]
                                                                              }, conversion_context, multiplier=5)
        self.assertEqual(prop.expressionString(),
                         '((11) + ((2^(@vector_tile_zoom - 0) - 1) / (2^(150 - 0) - 1)) * ((15) - (11))) * 5')
        # 'scale_exponential(@vector_tile_zoom,0,150,11,15,2) * 5')
        self.assertEqual(default_val, 55.0)

    def testInterpolateOpacityByZoom(self):
        conversion_context = QgsMapBoxGlStyleConversionContext()
        self.assertEqual(QgsMapBoxGlStyleConverter.parseInterpolateOpacityByZoom({'base': 1,
                                                                                  'stops': [[0, 0.1],
                                                                                            [150, 0.15],
                                                                                            [250, 0.2]]
                                                                                  }, 255,
                                                                                 conversion_context).expressionString(),
                         "CASE WHEN @vector_tile_zoom < 0 THEN set_color_part(@symbol_color, 'alpha', 25.5) WHEN @vector_tile_zoom >= 0 AND @vector_tile_zoom < 150 THEN set_color_part(@symbol_color, 'alpha', scale_linear(@vector_tile_zoom,0,150,25.5,38.25)) WHEN @vector_tile_zoom >= 150 AND @vector_tile_zoom < 250 THEN set_color_part(@symbol_color, 'alpha', scale_linear(@vector_tile_zoom,150,250,38.25,51)) WHEN @vector_tile_zoom >= 250 THEN set_color_part(@symbol_color, 'alpha', 51) END")
        self.assertEqual(QgsMapBoxGlStyleConverter.parseInterpolateOpacityByZoom({'base': 1,
                                                                                  'stops': [[0, 0.1],
                                                                                            [150, 0.15],
                                                                                            [250, 0.2]]
                                                                                  }, 100,
                                                                                 conversion_context).expressionString(),
                         "CASE WHEN @vector_tile_zoom < 0 THEN set_color_part(@symbol_color, 'alpha', 10) WHEN @vector_tile_zoom >= 0 AND @vector_tile_zoom < 150 THEN set_color_part(@symbol_color, 'alpha', scale_linear(@vector_tile_zoom,0,150,10,15)) WHEN @vector_tile_zoom >= 150 AND @vector_tile_zoom < 250 THEN set_color_part(@symbol_color, 'alpha', scale_linear(@vector_tile_zoom,150,250,15,20)) WHEN @vector_tile_zoom >= 250 THEN set_color_part(@symbol_color, 'alpha', 20) END")
        self.assertEqual(QgsMapBoxGlStyleConverter.parseInterpolateOpacityByZoom({'base': 1,
                                                                                  'stops': [[0, 0.1],
                                                                                            [150, 0.15]]
                                                                                  }, 255,
                                                                                 conversion_context).expressionString(),
                         "set_color_part(@symbol_color, 'alpha', scale_linear(@vector_tile_zoom,0,150,25.5,38.25))")
        self.assertEqual(QgsMapBoxGlStyleConverter.parseInterpolateOpacityByZoom({'base': 2,
                                                                                  'stops': [[0, 0.1],
                                                                                            [150, 0.15]]
                                                                                  }, 255,
                                                                                 conversion_context).expressionString(),
                         "set_color_part(@symbol_color, 'alpha', (25.5) + ((2^(@vector_tile_zoom - 0) - 1) / (2^(150 - 0) - 1)) * ((38.25) - (25.5)))")
        self.assertEqual(QgsMapBoxGlStyleConverter.parseInterpolateOpacityByZoom({'base': 2,
                                                                                  'stops': [[0, 0.1],
                                                                                            [150, 0.1]]
                                                                                  }, 255,
                                                                                 conversion_context).expressionString(),
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
        ], QgsMapBoxGlStyleConverter.PropertyType.Opacity, conversion_context, 2)
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
        ], QgsMapBoxGlStyleConverter.PropertyType.Numeric, conversion_context, 2)
        self.assertEqual(prop.expressionString(),
                         "CASE WHEN @vector_tile_zoom >= 10 AND @vector_tile_zoom <= 15 THEN (scale_linear(@vector_tile_zoom,10,15,0.1,0.3)) * 2 WHEN @vector_tile_zoom > 15 AND @vector_tile_zoom <= 18 THEN (scale_linear(@vector_tile_zoom,15,18,0.3,0.6)) * 2 WHEN @vector_tile_zoom > 18 THEN 1.2 END")
        self.assertEqual(default_val, 0.2)

        prop, default_color, default_val = QgsMapBoxGlStyleConverter.parseInterpolateListByZoom([
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
            ["match", ["get", "class"], ["ice", "glacier"], 0, 0.3]
        ], QgsMapBoxGlStyleConverter.PropertyType.Numeric, conversion_context, 2)
        self.assertEqual(prop.expressionString(),
                         'CASE WHEN @vector_tile_zoom >= 5 AND @vector_tile_zoom <= 6 THEN ((0) + ((1.5^(@vector_tile_zoom - 5) - 1) / (1.5^(6 - 5) - 1)) * ((CASE WHEN "class" IN (\'ice\', \'glacier\') THEN 0.3 ELSE 0 END) - (0))) * 2 WHEN @vector_tile_zoom > 6 AND @vector_tile_zoom <= 10 THEN ((CASE WHEN "class" IN (\'ice\', \'glacier\') THEN 0.3 ELSE 0 END) + ((1.5^(@vector_tile_zoom - 6) - 1) / (1.5^(10 - 6) - 1)) * ((CASE WHEN "class" IN (\'ice\', \'glacier\') THEN 0.2 ELSE 0 END) - (CASE WHEN "class" IN (\'ice\', \'glacier\') THEN 0.3 ELSE 0 END))) * 2 WHEN @vector_tile_zoom > 10 AND @vector_tile_zoom <= 11 THEN ((CASE WHEN "class" IN (\'ice\', \'glacier\') THEN 0.2 ELSE 0 END) + ((1.5^(@vector_tile_zoom - 10) - 1) / (1.5^(11 - 10) - 1)) * ((CASE WHEN "class" IN (\'ice\', \'glacier\') THEN 0.2 ELSE 0.3 END) - (CASE WHEN "class" IN (\'ice\', \'glacier\') THEN 0.2 ELSE 0 END))) * 2 WHEN @vector_tile_zoom > 11 AND @vector_tile_zoom <= 14 THEN ((CASE WHEN "class" IN (\'ice\', \'glacier\') THEN 0.2 ELSE 0.3 END) + ((1.5^(@vector_tile_zoom - 11) - 1) / (1.5^(14 - 11) - 1)) * ((CASE WHEN "class" IN (\'ice\', \'glacier\') THEN 0 ELSE 0.3 END) - (CASE WHEN "class" IN (\'ice\', \'glacier\') THEN 0.2 ELSE 0.3 END))) * 2 WHEN @vector_tile_zoom > 14 THEN ( ( CASE WHEN "class" IN (\'ice\', \'glacier\') THEN 0 ELSE 0.3 END ) * 2 ) END')

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
        self.assertEqual(QgsMapBoxGlStyleConverter.parseExpression(["to-number", ["get", "elevation"]],
                                                                   conversion_context),
                         '''to_real("elevation")''')
        self.assertEqual(QgsMapBoxGlStyleConverter.parseExpression(["%", 100, 20],
                                                                   conversion_context),
                         '''100 % 20''')
        self.assertEqual(QgsMapBoxGlStyleConverter.parseExpression(["match", ["get", "subclass"], "funicular", "rgba(243,243,246,0)", "rgb(243,243,246)"], conversion_context, True), '''CASE WHEN ("subclass" = 'funicular') THEN color_rgba(243,243,246,0) ELSE color_rgba(243,243,246,255) END''')

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
        self.assertEqual(labeling.labelSettings().fieldName,
                         '''lower(concat(concat("name_en",' - ',"name_fr"),"bar"))''')
        self.assertTrue(labeling.labelSettings().isExpression)

    def testFontFamilyReplacement(self):
        context = QgsMapBoxGlStyleConversionContext()
        style = {
            "layout": {
                "text-field": "{name_en}",
                "text-font": [
                    "not a font",
                    "also not a font"
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
        test_font = getTestFont()
        self.assertNotEqual(labeling.labelSettings().format().font().family(), test_font.family())

        # with a font replacement
        QgsApplication.fontManager().addFontFamilyReplacement('not a font', test_font.family())
        renderer, has_renderer, labeling, has_labeling = QgsMapBoxGlStyleConverter.parseSymbolLayer(style, context)
        self.assertFalse(has_renderer)
        self.assertTrue(has_labeling)
        self.assertEqual(labeling.labelSettings().format().font().family(), test_font.family())

    def testDataDefinedIconRotate(self):
        """ Test icon-rotate property that depends on a data attribute """
        context = QgsMapBoxGlStyleConversionContext()

        image = QImage(QSize(1, 1), QImage.Format.Format_ARGB32)
        context.setSprites(image, {"foo": {"x": 0, "y": 0, "width": 1, "height": 1, "pixelRatio": 1}})
        style = {
            "layout": {
                "icon-image": "{foo}",
                "icon-rotate": ["get", "ROTATION"],
                "text-size": 11,
                "icon-size": 1
            },
            "type": "symbol",
            "id": "poi_label",
            "source-layer": "poi_label"
        }
        renderer, has_renderer, labeling, has_labeling = QgsMapBoxGlStyleConverter.parseSymbolLayer(style, context)
        self.assertTrue(has_renderer)
        self.assertFalse(has_labeling)
        dd_props = renderer.symbol().symbolLayers()[0].dataDefinedProperties()
        prop = dd_props.property(QgsSymbolLayer.Property.PropertyAngle)
        self.assertEqual(prop.asExpression(), '"ROTATION"')

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
        self.assertEqual(rendererStyle.geometryType(), QgsWkbTypes.GeometryType.PointGeometry)
        properties = rendererStyle.symbol().symbolLayers()[0].properties()
        expected_properties = {
            'angle': '0',
            'cap_style': 'square',
            'color': '22,22,22,153,rgb:0.08627450980392157,0.08627450980392157,0.08627450980392157,0.59999999999999998',
            'horizontal_anchor_point': '1',
            'joinstyle': 'bevel',
            'name': 'circle',
            'offset': '11,22',
            'offset_map_unit_scale': '3x:0,0,0,0,0,0',
            'offset_unit': 'Pixel',
            'outline_color': '46,46,46,128,rgb:0.1803921568627451,0.1803921568627451,0.1803921568627451,0.50000762951094835',
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

    def testParseArrayStops(self):
        conversion_context = QgsMapBoxGlStyleConversionContext()
        exp = QgsMapBoxGlStyleConverter.parseArrayStops({}, conversion_context, 1)
        self.assertEqual(exp, '')

        exp = QgsMapBoxGlStyleConverter.parseArrayStops([[0, [0, 1]], [2, [3, 4]]], conversion_context, 1)
        self.assertEqual(exp,
                         'CASE @vector_tile_zoom <= 2 THEN array(0,1) WHEN @vector_tile_zoom > 2 THEN array(3,4) END')

        exp = QgsMapBoxGlStyleConverter.parseArrayStops([[0, [0, 1]], [2, [3, 4]]], conversion_context, 2)
        self.assertEqual(exp,
                         'CASE @vector_tile_zoom <= 2 THEN array(0,2) WHEN @vector_tile_zoom > 2 THEN array(6,8) END')

    def testParseLineDashArray(self):
        conversion_context = QgsMapBoxGlStyleConversionContext()
        style = {
            "id": "water line (intermittent)/river",
            "type": "line",
            "source": "esri",
            "source-layer": "water line (intermittent)",
            "filter": ["==", "_symbol", 3],
            "minzoom": 10,
            "layout": {
                "line-join": "round"
            },
            "paint": {
                "line-color": "#aad3df",
                "line-dasharray": {
                    "stops": [[10, [1, 1]], [17, [0.3, 0.2]]]
                },
                "line-width": {
                    "base": 1.2,
                    "stops": [[10, 1.5], [11, 2], [12, 3], [13, 5], [14, 6], [16, 10], [17, 12]]
                }
            }
        }
        has_renderer, rendererStyle = QgsMapBoxGlStyleConverter.parseLineLayer(style, conversion_context)
        self.assertTrue(has_renderer)
        self.assertEqual(rendererStyle.geometryType(), QgsWkbTypes.GeometryType.LineGeometry)
        self.assertTrue(rendererStyle.symbol()[0].useCustomDashPattern())
        dd_properties = rendererStyle.symbol().symbolLayers()[0].dataDefinedProperties()
        self.assertEqual(dd_properties.property(QgsSymbolLayer.Property.PropertyStrokeWidth).asExpression(),
                         "CASE WHEN @vector_tile_zoom >= 10 AND @vector_tile_zoom <= 11 THEN (1.5) + ((1.2^(@vector_tile_zoom - 10) - 1) / (1.2^(11 - 10) - 1)) * ((2) - (1.5)) WHEN @vector_tile_zoom > 11 AND @vector_tile_zoom <= 12 THEN (2) + ((1.2^(@vector_tile_zoom - 11) - 1) / (1.2^(12 - 11) - 1)) * ((3) - (2)) WHEN @vector_tile_zoom > 12 AND @vector_tile_zoom <= 13 THEN (3) + ((1.2^(@vector_tile_zoom - 12) - 1) / (1.2^(13 - 12) - 1)) * ((5) - (3)) WHEN @vector_tile_zoom > 13 AND @vector_tile_zoom <= 14 THEN (5) + ((1.2^(@vector_tile_zoom - 13) - 1) / (1.2^(14 - 13) - 1)) * ((6) - (5)) WHEN @vector_tile_zoom > 14 AND @vector_tile_zoom <= 16 THEN (6) + ((1.2^(@vector_tile_zoom - 14) - 1) / (1.2^(16 - 14) - 1)) * ((10) - (6)) WHEN @vector_tile_zoom > 16 AND @vector_tile_zoom <= 17 THEN (10) + ((1.2^(@vector_tile_zoom - 16) - 1) / (1.2^(17 - 16) - 1)) * ((12) - (10)) WHEN @vector_tile_zoom > 17 THEN 12 END")
        self.assertEqual(dd_properties.property(QgsSymbolLayer.Property.PropertyCustomDash).asExpression(),
                         "array_to_string(array_foreach(CASE WHEN @vector_tile_zoom > 10 AND @vector_tile_zoom <= 17 THEN array(1,1) WHEN @vector_tile_zoom > 17 THEN array(0.3,0.2) END,@element * (CASE WHEN @vector_tile_zoom >= 10 AND @vector_tile_zoom <= 11 THEN (1.5) + ((1.2^(@vector_tile_zoom - 10) - 1) / (1.2^(11 - 10) - 1)) * ((2) - (1.5)) WHEN @vector_tile_zoom > 11 AND @vector_tile_zoom <= 12 THEN (2) + ((1.2^(@vector_tile_zoom - 11) - 1) / (1.2^(12 - 11) - 1)) * ((3) - (2)) WHEN @vector_tile_zoom > 12 AND @vector_tile_zoom <= 13 THEN (3) + ((1.2^(@vector_tile_zoom - 12) - 1) / (1.2^(13 - 12) - 1)) * ((5) - (3)) WHEN @vector_tile_zoom > 13 AND @vector_tile_zoom <= 14 THEN (5) + ((1.2^(@vector_tile_zoom - 13) - 1) / (1.2^(14 - 13) - 1)) * ((6) - (5)) WHEN @vector_tile_zoom > 14 AND @vector_tile_zoom <= 16 THEN (6) + ((1.2^(@vector_tile_zoom - 14) - 1) / (1.2^(16 - 14) - 1)) * ((10) - (6)) WHEN @vector_tile_zoom > 16 AND @vector_tile_zoom <= 17 THEN (10) + ((1.2^(@vector_tile_zoom - 16) - 1) / (1.2^(17 - 16) - 1)) * ((12) - (10)) WHEN @vector_tile_zoom > 17 THEN 12 END)), ';')")

    def testParseLineDashArrayOddNumber(self):
        conversion_context = QgsMapBoxGlStyleConversionContext()
        style = {
            "id": "water line (intermittent)/river",
            "type": "line",
            "source": "esri",
            "source-layer": "water line (intermittent)",
            "filter": ["==", "_symbol", 3],
            "minzoom": 10,
            "layout": {
                "line-join": "round"
            },
            "paint": {
                "line-color": "#aad3df",
                "line-dasharray": [1, 2, 3],
                "line-width": {
                    "base": 1.2,
                    "stops": [[10, 1.5], [11, 2], [12, 3], [13, 5], [14, 6], [16, 10], [17, 12]]
                }
            }
        }
        has_renderer, rendererStyle = QgsMapBoxGlStyleConverter.parseLineLayer(style, conversion_context)
        self.assertTrue(has_renderer)
        self.assertEqual(rendererStyle.geometryType(), QgsWkbTypes.GeometryType.LineGeometry)
        self.assertTrue(rendererStyle.symbol()[0].useCustomDashPattern())
        self.assertEqual(rendererStyle.symbol()[0].customDashVector(), [6.0, 3.0])
        dd_properties = rendererStyle.symbol().symbolLayers()[0].dataDefinedProperties()
        self.assertEqual(dd_properties.property(QgsSymbolLayer.Property.PropertyStrokeWidth).asExpression(),
                         'CASE WHEN @vector_tile_zoom >= 10 AND @vector_tile_zoom <= 11 THEN (1.5) + ((1.2^(@vector_tile_zoom - 10) - 1) / (1.2^(11 - 10) - 1)) * ((2) - (1.5)) WHEN @vector_tile_zoom > 11 AND @vector_tile_zoom <= 12 THEN (2) + ((1.2^(@vector_tile_zoom - 11) - 1) / (1.2^(12 - 11) - 1)) * ((3) - (2)) WHEN @vector_tile_zoom > 12 AND @vector_tile_zoom <= 13 THEN (3) + ((1.2^(@vector_tile_zoom - 12) - 1) / (1.2^(13 - 12) - 1)) * ((5) - (3)) WHEN @vector_tile_zoom > 13 AND @vector_tile_zoom <= 14 THEN (5) + ((1.2^(@vector_tile_zoom - 13) - 1) / (1.2^(14 - 13) - 1)) * ((6) - (5)) WHEN @vector_tile_zoom > 14 AND @vector_tile_zoom <= 16 THEN (6) + ((1.2^(@vector_tile_zoom - 14) - 1) / (1.2^(16 - 14) - 1)) * ((10) - (6)) WHEN @vector_tile_zoom > 16 AND @vector_tile_zoom <= 17 THEN (10) + ((1.2^(@vector_tile_zoom - 16) - 1) / (1.2^(17 - 16) - 1)) * ((12) - (10)) WHEN @vector_tile_zoom > 17 THEN 12 END')
        self.assertEqual(dd_properties.property(QgsSymbolLayer.Property.PropertyCustomDash).asExpression(),
                         """array_to_string(array_foreach(array(4,2),@element * (CASE WHEN @vector_tile_zoom >= 10 AND @vector_tile_zoom <= 11 THEN (1.5) + ((1.2^(@vector_tile_zoom - 10) - 1) / (1.2^(11 - 10) - 1)) * ((2) - (1.5)) WHEN @vector_tile_zoom > 11 AND @vector_tile_zoom <= 12 THEN (2) + ((1.2^(@vector_tile_zoom - 11) - 1) / (1.2^(12 - 11) - 1)) * ((3) - (2)) WHEN @vector_tile_zoom > 12 AND @vector_tile_zoom <= 13 THEN (3) + ((1.2^(@vector_tile_zoom - 12) - 1) / (1.2^(13 - 12) - 1)) * ((5) - (3)) WHEN @vector_tile_zoom > 13 AND @vector_tile_zoom <= 14 THEN (5) + ((1.2^(@vector_tile_zoom - 13) - 1) / (1.2^(14 - 13) - 1)) * ((6) - (5)) WHEN @vector_tile_zoom > 14 AND @vector_tile_zoom <= 16 THEN (6) + ((1.2^(@vector_tile_zoom - 14) - 1) / (1.2^(16 - 14) - 1)) * ((10) - (6)) WHEN @vector_tile_zoom > 16 AND @vector_tile_zoom <= 17 THEN (10) + ((1.2^(@vector_tile_zoom - 16) - 1) / (1.2^(17 - 16) - 1)) * ((12) - (10)) WHEN @vector_tile_zoom > 17 THEN 12 END)), ';')""")

    def testParseLineDashArraySingleNumber(self):
        conversion_context = QgsMapBoxGlStyleConversionContext()
        style = {
            "id": "water line (intermittent)/river",
            "type": "line",
            "source": "esri",
            "source-layer": "water line (intermittent)",
            "filter": ["==", "_symbol", 3],
            "minzoom": 10,
            "layout": {
                "line-join": "round"
            },
            "paint": {
                "line-color": "#aad3df",
                "line-dasharray": [3],
                "line-width": {
                    "base": 1.2,
                    "stops": [[10, 1.5], [11, 2], [12, 3], [13, 5], [14, 6], [16, 10], [17, 12]]
                }
            }
        }
        has_renderer, rendererStyle = QgsMapBoxGlStyleConverter.parseLineLayer(style, conversion_context)
        self.assertTrue(has_renderer)
        self.assertEqual(rendererStyle.geometryType(), QgsWkbTypes.GeometryType.LineGeometry)
        self.assertFalse(rendererStyle.symbol()[0].useCustomDashPattern())
        dd_properties = rendererStyle.symbol().symbolLayers()[0].dataDefinedProperties()
        self.assertEqual(dd_properties.property(QgsSymbolLayer.Property.PropertyStrokeWidth).asExpression(),
                         'CASE WHEN @vector_tile_zoom >= 10 AND @vector_tile_zoom <= 11 THEN (1.5) + ((1.2^(@vector_tile_zoom - 10) - 1) / (1.2^(11 - 10) - 1)) * ((2) - (1.5)) WHEN @vector_tile_zoom > 11 AND @vector_tile_zoom <= 12 THEN (2) + ((1.2^(@vector_tile_zoom - 11) - 1) / (1.2^(12 - 11) - 1)) * ((3) - (2)) WHEN @vector_tile_zoom > 12 AND @vector_tile_zoom <= 13 THEN (3) + ((1.2^(@vector_tile_zoom - 12) - 1) / (1.2^(13 - 12) - 1)) * ((5) - (3)) WHEN @vector_tile_zoom > 13 AND @vector_tile_zoom <= 14 THEN (5) + ((1.2^(@vector_tile_zoom - 13) - 1) / (1.2^(14 - 13) - 1)) * ((6) - (5)) WHEN @vector_tile_zoom > 14 AND @vector_tile_zoom <= 16 THEN (6) + ((1.2^(@vector_tile_zoom - 14) - 1) / (1.2^(16 - 14) - 1)) * ((10) - (6)) WHEN @vector_tile_zoom > 16 AND @vector_tile_zoom <= 17 THEN (10) + ((1.2^(@vector_tile_zoom - 16) - 1) / (1.2^(17 - 16) - 1)) * ((12) - (10)) WHEN @vector_tile_zoom > 17 THEN 12 END')
        self.assertFalse(dd_properties.property(QgsSymbolLayer.Property.PropertyCustomDash).isActive())

    def testParseLineNoWidth(self):
        conversion_context = QgsMapBoxGlStyleConversionContext()
        style = {
            "id": "water line (intermittent)/river",
            "type": "line",
            "source": "esri",
            "source-layer": "water line (intermittent)",
            "paint": {
                "line-color": "#aad3df",
            }
        }
        has_renderer, rendererStyle = QgsMapBoxGlStyleConverter.parseLineLayer(style, conversion_context)
        self.assertTrue(has_renderer)
        self.assertEqual(rendererStyle.geometryType(), QgsWkbTypes.GeometryType.LineGeometry)
        self.assertEqual(rendererStyle.symbol()[0].width(), 1.0)

        conversion_context.setPixelSizeConversionFactor(0.5)
        has_renderer, rendererStyle = QgsMapBoxGlStyleConverter.parseLineLayer(style, conversion_context)
        self.assertTrue(has_renderer)
        self.assertEqual(rendererStyle.geometryType(), QgsWkbTypes.GeometryType.LineGeometry)
        self.assertEqual(rendererStyle.symbol()[0].width(), 0.5)

    def testLinePattern(self):
        """ Test line-pattern property """
        context = QgsMapBoxGlStyleConversionContext()

        image = QImage(QSize(1, 1), QImage.Format.Format_ARGB32)
        context.setSprites(image, {"foo": {"x": 0, "y": 0, "width": 1, "height": 1, "pixelRatio": 1}})
        style = {
            "id": "mountain range/ridge",
            "type": "line",
            "source": "esri",
            "source-layer": "mountain range",
            "filter": ["==", "_symbol", 1],
            "minzoom": 13,
            "layout": {
                "line-join": "round"
            },
            "paint": {
                "line-pattern": {"stops": [[13, "foo"], [15, "foo"]]},
                "line-width": {"stops": [[14, 20], [15, 40]]}
            }
        }
        has_renderer, rendererStyle = QgsMapBoxGlStyleConverter.parseLineLayer(style, context)
        self.assertTrue(has_renderer)
        self.assertEqual(rendererStyle.geometryType(), QgsWkbTypes.GeometryType.LineGeometry)
        self.assertEqual(rendererStyle.symbol().symbolLayers()[0].layerType(), 'RasterLine')
        dd_props = rendererStyle.symbol().symbolLayers()[0].dataDefinedProperties()
        prop = dd_props.property(QgsSymbolLayer.Property.PropertyFile)
        self.assertTrue(prop.isActive())

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
            "type": "symbol"
        }
        rendererStyle, has_renderer, labeling_style, has_labeling = QgsMapBoxGlStyleConverter.parseSymbolLayer(style, context)
        self.assertTrue(has_labeling)
        self.assertTrue(labeling_style.labelSettings().isExpression)
        self.assertEqual(labeling_style.labelSettings().fieldName, 'concat(\'Quarry \',"substance")')

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
            "type": "symbol"
        }
        rendererStyle, has_renderer, labeling_style, has_labeling = QgsMapBoxGlStyleConverter.parseSymbolLayer(style, context)
        self.assertTrue(has_labeling)
        self.assertTrue(labeling_style.labelSettings().isExpression)
        self.assertEqual(labeling_style.labelSettings().fieldName, 'concat("substance",\' Quarry\')')

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
            "type": "symbol"
        }
        rendererStyle, has_renderer, labeling_style, has_labeling = QgsMapBoxGlStyleConverter.parseSymbolLayer(style, context)
        self.assertTrue(has_labeling)
        self.assertTrue(labeling_style.labelSettings().isExpression)
        self.assertEqual(labeling_style.labelSettings().fieldName, 'concat(\'A \',"substance",\' Quarry\')')

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
            "type": "symbol"
        }
        rendererStyle, has_renderer, labeling_style, has_labeling = QgsMapBoxGlStyleConverter.parseSymbolLayer(style, context)
        self.assertTrue(has_labeling)
        self.assertFalse(labeling_style.labelSettings().isExpression)
        self.assertEqual(labeling_style.labelSettings().fieldName, 'substance')

    def test_parse_zoom_levels(self):
        context = QgsMapBoxGlStyleConversionContext()
        style = {
            "sources": {
                "Basemaps": {
                    "type": "vector",
                    "url": "https://xxxxxx"
                }
            },
            "layers": [
                {
                    "id": "water",
                    "source": "streets",
                    "source-layer": "water",
                    "minzoom": 3,
                    "maxzoom": 11,
                    "type": "fill",
                    "paint": {
                        "fill-color": "#00ffff"
                    }
                },
                {
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
                    "minzoom": 3,
                    "maxzoom": 11,
                    "paint": {
                        "text-color": "#666",
                        "text-halo-width": 1.5,
                        "text-halo-color": "rgba(255,255,255,0.95)",
                        "text-halo-blur": 1
                    },
                    "source-layer": "poi_label"
                }
            ]
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
                "Basemaps": {
                    "type": "vector",
                    "url": "https://xxxxxx"
                },
                "Texture-Relief": {
                    "tiles": [
                        "https://yyyyyy/v1/tiles/texturereliefshade/EPSG:3857/{z}/{x}/{y}.webp"
                    ],
                    "type": "raster",
                    "minzoom": 3,
                    "maxzoom": 20,
                    "tileSize": 256,
                    "attribution": "© 2022",
                }
            },
            "layers": [
                {
                    "layout": {
                        "visibility": "visible"
                    },
                    "paint": {
                        "raster-brightness-min": 0,
                        "raster-opacity": {
                            "stops": [
                                [
                                    1,
                                    0.35
                                ],
                                [
                                    7,
                                    0.35
                                ],
                                [
                                    8,
                                    0.65
                                ],
                                [
                                    15,
                                    0.65
                                ],
                                [
                                    16,
                                    0.3
                                ]
                            ]
                        },
                        "raster-resampling": "nearest",
                        "raster-contrast": 0
                    },
                    "id": "texture-relief-combined",
                    "source": "Texture-Relief",
                    "type": "raster"
                },
            ]
        }

        converter = QgsMapBoxGlStyleConverter()
        converter.convert(style, context)

        sources = converter.sources()
        self.assertEqual(len(sources), 1)

        raster_source = sources[0]
        self.assertIsInstance(raster_source, QgsMapBoxGlStyleRasterSource)

        self.assertEqual(raster_source.name(), 'Texture-Relief')
        self.assertEqual(raster_source.type(), Qgis.MapBoxGlStyleSourceType.Raster)
        self.assertEqual(raster_source.attribution(), '© 2022')
        self.assertEqual(raster_source.minimumZoom(), 3)
        self.assertEqual(raster_source.maximumZoom(), 20)
        self.assertEqual(raster_source.tileSize(), 256)
        self.assertEqual(raster_source.tiles(), ['https://yyyyyy/v1/tiles/texturereliefshade/EPSG:3857/{z}/{x}/{y}.webp'])

        # convert to raster layer
        rl = raster_source.toRasterLayer()
        self.assertIsInstance(rl, QgsRasterLayer)
        self.assertEqual(rl.source(), 'tilePixelRation=1&type=xyz&url=https://yyyyyy/v1/tiles/texturereliefshade/EPSG:3857/%7Bz%7D/%7Bx%7D/%7By%7D.webp&zmax=20&zmin=3')
        self.assertEqual(rl.providerType(), 'wms')

        # raster sublayers
        sub_layers = converter.createSubLayers()
        self.assertEqual(len(sub_layers), 1)
        raster_layer = sub_layers[0]
        self.assertIsInstance(raster_layer, QgsRasterLayer)
        self.assertEqual(raster_layer.name(), 'Texture-Relief')
        self.assertEqual(raster_layer.source(), 'tilePixelRation=1&type=xyz&url=https://yyyyyy/v1/tiles/texturereliefshade/EPSG:3857/%7Bz%7D/%7Bx%7D/%7By%7D.webp&zmax=20&zmin=3')
        self.assertEqual(raster_layer.pipe().dataDefinedProperties().property(QgsRasterPipe.Property.RendererOpacity).asExpression(), 'CASE WHEN @vector_tile_zoom >= 1 AND @vector_tile_zoom <= 7 THEN 35 WHEN @vector_tile_zoom > 7 AND @vector_tile_zoom <= 8 THEN (scale_linear(@vector_tile_zoom,7,8,0.35,0.65)) * 100 WHEN @vector_tile_zoom > 8 AND @vector_tile_zoom <= 15 THEN 65 WHEN @vector_tile_zoom > 15 AND @vector_tile_zoom <= 16 THEN (scale_linear(@vector_tile_zoom,15,16,0.65,0.3)) * 100 WHEN @vector_tile_zoom > 16 THEN 30 END')

    def testLabelWithStops(self):
        context = QgsMapBoxGlStyleConversionContext()
        style = {
            "layout": {
                "visibility": "visible",
                "text-field": {
                    "stops": [
                        [
                            6,
                            ""
                        ],
                        [
                            15,
                            "my {class} and {stuff}"
                        ]
                    ]
                }
            },
            "paint": {
                "text-color": "rgba(47, 47, 47, 1)",
            },
            "type": "symbol"
        }
        rendererStyle, has_renderer, labeling_style, has_labeling = QgsMapBoxGlStyleConverter.parseSymbolLayer(style, context)
        self.assertTrue(has_labeling)
        self.assertTrue(labeling_style.labelSettings().isExpression)
        self.assertEqual(labeling_style.labelSettings().fieldName, 'CASE WHEN @vector_tile_zoom > 6 AND @vector_tile_zoom < 15 THEN concat(\'my \',"class",\' and \',"stuff") WHEN @vector_tile_zoom >= 15 THEN concat(\'my \',"class",\' and \',"stuff") ELSE \'\' END')

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
            }
        }
        has_renderer, renderer = QgsMapBoxGlStyleConverter.parseFillLayer(style, context)
        self.assertTrue(has_renderer)

        # mapbox fill strokes are always 1 px wide
        self.assertEqual(renderer.symbol()[0].strokeWidth(), 0)

        self.assertEqual(renderer.symbol()[0].strokeStyle(), Qt.PenStyle.SolidLine)
        # if "fill-outline-color" is not specified, then MapBox specs state the
        # stroke color matches the value of fill-color if unspecified.
        self.assertEqual(renderer.symbol()[0].strokeColor().name(), '#47b312')
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
            }
        }
        has_renderer, renderer = QgsMapBoxGlStyleConverter.parseFillLayer(style, context)
        self.assertTrue(has_renderer)

        self.assertEqual(renderer.symbol()[0].strokeStyle(), Qt.PenStyle.SolidLine)
        self.assertEqual(renderer.symbol()[0].strokeColor().name(), '#ff0000')
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
            }
        }
        has_renderer, renderer = QgsMapBoxGlStyleConverter.parseFillLayer(style, context)
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
            "filter": [
                "==",
                "_symbol",
                0
            ],
            "minzoom": 0,
            "layout": {},
            "paint": {
                "fill-opacity": {
                    "stops": [
                        [
                            0,
                            0.1
                        ],
                        [
                            8,
                            0.2
                        ],
                        [
                            14,
                            0.32
                        ],
                        [
                            15,
                            0.6
                        ],
                        [
                            17,
                            0.8
                        ]
                    ]
                },
                "fill-color": {
                    "stops": [
                        [
                            0,
                            "#e1e3d0"
                        ],
                        [
                            8,
                            "#e1e3d0"
                        ],
                        [
                            14,
                            "#E1E3D0"
                        ],
                        [
                            15,
                            "#ecede3"
                        ],
                        [
                            17,
                            "#f1f2ea"
                        ]
                    ]
                }
            }
        }
        has_renderer, renderer = QgsMapBoxGlStyleConverter.parseFillLayer(style, context)
        self.assertTrue(has_renderer)
        dd_props = renderer.symbol().dataDefinedProperties()
        prop = dd_props.property(QgsSymbol.Property.PropertyOpacity)
        self.assertEqual(prop.asExpression(), 'CASE WHEN @vector_tile_zoom >= 0 AND @vector_tile_zoom <= 8 THEN (scale_linear(@vector_tile_zoom,0,8,0.1,0.2)) * 100 WHEN @vector_tile_zoom > 8 AND @vector_tile_zoom <= 14 THEN (scale_linear(@vector_tile_zoom,8,14,0.2,0.32)) * 100 WHEN @vector_tile_zoom > 14 AND @vector_tile_zoom <= 15 THEN (scale_linear(@vector_tile_zoom,14,15,0.32,0.6)) * 100 WHEN @vector_tile_zoom > 15 AND @vector_tile_zoom <= 17 THEN (scale_linear(@vector_tile_zoom,15,17,0.6,0.8)) * 100 WHEN @vector_tile_zoom > 17 THEN 80 END')

        dd_props = renderer.symbol()[0].dataDefinedProperties()
        prop = dd_props.property(QgsSymbolLayer.Property.PropertyFillColor)
        self.assertEqual(prop.asExpression(), 'CASE WHEN @vector_tile_zoom < 0 THEN color_hsla(66, 25, 85, 255) WHEN @vector_tile_zoom >= 0 AND @vector_tile_zoom < 8 THEN color_hsla(66, 25, 85, 255) WHEN @vector_tile_zoom >= 8 AND @vector_tile_zoom < 14 THEN color_hsla(66, 25, 85, 255) WHEN @vector_tile_zoom >= 14 AND @vector_tile_zoom < 15 THEN color_hsla(66, scale_linear(@vector_tile_zoom,14,15,25,21), scale_linear(@vector_tile_zoom,14,15,85,90), 255) WHEN @vector_tile_zoom >= 15 AND @vector_tile_zoom < 17 THEN color_hsla(scale_linear(@vector_tile_zoom,15,17,66,67), scale_linear(@vector_tile_zoom,15,17,21,23), scale_linear(@vector_tile_zoom,15,17,90,93), 255) WHEN @vector_tile_zoom >= 17 THEN color_hsla(67, 23, 93, 255) ELSE color_hsla(67, 23, 93, 255) END')

    def testFillColorDDHasBrush(self):
        context = QgsMapBoxGlStyleConversionContext()
        # from https://vectortiles.geo.admin.ch/styles/ch.swisstopo.basemap.vt/style.json
        style = {
            "id": "building_fill",
            "type": "fill",
            "source": "base_v1.0.0",
            "source-layer": "building",
            "minzoom": 14.0,
            "layout": {
                "visibility": "visible"
            },
            "paint": {
                "fill-color": [
                    "interpolate",
                    [
                        "linear"
                    ],
                    [
                        "zoom"
                    ],
                    14,
                    [
                        "match",
                        [
                            "get",
                            "class"
                        ],
                        [
                            "roof",
                            "cooling_tower"
                        ],
                        "rgb(210, 210, 214)",
                        "rgba(184, 184, 188, 1)"
                    ],
                    16,
                    [
                        "match",
                        [
                            "get",
                            "class"
                        ],
                        [
                            "roof",
                            "cooling_tower"
                        ],
                        "rgb(210, 210, 214)",
                        "rgba(184, 184, 188, 1)"
                    ]
                ],
                "fill-opacity": 1
            },
            "filter": [
                "all",
                [
                    "!=",
                    "class",
                    "covered_bridge"
                ]
            ]
        }
        has_renderer, renderer = QgsMapBoxGlStyleConverter.parseFillLayer(style, context)
        self.assertTrue(has_renderer)
        self.assertEqual(renderer.symbol()[0].brushStyle(), Qt.BrushStyle.SolidPattern)
        dd_props = renderer.symbol()[0].dataDefinedProperties()
        prop = dd_props.property(QgsSymbolLayer.Property.PropertyFillColor)
        self.assertEqual(prop.asExpression(), 'CASE WHEN @vector_tile_zoom >= 14 AND @vector_tile_zoom < 16 THEN color_hsla(color_part(CASE WHEN "class" IN (\'roof\', \'cooling_tower\') THEN color_rgba(210,210,214,255) ELSE color_rgba(184,184,188,255) END,\'hsl_hue\'), color_part(CASE WHEN "class" IN (\'roof\', \'cooling_tower\') THEN color_rgba(210,210,214,255) ELSE color_rgba(184,184,188,255) END,\'hsl_saturation\'), color_part(CASE WHEN "class" IN (\'roof\', \'cooling_tower\') THEN color_rgba(210,210,214,255) ELSE color_rgba(184,184,188,255) END,\'lightness\'), color_part(CASE WHEN "class" IN (\'roof\', \'cooling_tower\') THEN color_rgba(210,210,214,255) ELSE color_rgba(184,184,188,255) END,\'alpha\')) WHEN @vector_tile_zoom >= 16 THEN color_hsla(color_part(CASE WHEN "class" IN (\'roof\', \'cooling_tower\') THEN color_rgba(210,210,214,255) ELSE color_rgba(184,184,188,255) END,\'hsl_hue\'), color_part(CASE WHEN "class" IN (\'roof\', \'cooling_tower\') THEN color_rgba(210,210,214,255) ELSE color_rgba(184,184,188,255) END,\'hsl_saturation\'), color_part(CASE WHEN "class" IN (\'roof\', \'cooling_tower\') THEN color_rgba(210,210,214,255) ELSE color_rgba(184,184,188,255) END,\'lightness\'), color_part(CASE WHEN "class" IN (\'roof\', \'cooling_tower\') THEN color_rgba(210,210,214,255) ELSE color_rgba(184,184,188,255) END,\'alpha\')) ELSE color_hsla(color_part(CASE WHEN "class" IN (\'roof\', \'cooling_tower\') THEN color_rgba(210,210,214,255) ELSE color_rgba(184,184,188,255) END,\'hsl_hue\'), color_part(CASE WHEN "class" IN (\'roof\', \'cooling_tower\') THEN color_rgba(210,210,214,255) ELSE color_rgba(184,184,188,255) END,\'hsl_saturation\'), color_part(CASE WHEN "class" IN (\'roof\', \'cooling_tower\') THEN color_rgba(210,210,214,255) ELSE color_rgba(184,184,188,255) END,\'lightness\'), color_part(CASE WHEN "class" IN (\'roof\', \'cooling_tower\') THEN color_rgba(210,210,214,255) ELSE color_rgba(184,184,188,255) END,\'alpha\')) END')


if __name__ == '__main__':
    unittest.main()
