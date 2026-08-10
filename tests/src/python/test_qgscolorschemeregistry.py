"""QGIS Unit tests for QgsColorSchemeRegistry.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "25/07/2014"
__copyright__ = "Copyright 2014, The QGIS Project"


import unittest

from qgis.core import (
    QgsApplication,
    QgsColorScheme,
    QgsColorSchemeRegistry,
    QgsProject,
    QgsProjectColorScheme,
    QgsRecentColorScheme,
)
from qgis.PyQt.QtGui import QColor
from qgis.testing import QgisTestCase, start_app

start_app()


class TestQgsColorSchemeRegistry(QgisTestCase):
    def testCreateInstance(self):
        """Test creating global color scheme registry instance"""
        registry = QgsApplication.colorSchemeRegistry()
        self.assertTrue(registry)

    def testInstanceHasDefaultScheme(self):
        """Test global color scheme registry has default schemes"""
        registry = QgsApplication.colorSchemeRegistry()
        self.assertGreater(len(registry.schemes()), 0)

    def testCreateEmpty(self):
        """Test creating an empty color scheme registry"""
        registry = QgsColorSchemeRegistry()
        self.assertEqual(len(registry.schemes()), 0)

    def testAddScheme(self):
        """Test adding a scheme to a registry"""
        registry = QgsColorSchemeRegistry()
        self.assertEqual(len(registry.schemes()), 0)
        recentScheme = QgsRecentColorScheme()
        registry.addColorScheme(recentScheme)
        self.assertEqual(len(registry.schemes()), 1)

    def testAddDefaultScheme(self):
        """Test adding default schemes to a registry"""
        registry = QgsColorSchemeRegistry()
        self.assertEqual(len(registry.schemes()), 0)
        registry.addDefaultSchemes()
        self.assertGreater(len(registry.schemes()), 0)

    def testPopulateFromInstance(self):
        """Test adding schemes from global instance"""
        registry = QgsColorSchemeRegistry()
        self.assertEqual(len(registry.schemes()), 0)
        registry.populateFromInstance()
        self.assertEqual(
            len(registry.schemes()), len(QgsApplication.colorSchemeRegistry().schemes())
        )

    def testRemoveScheme(self):
        """Test removing a scheme from a registry"""
        registry = QgsColorSchemeRegistry()
        self.assertEqual(len(registry.schemes()), 0)
        recentScheme = QgsRecentColorScheme()
        registry.addColorScheme(recentScheme)
        self.assertEqual(len(registry.schemes()), 1)
        # remove the scheme
        registry.removeColorScheme(recentScheme)
        self.assertEqual(len(registry.schemes()), 0)
        # try removing a scheme not in the registry
        self.assertFalse(registry.removeColorScheme(recentScheme))

    def testOwnership(self):
        """
        Test that registered color schemes do not require that a reference to them is kept.
        They should be parented to the registry (on transfer) and even if there's no reference
        to the registry around (see the `del` below) this childship should continue to exist.
        """

        class TestColorScheme(QgsColorScheme):
            def schemeName(self):
                return "TestScheme"

            def fetchColors(self, context, baseColors):
                return None

            def clone(self):
                return TestColorScheme()

            def flags(self):
                return 1

        reg = QgsApplication.instance().colorSchemeRegistry()
        reg.addColorScheme(TestColorScheme())
        del reg

        reg = QgsApplication.instance().colorSchemeRegistry()

        self.assertIn("TestScheme", [scheme.schemeName() for scheme in reg.schemes()])

    def testSetProject(self):
        """Test that the project colors scheme follows setProject()"""
        registry = QgsApplication.colorSchemeRegistry()
        # other tests rely on the registry being bound to QgsProject.instance(), so restore that after
        self.addCleanup(registry.setProject, QgsProject.instance())

        project1 = QgsProject()
        project1.setProjectColors([[QColor(255, 0, 0), "red"]])
        project2 = QgsProject()
        project2.setProjectColors([[QColor(0, 255, 0), "green"]])

        registry.setProject(project1)
        schemes = [
            s for s in registry.schemes() if isinstance(s, QgsProjectColorScheme)
        ]
        self.assertEqual(len(schemes), 1)
        self.assertEqual(
            [[c[0], c[1]] for c in schemes[0].fetchColors()],
            [[QColor(255, 0, 0), "red"]],
        )

        # switching to a second project should swap the scheme's colors, not add a second scheme
        registry.setProject(project2)
        schemes = [
            s for s in registry.schemes() if isinstance(s, QgsProjectColorScheme)
        ]
        self.assertEqual(len(schemes), 1)
        self.assertEqual(
            [[c[0], c[1]] for c in schemes[0].fetchColors()],
            [[QColor(0, 255, 0), "green"]],
        )

        # clearing the project should remove the project colors scheme entirely
        registry.setProject(None)
        self.assertFalse(
            [s for s in registry.schemes() if isinstance(s, QgsProjectColorScheme)]
        )


if __name__ == "__main__":
    unittest.main()
