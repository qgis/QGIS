"""QGIS Unit tests for QgsBookmarkModel.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2019 by Nyall Dawson'
__date__ = '02/09/2019'
__copyright__ = 'Copyright 2019, The QGIS Project'

from qgis.PyQt.QtCore import QCoreApplication, QLocale, Qt
from qgis.core import (
    QgsBookmark,
    QgsBookmarkManager,
    QgsBookmarkManagerModel,
    QgsCoordinateReferenceSystem,
    QgsProject,
    QgsRectangle,
    QgsReferencedRectangle,
    QgsSettings,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsBookmarkManagerModel(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("QGIS_TestQgsBookmarkManager.com")
        QCoreApplication.setApplicationName("QGIS_TestQgsBookmarkManager")
        QgsSettings().clear()
        QLocale.setDefault(QLocale(QLocale.Language.English))
        start_app()

    def testBookmarkModel(self):
        p = QgsProject()
        project_manager = QgsBookmarkManager.createProjectBasedManager(p)
        app_manager = QgsBookmarkManager()

        # initially no bookmarks
        model = QgsBookmarkManagerModel(app_manager, project_manager)
        self.assertEqual(model.rowCount(), 0)
        self.assertEqual(model.columnCount(), 9)
        self.assertFalse(model.data(model.index(-1, 0)))
        self.assertFalse(model.data(model.index(1, 0)))
        self.assertFalse(model.data(model.index(0, 0)))
        self.assertFalse(model.data(model.index(0, 0), QgsBookmarkManagerModel.CustomRoles.RoleName))

        self.assertEqual(model.headerData(0, Qt.Orientation.Horizontal), 'Name')
        self.assertEqual(model.headerData(9, Qt.Orientation.Horizontal), 10)
        self.assertEqual(model.headerData(-1, Qt.Orientation.Horizontal), 0)

        self.assertFalse(model.setData(model.index(-1, 0), 4, Qt.ItemDataRole.EditRole))
        self.assertFalse(model.setData(model.index(0, 0), 4, Qt.ItemDataRole.EditRole))

        self.assertFalse(model.flags(model.index(0, 0)) & Qt.ItemFlag.ItemIsEnabled)
        self.assertFalse(model.flags(model.index(0, 0)) & Qt.ItemFlag.ItemIsEditable)

        # add some bookmarks
        b = QgsBookmark()
        b.setId('1')
        b.setGroup('group 1')
        b.setName('b1')
        b.setExtent(QgsReferencedRectangle(QgsRectangle(11, 21, 31, 41), QgsCoordinateReferenceSystem('EPSG:4326')))
        b.setRotation(0)

        b2 = QgsBookmark()
        b2.setId('2')
        b2.setGroup('group 2')
        b2.setName('b2')
        b2.setExtent(QgsReferencedRectangle(QgsRectangle(12, 22, 32, 42), QgsCoordinateReferenceSystem('EPSG:4326')))
        b2.setRotation(180.5)

        app_manager.addBookmark(b)
        app_manager.addBookmark(b2)

        self.assertEqual(model.rowCount(), 2)
        self.assertFalse(model.data(model.index(-1, 0)))
        self.assertEqual(model.data(model.index(0, 0)), 'b1')
        self.assertEqual(model.data(model.index(0, 1)), 'group 1')
        self.assertEqual(model.data(model.index(0, 2)), 11.0)
        self.assertEqual(model.data(model.index(0, 3)), 21.0)
        self.assertEqual(model.data(model.index(0, 4)), 31.0)
        self.assertEqual(model.data(model.index(0, 5)), 41.0)
        self.assertEqual(model.data(model.index(0, 6)), 0.0)
        self.assertEqual(model.data(model.index(0, 7)), 'EPSG:4326')
        self.assertEqual(model.data(model.index(0, 8)), None)
        self.assertEqual(model.data(model.index(0, 8), Qt.ItemDataRole.CheckStateRole), Qt.CheckState.Unchecked)
        self.assertEqual(model.data(model.index(0, 0), QgsBookmarkManagerModel.CustomRoles.RoleName), 'b1')
        self.assertEqual(model.data(model.index(0, 0), QgsBookmarkManagerModel.CustomRoles.RoleGroup), 'group 1')
        id = model.data(model.index(0, 0), QgsBookmarkManagerModel.CustomRoles.RoleId)
        self.assertEqual(app_manager.bookmarkById(id).name(), 'b1')
        self.assertEqual(model.data(model.index(0, 0), QgsBookmarkManagerModel.CustomRoles.RoleExtent), app_manager.bookmarkById(id).extent())

        self.assertEqual(model.data(model.index(1, 0)), 'b2')
        self.assertEqual(model.data(model.index(1, 1)), 'group 2')
        self.assertEqual(model.data(model.index(1, 2)), 12.0)
        self.assertEqual(model.data(model.index(1, 3)), 22.0)
        self.assertEqual(model.data(model.index(1, 4)), 32.0)
        self.assertEqual(model.data(model.index(1, 5)), 42.0)
        self.assertEqual(model.data(model.index(1, 6)), 180.5)
        self.assertEqual(model.data(model.index(1, 7)), 'EPSG:4326')
        self.assertEqual(model.data(model.index(1, 8)), None)
        self.assertEqual(model.data(model.index(1, 8), Qt.ItemDataRole.CheckStateRole), Qt.CheckState.Unchecked)
        self.assertEqual(model.data(model.index(1, 0), QgsBookmarkManagerModel.CustomRoles.RoleName), 'b2')
        self.assertEqual(model.data(model.index(1, 0), QgsBookmarkManagerModel.CustomRoles.RoleGroup), 'group 2')
        id = model.data(model.index(1, 0), QgsBookmarkManagerModel.CustomRoles.RoleId)
        self.assertEqual(app_manager.bookmarkById(id).name(), 'b2')
        self.assertEqual(model.data(model.index(1, 0), QgsBookmarkManagerModel.CustomRoles.RoleExtent), app_manager.bookmarkById(id).extent())
        self.assertFalse(model.data(model.index(2, 0)))

        self.assertFalse(model.setData(model.index(-1, 0), 4, Qt.ItemDataRole.EditRole))
        self.assertTrue(model.setData(model.index(0, 0), 'new name', Qt.ItemDataRole.EditRole))
        self.assertEqual(model.data(model.index(0, 0)), 'new name')
        self.assertEqual(app_manager.bookmarks()[0].name(), 'new name')
        self.assertTrue(model.setData(model.index(1, 1), 'new group', Qt.ItemDataRole.EditRole))
        self.assertEqual(model.data(model.index(1, 1)), 'new group')
        self.assertEqual(app_manager.bookmarks()[1].group(), 'new group')
        self.assertTrue(model.setData(model.index(0, 2), 1, Qt.ItemDataRole.EditRole))
        self.assertEqual(model.data(model.index(0, 2)), 1.0)
        self.assertEqual(app_manager.bookmarks()[0].extent().xMinimum(), 1.0)
        self.assertTrue(model.setData(model.index(0, 3), 2, Qt.ItemDataRole.EditRole))
        self.assertEqual(model.data(model.index(0, 3)), 2.0)
        self.assertEqual(app_manager.bookmarks()[0].extent().yMinimum(), 2.0)
        self.assertTrue(model.setData(model.index(0, 4), 3, Qt.ItemDataRole.EditRole))
        self.assertEqual(model.data(model.index(0, 4)), 3.0)
        self.assertEqual(app_manager.bookmarks()[0].extent().xMaximum(), 3.0)
        self.assertTrue(model.setData(model.index(0, 5), 4, Qt.ItemDataRole.EditRole))
        self.assertEqual(model.data(model.index(0, 5)), 4.0)
        self.assertEqual(app_manager.bookmarks()[0].extent().yMaximum(), 4.0)
        self.assertTrue(model.setData(model.index(0, 6), -1.2, Qt.ItemDataRole.EditRole))
        self.assertEqual(model.data(model.index(0, 6)), -1.2)
        self.assertEqual(app_manager.bookmarks()[0].rotation(), -1.2)
        self.assertFalse(model.setData(model.index(2, 0), 4, Qt.ItemDataRole.EditRole))

        self.assertTrue(model.flags(model.index(0, 0)) & Qt.ItemFlag.ItemIsEnabled)
        self.assertTrue(model.flags(model.index(0, 0)) & Qt.ItemFlag.ItemIsEditable)
        self.assertTrue(model.flags(model.index(0, 8)) & Qt.ItemFlag.ItemIsUserCheckable)
        self.assertTrue(model.flags(model.index(1, 8)) & Qt.ItemFlag.ItemIsUserCheckable)
        self.assertTrue(model.flags(model.index(1, 0)) & Qt.ItemFlag.ItemIsEnabled)
        self.assertTrue(model.flags(model.index(1, 0)) & Qt.ItemFlag.ItemIsEditable)
        self.assertFalse(model.flags(model.index(2, 0)) & Qt.ItemFlag.ItemIsEnabled)
        self.assertFalse(model.flags(model.index(2, 0)) & Qt.ItemFlag.ItemIsEditable)
        self.assertFalse(model.flags(model.index(2, 8)) & Qt.ItemFlag.ItemIsUserCheckable)

        # add bookmark to project manager
        b3 = QgsBookmark()
        b3.setId('3')
        b3.setName('b3')
        b3.setGroup('group 3')
        b3.setExtent(QgsReferencedRectangle(QgsRectangle(32, 32, 33, 43), QgsCoordinateReferenceSystem('EPSG:28355')))
        b3.setRotation(90)
        project_manager.addBookmark(b3)

        self.assertEqual(model.rowCount(), 3)
        self.assertFalse(model.data(model.index(-1, 0)))
        self.assertEqual(model.data(model.index(0, 0)), 'new name')
        self.assertEqual(model.data(model.index(0, 1)), 'group 1')
        self.assertEqual(model.data(model.index(0, 2)), 1.0)
        self.assertEqual(model.data(model.index(0, 3)), 2.0)
        self.assertEqual(model.data(model.index(0, 4)), 3.0)
        self.assertEqual(model.data(model.index(0, 5)), 4.0)
        self.assertEqual(model.data(model.index(0, 6)), -1.2)
        self.assertEqual(model.data(model.index(0, 7)), 'EPSG:4326')
        self.assertEqual(model.data(model.index(0, 8)), None)
        self.assertEqual(model.data(model.index(0, 8), Qt.ItemDataRole.CheckStateRole), Qt.CheckState.Unchecked)
        self.assertEqual(model.data(model.index(1, 0)), 'b2')
        self.assertEqual(model.data(model.index(1, 1)), 'new group')
        self.assertEqual(model.data(model.index(1, 2)), 12.0)
        self.assertEqual(model.data(model.index(1, 3)), 22.0)
        self.assertEqual(model.data(model.index(1, 4)), 32.0)
        self.assertEqual(model.data(model.index(1, 5)), 42.0)
        self.assertEqual(model.data(model.index(1, 6)), 180.5)
        self.assertEqual(model.data(model.index(1, 7)), 'EPSG:4326')
        self.assertEqual(model.data(model.index(1, 8)), None)
        self.assertEqual(model.data(model.index(1, 8), Qt.ItemDataRole.CheckStateRole), Qt.CheckState.Unchecked)
        self.assertEqual(model.data(model.index(2, 0)), 'b3')
        self.assertEqual(model.data(model.index(2, 1)), 'group 3')
        self.assertEqual(model.data(model.index(2, 2)), 32.0)
        self.assertEqual(model.data(model.index(2, 3)), 32.0)
        self.assertEqual(model.data(model.index(2, 4)), 33.0)
        self.assertEqual(model.data(model.index(2, 5)), 43.0)
        self.assertEqual(model.data(model.index(2, 6)), 90.0)
        self.assertEqual(model.data(model.index(2, 7)), 'EPSG:28355')
        self.assertEqual(model.data(model.index(2, 8)), None)
        self.assertEqual(model.data(model.index(2, 8), Qt.ItemDataRole.CheckStateRole), Qt.CheckState.Checked)
        self.assertEqual(model.data(model.index(2, 0), QgsBookmarkManagerModel.CustomRoles.RoleName), 'b3')
        self.assertEqual(model.data(model.index(2, 0), QgsBookmarkManagerModel.CustomRoles.RoleGroup), 'group 3')
        id = model.data(model.index(2, 0), QgsBookmarkManagerModel.CustomRoles.RoleId)
        self.assertEqual(project_manager.bookmarkById(id).name(), 'b3')
        self.assertEqual(model.data(model.index(2, 0), QgsBookmarkManagerModel.CustomRoles.RoleExtent), project_manager.bookmarkById(id).extent())
        self.assertFalse(model.data(model.index(3, 0)))

        self.assertTrue(model.setData(model.index(2, 0), 'new name 2', Qt.ItemDataRole.EditRole))
        self.assertEqual(model.data(model.index(2, 0)), 'new name 2')
        self.assertEqual(project_manager.bookmarks()[0].name(), 'new name 2')
        self.assertTrue(model.setData(model.index(2, 1), 'new group', Qt.ItemDataRole.EditRole))
        self.assertEqual(model.data(model.index(2, 1)), 'new group')
        self.assertEqual(project_manager.bookmarks()[0].group(), 'new group')
        self.assertTrue(model.setData(model.index(2, 2), 1, Qt.ItemDataRole.EditRole))
        self.assertEqual(model.data(model.index(2, 2)), 1.0)
        self.assertEqual(project_manager.bookmarks()[0].extent().xMinimum(), 1.0)
        self.assertTrue(model.setData(model.index(2, 3), 2, Qt.ItemDataRole.EditRole))
        self.assertEqual(model.data(model.index(2, 3)), 2.0)
        self.assertEqual(project_manager.bookmarks()[0].extent().yMinimum(), 2.0)
        self.assertTrue(model.setData(model.index(2, 4), 3, Qt.ItemDataRole.EditRole))
        self.assertEqual(model.data(model.index(2, 4)), 3.0)
        self.assertEqual(project_manager.bookmarks()[0].extent().xMaximum(), 3.0)
        self.assertTrue(model.setData(model.index(2, 5), 4, Qt.ItemDataRole.EditRole))
        self.assertEqual(model.data(model.index(2, 5)), 4.0)
        self.assertEqual(project_manager.bookmarks()[0].extent().yMaximum(), 4.0)
        self.assertTrue(model.setData(model.index(2, 6), 361, Qt.ItemDataRole.EditRole))
        self.assertEqual(model.data(model.index(2, 6)), 361)
        self.assertEqual(project_manager.bookmarks()[0].rotation(), 361)
        self.assertFalse(model.setData(model.index(3, 0), 4, Qt.ItemDataRole.EditRole))

        self.assertTrue(model.flags(model.index(0, 0)) & Qt.ItemFlag.ItemIsEnabled)
        self.assertTrue(model.flags(model.index(0, 0)) & Qt.ItemFlag.ItemIsEditable)
        self.assertTrue(model.flags(model.index(0, 8)) & Qt.ItemFlag.ItemIsUserCheckable)
        self.assertTrue(model.flags(model.index(1, 8)) & Qt.ItemFlag.ItemIsUserCheckable)
        self.assertTrue(model.flags(model.index(1, 0)) & Qt.ItemFlag.ItemIsEnabled)
        self.assertTrue(model.flags(model.index(1, 0)) & Qt.ItemFlag.ItemIsEditable)
        self.assertTrue(model.flags(model.index(2, 0)) & Qt.ItemFlag.ItemIsEnabled)
        self.assertTrue(model.flags(model.index(2, 0)) & Qt.ItemFlag.ItemIsEditable)
        self.assertTrue(model.flags(model.index(2, 8)) & Qt.ItemFlag.ItemIsUserCheckable)
        self.assertFalse(model.flags(model.index(3, 0)) & Qt.ItemFlag.ItemIsEnabled)
        self.assertFalse(model.flags(model.index(3, 0)) & Qt.ItemFlag.ItemIsEditable)
        self.assertFalse(model.flags(model.index(3, 8)) & Qt.ItemFlag.ItemIsUserCheckable)

        # try transferring bookmark from app->project
        self.assertTrue(model.setData(model.index(1, 8), Qt.CheckState.Checked, Qt.ItemDataRole.CheckStateRole))
        self.assertEqual([b.name() for b in project_manager.bookmarks()], ['new name 2', 'b2'])
        self.assertEqual([b.name() for b in app_manager.bookmarks()], ['new name'])
        self.assertFalse(model.setData(model.index(1, 8), Qt.CheckState.Checked, Qt.ItemDataRole.CheckStateRole))

        # try transferring bookmark from project->app
        self.assertTrue(model.setData(model.index(1, 8), Qt.CheckState.Unchecked, Qt.ItemDataRole.CheckStateRole))
        self.assertEqual([b.name() for b in project_manager.bookmarks()], ['b2'])
        self.assertEqual([b.name() for b in app_manager.bookmarks()], ['new name', 'new name 2'])
        self.assertFalse(model.setData(model.index(1, 8), Qt.CheckState.Unchecked, Qt.ItemDataRole.CheckStateRole))

        # remove rows
        model.removeRows(0, 1)
        self.assertEqual([b.name() for b in project_manager.bookmarks()], ['b2'])
        self.assertEqual([b.name() for b in app_manager.bookmarks()], ['new name 2'])
        model.removeRows(0, 2)
        self.assertEqual([b.name() for b in project_manager.bookmarks()], [])
        self.assertEqual([b.name() for b in app_manager.bookmarks()], [])


if __name__ == '__main__':
    unittest.main()
