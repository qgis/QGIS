"""QGIS Unit tests for QgsCoordinateReferenceSystemModel.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "(C) 2022 by Nyall Dawson"
__date__ = "12/07/2023"
__copyright__ = "Copyright 2023, The QGIS Project"


from qgis.PyQt.QtCore import Qt, QModelIndex
from qgis.core import (
    Qgis,
    QgsApplication,
    QgsCoordinateReferenceSystem,
    QgsCoordinateReferenceSystemUtils,
)
from qgis.gui import (
    QgsCoordinateReferenceSystemModel,
    QgsCoordinateReferenceSystemProxyModel,
)

import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsCoordinateReferenceSystemModel(QgisTestCase):

    def test_model(self):
        model = QgsCoordinateReferenceSystemModel()
        # top level items -- we expect to find Projected, Geographic (2D), Geographic (3D),
        # Compound, Vertical amongst others
        self.assertGreaterEqual(model.rowCount(QModelIndex()), 5)
        top_level_items = [
            model.data(model.index(row, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
            for row in range(model.rowCount(QModelIndex()))
        ]
        self.assertIn("Projected", top_level_items)
        self.assertIn("Geographic (2D)", top_level_items)
        self.assertIn("Geographic (3D)", top_level_items)
        self.assertIn("Compound", top_level_items)
        self.assertIn("Vertical", top_level_items)

        # projection methods should not be at top level
        self.assertNotIn("Cassini", top_level_items)

        # user and custom groups should not be created until required
        self.assertNotIn("User-defined", top_level_items)
        self.assertNotIn("Custom", top_level_items)

        # check group ids
        top_level_item_group_ids = [
            model.data(
                model.index(row, 0, QModelIndex()),
                QgsCoordinateReferenceSystemModel.Roles.RoleGroupId,
            )
            for row in range(model.rowCount(QModelIndex()))
        ]
        self.assertIn("Projected", top_level_item_group_ids)
        self.assertIn("Geographic2d", top_level_item_group_ids)
        self.assertIn("Geographic3d", top_level_item_group_ids)
        self.assertIn("Compound", top_level_item_group_ids)
        self.assertIn("Vertical", top_level_item_group_ids)

        # find WGS84 in Geographic2d group
        geographic_2d_index = [
            model.index(row, 0, QModelIndex())
            for row in range(model.rowCount(QModelIndex()))
            if model.data(
                model.index(row, 0, QModelIndex()),
                QgsCoordinateReferenceSystemModel.Roles.RoleGroupId,
            )
            == "Geographic2d"
        ][0]

        # for proj 9, there's > 1300 crs in this group
        self.assertGreaterEqual(model.rowCount(geographic_2d_index), 1000)

        wgs84_index = [
            model.index(row, 0, geographic_2d_index)
            for row in range(model.rowCount(geographic_2d_index))
            if model.data(
                model.index(row, 0, geographic_2d_index),
                QgsCoordinateReferenceSystemModel.Roles.RoleAuthId,
            )
            == "EPSG:4326"
        ][0]
        # test model roles
        self.assertEqual(model.data(wgs84_index, Qt.ItemDataRole.DisplayRole), "WGS 84")
        self.assertEqual(
            model.data(
                model.index(wgs84_index.row(), 1, wgs84_index.parent()),
                Qt.ItemDataRole.DisplayRole,
            ),
            "EPSG:4326",
        )
        self.assertEqual(
            model.data(wgs84_index, QgsCoordinateReferenceSystemModel.Roles.RoleName),
            "WGS 84",
        )
        self.assertFalse(
            model.data(
                wgs84_index, QgsCoordinateReferenceSystemModel.Roles.RoleDeprecated
            )
        )
        # the proj and wkt roles are only available for non-standard CRS
        self.assertFalse(
            model.data(wgs84_index, QgsCoordinateReferenceSystemModel.Roles.RoleWkt)
        )
        self.assertFalse(
            model.data(wgs84_index, QgsCoordinateReferenceSystemModel.Roles.RoleProj)
        )

        # check that same result is returned by authIdToIndex
        self.assertEqual(model.authIdToIndex("EPSG:4326"), wgs84_index)

        # find EPSG:4329 in Geographic3d group (also tests a deprecated CRS)
        geographic_3d_index = [
            model.index(row, 0, QModelIndex())
            for row in range(model.rowCount(QModelIndex()))
            if model.data(
                model.index(row, 0, QModelIndex()),
                QgsCoordinateReferenceSystemModel.Roles.RoleGroupId,
            )
            == "Geographic3d"
        ][0]

        # for proj 9, there's > 200 crs in this group
        self.assertGreaterEqual(model.rowCount(geographic_3d_index), 200)

        epsg_4329_index = [
            model.index(row, 0, geographic_3d_index)
            for row in range(model.rowCount(geographic_3d_index))
            if model.data(
                model.index(row, 0, geographic_3d_index),
                QgsCoordinateReferenceSystemModel.Roles.RoleAuthId,
            )
            == "EPSG:4329"
        ][0]
        # test model roles
        self.assertEqual(
            model.data(epsg_4329_index, Qt.ItemDataRole.DisplayRole), "WGS 84 (3D)"
        )
        self.assertEqual(
            model.data(
                model.index(epsg_4329_index.row(), 1, epsg_4329_index.parent()),
                Qt.ItemDataRole.DisplayRole,
            ),
            "EPSG:4329",
        )
        self.assertEqual(
            model.data(
                epsg_4329_index, QgsCoordinateReferenceSystemModel.Roles.RoleName
            ),
            "WGS 84 (3D)",
        )
        self.assertTrue(
            model.data(
                epsg_4329_index, QgsCoordinateReferenceSystemModel.Roles.RoleDeprecated
            )
        )
        # the proj and wkt roles are only available for non-standard CRS
        self.assertFalse(
            model.data(epsg_4329_index, QgsCoordinateReferenceSystemModel.Roles.RoleWkt)
        )
        self.assertFalse(
            model.data(
                epsg_4329_index, QgsCoordinateReferenceSystemModel.Roles.RoleProj
            )
        )

        # check that same result is returned by authIdToIndex
        self.assertEqual(model.authIdToIndex("EPSG:4329"), epsg_4329_index)

        # find a vertical crs
        vertical_index = [
            model.index(row, 0, QModelIndex())
            for row in range(model.rowCount(QModelIndex()))
            if model.data(
                model.index(row, 0, QModelIndex()),
                QgsCoordinateReferenceSystemModel.Roles.RoleGroupId,
            )
            == "Vertical"
        ][0]

        # for proj 9, there's > 400 crs in this group
        self.assertGreaterEqual(model.rowCount(vertical_index), 400)

        ahd_index = [
            model.index(row, 0, vertical_index)
            for row in range(model.rowCount(vertical_index))
            if model.data(
                model.index(row, 0, vertical_index),
                QgsCoordinateReferenceSystemModel.Roles.RoleAuthId,
            )
            == "EPSG:5711"
        ][0]
        # test model roles
        self.assertEqual(
            model.data(ahd_index, Qt.ItemDataRole.DisplayRole), "AHD height"
        )
        self.assertEqual(
            model.data(
                model.index(ahd_index.row(), 1, ahd_index.parent()),
                Qt.ItemDataRole.DisplayRole,
            ),
            "EPSG:5711",
        )
        self.assertEqual(
            model.data(ahd_index, QgsCoordinateReferenceSystemModel.Roles.RoleName),
            "AHD height",
        )
        self.assertFalse(
            model.data(
                ahd_index, QgsCoordinateReferenceSystemModel.Roles.RoleDeprecated
            )
        )
        # the proj and wkt roles are only available for non-standard CRS
        self.assertFalse(
            model.data(ahd_index, QgsCoordinateReferenceSystemModel.Roles.RoleWkt)
        )
        self.assertFalse(
            model.data(ahd_index, QgsCoordinateReferenceSystemModel.Roles.RoleProj)
        )

        # check that same result is returned by authIdToIndex
        self.assertEqual(model.authIdToIndex("EPSG:5711"), ahd_index)

        # check projected group
        projected_index = [
            model.index(row, 0, QModelIndex())
            for row in range(model.rowCount(QModelIndex()))
            if model.data(
                model.index(row, 0, QModelIndex()),
                QgsCoordinateReferenceSystemModel.Roles.RoleGroupId,
            )
            == "Projected"
        ][0]
        # for proj 9, there's > 50 projection methods in this group
        self.assertGreaterEqual(model.rowCount(projected_index), 50)

        # find Albers equal area group
        aea_group_index = [
            model.index(row, 0, projected_index)
            for row in range(model.rowCount(projected_index))
            if model.data(
                model.index(row, 0, projected_index),
                QgsCoordinateReferenceSystemModel.Roles.RoleGroupId,
            )
            == "aea"
        ][0]
        # for proj 9, there's > 100 crs in this group
        self.assertGreaterEqual(model.rowCount(aea_group_index), 100)

        # find epsg:3577 in this group
        epsg_3577_index = [
            model.index(row, 0, aea_group_index)
            for row in range(model.rowCount(aea_group_index))
            if model.data(
                model.index(row, 0, aea_group_index),
                QgsCoordinateReferenceSystemModel.Roles.RoleAuthId,
            )
            == "EPSG:3577"
        ][0]
        # test model roles
        self.assertEqual(
            model.data(epsg_3577_index, Qt.ItemDataRole.DisplayRole),
            "GDA94 / Australian Albers",
        )
        self.assertEqual(
            model.data(
                model.index(epsg_3577_index.row(), 1, epsg_3577_index.parent()),
                Qt.ItemDataRole.DisplayRole,
            ),
            "EPSG:3577",
        )
        self.assertEqual(
            model.data(
                epsg_3577_index, QgsCoordinateReferenceSystemModel.Roles.RoleName
            ),
            "GDA94 / Australian Albers",
        )
        self.assertFalse(
            model.data(
                epsg_3577_index, QgsCoordinateReferenceSystemModel.Roles.RoleDeprecated
            )
        )
        # the proj and wkt roles are only available for non-standard CRS
        self.assertFalse(
            model.data(epsg_3577_index, QgsCoordinateReferenceSystemModel.Roles.RoleWkt)
        )
        self.assertFalse(
            model.data(
                epsg_3577_index, QgsCoordinateReferenceSystemModel.Roles.RoleProj
            )
        )
        self.assertEqual(
            model.data(epsg_3577_index, QgsCoordinateReferenceSystemModel.Roles.Group),
            "Projected",
        )
        self.assertEqual(
            model.data(
                epsg_3577_index, QgsCoordinateReferenceSystemModel.Roles.Projection
            ),
            "Albers Equal Area",
        )

        # check that same result is returned by authIdToIndex
        self.assertEqual(model.authIdToIndex("EPSG:3577"), epsg_3577_index)

        # now add a custom crs and ensure it appears in the model
        prev_top_level_count = model.rowCount(QModelIndex())
        registry = QgsApplication.coordinateReferenceSystemRegistry()
        crs = QgsCoordinateReferenceSystem.fromProj(
            "+proj=aea +lat_1=20 +lat_2=-23 +lat_0=4 +lon_0=29 +x_0=10 +y_0=3 +datum=WGS84 +units=m +no_defs"
        )
        res = registry.addUserCrs(crs, "my custom crs")
        self.assertEqual(res, 100000)

        self.assertEqual(model.rowCount(QModelIndex()), prev_top_level_count + 1)
        top_level_items = [
            model.data(model.index(row, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
            for row in range(model.rowCount(QModelIndex()))
        ]
        self.assertIn("User-defined", top_level_items)
        self.assertNotIn("Custom", top_level_items)

        # check group ids
        top_level_item_group_ids = [
            model.data(
                model.index(row, 0, QModelIndex()),
                QgsCoordinateReferenceSystemModel.Roles.RoleGroupId,
            )
            for row in range(model.rowCount(QModelIndex()))
        ]
        self.assertIn("USER", top_level_item_group_ids)

        # find user crs
        user_index = [
            model.index(row, 0, QModelIndex())
            for row in range(model.rowCount(QModelIndex()))
            if model.data(
                model.index(row, 0, QModelIndex()),
                QgsCoordinateReferenceSystemModel.Roles.RoleGroupId,
            )
            == "USER"
        ][0]
        self.assertEqual(model.rowCount(user_index), 1)

        user_crs_index = model.index(0, 0, user_index)
        # test model roles
        self.assertEqual(
            model.data(user_crs_index, Qt.ItemDataRole.DisplayRole), "my custom crs"
        )
        self.assertEqual(
            model.data(
                model.index(user_crs_index.row(), 1, user_crs_index.parent()),
                Qt.ItemDataRole.DisplayRole,
            ),
            "USER:100000",
        )
        self.assertEqual(
            model.data(
                user_crs_index, QgsCoordinateReferenceSystemModel.Roles.RoleName
            ),
            "my custom crs",
        )
        self.assertFalse(
            model.data(
                user_crs_index, QgsCoordinateReferenceSystemModel.Roles.RoleDeprecated
            )
        )
        # the proj and wkt roles are only available for non-standard CRS
        self.assertEqual(
            model.data(user_crs_index, QgsCoordinateReferenceSystemModel.Roles.RoleWkt)[
                :8
            ],
            "PROJCRS[",
        )
        self.assertEqual(
            model.data(
                user_crs_index, QgsCoordinateReferenceSystemModel.Roles.RoleProj
            ),
            "+proj=aea +lat_1=20 +lat_2=-23 +lat_0=4 +lon_0=29 +x_0=10 +y_0=3 +datum=WGS84 +units=m +no_defs",
        )

        # check that same result is returned by authIdToIndex
        self.assertEqual(model.authIdToIndex("USER:100000"), user_crs_index)

        # modify user crs
        crs = QgsCoordinateReferenceSystem.fromProj(
            "+proj=aea +lat_1=21 +lat_2=-23 +lat_0=4 +lon_0=29 +x_0=10 +y_0=3 +datum=WGS84 +units=m +no_defs"
        )
        self.assertTrue(registry.updateUserCrs(100000, crs, "my custom crs rev 2"))
        user_index = [
            model.index(row, 0, QModelIndex())
            for row in range(model.rowCount(QModelIndex()))
            if model.data(
                model.index(row, 0, QModelIndex()),
                QgsCoordinateReferenceSystemModel.Roles.RoleGroupId,
            )
            == "USER"
        ][0]
        self.assertEqual(model.rowCount(user_index), 1)

        user_crs_index = model.index(0, 0, user_index)
        # test model roles
        self.assertEqual(
            model.data(user_crs_index, Qt.ItemDataRole.DisplayRole),
            "my custom crs rev 2",
        )
        self.assertEqual(
            model.data(
                model.index(user_crs_index.row(), 1, user_crs_index.parent()),
                Qt.ItemDataRole.DisplayRole,
            ),
            "USER:100000",
        )
        self.assertEqual(
            model.data(
                user_crs_index, QgsCoordinateReferenceSystemModel.Roles.RoleName
            ),
            "my custom crs rev 2",
        )
        self.assertFalse(
            model.data(
                user_crs_index, QgsCoordinateReferenceSystemModel.Roles.RoleDeprecated
            )
        )
        # the proj and wkt roles are only available for non-standard CRS
        self.assertEqual(
            model.data(user_crs_index, QgsCoordinateReferenceSystemModel.Roles.RoleWkt)[
                :8
            ],
            "PROJCRS[",
        )
        self.assertEqual(
            model.data(
                user_crs_index, QgsCoordinateReferenceSystemModel.Roles.RoleProj
            ),
            "+proj=aea +lat_1=21 +lat_2=-23 +lat_0=4 +lon_0=29 +x_0=10 +y_0=3 +datum=WGS84 +units=m +no_defs",
        )

        # remove
        registry.removeUserCrs(100000)
        self.assertEqual(model.rowCount(user_index), 0)

        # add a non-standard crs (does not correspond to any db entry)
        prev_top_level_count = model.rowCount(QModelIndex())
        crs = QgsCoordinateReferenceSystem.fromProj(
            "+proj=aea +lat_1=1.5 +lat_2=-23 +lat_0=4 +lon_0=29 +x_0=10 +y_0=3 +datum=WGS84 +units=m +no_defs"
        )
        model.addCustomCrs(crs)

        self.assertEqual(model.rowCount(QModelIndex()), prev_top_level_count + 1)
        top_level_items = [
            model.data(model.index(row, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
            for row in range(model.rowCount(QModelIndex()))
        ]
        self.assertIn("Custom", top_level_items)

        # check group ids
        top_level_item_group_ids = [
            model.data(
                model.index(row, 0, QModelIndex()),
                QgsCoordinateReferenceSystemModel.Roles.RoleGroupId,
            )
            for row in range(model.rowCount(QModelIndex()))
        ]
        self.assertIn("CUSTOM", top_level_item_group_ids)
        custom_index = [
            model.index(row, 0, QModelIndex())
            for row in range(model.rowCount(QModelIndex()))
            if model.data(
                model.index(row, 0, QModelIndex()),
                QgsCoordinateReferenceSystemModel.Roles.RoleGroupId,
            )
            == "CUSTOM"
        ][0]
        self.assertEqual(model.rowCount(custom_index), 1)

        custom_crs_index = model.index(0, 0, custom_index)
        # test model roles
        self.assertEqual(
            model.data(custom_crs_index, Qt.ItemDataRole.DisplayRole), "Custom CRS"
        )
        self.assertFalse(
            model.data(
                model.index(custom_crs_index.row(), 1, custom_crs_index.parent()),
                Qt.ItemDataRole.DisplayRole,
            )
        )
        self.assertEqual(
            model.data(
                custom_crs_index, QgsCoordinateReferenceSystemModel.Roles.RoleName
            ),
            "Custom CRS",
        )
        self.assertFalse(
            model.data(
                custom_crs_index, QgsCoordinateReferenceSystemModel.Roles.RoleAuthId
            )
        )
        self.assertFalse(
            model.data(
                custom_crs_index, QgsCoordinateReferenceSystemModel.Roles.RoleDeprecated
            )
        )
        # the proj and wkt roles are only available for non-standard CRS
        self.assertEqual(
            model.data(
                custom_crs_index, QgsCoordinateReferenceSystemModel.Roles.RoleWkt
            )[:8],
            "PROJCRS[",
        )
        self.assertEqual(
            model.data(
                custom_crs_index, QgsCoordinateReferenceSystemModel.Roles.RoleProj
            ),
            "+proj=aea +lat_1=1.5 +lat_2=-23 +lat_0=4 +lon_0=29 +x_0=10 +y_0=3 +datum=WGS84 +units=m +no_defs",
        )

    def test_proxy_model(self):
        model = QgsCoordinateReferenceSystemProxyModel()
        # top level items -- we expect to find Projected, Geographic (2D), Geographic (3D),
        # Compound, Vertical amongst others
        self.assertGreaterEqual(model.rowCount(QModelIndex()), 5)
        top_level_items = [
            model.data(model.index(row, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
            for row in range(model.rowCount(QModelIndex()))
        ]
        self.assertIn("Projected", top_level_items)
        self.assertIn("Geographic (2D)", top_level_items)
        self.assertIn("Geographic (3D)", top_level_items)
        self.assertIn("Compound", top_level_items)
        self.assertIn("Vertical", top_level_items)

        # filter by type
        model.setFilters(QgsCoordinateReferenceSystemProxyModel.Filter.FilterHorizontal)
        top_level_items = [
            model.data(model.index(row, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
            for row in range(model.rowCount(QModelIndex()))
        ]
        self.assertIn("Projected", top_level_items)
        self.assertIn("Geographic (2D)", top_level_items)
        self.assertIn("Geographic (3D)", top_level_items)
        self.assertNotIn("Compound", top_level_items)
        self.assertNotIn("Vertical", top_level_items)

        model.setFilters(QgsCoordinateReferenceSystemProxyModel.Filter.FilterVertical)
        top_level_items = [
            model.data(model.index(row, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
            for row in range(model.rowCount(QModelIndex()))
        ]
        self.assertNotIn("Projected", top_level_items)
        self.assertNotIn("Geographic (2D)", top_level_items)
        self.assertNotIn("Geographic (3D)", top_level_items)
        self.assertNotIn("Compound", top_level_items)
        self.assertIn("Vertical", top_level_items)

        model.setFilters(QgsCoordinateReferenceSystemProxyModel.Filter.FilterCompound)
        top_level_items = [
            model.data(model.index(row, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
            for row in range(model.rowCount(QModelIndex()))
        ]
        self.assertNotIn("Projected", top_level_items)
        self.assertNotIn("Geographic (2D)", top_level_items)
        self.assertNotIn("Geographic (3D)", top_level_items)
        self.assertIn("Compound", top_level_items)
        self.assertNotIn("Vertical", top_level_items)

        model.setFilters(
            QgsCoordinateReferenceSystemProxyModel.Filters(
                QgsCoordinateReferenceSystemProxyModel.Filter.FilterCompound
                | QgsCoordinateReferenceSystemProxyModel.Filter.FilterVertical
            )
        )
        top_level_items = [
            model.data(model.index(row, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
            for row in range(model.rowCount(QModelIndex()))
        ]
        self.assertNotIn("Projected", top_level_items)
        self.assertNotIn("Geographic (2D)", top_level_items)
        self.assertNotIn("Geographic (3D)", top_level_items)
        self.assertIn("Compound", top_level_items)
        self.assertIn("Vertical", top_level_items)

        model.setFilters(QgsCoordinateReferenceSystemProxyModel.Filters())

        # find WGS84 in Geographic2d group
        geographic_2d_index = [
            model.index(row, 0, QModelIndex())
            for row in range(model.rowCount(QModelIndex()))
            if model.data(
                model.index(row, 0, QModelIndex()),
                QgsCoordinateReferenceSystemModel.Roles.RoleGroupId,
            )
            == "Geographic2d"
        ][0]

        wgs84_index = [
            model.index(row, 0, geographic_2d_index)
            for row in range(model.rowCount(geographic_2d_index))
            if model.data(
                model.index(row, 0, geographic_2d_index),
                QgsCoordinateReferenceSystemModel.Roles.RoleAuthId,
            )
            == "EPSG:4326"
        ][0]
        # test model roles
        self.assertEqual(model.data(wgs84_index, Qt.ItemDataRole.DisplayRole), "WGS 84")
        self.assertEqual(
            model.data(
                model.index(wgs84_index.row(), 1, wgs84_index.parent()),
                Qt.ItemDataRole.DisplayRole,
            ),
            "EPSG:4326",
        )
        self.assertEqual(
            model.data(wgs84_index, QgsCoordinateReferenceSystemModel.Roles.RoleName),
            "WGS 84",
        )
        self.assertFalse(
            model.data(
                wgs84_index, QgsCoordinateReferenceSystemModel.Roles.RoleDeprecated
            )
        )
        # the proj and wkt roles are only available for non-standard CRS
        self.assertFalse(
            model.data(wgs84_index, QgsCoordinateReferenceSystemModel.Roles.RoleWkt)
        )
        self.assertFalse(
            model.data(wgs84_index, QgsCoordinateReferenceSystemModel.Roles.RoleProj)
        )

        # find EPSG:4329 in Geographic3d group (also tests a deprecated CRS)
        geographic_3d_index = [
            model.index(row, 0, QModelIndex())
            for row in range(model.rowCount(QModelIndex()))
            if model.data(
                model.index(row, 0, QModelIndex()),
                QgsCoordinateReferenceSystemModel.Roles.RoleGroupId,
            )
            == "Geographic3d"
        ][0]

        epsg_4329_index = [
            model.index(row, 0, geographic_3d_index)
            for row in range(model.rowCount(geographic_3d_index))
            if model.data(
                model.index(row, 0, geographic_3d_index),
                QgsCoordinateReferenceSystemModel.Roles.RoleAuthId,
            )
            == "EPSG:4329"
        ][0]
        # test model roles
        self.assertEqual(
            model.data(epsg_4329_index, Qt.ItemDataRole.DisplayRole), "WGS 84 (3D)"
        )
        self.assertEqual(
            model.data(
                model.index(epsg_4329_index.row(), 1, epsg_4329_index.parent()),
                Qt.ItemDataRole.DisplayRole,
            ),
            "EPSG:4329",
        )
        self.assertEqual(
            model.data(
                epsg_4329_index, QgsCoordinateReferenceSystemModel.Roles.RoleName
            ),
            "WGS 84 (3D)",
        )
        self.assertTrue(
            model.data(
                epsg_4329_index, QgsCoordinateReferenceSystemModel.Roles.RoleDeprecated
            )
        )
        # the proj and wkt roles are only available for non-standard CRS
        self.assertFalse(
            model.data(epsg_4329_index, QgsCoordinateReferenceSystemModel.Roles.RoleWkt)
        )
        self.assertFalse(
            model.data(
                epsg_4329_index, QgsCoordinateReferenceSystemModel.Roles.RoleProj
            )
        )

        model.setFilterDeprecated(True)
        geographic_3d_index = [
            model.index(row, 0, QModelIndex())
            for row in range(model.rowCount(QModelIndex()))
            if model.data(
                model.index(row, 0, QModelIndex()),
                QgsCoordinateReferenceSystemModel.Roles.RoleGroupId,
            )
            == "Geographic3d"
        ][0]
        self.assertFalse(
            [
                model.index(row, 0, geographic_3d_index)
                for row in range(model.rowCount(geographic_3d_index))
                if model.data(
                    model.index(row, 0, geographic_3d_index),
                    QgsCoordinateReferenceSystemModel.Roles.RoleAuthId,
                )
                == "EPSG:4329"
            ]
        )
        model.setFilterDeprecated(False)
        geographic_3d_index = [
            model.index(row, 0, QModelIndex())
            for row in range(model.rowCount(QModelIndex()))
            if model.data(
                model.index(row, 0, QModelIndex()),
                QgsCoordinateReferenceSystemModel.Roles.RoleGroupId,
            )
            == "Geographic3d"
        ][0]
        epsg_4329_index = [
            model.index(row, 0, geographic_3d_index)
            for row in range(model.rowCount(geographic_3d_index))
            if model.data(
                model.index(row, 0, geographic_3d_index),
                QgsCoordinateReferenceSystemModel.Roles.RoleAuthId,
            )
            == "EPSG:4329"
        ][0]
        self.assertTrue(epsg_4329_index.isValid())

        # filter by string
        model.setFilterString("GDA94")
        geographic_3d_index = [
            model.index(row, 0, QModelIndex())
            for row in range(model.rowCount(QModelIndex()))
            if model.data(
                model.index(row, 0, QModelIndex()),
                QgsCoordinateReferenceSystemModel.Roles.RoleGroupId,
            )
            == "Geographic3d"
        ][0]
        self.assertFalse(
            [
                model.index(row, 0, geographic_3d_index)
                for row in range(model.rowCount(geographic_3d_index))
                if model.data(
                    model.index(row, 0, geographic_3d_index),
                    QgsCoordinateReferenceSystemModel.Roles.RoleAuthId,
                )
                == "EPSG:4329"
            ]
        )
        epsg_4939_index = [
            model.index(row, 0, geographic_3d_index)
            for row in range(model.rowCount(geographic_3d_index))
            if model.data(
                model.index(row, 0, geographic_3d_index),
                QgsCoordinateReferenceSystemModel.Roles.RoleAuthId,
            )
            == "EPSG:4939"
        ][0]
        self.assertTrue(epsg_4939_index.isValid())
        epsg_4347_index = [
            model.index(row, 0, geographic_3d_index)
            for row in range(model.rowCount(geographic_3d_index))
            if model.data(
                model.index(row, 0, geographic_3d_index),
                QgsCoordinateReferenceSystemModel.Roles.RoleAuthId,
            )
            == "EPSG:4347"
        ][0]
        self.assertTrue(epsg_4347_index.isValid())

        model.setFilterString("equal gda2020 Area")
        projected_index = [
            model.index(row, 0, QModelIndex())
            for row in range(model.rowCount(QModelIndex()))
            if model.data(
                model.index(row, 0, QModelIndex()),
                QgsCoordinateReferenceSystemModel.Roles.RoleGroupId,
            )
            == "Projected"
        ][0]
        aae_index = [
            model.index(row, 0, projected_index)
            for row in range(model.rowCount(projected_index))
            if model.data(
                model.index(row, 0, projected_index),
                Qt.ItemDataRole.DisplayRole,
            )
            == "Albers Equal Area"
        ][0]
        epsg_9473_index = [
            model.index(row, 0, aae_index)
            for row in range(model.rowCount(aae_index))
            if model.data(
                model.index(row, 0, aae_index),
                QgsCoordinateReferenceSystemModel.Roles.RoleAuthId,
            )
            == "EPSG:9473"
        ][0]

        model.setFilterString("")

        # set filtered list of crs to show
        model.setFilterAuthIds({"epsg:4289", "EPSG:4196"})
        geographic_2d_index = [
            model.index(row, 0, QModelIndex())
            for row in range(model.rowCount(QModelIndex()))
            if model.data(
                model.index(row, 0, QModelIndex()),
                QgsCoordinateReferenceSystemModel.Roles.RoleGroupId,
            )
            == "Geographic2d"
        ][0]
        self.assertFalse(
            [
                model.index(row, 0, geographic_2d_index)
                for row in range(model.rowCount(geographic_2d_index))
                if model.data(
                    model.index(row, 0, geographic_2d_index),
                    QgsCoordinateReferenceSystemModel.Roles.RoleAuthId,
                )
                == "EPSG:4169"
            ]
        )
        epsg_4289_index = [
            model.index(row, 0, geographic_2d_index)
            for row in range(model.rowCount(geographic_2d_index))
            if model.data(
                model.index(row, 0, geographic_2d_index),
                QgsCoordinateReferenceSystemModel.Roles.RoleAuthId,
            )
            == "EPSG:4289"
        ][0]
        self.assertTrue(epsg_4289_index.isValid())
        epsg_4196_index = [
            model.index(row, 0, geographic_2d_index)
            for row in range(model.rowCount(geographic_2d_index))
            if model.data(
                model.index(row, 0, geographic_2d_index),
                QgsCoordinateReferenceSystemModel.Roles.RoleAuthId,
            )
            == "EPSG:4196"
        ][0]
        self.assertTrue(epsg_4196_index.isValid())


if __name__ == "__main__":
    unittest.main()
