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

import qgis

from qgis.core import QgsFontUtils
from utilities import (
    TestCase,
    getQgisTestApp,
    unittest,
    getTestFontFamily,
    loadTestFonts
)

QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()


class TestQgsFontUtils(TestCase):

    @classmethod
    def setUpClass(cls):
        cls._family = getTestFontFamily()
        cls._has_style = QgsFontUtils.fontFamilyHasStyle

    def test_loading_base_test_fonts(self):
        loadTestFonts()

    def test_loading_every_test_font(self):
        QgsFontUtils.loadStandardTestFonts(['All'])
        # styles = ''
        # for style in QFontDatabase().styles(self._family):
        #     styles += ' ' + style
        # print self._family + ' styles:' + styles

        res = (
            self._has_style(self._family, 'Roman')
            and self._has_style(self._family, 'Oblique')
            and self._has_style(self._family, 'Bold')
            and self._has_style(self._family, 'Bold Oblique')
        )
        msg = self._family + ' test font styles could not be loaded'
        assert res, msg

    def test_get_specific_test_font(self):
        # default returned is Roman at 12 pt
        f = QgsFontUtils.getStandardTestFont('Bold Oblique', 14)
        """:type: QFont"""
        res = (
            f.family() == self._family
            and f.bold()
            and f.italic()
            and f.pointSize() == 14
        )
        msg = self._family + ' test font Bold Oblique at 14 pt not retrieved'
        assert res, msg


if __name__ == '__main__':
    unittest.main()
