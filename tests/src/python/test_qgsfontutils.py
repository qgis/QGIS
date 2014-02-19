# -*- coding: utf-8 -*-
"""QGIS Unit tests for core QgsFontUtils class

From build dir: ctest -R PyQgsFontUtils -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Larry Shaffer'
__date__ = '2014/02/19'
__copyright__ = 'Copyright 2014, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'


from PyQt4.QtGui import *

from qgis.core import (
    QgsFontUtils
)

from utilities import (
    TestCase,
    getQgisTestApp,
    unittest,
    expectedFailure,
    unitTestDataPath,
)

QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()


class TestQgsFontUtils(TestCase):

    @classmethod
    def setUpClass(cls):
        cls._family = 'QGIS Vera Sans'
        cls._fontdb = QFontDatabase()
        """:type : QFontDatabase"""

    def test_loading_specific_test_font(self):
        QgsFontUtils.loadStandardTestFonts(['Roman'])
        msg = self._family + ' Roman test font styles could not be loaded'
        assert self._has_style(self._family, 'Roman'), msg

    def test_loading_all_test_fonts(self):
        QgsFontUtils.loadStandardTestFonts(['All'])
        # styles = ''
        # for style in self._fontdb.styles(self._family):
        #     styles += ' ' + style
        # print self._family + ' styles:' + styles
        msg = self._family + ' test font styles could not be loaded'
        res = (self._has_style(self._family, 'Roman')
               and self._has_style(self._family, 'Oblique')
               and self._has_style(self._family, 'Bold')
               and self._has_style(self._family, 'Bold Oblique'))
        assert res, msg

    def _has_style(self, family, style):
        return (family in self._fontdb.families()
                and style in self._fontdb.styles(family))

if __name__ == '__main__':
    unittest.main()
