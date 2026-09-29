"""QGIS Unit tests for QgsMapLayerStyleModel

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import unittest

from qgis.core import (
    QgsVectorLayer,
)
from qgis.gui import QgsMapLayerStyleModel
from qgis.PyQt.QtCore import QModelIndex, Qt
from qgis.PyQt.QtTest import QSignalSpy
from qgis.testing import QgisTestCase, start_app

start_app()


def create_layer(name):
    layer = QgsVectorLayer(
        "Point?crs=EPSG:3111&field=fldtxt:string&field=fldint:integer", name, "memory"
    )
    return layer


class TestQgsMapLayeStyleModel(QgisTestCase):
    def testModel(self):
        """
        Test basic model behavior
        """
        vl = QgsVectorLayer(
            "Point?crs=EPSG:3111&field=fldtxt:string&field=fldint:integer",
            "test",
            "memory",
        )
        manager = vl.styleManager()
        model = QgsMapLayerStyleModel(manager)

        data_changed_spy = QSignalSpy(model.dataChanged)

        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.data(model.index(0, 0, QModelIndex())), "default")
        # invalid index
        self.assertIsNone(model.data(model.index(-1, 0, QModelIndex())))
        self.assertIsNone(model.data(model.index(1, 0, QModelIndex())))
        self.assertIsNone(model.data(model.index(0, 1, QModelIndex())))
        # other roles
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.ToolTipRole),
            "default",
        )
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.EditRole),
            "default",
        )

        self.assertEqual(len(data_changed_spy), 0)
        manager.renameStyle("default", "my style")
        self.assertEqual(len(data_changed_spy), 1)
        self.assertEqual(model.data(model.index(0, 0, QModelIndex())), "my style")
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.ToolTipRole),
            "my style",
        )
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.EditRole),
            "my style",
        )

        # invalid rename
        manager.renameStyle("defaultxZX", "zxxmy style")
        self.assertEqual(len(data_changed_spy), 1)
        self.assertEqual(model.data(model.index(0, 0, QModelIndex())), "my style")

        # add style
        manager.addStyleFromLayer("New Style")
        self.assertEqual(model.rowCount(), 2)
        self.assertEqual(model.data(model.index(0, 0, QModelIndex())), "my style")
        self.assertEqual(model.data(model.index(1, 0, QModelIndex())), "New Style")

        manager.addStyleFromLayer("Another Style")
        self.assertEqual(model.rowCount(), 3)
        self.assertEqual(model.data(model.index(0, 0, QModelIndex())), "my style")
        self.assertEqual(model.data(model.index(1, 0, QModelIndex())), "New Style")
        self.assertEqual(model.data(model.index(2, 0, QModelIndex())), "Another Style")
        self.assertIsNone(model.data(model.index(3, 0, QModelIndex())))

        manager.removeStyle("New Style")
        self.assertEqual(model.rowCount(), 2)
        self.assertEqual(model.data(model.index(0, 0, QModelIndex())), "my style")
        self.assertEqual(model.data(model.index(1, 0, QModelIndex())), "Another Style")

        manager.renameStyle("Another Style", "Another Style2")
        self.assertEqual(len(data_changed_spy), 2)
        self.assertEqual(model.rowCount(), 2)
        self.assertEqual(model.data(model.index(0, 0, QModelIndex())), "my style")
        self.assertEqual(model.data(model.index(1, 0, QModelIndex())), "Another Style2")

        self.assertEqual(model.indexForName("my style").row(), 0)
        self.assertEqual(model.indexForName("Another Style2").row(), 1)
        self.assertFalse(model.indexForName("xxx").isValid())

    def test_model_edits(self):
        vl = QgsVectorLayer(
            "Point?crs=EPSG:3111&field=fldtxt:string&field=fldint:integer",
            "test",
            "memory",
        )
        manager = vl.styleManager()
        model = QgsMapLayerStyleModel(manager)

        self.assertTrue(
            model.flags(model.index(0, 0, QModelIndex())) & Qt.ItemFlag.ItemIsEditable
        )
        manager.addStyleFromLayer("New Style")

        self.assertFalse(model.setData(model.index(-1, 0, QModelIndex()), "xxx"))
        self.assertFalse(
            model.setData(
                model.index(0, 0, QModelIndex()), "xxx", Qt.ItemDataRole.CheckStateRole
            )
        )

        # can't rename to empty name
        self.assertFalse(model.setData(model.index(0, 0, QModelIndex()), ""))
        # rename to same name - no op
        self.assertTrue(model.setData(model.index(0, 0, QModelIndex()), "default"))
        # can't rename to another existing name
        self.assertFalse(model.setData(model.index(0, 0, QModelIndex()), "New Style"))

        # valid renames
        self.assertTrue(
            model.setData(model.index(0, 0, QModelIndex()), "Renamed style")
        )
        self.assertCountEqual(manager.styles(), ["New Style", "Renamed style"])
        self.assertEqual(model.data(model.index(0, 0, QModelIndex())), "Renamed style")
        self.assertEqual(model.data(model.index(1, 0, QModelIndex())), "New Style")

        self.assertTrue(model.setData(model.index(1, 0, QModelIndex()), "Better style"))
        self.assertCountEqual(manager.styles(), ["Better style", "Renamed style"])
        self.assertEqual(model.data(model.index(0, 0, QModelIndex())), "Renamed style")
        self.assertEqual(model.data(model.index(1, 0, QModelIndex())), "Better style")


if __name__ == "__main__":
    unittest.main()
