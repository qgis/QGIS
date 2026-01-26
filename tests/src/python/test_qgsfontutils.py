"""QGIS Unit tests for core QgsFontUtils class

From build dir: ctest -R PyQgsFontUtils -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Larry Shaffer"
__date__ = "2014/02/19"
__copyright__ = "Copyright 2014, The QGIS Project"

from qgis.PyQt.QtCore import QCoreApplication
from qgis.core import QgsFontUtils, QgsSettings
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import getTestFontFamily, loadTestFonts


class TestQgsFontUtils(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("QGIS_TestPyQgsFontUtils.com")
        QCoreApplication.setApplicationName("QGIS_TestPyQgsFontUtils")
        QgsSettings().clear()

        start_app()

        cls._family = getTestFontFamily()
        cls._has_style = QgsFontUtils.fontFamilyHasStyle

    def test_loading_base_test_fonts(self):
        loadTestFonts()

    def test_loading_every_test_font(self):
        QgsFontUtils.loadStandardTestFonts(["All"])
        # styles = ''
        # for style in QFontDatabase().styles(self._family):
        #     styles += ' ' + style
        # print self._family + ' styles:' + styles

        res = (
            self._has_style(self._family, "Roman")
            and self._has_style(self._family, "Oblique")
            and self._has_style(self._family, "Bold")
            and self._has_style(self._family, "Bold Oblique")
        )
        msg = self._family + " test font styles could not be loaded"
        assert res, msg

    def test_get_specific_test_font(self):
        # default returned is Roman at 12 pt
        f = QgsFontUtils.getStandardTestFont("Bold Oblique", 14)
        """:type: QFont"""
        res = (
            f.family() == self._family
            and f.bold()
            and f.italic()
            and f.pointSize() == 14
        )
        msg = self._family + " test font Bold Oblique at 14 pt not retrieved"
        assert res, msg

    def testToFromMimeData(self):
        """
        Test converting QFonts to and from mime data
        """
        f = QgsFontUtils.getStandardTestFont("Bold Oblique", 14)
        mime_data = QgsFontUtils.toMimeData(f)
        self.assertTrue(mime_data is not None)

        res, ok = QgsFontUtils.fromMimeData(None)
        self.assertFalse(ok)
        res, ok = QgsFontUtils.fromMimeData(mime_data)
        self.assertTrue(ok)

        expected = (
            res.family() == self._family
            and res.bold()
            and res.italic()
            and res.pointSize() == 14
        )
        msg = (
            self._family
            + " test font Bold Oblique at 14 pt not retrieved from mime data"
        )
        self.assertTrue(res, msg)

    def testRecentFonts(self):
        """
        Test adding and retrieving recent fonts
        """

        # test empty list
        self.assertFalse(QgsFontUtils.recentFontFamilies())
        QgsFontUtils.addRecentFontFamily("Comic Sans FTW, suckers")
        self.assertEqual(QgsFontUtils.recentFontFamilies(), ["Comic Sans FTW, suckers"])
        QgsFontUtils.addRecentFontFamily("Arial")
        self.assertEqual(
            QgsFontUtils.recentFontFamilies(), ["Arial", "Comic Sans FTW, suckers"]
        )
        QgsFontUtils.addRecentFontFamily("Arial2")
        QgsFontUtils.addRecentFontFamily("Arial3")
        QgsFontUtils.addRecentFontFamily("Arial4")
        QgsFontUtils.addRecentFontFamily("Arial5")
        QgsFontUtils.addRecentFontFamily("Arial6")
        QgsFontUtils.addRecentFontFamily("Arial7")
        QgsFontUtils.addRecentFontFamily("Arial8")
        QgsFontUtils.addRecentFontFamily("Arial9")
        QgsFontUtils.addRecentFontFamily("Arial10")
        self.assertEqual(
            QgsFontUtils.recentFontFamilies(),
            [
                "Arial10",
                "Arial9",
                "Arial8",
                "Arial7",
                "Arial6",
                "Arial5",
                "Arial4",
                "Arial3",
                "Arial2",
                "Arial",
            ],
        )
        QgsFontUtils.addRecentFontFamily("Arial9")
        self.assertEqual(
            QgsFontUtils.recentFontFamilies(),
            [
                "Arial9",
                "Arial10",
                "Arial8",
                "Arial7",
                "Arial6",
                "Arial5",
                "Arial4",
                "Arial3",
                "Arial2",
                "Arial",
            ],
        )
        QgsFontUtils.addRecentFontFamily("Comic Sans FTW, suckers")
        self.assertEqual(
            QgsFontUtils.recentFontFamilies(),
            [
                "Comic Sans FTW, suckers",
                "Arial9",
                "Arial10",
                "Arial8",
                "Arial7",
                "Arial6",
                "Arial5",
                "Arial4",
                "Arial3",
                "Arial2",
            ],
        )


if __name__ == "__main__":
    unittest.main()
