"""
test_qgstabwidget.py
                     --------------------------------------
               Date                 : September 2016
               Copyright            : (C) 2016 Matthias Kuhn
               email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
"""

from qgis.PyQt.QtWidgets import QWidget
from qgis.gui import QgsTabWidget
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsTabWidget(QgisTestCase):

    def setUp(self):
        """Run before each test."""
        pass

    def tearDown(self):
        """Run after each test."""
        pass

    def testQgsTabWidget(self):
        tabWidget = QgsTabWidget()

        wdg1 = QWidget()
        wdg2 = QWidget()
        wdg3 = QWidget()

        tabWidget.addTab(wdg1, "1")
        tabWidget.addTab(wdg2, "2")
        tabWidget.addTab(wdg3, "3")

        tabWidget.hideTab(wdg2)
        self.assertEqual(tabWidget.count(), 2)
        tabWidget.showTab(wdg2)
        self.assertEqual(tabWidget.count(), 3)

        self.assertEqual(tabWidget.tabText(0), "1")
        self.assertEqual(tabWidget.tabText(1), "2")
        self.assertEqual(tabWidget.tabText(2), "3")

        tabWidget.hideTab(wdg2)
        tabWidget.removeTab(1)
        self.assertEqual(tabWidget.tabText(0), "1")
        tabWidget.showTab(wdg2)
        self.assertEqual(tabWidget.tabText(1), "2")
        self.assertEqual(tabWidget.count(), 2)

        # Show an already visible tab
        tabWidget.showTab(wdg2)
        self.assertEqual(tabWidget.count(), 2)

        # Hide twice
        tabWidget.hideTab(wdg2)
        self.assertEqual(tabWidget.count(), 1)
        tabWidget.hideTab(wdg2)
        self.assertEqual(tabWidget.count(), 1)

        tabWidget.hideTab(wdg1)
        self.assertEqual(tabWidget.count(), 0)

        tabWidget.showTab(wdg1)
        tabWidget.showTab(wdg2)
        self.assertEqual(tabWidget.count(), 2)

        tabWidget.removeTab(0)
        self.assertEqual(tabWidget.count(), 1)
        tabWidget.hideTab(wdg2)
        self.assertEqual(tabWidget.count(), 0)


if __name__ == "__main__":
    unittest.main()
