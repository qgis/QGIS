"""QGIS Unit tests for QgsTextFragment.

Run with: ctest -V -R QgsTextFragment

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '12/05/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import qgis  # NOQA

from qgis.core import (
    QgsTextFragment,
    QgsTextCharacterFormat,
    QgsStringUtils
)
from qgis.PyQt.QtGui import QColor
from qgis.testing import start_app, unittest

start_app()


class TestQgsTextFragment(unittest.TestCase):

    def testConstructors(self):
        # empty
        frag = QgsTextFragment()
        self.assertFalse(frag.text())

        fragment = QgsTextFragment('ludicrous gibs!')
        self.assertEqual(fragment.text(), 'ludicrous gibs!')

    def testSetText(self):
        fragment = QgsTextFragment()
        fragment.setText('ludicrous gibs!')
        self.assertEqual(fragment.text(), 'ludicrous gibs!')

    def testSetCharacterFormat(self):
        fragment = QgsTextFragment('a')

        self.assertFalse(fragment.characterFormat().textColor().isValid())
        format = QgsTextCharacterFormat()
        format.setTextColor(QColor(255, 0, 0))
        fragment.setCharacterFormat(format)
        self.assertTrue(fragment.characterFormat().textColor().isValid())
        self.assertEqual(fragment.characterFormat().textColor().name(), '#ff0000')

    def testCapitalize(self):
        fragment = QgsTextFragment('ludicrous gibs!')
        fragment.applyCapitalization(QgsStringUtils.TitleCase)
        self.assertEqual(fragment.text(), 'Ludicrous Gibs!')


if __name__ == '__main__':
    unittest.main()
