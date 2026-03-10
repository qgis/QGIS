"""QGIS Unit tests for QgsElevationProfileManager.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import unittest

from qgis.core import (
    QgsElevationProfile,
    QgsElevationProfileManager,
    QgsProject,
    QgsReadWriteContext,
)
from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtXml import QDomDocument
from qgis.testing import QgisTestCase, start_app
from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsElevationProfileManager(QgisTestCase):
    def setUp(self):
        """Run before each test."""
        self.manager = None
        self.aboutFired = False

    def tearDown(self):
        """Run after each test."""
        pass

    def testAddProfile(self):
        project = QgsProject()
        profile = QgsElevationProfile(project)
        profile.setName("test profile")

        manager = QgsElevationProfileManager(project)

        profile_about_to_be_added_spy = QSignalSpy(manager.profileAboutToBeAdded)
        profile_added_spy = QSignalSpy(manager.profileAdded)
        self.assertTrue(manager.addProfile(profile))
        self.assertEqual(len(profile_about_to_be_added_spy), 1)
        self.assertEqual(profile_about_to_be_added_spy[0][0], "test profile")
        self.assertEqual(len(profile_added_spy), 1)
        self.assertEqual(profile_added_spy[0][0], "test profile")

        # adding it again should fail
        self.assertFalse(manager.addProfile(profile))

        # try adding a second profile
        profile2 = QgsElevationProfile(project)
        profile2.setName("test profile2")
        self.assertTrue(manager.addProfile(profile2))
        self.assertEqual(len(profile_added_spy), 2)
        self.assertEqual(profile_about_to_be_added_spy[1][0], "test profile2")
        self.assertEqual(len(profile_about_to_be_added_spy), 2)
        self.assertEqual(profile_added_spy[1][0], "test profile2")

        # adding a profile with duplicate name should fail
        profile3 = QgsElevationProfile(project)
        profile3.setName("test profile2")
        self.assertFalse(manager.addProfile(profile3))

    def testProfiles(self):
        project = QgsProject()
        manager = QgsElevationProfileManager(project)
        profile = QgsElevationProfile(project)
        profile.setName("test profile")
        profile2 = QgsElevationProfile(project)
        profile2.setName("test profile2")
        profile3 = QgsElevationProfile(project)
        profile3.setName("test profile3")

        manager.addProfile(profile)
        self.assertEqual(manager.profiles(), [profile])
        manager.addProfile(profile2)
        self.assertEqual(set(manager.profiles()), {profile, profile2})
        manager.addProfile(profile3)
        self.assertEqual(set(manager.profiles()), {profile, profile2, profile3})

    def aboutToBeRemoved(self, name):
        # composition should still exist at this time
        self.assertEqual(name, "test composition")
        self.assertTrue(self.manager.compositionByName("test composition"))
        self.aboutFired = True

    def profileAboutToBeRemoved(self, name):
        # profile should still exist at this time
        self.assertEqual(name, "test profile")
        self.assertTrue(self.manager.profileByName("test profile"))
        self.aboutFired = True

    def testRemoveProfile(self):
        project = QgsProject()
        profile = QgsElevationProfile(project)
        profile.setName("test profile")

        self.manager = QgsElevationProfileManager(project)
        profile_removed_spy = QSignalSpy(self.manager.profileRemoved)
        profile_about_to_be_removed_spy = QSignalSpy(
            self.manager.profileAboutToBeRemoved
        )
        # tests that profile still exists when profileAboutToBeRemoved is fired
        self.manager.profileAboutToBeRemoved.connect(self.profileAboutToBeRemoved)

        # not added, should fail
        self.assertFalse(self.manager.removeProfile(profile))
        self.assertEqual(len(profile_removed_spy), 0)
        self.assertEqual(len(profile_about_to_be_removed_spy), 0)

        self.assertTrue(self.manager.addProfile(profile))
        self.assertEqual(self.manager.profiles(), [profile])
        self.assertTrue(self.manager.removeProfile(profile))
        self.assertEqual(len(self.manager.profiles()), 0)
        self.assertEqual(len(profile_removed_spy), 1)
        self.assertEqual(profile_removed_spy[0][0], "test profile")
        self.assertEqual(len(profile_about_to_be_removed_spy), 1)
        self.assertEqual(profile_about_to_be_removed_spy[0][0], "test profile")
        self.assertTrue(self.aboutFired)
        self.manager = None

    def testClear(self):
        project = QgsProject()
        manager = QgsElevationProfileManager(project)

        # add a bunch of profiles
        profile = QgsElevationProfile(project)
        profile.setName("test profile")
        profile2 = QgsElevationProfile(project)
        profile2.setName("test profile2")
        profile3 = QgsElevationProfile(project)
        profile3.setName("test profile3")

        manager.addProfile(profile)
        manager.addProfile(profile2)
        manager.addProfile(profile3)

        profile_removed_spy = QSignalSpy(manager.profileRemoved)
        profile_about_to_be_removed_spy = QSignalSpy(manager.profileAboutToBeRemoved)
        manager.clear()
        self.assertEqual(len(manager.profiles()), 0)
        self.assertEqual(len(profile_removed_spy), 3)
        self.assertEqual(len(profile_about_to_be_removed_spy), 3)

    def testProfilesByName(self):
        project = QgsProject()
        manager = QgsElevationProfileManager(project)

        # add a bunch of profiles
        profile = QgsElevationProfile(project)
        profile.setName("test profile")
        profile2 = QgsElevationProfile(project)
        profile2.setName("test profile2")
        profile3 = QgsElevationProfile(project)
        profile3.setName("test profile3")

        manager.addProfile(profile)
        manager.addProfile(profile2)
        manager.addProfile(profile3)

        self.assertFalse(manager.profileByName("asdf"))
        self.assertEqual(manager.profileByName("test profile"), profile)
        self.assertEqual(manager.profileByName("test profile2"), profile2)
        self.assertEqual(manager.profileByName("test profile3"), profile3)

    def testReadWriteXml(self):
        """
        Test reading and writing profile manager state to XML
        """
        project = QgsProject()
        manager = QgsElevationProfileManager(project)

        # add a bunch of profiles
        profile = QgsElevationProfile(project)
        profile.setName("test profile")
        profile2 = QgsElevationProfile(project)
        profile2.setName("test profile2")
        profile3 = QgsElevationProfile(project)
        profile3.setName("test profile3")

        manager.addProfile(profile)
        manager.addProfile(profile2)
        manager.addProfile(profile3)

        # save to xml
        context = QgsReadWriteContext()
        doc = QDomDocument("testdoc")
        elem = manager.writeXml(doc, context)
        doc.appendChild(elem)

        # restore from xml
        project2 = QgsProject()
        manager2 = QgsElevationProfileManager(project2)
        self.assertTrue(manager2.readXml(elem, doc, context))

        self.assertEqual(len(manager2.profiles()), 3)
        names = [c.name() for c in manager2.profiles()]
        self.assertCountEqual(names, ["test profile", "test profile2", "test profile3"])

    def testGenerateUniqueTitle(self):
        project = QgsProject()
        manager = QgsElevationProfileManager(project)
        self.assertEqual(
            manager.generateUniqueTitle(),
            "Elevation Profile 1",
        )

        profile = QgsElevationProfile(project)
        profile.setName(manager.generateUniqueTitle())
        manager.addProfile(profile)

        self.assertEqual(manager.generateUniqueTitle(), "Elevation Profile 2")
        profile2 = QgsElevationProfile(project)
        profile2.setName(manager.generateUniqueTitle())
        manager.addProfile(profile2)

        self.assertEqual(manager.generateUniqueTitle(), "Elevation Profile 3")

        manager.clear()
        self.assertEqual(manager.generateUniqueTitle(), "Elevation Profile 1")

    def testRenameSignal(self):
        project = QgsProject()
        manager = QgsElevationProfileManager(project)
        profile = QgsElevationProfile(project)
        profile.setName("c1")
        manager.addProfile(profile)
        profile2 = QgsElevationProfile(project)
        profile2.setName("c2")
        manager.addProfile(profile2)

        profile_renamed_spy = QSignalSpy(manager.profileRenamed)
        profile.setName("d1")
        self.assertEqual(len(profile_renamed_spy), 1)
        self.assertEqual(profile_renamed_spy[0][1], "d1")
        profile2.setName("d2")
        self.assertEqual(len(profile_renamed_spy), 2)
        self.assertEqual(profile_renamed_spy[1][1], "d2")


if __name__ == "__main__":
    unittest.main()
