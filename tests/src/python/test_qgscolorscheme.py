# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsColorScheme.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '25/07/2014'
__copyright__ = 'Copyright 2014, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis
from utilities import unittest, TestCase
from qgis.core import QgsColorScheme
from PyQt4.QtGui import QColor

#Make a dummy color scheme for testing

class DummyColorScheme(QgsColorScheme):
    def __init__(self, parent=None):
        QgsColorScheme.__init__(self)

    def schemeName(self):
        return "Dummy scheme"

    def fetchColors(self,context='', baseColor=QColor()):
        if ( context == "testscheme" ):
            return [[QColor(255, 255, 0),'schemetest']]
        elif baseColor.isValid():
            return [[baseColor,'base']]
        else:
            return [[QColor( 255, 0, 0 ),'red'],[QColor( 0, 255, 0 ), None]]

    def clone(self):
        return DummyColorScheme()


class TestQgsColorScheme(TestCase):

    def testCreateScheme(self):
        """Test creating a new color scheme"""
        dummyScheme = DummyColorScheme()
        self.assertTrue(dummyScheme)

    def testGetSchemeName(self):
        """Test getting color scheme name"""
        dummyScheme = DummyColorScheme()
        self.assertEqual(dummyScheme.schemeName(), "Dummy scheme")

    def testColorsNoBase(self):
        """Test getting colors without passing a base color"""
        dummyScheme = DummyColorScheme()
        colors = dummyScheme.fetchColors()
        self.assertEqual( len(colors), 2 )
        self.assertEqual( colors[0][0], QColor( 255, 0, 0 ) )
        self.assertEqual( colors[0][1], 'red')
        self.assertEqual( colors[1][0], QColor( 0, 255, 0 ) )
        self.assertEqual( colors[1][1], None)

    def testColorsWithBase(self):
        """Test getting colors with a base color"""
        dummyScheme = DummyColorScheme()
        testColor = QColor( 0, 0, 255 )
        colors = dummyScheme.fetchColors( None, testColor )
        self.assertEqual( len(colors), 1 )
        self.assertEqual( colors[0][0], testColor )
        self.assertEqual( colors[0][1], 'base')

    def testColorsWithScheme(self):
        """Test getting colors when specifying a scheme"""
        dummyScheme = DummyColorScheme()
        colors = dummyScheme.fetchColors( 'testscheme' )
        self.assertEqual( len(colors), 1 )
        self.assertEqual( colors[0][0], QColor( 255, 255, 0 ) )
        self.assertEqual( colors[0][1], 'schemetest')

    def testClone(self):
        """Test cloning a color scheme"""
        dummyScheme = DummyColorScheme()
        colors = dummyScheme.fetchColors()
        dummySchemeClone = dummyScheme.clone()
        colorsClone = dummySchemeClone.fetchColors()
        self.assertEqual( colors, colorsClone )


if __name__ == "__main__":
        unittest.main()
