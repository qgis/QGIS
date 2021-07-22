# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsAidwIterpolation.

Requires a GPU with OpenCL enabled.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2021 by Alessandro Pasotti'
__date__ = '15/07/2021'
__copyright__ = 'Copyright 2021, The QGIS Project'

import os

import qgis  # NOQA

from qgis.PyQt.QtCore import QTemporaryDir, QCoreApplication
from qgis.analysis import QgsAidwInterpolation, QgsIDWInterpolator
from qgis.core import (
    Qgis,
    QgsVectorLayer,
    QgsRasterLayer,
    QgsFeature,
    QgsGeometry,
    QgsRectangle,
    QgsSettings,
    QgsPointXY,
    QgsRaster,
    QgsRasterFileWriter,
    QgsCoordinateReferenceSystem,
)
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsAidwIterpolation(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""

        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain(cls.__name__)
        QCoreApplication.setApplicationName(cls.__name__)
        start_app()
        cls.tempDir = QTemporaryDir()
        QgsSettings().setValue("OpenClEnabled", True, QgsSettings.Core)

    def testInterpolator(self):

        data_layer = QgsVectorLayer('memory?geometry=Point&crs=EPSG:4326&field=name:string(0,0)&field=my_attr:double(0,0)', 'data', 'memory')
        self.assertTrue(data_layer.isValid())

        for feature_data in (
            # dummy attr, x, y, value attr
            ('1', 0, 0, 1),
            ('2', 2, 0, 2),
            ('3', 0, 2, 3),
            ('4', 2, 2, 4),
        ):
            f = QgsFeature(data_layer.fields())
            f.setAttribute(0, feature_data[0])
            f.setAttribute(1, feature_data[3])
            f.setGeometry(QgsGeometry.fromWkt('point({} {})'.format(feature_data[1], feature_data[2])))
            self.assertTrue(data_layer.dataProvider().addFeatures([f]))

        interpolated_path = os.path.join(self.tempDir.path(), 'interpolated.tiff')
        writer = QgsRasterFileWriter(interpolated_path)
        extent = QgsRectangle(-0.5, -0.5, 2.5, 2.5)
        writer.createOneBandRaster(Qgis.Float64, 3, 3, extent, QgsCoordinateReferenceSystem('EPSG:4326'))
        del writer

        interpolated_layer = QgsRasterLayer(interpolated_path, 'interpolated')
        self.assertEqual(interpolated_layer.crs().authid(), 'EPSG:4326')
        self.assertTrue(interpolated_layer.isValid())
        self.assertEqual(interpolated_layer.extent(), QgsRectangle(-0.5, -0.5, 2.5, 2.5))

        interpolator = QgsAidwInterpolation(data_layer, 'my_attr', interpolated_layer)

        interpolator.process()

        # Reload the layer
        interpolated_layer = QgsRasterLayer(interpolated_path, 'interpolated')
        self.assertTrue(interpolated_layer.isValid())

        # Expected values, sample along diagonal
        sample_points = [(i, i) for i in range(3)]

        layer_data = QgsIDWInterpolator.LayerData()
        layer_data.source = data_layer
        layer_data.interpolationAttribute = 1

        control_interpolator = QgsIDWInterpolator([layer_data])

        expected_values = []
        for sample_point in sample_points:
            ret, result = control_interpolator.interpolatePoint(*sample_point)
            self.assertEqual(ret, 0)
            expected_values.append(result)

        # print(expected_values)

        # Print whole raster
        for y in range(3):
            row = [interpolated_layer.dataProvider().identify(QgsPointXY(x, y), QgsRaster.IdentifyFormatValue).results()[1] for x in range(3)]
            print(row)

        # Check iterpolation results
        for i in range(3):
            expected_value = expected_values[i]
            calculated_value = interpolated_layer.dataProvider().identify(QgsPointXY(*sample_points[i]), QgsRaster.IdentifyFormatValue).results()[1]
            self.assertAlmostEqual(expected_value, calculated_value, 2)


if __name__ == '__main__':
    unittest.main()
