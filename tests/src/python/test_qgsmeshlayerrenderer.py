"""QGIS Unit tests for QgsMeshLayerRenderer.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import os
import unittest

from qgis.PyQt.QtCore import QSize
from qgis.core import Qgis, QgsDoubleRange, QgsMapSettings, QgsMeshLayer
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
            os.path.join(unitTestDataPath(), "mesh", "quad_flower.2dm"), "mdal", "mdal"
        )
        self.assertTrue(mesh_layer.isValid())

        # set layer as elevation enabled
        mesh_layer.elevationProperties().setMode(
            Qgis.MeshElevationMode.FixedElevationRange
        )
        mesh_layer.elevationProperties().setFixedRange(QgsDoubleRange(33, 38))

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
                "No Z range filter on map settings, fixed elevation range layer",
                "elevation_no_filter",
                map_settings,
            )
        )

        # map settings range includes layer's range
        map_settings.setZRange(QgsDoubleRange(30, 35))
        self.assertTrue(
            self.render_map_settings_check(
                "Z range filter on map settings includes layers fixed range",
                "fixed_elevation_range_included",
                map_settings,
            )
        )

        # map settings range excludes layer's range
        map_settings.setZRange(QgsDoubleRange(130, 135))
        self.assertTrue(
            self.render_map_settings_check(
                "Z range filter on map settings outside of layers fixed range",
                "fixed_elevation_range_excluded",
                map_settings,
            )
        )

    def test_render_fixed_range_per_group_with_z_range_filter(self):
        """
        Test rendering a mesh with a fixed range per group when
        map settings has a z range filter
        """
        layer = QgsMeshLayer(
            self.get_test_data_path("mesh/netcdf_parent_quantity.nc").as_posix(),
            "mesh",
            "mdal",
        )
        self.assertTrue(layer.isValid())

        # set layer as elevation enabled
        layer.elevationProperties().setMode(Qgis.MeshElevationMode.FixedRangePerGroup)
        layer.elevationProperties().setFixedRangePerGroup(
            {
                1: QgsDoubleRange(33, 38),
                2: QgsDoubleRange(35, 40),
                3: QgsDoubleRange(40, 48),
            }
        )

        map_settings = QgsMapSettings()
        map_settings.setOutputSize(QSize(400, 400))
        map_settings.setOutputDpi(96)
        map_settings.setDestinationCrs(layer.crs())
        map_settings.setExtent(layer.extent())
        map_settings.setLayers([layer])

        # no filter on map settings
        map_settings.setZRange(QgsDoubleRange())
        self.assertTrue(
            self.render_map_settings_check(
                "No Z range filter on map settings, elevation range per group",
                "elevation_range_per_group_no_filter",
                map_settings,
            )
        )

        # map settings range matches group 3 only
        map_settings.setZRange(QgsDoubleRange(40.5, 49.5))
        self.assertTrue(
            self.render_map_settings_check(
                "Z range filter on map settings matches group 3 only",
                "elevation_range_per_group_match3",
                map_settings,
            )
        )

        # map settings range matches group 1 and 2
        map_settings.setZRange(QgsDoubleRange(33, 39.5))
        self.assertTrue(
            self.render_map_settings_check(
                "Z range filter on map settings matches group 1 and 2",
                "elevation_range_per_group_match1and2",
                map_settings,
            )
        )

        # map settings range excludes layer's range
        map_settings.setZRange(QgsDoubleRange(130, 135))
        self.assertTrue(
            self.render_map_settings_check(
                "Z range filter on map settings outside of layer group ranges",
                "fixed_elevation_range_excluded",
                map_settings,
            )
        )


if __name__ == "__main__":
    unittest.main()
