# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsArcGisRestUtils

From build dir, run: ctest -R QgsArcGisRestUtils -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2022 by Nyall Dawson'
__date__ = '14/07/2022'
__copyright__ = 'Copyright 2022, The QGIS Project'

import qgis  # NOQA
from qgis.core import (
    QgsGeometry,
    QgsArcGisRestUtils
)
from qgis.testing import start_app, unittest

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsArcGisRestUtils(unittest.TestCase):

    def test_geometry_to_json(self):
        tests = [
            ('Point(1 2)', {'x': 1.0, 'y': 2.0}),
            ('PointZ(1 2 3)', {'x': 1.0, 'y': 2.0, 'z': 3.0}),
            ('PointM(1 2 4)', {'x': 1.0, 'y': 2.0, 'm': 4.0}),
            ('PointZM(1 2 3 4)', {'x': 1.0, 'y': 2.0, 'z': 3.0, 'm': 4.0}),
            ('MultiPoint(1 1, 10 1, 3 4)',
             {'hasM': False, 'hasZ': False, 'points': [[1.0, 1.0], [10.0, 1.0], [3.0, 4.0]]}),
            ('MultiPointZ(1 1 3, 10 1 8, 3 4 9)',
             {'hasM': False, 'hasZ': True, 'points': [[1.0, 1.0, 3.0], [10.0, 1.0, 8.0], [3.0, 4.0, 9.0]]}),
            ('MultiPointM(1 1 22, 10 1 23, 3 4 24)', {'hasM': True, 'hasZ': False,
                                                      'points': [[1.0, 1.0, 22.0], [10.0, 1.0, 23.0],
                                                                 [3.0, 4.0, 24.0]]}),
            ('MultiPointZM(1 1 3 22, 10 1 8 24, 3 4 9 55)',
             {'hasM': True, 'hasZ': True,
              'points': [[1.0, 1.0, 3.0, 22.0], [10.0, 1.0, 8.0, 24.0], [3.0, 4.0, 9.0, 55.0]]}),
            ('LineString(1 2, 3 4)',
             {'hasM': False, 'hasZ': False, 'paths': [[[1.0, 2.0], [3.0, 4.0]]]}),
            ('LineString(1 2, 3 4, 5 6)',
             {'hasM': False, 'hasZ': False, 'paths': [[[1.0, 2.0], [3.0, 4.0], [5.0, 6.0]]]}),
            ('LineStringZ(1 2 11, 3 4 12)',
             {'hasM': False, 'hasZ': True, 'paths': [[[1.0, 2.0, 11.0], [3.0, 4.0, 12.0]]]}),
            ('LineStringM(1 2 21, 3 4 22)',
             {'hasM': True, 'hasZ': False, 'paths': [[[1.0, 2.0, 21.0], [3.0, 4.0, 22.0]]]}),
            ('LineStringZM(1 2 21 22, 3 4 31 33)',
             {'hasM': True, 'hasZ': True, 'paths': [[[1.0, 2.0, 21.0, 22.0], [3.0, 4.0, 31.0, 33.0]]]}),
            ('CircularString (0 0, 1 4, 3 3)',
             {'hasM': False, 'hasZ': False, 'curvePaths': [[[0.0, 0.0],
                                                            {'c': [[3.0, 3.0], [1.0, 4.0]]}]]}),
            ('CircularString (1 2, 5 4, 7 2.2, 10 0.1, 13 4)',
             {'hasM': False, 'hasZ': False, 'curvePaths': [[[1.0, 2.0],
                                                            {'c': [[7.0, 2.2], [5.0, 4.0]]},
                                                            {'c': [[13.0, 4.0], [10.0, 0.1]]}]]}),
            ('CircularString (1 2, 5 4, 7 2.2, 10 0.1, 13 4, 15 6)',  # invalid curve, has extra node which must be ignored
             {'hasM': False, 'hasZ': False, 'curvePaths': [[[1.0, 2.0],
                                                            {'c': [[7.0, 2.2], [5.0, 4.0]]},
                                                            {'c': [[13.0, 4.0], [10.0, 0.1]]}]]}),
            ('CircularStringZ (1 2 3, 5 4 4, 7 2.2 5)',
             {'hasM': False, 'hasZ': True, 'curvePaths': [[[1.0, 2.0, 3.0],
                                                           {'c': [[7.0, 2.2, 5.0], [5.0, 4.0, 4.0]]}]]}),
            ('CircularStringM (1 2 3, 5 4 4, 7 2.2 5)',
             {'hasM': True, 'hasZ': False, 'curvePaths': [[[1.0, 2.0, 3.0],
                                                           {'c': [[7.0, 2.2, 5.0], [5.0, 4.0, 4.0]]}]]}),
            ('CircularStringZM (1 2 3 11, 5 4 4 12, 7 2.2 5 13)',
             {'hasM': True, 'hasZ': True, 'curvePaths': [[[1.0, 2.0, 3.0, 11.0],
                                                          {'c': [[7.0, 2.2, 5.0, 13.0], [5.0, 4.0, 4.0, 12.0]]}]]}),

            ('CompoundCurve ((-1 -5, 1 2),CircularString (1 2, 5 4, 7 2.20, 10 0.1, 13 4),(13 4, 17 -6))',
             {'hasM': False, 'hasZ': False, 'curvePaths': [[[-1.0, -5.0],
                                                            [1.0, 2.0],
                                                            {'c': [[7.0, 2.2], [5.0, 4.0]]},
                                                            {'c': [[13.0, 4.0], [10.0, 0.1]]},
                                                            [17.0, -6.0]
                                                            ]]}),
            ('CompoundCurveZ ((-1 -5 3, 1 2 4),CircularStringZ (1 2 4, 5 4 5, 7 2.20 6, 10 0.1 7, 13 4 8),(13 4 8, 17 -6 9))',
             {'hasM': False, 'hasZ': True, 'curvePaths': [[[-1.0, -5.0, 3.0],
                                                           [1.0, 2.0, 4.0],
                                                           {'c': [[7.0, 2.2, 6.0], [5.0, 4.0, 5.0]]},
                                                           {'c': [[13.0, 4.0, 8.0], [10.0, 0.1, 7.0]]},
                                                           [17.0, -6.0, 9.0]
                                                           ]]}),
            (
                'CompoundCurveM ((-1 -5 3, 1 2 4),CircularStringM (1 2 4, 5 4 5, 7 2.20 6, 10 0.1 7, 13 4 8),(13 4 8, 17 -6 9))',
                {'hasM': True, 'hasZ': False, 'curvePaths': [[[-1.0, -5.0, 3.0],
                                                              [1.0, 2.0, 4.0],
                                                              {'c': [[7.0, 2.2, 6.0], [5.0, 4.0, 5.0]]},
                                                              {'c': [[13.0, 4.0, 8.0], [10.0, 0.1, 7.0]]},
                                                              [17.0, -6.0, 9.0]
                                                              ]]}),
            ('CompoundCurveZM ((-1 -5 3 11, 1 2 4 12),CircularStringZM (1 2 4 12, 5 4 5 13, 7 2.20 6 14, 10 0.1 7 15, 13 4 8 16),(13 4 8 16, 17 -6 9 17))',
             {'hasM': True, 'hasZ': True, 'curvePaths': [[[-1.0, -5.0, 3.0, 11.0],
                                                         [1.0, 2.0, 4.0, 12.0],
                                                         {'c': [[7.0, 2.2, 6.0, 14.0], [5.0, 4.0, 5.0, 13.0]]},
                                                         {'c': [[13.0, 4.0, 8.0, 16.0], [10.0, 0.1, 7.0, 15.0]]},
                                                         [17.0, -6.0, 9.0, 17.0]
                                                          ]]}),
            ('MultiCurve (CircularString (1 2, 5 4, 7 2.2, 10 0.1, 13 4),CircularString (-11 -3, 5 7, 10 -1))',
             {'hasM': False, 'hasZ': False, 'curvePaths': [[[1.0, 2.0],
                                                            {'c': [[7.0, 2.2], [5.0, 4.0]]},
                                                            {'c': [[13.0, 4.0], [10.0, 0.1]]}
                                                            ],
                                                           [
                                                               [-11.0, -3.0],
                                                               {'c': [[10.0, -1.0], [5.0, 7.0]]}
             ]]}),
            ('MultiCurveZ (CircularStringZ (1 2 10, 5 4 11, 7 2.2 12, 10 0.1 13, 13 4 14),CircularStringZ (-11 -3 20, 5 7 21, 10 -1 22))',
             {'hasM': False, 'hasZ': True, 'curvePaths': [[[1.0, 2.0, 10.0],
                                                           {'c': [[7.0, 2.2, 12.0], [5.0, 4.0, 11.0]]},
                                                           {'c': [[13.0, 4.0, 14.0], [10.0, 0.1, 13.0]]}
                                                           ],
                                                          [
                 [-11.0, -3.0, 20.0],
                 {'c': [[10.0, -1.0, 22.0], [5.0, 7.0, 21.0]]}
             ]]}),
            (
                'MultiCurveM (CircularStringM (1 2 10, 5 4 11, 7 2.2 12, 10 0.1 13, 13 4 14),CircularStringM (-11 -3 20, 5 7 21, 10 -1 22))',
                {'hasM': True, 'hasZ': False, 'curvePaths': [[[1.0, 2.0, 10.0],
                                                              {'c': [[7.0, 2.2, 12.0], [5.0, 4.0, 11.0]]},
                                                              {'c': [[13.0, 4.0, 14.0], [10.0, 0.1, 13.0]]}
                                                              ],
                                                             [
                    [-11.0, -3.0, 20.0],
                    {'c': [[10.0, -1.0, 22.0], [5.0, 7.0, 21.0]]}
                ]]}),
            (
                'MultiCurveZM (CircularStringZM (1 2 10 20, 5 4 11 21, 7 2.2 12 22, 10 0.1 13 23, 13 4 14 24),CircularStringZM (-11 -3 20 31, 5 7 21 32, 10 -1 22 33))',
                {'hasM': True, 'hasZ': True, 'curvePaths': [[[1.0, 2.0, 10.0, 20.0],
                                                             {'c': [[7.0, 2.2, 12.0, 22.0], [5.0, 4.0, 11.0, 21.0]]},
                                                             {'c': [[13.0, 4.0, 14.0, 24.0], [10.0, 0.1, 13.0, 23.0]]}
                                                             ],
                                                            [
                    [-11.0, -3.0, 20.0, 31.0],
                    {'c': [[10.0, -1.0, 22.0, 33.0], [5.0, 7.0, 21.0, 32.0]]}
                ]]}),
            ('MultiLineString((1 2, 3 4, 3 6), (11 12, 13 16, 18 19, 21 3))',
             {'hasM': False, 'hasZ': False,
              'paths': [[[1.0, 2.0], [3.0, 4.0], [3.0, 6.0]],
                        [[11.0, 12.0], [13.0, 16.0], [18.0, 19.0], [21.0, 3.0]]]}),
            ('MultiLineStringZ((1 2 11, 3 4 12, 3 6 13), (11 12 21, 13 16 22, 18 19 23, 21 3 24))',
             {'hasM': False, 'hasZ': True,
              'paths': [[[1.0, 2.0, 11.0], [3.0, 4.0, 12.0], [3.0, 6.0, 13.0]],
                        [[11.0, 12.0, 21.0], [13.0, 16.0, 22.0], [18.0, 19.0, 23.0], [21.0, 3.0, 24.0]]]}),
            ('MultiLineStringM((1 2 11, 3 4 12, 3 6 13), (11 12 21, 13 16 22, 18 19 23, 21 3 24))',
             {'hasM': True, 'hasZ': False,
              'paths': [[[1.0, 2.0, 11.0], [3.0, 4.0, 12.0], [3.0, 6.0, 13.0]],
                        [[11.0, 12.0, 21.0], [13.0, 16.0, 22.0], [18.0, 19.0, 23.0], [21.0, 3.0, 24.0]]]}),
            (
                'MultiLineStringZM((1 2 11 33, 3 4 12 34, 3 6 13 35), (11 12 21 31, 13 16 22 32, 18 19 23 33, 21 3 24 34))',
                {'hasM': True, 'hasZ': True,
                 'paths': [[[1.0, 2.0, 11.0, 33.0], [3.0, 4.0, 12.0, 34.0], [3.0, 6.0, 13.0, 35.0]],
                           [[11.0, 12.0, 21.0, 31.0], [13.0, 16.0, 22.0, 32.0], [18.0, 19.0, 23.0, 33.0],
                            [21.0, 3.0, 24.0, 34.0]]]}),
            ('Polygon((1 2, 10 2, 10 12, 1 12, 1 2))',
             {'hasM': False, 'hasZ': False,
              'rings': [[[1.0, 2.0], [1.0, 12.0], [10.0, 12.0], [10.0, 2.0], [1.0, 2.0]]]}),
            ('Polygon((1 2, 10 2, 10 12, 1 12, 1 2),(5 6, 6 6, 5 7, 5 6))',
             {'hasM': False, 'hasZ': False,
              'rings': [[[1.0, 2.0], [1.0, 12.0], [10.0, 12.0], [10.0, 2.0], [1.0, 2.0]],
                        [[5.0, 6.0], [6.0, 6.0], [5.0, 7.0], [5.0, 6.0]]]}),
            ('PolygonZ((1 2 33, 10 2 33, 10 12 33, 1 12 33, 1 2 33))',
             {'hasM': False, 'hasZ': True,
              'rings': [
                  [[1.0, 2.0, 33.0], [1.0, 12.0, 33.0], [10.0, 12.0, 33.0], [10.0, 2.0, 33.0], [1.0, 2.0, 33.0]]]}),
            ('PolygonM((1 2 44, 10 2 44, 10 12 44, 1 12 44, 1 2 44))',
             {'hasM': True, 'hasZ': False,
              'rings': [
                  [[1.0, 2.0, 44.0], [1.0, 12.0, 44.0], [10.0, 12.0, 44.0], [10.0, 2.0, 44.0], [1.0, 2.0, 44.0]]]}),
            ('PolygonZM((1 2 33 44, 10 2 33 44, 10 12 33 45, 1 12 33 46, 1 2 33 44))',
             {'hasM': True, 'hasZ': True, 'rings': [
                 [[1.0, 2.0, 33.0, 44.0], [1.0, 12.0, 33.0, 46.0], [10.0, 12.0, 33.0, 45.0], [10.0, 2.0, 33.0, 44.0],
                  [1.0, 2.0, 33.0, 44.0]]]}),
            ('CurvePolygon(CircularString(1 2, 10 2, 10 12, 1 12, 1 2))',
             {'hasM': False, 'hasZ': False,
              'curveRings': [[[1.0, 2.0], {'c': [[10.0, 12.0], [1.0, 12.0]]}, {'c': [[1.0, 2.0], [10.0, 2.0]]}]]}),
            ('CurvePolygon(CircularString(1 2, 10 2, 10 12, 1 12, 1 2), CircularString(21 2, 20 2, 20 12, 21 12, 21 2))',
             {'hasM': False, 'hasZ': False,
              'curveRings': [[[1.0, 2.0], {'c': [[10.0, 12.0], [1.0, 12.0]]}, {'c': [[1.0, 2.0], [10.0, 2.0]]}],
                             [[21.0, 2.0], {'c': [[20.0, 12.0], [21.0, 12.0]]}, {'c': [[21.0, 2.0], [20.0, 2.0]]}]]}),

            (
                'CurvePolygon (CompoundCurve ((0 -23.43778, 0 -15.43778, 0 23.43778),CircularString (0 23.43778, -45 100, -90 23.43778),(-90 23.43778, -90 -23.43778),CircularString (-90 -23.43778, -45 -16.43778, 0 -23.43778)),CompoundCurve (CircularString (-30 0, -48 -12, -60 0, -48 -6, -30 0)))',
                {'hasM': False, 'hasZ': False,
                 'curveRings': [[[0.0, -23.43778], {'c': [[-90.0, -23.43778], [-45.0, -16.43778]]}, [-90.0, 23.43778],
                                 {'c': [[0.0, 23.43778], [-45.0, 100.0]]}, [0.0, -15.43778], [0.0, -23.43778]],
                                [[-30.0, 0.0], {'c': [[-60.0, 0.0], [-48.0, -6.0]]}, {'c': [[-30.0, 0.0], [-48.0, -12.0]]}]]
                 }),



            ('MultiPolygon(((1 2, 10 2, 10 12, 1 12, 1 2)),((20 10, 22 10, 20 13, 20 10)))',
             {'hasM': False, 'hasZ': False,
              'rings': [[[1.0, 2.0], [1.0, 12.0], [10.0, 12.0], [10.0, 2.0], [1.0, 2.0]],
                        [[20.0, 10.0], [20.0, 13.0], [22.0, 10.0], [20.0, 10.0]]]}),
            ('MultiPolygon(((1 2, 10 2, 10 12, 1 12, 1 2),(5 6, 6 6, 5 7, 5 6)),((20 10, 22 10, 20 13, 20 10)))',
             {'hasM': False, 'hasZ': False,
              'rings': [[[1.0, 2.0], [1.0, 12.0], [10.0, 12.0], [10.0, 2.0], [1.0, 2.0]],
                        [[5.0, 6.0], [6.0, 6.0], [5.0, 7.0], [5.0, 6.0]],
                        [[20.0, 10.0], [20.0, 13.0], [22.0, 10.0], [20.0, 10.0]]]}),

            ('MultiSurface(CurvePolygon(CircularString(1 2, 10 2, 10 12, 1 12, 1 2)),CurvePolygon(CircularString(1 2, 10 2, 10 12, 1 12, 1 2), CircularString(21 2, 20 2, 20 12, 21 12, 21 2)))',
             {'hasM': False, 'hasZ': False,
              'curveRings': [
                  [[1.0, 2.0], {'c': [[10.0, 12.0], [1.0, 12.0]]}, {'c': [[1.0, 2.0], [10.0, 2.0]]}],
                  [[1.0, 2.0], {'c': [[10.0, 12.0], [1.0, 12.0]]}, {'c': [[1.0, 2.0], [10.0, 2.0]]}],
                  [[21.0, 2.0], {'c': [[20.0, 12.0], [21.0, 12.0]]}, {'c': [[21.0, 2.0], [20.0, 2.0]]}]]})
        ]

        for test_wkt, expected in tests:
            input = QgsGeometry.fromWkt(test_wkt)
            json = QgsArcGisRestUtils.geometryToJson(input)
            self.assertEqual(json, expected, f'Mismatch for {test_wkt}')


if __name__ == '__main__':
    unittest.main()
