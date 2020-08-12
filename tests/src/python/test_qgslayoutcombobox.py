# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsLayoutComboBox

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2019 by Nyall Dawson'
__date__ = '11/03/2019'
__copyright__ = 'Copyright 2019, The QGIS Project'

import qgis  # NOQA

from qgis.PyQt.QtXml import QDomDocument

from qgis.core import (QgsPrintLayout,
                       QgsLayoutManager,
                       QgsLayoutManagerModel,
                       QgsLayoutManagerProxyModel,
                       QgsProject,
                       QgsReport,
                       QgsMasterLayoutInterface)
from qgis.gui import QgsLayoutComboBox
from qgis.PyQt.QtCore import Qt, QModelIndex
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath
from qgis.PyQt.QtXml import QDomDocument
from qgis.PyQt.QtTest import QSignalSpy

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsLayoutComboBox(unittest.TestCase):

    def setUp(self):
        """Run before each test."""
        self.manager = None
        self.aboutFired = False

    def tearDown(self):
        """Run after each test."""
        pass

    def testCombo(self):
        project = QgsProject()
        manager = QgsLayoutManager(project)
        layout = QgsPrintLayout(project)
        layout.setName('ccc')
        self.assertTrue(manager.addLayout(layout))
        layout2 = QgsPrintLayout(project)
        layout2.setName('bbb')
        self.assertTrue(manager.addLayout(layout2))
        r = QgsReport(project)
        r.setName('ddd')
        manager.addLayout(r)

        combo = QgsLayoutComboBox(None, manager)
        spy = QSignalSpy(combo.layoutChanged)
        self.assertEqual(combo.count(), 3)

        self.assertEqual(combo.itemText(0), 'bbb')
        self.assertEqual(combo.itemText(1), 'ccc')
        self.assertEqual(combo.itemText(2), 'ddd')

        self.assertEqual(combo.layout(0), layout2)
        self.assertEqual(combo.layout(1), layout)
        self.assertEqual(combo.layout(2), r)

        combo.setCurrentLayout(None)
        self.assertEqual(combo.currentLayout(), None)
        self.assertEqual(len(spy), 1)
        combo.setCurrentLayout(layout)
        self.assertEqual(combo.currentLayout(), layout)
        self.assertEqual(len(spy), 2)
        combo.setCurrentLayout(r)
        self.assertEqual(combo.currentLayout(), r)
        self.assertEqual(len(spy), 3)
        combo.setCurrentLayout(layout2)
        self.assertEqual(combo.currentLayout(), layout2)
        self.assertEqual(len(spy), 4)

        combo.setAllowEmptyLayout(True)
        self.assertEqual(combo.count(), 4)
        self.assertEqual(combo.itemText(0), '')
        self.assertEqual(combo.itemText(1), 'bbb')
        self.assertEqual(combo.itemText(2), 'ccc')
        self.assertEqual(combo.itemText(3), 'ddd')
        combo.setCurrentLayout(None)
        self.assertEqual(combo.currentIndex(), 0)

        combo.setFilters(QgsLayoutManagerProxyModel.FilterPrintLayouts)
        self.assertEqual(combo.count(), 3)
        self.assertEqual(combo.itemText(0), '')
        self.assertEqual(combo.itemText(1), 'bbb')
        self.assertEqual(combo.itemText(2), 'ccc')

        combo.setFilters(QgsLayoutManagerProxyModel.FilterReports)
        self.assertEqual(combo.filters(), QgsLayoutManagerProxyModel.FilterReports)
        self.assertEqual(combo.count(), 2)
        self.assertEqual(combo.itemText(0), '')
        self.assertEqual(combo.itemText(1), 'ddd')


if __name__ == '__main__':
    unittest.main()
