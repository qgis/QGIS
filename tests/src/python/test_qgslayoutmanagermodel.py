"""QGIS Unit tests for QgsLayoutManagerModel.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "(C) 2019 by Nyall Dawson"
__date__ = "11/03/2019"
__copyright__ = "Copyright 2019, The QGIS Project"

from qgis.PyQt.QtCore import QModelIndex, Qt
from qgis.core import (
    QgsLayoutManager,
    QgsLayoutManagerModel,
    QgsLayoutManagerProxyModel,
    QgsPrintLayout,
    QgsProject,
    QgsReport,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsLayoutManagerModel(QgisTestCase):

    def setUp(self):
        """Run before each test."""
        self.manager = None
        self.aboutFired = False

    def tearDown(self):
        """Run after each test."""
        pass

    def testModel(self):
        project = QgsProject()
        manager = QgsLayoutManager(project)
        model = QgsLayoutManagerModel(manager)
        self.assertEqual(model.rowCount(QModelIndex()), 0)
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            None,
        )
        self.assertEqual(
            model.data(
                model.index(0, 0, QModelIndex()), QgsLayoutManagerModel.Role.LayoutRole
            ),
            None,
        )
        self.assertEqual(model.layoutFromIndex(model.index(0, 0, QModelIndex())), None)
        self.assertEqual(model.indexFromLayout(None), QModelIndex())

        layout = QgsPrintLayout(project)
        layout.setName("test layout")
        self.assertEqual(model.indexFromLayout(layout), QModelIndex())
        self.assertTrue(manager.addLayout(layout))

        self.assertEqual(model.rowCount(QModelIndex()), 1)
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "test layout",
        )
        self.assertEqual(
            model.data(
                model.index(0, 0, QModelIndex()), QgsLayoutManagerModel.Role.LayoutRole
            ),
            layout,
        )
        self.assertEqual(
            model.layoutFromIndex(model.index(0, 0, QModelIndex())), layout
        )
        self.assertEqual(
            model.indexFromLayout(layout), model.index(0, 0, QModelIndex())
        )
        self.assertEqual(
            model.data(model.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            None,
        )
        self.assertEqual(
            model.data(
                model.index(1, 0, QModelIndex()), QgsLayoutManagerModel.Role.LayoutRole
            ),
            None,
        )
        self.assertEqual(model.layoutFromIndex(model.index(1, 0, QModelIndex())), None)

        layout.setName("test Layout")
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "test Layout",
        )

        layout2 = QgsPrintLayout(project)
        layout2.setName("test layout2")
        self.assertTrue(manager.addLayout(layout2))

        self.assertEqual(model.rowCount(QModelIndex()), 2)
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "test Layout",
        )
        self.assertEqual(
            model.data(
                model.index(0, 0, QModelIndex()), QgsLayoutManagerModel.Role.LayoutRole
            ),
            layout,
        )
        self.assertEqual(
            model.layoutFromIndex(model.index(0, 0, QModelIndex())), layout
        )
        self.assertEqual(
            model.indexFromLayout(layout), model.index(0, 0, QModelIndex())
        )
        self.assertEqual(
            model.data(model.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "test layout2",
        )
        self.assertEqual(
            model.data(
                model.index(1, 0, QModelIndex()), QgsLayoutManagerModel.Role.LayoutRole
            ),
            layout2,
        )
        self.assertEqual(
            model.layoutFromIndex(model.index(1, 0, QModelIndex())), layout2
        )
        self.assertEqual(
            model.indexFromLayout(layout2), model.index(1, 0, QModelIndex())
        )

        manager.removeLayout(layout)
        self.assertEqual(model.rowCount(QModelIndex()), 1)
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "test layout2",
        )
        self.assertEqual(
            model.data(
                model.index(0, 0, QModelIndex()), QgsLayoutManagerModel.Role.LayoutRole
            ),
            layout2,
        )
        self.assertEqual(
            model.data(model.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            None,
        )
        self.assertEqual(
            model.data(
                model.index(1, 0, QModelIndex()), QgsLayoutManagerModel.Role.LayoutRole
            ),
            None,
        )
        self.assertEqual(
            model.layoutFromIndex(model.index(0, 0, QModelIndex())), layout2
        )
        self.assertEqual(model.layoutFromIndex(model.index(1, 0, QModelIndex())), None)
        self.assertEqual(
            model.indexFromLayout(layout2), model.index(0, 0, QModelIndex())
        )

        manager.clear()
        self.assertEqual(model.rowCount(QModelIndex()), 0)
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            None,
        )
        self.assertEqual(
            model.data(
                model.index(0, 0, QModelIndex()), QgsLayoutManagerModel.Role.LayoutRole
            ),
            None,
        )
        self.assertEqual(model.layoutFromIndex(model.index(0, 0, QModelIndex())), None)

        # with empty row
        model.setAllowEmptyLayout(True)
        self.assertEqual(model.rowCount(QModelIndex()), 1)
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            None,
        )
        self.assertEqual(
            model.data(
                model.index(0, 0, QModelIndex()), QgsLayoutManagerModel.Role.LayoutRole
            ),
            None,
        )
        self.assertEqual(model.layoutFromIndex(model.index(0, 0, QModelIndex())), None)

        layout = QgsPrintLayout(project)
        layout.setName("test layout")
        self.assertTrue(manager.addLayout(layout))
        self.assertEqual(model.rowCount(QModelIndex()), 2)
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            None,
        )
        self.assertEqual(
            model.data(
                model.index(0, 0, QModelIndex()), QgsLayoutManagerModel.Role.LayoutRole
            ),
            None,
        )
        self.assertEqual(
            model.data(model.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "test layout",
        )
        self.assertEqual(
            model.data(
                model.index(1, 0, QModelIndex()), QgsLayoutManagerModel.Role.LayoutRole
            ),
            layout,
        )
        self.assertEqual(
            model.data(model.index(2, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            None,
        )
        self.assertEqual(
            model.data(
                model.index(2, 0, QModelIndex()), QgsLayoutManagerModel.Role.LayoutRole
            ),
            None,
        )
        self.assertEqual(model.layoutFromIndex(model.index(0, 0, QModelIndex())), None)
        self.assertEqual(
            model.layoutFromIndex(model.index(1, 0, QModelIndex())), layout
        )
        self.assertEqual(
            model.indexFromLayout(layout), model.index(1, 0, QModelIndex())
        )

        layout2 = QgsPrintLayout(project)
        layout2.setName("test layout2")
        self.assertTrue(manager.addLayout(layout2))
        self.assertEqual(model.rowCount(QModelIndex()), 3)
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            None,
        )
        self.assertEqual(
            model.data(
                model.index(0, 0, QModelIndex()), QgsLayoutManagerModel.Role.LayoutRole
            ),
            None,
        )
        self.assertEqual(
            model.data(model.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "test layout",
        )
        self.assertEqual(
            model.data(
                model.index(1, 0, QModelIndex()), QgsLayoutManagerModel.Role.LayoutRole
            ),
            layout,
        )
        self.assertEqual(
            model.data(model.index(2, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "test layout2",
        )
        self.assertEqual(
            model.data(
                model.index(2, 0, QModelIndex()), QgsLayoutManagerModel.Role.LayoutRole
            ),
            layout2,
        )
        self.assertEqual(model.layoutFromIndex(model.index(0, 0, QModelIndex())), None)
        self.assertEqual(
            model.layoutFromIndex(model.index(1, 0, QModelIndex())), layout
        )
        self.assertEqual(
            model.layoutFromIndex(model.index(2, 0, QModelIndex())), layout2
        )
        self.assertEqual(
            model.indexFromLayout(layout), model.index(1, 0, QModelIndex())
        )
        self.assertEqual(
            model.indexFromLayout(layout2), model.index(2, 0, QModelIndex())
        )

        manager.clear()
        self.assertEqual(model.rowCount(QModelIndex()), 1)
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            None,
        )
        self.assertEqual(
            model.data(
                model.index(0, 0, QModelIndex()), QgsLayoutManagerModel.Role.LayoutRole
            ),
            None,
        )
        self.assertEqual(model.layoutFromIndex(model.index(0, 0, QModelIndex())), None)

    def testProxyModel(self):
        project = QgsProject()
        manager = QgsLayoutManager(project)
        model = QgsLayoutManagerModel(manager)
        proxy = QgsLayoutManagerProxyModel()
        proxy.setSourceModel(model)
        self.assertEqual(proxy.rowCount(QModelIndex()), 0)
        self.assertEqual(
            proxy.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            None,
        )
        self.assertEqual(
            proxy.data(
                model.index(0, 0, QModelIndex()), QgsLayoutManagerModel.Role.LayoutRole
            ),
            None,
        )

        layout = QgsPrintLayout(project)
        layout.setName("ccc")
        self.assertTrue(manager.addLayout(layout))

        self.assertEqual(proxy.rowCount(QModelIndex()), 1)
        self.assertEqual(
            proxy.data(proxy.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "ccc",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(0, 0, QModelIndex()), QgsLayoutManagerModel.Role.LayoutRole
            ),
            layout,
        )
        self.assertEqual(
            proxy.data(proxy.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            None,
        )
        self.assertEqual(
            proxy.data(
                proxy.index(1, 0, QModelIndex()), QgsLayoutManagerModel.Role.LayoutRole
            ),
            None,
        )

        layout2 = QgsPrintLayout(project)
        layout2.setName("bbb")
        self.assertTrue(manager.addLayout(layout2))
        self.assertEqual(proxy.rowCount(QModelIndex()), 2)
        self.assertEqual(
            proxy.data(proxy.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "bbb",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(0, 0, QModelIndex()), QgsLayoutManagerModel.Role.LayoutRole
            ),
            layout2,
        )
        self.assertEqual(
            proxy.data(proxy.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "ccc",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(1, 0, QModelIndex()), QgsLayoutManagerModel.Role.LayoutRole
            ),
            layout,
        )

        layout.setName("aaa")
        self.assertEqual(
            proxy.data(proxy.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "aaa",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(0, 0, QModelIndex()), QgsLayoutManagerModel.Role.LayoutRole
            ),
            layout,
        )
        self.assertEqual(
            proxy.data(proxy.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "bbb",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(1, 0, QModelIndex()), QgsLayoutManagerModel.Role.LayoutRole
            ),
            layout2,
        )

        model.setAllowEmptyLayout(True)
        self.assertEqual(proxy.rowCount(QModelIndex()), 3)
        self.assertEqual(
            proxy.data(proxy.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            None,
        )
        self.assertEqual(
            proxy.data(
                proxy.index(0, 0, QModelIndex()), QgsLayoutManagerModel.Role.LayoutRole
            ),
            None,
        )
        self.assertEqual(
            proxy.data(proxy.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "aaa",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(1, 0, QModelIndex()), QgsLayoutManagerModel.Role.LayoutRole
            ),
            layout,
        )
        self.assertEqual(
            proxy.data(proxy.index(2, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "bbb",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(2, 0, QModelIndex()), QgsLayoutManagerModel.Role.LayoutRole
            ),
            layout2,
        )

        r = QgsReport(project)
        r.setName("bbb2")
        manager.addLayout(r)
        self.assertEqual(proxy.rowCount(QModelIndex()), 4)
        self.assertEqual(
            proxy.data(proxy.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            None,
        )
        self.assertEqual(
            proxy.data(
                proxy.index(0, 0, QModelIndex()), QgsLayoutManagerModel.Role.LayoutRole
            ),
            None,
        )
        self.assertEqual(
            proxy.data(proxy.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "aaa",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(1, 0, QModelIndex()), QgsLayoutManagerModel.Role.LayoutRole
            ),
            layout,
        )
        self.assertEqual(
            proxy.data(proxy.index(2, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "bbb",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(2, 0, QModelIndex()), QgsLayoutManagerModel.Role.LayoutRole
            ),
            layout2,
        )
        self.assertEqual(
            proxy.data(proxy.index(3, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "bbb2",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(3, 0, QModelIndex()), QgsLayoutManagerModel.Role.LayoutRole
            ),
            r,
        )

        proxy.setFilterString("xx")
        self.assertEqual(proxy.rowCount(QModelIndex()), 1)
        self.assertEqual(
            proxy.data(proxy.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            None,
        )
        self.assertEqual(
            proxy.data(
                proxy.index(0, 0, QModelIndex()), QgsLayoutManagerModel.Role.LayoutRole
            ),
            None,
        )
        proxy.setFilterString("bb")
        self.assertEqual(proxy.rowCount(QModelIndex()), 3)
        self.assertEqual(
            proxy.data(proxy.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            None,
        )
        self.assertEqual(
            proxy.data(
                proxy.index(0, 0, QModelIndex()), QgsLayoutManagerModel.Role.LayoutRole
            ),
            None,
        )
        self.assertEqual(
            proxy.data(proxy.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "bbb",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(1, 0, QModelIndex()), QgsLayoutManagerModel.Role.LayoutRole
            ),
            layout2,
        )
        self.assertEqual(
            proxy.data(proxy.index(2, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "bbb2",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(2, 0, QModelIndex()), QgsLayoutManagerModel.Role.LayoutRole
            ),
            r,
        )
        proxy.setFilterString("")

        proxy.setFilters(QgsLayoutManagerProxyModel.Filter.FilterPrintLayouts)
        self.assertEqual(
            proxy.filters(), QgsLayoutManagerProxyModel.Filter.FilterPrintLayouts
        )
        self.assertEqual(proxy.rowCount(QModelIndex()), 3)
        self.assertEqual(
            proxy.data(proxy.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            None,
        )
        self.assertEqual(
            proxy.data(
                proxy.index(0, 0, QModelIndex()), QgsLayoutManagerModel.Role.LayoutRole
            ),
            None,
        )
        self.assertEqual(
            proxy.data(proxy.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "aaa",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(1, 0, QModelIndex()), QgsLayoutManagerModel.Role.LayoutRole
            ),
            layout,
        )
        self.assertEqual(
            proxy.data(proxy.index(2, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "bbb",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(2, 0, QModelIndex()), QgsLayoutManagerModel.Role.LayoutRole
            ),
            layout2,
        )
        proxy.setFilterString("bb")
        self.assertEqual(proxy.rowCount(QModelIndex()), 2)
        self.assertEqual(
            proxy.data(proxy.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            None,
        )
        self.assertEqual(
            proxy.data(
                proxy.index(0, 0, QModelIndex()), QgsLayoutManagerModel.Role.LayoutRole
            ),
            None,
        )
        self.assertEqual(
            proxy.data(proxy.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "bbb",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(1, 0, QModelIndex()), QgsLayoutManagerModel.Role.LayoutRole
            ),
            layout2,
        )
        proxy.setFilterString("")

        proxy.setFilters(QgsLayoutManagerProxyModel.Filter.FilterReports)
        self.assertEqual(
            proxy.filters(), QgsLayoutManagerProxyModel.Filter.FilterReports
        )
        self.assertEqual(proxy.rowCount(QModelIndex()), 2)
        self.assertEqual(
            proxy.data(proxy.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            None,
        )
        self.assertEqual(
            proxy.data(
                proxy.index(0, 0, QModelIndex()), QgsLayoutManagerModel.Role.LayoutRole
            ),
            None,
        )
        self.assertEqual(
            proxy.data(proxy.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "bbb2",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(1, 0, QModelIndex()), QgsLayoutManagerModel.Role.LayoutRole
            ),
            r,
        )

        proxy.setFilterString("bb")
        self.assertEqual(proxy.rowCount(QModelIndex()), 2)
        self.assertEqual(
            proxy.data(proxy.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            None,
        )
        self.assertEqual(
            proxy.data(
                proxy.index(0, 0, QModelIndex()), QgsLayoutManagerModel.Role.LayoutRole
            ),
            None,
        )
        self.assertEqual(
            proxy.data(proxy.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "bbb2",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(1, 0, QModelIndex()), QgsLayoutManagerModel.Role.LayoutRole
            ),
            r,
        )
        proxy.setFilterString("aaa")
        self.assertEqual(proxy.rowCount(QModelIndex()), 1)
        self.assertEqual(
            proxy.data(proxy.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            None,
        )
        self.assertEqual(
            proxy.data(
                proxy.index(0, 0, QModelIndex()), QgsLayoutManagerModel.Role.LayoutRole
            ),
            None,
        )
        proxy.setFilterString("")

        proxy.setFilters(
            QgsLayoutManagerProxyModel.Filter.FilterPrintLayouts
            | QgsLayoutManagerProxyModel.Filter.FilterReports
        )
        self.assertEqual(
            proxy.filters(),
            QgsLayoutManagerProxyModel.Filter.FilterPrintLayouts
            | QgsLayoutManagerProxyModel.Filter.FilterReports,
        )
        self.assertEqual(proxy.rowCount(QModelIndex()), 4)


if __name__ == "__main__":
    unittest.main()
