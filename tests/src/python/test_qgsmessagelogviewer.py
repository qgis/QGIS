"""QGIS Unit tests for QgsMessageLogViewer.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

from qgis.PyQt.QtWidgets import QTabWidget, QPlainTextEdit
from qgis.core import Qgis
from qgis.gui import QgsMessageLogViewer
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsMessageLogViewer(QgisTestCase):

    def testLogging(self):
        """
        Test logging to viewier
        """

        viewer = QgsMessageLogViewer()
        # find child widget
        tab_widget = viewer.findChildren(QTabWidget)[0]
        self.assertEqual(tab_widget.count(), 0)

        viewer.logMessage("Sample message", "Cat 1", Qgis.MessageLevel.Warning)
        self.assertEqual(tab_widget.count(), 1)
        widget_cat_1 = tab_widget.widget(0)
        self.assertIsInstance(widget_cat_1, QPlainTextEdit)
        self.assertIn("WARNING    Sample message", widget_cat_1.toPlainText())

        viewer.logMessage("second message", "Cat 1", Qgis.MessageLevel.Info)
        self.assertIn("WARNING    Sample message", widget_cat_1.toPlainText())
        self.assertIn("INFO    second message", widget_cat_1.toPlainText())

        viewer.logMessage("third message", "Cat 1", Qgis.MessageLevel.Success)
        self.assertIn("WARNING    Sample message", widget_cat_1.toPlainText())
        self.assertIn("INFO    second message", widget_cat_1.toPlainText())
        self.assertIn("SUCCESS    third message", widget_cat_1.toPlainText())

        viewer.logMessage("Other message", "Cat 2", Qgis.MessageLevel.Warning)
        self.assertEqual(tab_widget.count(), 2)
        widget_cat_2 = tab_widget.widget(1)
        self.assertIsInstance(widget_cat_2, QPlainTextEdit)
        self.assertIn("WARNING    Other message", widget_cat_2.toPlainText())
        self.assertIn("WARNING    Sample message", widget_cat_1.toPlainText())
        self.assertIn("INFO    second message", widget_cat_1.toPlainText())
        self.assertIn("SUCCESS    third message", widget_cat_1.toPlainText())

        # log a message with HTML characters
        viewer.logMessage("Special <characters>", "Cat 2", Qgis.MessageLevel.Warning)
        self.assertIn("WARNING    Other message", widget_cat_2.toPlainText())
        self.assertIn("WARNING    Special <characters>", widget_cat_2.toPlainText())


if __name__ == "__main__":
    unittest.main()
