"""QGIS Unit tests for QgsElevationProfileManagerModel.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

from qgis.PyQt.QtCore import QModelIndex, Qt
from qgis.core import (
    QgsElevationProfileManager,
    QgsElevationProfileManagerModel,
    QgsElevationProfileManagerProxyModel,
    QgsElevationProfile,
    QgsProject,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsElevationProfileManagerModel(QgisTestCase):

    def setUp(self):
        """Run before each test."""
        self.manager = None
        self.aboutFired = False

    def tearDown(self):
        """Run after each test."""
        pass

    def testModel(self):
        project = QgsProject()
        manager = QgsElevationProfileManager(project)
        model = QgsElevationProfileManagerModel(manager)
        self.assertEqual(model.rowCount(QModelIndex()), 0)
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            None,
        )
        self.assertEqual(
            model.data(
                model.index(0, 0, QModelIndex()),
                QgsElevationProfileManagerModel.CustomRole.ElevationProfile,
            ),
            None,
        )
        self.assertEqual(model.profileFromIndex(model.index(0, 0, QModelIndex())), None)
        self.assertEqual(model.indexFromProfile(None), QModelIndex())

        profile = QgsElevationProfile(project)
        profile.setName("test profile")
        self.assertEqual(model.indexFromProfile(profile), QModelIndex())
        self.assertTrue(manager.addProfile(profile))

        self.assertEqual(model.rowCount(QModelIndex()), 1)
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "test profile",
        )
        self.assertEqual(
            model.data(
                model.index(0, 0, QModelIndex()),
                QgsElevationProfileManagerModel.CustomRole.ElevationProfile,
            ),
            profile,
        )
        self.assertEqual(
            model.profileFromIndex(model.index(0, 0, QModelIndex())), profile
        )
        self.assertEqual(
            model.indexFromProfile(profile), model.index(0, 0, QModelIndex())
        )
        self.assertEqual(
            model.data(model.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            None,
        )
        self.assertEqual(
            model.data(
                model.index(1, 0, QModelIndex()),
                QgsElevationProfileManagerModel.CustomRole.ElevationProfile,
            ),
            None,
        )
        self.assertEqual(model.profileFromIndex(model.index(1, 0, QModelIndex())), None)

        profile.setName("test profile")
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "test profile",
        )

        profile2 = QgsElevationProfile(project)
        profile2.setName("test profile2")
        self.assertTrue(manager.addProfile(profile2))

        self.assertEqual(model.rowCount(QModelIndex()), 2)
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "test profile",
        )
        self.assertEqual(
            model.data(
                model.index(0, 0, QModelIndex()),
                QgsElevationProfileManagerModel.CustomRole.ElevationProfile,
            ),
            profile,
        )
        self.assertEqual(
            model.profileFromIndex(model.index(0, 0, QModelIndex())), profile
        )
        self.assertEqual(
            model.indexFromProfile(profile), model.index(0, 0, QModelIndex())
        )
        self.assertEqual(
            model.data(model.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "test profile2",
        )
        self.assertEqual(
            model.data(
                model.index(1, 0, QModelIndex()),
                QgsElevationProfileManagerModel.CustomRole.ElevationProfile,
            ),
            profile2,
        )
        self.assertEqual(
            model.profileFromIndex(model.index(1, 0, QModelIndex())), profile2
        )
        self.assertEqual(
            model.indexFromProfile(profile2), model.index(1, 0, QModelIndex())
        )

        manager.removeProfile(profile)
        self.assertEqual(model.rowCount(QModelIndex()), 1)
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "test profile2",
        )
        self.assertEqual(
            model.data(
                model.index(0, 0, QModelIndex()),
                QgsElevationProfileManagerModel.CustomRole.ElevationProfile,
            ),
            profile2,
        )
        self.assertEqual(
            model.data(model.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            None,
        )
        self.assertEqual(
            model.data(
                model.index(1, 0, QModelIndex()),
                QgsElevationProfileManagerModel.CustomRole.ElevationProfile,
            ),
            None,
        )
        self.assertEqual(
            model.profileFromIndex(model.index(0, 0, QModelIndex())), profile2
        )
        self.assertEqual(model.profileFromIndex(model.index(1, 0, QModelIndex())), None)
        self.assertEqual(
            model.indexFromProfile(profile2), model.index(0, 0, QModelIndex())
        )

        manager.clear()
        self.assertEqual(model.rowCount(QModelIndex()), 0)
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            None,
        )
        self.assertEqual(
            model.data(
                model.index(0, 0, QModelIndex()),
                QgsElevationProfileManagerModel.CustomRole.ElevationProfile,
            ),
            None,
        )
        self.assertEqual(model.profileFromIndex(model.index(0, 0, QModelIndex())), None)

    def testProxyModel(self):
        project = QgsProject()
        manager = QgsElevationProfileManager(project)
        model = QgsElevationProfileManagerModel(manager)
        proxy = QgsElevationProfileManagerProxyModel()
        proxy.setSourceModel(model)
        self.assertEqual(proxy.rowCount(QModelIndex()), 0)
        self.assertEqual(
            proxy.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            None,
        )
        self.assertEqual(
            proxy.data(
                model.index(0, 0, QModelIndex()),
                QgsElevationProfileManagerModel.CustomRole.ElevationProfile,
            ),
            None,
        )

        profile = QgsElevationProfile(project)
        profile.setName("ccc")
        self.assertTrue(manager.addProfile(profile))

        self.assertEqual(proxy.rowCount(QModelIndex()), 1)
        self.assertEqual(
            proxy.data(proxy.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "ccc",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(0, 0, QModelIndex()),
                QgsElevationProfileManagerModel.CustomRole.ElevationProfile,
            ),
            profile,
        )
        self.assertEqual(
            proxy.data(proxy.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            None,
        )
        self.assertEqual(
            proxy.data(
                proxy.index(1, 0, QModelIndex()),
                QgsElevationProfileManagerModel.CustomRole.ElevationProfile,
            ),
            None,
        )

        profile2 = QgsElevationProfile(project)
        profile2.setName("bbb")
        self.assertTrue(manager.addProfile(profile2))
        self.assertEqual(proxy.rowCount(QModelIndex()), 2)
        self.assertEqual(
            proxy.data(proxy.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "bbb",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(0, 0, QModelIndex()),
                QgsElevationProfileManagerModel.CustomRole.ElevationProfile,
            ),
            profile2,
        )
        self.assertEqual(
            proxy.data(proxy.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "ccc",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(1, 0, QModelIndex()),
                QgsElevationProfileManagerModel.CustomRole.ElevationProfile,
            ),
            profile,
        )

        profile.setName("aaa")
        self.assertEqual(
            proxy.data(proxy.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "aaa",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(0, 0, QModelIndex()),
                QgsElevationProfileManagerModel.CustomRole.ElevationProfile,
            ),
            profile,
        )
        self.assertEqual(
            proxy.data(proxy.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "bbb",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(1, 0, QModelIndex()),
                QgsElevationProfileManagerModel.CustomRole.ElevationProfile,
            ),
            profile2,
        )

        r = QgsElevationProfile(project)
        r.setName("bbb2")
        manager.addProfile(r)

        proxy.setFilterString("xx")
        self.assertEqual(proxy.rowCount(QModelIndex()), 0)
        proxy.setFilterString("bb")
        self.assertEqual(proxy.rowCount(QModelIndex()), 2)
        self.assertEqual(
            proxy.data(proxy.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "bbb",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(0, 0, QModelIndex()),
                QgsElevationProfileManagerModel.CustomRole.ElevationProfile,
            ),
            profile2,
        )
        self.assertEqual(
            proxy.data(proxy.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "bbb2",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(1, 0, QModelIndex()),
                QgsElevationProfileManagerModel.CustomRole.ElevationProfile,
            ),
            r,
        )
        proxy.setFilterString("")

        self.assertEqual(proxy.rowCount(QModelIndex()), 3)


if __name__ == "__main__":
    unittest.main()
