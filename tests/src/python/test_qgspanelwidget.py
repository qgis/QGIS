"""QGIS Unit tests for QgsPanelWidget.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "16/08/2016"
__copyright__ = "Copyright 2016, The QGIS Project"

from qgis.PyQt.QtWidgets import QDialog, QWidget
from qgis.gui import QgsPanelWidget
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsPanelWidget(QgisTestCase):

    def testFindParentPanel(self):
        """test QgsPanelWidget.findParentPanel"""

        # no widget
        self.assertFalse(QgsPanelWidget.findParentPanel(None))

        # widget with no parent
        w = QWidget()
        self.assertFalse(QgsPanelWidget.findParentPanel(w))

        # widget with no panel parent
        w2 = QWidget(w)
        self.assertFalse(QgsPanelWidget.findParentPanel(w2))

        # panel widget itself
        w3 = QgsPanelWidget()
        self.assertEqual(QgsPanelWidget.findParentPanel(w3), w3)

        # widget with direct QgsPanelWidget parent
        w4 = QWidget(w3)
        self.assertEqual(QgsPanelWidget.findParentPanel(w4), w3)

        # widget with QgsPanelWidget grandparent
        w5 = QWidget(w4)
        self.assertEqual(QgsPanelWidget.findParentPanel(w5), w3)

        # chain should be broken when a new window is encountered
        n = QgsPanelWidget()
        n2 = QDialog(n)
        n3 = QWidget(n2)
        self.assertFalse(QgsPanelWidget.findParentPanel(n3))


if __name__ == "__main__":
    unittest.main()
