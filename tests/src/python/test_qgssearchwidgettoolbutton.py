# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsSearchWidgetToolButton.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '18/05/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis # switch sip api

from qgis.gui import QgsSearchWidgetToolButton, QgsSearchWidgetWrapper

from qgis.testing import (start_app,
                          unittest
                          )

start_app()


class TestQgsSearchWidgetToolButton(unittest.TestCase):

    def testAvailableFlags(self):
        """
        Test setting available flags
        """
        w = QgsSearchWidgetToolButton()
        w.setAvailableFlags(QgsSearchWidgetWrapper.EqualTo |
                            QgsSearchWidgetWrapper.NotEqualTo |
                            QgsSearchWidgetWrapper.CaseInsensitive)

        flags = w.availableFlags()
        self.assertTrue(flags & QgsSearchWidgetWrapper.EqualTo)
        self.assertTrue(flags & QgsSearchWidgetWrapper.NotEqualTo)
        self.assertTrue(flags & QgsSearchWidgetWrapper.CaseInsensitive)
        self.assertFalse(flags & QgsSearchWidgetWrapper.Between)

        # setting available flags should update active flags
        w.setActiveFlags(QgsSearchWidgetWrapper.NotEqualTo | QgsSearchWidgetWrapper.CaseInsensitive)
        w.setAvailableFlags(QgsSearchWidgetWrapper.EqualTo |
                            QgsSearchWidgetWrapper.NotEqualTo)
        flags = w.activeFlags()
        self.assertFalse(flags & QgsSearchWidgetWrapper.EqualTo)
        self.assertTrue(flags & QgsSearchWidgetWrapper.NotEqualTo)
        self.assertFalse(flags & QgsSearchWidgetWrapper.CaseInsensitive)

    def testActiveFlags(self):
        """
        Test setting/retrieving active flag logic
        """
        w = QgsSearchWidgetToolButton()
        w.setAvailableFlags(QgsSearchWidgetWrapper.EqualTo |
                            QgsSearchWidgetWrapper.NotEqualTo |
                            QgsSearchWidgetWrapper.CaseInsensitive)

        w.setActiveFlags(QgsSearchWidgetWrapper.EqualTo)
        flags = w.activeFlags()
        self.assertTrue(flags & QgsSearchWidgetWrapper.EqualTo)
        self.assertFalse(flags & QgsSearchWidgetWrapper.NotEqualTo)

        w.setActiveFlags(QgsSearchWidgetWrapper.EqualTo | QgsSearchWidgetWrapper.CaseInsensitive)
        flags = w.activeFlags()
        self.assertTrue(flags & QgsSearchWidgetWrapper.EqualTo)
        self.assertTrue(flags & QgsSearchWidgetWrapper.CaseInsensitive)

        # setting a non-available flag as active
        w.setAvailableFlags(QgsSearchWidgetWrapper.EqualTo |
                            QgsSearchWidgetWrapper.NotEqualTo)
        w.setActiveFlags(QgsSearchWidgetWrapper.EqualTo | QgsSearchWidgetWrapper.CaseInsensitive)
        flags = w.activeFlags()
        self.assertTrue(flags & QgsSearchWidgetWrapper.EqualTo)
        self.assertFalse(flags & QgsSearchWidgetWrapper.CaseInsensitive)

        # setting conflicting flags
        w.setActiveFlags(QgsSearchWidgetWrapper.EqualTo | QgsSearchWidgetWrapper.NotEqualTo)
        flags = w.activeFlags()
        self.assertTrue(flags & QgsSearchWidgetWrapper.EqualTo)
        self.assertFalse(flags & QgsSearchWidgetWrapper.NotEqualTo)

    def testToggleFlag(self):
        """ Test toggling flags """
        w = QgsSearchWidgetToolButton()
        w.setAvailableFlags(QgsSearchWidgetWrapper.EqualTo |
                            QgsSearchWidgetWrapper.NotEqualTo |
                            QgsSearchWidgetWrapper.CaseInsensitive)
        w.setActiveFlags(QgsSearchWidgetWrapper.EqualTo)
        # should set flag
        w.toggleFlag(QgsSearchWidgetWrapper.CaseInsensitive)
        flags = w.activeFlags()
        self.assertTrue(flags & QgsSearchWidgetWrapper.EqualTo)
        self.assertFalse(flags & QgsSearchWidgetWrapper.NotEqualTo)
        self.assertTrue(flags & QgsSearchWidgetWrapper.CaseInsensitive)
        # should clear flag
        w.toggleFlag(QgsSearchWidgetWrapper.CaseInsensitive)
        flags = w.activeFlags()
        self.assertTrue(flags & QgsSearchWidgetWrapper.EqualTo)
        self.assertFalse(flags & QgsSearchWidgetWrapper.NotEqualTo)
        self.assertFalse(flags & QgsSearchWidgetWrapper.CaseInsensitive)

        #toggling non-available flag should be ignored
        w.setAvailableFlags(QgsSearchWidgetWrapper.Between |
                            QgsSearchWidgetWrapper.NotEqualTo)
        w.setActiveFlags(QgsSearchWidgetWrapper.Between)
        # should be ignored
        w.toggleFlag(QgsSearchWidgetWrapper.CaseInsensitive)
        w.toggleFlag(QgsSearchWidgetWrapper.LessThan)
        flags = w.activeFlags()
        self.assertFalse(flags & QgsSearchWidgetWrapper.CaseInsensitive)
        self.assertFalse(flags & QgsSearchWidgetWrapper.LessThan)
        self.assertTrue(flags & QgsSearchWidgetWrapper.Between)

        # toggling exclusive flag should result in other exclusive flags being cleared
        w.setAvailableFlags(QgsSearchWidgetWrapper.Between |
                            QgsSearchWidgetWrapper.NotEqualTo |
                            QgsSearchWidgetWrapper.CaseInsensitive)
        w.setActiveFlags(QgsSearchWidgetWrapper.Between | QgsSearchWidgetWrapper.CaseInsensitive)
        w.toggleFlag(QgsSearchWidgetWrapper.Between)
        flags = w.activeFlags()
        self.assertTrue(flags & QgsSearchWidgetWrapper.CaseInsensitive)
        self.assertFalse(flags & QgsSearchWidgetWrapper.NotEqualTo)
        self.assertTrue(flags & QgsSearchWidgetWrapper.Between)
        w.toggleFlag(QgsSearchWidgetWrapper.NotEqualTo)
        flags = w.activeFlags()
        self.assertTrue(flags & QgsSearchWidgetWrapper.CaseInsensitive)
        self.assertTrue(flags & QgsSearchWidgetWrapper.NotEqualTo)
        self.assertFalse(flags & QgsSearchWidgetWrapper.Between)

    def testSetInactive(self):
        """ Test setting the search as inactive """
        w = QgsSearchWidgetToolButton()
        w.setAvailableFlags(QgsSearchWidgetWrapper.EqualTo |
                            QgsSearchWidgetWrapper.NotEqualTo |
                            QgsSearchWidgetWrapper.CaseInsensitive)
        w.setActiveFlags(QgsSearchWidgetWrapper.EqualTo |
                         QgsSearchWidgetWrapper.CaseInsensitive)
        self.assertTrue(w.isActive())
        w.setInactive()
        flags = w.activeFlags()
        self.assertFalse(flags & QgsSearchWidgetWrapper.EqualTo)
        self.assertTrue(flags & QgsSearchWidgetWrapper.CaseInsensitive)
        self.assertFalse(w.isActive())

    def testSetActive(self):
        """ Test setting the search as active should adopt default flags"""
        w = QgsSearchWidgetToolButton()
        w.setAvailableFlags(QgsSearchWidgetWrapper.Between |
                            QgsSearchWidgetWrapper.NotEqualTo |
                            QgsSearchWidgetWrapper.CaseInsensitive)
        w.setActiveFlags(QgsSearchWidgetWrapper.CaseInsensitive)
        w.setDefaultFlags(QgsSearchWidgetWrapper.NotEqualTo)
        self.assertFalse(w.isActive())
        w.setActive()
        flags = w.activeFlags()
        self.assertTrue(flags & QgsSearchWidgetWrapper.NotEqualTo)
        self.assertTrue(flags & QgsSearchWidgetWrapper.CaseInsensitive)
        self.assertTrue(w.isActive())

if __name__ == '__main__':
    unittest.main()
