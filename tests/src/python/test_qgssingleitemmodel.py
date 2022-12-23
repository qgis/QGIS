# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsElevationProfileCanvas

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '28/3/2022'
__copyright__ = 'Copyright 2022, The QGIS Project'

import qgis  # NOQA

from qgis.PyQt.QtCore import (
    QDir,
    QEvent,
    Qt,
    QPoint,
    QPointF
)
from qgis.PyQt.QtGui import (
    QColor
)

from qgis.core import (
    QgsSingleItemModel
)
from qgis.testing import start_app, unittest

app = start_app()


class TestQgsSingleItemModel(unittest.TestCase):

    def testModel(self):
        model = QgsSingleItemModel(None, 'my item', {
            Qt.ForegroundRole: QColor(255, 0, 0),
            Qt.BackgroundRole: QColor(0, 255, 0),
            Qt.UserRole + 123: 'abc'
        }, Qt.ItemIsSelectable | Qt.ItemIsDropEnabled)

        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.columnCount(), 1)

        index = model.index(0, 0)
        self.assertEqual(model.data(index, Qt.DisplayRole), 'my item')
        # by default tooltip should follow item text
        self.assertEqual(model.data(index, Qt.ToolTipRole), 'my item')
        self.assertEqual(model.data(index, Qt.ForegroundRole), QColor(255, 0, 0))
        self.assertEqual(model.data(index, Qt.BackgroundRole), QColor(0, 255, 0))
        self.assertEqual(model.data(index, Qt.BackgroundRole), QColor(0, 255, 0))
        self.assertEqual(model.data(index, Qt.UserRole + 123), 'abc')
        self.assertIsNone(model.data(index, Qt.UserRole + 124))

        self.assertEqual(model.flags(index), Qt.ItemIsSelectable | Qt.ItemIsDropEnabled)

    def testToolTip(self):
        model = QgsSingleItemModel(None, 'my item', {
            Qt.ToolTipRole: 'abc'
        }, Qt.ItemIsSelectable | Qt.ItemIsDropEnabled)

        index = model.index(0, 0)
        self.assertEqual(model.data(index, Qt.DisplayRole), 'my item')
        # manually specified tooltip should take precedence
        self.assertEqual(model.data(index, Qt.ToolTipRole), 'abc')

    def testModelWithColumns(self):
        model = QgsSingleItemModel(None, [
            {Qt.DisplayRole: 'col 1', Qt.ToolTipRole: 'column 1'},
            {Qt.DisplayRole: 'col 2', Qt.ToolTipRole: 'column 2'},
            {Qt.DisplayRole: 'col 3'},
        ], Qt.ItemIsSelectable | Qt.ItemIsDropEnabled)

        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.columnCount(), 3)

        index = model.index(0, 0)
        self.assertEqual(model.data(index, Qt.DisplayRole), 'col 1')
        self.assertEqual(model.data(index, Qt.ToolTipRole), 'column 1')
        index = model.index(0, 1)
        self.assertEqual(model.data(index, Qt.DisplayRole), 'col 2')
        self.assertEqual(model.data(index, Qt.ToolTipRole), 'column 2')
        index = model.index(0, 2)
        self.assertEqual(model.data(index, Qt.DisplayRole), 'col 3')
        self.assertFalse(model.data(index, Qt.ToolTipRole))

        self.assertEqual(model.flags(index), Qt.ItemIsSelectable | Qt.ItemIsDropEnabled)


if __name__ == '__main__':
    unittest.main()
