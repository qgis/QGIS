"""QGIS Unit tests for QgsLayoutManager.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "(C) 2017 by Nyall Dawson"
__date__ = "15/03/2017"
__copyright__ = "Copyright 2017, The QGIS Project"

from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    QgsLayoutManager,
    QgsMasterLayoutInterface,
    QgsPrintLayout,
    QgsProject,
    QgsReport,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsLayoutManager(QgisTestCase):

    def setUp(self):
        """Run before each test."""
        self.manager = None
        self.aboutFired = False

    def tearDown(self):
        """Run after each test."""
        pass

    def testAddLayout(self):
        project = QgsProject()
        layout = QgsPrintLayout(project)
        layout.setName("test layout")

        manager = QgsLayoutManager(project)

        layout_about_to_be_added_spy = QSignalSpy(manager.layoutAboutToBeAdded)
        layout_added_spy = QSignalSpy(manager.layoutAdded)
        self.assertTrue(manager.addLayout(layout))
        self.assertEqual(len(layout_about_to_be_added_spy), 1)
        self.assertEqual(layout_about_to_be_added_spy[0][0], "test layout")
        self.assertEqual(len(layout_added_spy), 1)
        self.assertEqual(layout_added_spy[0][0], "test layout")

        # adding it again should fail
        self.assertFalse(manager.addLayout(layout))

        # try adding a second layout
        layout2 = QgsPrintLayout(project)
        layout2.setName("test layout2")
        self.assertTrue(manager.addLayout(layout2))
        self.assertEqual(len(layout_added_spy), 2)
        self.assertEqual(layout_about_to_be_added_spy[1][0], "test layout2")
        self.assertEqual(len(layout_about_to_be_added_spy), 2)
        self.assertEqual(layout_added_spy[1][0], "test layout2")

        # adding a layout with duplicate name should fail
        layout3 = QgsPrintLayout(project)
        layout3.setName("test layout2")
        self.assertFalse(manager.addLayout(layout3))

    def testLayouts(self):
        project = QgsProject()
        manager = QgsLayoutManager(project)
        layout = QgsPrintLayout(project)
        layout.setName("test layout")
        layout2 = QgsPrintLayout(project)
        layout2.setName("test layout2")
        layout3 = QgsPrintLayout(project)
        layout3.setName("test layout3")

        manager.addLayout(layout)
        self.assertEqual(manager.layouts(), [layout])
        manager.addLayout(layout2)
        self.assertEqual(set(manager.layouts()), {layout, layout2})
        manager.addLayout(layout3)
        self.assertEqual(set(manager.layouts()), {layout, layout2, layout3})

    def aboutToBeRemoved(self, name):
        # composition should still exist at this time
        self.assertEqual(name, "test composition")
        self.assertTrue(self.manager.compositionByName("test composition"))
        self.aboutFired = True

    def layoutAboutToBeRemoved(self, name):
        # layout should still exist at this time
        self.assertEqual(name, "test layout")
        self.assertTrue(self.manager.layoutByName("test layout"))
        self.aboutFired = True

    def testRemoveLayout(self):
        project = QgsProject()
        layout = QgsPrintLayout(project)
        layout.setName("test layout")

        self.manager = QgsLayoutManager(project)
        layout_removed_spy = QSignalSpy(self.manager.layoutRemoved)
        layout_about_to_be_removed_spy = QSignalSpy(self.manager.layoutAboutToBeRemoved)
        # tests that layout still exists when layoutAboutToBeRemoved is fired
        self.manager.layoutAboutToBeRemoved.connect(self.layoutAboutToBeRemoved)

        # not added, should fail
        self.assertFalse(self.manager.removeLayout(layout))
        self.assertEqual(len(layout_removed_spy), 0)
        self.assertEqual(len(layout_about_to_be_removed_spy), 0)

        self.assertTrue(self.manager.addLayout(layout))
        self.assertEqual(self.manager.layouts(), [layout])
        self.assertTrue(self.manager.removeLayout(layout))
        self.assertEqual(len(self.manager.layouts()), 0)
        self.assertEqual(len(layout_removed_spy), 1)
        self.assertEqual(layout_removed_spy[0][0], "test layout")
        self.assertEqual(len(layout_about_to_be_removed_spy), 1)
        self.assertEqual(layout_about_to_be_removed_spy[0][0], "test layout")
        self.assertTrue(self.aboutFired)
        self.manager = None

    def testClear(self):
        project = QgsProject()
        manager = QgsLayoutManager(project)

        # add a bunch of layouts
        layout = QgsPrintLayout(project)
        layout.setName("test layout")
        layout2 = QgsPrintLayout(project)
        layout2.setName("test layout2")
        layout3 = QgsPrintLayout(project)
        layout3.setName("test layout3")

        manager.addLayout(layout)
        manager.addLayout(layout2)
        manager.addLayout(layout3)

        layout_removed_spy = QSignalSpy(manager.layoutRemoved)
        layout_about_to_be_removed_spy = QSignalSpy(manager.layoutAboutToBeRemoved)
        manager.clear()
        self.assertEqual(len(manager.layouts()), 0)
        self.assertEqual(len(layout_removed_spy), 3)
        self.assertEqual(len(layout_about_to_be_removed_spy), 3)

    def testLayoutsByName(self):
        project = QgsProject()
        manager = QgsLayoutManager(project)

        # add a bunch of layouts
        layout = QgsPrintLayout(project)
        layout.setName("test layout")
        layout2 = QgsPrintLayout(project)
        layout2.setName("test layout2")
        layout3 = QgsPrintLayout(project)
        layout3.setName("test layout3")

        manager.addLayout(layout)
        manager.addLayout(layout2)
        manager.addLayout(layout3)

        self.assertFalse(manager.layoutByName("asdf"))
        self.assertEqual(manager.layoutByName("test layout"), layout)
        self.assertEqual(manager.layoutByName("test layout2"), layout2)
        self.assertEqual(manager.layoutByName("test layout3"), layout3)

    def testReadWriteXml(self):
        """
        Test reading and writing layout manager state to XML
        """
        project = QgsProject()
        manager = QgsLayoutManager(project)

        # add a bunch of layouts
        layout = QgsPrintLayout(project)
        layout.setName("test layout")
        layout2 = QgsPrintLayout(project)
        layout2.setName("test layout2")
        layout3 = QgsPrintLayout(project)
        layout3.setName("test layout3")

        manager.addLayout(layout)
        manager.addLayout(layout2)
        manager.addLayout(layout3)

        # save to xml
        doc = QDomDocument("testdoc")
        elem = manager.writeXml(doc)
        doc.appendChild(elem)

        # restore from xml
        project2 = QgsProject()
        manager2 = QgsLayoutManager(project2)
        self.assertTrue(manager2.readXml(elem, doc))

        self.assertEqual(len(manager2.layouts()), 3)
        names = [c.name() for c in manager2.layouts()]
        self.assertCountEqual(names, ["test layout", "test layout2", "test layout3"])

    def testDuplicateLayout(self):
        """
        Test duplicating layouts
        """
        project = QgsProject()
        manager = QgsLayoutManager(project)
        doc = QDomDocument("testdoc")
        self.assertFalse(manager.duplicateLayout(None, "dest"))

        layout = QgsPrintLayout(project)
        layout.setName("test layout")
        layout.initializeDefaults()
        manager.addLayout(layout)
        # duplicate name
        self.assertFalse(manager.duplicateLayout(layout, "test layout"))
        result = manager.duplicateLayout(layout, "dupe layout")

        self.assertTrue(result)
        # make sure result in stored in manager
        self.assertEqual(result, manager.layoutByName("dupe layout"))
        self.assertEqual(result.name(), "dupe layout")
        self.assertEqual(result.pageCollection().pageCount(), 1)

    def testGenerateUniqueTitle(self):
        project = QgsProject()
        manager = QgsLayoutManager(project)
        self.assertEqual(
            manager.generateUniqueTitle(QgsMasterLayoutInterface.Type.PrintLayout),
            "Layout 1",
        )
        self.assertEqual(
            manager.generateUniqueTitle(QgsMasterLayoutInterface.Type.Report),
            "Report 1",
        )

        layout = QgsPrintLayout(project)
        layout.setName(manager.generateUniqueTitle())
        manager.addLayout(layout)

        self.assertEqual(manager.generateUniqueTitle(), "Layout 2")
        self.assertEqual(
            manager.generateUniqueTitle(QgsMasterLayoutInterface.Type.Report),
            "Report 1",
        )
        layout2 = QgsPrintLayout(project)
        layout2.setName(manager.generateUniqueTitle())
        manager.addLayout(layout2)

        self.assertEqual(manager.generateUniqueTitle(), "Layout 3")

        report1 = QgsReport(project)
        report1.setName(
            manager.generateUniqueTitle(QgsMasterLayoutInterface.Type.Report)
        )
        manager.addLayout(report1)
        self.assertEqual(
            manager.generateUniqueTitle(QgsMasterLayoutInterface.Type.Report),
            "Report 2",
        )

        manager.clear()
        self.assertEqual(manager.generateUniqueTitle(), "Layout 1")
        self.assertEqual(
            manager.generateUniqueTitle(QgsMasterLayoutInterface.Type.Report),
            "Report 1",
        )

    def testRenameSignal(self):
        project = QgsProject()
        manager = QgsLayoutManager(project)
        layout = QgsPrintLayout(project)
        layout.setName("c1")
        manager.addLayout(layout)
        layout2 = QgsPrintLayout(project)
        layout2.setName("c2")
        manager.addLayout(layout2)

        layout_renamed_spy = QSignalSpy(manager.layoutRenamed)
        layout.setName("d1")
        self.assertEqual(len(layout_renamed_spy), 1)
        # self.assertEqual(layout_renamed_spy[0][0], layout)
        self.assertEqual(layout_renamed_spy[0][1], "d1")
        layout2.setName("d2")
        self.assertEqual(len(layout_renamed_spy), 2)
        # self.assertEqual(layout_renamed_spy[1][0], layout2)
        self.assertEqual(layout_renamed_spy[1][1], "d2")


if __name__ == "__main__":
    unittest.main()
