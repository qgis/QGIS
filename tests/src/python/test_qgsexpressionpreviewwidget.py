"""QGIS Unit tests for QgsExpressionPreviewWidget

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

from qgis.PyQt.QtWidgets import QToolButton
from qgis.gui import QgsExpressionPreviewWidget
from qgis.core import (
    QgsExpressionContext,
    QgsExpressionContextScope
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsExpressionPreviewWidget(QgisTestCase):

    def test_custom_mode(self):
        """
        Test using a custom preview generator with the widget
        """
        def make_context(value):
            res = QgsExpressionContext()
            scope = QgsExpressionContextScope()
            scope.setVariable('test', value)
            scope.setVariable('test2', value * 2)
            res.appendScope(scope)
            return res

        w = QgsExpressionPreviewWidget()
        w.setCustomPreviewGenerator('Band',
                                    [['Band 1', 1], ['Band 2', 2], ['Band 3', 3]],
                                    make_context)
        w.setExpressionText("@test * 5")
        self.assertEqual(w.currentPreviewText(), '5')
        w.setExpressionText("@test2 * 5")
        self.assertEqual(w.currentPreviewText(), '10')

        next_button = w.findChild(QToolButton, 'mCustomButtonNext')
        prev_button = w.findChild(QToolButton, 'mCustomButtonPrev')
        self.assertFalse(prev_button.isEnabled())
        self.assertTrue(next_button.isEnabled())
        next_button.click()
        self.assertEqual(w.currentPreviewText(), '20')
        self.assertTrue(prev_button.isEnabled())
        self.assertTrue(next_button.isEnabled())
        next_button.click()
        self.assertEqual(w.currentPreviewText(), '30')
        self.assertTrue(prev_button.isEnabled())
        self.assertFalse(next_button.isEnabled())
        prev_button.click()
        self.assertEqual(w.currentPreviewText(), '20')
        self.assertTrue(prev_button.isEnabled())
        self.assertTrue(next_button.isEnabled())
        prev_button.click()
        self.assertEqual(w.currentPreviewText(), '10')
        self.assertFalse(prev_button.isEnabled())
        self.assertTrue(next_button.isEnabled())


if __name__ == '__main__':
    unittest.main()
