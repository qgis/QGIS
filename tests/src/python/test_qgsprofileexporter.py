"""QGIS Unit tests for QgsRasterLayer profile generation

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '18/03/2022'
__copyright__ = 'Copyright 2022, The QGIS Project'

import os

import qgis  # NOQA
from qgis.core import (
    Qgis,
    QgsCoordinateReferenceSystem,
    QgsLineString,
    QgsProfileIdentifyContext,
    QgsProfilePoint,
    QgsProfileRequest,
    QgsProfileSnapContext,
    QgsRasterLayer,
    QgsProfileExporter
)
from qgis.testing import start_app, unittest

from utilities import unitTestDataPath

start_app()


class TestQgsProfileExporter(unittest.TestCase):

    def testExport(self):
        rl = QgsRasterLayer(os.path.join(unitTestDataPath(), '3d', 'dtm.tif'), 'DTM')
        self.assertTrue(rl.isValid())
        rl.elevationProperties().setEnabled(True)

        curve = QgsLineString()
        curve.fromWkt('LineString (-348095.18706532847136259 6633687.0235139261931181, -347271.57799367723055184 6633093.13086318597197533, -346140.60267287614988163 6632697.89590711053460836, -345777.013075890194159 6631575.50219972990453243)')
        req = QgsProfileRequest(curve)
        req.setCrs(QgsCoordinateReferenceSystem('EPSG:3857'))

        exporter = QgsProfileExporter(
            [rl],
            req, Qgis.ProfileExportType.Features3D)

        exporter.run()

        layers = exporter.toLayers()
        self.assertEqual(len(layers), 1)

        output_layer = layers[0]
        self.assertEqual(output_layer.wkbType(), Qgis.WkbType.LineStringZ)

        features = [f for f in output_layer.getFeatures()]

        self.assertEqual(len(features), 1)
        self.assertEqual(features[0][0], rl.id())
        self.assertEqual(features[0].geometry().constGet().numPoints(), 1394)
        self.assertEqual(features[0].geometry().constGet().pointN(0).asWkt(-2), 'PointZ (-348100 6633700 200)')
        self.assertEqual(features[0].geometry().constGet().pointN(1393).asWkt(-2), 'PointZ (-345800 6631600 100)')


if __name__ == '__main__':
    unittest.main()
