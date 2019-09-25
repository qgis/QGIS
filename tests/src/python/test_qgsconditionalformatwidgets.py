
# -*- coding: utf-8 -*-
"""QGIS Unit tests for the conditional format widgets.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '2019-09-25'
__copyright__ = 'Copyright 2019, The QGIS Project'

from qgis.core import (QgsConditionalStyle,
                       QgsMarkerSymbol)
from qgis.gui import QgsEditConditionalFormatRuleWidget
from qgis.testing import (start_app,
                          unittest,
                          )
from utilities import unitTestDataPath, getTestFont
from qgis.PyQt.QtGui import QColor

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestPyQgsConditionalFormatWidgets(unittest.TestCase):

    def testEditorWidget(self):
        c = QgsConditionalStyle()
        c.setName('')

        w = QgsEditConditionalFormatRuleWidget()
        w.loadStyle(c)
        self.assertEqual(w.currentStyle(), c)
        w.setRule('my rule')
        self.assertEqual(w.currentStyle().rule(), 'my rule')

        c.setName('n')
        w = QgsEditConditionalFormatRuleWidget()
        w.loadStyle(c)
        self.assertEqual(w.currentStyle(), c)

        c.setRule('1=1')
        w = QgsEditConditionalFormatRuleWidget()
        w.loadStyle(c)
        self.assertEqual(w.currentStyle(), c)

        c.setBackgroundColor(QColor(255, 0, 0))
        w = QgsEditConditionalFormatRuleWidget()
        w.loadStyle(c)
        self.assertEqual(w.currentStyle(), c)

        c.setTextColor(QColor(0, 255, 0))
        w = QgsEditConditionalFormatRuleWidget()
        w.loadStyle(c)
        self.assertEqual(w.currentStyle(), c)

        c.setSymbol(QgsMarkerSymbol.createSimple({}))
        w = QgsEditConditionalFormatRuleWidget()
        w.loadStyle(c)
        self.assertEqual(w.currentStyle(), c)

        f = getTestFont()
        c.setFont(f)
        w = QgsEditConditionalFormatRuleWidget()
        w.loadStyle(c)
        self.assertEqual(w.currentStyle().font(), c.font())

        f.setBold(True)
        c.setFont(f)
        w = QgsEditConditionalFormatRuleWidget()
        w.loadStyle(c)
        self.assertEqual(w.currentStyle().font().bold(), True)

        f.setItalic(True)
        c.setFont(f)
        w = QgsEditConditionalFormatRuleWidget()
        w.loadStyle(c)
        self.assertEqual(w.currentStyle().font().italic(), True)

        f.setStrikeOut(True)
        c.setFont(f)
        w = QgsEditConditionalFormatRuleWidget()
        w.loadStyle(c)
        self.assertEqual(w.currentStyle().font().strikeOut(), True)

        f.setUnderline(True)
        c.setFont(f)
        w = QgsEditConditionalFormatRuleWidget()
        w.loadStyle(c)
        self.assertEqual(w.currentStyle().font().underline(), True)


if __name__ == '__main__':
    unittest.main()
