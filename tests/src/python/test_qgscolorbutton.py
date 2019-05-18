# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsColorButton.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '25/05/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'

import qgis  # NOQA

from qgis.gui import QgsColorButton
from qgis.core import QgsApplication, QgsProjectColorScheme
from qgis.testing import start_app, unittest
from qgis.PyQt.QtGui import QColor
from qgis.PyQt.QtTest import QSignalSpy

start_app()


class TestQgsColorButton(unittest.TestCase):

    def testClearingColors(self):
        """
        Test setting colors to transparent
        """

        # start with a valid color
        button = QgsColorButton()
        button.setAllowOpacity(True)
        button.setColor(QColor(255, 100, 200, 255))
        self.assertEqual(button.color(), QColor(255, 100, 200, 255))

        # now set to no color
        button.setToNoColor()
        # ensure that only the alpha channel has changed - not the other color components
        self.assertEqual(button.color(), QColor(255, 100, 200, 0))

    def testLinkProjectColor(self):
        """
        Test linking to a project color
        """
        project_scheme = [s for s in QgsApplication.colorSchemeRegistry().schemes() if isinstance(s, QgsProjectColorScheme)][0]
        project_scheme.setColors([[QColor(255, 0, 0), 'col1'], [QColor(0, 255, 0), 'col2']])
        button = QgsColorButton()
        spy = QSignalSpy(button.unlinked)
        button.setColor(QColor(0, 0, 255))
        self.assertFalse(button.linkedProjectColorName())

        button.linkToProjectColor('col1')
        self.assertEqual(button.linkedProjectColorName(), 'col1')
        self.assertEqual(button.color().name(), '#ff0000')
        self.assertEqual(len(spy), 0)

        button.unlink()
        self.assertFalse(button.linkedProjectColorName())
        self.assertEqual(button.color().name(), '#0000ff')
        self.assertEqual(len(spy), 1)

        button.linkToProjectColor('col2')
        self.assertEqual(button.linkedProjectColorName(), 'col2')
        self.assertEqual(button.color().name(), '#00ff00')
        self.assertEqual(len(spy), 1)

        project_scheme.setColors([[QColor(255, 0, 0), 'xcol1'], [QColor(0, 255, 0), 'xcol2']])
        # linked color no longer exists
        self.assertFalse(button.linkedProjectColorName())
        self.assertEqual(button.color().name(), '#0000ff')
        self.assertEqual(len(spy), 2)


if __name__ == '__main__':
    unittest.main()
