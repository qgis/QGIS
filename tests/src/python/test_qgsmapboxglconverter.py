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
from qgis.PyQt.QtCore import QSize, QCoreApplication
from qgis.PyQt.QtGui import (QColor, QImage)
from qgis.core import (QgsMapBoxGlStyleConverter,
                       QgsMapBoxGlStyleConversionContext,
                       QgsMapBoxGlStyleRasterSource,
                       QgsSymbolLayer,
                       QgsWkbTypes,
                       QgsApplication,
                       QgsFontManager,
                       QgsSettings,
                       Qgis,
                       QgsRasterLayer,
                       QgsRasterPipe
                       )
from qgis.testing import start_app, unittest

from utilities import unitTestDataPath, getTestFont

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsMapBoxGlStyleConverter(unittest.TestCase):
    maxDiff = 100000

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("QGIS_TestQgsMapBoxGlStyleConverter.com")
        QCoreApplication.setApplicationName("QGIS_TestQgsMapBoxGlStyleConverter")
        QgsSettings().clear()
        start_app()

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
                         'CASE WHEN @vector_tile_zoom < 0 THEN color_hsla(59, 81, 70, 255) WHEN @vector_tile_zoom >= 0 AND @vector_tile_zoom < 150 THEN color_hsla(scale_linear(@vector_tile_zoom,0,150,59,352), scale_linear(@vector_tile_zoom,0,150,81,59), scale_linear(@vector_tile_zoom,0,150,70,44), 255) WHEN @vector_tile_zoom >= 150 AND @vector_tile_zoom < 250 THEN color_hsla(scale_linear(@vector_tile_zoom,150,250,352,0), scale_linear(@vector_tile_zoom,150,250,59,72), scale_linear(@vector_tile_zoom,150,250,44,63), 255) WHEN @vector_tile_zoom >= 250 THEN color_hsla(0, 72, 63, 255) ELSE color_hsla(0, 72, 63, 255) END')
        self.assertEqual(default_col.name(), '#f1f075')
        props, default_col = QgsMapBoxGlStyleConverter.parseInterpolateColorByZoom({'base': 2,
                                                                                    'stops': [[0, '#f1f075'],
                                                                                              [150, '#b52e3e'],
                                                                                              [250, '#e55e5e']]
                                                                                    },
                                                                                   conversion_context)
        self.assertEqual(props.expressionString(),
                         'CASE WHEN @vector_tile_zoom < 0 THEN color_hsla(59, 81, 70, 255) WHEN @vector_tile_zoom >= 0 AND @vector_tile_zoom < 150 THEN color_hsla(scale_exp(@vector_tile_zoom,0,150,59,352,2), scale_exp(@vector_tile_zoom,0,150,81,59,2), scale_exp(@vector_tile_zoom,0,150,70,44,2), 255) WHEN @vector_tile_zoom >= 150 AND @vector_tile_zoom < 250 THEN color_hsla(scale_exp(@vector_tile_zoom,150,250,352,0,2), scale_exp(@vector_tile_zoom,150,250,59,72,2), scale_exp(@vector_tile_zoom,150,250,44,63,2), 255) WHEN @vector_tile_zoom >= 250 THEN color_hsla(0, 72, 63, 255) ELSE color_hsla(0, 72, 63, 255) END')
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
                         "set_color_part(@symbol_color, 'alpha', scale_exp(@vector_tile_zoom,0,150,25.5,38.25,2))")
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
                         "CASE WHEN @vector_tile_zoom > 10 AND @vector_tile_zoom <= 15 THEN scale_linear(@vector_tile_zoom,10,15,0.1,0.3) * 2 WHEN @vector_tile_zoom > 15 AND @vector_tile_zoom <= 18 THEN scale_linear(@vector_tile_zoom,15,18,0.3,0.6) * 2 WHEN @vector_tile_zoom > 18 THEN 1.2 END")
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
                         "CASE WHEN @vector_tile_zoom > 5 AND @vector_tile_zoom <= 6 THEN scale_exp(@vector_tile_zoom,5,6,0,CASE WHEN \"class\" IN ('ice', 'glacier') THEN 0.3 ELSE 0 END,1.5) * 2 WHEN @vector_tile_zoom > 6 AND @vector_tile_zoom <= 10 THEN scale_exp(@vector_tile_zoom,6,10,CASE WHEN \"class\" IN ('ice', 'glacier') THEN 0.3 ELSE 0 END,CASE WHEN \"class\" IN ('ice', 'glacier') THEN 0.2 ELSE 0 END,1.5) * 2 WHEN @vector_tile_zoom > 10 AND @vector_tile_zoom <= 11 THEN scale_exp(@vector_tile_zoom,10,11,CASE WHEN \"class\" IN ('ice', 'glacier') THEN 0.2 ELSE 0 END,CASE WHEN \"class\" IN ('ice', 'glacier') THEN 0.2 ELSE 0.3 END,1.5) * 2 WHEN @vector_tile_zoom > 11 AND @vector_tile_zoom <= 14 THEN scale_exp(@vector_tile_zoom,11,14,CASE WHEN \"class\" IN ('ice', 'glacier') THEN 0.2 ELSE 0.3 END,CASE WHEN \"class\" IN ('ice', 'glacier') THEN 0 ELSE 0.3 END,1.5) * 2 WHEN @vector_tile_zoom > 14 THEN ( ( CASE WHEN \"class\" IN ('ice', 'glacier') THEN 0 ELSE 0.3 END ) * 2 ) END")

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

        image = QImage(QSize(1, 1), QImage.Format_ARGB32)
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
        prop = dd_props.property(QgsSymbolLayer.PropertyAngle)
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

    def testParseArrayStops(self):
        conversion_context = QgsMapBoxGlStyleConversionContext()
        exp = QgsMapBoxGlStyleConverter.parseArrayStops({}, conversion_context, 1)
        self.assertEqual(exp, '')

        exp = QgsMapBoxGlStyleConverter.parseArrayStops([[0, [0, 1]], [2, [3, 4]]], conversion_context, 1)
        self.assertEqual(exp,
                         'CASE WHEN @vector_tile_zoom > 0 AND @vector_tile_zoom <= 2 THEN array(0,1) WHEN @vector_tile_zoom > 2 THEN array(3,4) END')

        exp = QgsMapBoxGlStyleConverter.parseArrayStops([[0, [0, 1]], [2, [3, 4]]], conversion_context, 2)
        self.assertEqual(exp,
                         'CASE WHEN @vector_tile_zoom > 0 AND @vector_tile_zoom <= 2 THEN array(0,2) WHEN @vector_tile_zoom > 2 THEN array(6,8) END')

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
        self.assertEqual(rendererStyle.geometryType(), QgsWkbTypes.LineGeometry)
        self.assertTrue(rendererStyle.symbol()[0].useCustomDashPattern())
        dd_properties = rendererStyle.symbol().symbolLayers()[0].dataDefinedProperties()
        self.assertEqual(dd_properties.property(QgsSymbolLayer.PropertyStrokeWidth).asExpression(),
                         'CASE WHEN @vector_tile_zoom > 10 AND @vector_tile_zoom <= 11 THEN scale_exp(@vector_tile_zoom,10,11,1.5,2,1.2) WHEN @vector_tile_zoom > 11 AND @vector_tile_zoom <= 12 THEN scale_exp(@vector_tile_zoom,11,12,2,3,1.2) WHEN @vector_tile_zoom > 12 AND @vector_tile_zoom <= 13 THEN scale_exp(@vector_tile_zoom,12,13,3,5,1.2) WHEN @vector_tile_zoom > 13 AND @vector_tile_zoom <= 14 THEN scale_exp(@vector_tile_zoom,13,14,5,6,1.2) WHEN @vector_tile_zoom > 14 AND @vector_tile_zoom <= 16 THEN scale_exp(@vector_tile_zoom,14,16,6,10,1.2) WHEN @vector_tile_zoom > 16 AND @vector_tile_zoom <= 17 THEN scale_exp(@vector_tile_zoom,16,17,10,12,1.2) WHEN @vector_tile_zoom > 17 THEN 12 END')
        self.assertEqual(dd_properties.property(QgsSymbolLayer.PropertyCustomDash).asExpression(),
                         'array_to_string(array_foreach(CASE WHEN @vector_tile_zoom > 10 AND @vector_tile_zoom <= 17 THEN array(1,1) WHEN @vector_tile_zoom > 17 THEN array(0.3,0.2) END,@element * (CASE WHEN @vector_tile_zoom > 10 AND @vector_tile_zoom <= 11 THEN scale_exp(@vector_tile_zoom,10,11,1.5,2,1.2) WHEN @vector_tile_zoom > 11 AND @vector_tile_zoom <= 12 THEN scale_exp(@vector_tile_zoom,11,12,2,3,1.2) WHEN @vector_tile_zoom > 12 AND @vector_tile_zoom <= 13 THEN scale_exp(@vector_tile_zoom,12,13,3,5,1.2) WHEN @vector_tile_zoom > 13 AND @vector_tile_zoom <= 14 THEN scale_exp(@vector_tile_zoom,13,14,5,6,1.2) WHEN @vector_tile_zoom > 14 AND @vector_tile_zoom <= 16 THEN scale_exp(@vector_tile_zoom,14,16,6,10,1.2) WHEN @vector_tile_zoom > 16 AND @vector_tile_zoom <= 17 THEN scale_exp(@vector_tile_zoom,16,17,10,12,1.2) WHEN @vector_tile_zoom > 17 THEN 12 END)), \';\')')

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
        self.assertEqual(rendererStyle.geometryType(), QgsWkbTypes.LineGeometry)
        self.assertTrue(rendererStyle.symbol()[0].useCustomDashPattern())
        self.assertEqual(rendererStyle.symbol()[0].customDashVector(), [6.0, 3.0])
        dd_properties = rendererStyle.symbol().symbolLayers()[0].dataDefinedProperties()
        self.assertEqual(dd_properties.property(QgsSymbolLayer.PropertyStrokeWidth).asExpression(),
                         'CASE WHEN @vector_tile_zoom > 10 AND @vector_tile_zoom <= 11 THEN scale_exp(@vector_tile_zoom,10,11,1.5,2,1.2) WHEN @vector_tile_zoom > 11 AND @vector_tile_zoom <= 12 THEN scale_exp(@vector_tile_zoom,11,12,2,3,1.2) WHEN @vector_tile_zoom > 12 AND @vector_tile_zoom <= 13 THEN scale_exp(@vector_tile_zoom,12,13,3,5,1.2) WHEN @vector_tile_zoom > 13 AND @vector_tile_zoom <= 14 THEN scale_exp(@vector_tile_zoom,13,14,5,6,1.2) WHEN @vector_tile_zoom > 14 AND @vector_tile_zoom <= 16 THEN scale_exp(@vector_tile_zoom,14,16,6,10,1.2) WHEN @vector_tile_zoom > 16 AND @vector_tile_zoom <= 17 THEN scale_exp(@vector_tile_zoom,16,17,10,12,1.2) WHEN @vector_tile_zoom > 17 THEN 12 END')
        self.assertEqual(dd_properties.property(QgsSymbolLayer.PropertyCustomDash).asExpression(),
                         """array_to_string(array_foreach(array(4,2),@element * (CASE WHEN @vector_tile_zoom > 10 AND @vector_tile_zoom <= 11 THEN scale_exp(@vector_tile_zoom,10,11,1.5,2,1.2) WHEN @vector_tile_zoom > 11 AND @vector_tile_zoom <= 12 THEN scale_exp(@vector_tile_zoom,11,12,2,3,1.2) WHEN @vector_tile_zoom > 12 AND @vector_tile_zoom <= 13 THEN scale_exp(@vector_tile_zoom,12,13,3,5,1.2) WHEN @vector_tile_zoom > 13 AND @vector_tile_zoom <= 14 THEN scale_exp(@vector_tile_zoom,13,14,5,6,1.2) WHEN @vector_tile_zoom > 14 AND @vector_tile_zoom <= 16 THEN scale_exp(@vector_tile_zoom,14,16,6,10,1.2) WHEN @vector_tile_zoom > 16 AND @vector_tile_zoom <= 17 THEN scale_exp(@vector_tile_zoom,16,17,10,12,1.2) WHEN @vector_tile_zoom > 17 THEN 12 END)), ';')""")

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
        self.assertEqual(rendererStyle.geometryType(), QgsWkbTypes.LineGeometry)
        self.assertFalse(rendererStyle.symbol()[0].useCustomDashPattern())
        dd_properties = rendererStyle.symbol().symbolLayers()[0].dataDefinedProperties()
        self.assertEqual(dd_properties.property(QgsSymbolLayer.PropertyStrokeWidth).asExpression(),
                         'CASE WHEN @vector_tile_zoom > 10 AND @vector_tile_zoom <= 11 THEN scale_exp(@vector_tile_zoom,10,11,1.5,2,1.2) WHEN @vector_tile_zoom > 11 AND @vector_tile_zoom <= 12 THEN scale_exp(@vector_tile_zoom,11,12,2,3,1.2) WHEN @vector_tile_zoom > 12 AND @vector_tile_zoom <= 13 THEN scale_exp(@vector_tile_zoom,12,13,3,5,1.2) WHEN @vector_tile_zoom > 13 AND @vector_tile_zoom <= 14 THEN scale_exp(@vector_tile_zoom,13,14,5,6,1.2) WHEN @vector_tile_zoom > 14 AND @vector_tile_zoom <= 16 THEN scale_exp(@vector_tile_zoom,14,16,6,10,1.2) WHEN @vector_tile_zoom > 16 AND @vector_tile_zoom <= 17 THEN scale_exp(@vector_tile_zoom,16,17,10,12,1.2) WHEN @vector_tile_zoom > 17 THEN 12 END')
        self.assertFalse(dd_properties.property(QgsSymbolLayer.PropertyCustomDash).isActive())

    def testLinePattern(self):
        """ Test line-pattern property """
        context = QgsMapBoxGlStyleConversionContext()

        image = QImage(QSize(1, 1), QImage.Format_ARGB32)
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
        self.assertEqual(rendererStyle.geometryType(), QgsWkbTypes.LineGeometry)
        self.assertEqual(rendererStyle.symbol().symbolLayers()[0].layerType(), 'RasterLine')
        dd_props = rendererStyle.symbol().symbolLayers()[0].dataDefinedProperties()
        prop = dd_props.property(QgsSymbolLayer.PropertyFile)
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
        self.assertEqual(raster_layer.pipe().dataDefinedProperties().property(QgsRasterPipe.RendererOpacity).asExpression(), 'CASE WHEN @vector_tile_zoom > 1 AND @vector_tile_zoom <= 7 THEN 35 WHEN @vector_tile_zoom > 7 AND @vector_tile_zoom <= 8 THEN scale_linear(@vector_tile_zoom,7,8,0.35,0.65) * 100 WHEN @vector_tile_zoom > 8 AND @vector_tile_zoom <= 15 THEN 65 WHEN @vector_tile_zoom > 15 AND @vector_tile_zoom <= 16 THEN scale_linear(@vector_tile_zoom,15,16,0.65,0.3) * 100 WHEN @vector_tile_zoom > 16 THEN 30 END')

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


if __name__ == '__main__':
    unittest.main()
