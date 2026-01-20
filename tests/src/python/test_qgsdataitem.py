"""QGIS Unit tests for QgsDataItem

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Even Rouault"
__date__ = "14/11/2020 late in the night"
__copyright__ = "Copyright 2020, The QGIS Project"


import os

from qgis.core import QgsDataCollectionItem, QgsDirectoryItem, QgsDataItem, Qgis
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

app = start_app()


class DummyDataItem(QgsDataItem):

    def __init__(self):
        QgsDataItem.__init__(self, Qgis.BrowserItemType.Custom, None, "dummy", "dummy")
        self.setCapabilities(
            Qgis.BrowserItemCapability.Fast | Qgis.BrowserItemCapability.Fertile
        )

    def createChildren(self):
        child1 = QgsDataItem(Qgis.BrowserItemType.Custom, None, "child1", "child1")
        child1.addChildItem(
            QgsDataItem(Qgis.BrowserItemType.Custom, None, "grandchild1", "grandchild1")
        )
        grandchild2 = QgsDataItem(
            Qgis.BrowserItemType.Custom, None, "grandchild2", "grandchild2"
        )
        grandchild2.addChildItem(
            QgsDataItem(
                Qgis.BrowserItemType.Custom,
                None,
                "greatgrandchild1",
                "greatgrandchild1",
            )
        )
        child1.addChildItem(grandchild2)
        child2 = QgsDataItem(Qgis.BrowserItemType.Custom, None, "child2", "child2")
        child2.addChildItem(
            QgsDataItem(Qgis.BrowserItemType.Custom, None, "grandchild3", "grandchild3")
        )
        child3 = QgsDataItem(Qgis.BrowserItemType.Custom, None, "child3", "child3")

        return [child1, child2, child3]


class TestQgsDataItem(QgisTestCase):

    def test_ancestors(self):
        root_item = DummyDataItem()
        root_item.populate()
        # check that item is fully populated
        self.assertEqual(
            [c.name() for c in root_item.children()], ["child1", "child2", "child3"]
        )
        child1, child2, child3 = root_item.children()
        self.assertEqual(
            [c.name() for c in child1.children()], ["grandchild1", "grandchild2"]
        )
        grandchild1, grandchild2 = child1.children()
        self.assertEqual(
            [c.name() for c in grandchild2.children()], ["greatgrandchild1"]
        )
        greatgrandchild = grandchild2.children()[0]
        self.assertEqual([c.name() for c in child2.children()], ["grandchild3"])

        # test ancestor methods
        self.assertEqual(root_item.creatorAncestorDepth(), 0)
        self.assertEqual(child1.creatorAncestorDepth(), 1)
        self.assertEqual(child2.creatorAncestorDepth(), 1)
        self.assertEqual(child3.creatorAncestorDepth(), 1)
        self.assertEqual(grandchild1.creatorAncestorDepth(), 2)
        self.assertEqual(grandchild2.creatorAncestorDepth(), 2)
        self.assertEqual(greatgrandchild.creatorAncestorDepth(), 3)

        self.assertEqual(root_item.ancestorAtDepth(-1), None)
        self.assertEqual(root_item.ancestorAtDepth(0), root_item)
        self.assertEqual(root_item.ancestorAtDepth(1), None)

        self.assertEqual(child1.ancestorAtDepth(-1), None)
        self.assertEqual(child1.ancestorAtDepth(0), child1)
        self.assertEqual(child1.ancestorAtDepth(1), root_item)
        self.assertEqual(child1.ancestorAtDepth(2), None)

        self.assertEqual(child2.ancestorAtDepth(-1), None)
        self.assertEqual(child2.ancestorAtDepth(0), child2)
        self.assertEqual(child2.ancestorAtDepth(1), root_item)
        self.assertEqual(child2.ancestorAtDepth(2), None)

        self.assertEqual(child3.ancestorAtDepth(-1), None)
        self.assertEqual(child3.ancestorAtDepth(0), child3)
        self.assertEqual(child3.ancestorAtDepth(1), root_item)
        self.assertEqual(child3.ancestorAtDepth(2), None)

        self.assertEqual(grandchild1.ancestorAtDepth(-1), None)
        self.assertEqual(grandchild1.ancestorAtDepth(0), grandchild1)
        self.assertEqual(grandchild1.ancestorAtDepth(1), child1)
        self.assertEqual(grandchild1.ancestorAtDepth(2), root_item)
        self.assertEqual(grandchild1.ancestorAtDepth(3), None)

        self.assertEqual(grandchild2.ancestorAtDepth(-1), None)
        self.assertEqual(grandchild2.ancestorAtDepth(0), grandchild2)
        self.assertEqual(grandchild2.ancestorAtDepth(1), child1)
        self.assertEqual(grandchild2.ancestorAtDepth(2), root_item)
        self.assertEqual(grandchild2.ancestorAtDepth(3), None)

        self.assertEqual(greatgrandchild.ancestorAtDepth(-1), None)
        self.assertEqual(greatgrandchild.ancestorAtDepth(0), greatgrandchild)
        self.assertEqual(greatgrandchild.ancestorAtDepth(1), grandchild2)
        self.assertEqual(greatgrandchild.ancestorAtDepth(2), child1)
        self.assertEqual(greatgrandchild.ancestorAtDepth(3), root_item)
        self.assertEqual(greatgrandchild.ancestorAtDepth(4), None)

    def test_databaseConnection(self):

        dataitem = QgsDataCollectionItem(None, "name", "/invalid_path", "ogr")
        self.assertIsNone(dataitem.databaseConnection())
        dataitem = QgsDirectoryItem(
            None, "name", os.path.join(unitTestDataPath(), "provider")
        )
        children = dataitem.createChildren()
        # Check spatialite and gpkg
        spatialite_item = [i for i in children if i.path().endswith("spatialite.db")][0]
        geopackage_item = [i for i in children if i.path().endswith("geopackage.gpkg")][
            0
        ]
        textfile_item = [i for i in children if i.path().endswith(".xml")][0]

        self.assertIsNotNone(spatialite_item.databaseConnection())
        self.assertIsNotNone(geopackage_item.databaseConnection())
        self.assertIsNone(textfile_item.databaseConnection())


if __name__ == "__main__":
    unittest.main()
