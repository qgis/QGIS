"""QGIS Unit tests for QgsMeshLayerRenderer.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import os
import unittest

from qgis.PyQt.QtCore import QSize
from qgis.core import (
    Qgis,
    QgsDoubleRange,
    QgsMapSettings,
    QgsMeshLayer
)
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()


class TestQgsMeshLayerLabeling(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "mesh"

    def test_render_fixed_elevation_range_with_z_range_filter(self):
        """
        Test rendering a mesh with a fixed elevation range when
        map settings has a z range filtrer
        """
        mesh_layer = QgsMeshLayer(
            os.path.join(unitTestDataPath(), 'mesh', 'quad_flower.2dm'),
            'mdal', 'mdal')
        self.assertTrue(mesh_layer.isValid())

        # set layer as elevation enabled
        mesh_layer.elevationProperties().setMode(
            Qgis.MeshElevationMode.FixedElevationRange
        )
        mesh_layer.elevationProperties().setFixedRange(
            QgsDoubleRange(33, 38)
        )

        map_settings = QgsMapSettings()
        map_settings.setOutputSize(QSize(400, 400))
        map_settings.setOutputDpi(96)
        map_settings.setDestinationCrs(mesh_layer.crs())
        map_settings.setExtent(mesh_layer.extent())
        map_settings.setLayers([mesh_layer])

        # no filter on map settings
        map_settings.setZRange(QgsDoubleRange())
        self.assertTrue(
            self.render_map_settings_check(
                'No Z range filter on map settings, fixed elevation range layer',
                'elevation_no_filter',
                map_settings)
        )

        # map settings range includes layer's range
        map_settings.setZRange(QgsDoubleRange(30, 35))
        self.assertTrue(
            self.render_map_settings_check(
                'Z range filter on map settings includes layers fixed range',
                'fixed_elevation_range_included',
                map_settings)
        )

        # map settings range excludes layer's range
        map_settings.setZRange(QgsDoubleRange(130, 135))
        self.assertTrue(
            self.render_map_settings_check(
                'Z range filter on map settings outside of layers fixed range',
                'fixed_elevation_range_excluded',
                map_settings)
        )


if __name__ == '__main__':
    unittest.main()
