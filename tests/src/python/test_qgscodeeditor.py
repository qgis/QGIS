# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsCodeEditor

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '03/10/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import qgis  # NOQA

from qgis.core import QgsSettings, QgsApplication
from qgis.gui import QgsCodeEditor

from qgis.PyQt.QtCore import QCoreApplication
from qgis.PyQt.QtGui import QColor
from qgis.testing import start_app, unittest

start_app()


class TestQgsCodeEditor(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("QGIS_TestPyQgsColorScheme.com")
        QCoreApplication.setApplicationName("QGIS_TestPyQgsColorScheme")
        QgsSettings().clear()
        start_app()

    def testDefaultColors(self):
        # default color theme, default application theme
        QgsApplication.setUITheme('default')
        self.assertEqual(QgsCodeEditor.defaultColor(QgsCodeEditor.ColorRole.Keyword).name(), '#8959a8')

        # default colors should respond to application ui theme
        QgsApplication.setUITheme('Night Mapping')
        self.assertEqual(QgsCodeEditor.defaultColor(QgsCodeEditor.ColorRole.Keyword).name(), '#6cbcf7')

        # explicit theme, should override ui theme defaults
        self.assertEqual(QgsCodeEditor.defaultColor(QgsCodeEditor.ColorRole.Keyword, 'solarized').name(), '#b79b00')
        self.assertEqual(QgsCodeEditor.defaultColor(QgsCodeEditor.ColorRole.Keyword, 'solarized_dark').name(), '#6cbcf7')

    def testColors(self):
        QgsApplication.setUITheme('default')
        self.assertEqual(QgsCodeEditor.color(QgsCodeEditor.ColorRole.Keyword).name(), '#8959a8')
        QgsApplication.setUITheme('Night Mapping')
        self.assertEqual(QgsCodeEditor.color(QgsCodeEditor.ColorRole.Keyword).name(), '#6cbcf7')

        QgsSettings().setValue('codeEditor/colorScheme', 'solarized', QgsSettings.Gui)
        self.assertEqual(QgsCodeEditor.color(QgsCodeEditor.ColorRole.Keyword).name(), '#b79b00')
        QgsSettings().setValue('codeEditor/colorScheme', 'solarized_dark', QgsSettings.Gui)
        self.assertEqual(QgsCodeEditor.color(QgsCodeEditor.ColorRole.Keyword).name(), '#6cbcf7')

        QgsSettings().setValue('codeEditor/overrideColors', True, QgsSettings.Gui)
        QgsCodeEditor.setColor(QgsCodeEditor.ColorRole.Keyword, QColor('#cc11bb'))
        self.assertEqual(QgsCodeEditor.color(QgsCodeEditor.ColorRole.Keyword).name(), '#cc11bb')


if __name__ == '__main__':
    unittest.main()
