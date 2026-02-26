"""QGIS Unit tests for QgsSelectiveMaskingSourceSetManager.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import unittest

from qgis.core import (
    Qgis,
    QgsProject,
    QgsReadWriteContext,
    QgsSelectiveMaskingSourceSet,
    QgsSelectiveMaskingSourceSetManager,
    QgsSelectiveMaskSource,
)
from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtXml import QDomDocument
from qgis.testing import QgisTestCase, start_app
from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsSelectiveMaskingSourceSetManager(QgisTestCase):
    def setUp(self):
        """Run before each test."""
        self.manager = None
        self.aboutFired = False

    def tearDown(self):
        """Run after each test."""
        pass

    def testAddSet(self):
        project = QgsProject()
        source_set = QgsSelectiveMaskingSourceSet()
        source_set.setName("test source set")

        manager = QgsSelectiveMaskingSourceSetManager(project)

        set_about_to_be_added_spy = QSignalSpy(manager.setAboutToBeAdded)
        set_added_spy = QSignalSpy(manager.setAdded)
        self.assertTrue(manager.addSet(source_set))
        self.assertEqual(len(set_about_to_be_added_spy), 1)
        self.assertEqual(set_about_to_be_added_spy[0][0], "test source set")
        self.assertEqual(len(set_added_spy), 1)
        self.assertEqual(set_added_spy[0][0], "test source set")

        # adding it again should fail
        self.assertFalse(manager.addSet(source_set))

        # try adding a second set
        source_set2 = QgsSelectiveMaskingSourceSet()
        source_set2.setName("test source set 2")
        self.assertTrue(manager.addSet(source_set2))
        self.assertEqual(len(set_added_spy), 2)
        self.assertEqual(set_about_to_be_added_spy[1][0], "test source set 2")
        self.assertEqual(len(set_about_to_be_added_spy), 2)
        self.assertEqual(set_added_spy[1][0], "test source set 2")

        # adding a set with duplicate name should fail
        source_set3 = QgsSelectiveMaskingSourceSet()
        source_set3.setName("test source set 2")
        self.assertFalse(manager.addSet(source_set3))

    def testSets(self):
        project = QgsProject()
        manager = QgsSelectiveMaskingSourceSetManager(project)
        source_set = QgsSelectiveMaskingSourceSet()
        source_set.setName("test set")
        source_set2 = QgsSelectiveMaskingSourceSet()
        source_set2.setName("test set 2")
        source_set3 = QgsSelectiveMaskingSourceSet()
        source_set3.setName("test set 3")

        manager.addSet(source_set)
        self.assertEqual([s.name() for s in manager.sets()], ["test set"])
        manager.addSet(source_set2)
        self.assertCountEqual(
            [s.name() for s in manager.sets()], ["test set", "test set 2"]
        )
        manager.addSet(source_set3)
        self.assertCountEqual(
            [s.name() for s in manager.sets()], ["test set", "test set 2", "test set 3"]
        )

    def setAboutToBeRemoved(self, name):
        # set should still exist at this time
        self.assertEqual(name, "test source set")
        self.assertEqual(
            self.manager.setByName("test source set").name(), "test source set"
        )
        self.aboutFired = True

    def testRemoveSet(self):
        project = QgsProject()
        source_set = QgsSelectiveMaskingSourceSet()
        source_set.setName("test source set")

        self.manager = QgsSelectiveMaskingSourceSetManager(project)
        set_removed_spy = QSignalSpy(self.manager.setRemoved)
        set_about_to_be_removed_spy = QSignalSpy(self.manager.setAboutToBeRemoved)
        # tests that set still exists when setAboutToBeRemoved is fired
        self.manager.setAboutToBeRemoved.connect(self.setAboutToBeRemoved)

        # not added, should fail
        self.assertFalse(self.manager.removeSet("xxx"))
        self.assertEqual(len(set_removed_spy), 0)
        self.assertEqual(len(set_about_to_be_removed_spy), 0)

        self.assertTrue(self.manager.addSet(source_set))
        self.assertEqual([s.name() for s in self.manager.sets()], ["test source set"])
        self.assertTrue(self.manager.removeSet("test source set"))
        self.assertEqual(len(self.manager.sets()), 0)
        self.assertEqual(len(set_removed_spy), 1)
        self.assertEqual(set_removed_spy[0][0], "test source set")
        self.assertEqual(len(set_about_to_be_removed_spy), 1)
        self.assertEqual(set_about_to_be_removed_spy[0][0], "test source set")
        self.assertTrue(self.aboutFired)
        self.manager = None

    def testClear(self):
        project = QgsProject()
        manager = QgsSelectiveMaskingSourceSetManager(project)

        # add a bunch of sets
        source_set = QgsSelectiveMaskingSourceSet()
        source_set.setName("test set")
        source_set2 = QgsSelectiveMaskingSourceSet()
        source_set2.setName("test set 2")
        source_set3 = QgsSelectiveMaskingSourceSet()
        source_set3.setName("test set 3")

        manager.addSet(source_set)
        manager.addSet(source_set2)
        manager.addSet(source_set3)

        set_removed_spy = QSignalSpy(manager.setRemoved)
        set_about_to_be_removed_spy = QSignalSpy(manager.setAboutToBeRemoved)
        manager.clear()
        self.assertEqual(len(manager.sets()), 0)
        self.assertEqual(len(set_removed_spy), 3)
        self.assertEqual(len(set_about_to_be_removed_spy), 3)

    def testSetsByName(self):
        project = QgsProject()
        manager = QgsSelectiveMaskingSourceSetManager(project)

        # add a bunch of sets
        source_set = QgsSelectiveMaskingSourceSet()
        source_set.setName("test set")
        source_set2 = QgsSelectiveMaskingSourceSet()
        source_set2.setName("test set 2")
        source_set3 = QgsSelectiveMaskingSourceSet()
        source_set3.setName("test set 3")

        manager.addSet(source_set)
        manager.addSet(source_set2)
        manager.addSet(source_set3)

        self.assertFalse(manager.setByName("asdf").isValid())
        self.assertEqual(manager.setByName("test set").name(), "test set")
        self.assertEqual(manager.setByName("test set 2").name(), "test set 2")
        self.assertEqual(manager.setByName("test set 3").name(), "test set 3")

    def testSetsById(self):
        project = QgsProject()
        manager = QgsSelectiveMaskingSourceSetManager(project)

        # add a bunch of sets
        source_set = QgsSelectiveMaskingSourceSet()
        source_set.setName("test set")
        source_set2 = QgsSelectiveMaskingSourceSet()
        source_set2.setName("test set 2")
        source_set3 = QgsSelectiveMaskingSourceSet()
        source_set3.setName("test set 3")

        manager.addSet(source_set)
        manager.addSet(source_set2)
        manager.addSet(source_set3)

        self.assertFalse(manager.setById("asdf").isValid())
        self.assertEqual(manager.setById(source_set.id()).name(), "test set")
        self.assertEqual(manager.setById(source_set2.id()).name(), "test set 2")
        self.assertEqual(manager.setById(source_set3.id()).name(), "test set 3")

    def test_update_set(self):
        project = QgsProject()
        manager = QgsSelectiveMaskingSourceSetManager(project)

        # add a bunch of sets
        source_set = QgsSelectiveMaskingSourceSet()
        source_set.setName("test set")
        source_set.append(
            QgsSelectiveMaskSource("layer_1", Qgis.SelectiveMaskSourceType.Label)
        )
        source_set.append(
            QgsSelectiveMaskSource("layer_2", Qgis.SelectiveMaskSourceType.Label)
        )

        source_set2 = QgsSelectiveMaskingSourceSet()
        source_set2.setName("test set 2")
        source_set2.append(
            QgsSelectiveMaskSource("layer_3", Qgis.SelectiveMaskSourceType.Label)
        )

        source_set3 = QgsSelectiveMaskingSourceSet()
        source_set3.setName("test set 3")

        manager.addSet(source_set)
        manager.addSet(source_set2)
        manager.addSet(source_set3)

        self.assertEqual(len(manager.sets()), 3)
        updated_set = QgsSelectiveMaskingSourceSet(source_set)
        updated_set.setSources(
            [QgsSelectiveMaskSource("layer_4", Qgis.SelectiveMaskSourceType.Label)]
        )
        self.assertEqual(
            [s.layerId() for s in manager.setById(source_set.id()).sources()],
            ["layer_1", "layer_2"],
        )
        self.assertEqual(
            [s.layerId() for s in manager.setById(source_set2.id()).sources()],
            ["layer_3"],
        )
        self.assertEqual(
            [s.layerId() for s in manager.setById(source_set3.id()).sources()], []
        )
        self.assertTrue(manager.updateSet(updated_set))

        self.assertEqual(len(manager.sets()), 3)
        self.assertEqual(
            [s.layerId() for s in manager.setById(source_set.id()).sources()],
            ["layer_4"],
        )
        self.assertEqual(
            [s.layerId() for s in manager.setById(source_set2.id()).sources()],
            ["layer_3"],
        )
        self.assertEqual(
            [s.layerId() for s in manager.setById(source_set3.id()).sources()], []
        )

        updated_set = QgsSelectiveMaskingSourceSet(source_set3)
        updated_set.setSources(
            [
                QgsSelectiveMaskSource("layer_5", Qgis.SelectiveMaskSourceType.Label),
                QgsSelectiveMaskSource("layer_6", Qgis.SelectiveMaskSourceType.Label),
            ]
        )
        self.assertTrue(manager.updateSet(updated_set))
        self.assertEqual(len(manager.sets()), 3)
        self.assertEqual(
            [s.layerId() for s in manager.setById(source_set.id()).sources()],
            ["layer_4"],
        )
        self.assertEqual(
            [s.layerId() for s in manager.setById(source_set2.id()).sources()],
            ["layer_3"],
        )
        self.assertEqual(
            [s.layerId() for s in manager.setById(source_set3.id()).sources()],
            ["layer_5", "layer_6"],
        )

        # can't update a set not in the manager
        self.assertFalse(manager.updateSet(QgsSelectiveMaskingSourceSet()))

    def testReadWriteXml(self):
        """
        Test reading and writing set manager state to XML
        """
        project = QgsProject()
        manager = QgsSelectiveMaskingSourceSetManager(project)

        # add a bunch of sets
        source_set = QgsSelectiveMaskingSourceSet()
        source_set.setName("test set")
        source_set2 = QgsSelectiveMaskingSourceSet()
        source_set2.setName("test set 2")
        source_set3 = QgsSelectiveMaskingSourceSet()
        source_set3.setName("test set 3")

        manager.addSet(source_set)
        manager.addSet(source_set2)
        manager.addSet(source_set3)

        # save to xml
        context = QgsReadWriteContext()
        doc = QDomDocument("testdoc")
        elem = manager.writeXml(doc, context)
        doc.appendChild(elem)

        # restore from xml
        project2 = QgsProject()
        manager2 = QgsSelectiveMaskingSourceSetManager(project2)
        self.assertTrue(manager2.readXml(elem, doc, context))

        self.assertEqual(len(manager2.sets()), 3)
        names = [c.name() for c in manager2.sets()]
        self.assertCountEqual(names, ["test set", "test set 2", "test set 3"])

    def testGenerateUniqueTitle(self):
        project = QgsProject()
        manager = QgsSelectiveMaskingSourceSetManager(project)
        self.assertEqual(
            manager.generateUniqueTitle(),
            "Masking Source Set 1",
        )

        source_set = QgsSelectiveMaskingSourceSet()
        source_set.setName(manager.generateUniqueTitle())
        manager.addSet(source_set)

        self.assertEqual(manager.generateUniqueTitle(), "Masking Source Set 2")
        source_set2 = QgsSelectiveMaskingSourceSet()
        source_set2.setName(manager.generateUniqueTitle())
        manager.addSet(source_set2)

        self.assertEqual(manager.generateUniqueTitle(), "Masking Source Set 3")

        manager.clear()
        self.assertEqual(manager.generateUniqueTitle(), "Masking Source Set 1")

    def testRenameSignal(self):
        project = QgsProject()
        manager = QgsSelectiveMaskingSourceSetManager(project)
        source_set = QgsSelectiveMaskingSourceSet()
        source_set.setName("c1")
        manager.addSet(source_set)
        source_set2 = QgsSelectiveMaskingSourceSet()
        source_set2.setName("c2")
        manager.addSet(source_set2)

        set_renamed_spy = QSignalSpy(manager.setRenamed)
        self.assertFalse(manager.renameSet("xxx", "yyy"))
        self.assertEqual(len(set_renamed_spy), 0)
        self.assertFalse(manager.renameSet("c1", "c2"))
        self.assertEqual(len(set_renamed_spy), 0)

        self.assertTrue(manager.renameSet("c1", "d1"))
        self.assertEqual(len(set_renamed_spy), 1)
        self.assertEqual(set_renamed_spy[0][1], "d1")
        self.assertTrue(manager.renameSet("c2", "d2"))
        self.assertEqual(len(set_renamed_spy), 2)
        self.assertEqual(set_renamed_spy[1][1], "d2")

    def testChangedSignal(self):
        project = QgsProject()
        manager = QgsSelectiveMaskingSourceSetManager(project)
        spy = QSignalSpy(manager.changed)
        source_set = QgsSelectiveMaskingSourceSet()
        source_set.setName("c1")
        manager.addSet(source_set)
        self.assertEqual(len(spy), 1)

        source_set = QgsSelectiveMaskingSourceSet()
        source_set.setName("c2")
        manager.addSet(source_set)
        self.assertEqual(len(spy), 2)

        manager.renameSet("c2", "c3")
        self.assertEqual(len(spy), 3)

        updated_set = QgsSelectiveMaskingSourceSet(source_set)
        updated_set.setSources(
            [QgsSelectiveMaskSource("layer_4", Qgis.SelectiveMaskSourceType.Label)]
        )
        manager.updateSet(updated_set)
        self.assertEqual(len(spy), 4)

        manager.removeSet("c3")
        self.assertEqual(len(spy), 5)

        manager.clear()
        self.assertEqual(len(spy), 6)


if __name__ == "__main__":
    unittest.main()
