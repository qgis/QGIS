"""QGIS Unit tests for QgsElevationProfileCanvas

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "28/3/2022"
__copyright__ = "Copyright 2022, The QGIS Project"

from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtGui import QColor
from qgis.core import QgsSingleItemModel
import unittest
from qgis.testing import start_app, QgisTestCase

app = start_app()


class TestQgsSingleItemModel(QgisTestCase):

    def testModel(self):
        model = QgsSingleItemModel(
            None,
            "my item",
            {
                Qt.ItemDataRole.ForegroundRole: QColor(255, 0, 0),
                Qt.ItemDataRole.BackgroundRole: QColor(0, 255, 0),
                Qt.ItemDataRole.UserRole + 123: "abc",
            },
            Qt.ItemFlag.ItemIsSelectable | Qt.ItemFlag.ItemIsDropEnabled,
        )

        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.columnCount(), 1)

        index = model.index(0, 0)
        self.assertEqual(model.data(index, Qt.ItemDataRole.DisplayRole), "my item")
        # by default tooltip should follow item text
        self.assertEqual(model.data(index, Qt.ItemDataRole.ToolTipRole), "my item")
        self.assertEqual(
            model.data(index, Qt.ItemDataRole.ForegroundRole), QColor(255, 0, 0)
        )
        self.assertEqual(
            model.data(index, Qt.ItemDataRole.BackgroundRole), QColor(0, 255, 0)
        )
        self.assertEqual(
            model.data(index, Qt.ItemDataRole.BackgroundRole), QColor(0, 255, 0)
        )
        self.assertEqual(model.data(index, Qt.ItemDataRole.UserRole + 123), "abc")
        self.assertIsNone(model.data(index, Qt.ItemDataRole.UserRole + 124))

        self.assertEqual(
            model.flags(index),
            Qt.ItemFlag.ItemIsSelectable | Qt.ItemFlag.ItemIsDropEnabled,
        )

    def testToolTip(self):
        model = QgsSingleItemModel(
            None,
            "my item",
            {Qt.ItemDataRole.ToolTipRole: "abc"},
            Qt.ItemFlag.ItemIsSelectable | Qt.ItemFlag.ItemIsDropEnabled,
        )

        index = model.index(0, 0)
        self.assertEqual(model.data(index, Qt.ItemDataRole.DisplayRole), "my item")
        # manually specified tooltip should take precedence
        self.assertEqual(model.data(index, Qt.ItemDataRole.ToolTipRole), "abc")

    def testModelWithColumns(self):
        model = QgsSingleItemModel(
            None,
            [
                {
                    Qt.ItemDataRole.DisplayRole: "col 1",
                    Qt.ItemDataRole.ToolTipRole: "column 1",
                },
                {
                    Qt.ItemDataRole.DisplayRole: "col 2",
                    Qt.ItemDataRole.ToolTipRole: "column 2",
                },
                {Qt.ItemDataRole.DisplayRole: "col 3"},
            ],
            Qt.ItemFlag.ItemIsSelectable | Qt.ItemFlag.ItemIsDropEnabled,
        )

        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.columnCount(), 3)

        index = model.index(0, 0)
        self.assertEqual(model.data(index, Qt.ItemDataRole.DisplayRole), "col 1")
        self.assertEqual(model.data(index, Qt.ItemDataRole.ToolTipRole), "column 1")
        index = model.index(0, 1)
        self.assertEqual(model.data(index, Qt.ItemDataRole.DisplayRole), "col 2")
        self.assertEqual(model.data(index, Qt.ItemDataRole.ToolTipRole), "column 2")
        index = model.index(0, 2)
        self.assertEqual(model.data(index, Qt.ItemDataRole.DisplayRole), "col 3")
        self.assertFalse(model.data(index, Qt.ItemDataRole.ToolTipRole))

        self.assertEqual(
            model.flags(index),
            Qt.ItemFlag.ItemIsSelectable | Qt.ItemFlag.ItemIsDropEnabled,
        )


if __name__ == "__main__":
    unittest.main()
