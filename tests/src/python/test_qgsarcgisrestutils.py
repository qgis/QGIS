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

from qgis.PyQt.QtCore import QDate, QDateTime, Qt, QTime, QTimeZone, QVariant
from qgis.core import (
    NULL,
    QgsArcGisRestContext,
    QgsArcGisRestUtils,
    QgsCoordinateReferenceSystem,
    QgsFeature,
    QgsField,
    QgsFieldConstraints,
    QgsFields,
    QgsGeometry,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsArcGisRestUtils(QgisTestCase):

    def test_json_to_geometry(self):
        tests = [('esriGeometryPolyline',
                  {'curvePaths': [[[148.38318344186538, -17.139016173514584], {'c': [[150.99306515432036, -16.733335282152847], [150.46567981044387, -15.976063814628597]]}, {'c': [[153.67056039605595, -16.314131721058928], [153.02147048339893, -17.855719902399045]]}, [155.13101252753447, -14.556180364908652], [150.533293548165, -13.636636042108115], {'c': [[147.6394350119692, -14.353339106615321], [147.99102541902195, -12.83879629512887]]}]],
                   'hasM': False, 'hasZ': False},
                  'MultiCurve (CompoundCurve (CircularString (148.38318 -17.13902, 150.46568 -15.97606, 150.99307 -16.73334),CircularString (150.99307 -16.73334, 153.02147 -17.85572, 153.67056 -16.31413),(153.67056 -16.31413, 155.13101 -14.55618, 150.53329 -13.63664),CircularString (150.53329 -13.63664, 147.99103 -12.8388, 147.63944 -14.35334)))')]
        for type_string, json, expected in tests:
            geometry, _ = QgsArcGisRestUtils.convertGeometry(json, type_string, json.get('hasM'), json.get('hasZ'))
            self.assertEqual(geometry.asWkt(5), expected)

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
            ('CircularString (1 2, 5 4, 7 2.2, 10 0.1, 13 4, 15 6)',
             # invalid curve, has extra node which must be ignored
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
            # compound curve with no curved components should return paths, not curvePaths
            ('CompoundCurve ((-1 -5, 1 2))',
             {'hasM': False, 'hasZ': False, 'paths': [[[-1.0, -5.0], [1.0, 2.0]]]}),
            ('CompoundCurve ((-1 -5, 1 2),CircularString (1 2, 5 4, 7 2.20, 10 0.1, 13 4),(13 4, 17 -6))',
             {'hasM': False, 'hasZ': False, 'curvePaths': [[[-1.0, -5.0],
                                                            [1.0, 2.0],
                                                            {'c': [[7.0, 2.2], [5.0, 4.0]]},
                                                            {'c': [[13.0, 4.0], [10.0, 0.1]]},
                                                            [17.0, -6.0]
                                                            ]]}),
            (
                'CompoundCurveZ ((-1 -5 3, 1 2 4),CircularStringZ (1 2 4, 5 4 5, 7 2.20 6, 10 0.1 7, 13 4 8),(13 4 8, 17 -6 9))',
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
            (
                'CompoundCurveZM ((-1 -5 3 11, 1 2 4 12),CircularStringZM (1 2 4 12, 5 4 5 13, 7 2.20 6 14, 10 0.1 7 15, 13 4 8 16),(13 4 8 16, 17 -6 9 17))',
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
            (
                'MultiCurveZ (CircularStringZ (1 2 10, 5 4 11, 7 2.2 12, 10 0.1 13, 13 4 14),CircularStringZ (-11 -3 20, 5 7 21, 10 -1 22))',
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
                                                                {'c': [[10.0, -1.0, 22.0, 33.0],
                                                                       [5.0, 7.0, 21.0, 32.0]]}
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
            (
                'CurvePolygon(CircularString(1 2, 10 2, 10 12, 1 12, 1 2), CircularString(21 2, 20 2, 20 12, 21 12, 21 2))',
                {'hasM': False, 'hasZ': False,
                 'curveRings': [[[1.0, 2.0], {'c': [[10.0, 12.0], [1.0, 12.0]]}, {'c': [[1.0, 2.0], [10.0, 2.0]]}],
                                [[21.0, 2.0], {'c': [[20.0, 12.0], [21.0, 12.0]]}, {'c': [[21.0, 2.0], [20.0, 2.0]]}]]}),

            (
                'CurvePolygon (CompoundCurve ((0 -23.43778, 0 -15.43778, 0 23.43778),CircularString (0 23.43778, -45 100, -90 23.43778),(-90 23.43778, -90 -23.43778),CircularString (-90 -23.43778, -45 -16.43778, 0 -23.43778)),CompoundCurve (CircularString (-30 0, -48 -12, -60 0, -48 -6, -30 0)))',
                {'hasM': False, 'hasZ': False,
                 'curveRings': [[[0.0, -23.43778], {'c': [[-90.0, -23.43778], [-45.0, -16.43778]]}, [-90.0, 23.43778],
                                 {'c': [[0.0, 23.43778], [-45.0, 100.0]]}, [0.0, -15.43778], [0.0, -23.43778]],
                                [[-30.0, 0.0], {'c': [[-60.0, 0.0], [-48.0, -6.0]]},
                                 {'c': [[-30.0, 0.0], [-48.0, -12.0]]}]]
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

            (
                'MultiSurface(CurvePolygon(CircularString(1 2, 10 2, 10 12, 1 12, 1 2)),CurvePolygon(CircularString(1 2, 10 2, 10 12, 1 12, 1 2), CircularString(21 2, 20 2, 20 12, 21 12, 21 2)))',
                {'hasM': False, 'hasZ': False,
                 'curveRings': [
                     [[1.0, 2.0], {'c': [[10.0, 12.0], [1.0, 12.0]]}, {'c': [[1.0, 2.0], [10.0, 2.0]]}],
                     [[1.0, 2.0], {'c': [[10.0, 12.0], [1.0, 12.0]]}, {'c': [[1.0, 2.0], [10.0, 2.0]]}],
                     [[21.0, 2.0], {'c': [[20.0, 12.0], [21.0, 12.0]]}, {'c': [[21.0, 2.0], [20.0, 2.0]]}]]})
        ]

        context = QgsArcGisRestContext()
        for test_wkt, expected in tests:
            input = QgsGeometry.fromWkt(test_wkt)
            json = QgsArcGisRestUtils.geometryToJson(input, context)
            self.assertEqual(json, expected, f'Mismatch for {test_wkt}')

    def test_crs_conversion(self):
        self.assertEqual(QgsArcGisRestUtils.crsToJson(QgsCoordinateReferenceSystem()), {})
        self.assertEqual(QgsArcGisRestUtils.crsToJson(QgsCoordinateReferenceSystem('EPSG:4326')), {'wkid': '4326'})
        self.assertEqual(QgsArcGisRestUtils.crsToJson(QgsCoordinateReferenceSystem('EPSG:3857')), {'wkid': '3857'})
        self.assertEqual(QgsArcGisRestUtils.crsToJson(QgsCoordinateReferenceSystem('ESRI:53079')), {'wkid': '53079'})

        # should be represented via wkt
        self.assertTrue(QgsArcGisRestUtils.crsToJson(QgsCoordinateReferenceSystem('IGNF:WGS84'))['wkt'])

    def test_geometry_with_crs(self):
        geom = QgsGeometry.fromWkt('Point( 1 2)')
        context = QgsArcGisRestContext()
        self.assertEqual(QgsArcGisRestUtils.geometryToJson(geom, context, QgsCoordinateReferenceSystem('EPSG:4326')),
                         {'spatialReference': {'wkid': '4326'}, 'x': 1.0, 'y': 2.0})
        self.assertEqual(QgsArcGisRestUtils.geometryToJson(geom, context, QgsCoordinateReferenceSystem('EPSG:3857')),
                         {'spatialReference': {'wkid': '3857'}, 'x': 1.0, 'y': 2.0})

    def test_feature_to_json(self):
        test_fields = QgsFields()

        attributes = []

        test_fields.append(QgsField('a_string_field', QVariant.String))
        attributes.append('my string value')

        test_fields.append(QgsField('a_int_field', QVariant.Int))
        attributes.append(5)

        test_fields.append(QgsField('a_double_field', QVariant.Double))
        attributes.append(5.5)

        test_fields.append(QgsField('a_boolean_field', QVariant.Bool))
        attributes.append(True)

        test_fields.append(QgsField('a_datetime_field', QVariant.DateTime))
        attributes.append(QDateTime(QDate(2022, 3, 4), QTime(12, 13, 14), Qt.TimeSpec.UTC))

        test_fields.append(QgsField('a_date_field', QVariant.Date))
        attributes.append(QDate(2022, 3, 4))

        test_fields.append(QgsField('a_null_value', QVariant.String))
        attributes.append(NULL)

        test_feature = QgsFeature(test_fields)
        test_feature.setAttributes(attributes)
        test_feature.setGeometry(QgsGeometry.fromWkt('Point(1 2)'))

        context = QgsArcGisRestContext()
        context.setTimeZone(QTimeZone.utc())
        res = QgsArcGisRestUtils.featureToJson(test_feature, context)
        self.assertEqual(res, {'attributes': {'a_boolean_field': True,
                                              'a_datetime_field': 1646395994000,
                                              'a_date_field': 1646352000000,
                                              'a_double_field': 5.5,
                                              'a_int_field': 5,
                                              'a_string_field': 'my%20string%20value',
                                              'a_null_value': None},
                               'geometry': {'x': 1.0, 'y': 2.0}})
        # without geometry
        res = QgsArcGisRestUtils.featureToJson(test_feature, context, flags=QgsArcGisRestUtils.FeatureToJsonFlags(QgsArcGisRestUtils.FeatureToJsonFlag.IncludeNonObjectIdAttributes))
        self.assertEqual(res, {'attributes': {'a_boolean_field': True,
                                              'a_datetime_field': 1646395994000,
                                              'a_date_field': 1646352000000,
                                              'a_double_field': 5.5,
                                              'a_int_field': 5,
                                              'a_string_field': 'my%20string%20value',
                                              'a_null_value': None}})
        # without attributes
        context.setObjectIdFieldName('a_int_field')
        res = QgsArcGisRestUtils.featureToJson(test_feature, context, flags=QgsArcGisRestUtils.FeatureToJsonFlags(QgsArcGisRestUtils.FeatureToJsonFlag.IncludeGeometry))
        self.assertEqual(res, {'attributes': {
            'a_int_field': 5},
            'geometry': {'x': 1.0, 'y': 2.0}})

        # with special characters

        attributes[0] = 'aaa" \' , . - ; : ä ö ü è é à ? + & \\ /'
        test_feature.setAttributes(attributes)
        res = QgsArcGisRestUtils.featureToJson(test_feature, context, flags=QgsArcGisRestUtils.FeatureToJsonFlags(QgsArcGisRestUtils.FeatureToJsonFlag.IncludeNonObjectIdAttributes))
        self.assertEqual(res, {'attributes': {'a_boolean_field': True,
                                              'a_datetime_field': 1646395994000,
                                              'a_date_field': 1646352000000,
                                              'a_double_field': 5.5,
                                              'a_int_field': 5,
                                              'a_string_field': """aaa%5C%22%20'%20%2C%20.%20-%20%3B%20%3A%20%C3%A4%20%C3%B6%20%C3%BC%20%C3%A8%20%C3%A9%20%C3%A0%20%3F%20%2B%20%26%20%5C%5C%20%2F""",
                                              'a_null_value': None}})

    def test_field_to_json(self):
        field = QgsField('my name', QVariant.LongLong)
        field.setAlias('my alias')
        self.assertEqual(QgsArcGisRestUtils.fieldDefinitionToJson(field), {'alias': 'my alias', 'editable': True, 'name': 'my name', 'nullable': True, 'type': 'esriFieldTypeInteger'})
        field = QgsField('my name', QVariant.Int)
        self.assertEqual(QgsArcGisRestUtils.fieldDefinitionToJson(field), {'editable': True, 'name': 'my name', 'nullable': True, 'type': 'esriFieldTypeSmallInteger'})
        field = QgsField('my name', QVariant.Double)
        self.assertEqual(QgsArcGisRestUtils.fieldDefinitionToJson(field), {'editable': True, 'name': 'my name', 'nullable': True, 'type': 'esriFieldTypeDouble'})
        field = QgsField('my name', QVariant.String)
        self.assertEqual(QgsArcGisRestUtils.fieldDefinitionToJson(field), {'editable': True, 'name': 'my name', 'nullable': True, 'type': 'esriFieldTypeString'})
        field = QgsField('my name', QVariant.DateTime)
        self.assertEqual(QgsArcGisRestUtils.fieldDefinitionToJson(field), {'editable': True, 'name': 'my name', 'nullable': True, 'type': 'esriFieldTypeDate'})
        field = QgsField('my name', QVariant.ByteArray)
        self.assertEqual(QgsArcGisRestUtils.fieldDefinitionToJson(field), {'editable': True, 'name': 'my name', 'nullable': True, 'type': 'esriFieldTypeBlob'})

        # unsupported type
        field = QgsField('my name', QVariant.Time)
        self.assertEqual(QgsArcGisRestUtils.fieldDefinitionToJson(field), {'editable': True, 'name': 'my name', 'nullable': True, 'type': 'esriFieldTypeString'})

        # not nullable
        field = QgsField('my name', QVariant.Int)
        field_constraints = field.constraints()
        field_constraints.setConstraint(QgsFieldConstraints.Constraint.ConstraintNotNull)
        field.setConstraints(field_constraints)
        self.assertEqual(QgsArcGisRestUtils.fieldDefinitionToJson(field), {'editable': True, 'name': 'my name', 'nullable': False, 'type': 'esriFieldTypeSmallInteger'})


if __name__ == '__main__':
    unittest.main()
