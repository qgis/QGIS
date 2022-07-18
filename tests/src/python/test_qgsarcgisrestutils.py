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
from qgis.PyQt.QtCore import (
    Qt,
    QVariant,
    QDate,
    QTime,
    QDateTime,
    QTimeZone
)
from qgis.core import (
    QgsGeometry,
    QgsArcGisRestUtils,
    QgsCoordinateReferenceSystem,
    QgsFields,
    QgsField,
    QgsFeature,
    NULL
)
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsArcGisRestUtils(unittest.TestCase):

    def test_json_to_geometry(self):
        tests = [('esriGeometryPolyline',
                  {'curvePaths': [[[148.38318344186538, -17.139016173514584], {'c': [[150.99306515432036, -16.733335282152847], [150.46567981044387, -15.976063814628597]]}, {'c': [[153.67056039605595, -16.314131721058928], [153.02147048339893, -17.855719902399045]]}, [155.13101252753447, -14.556180364908652], [150.533293548165, -13.636636042108115], {'c': [[147.6394350119692, -14.353339106615321], [147.99102541902195, -12.83879629512887]]}]],
                   'hasM': False, 'hasZ': False},
                  'MultiCurve (CompoundCurve (CircularString (148.38318 -17.13902, 150.46568 -15.97606, 150.99307 -16.73334),CircularString (150.99307 -16.73334, 153.02147 -17.85572, 153.67056 -16.31413),(153.67056 -16.31413, 155.13101 -14.55618, 150.53329 -13.63664),CircularString (150.53329 -13.63664, 147.99103 -12.8388, 147.63944 -14.35334)))')]
        for type_string, json, expected in tests:
            geometry, _ = QgsArcGisRestUtils.convertGeometry(json, type_string, json.get('hasM'), json.get('hasZ'))
            self.assertEqual(geometry.asWkt(5), expected)


if __name__ == '__main__':
    unittest.main()
