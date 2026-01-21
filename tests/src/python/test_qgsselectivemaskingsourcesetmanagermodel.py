"""QGIS Unit tests for QgsSelectiveMaskingSourceSetManagerModel.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

from qgis.PyQt.QtCore import QModelIndex, Qt
from qgis.core import (
    QgsSelectiveMaskingSourceSetManager,
    QgsSelectiveMaskingSourceSetManagerModel,
    QgsSelectiveMaskingSourceSetManagerProxyModel,
    QgsSelectiveMaskingSourceSet,
    QgsProject,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsSelectiveMaskingSourceSetManagerModel(QgisTestCase):

    def setUp(self):
        """Run before each test."""
        self.manager = None
        self.aboutFired = False

    def tearDown(self):
        """Run after each test."""
        pass

    def testModel(self):
        project = QgsProject()
        manager = QgsSelectiveMaskingSourceSetManager(project)
        model = QgsSelectiveMaskingSourceSetManagerModel(manager)
        self.assertEqual(model.rowCount(QModelIndex()), 0)
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            None,
        )
        self.assertEqual(
            model.data(
                model.index(0, 0, QModelIndex()),
                QgsSelectiveMaskingSourceSetManagerModel.CustomRole.Object,
            ),
            None,
        )
        self.assertFalse(
            model.data(
                model.index(0, 0, QModelIndex()),
                QgsSelectiveMaskingSourceSetManagerModel.CustomRole.Object,
            )
        )
        self.assertEqual(model.setFromIndex(model.index(0, 0, QModelIndex())), None)
        self.assertEqual(model.indexFromSet(None), QModelIndex())

        source_set = QgsSelectiveMaskingSourceSet()
        source_set.setName("test source set")
        self.assertEqual(model.indexFromSet(source_set), QModelIndex())
        self.assertTrue(manager.addSet(source_set))

        self.assertEqual(model.rowCount(QModelIndex()), 1)
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "test source set",
        )
        self.assertEqual(
            model.data(
                model.index(0, 0, QModelIndex()),
                QgsSelectiveMaskingSourceSetManagerModel.CustomRole.Object,
            ).id(),
            source_set.id(),
        )
        self.assertEqual(
            model.data(
                model.index(0, 0, QModelIndex()),
                QgsSelectiveMaskingSourceSetManagerModel.CustomRole.SetId,
            ),
            source_set.id(),
        )
        self.assertEqual(
            model.setFromIndex(model.index(0, 0, QModelIndex())).id(), source_set.id()
        )
        self.assertEqual(
            model.indexFromSet(source_set), model.index(0, 0, QModelIndex())
        )
        self.assertEqual(
            model.data(model.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            None,
        )
        self.assertEqual(
            model.data(
                model.index(1, 0, QModelIndex()),
                QgsSelectiveMaskingSourceSetManagerModel.CustomRole.Object,
            ),
            None,
        )
        self.assertFalse(
            model.data(
                model.index(1, 0, QModelIndex()),
                QgsSelectiveMaskingSourceSetManagerModel.CustomRole.SetId,
            )
        )
        self.assertEqual(model.setFromIndex(model.index(1, 0, QModelIndex())), None)

        manager.renameSet("test source set", "source set")
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "source set",
        )

        source_set2 = QgsSelectiveMaskingSourceSet()
        source_set2.setName("test source set2")
        self.assertTrue(manager.addSet(source_set2))

        self.assertEqual(model.rowCount(QModelIndex()), 2)
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "source set",
        )
        self.assertEqual(
            model.data(
                model.index(0, 0, QModelIndex()),
                QgsSelectiveMaskingSourceSetManagerModel.CustomRole.Object,
            ).id(),
            source_set.id(),
        )
        self.assertEqual(
            model.data(
                model.index(0, 0, QModelIndex()),
                QgsSelectiveMaskingSourceSetManagerModel.CustomRole.SetId,
            ),
            source_set.id(),
        )
        self.assertEqual(
            model.setFromIndex(model.index(0, 0, QModelIndex())).id(), source_set.id()
        )
        self.assertEqual(
            model.indexFromSet(source_set), model.index(0, 0, QModelIndex())
        )
        self.assertEqual(
            model.data(model.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "test source set2",
        )
        self.assertEqual(
            model.data(
                model.index(1, 0, QModelIndex()),
                QgsSelectiveMaskingSourceSetManagerModel.CustomRole.Object,
            ).id(),
            source_set2.id(),
        )
        self.assertEqual(
            model.data(
                model.index(1, 0, QModelIndex()),
                QgsSelectiveMaskingSourceSetManagerModel.CustomRole.SetId,
            ),
            source_set2.id(),
        )
        self.assertEqual(
            model.setFromIndex(model.index(1, 0, QModelIndex())).id(), source_set2.id()
        )
        self.assertEqual(
            model.indexFromSet(source_set2), model.index(1, 0, QModelIndex())
        )

        manager.removeSet("source set")
        self.assertEqual(model.rowCount(QModelIndex()), 1)
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "test source set2",
        )
        self.assertEqual(
            model.data(
                model.index(0, 0, QModelIndex()),
                QgsSelectiveMaskingSourceSetManagerModel.CustomRole.Object,
            ).id(),
            source_set2.id(),
        )
        self.assertEqual(
            model.data(
                model.index(0, 0, QModelIndex()),
                QgsSelectiveMaskingSourceSetManagerModel.CustomRole.SetId,
            ),
            source_set2.id(),
        )
        self.assertEqual(
            model.data(model.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            None,
        )
        self.assertEqual(
            model.data(
                model.index(1, 0, QModelIndex()),
                QgsSelectiveMaskingSourceSetManagerModel.CustomRole.Object,
            ),
            None,
        )
        self.assertFalse(
            model.data(
                model.index(1, 0, QModelIndex()),
                QgsSelectiveMaskingSourceSetManagerModel.CustomRole.SetId,
            )
        )
        self.assertEqual(
            model.setFromIndex(model.index(0, 0, QModelIndex())).id(), source_set2.id()
        )
        self.assertEqual(model.setFromIndex(model.index(1, 0, QModelIndex())), None)
        self.assertEqual(
            model.indexFromSet(source_set2), model.index(0, 0, QModelIndex())
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
                QgsSelectiveMaskingSourceSetManagerModel.CustomRole.Object,
            ),
            None,
        )
        self.assertFalse(
            model.data(
                model.index(0, 0, QModelIndex()),
                QgsSelectiveMaskingSourceSetManagerModel.CustomRole.SetId,
            )
        )
        self.assertEqual(model.setFromIndex(model.index(0, 0, QModelIndex())), None)

    def testProxyModel(self):
        project = QgsProject()
        manager = QgsSelectiveMaskingSourceSetManager(project)
        model = QgsSelectiveMaskingSourceSetManagerModel(manager)
        proxy = QgsSelectiveMaskingSourceSetManagerProxyModel()
        proxy.setSourceModel(model)
        self.assertEqual(proxy.rowCount(QModelIndex()), 0)
        self.assertEqual(
            proxy.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            None,
        )
        self.assertEqual(
            proxy.data(
                model.index(0, 0, QModelIndex()),
                QgsSelectiveMaskingSourceSetManagerModel.CustomRole.Object,
            ),
            None,
        )
        self.assertFalse(
            proxy.data(
                model.index(0, 0, QModelIndex()),
                QgsSelectiveMaskingSourceSetManagerModel.CustomRole.SetId,
            )
        )

        source_set = QgsSelectiveMaskingSourceSet()
        source_set.setName("ccc")
        self.assertTrue(manager.addSet(source_set))

        self.assertEqual(proxy.rowCount(QModelIndex()), 1)
        self.assertEqual(
            proxy.data(proxy.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "ccc",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(0, 0, QModelIndex()),
                QgsSelectiveMaskingSourceSetManagerModel.CustomRole.Object,
            ).id(),
            source_set.id(),
        )
        self.assertEqual(
            proxy.data(
                proxy.index(0, 0, QModelIndex()),
                QgsSelectiveMaskingSourceSetManagerModel.CustomRole.SetId,
            ),
            source_set.id(),
        )
        self.assertEqual(
            proxy.data(proxy.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            None,
        )
        self.assertEqual(
            proxy.data(
                proxy.index(1, 0, QModelIndex()),
                QgsSelectiveMaskingSourceSetManagerModel.CustomRole.Object,
            ),
            None,
        )
        self.assertFalse(
            proxy.data(
                proxy.index(1, 0, QModelIndex()),
                QgsSelectiveMaskingSourceSetManagerModel.CustomRole.SetId,
            )
        )

        source_set2 = QgsSelectiveMaskingSourceSet()
        source_set2.setName("bbb")
        self.assertTrue(manager.addSet(source_set2))

        self.assertEqual(proxy.rowCount(QModelIndex()), 2)
        self.assertEqual(
            proxy.data(proxy.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "bbb",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(0, 0, QModelIndex()),
                QgsSelectiveMaskingSourceSetManagerModel.CustomRole.Object,
            ).id(),
            source_set2.id(),
        )
        self.assertEqual(
            proxy.data(
                proxy.index(0, 0, QModelIndex()),
                QgsSelectiveMaskingSourceSetManagerModel.CustomRole.SetId,
            ),
            source_set2.id(),
        )
        self.assertEqual(
            proxy.data(proxy.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "ccc",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(1, 0, QModelIndex()),
                QgsSelectiveMaskingSourceSetManagerModel.CustomRole.Object,
            ).id(),
            source_set.id(),
        )
        self.assertEqual(
            proxy.data(
                proxy.index(1, 0, QModelIndex()),
                QgsSelectiveMaskingSourceSetManagerModel.CustomRole.SetId,
            ),
            source_set.id(),
        )

        manager.renameSet("ccc", "aaa")
        self.assertEqual(
            proxy.data(proxy.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "aaa",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(0, 0, QModelIndex()),
                QgsSelectiveMaskingSourceSetManagerModel.CustomRole.Object,
            ).id(),
            source_set.id(),
        )
        self.assertEqual(
            proxy.data(
                proxy.index(0, 0, QModelIndex()),
                QgsSelectiveMaskingSourceSetManagerModel.CustomRole.SetId,
            ),
            source_set.id(),
        )
        self.assertEqual(
            proxy.data(proxy.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "bbb",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(1, 0, QModelIndex()),
                QgsSelectiveMaskingSourceSetManagerModel.CustomRole.Object,
            ).id(),
            source_set2.id(),
        )
        self.assertEqual(
            proxy.data(
                proxy.index(1, 0, QModelIndex()),
                QgsSelectiveMaskingSourceSetManagerModel.CustomRole.SetId,
            ),
            source_set2.id(),
        )

        source_set3 = QgsSelectiveMaskingSourceSet()
        source_set3.setName("bbb2")
        self.assertTrue(manager.addSet(source_set3))

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
                QgsSelectiveMaskingSourceSetManagerModel.CustomRole.Object,
            ).id(),
            source_set2.id(),
        )
        self.assertEqual(
            proxy.data(
                proxy.index(0, 0, QModelIndex()),
                QgsSelectiveMaskingSourceSetManagerModel.CustomRole.SetId,
            ),
            source_set2.id(),
        )
        self.assertEqual(
            proxy.data(proxy.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "bbb2",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(1, 0, QModelIndex()),
                QgsSelectiveMaskingSourceSetManagerModel.CustomRole.Object,
            ).id(),
            source_set3.id(),
        )
        self.assertEqual(
            proxy.data(
                proxy.index(1, 0, QModelIndex()),
                QgsSelectiveMaskingSourceSetManagerModel.CustomRole.SetId,
            ),
            source_set3.id(),
        )
        proxy.setFilterString("")

        self.assertEqual(proxy.rowCount(QModelIndex()), 3)


if __name__ == "__main__":
    unittest.main()
