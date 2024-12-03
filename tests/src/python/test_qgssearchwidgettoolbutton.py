"""QGIS Unit tests for QgsSearchWidgetToolButton.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "18/05/2016"
__copyright__ = "Copyright 2016, The QGIS Project"


from qgis.gui import QgsSearchWidgetToolButton, QgsSearchWidgetWrapper
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsSearchWidgetToolButton(QgisTestCase):

    def testAvailableFlags(self):
        """
        Test setting available flags
        """
        w = QgsSearchWidgetToolButton()
        w.setAvailableFlags(
            QgsSearchWidgetWrapper.FilterFlag.EqualTo
            | QgsSearchWidgetWrapper.FilterFlag.NotEqualTo
            | QgsSearchWidgetWrapper.FilterFlag.CaseInsensitive
        )

        flags = w.availableFlags()
        self.assertTrue(flags & QgsSearchWidgetWrapper.FilterFlag.EqualTo)
        self.assertTrue(flags & QgsSearchWidgetWrapper.FilterFlag.NotEqualTo)
        self.assertTrue(flags & QgsSearchWidgetWrapper.FilterFlag.CaseInsensitive)
        self.assertFalse(flags & QgsSearchWidgetWrapper.FilterFlag.Between)

        # setting available flags should update active flags
        w.setActiveFlags(
            QgsSearchWidgetWrapper.FilterFlag.NotEqualTo
            | QgsSearchWidgetWrapper.FilterFlag.CaseInsensitive
        )
        w.setAvailableFlags(
            QgsSearchWidgetWrapper.FilterFlag.EqualTo
            | QgsSearchWidgetWrapper.FilterFlag.NotEqualTo
        )
        flags = w.activeFlags()
        self.assertFalse(flags & QgsSearchWidgetWrapper.FilterFlag.EqualTo)
        self.assertTrue(flags & QgsSearchWidgetWrapper.FilterFlag.NotEqualTo)
        self.assertFalse(flags & QgsSearchWidgetWrapper.FilterFlag.CaseInsensitive)

    def testActiveFlags(self):
        """
        Test setting/retrieving active flag logic
        """
        w = QgsSearchWidgetToolButton()
        w.setAvailableFlags(
            QgsSearchWidgetWrapper.FilterFlag.EqualTo
            | QgsSearchWidgetWrapper.FilterFlag.NotEqualTo
            | QgsSearchWidgetWrapper.FilterFlag.CaseInsensitive
        )

        w.setActiveFlags(QgsSearchWidgetWrapper.FilterFlag.EqualTo)
        flags = w.activeFlags()
        self.assertTrue(flags & QgsSearchWidgetWrapper.FilterFlag.EqualTo)
        self.assertFalse(flags & QgsSearchWidgetWrapper.FilterFlag.NotEqualTo)

        w.setActiveFlags(
            QgsSearchWidgetWrapper.FilterFlag.EqualTo
            | QgsSearchWidgetWrapper.FilterFlag.CaseInsensitive
        )
        flags = w.activeFlags()
        self.assertTrue(flags & QgsSearchWidgetWrapper.FilterFlag.EqualTo)
        self.assertTrue(flags & QgsSearchWidgetWrapper.FilterFlag.CaseInsensitive)

        # setting a non-available flag as active
        w.setAvailableFlags(
            QgsSearchWidgetWrapper.FilterFlag.EqualTo
            | QgsSearchWidgetWrapper.FilterFlag.NotEqualTo
        )
        w.setActiveFlags(
            QgsSearchWidgetWrapper.FilterFlag.EqualTo
            | QgsSearchWidgetWrapper.FilterFlag.CaseInsensitive
        )
        flags = w.activeFlags()
        self.assertTrue(flags & QgsSearchWidgetWrapper.FilterFlag.EqualTo)
        self.assertFalse(flags & QgsSearchWidgetWrapper.FilterFlag.CaseInsensitive)

        # setting conflicting flags
        w.setActiveFlags(
            QgsSearchWidgetWrapper.FilterFlag.EqualTo
            | QgsSearchWidgetWrapper.FilterFlag.NotEqualTo
        )
        flags = w.activeFlags()
        self.assertTrue(flags & QgsSearchWidgetWrapper.FilterFlag.EqualTo)
        self.assertFalse(flags & QgsSearchWidgetWrapper.FilterFlag.NotEqualTo)

    def testToggleFlag(self):
        """Test toggling flags"""
        w = QgsSearchWidgetToolButton()
        w.setAvailableFlags(
            QgsSearchWidgetWrapper.FilterFlag.EqualTo
            | QgsSearchWidgetWrapper.FilterFlag.NotEqualTo
            | QgsSearchWidgetWrapper.FilterFlag.CaseInsensitive
        )
        w.setActiveFlags(QgsSearchWidgetWrapper.FilterFlag.EqualTo)
        # should set flag
        w.toggleFlag(QgsSearchWidgetWrapper.FilterFlag.CaseInsensitive)
        flags = w.activeFlags()
        self.assertTrue(flags & QgsSearchWidgetWrapper.FilterFlag.EqualTo)
        self.assertFalse(flags & QgsSearchWidgetWrapper.FilterFlag.NotEqualTo)
        self.assertTrue(flags & QgsSearchWidgetWrapper.FilterFlag.CaseInsensitive)
        # should clear flag
        w.toggleFlag(QgsSearchWidgetWrapper.FilterFlag.CaseInsensitive)
        flags = w.activeFlags()
        self.assertTrue(flags & QgsSearchWidgetWrapper.FilterFlag.EqualTo)
        self.assertFalse(flags & QgsSearchWidgetWrapper.FilterFlag.NotEqualTo)
        self.assertFalse(flags & QgsSearchWidgetWrapper.FilterFlag.CaseInsensitive)

        # toggling non-available flag should be ignored
        w.setAvailableFlags(
            QgsSearchWidgetWrapper.FilterFlag.Between
            | QgsSearchWidgetWrapper.FilterFlag.NotEqualTo
        )
        w.setActiveFlags(QgsSearchWidgetWrapper.FilterFlag.Between)
        # should be ignored
        w.toggleFlag(QgsSearchWidgetWrapper.FilterFlag.CaseInsensitive)
        w.toggleFlag(QgsSearchWidgetWrapper.FilterFlag.LessThan)
        flags = w.activeFlags()
        self.assertFalse(flags & QgsSearchWidgetWrapper.FilterFlag.CaseInsensitive)
        self.assertFalse(flags & QgsSearchWidgetWrapper.FilterFlag.LessThan)
        self.assertTrue(flags & QgsSearchWidgetWrapper.FilterFlag.Between)

        # toggling exclusive flag should result in other exclusive flags being cleared
        w.setAvailableFlags(
            QgsSearchWidgetWrapper.FilterFlag.Between
            | QgsSearchWidgetWrapper.FilterFlag.NotEqualTo
            | QgsSearchWidgetWrapper.FilterFlag.CaseInsensitive
        )
        w.setActiveFlags(
            QgsSearchWidgetWrapper.FilterFlag.Between
            | QgsSearchWidgetWrapper.FilterFlag.CaseInsensitive
        )
        w.toggleFlag(QgsSearchWidgetWrapper.FilterFlag.Between)
        flags = w.activeFlags()
        self.assertTrue(flags & QgsSearchWidgetWrapper.FilterFlag.CaseInsensitive)
        self.assertFalse(flags & QgsSearchWidgetWrapper.FilterFlag.NotEqualTo)
        self.assertTrue(flags & QgsSearchWidgetWrapper.FilterFlag.Between)
        w.toggleFlag(QgsSearchWidgetWrapper.FilterFlag.NotEqualTo)
        flags = w.activeFlags()
        self.assertTrue(flags & QgsSearchWidgetWrapper.FilterFlag.CaseInsensitive)
        self.assertTrue(flags & QgsSearchWidgetWrapper.FilterFlag.NotEqualTo)
        self.assertFalse(flags & QgsSearchWidgetWrapper.FilterFlag.Between)

    def testSetInactive(self):
        """Test setting the search as inactive"""
        w = QgsSearchWidgetToolButton()
        w.setAvailableFlags(
            QgsSearchWidgetWrapper.FilterFlag.EqualTo
            | QgsSearchWidgetWrapper.FilterFlag.NotEqualTo
            | QgsSearchWidgetWrapper.FilterFlag.CaseInsensitive
        )
        w.setActiveFlags(
            QgsSearchWidgetWrapper.FilterFlag.EqualTo
            | QgsSearchWidgetWrapper.FilterFlag.CaseInsensitive
        )
        self.assertTrue(w.isActive())
        w.setInactive()
        flags = w.activeFlags()
        self.assertFalse(flags & QgsSearchWidgetWrapper.FilterFlag.EqualTo)
        self.assertTrue(flags & QgsSearchWidgetWrapper.FilterFlag.CaseInsensitive)
        self.assertFalse(w.isActive())

    def testSetActive(self):
        """Test setting the search as active should adopt default flags"""
        w = QgsSearchWidgetToolButton()
        w.setAvailableFlags(
            QgsSearchWidgetWrapper.FilterFlag.Between
            | QgsSearchWidgetWrapper.FilterFlag.NotEqualTo
            | QgsSearchWidgetWrapper.FilterFlag.CaseInsensitive
        )
        w.setActiveFlags(QgsSearchWidgetWrapper.FilterFlag.CaseInsensitive)
        w.setDefaultFlags(QgsSearchWidgetWrapper.FilterFlag.NotEqualTo)
        self.assertFalse(w.isActive())
        w.setActive()
        flags = w.activeFlags()
        self.assertTrue(flags & QgsSearchWidgetWrapper.FilterFlag.NotEqualTo)
        self.assertTrue(flags & QgsSearchWidgetWrapper.FilterFlag.CaseInsensitive)
        self.assertTrue(w.isActive())


if __name__ == "__main__":
    unittest.main()
