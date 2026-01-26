"""QGIS Unit tests for QgsExpressionLineEdit

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "20/08/2016"
__copyright__ = "Copyright 2016, The QGIS Project"


try:
    from qgis.PyQt.QtTest import QSignalSpy

    use_signal_spy = True
except:
    use_signal_spy = False

from qgis.gui import QgsExpressionLineEdit
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsExpressionLineEdit(QgisTestCase):

    def testDialog(self):
        """test dialog related methods"""
        w = QgsExpressionLineEdit()
        w.setExpressionDialogTitle("test")
        self.assertEqual(w.expressionDialogTitle(), "test")

    def testSetGetExpression(self):
        """test setting and getting expression"""
        w = QgsExpressionLineEdit()
        self.assertFalse(w.expression())
        w.setExpression("1+2")
        self.assertEqual(w.expression(), "1+2")
        result, error = w.isValidExpression()
        self.assertTrue(result)
        w.setExpression("1+")
        self.assertEqual(w.expression(), "1+")
        result, error = w.isValidExpression()
        self.assertFalse(result)
        self.assertTrue(error)

        # try with a multiline widget too
        w.setMultiLine(True)
        self.assertEqual(w.expression(), "1+")
        w.setExpression("1+3")
        self.assertEqual(w.expression(), "1+3")

        # and flip back again...
        w.setMultiLine(False)
        self.assertEqual(w.expression(), "1+3")

    @unittest.skipIf(not use_signal_spy, "No QSignalSpy available")
    def test_ChangedSignals(self):
        """test that signals are correctly emitted when changing expressions"""

        w = QgsExpressionLineEdit()

        expression_changed_spy = QSignalSpy(w.expressionChanged)
        w.setExpression("1+1")

        self.assertEqual(len(expression_changed_spy), 1)
        self.assertEqual(expression_changed_spy[0][0], "1+1")


if __name__ == "__main__":
    unittest.main()
