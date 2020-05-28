# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsBookmarkManager.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2019 by Nyall Dawson'
__date__ = '02/09/2019'
__copyright__ = 'Copyright 2019, The QGIS Project'

import qgis  # NOQA
import os

from qgis.PyQt.QtCore import QCoreApplication, QLocale, QTemporaryDir, QEvent
from qgis.PyQt.QtXml import QDomDocument

from qgis.core import (QgsBookmark,
                       QgsBookmarkManager,
                       QgsProject,
                       QgsReferencedRectangle,
                       QgsRectangle,
                       QgsCoordinateReferenceSystem,
                       QgsSettings,
                       QgsApplication)

from qgis.testing import start_app, unittest
from utilities import unitTestDataPath
from qgis.PyQt.QtXml import QDomDocument
from qgis.PyQt.QtTest import QSignalSpy

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsBookmarkManager(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("QGIS_TestQgsBookmarkManager.com")
        QCoreApplication.setApplicationName("QGIS_TestQgsBookmarkManager")
        QgsSettings().clear()
        QLocale.setDefault(QLocale(QLocale.English))
        start_app()

    def setUp(self):
        """Run before each test."""
        self.manager = None
        self.aboutFired = False

    def tearDown(self):
        """Run after each test."""
        pass

    def testBookmark(self):
        b = QgsBookmark()
        self.assertFalse(b.id())
        self.assertFalse(b.name())
        b.setId('id')
        self.assertEqual(b.id(), 'id')
        b.setName('name')
        self.assertEqual(b.name(), 'name')
        b.setGroup('group')
        self.assertEqual(b.group(), 'group')
        b.setExtent(QgsReferencedRectangle(QgsRectangle(1, 2, 3, 4), QgsCoordinateReferenceSystem('EPSG:3111')))
        self.assertEqual(b.extent(), QgsReferencedRectangle(QgsRectangle(1, 2, 3, 4), QgsCoordinateReferenceSystem('EPSG:3111')))

    def testBookmarkEquality(self):
        b = QgsBookmark()
        b.setId('id')
        b.setName('name')
        b.setGroup('group')
        b.setExtent(QgsReferencedRectangle(QgsRectangle(1, 2, 3, 4), QgsCoordinateReferenceSystem('EPSG:3111')))
        b2 = QgsBookmark()
        b2.setId('id')
        b2.setName('name')
        b2.setGroup('group')
        b2.setExtent(QgsReferencedRectangle(QgsRectangle(1, 2, 3, 4), QgsCoordinateReferenceSystem('EPSG:3111')))
        self.assertEqual(b, b2)
        b2.setId('x')
        self.assertNotEqual(b, b2)
        b2.setId('id')
        self.assertEqual(b, b2)
        b2.setName('x')
        self.assertNotEqual(b, b2)
        b2.setName('name')
        self.assertEqual(b, b2)
        b2.setGroup('x')
        self.assertNotEqual(b, b2)
        b2.setGroup('group')
        self.assertEqual(b, b2)
        b2.setExtent(QgsReferencedRectangle(QgsRectangle(1, 2, 3, 5), QgsCoordinateReferenceSystem('EPSG:3111')))
        self.assertNotEqual(b, b2)
        b2.setExtent(QgsReferencedRectangle(QgsRectangle(1, 2, 3, 4), QgsCoordinateReferenceSystem('EPSG:4326')))
        self.assertNotEqual(b, b2)

    def testAddBookmark(self):
        project = QgsProject()
        b = QgsBookmark()
        b.setName('b1')
        b.setExtent(QgsReferencedRectangle(QgsRectangle(1, 2, 3, 4), QgsCoordinateReferenceSystem('EPSG:3111')))

        manager = QgsBookmarkManager.createProjectBasedManager(project)

        bookmark_about_to_be_added_spy = QSignalSpy(manager.bookmarkAboutToBeAdded)
        bookmark_added_spy = QSignalSpy(manager.bookmarkAdded)
        id, res = manager.addBookmark(b)
        self.assertTrue(res)
        self.assertEqual(len(bookmark_about_to_be_added_spy), 1)
        self.assertEqual(bookmark_about_to_be_added_spy[0][0], id)
        self.assertEqual(len(bookmark_added_spy), 1)
        self.assertEqual(bookmark_added_spy[0][0], id)

        b = manager.bookmarkById(id)
        self.assertEqual(b.name(), 'b1')
        self.assertEqual(b.extent(), QgsReferencedRectangle(QgsRectangle(1, 2, 3, 4), QgsCoordinateReferenceSystem('EPSG:3111')))

        # adding it again should fail
        id, res = manager.addBookmark(b)
        self.assertFalse(res)

        # try adding a second bookmark
        b2 = QgsBookmark()
        b2.setId('my id')
        b2.setName('b2')
        b2.setExtent(QgsReferencedRectangle(QgsRectangle(11, 21, 31, 41), QgsCoordinateReferenceSystem('EPSG:4326')))

        id2, res = manager.addBookmark(b2)
        self.assertTrue(res)
        self.assertEqual(id2, 'my id')
        self.assertEqual(len(bookmark_added_spy), 2)
        self.assertEqual(bookmark_about_to_be_added_spy[1][0], 'my id')
        self.assertEqual(len(bookmark_about_to_be_added_spy), 2)
        self.assertEqual(bookmark_added_spy[1][0], 'my id')

        # adding a bookmark with duplicate id should fail
        b3 = QgsBookmark()
        b3.setId('my id')

        id, res = manager.addBookmark(b3)
        self.assertFalse(res)

    def testBookmarks(self):
        project = QgsProject()
        manager = QgsBookmarkManager.createProjectBasedManager(project)

        b = QgsBookmark()
        b.setId('1')
        b.setName('b1')
        b.setExtent(QgsReferencedRectangle(QgsRectangle(11, 21, 31, 41), QgsCoordinateReferenceSystem('EPSG:4326')))

        b2 = QgsBookmark()
        b2.setId('2')
        b2.setName('b2')
        b2.setExtent(QgsReferencedRectangle(QgsRectangle(12, 22, 32, 42), QgsCoordinateReferenceSystem('EPSG:4326')))

        b3 = QgsBookmark()
        b3.setId('3')
        b3.setName('b3')
        b3.setExtent(QgsReferencedRectangle(QgsRectangle(32, 32, 33, 43), QgsCoordinateReferenceSystem('EPSG:4326')))

        manager.addBookmark(b)
        self.assertEqual(manager.bookmarks(), [b])
        manager.addBookmark(b2)
        self.assertEqual(manager.bookmarks(), [b, b2])
        manager.addBookmark(b3)
        self.assertEqual(manager.bookmarks(), [b, b2, b3])

    def testBookmarkGroups(self):
        project = QgsProject()
        manager = QgsBookmarkManager.createProjectBasedManager(project)

        b = QgsBookmark()
        b.setId('1')
        b.setName('b1')
        manager.addBookmark(b)
        b2 = QgsBookmark()
        b2.setId('2')
        b2.setName('b2')
        b2.setGroup('group1')
        manager.addBookmark(b2)
        b3 = QgsBookmark()
        b3.setId('3')
        b3.setName('b3')
        b3.setGroup('group2')
        manager.addBookmark(b3)

        # test that groups are adjusted when bookmarks are added
        self.assertEqual(manager.groups(), ['', 'group1', 'group2'])

        manager.removeBookmark('3')

        # test that groups are adjusted when a bookmark is removed
        self.assertEqual(manager.groups(), ['', 'group1'])

        b2.setGroup('groupmodified')
        manager.updateBookmark(b2)

        # test that groups are adjusted when a bookmark group is edited
        self.assertEqual(manager.groups(), ['', 'groupmodified'])

    def bookmarkAboutToBeRemoved(self, id):
        # bookmark should still exist at this time
        self.assertEqual(id, '1')
        self.assertTrue(self.manager.bookmarkById('1').name())
        self.aboutFired = True

    def testRemoveBookmark(self):
        project = QgsProject()

        b = QgsBookmark()
        b.setId('1')
        b.setName('b1')
        b.setExtent(QgsReferencedRectangle(QgsRectangle(11, 21, 31, 41), QgsCoordinateReferenceSystem('EPSG:4326')))

        self.manager = QgsBookmarkManager.createProjectBasedManager(project)
        bookmark_removed_spy = QSignalSpy(self.manager.bookmarkRemoved)
        bookmark_about_to_be_removed_spy = QSignalSpy(self.manager.bookmarkAboutToBeRemoved)
        # tests that bookmark still exists when bookmarkAboutToBeRemoved is fired
        self.manager.bookmarkAboutToBeRemoved.connect(self.bookmarkAboutToBeRemoved)

        # not added, should fail
        self.assertFalse(self.manager.removeBookmark(b.id()))
        self.assertEqual(len(bookmark_removed_spy), 0)
        self.assertEqual(len(bookmark_about_to_be_removed_spy), 0)

        self.assertTrue(self.manager.addBookmark(b)[1])
        self.assertEqual(self.manager.bookmarks(), [b])
        self.assertTrue(self.manager.removeBookmark(b.id()))
        self.assertEqual(len(self.manager.bookmarks()), 0)
        self.assertEqual(len(bookmark_removed_spy), 1)
        self.assertEqual(bookmark_removed_spy[0][0], '1')
        self.assertEqual(len(bookmark_about_to_be_removed_spy), 1)
        self.assertEqual(bookmark_about_to_be_removed_spy[0][0], '1')
        self.assertTrue(self.aboutFired)
        self.manager = None

    def testClear(self):
        project = QgsProject()
        manager = QgsBookmarkManager.createProjectBasedManager(project)

        # add a bunch of bookmarks
        b = QgsBookmark()
        b.setId('1')
        b.setName('b1')
        b.setExtent(QgsReferencedRectangle(QgsRectangle(11, 21, 31, 41), QgsCoordinateReferenceSystem('EPSG:4326')))

        b2 = QgsBookmark()
        b2.setId('2')
        b2.setName('b2')
        b2.setExtent(QgsReferencedRectangle(QgsRectangle(12, 22, 32, 42), QgsCoordinateReferenceSystem('EPSG:4326')))

        b3 = QgsBookmark()
        b3.setId('3')
        b3.setName('b3')
        b3.setExtent(QgsReferencedRectangle(QgsRectangle(32, 32, 33, 43), QgsCoordinateReferenceSystem('EPSG:4326')))

        manager.addBookmark(b)
        manager.addBookmark(b2)
        manager.addBookmark(b3)

        bookmark_removed_spy = QSignalSpy(manager.bookmarkRemoved)
        bookmark_about_to_be_removed_spy = QSignalSpy(manager.bookmarkAboutToBeRemoved)
        manager.clear()
        self.assertEqual(len(manager.bookmarks()), 0)
        self.assertEqual(len(bookmark_removed_spy), 3)
        self.assertEqual(len(bookmark_about_to_be_removed_spy), 3)

    def testBookmarksById(self):
        project = QgsProject()
        manager = QgsBookmarkManager.createProjectBasedManager(project)

        # add a bunch of bookmarks
        b = QgsBookmark()
        b.setId('1')
        b.setName('b1')
        b.setExtent(QgsReferencedRectangle(QgsRectangle(11, 21, 31, 41), QgsCoordinateReferenceSystem('EPSG:4326')))

        b2 = QgsBookmark()
        b2.setId('2')
        b2.setName('b2')
        b2.setExtent(QgsReferencedRectangle(QgsRectangle(12, 22, 32, 42), QgsCoordinateReferenceSystem('EPSG:4326')))

        b3 = QgsBookmark()
        b3.setId('3')
        b3.setName('b3')
        b3.setExtent(QgsReferencedRectangle(QgsRectangle(32, 32, 33, 43), QgsCoordinateReferenceSystem('EPSG:4326')))

        manager.addBookmark(b)
        manager.addBookmark(b2)
        manager.addBookmark(b3)

        self.assertFalse(manager.bookmarkById('asdf').name())
        self.assertEqual(manager.bookmarkById('1'), b)
        self.assertEqual(manager.bookmarkById('2'), b2)
        self.assertEqual(manager.bookmarkById('3'), b3)

    def testReadWriteXml(self):
        """
        Test reading and writing bookmark manager state to XML
        """
        project = QgsProject()
        manager = QgsBookmarkManager.createProjectBasedManager(project)

        # add a bunch of bookmarks
        b = QgsBookmark()
        b.setId('1')
        b.setName('b1')
        b.setExtent(QgsReferencedRectangle(QgsRectangle(11, 21, 31, 41), QgsCoordinateReferenceSystem('EPSG:4326')))

        b2 = QgsBookmark()
        b2.setId('2')
        b2.setName('b2')
        b2.setExtent(QgsReferencedRectangle(QgsRectangle(12, 22, 32, 42), QgsCoordinateReferenceSystem('EPSG:4326')))

        b3 = QgsBookmark()
        b3.setId('3')
        b3.setName('b3')
        b3.setExtent(QgsReferencedRectangle(QgsRectangle(32, 32, 33, 43), QgsCoordinateReferenceSystem('EPSG:4326')))

        manager.addBookmark(b)
        manager.addBookmark(b2)
        manager.addBookmark(b3)

        # save to xml
        doc = QDomDocument("testdoc")
        elem = manager.writeXml(doc)
        doc.appendChild(elem)

        # restore from xml
        project2 = QgsProject()
        manager2 = QgsBookmarkManager.createProjectBasedManager(project2)
        self.assertTrue(manager2.readXml(elem, doc))

        self.assertEqual(len(manager2.bookmarks()), 3)
        names = [c.name() for c in manager2.bookmarks()]
        self.assertCountEqual(names, ['b1', 'b2', 'b3'])

    def testUpdateBookmark(self):
        project = QgsProject()
        manager = QgsBookmarkManager.createProjectBasedManager(project)
        changed_spy = QSignalSpy(manager.bookmarkChanged)

        b = QgsBookmark()
        b.setId('1')
        b.setName('b1')
        b.setExtent(QgsReferencedRectangle(QgsRectangle(11, 21, 31, 41), QgsCoordinateReferenceSystem('EPSG:4326')))

        self.assertFalse(manager.updateBookmark(b))
        self.assertEqual(len(changed_spy), 0)
        manager.addBookmark(b)

        b.setName('new b1')
        self.assertTrue(manager.updateBookmark(b))
        self.assertEqual(manager.bookmarkById('1').name(), 'new b1')
        self.assertEqual(len(changed_spy), 1)
        self.assertEqual(changed_spy[-1][0], '1')

        b2 = QgsBookmark()
        b2.setId('2')
        b2.setName('b2')
        b2.setExtent(QgsReferencedRectangle(QgsRectangle(12, 22, 32, 42), QgsCoordinateReferenceSystem('EPSG:4326')))
        manager.addBookmark(b2)

        b.setName('new b1 2')
        b2.setName('new b2 2')
        self.assertTrue(manager.updateBookmark(b))
        self.assertEqual(manager.bookmarkById('1').name(), 'new b1 2')
        self.assertEqual(manager.bookmarkById('2').name(), 'b2')
        self.assertEqual(len(changed_spy), 2)
        self.assertEqual(changed_spy[-1][0], '1')
        self.assertTrue(manager.updateBookmark(b2))
        self.assertEqual(manager.bookmarkById('1').name(), 'new b1 2')
        self.assertEqual(manager.bookmarkById('2').name(), 'new b2 2')
        self.assertEqual(len(changed_spy), 3)
        self.assertEqual(changed_spy[-1][0], '2')

    def testOldBookmarks(self):
        """
        Test upgrading older bookmark storage format
        """
        project_path = os.path.join(TEST_DATA_DIR, 'projects', 'old_bookmarks.qgs')
        p = QgsProject()
        self.assertTrue(p.read(project_path))
        self.assertEqual(len(p.bookmarkManager().bookmarks()), 3)
        self.assertEqual(p.bookmarkManager().bookmarkById('bookmark_0').name(), 'b1')
        self.assertEqual(p.bookmarkManager().bookmarkById('bookmark_0').extent().crs().authid(), 'EPSG:4283')
        self.assertEqual(p.bookmarkManager().bookmarkById('bookmark_0').extent().toString(1), '150.0,-23.0 : 150.6,-22.0')

        self.assertEqual(p.bookmarkManager().bookmarkById('bookmark_1').name(), 'b2')
        self.assertEqual(p.bookmarkManager().bookmarkById('bookmark_1').extent().crs().authid(), 'EPSG:4283')
        self.assertEqual(p.bookmarkManager().bookmarkById('bookmark_1').extent().toString(1), '149.0,-21.6 : 149.4,-21.1')

        self.assertEqual(p.bookmarkManager().bookmarkById('bookmark_2').name(), 'b3')
        self.assertEqual(p.bookmarkManager().bookmarkById('bookmark_2').extent().crs().authid(), 'EPSG:28355')
        self.assertEqual(p.bookmarkManager().bookmarkById('bookmark_2').extent().toString(1), '807985.7,7450916.9 : 876080.0,7564407.4')

    def testFileStorage(self):
        """
        Test file bound manager
        """
        manager = QgsBookmarkManager()

        tmpDir = QTemporaryDir()
        tmpFile = "{}/bookmarks.xml".format(tmpDir.path())

        manager.initialize(tmpFile)

        # add a bunch of bookmarks
        b = QgsBookmark()
        b.setId('1')
        b.setName('b1')
        b.setExtent(QgsReferencedRectangle(QgsRectangle(11, 21, 31, 41), QgsCoordinateReferenceSystem('EPSG:4326')))

        b2 = QgsBookmark()
        b2.setId('2')
        b2.setName('b2')
        b2.setExtent(QgsReferencedRectangle(QgsRectangle(12, 22, 32, 42), QgsCoordinateReferenceSystem('EPSG:4326')))

        b3 = QgsBookmark()
        b3.setId('3')
        b3.setName('b3')
        b3.setExtent(QgsReferencedRectangle(QgsRectangle(32, 32, 33, 43), QgsCoordinateReferenceSystem('EPSG:4326')))

        manager.addBookmark(b)
        manager.addBookmark(b2)
        manager.addBookmark(b3)

        # destroy manager, causes write to disk
        manager.deleteLater()
        QCoreApplication.sendPostedEvents(None, QEvent.DeferredDelete)
        del manager

        # create another new manager with same key, should contain existing bookmarks
        manager2 = QgsBookmarkManager()
        self.assertFalse(manager2.bookmarks())
        manager2.initialize(tmpFile)
        self.assertEqual(manager2.bookmarks(), [b, b2, b3])

        # but a manager with a different key should not...
        tmpFile2 = "{}/bookmarks2.xml".format(tmpDir.path())
        manager3 = QgsBookmarkManager()
        manager3.initialize(tmpFile2)
        self.assertEqual(manager3.bookmarks(), [])

    def testApplicationInstance(self):
        """
        Test storage in the application instance
        """
        manager = QgsApplication.bookmarkManager()

        # add a bunch of bookmarks
        b = QgsBookmark()
        b.setId('1')
        b.setName('b1')
        b.setExtent(QgsReferencedRectangle(QgsRectangle(11, 21, 31, 41), QgsCoordinateReferenceSystem('EPSG:4326')))

        b2 = QgsBookmark()
        b2.setId('2')
        b2.setName('b2')
        b2.setExtent(QgsReferencedRectangle(QgsRectangle(12, 22, 32, 42), QgsCoordinateReferenceSystem('EPSG:4326')))

        b3 = QgsBookmark()
        b3.setId('3')
        b3.setName('b3')
        b3.setExtent(QgsReferencedRectangle(QgsRectangle(32, 32, 33, 43), QgsCoordinateReferenceSystem('EPSG:4326')))

        manager.addBookmark(b)
        manager.addBookmark(b2)
        manager.addBookmark(b3)

        manager2 = QgsApplication.bookmarkManager()
        self.assertEqual(manager2.bookmarks(), [b, b2, b3])

        manager3 = QgsBookmarkManager(QgsProject.instance())
        self.assertEqual(manager3.bookmarks(), [])

    def testMoveBookmark(self):
        """
        Test moving a bookmark from one manager to another
        """
        p = QgsProject()
        manager = QgsBookmarkManager(p)
        manager2 = QgsBookmarkManager(p)

        # add a bunch of bookmarks
        b = QgsBookmark()
        b.setId('1')
        b.setName('b1')
        b.setExtent(QgsReferencedRectangle(QgsRectangle(11, 21, 31, 41), QgsCoordinateReferenceSystem('EPSG:4326')))

        b2 = QgsBookmark()
        b2.setId('2')
        b2.setName('b2')
        b2.setExtent(QgsReferencedRectangle(QgsRectangle(12, 22, 32, 42), QgsCoordinateReferenceSystem('EPSG:4326')))

        b3 = QgsBookmark()
        b3.setId('3')
        b3.setName('b3')
        b3.setExtent(QgsReferencedRectangle(QgsRectangle(32, 32, 33, 43), QgsCoordinateReferenceSystem('EPSG:4326')))

        manager.addBookmark(b)
        manager.addBookmark(b2)
        manager2.addBookmark(b3)

        self.assertEqual(manager.bookmarks(), [b, b2])
        self.assertEqual(manager2.bookmarks(), [b3])

        self.assertFalse(manager.moveBookmark('bbbb', manager2))
        self.assertFalse(manager.moveBookmark(b3.id(), manager2))
        self.assertEqual(manager.bookmarks(), [b, b2])
        self.assertEqual(manager2.bookmarks(), [b3])
        self.assertTrue(manager.moveBookmark(b.id(), manager2))
        self.assertEqual(manager.bookmarks(), [b2])
        self.assertEqual(manager2.bookmarks(), [b3, b])
        self.assertFalse(manager.moveBookmark(b.id(), manager2))
        self.assertTrue(manager2.moveBookmark(b3.id(), manager))
        self.assertEqual(manager.bookmarks(), [b2, b3])
        self.assertEqual(manager2.bookmarks(), [b])

    def testExportImport(self):
        p = QgsProject()
        manager = QgsBookmarkManager.createProjectBasedManager(p)
        manager2 = QgsBookmarkManager.createProjectBasedManager(p)
        manager3 = QgsBookmarkManager.createProjectBasedManager(p)

        tmpDir = QTemporaryDir()
        tmpFile = "{}/bookmarks.xml".format(tmpDir.path())

        # no managers
        self.assertTrue(QgsBookmarkManager.exportToFile(tmpFile, []))
        self.assertTrue(manager3.importFromFile(tmpFile))
        self.assertFalse(manager3.bookmarks())

        # no bookmarks
        self.assertTrue(QgsBookmarkManager.exportToFile(tmpFile, [manager]))
        self.assertTrue(manager3.importFromFile(tmpFile))
        self.assertFalse(manager3.bookmarks())

        # add a bunch of bookmarks
        b = QgsBookmark()
        b.setId('1')
        b.setName('b1')
        b.setGroup('g1')
        b.setExtent(QgsReferencedRectangle(QgsRectangle(11, 21, 31, 41), QgsCoordinateReferenceSystem('EPSG:4326')))

        b2 = QgsBookmark()
        b2.setId('2')
        b2.setName('b2')
        b2.setExtent(QgsReferencedRectangle(QgsRectangle(12, 22, 32, 42), QgsCoordinateReferenceSystem('EPSG:4326')))

        b3 = QgsBookmark()
        b3.setId('3')
        b3.setName('b3')
        b3.setGroup('g1')
        b3.setExtent(QgsReferencedRectangle(QgsRectangle(32, 32, 33, 43), QgsCoordinateReferenceSystem('EPSG:4326')))

        manager.addBookmark(b)
        manager.addBookmark(b2)
        manager2.addBookmark(b3)

        # export one manager's bookmarks
        self.assertTrue(QgsBookmarkManager.exportToFile(tmpFile, [manager]))
        self.assertTrue(manager3.importFromFile(tmpFile))
        self.assertEqual([(b.name(), b.extent()) for b in manager3.bookmarks()], [(b.name(), b.extent()) for b in [b, b2]])

        manager3.clear()
        # export both manager's bookmarks
        self.assertTrue(QgsBookmarkManager.exportToFile(tmpFile, [manager, manager2]))
        self.assertTrue(manager3.importFromFile(tmpFile))
        self.assertEqual([(b.name(), b.extent()) for b in manager3.bookmarks()], [(b.name(), b.extent()) for b in [b, b2, b3]])

        manager3.clear()
        # restrict to group
        self.assertTrue(QgsBookmarkManager.exportToFile(tmpFile, [manager, manager2], 'g1'))
        self.assertTrue(manager3.importFromFile(tmpFile))
        self.assertEqual([(b.name(), b.extent()) for b in manager3.bookmarks()], [(b.name(), b.extent()) for b in [b, b3]])

    def testRenameGroup(self):
        """
        Test renaming a bookmark group
        """
        p = QgsProject()
        manager = QgsBookmarkManager(p)

        # add a bunch of bookmarks
        b = QgsBookmark()
        b.setId('1')
        b.setName('b1')
        b.setGroup('g1')
        b.setExtent(QgsReferencedRectangle(QgsRectangle(11, 21, 31, 41), QgsCoordinateReferenceSystem('EPSG:4326')))

        b2 = QgsBookmark()
        b2.setId('2')
        b2.setName('b2')
        b2.setGroup('g1')
        b2.setExtent(QgsReferencedRectangle(QgsRectangle(12, 22, 32, 42), QgsCoordinateReferenceSystem('EPSG:4326')))

        b3 = QgsBookmark()
        b3.setId('3')
        b3.setName('b3')
        b3.setGroup('g3')
        b3.setExtent(QgsReferencedRectangle(QgsRectangle(32, 32, 33, 43), QgsCoordinateReferenceSystem('EPSG:4326')))

        manager.addBookmark(b)
        manager.addBookmark(b2)
        manager.addBookmark(b3)

        changed_spy = QSignalSpy(manager.bookmarkChanged)
        self.assertEqual([b.group() for b in manager.bookmarks()],
                         ['g1', 'g1', 'g3'])

        manager.renameGroup('xxxxx', 'yyyyy')
        self.assertEqual([b.group() for b in manager.bookmarks()],
                         ['g1', 'g1', 'g3'])
        self.assertEqual(len(changed_spy), 0)
        manager.renameGroup('', '')
        self.assertEqual([b.group() for b in manager.bookmarks()],
                         ['g1', 'g1', 'g3'])
        self.assertEqual(len(changed_spy), 0)
        manager.renameGroup('g1', 'g2')
        self.assertEqual([b.group() for b in manager.bookmarks()],
                         ['g2', 'g2', 'g3'])
        self.assertEqual(len(changed_spy), 2)
        self.assertEqual(changed_spy[0][0], '1')
        self.assertEqual(changed_spy[1][0], '2')
        manager.renameGroup('g3', 'g2')
        self.assertEqual([b.group() for b in manager.bookmarks()],
                         ['g2', 'g2', 'g2'])
        self.assertEqual(len(changed_spy), 3)
        self.assertEqual(changed_spy[2][0], '3')
        manager.renameGroup('g2', 'g')
        self.assertEqual([b.group() for b in manager.bookmarks()],
                         ['g', 'g', 'g'])
        self.assertEqual(len(changed_spy), 6)
        self.assertEqual(changed_spy[3][0], '1')
        self.assertEqual(changed_spy[4][0], '2')
        self.assertEqual(changed_spy[5][0], '3')


if __name__ == '__main__':
    unittest.main()
