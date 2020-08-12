# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsFontButton.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '04/06/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'

import qgis  # NOQA

from qgis.core import QgsTextFormat
from qgis.gui import QgsFontButton, QgsMapCanvas
from qgis.testing import start_app, unittest
from qgis.PyQt.QtGui import QColor, QFont
from qgis.PyQt.QtTest import QSignalSpy
from utilities import getTestFont

start_app()


class TestQgsFontButton(unittest.TestCase):

    def testGettersSetters(self):
        button = QgsFontButton()
        canvas = QgsMapCanvas()

        button.setDialogTitle('test title')
        self.assertEqual(button.dialogTitle(), 'test title')

        button.setMapCanvas(canvas)
        self.assertEqual(button.mapCanvas(), canvas)

    def testSetGetFormat(self):
        button = QgsFontButton()

        s = QgsTextFormat()
        s.setFont(getTestFont())
        s.setNamedStyle('Italic')
        s.setSize(5)
        s.setColor(QColor(255, 0, 0))
        s.setOpacity(0.5)

        signal_spy = QSignalSpy(button.changed)
        button.setTextFormat(s)
        self.assertEqual(len(signal_spy), 1)

        r = button.textFormat()
        self.assertEqual(r.font().family(), 'QGIS Vera Sans')
        self.assertEqual(r.namedStyle(), 'Italic')
        self.assertEqual(r.size(), 5)
        self.assertEqual(r.color(), QColor(255, 0, 0))
        self.assertEqual(r.opacity(), 0.5)

    def testSetGetFont(self):
        button = QgsFontButton()
        button.setMode(QgsFontButton.ModeQFont)
        self.assertEqual(button.mode(), QgsFontButton.ModeQFont)

        s = getTestFont()
        s.setPointSize(16)

        signal_spy = QSignalSpy(button.changed)
        button.setCurrentFont(s)
        self.assertEqual(len(signal_spy), 1)

        r = button.currentFont()
        self.assertEqual(r.family(), 'QGIS Vera Sans')
        self.assertEqual(r.styleName(), 'Roman')
        self.assertEqual(r.pointSize(), 16)

    def testSetColor(self):
        button = QgsFontButton()

        s = QgsTextFormat()
        s.setFont(getTestFont())
        s.setNamedStyle('Italic')
        s.setSize(5)
        s.setColor(QColor(255, 0, 0))
        s.setOpacity(0.5)
        button.setTextFormat(s)

        signal_spy = QSignalSpy(button.changed)
        button.setColor(QColor(0, 255, 0))
        self.assertEqual(len(signal_spy), 1)

        r = button.textFormat()
        self.assertEqual(r.font().family(), 'QGIS Vera Sans')
        self.assertEqual(r.namedStyle(), 'Italic')
        self.assertEqual(r.size(), 5)
        self.assertEqual(r.color().name(), QColor(0, 255, 0).name())
        self.assertEqual(r.opacity(), 0.5)

        # set same color, should not emit signal
        button.setColor(QColor(0, 255, 0))
        self.assertEqual(len(signal_spy), 1)

        # color with transparency - should be stripped
        button.setColor(QColor(0, 255, 0, 100))
        r = button.textFormat()
        self.assertEqual(r.color(), QColor(0, 255, 0))

    def testNull(self):
        button = QgsFontButton()
        self.assertFalse(button.showNullFormat())
        button.setShowNullFormat(True)
        self.assertTrue(button.showNullFormat())

        s = QgsTextFormat()
        s.setFont(getTestFont())
        button.setTextFormat(s)
        self.assertTrue(button.textFormat().isValid())
        button.setToNullFormat()
        self.assertFalse(button.textFormat().isValid())
        button.setTextFormat(s)
        self.assertTrue(button.textFormat().isValid())


if __name__ == '__main__':
    unittest.main()
