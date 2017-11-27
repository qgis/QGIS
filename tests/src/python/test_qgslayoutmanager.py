# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsLayoutManager.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2017 by Nyall Dawson'
__date__ = '15/03/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA


from qgis.PyQt.QtXml import QDomDocument

from qgis.core import (QgsComposition,
                       QgsLayoutManager,
                       QgsProject)

from qgis.testing import start_app, unittest
from utilities import unitTestDataPath
from qgis.PyQt.QtXml import QDomDocument
from qgis.PyQt.QtTest import QSignalSpy

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsLayoutManager(unittest.TestCase):

    def setUp(self):
        """Run before each test."""
        self.manager = None
        self.aboutFired = False

    def tearDown(self):
        """Run after each test."""
        pass

    def testAddComposition(self):
        project = QgsProject()
        composition = QgsComposition(project)
        composition.setName('test composition')

        manager = QgsLayoutManager(project)

        composition_about_to_be_added_spy = QSignalSpy(manager.compositionAboutToBeAdded)
        composition_added_spy = QSignalSpy(manager.compositionAdded)
        self.assertTrue(manager.addComposition(composition))
        self.assertEqual(len(composition_about_to_be_added_spy), 1)
        self.assertEqual(composition_about_to_be_added_spy[0][0], 'test composition')
        self.assertEqual(len(composition_added_spy), 1)
        self.assertEqual(composition_added_spy[0][0], 'test composition')

        # adding it again should fail
        self.assertFalse(manager.addComposition(composition))

        # try adding a second composition
        composition2 = QgsComposition(project)
        composition2.setName('test composition2')
        self.assertTrue(manager.addComposition(composition2))
        self.assertEqual(len(composition_added_spy), 2)
        self.assertEqual(composition_about_to_be_added_spy[1][0], 'test composition2')
        self.assertEqual(len(composition_about_to_be_added_spy), 2)
        self.assertEqual(composition_added_spy[1][0], 'test composition2')

        # adding a composition with duplicate name should fail
        composition3 = QgsComposition(project)
        composition3.setName('test composition2')
        self.assertFalse(manager.addComposition(composition3))

    def testCompositions(self):
        project = QgsProject()
        manager = QgsLayoutManager(project)
        composition = QgsComposition(project)
        composition.setName('test composition')
        composition2 = QgsComposition(project)
        composition2.setName('test composition2')
        composition3 = QgsComposition(project)
        composition3.setName('test composition3')

        manager.addComposition(composition)
        self.assertEqual(manager.compositions(), [composition])
        manager.addComposition(composition2)
        self.assertEqual(set(manager.compositions()), {composition, composition2})
        manager.addComposition(composition3)
        self.assertEqual(set(manager.compositions()), {composition, composition2, composition3})

    def aboutToBeRemoved(self, name):
        # composition should still exist at this time
        self.assertEqual(name, 'test composition')
        self.assertTrue(self.manager.compositionByName('test composition'))
        self.aboutFired = True

    def testRemoveComposition(self):
        project = QgsProject()
        composition = QgsComposition(project)
        composition.setName('test composition')

        self.manager = QgsLayoutManager(project)
        composition_removed_spy = QSignalSpy(self.manager.compositionRemoved)
        composition_about_to_be_removed_spy = QSignalSpy(self.manager.compositionAboutToBeRemoved)
        # tests that composition still exists when compositionAboutToBeRemoved is fired
        self.manager.compositionAboutToBeRemoved.connect(self.aboutToBeRemoved)

        # not added, should fail
        self.assertFalse(self.manager.removeComposition(composition))
        self.assertEqual(len(composition_removed_spy), 0)
        self.assertEqual(len(composition_about_to_be_removed_spy), 0)

        self.assertTrue(self.manager.addComposition(composition))
        self.assertEqual(self.manager.compositions(), [composition])
        self.assertTrue(self.manager.removeComposition(composition))
        self.assertEqual(len(self.manager.compositions()), 0)
        self.assertEqual(len(composition_removed_spy), 1)
        self.assertEqual(composition_removed_spy[0][0], 'test composition')
        self.assertEqual(len(composition_about_to_be_removed_spy), 1)
        self.assertEqual(composition_about_to_be_removed_spy[0][0], 'test composition')
        self.assertTrue(self.aboutFired)
        self.manager = None

    def testClear(self):
        project = QgsProject()
        manager = QgsLayoutManager(project)

        # add a bunch of compositions
        composition = QgsComposition(project)
        composition.setName('test composition')
        composition2 = QgsComposition(project)
        composition2.setName('test composition2')
        composition3 = QgsComposition(project)
        composition3.setName('test composition3')

        manager.addComposition(composition)
        manager.addComposition(composition2)
        manager.addComposition(composition3)

        composition_removed_spy = QSignalSpy(manager.compositionRemoved)
        composition_about_to_be_removed_spy = QSignalSpy(manager.compositionAboutToBeRemoved)
        manager.clear()
        self.assertEqual(len(manager.compositions()), 0)
        self.assertEqual(len(composition_removed_spy), 3)
        self.assertEqual(len(composition_about_to_be_removed_spy), 3)

    def testCompositionByName(self):
        project = QgsProject()
        manager = QgsLayoutManager(project)

        # add a bunch of compositions
        composition = QgsComposition(project)
        composition.setName('test composition')
        composition2 = QgsComposition(project)
        composition2.setName('test composition2')
        composition3 = QgsComposition(project)
        composition3.setName('test composition3')

        manager.addComposition(composition)
        manager.addComposition(composition2)
        manager.addComposition(composition3)

        self.assertFalse(manager.compositionByName('asdf'))
        self.assertEqual(manager.compositionByName('test composition'), composition)
        self.assertEqual(manager.compositionByName('test composition2'), composition2)
        self.assertEqual(manager.compositionByName('test composition3'), composition3)

    def testReadWriteXml(self):
        """
        Test reading and writing layout manager state to XML
        """
        project = QgsProject()
        manager = QgsLayoutManager(project)

        # add a bunch of compositions
        composition = QgsComposition(project)
        composition.setName('test composition')
        composition2 = QgsComposition(project)
        composition2.setName('test composition2')
        composition3 = QgsComposition(project)
        composition3.setName('test composition3')

        manager.addComposition(composition)
        manager.addComposition(composition2)
        manager.addComposition(composition3)

        # save to xml
        doc = QDomDocument("testdoc")
        elem = manager.writeXml(doc)
        doc.appendChild(elem)

        # restore from xml
        project2 = QgsProject()
        manager2 = QgsLayoutManager(project2)
        self.assertTrue(manager2.readXml(elem, doc))

        self.assertEqual(len(manager2.compositions()), 3)
        names = [c.name() for c in manager2.compositions()]
        self.assertEqual(set(names), {'test composition', 'test composition2', 'test composition3'})

    def testSaveAsTemplate(self):
        """
        Test saving composition as template
        """
        project = QgsProject()
        manager = QgsLayoutManager(project)
        doc = QDomDocument("testdoc")
        self.assertFalse(manager.saveAsTemplate('not in manager', doc))

        composition = QgsComposition(project)
        composition.setName('test composition')
        manager.addComposition(composition)
        self.assertTrue(manager.saveAsTemplate('test composition', doc))

    def testDuplicateComposition(self):
        """
        Test duplicating compositions
        """
        project = QgsProject()
        manager = QgsLayoutManager(project)
        doc = QDomDocument("testdoc")
        self.assertFalse(manager.duplicateComposition('not in manager', 'dest'))

        composition = QgsComposition(project)
        composition.setName('test composition')
        composition.setPaperSize(100, 200)
        manager.addComposition(composition)
        # duplicate name
        self.assertFalse(manager.duplicateComposition('test composition', 'test composition'))

        result = manager.duplicateComposition('test composition', 'dupe composition')
        self.assertTrue(result)
        # make sure result in stored in manager
        self.assertEqual(result, manager.compositionByName('dupe composition'))
        self.assertEqual(result.name(), 'dupe composition')
        self.assertEqual(result.paperHeight(), 200)
        self.assertEqual(result.paperWidth(), 100)

    def testGenerateUniqueTitle(self):
        project = QgsProject()
        manager = QgsLayoutManager(project)
        self.assertEqual(manager.generateUniqueTitle(), 'Composer 1')

        composition = QgsComposition(project)
        composition.setName(manager.generateUniqueTitle())
        manager.addComposition(composition)

        self.assertEqual(manager.generateUniqueTitle(), 'Composer 2')
        composition2 = QgsComposition(project)
        composition2.setName(manager.generateUniqueTitle())
        manager.addComposition(composition2)

        self.assertEqual(manager.generateUniqueTitle(), 'Composer 3')
        manager.clear()
        self.assertEqual(manager.generateUniqueTitle(), 'Composer 1')

    def testRenameSignal(self):
        project = QgsProject()
        manager = QgsLayoutManager(project)
        composition = QgsComposition(project)
        composition.setName('c1')
        manager.addComposition(composition)
        composition2 = QgsComposition(project)
        composition2.setName('c2')
        manager.addComposition(composition2)

        composition_renamed_spy = QSignalSpy(manager.compositionRenamed)
        composition.setName('d1')
        self.assertEqual(len(composition_renamed_spy), 1)
        self.assertEqual(composition_renamed_spy[0][0], composition)
        self.assertEqual(composition_renamed_spy[0][1], 'd1')
        composition2.setName('d2')
        self.assertEqual(len(composition_renamed_spy), 2)
        self.assertEqual(composition_renamed_spy[1][0], composition2)
        self.assertEqual(composition_renamed_spy[1][1], 'd2')


if __name__ == '__main__':
    unittest.main()
