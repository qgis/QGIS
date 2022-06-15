# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsFontManager

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '15/06/2022'
__copyright__ = 'Copyright 2022, The QGIS Project'

import qgis  # NOQA

from qgis.core import (
    QgsFontManager,
    QgsSettings
)
from qgis.PyQt.QtCore import QCoreApplication
from qgis.PyQt.QtTest import QSignalSpy

from qgis.testing import start_app, unittest
from utilities import unitTestDataPath


start_app()


class TestQgsFontManager(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("QGIS_TestQgsFontManager.com")
        QCoreApplication.setApplicationName("QGIS_TestQgsFontManager")
        QgsSettings().clear()
        start_app()

    def test_family_replacement(self):
        manager = QgsFontManager()
        self.assertFalse(manager.fontFamilyReplacements())

        manager.addFontFamilyReplacement('comic sans', 'something better')
        self.assertEqual(manager.fontFamilyReplacements(), {'comic sans': 'something better'})

        manager.addFontFamilyReplacement('arial', 'something else better')
        self.assertEqual(manager.fontFamilyReplacements(), {'arial': 'something else better', 'comic sans': 'something better'})

        manager.addFontFamilyReplacement('arial', 'comic sans')
        self.assertEqual(manager.fontFamilyReplacements(), {'arial': 'comic sans', 'comic sans': 'something better'})

        manager.addFontFamilyReplacement('arial', '')
        self.assertEqual(manager.fontFamilyReplacements(), {'comic sans': 'something better'})

        manager.setFontFamilyReplacements({'arial': 'something else better2', 'comic sans': 'something better2'})
        self.assertEqual(manager.fontFamilyReplacements(), {'arial': 'something else better2', 'comic sans': 'something better2'})


if __name__ == '__main__':
    unittest.main()
