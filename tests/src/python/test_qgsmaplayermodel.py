"""QGIS Unit tests for QgsMapLayerModel

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "16/11/2016"
__copyright__ = "Copyright 2016, The QGIS Project"

from qgis.PyQt.QtCore import QCoreApplication, QEvent, QModelIndex, Qt
from qgis.core import (
    QgsApplication,
    QgsMapLayerModel,
    QgsProject,
    QgsVectorLayer,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


def create_layer(name):
    layer = QgsVectorLayer(
        "Point?crs=EPSG:3111&field=fldtxt:string&field=fldint:integer", name, "memory"
    )
    return layer


class TestQgsMapLayerModel(QgisTestCase):

    def testGettersSetters(self):
        """test model getters/setters"""
        m = QgsMapLayerModel()

        m.setItemsCheckable(True)
        self.assertTrue(m.itemsCheckable())
        m.setItemsCheckable(False)
        self.assertFalse(m.itemsCheckable())

        m.setAllowEmptyLayer(True)
        self.assertTrue(m.allowEmptyLayer())
        m.setAllowEmptyLayer(False)
        self.assertFalse(m.allowEmptyLayer())

        m.setShowCrs(True)
        self.assertTrue(m.showCrs())
        m.setShowCrs(False)
        self.assertFalse(m.showCrs())

        m.setAdditionalItems(["a", "b"])
        self.assertEqual(m.additionalItems(), ["a", "b"])
        m.setAdditionalItems([])
        self.assertFalse(m.additionalItems())

    def testAddingRemovingLayers(self):
        # test model handles layer addition and removal
        m = QgsMapLayerModel()

        self.assertEqual(m.rowCount(QModelIndex()), 0)

        l1 = create_layer("l1")
        QgsProject.instance().addMapLayer(l1)
        self.assertEqual(m.rowCount(QModelIndex()), 1)
        self.assertEqual(m.layerFromIndex(m.index(0, 0)), l1)
        l2 = create_layer("l2")
        QgsProject.instance().addMapLayer(l2)
        self.assertEqual(m.rowCount(QModelIndex()), 2)
        self.assertEqual(m.layerFromIndex(m.index(0, 0)), l1)
        self.assertEqual(m.layerFromIndex(m.index(1, 0)), l2)
        QgsProject.instance().removeMapLayer(l1)
        self.assertEqual(m.rowCount(QModelIndex()), 1)
        self.assertEqual(m.layerFromIndex(m.index(0, 0)), l2)
        QgsProject.instance().removeMapLayer(l2)
        self.assertEqual(m.rowCount(QModelIndex()), 0)

        # try creating a model when layers already exist in registry
        l1 = create_layer("l1")
        l2 = create_layer("l2")
        QgsProject.instance().addMapLayers([l1, l2])
        m = QgsMapLayerModel()
        self.assertEqual(m.rowCount(QModelIndex()), 2)
        QgsProject.instance().removeMapLayers([l1.id(), l2.id()])
        self.assertEqual(m.rowCount(QModelIndex()), 0)

    def testCheckAll(self):
        l1 = create_layer("l1")
        l2 = create_layer("l2")
        QgsProject.instance().addMapLayers([l1, l2])
        m = QgsMapLayerModel()
        m.setItemsCheckable(True)
        self.assertFalse(m.layersChecked())
        self.assertEqual(set(m.layersChecked(Qt.CheckState.Unchecked)), {l1, l2})

        m.checkAll(Qt.CheckState.Checked)
        self.assertEqual(set(m.layersChecked()), {l1, l2})
        self.assertFalse(set(m.layersChecked(Qt.CheckState.Unchecked)))

        m.checkAll(Qt.CheckState.Unchecked)
        self.assertFalse(m.layersChecked())
        self.assertEqual(set(m.layersChecked(Qt.CheckState.Unchecked)), {l1, l2})

        QgsProject.instance().removeMapLayers([l1.id(), l2.id()])

    def testAllowEmpty(self):
        l1 = create_layer("l1")
        l2 = create_layer("l2")
        QgsProject.instance().addMapLayers([l1, l2])
        m = QgsMapLayerModel()
        self.assertEqual(m.rowCount(QModelIndex()), 2)

        m.setAllowEmptyLayer(True)
        self.assertEqual(m.rowCount(QModelIndex()), 3)
        self.assertFalse(m.data(m.index(0, 0), Qt.ItemDataRole.DisplayRole))
        m.setAllowEmptyLayer(False)
        self.assertEqual(m.rowCount(QModelIndex()), 2)
        self.assertTrue(m.data(m.index(0, 0), Qt.ItemDataRole.DisplayRole))

        # add layers after allow empty is true
        m.setAllowEmptyLayer(True)
        l3 = create_layer("l3")
        QgsProject.instance().addMapLayers([l3])
        self.assertEqual(m.rowCount(QModelIndex()), 4)
        self.assertFalse(m.data(m.index(0, 0), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(m.data(m.index(1, 0), Qt.ItemDataRole.DisplayRole), "l1")
        self.assertEqual(m.data(m.index(2, 0), Qt.ItemDataRole.DisplayRole), "l2")
        self.assertEqual(m.data(m.index(3, 0), Qt.ItemDataRole.DisplayRole), "l3")

        self.assertIsNone(m.data(m.index(0, 0), Qt.ItemDataRole.DecorationRole))
        # set icon and text for empty item
        m.setAllowEmptyLayer(
            True, "empty", QgsApplication.getThemeIcon("/mItemBookmark.svg")
        )
        self.assertEqual(m.data(m.index(0, 0), Qt.ItemDataRole.DisplayRole), "empty")
        self.assertFalse(m.data(m.index(0, 0), Qt.ItemDataRole.DecorationRole).isNull())

        QgsProject.instance().removeMapLayers([l1.id(), l2.id(), l3.id()])

    def testAdditionalItems(self):
        l1 = create_layer("l1")
        l2 = create_layer("l2")
        QgsProject.instance().addMapLayers([l1, l2])
        m = QgsMapLayerModel()
        self.assertEqual(m.rowCount(QModelIndex()), 2)

        m.setAdditionalItems(["a", "b"])
        self.assertEqual(m.rowCount(QModelIndex()), 4)
        self.assertEqual(m.data(m.index(0, 0), Qt.ItemDataRole.DisplayRole), "l1")
        self.assertEqual(m.data(m.index(1, 0), Qt.ItemDataRole.DisplayRole), "l2")
        self.assertEqual(m.data(m.index(2, 0), Qt.ItemDataRole.DisplayRole), "a")
        self.assertEqual(m.data(m.index(3, 0), Qt.ItemDataRole.DisplayRole), "b")

        m.setAllowEmptyLayer(True)
        self.assertEqual(m.rowCount(QModelIndex()), 5)
        self.assertFalse(m.data(m.index(0, 0), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(m.data(m.index(1, 0), Qt.ItemDataRole.DisplayRole), "l1")
        self.assertEqual(m.data(m.index(2, 0), Qt.ItemDataRole.DisplayRole), "l2")
        self.assertEqual(m.data(m.index(3, 0), Qt.ItemDataRole.DisplayRole), "a")
        self.assertEqual(m.data(m.index(4, 0), Qt.ItemDataRole.DisplayRole), "b")

        QgsProject.instance().removeMapLayers([l1.id(), l2.id()])

        self.assertEqual(m.rowCount(QModelIndex()), 3)
        self.assertFalse(m.data(m.index(0, 0), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(m.data(m.index(1, 0), Qt.ItemDataRole.DisplayRole), "a")
        self.assertEqual(m.data(m.index(2, 0), Qt.ItemDataRole.DisplayRole), "b")

    def testAdditionalLayers(self):
        l1 = create_layer("l1")
        l2 = create_layer("l2")
        QgsProject.instance().addMapLayers([l1, l2])
        m = QgsMapLayerModel()
        self.assertEqual(m.rowCount(QModelIndex()), 2)
        l3 = create_layer("l3")
        l4 = create_layer("l4")
        m.setAdditionalLayers([l3, l4])
        self.assertEqual(m.rowCount(QModelIndex()), 4)

        m.setAdditionalItems(["a", "b"])
        self.assertEqual(m.rowCount(QModelIndex()), 6)
        self.assertEqual(m.data(m.index(0, 0), Qt.ItemDataRole.DisplayRole), "l1")
        self.assertEqual(m.data(m.index(1, 0), Qt.ItemDataRole.DisplayRole), "l2")
        self.assertEqual(m.data(m.index(2, 0), Qt.ItemDataRole.DisplayRole), "l3")
        self.assertEqual(m.data(m.index(3, 0), Qt.ItemDataRole.DisplayRole), "l4")
        self.assertEqual(m.data(m.index(4, 0), Qt.ItemDataRole.DisplayRole), "a")
        self.assertEqual(m.data(m.index(5, 0), Qt.ItemDataRole.DisplayRole), "b")

        m.setAllowEmptyLayer(True)
        self.assertEqual(m.rowCount(QModelIndex()), 7)
        self.assertFalse(m.data(m.index(0, 0), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(m.data(m.index(1, 0), Qt.ItemDataRole.DisplayRole), "l1")
        self.assertEqual(m.data(m.index(2, 0), Qt.ItemDataRole.DisplayRole), "l2")
        self.assertEqual(m.data(m.index(3, 0), Qt.ItemDataRole.DisplayRole), "l3")
        self.assertEqual(m.data(m.index(4, 0), Qt.ItemDataRole.DisplayRole), "l4")
        self.assertEqual(m.data(m.index(5, 0), Qt.ItemDataRole.DisplayRole), "a")
        self.assertEqual(m.data(m.index(6, 0), Qt.ItemDataRole.DisplayRole), "b")

        l3.deleteLater()
        QCoreApplication.sendPostedEvents(None, QEvent.Type.DeferredDelete)
        self.assertEqual(m.rowCount(QModelIndex()), 6)
        self.assertFalse(m.data(m.index(0, 0), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(m.data(m.index(1, 0), Qt.ItemDataRole.DisplayRole), "l1")
        self.assertEqual(m.data(m.index(2, 0), Qt.ItemDataRole.DisplayRole), "l2")
        self.assertEqual(m.data(m.index(3, 0), Qt.ItemDataRole.DisplayRole), "l4")
        self.assertEqual(m.data(m.index(4, 0), Qt.ItemDataRole.DisplayRole), "a")
        self.assertEqual(m.data(m.index(5, 0), Qt.ItemDataRole.DisplayRole), "b")

        l5 = create_layer("l5")
        l6 = create_layer("l6")
        m.setAdditionalLayers([l5, l6, l4])
        self.assertEqual(m.rowCount(QModelIndex()), 8)
        self.assertFalse(m.data(m.index(0, 0), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(m.data(m.index(1, 0), Qt.ItemDataRole.DisplayRole), "l1")
        self.assertEqual(m.data(m.index(2, 0), Qt.ItemDataRole.DisplayRole), "l2")
        self.assertEqual(m.data(m.index(3, 0), Qt.ItemDataRole.DisplayRole), "l5")
        self.assertEqual(m.data(m.index(4, 0), Qt.ItemDataRole.DisplayRole), "l6")
        self.assertEqual(m.data(m.index(5, 0), Qt.ItemDataRole.DisplayRole), "l4")
        self.assertEqual(m.data(m.index(6, 0), Qt.ItemDataRole.DisplayRole), "a")
        self.assertEqual(m.data(m.index(7, 0), Qt.ItemDataRole.DisplayRole), "b")

        m.setAdditionalLayers([l5, l4])
        self.assertEqual(m.rowCount(QModelIndex()), 7)
        self.assertFalse(m.data(m.index(0, 0), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(m.data(m.index(1, 0), Qt.ItemDataRole.DisplayRole), "l1")
        self.assertEqual(m.data(m.index(2, 0), Qt.ItemDataRole.DisplayRole), "l2")
        self.assertEqual(m.data(m.index(3, 0), Qt.ItemDataRole.DisplayRole), "l5")
        self.assertEqual(m.data(m.index(4, 0), Qt.ItemDataRole.DisplayRole), "l4")
        self.assertEqual(m.data(m.index(5, 0), Qt.ItemDataRole.DisplayRole), "a")
        self.assertEqual(m.data(m.index(6, 0), Qt.ItemDataRole.DisplayRole), "b")

        QgsProject.instance().removeMapLayers([l1.id(), l2.id()])

        self.assertEqual(m.rowCount(QModelIndex()), 5)
        self.assertFalse(m.data(m.index(0, 0), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(m.data(m.index(1, 0), Qt.ItemDataRole.DisplayRole), "l5")
        self.assertEqual(m.data(m.index(2, 0), Qt.ItemDataRole.DisplayRole), "l4")
        self.assertEqual(m.data(m.index(3, 0), Qt.ItemDataRole.DisplayRole), "a")
        self.assertEqual(m.data(m.index(4, 0), Qt.ItemDataRole.DisplayRole), "b")

    def testIndexFromLayer(self):
        l1 = create_layer("l1")
        l2 = create_layer("l2")
        QgsProject.instance().addMapLayers([l1, l2])
        m = QgsMapLayerModel()
        l3 = create_layer("l3")  # not in registry

        self.assertEqual(m.indexFromLayer(l1).row(), 0)
        self.assertEqual(m.layerFromIndex(m.indexFromLayer(l1)), l1)
        self.assertEqual(m.indexFromLayer(l2).row(), 1)
        self.assertEqual(m.layerFromIndex(m.indexFromLayer(l2)), l2)
        self.assertFalse(m.indexFromLayer(l3).isValid())

        m.setAllowEmptyLayer(True)
        self.assertEqual(m.indexFromLayer(l1).row(), 1)
        self.assertEqual(m.indexFromLayer(l2).row(), 2)

        QgsProject.instance().removeMapLayers([l1.id(), l2.id()])

    def testProject(self):
        lA = create_layer("lA")
        lB = create_layer("lB")
        projectA = QgsProject.instance()
        projectB = QgsProject()

        projectA.addMapLayer(lA)
        projectB.addMapLayer(lB)

        m = QgsMapLayerModel()
        self.assertEqual(m.data(m.index(0, 0), Qt.ItemDataRole.DisplayRole), lA.name())

        m.setProject(projectB)
        self.assertEqual(m.data(m.index(0, 0), Qt.ItemDataRole.DisplayRole), lB.name())

        m.setProject(projectA)
        self.assertEqual(m.data(m.index(0, 0), Qt.ItemDataRole.DisplayRole), lA.name())

        QgsProject.instance().removeAllMapLayers()

    def testDisplayRole(self):
        l1 = create_layer("l1")
        l2 = create_layer("l2")
        QgsProject.instance().addMapLayers([l1, l2])
        m = QgsMapLayerModel()
        self.assertEqual(m.data(m.index(0, 0), Qt.ItemDataRole.DisplayRole), "l1")
        self.assertEqual(m.data(m.index(1, 0), Qt.ItemDataRole.DisplayRole), "l2")
        m.setAllowEmptyLayer(True)
        self.assertFalse(m.data(m.index(0, 0), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(m.data(m.index(1, 0), Qt.ItemDataRole.DisplayRole), "l1")
        self.assertEqual(m.data(m.index(2, 0), Qt.ItemDataRole.DisplayRole), "l2")

        QgsProject.instance().removeMapLayers([l1.id(), l2.id()])

    def testDisplayRoleShowCrs(self):
        l1 = create_layer("l1")
        l2 = create_layer("l2")
        l3 = QgsVectorLayer(
            "NoGeometry?field=fldtxt:string&field=fldint:integer", "no geom", "memory"
        )

        QgsProject.instance().addMapLayers([l1, l2, l3])
        m = QgsMapLayerModel()
        m.setShowCrs(True)
        self.assertEqual(
            m.data(m.index(0, 0), Qt.ItemDataRole.DisplayRole), "l1 [EPSG:3111]"
        )
        self.assertEqual(
            m.data(m.index(1, 0), Qt.ItemDataRole.DisplayRole), "l2 [EPSG:3111]"
        )
        self.assertEqual(m.data(m.index(2, 0), Qt.ItemDataRole.DisplayRole), "no geom")

        m.setAllowEmptyLayer(True)
        self.assertFalse(m.data(m.index(0, 0), Qt.ItemDataRole.DisplayRole))
        self.assertEqual(
            m.data(m.index(1, 0), Qt.ItemDataRole.DisplayRole), "l1 [EPSG:3111]"
        )
        self.assertEqual(
            m.data(m.index(2, 0), Qt.ItemDataRole.DisplayRole), "l2 [EPSG:3111]"
        )
        self.assertEqual(m.data(m.index(3, 0), Qt.ItemDataRole.DisplayRole), "no geom")

        m.setAdditionalItems(["a"])
        self.assertEqual(m.data(m.index(4, 0), Qt.ItemDataRole.DisplayRole), "a")

        QgsProject.instance().removeMapLayers([l1.id(), l2.id(), l3.id()])

    def testLayerIdRole(self):
        l1 = create_layer("l1")
        l2 = create_layer("l2")
        QgsProject.instance().addMapLayers([l1, l2])
        m = QgsMapLayerModel()
        self.assertEqual(
            m.data(m.index(0, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole), l1.id()
        )
        self.assertEqual(
            m.data(m.index(1, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole), l2.id()
        )
        m.setAllowEmptyLayer(True)
        self.assertFalse(
            m.data(m.index(0, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole)
        )
        self.assertEqual(
            m.data(m.index(1, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole), l1.id()
        )
        self.assertEqual(
            m.data(m.index(2, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole), l2.id()
        )

        m.setAdditionalItems(["a"])
        self.assertFalse(
            m.data(m.index(3, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole)
        )

        QgsProject.instance().removeMapLayers([l1.id(), l2.id()])

    def testLayerRole(self):
        l1 = create_layer("l1")
        l2 = create_layer("l2")
        QgsProject.instance().addMapLayers([l1, l2])
        m = QgsMapLayerModel()
        self.assertEqual(
            m.data(m.index(0, 0), QgsMapLayerModel.ItemDataRole.LayerRole), l1
        )
        self.assertEqual(
            m.data(m.index(1, 0), QgsMapLayerModel.ItemDataRole.LayerRole), l2
        )
        m.setAllowEmptyLayer(True)
        self.assertFalse(m.data(m.index(0, 0), QgsMapLayerModel.ItemDataRole.LayerRole))
        self.assertEqual(
            m.data(m.index(1, 0), QgsMapLayerModel.ItemDataRole.LayerRole), l1
        )
        self.assertEqual(
            m.data(m.index(2, 0), QgsMapLayerModel.ItemDataRole.LayerRole), l2
        )

        m.setAdditionalItems(["a"])
        self.assertFalse(m.data(m.index(3, 0), QgsMapLayerModel.ItemDataRole.LayerRole))

        QgsProject.instance().removeMapLayers([l1.id(), l2.id()])

    def testIsEmptyRole(self):
        l1 = create_layer("l1")
        l2 = create_layer("l2")
        QgsProject.instance().addMapLayers([l1, l2])
        m = QgsMapLayerModel()
        self.assertFalse(m.data(m.index(0, 0), QgsMapLayerModel.ItemDataRole.EmptyRole))
        self.assertFalse(m.data(m.index(1, 0), QgsMapLayerModel.ItemDataRole.EmptyRole))
        m.setAllowEmptyLayer(True)
        self.assertTrue(m.data(m.index(0, 0), QgsMapLayerModel.ItemDataRole.EmptyRole))
        self.assertFalse(m.data(m.index(1, 0), QgsMapLayerModel.ItemDataRole.EmptyRole))
        self.assertFalse(m.data(m.index(2, 0), QgsMapLayerModel.ItemDataRole.EmptyRole))

        m.setAdditionalItems(["a"])
        self.assertFalse(m.data(m.index(3, 0), QgsMapLayerModel.ItemDataRole.EmptyRole))

        QgsProject.instance().removeMapLayers([l1.id(), l2.id()])

    def testIsAdditionalRole(self):
        l1 = create_layer("l1")
        l2 = create_layer("l2")
        QgsProject.instance().addMapLayers([l1, l2])
        m = QgsMapLayerModel()
        self.assertFalse(
            m.data(m.index(0, 0), QgsMapLayerModel.ItemDataRole.AdditionalRole)
        )
        self.assertFalse(
            m.data(m.index(1, 0), QgsMapLayerModel.ItemDataRole.AdditionalRole)
        )
        m.setAllowEmptyLayer(True)
        self.assertFalse(
            m.data(m.index(0, 0), QgsMapLayerModel.ItemDataRole.AdditionalRole)
        )
        self.assertFalse(
            m.data(m.index(1, 0), QgsMapLayerModel.ItemDataRole.AdditionalRole)
        )
        self.assertFalse(
            m.data(m.index(2, 0), QgsMapLayerModel.ItemDataRole.AdditionalRole)
        )

        m.setAdditionalItems(["a"])
        self.assertFalse(
            m.data(m.index(0, 0), QgsMapLayerModel.ItemDataRole.AdditionalRole)
        )
        self.assertFalse(
            m.data(m.index(1, 0), QgsMapLayerModel.ItemDataRole.AdditionalRole)
        )
        self.assertFalse(
            m.data(m.index(2, 0), QgsMapLayerModel.ItemDataRole.AdditionalRole)
        )
        self.assertTrue(
            m.data(m.index(3, 0), QgsMapLayerModel.ItemDataRole.AdditionalRole)
        )

        QgsProject.instance().removeMapLayers([l1.id(), l2.id()])

    def testCheckStateRole(self):
        l1 = create_layer("l1")
        l2 = create_layer("l2")
        QgsProject.instance().addMapLayers([l1, l2])
        m = QgsMapLayerModel()

        # not checkable
        self.assertFalse(m.data(m.index(0, 0), Qt.ItemDataRole.CheckStateRole))
        self.assertFalse(m.data(m.index(1, 0), Qt.ItemDataRole.CheckStateRole))
        m.setAllowEmptyLayer(True)
        self.assertFalse(m.data(m.index(0, 0), Qt.ItemDataRole.CheckStateRole))
        self.assertFalse(m.data(m.index(1, 0), Qt.ItemDataRole.CheckStateRole))
        self.assertFalse(m.data(m.index(2, 0), Qt.ItemDataRole.CheckStateRole))
        m.setAllowEmptyLayer(False)

        # checkable
        m.setItemsCheckable(True)
        m.checkAll(Qt.CheckState.Checked)
        self.assertTrue(m.data(m.index(0, 0), Qt.ItemDataRole.CheckStateRole))
        self.assertTrue(m.data(m.index(1, 0), Qt.ItemDataRole.CheckStateRole))
        m.setAllowEmptyLayer(True)
        self.assertFalse(m.data(m.index(0, 0), Qt.ItemDataRole.CheckStateRole))
        self.assertTrue(m.data(m.index(1, 0), Qt.ItemDataRole.CheckStateRole))
        self.assertTrue(m.data(m.index(2, 0), Qt.ItemDataRole.CheckStateRole))

        m.setAdditionalItems(["a"])
        self.assertFalse(m.data(m.index(3, 0), Qt.ItemDataRole.CheckStateRole))

        QgsProject.instance().removeMapLayers([l1.id(), l2.id()])

    def testFlags(self):
        l1 = create_layer("l1")
        l2 = create_layer("l2")
        QgsProject.instance().addMapLayers([l1, l2])
        m = QgsMapLayerModel()

        self.assertFalse(m.flags(QModelIndex()) & Qt.ItemFlag.ItemIsDropEnabled)

        # not checkable
        self.assertFalse(m.flags(m.index(0, 0)) & Qt.ItemFlag.ItemIsUserCheckable)
        self.assertFalse(m.flags(m.index(0, 0)) & Qt.ItemFlag.ItemIsDragEnabled)
        self.assertFalse(m.flags(m.index(1, 0)) & Qt.ItemFlag.ItemIsUserCheckable)
        self.assertFalse(m.flags(m.index(1, 0)) & Qt.ItemFlag.ItemIsDragEnabled)
        m.setAllowEmptyLayer(True)
        self.assertFalse(m.flags(m.index(0, 0)) & Qt.ItemFlag.ItemIsUserCheckable)
        self.assertFalse(m.flags(m.index(0, 0)) & Qt.ItemFlag.ItemIsDragEnabled)
        self.assertFalse(m.flags(m.index(1, 0)) & Qt.ItemFlag.ItemIsUserCheckable)
        self.assertFalse(m.flags(m.index(1, 0)) & Qt.ItemFlag.ItemIsDragEnabled)
        self.assertFalse(m.flags(m.index(2, 0)) & Qt.ItemFlag.ItemIsUserCheckable)
        self.assertFalse(m.flags(m.index(2, 0)) & Qt.ItemFlag.ItemIsDragEnabled)
        m.setAllowEmptyLayer(False)

        # checkable
        m.setItemsCheckable(True)
        self.assertTrue(m.flags(m.index(0, 0)) & Qt.ItemFlag.ItemIsUserCheckable)
        self.assertFalse(m.flags(m.index(0, 0)) & Qt.ItemFlag.ItemIsDragEnabled)
        self.assertTrue(m.flags(m.index(1, 0)) & Qt.ItemFlag.ItemIsUserCheckable)
        self.assertFalse(m.flags(m.index(1, 0)) & Qt.ItemFlag.ItemIsDragEnabled)
        m.setAllowEmptyLayer(True)
        self.assertFalse(m.flags(m.index(0, 0)) & Qt.ItemFlag.ItemIsUserCheckable)
        self.assertFalse(m.flags(m.index(0, 0)) & Qt.ItemFlag.ItemIsDragEnabled)
        self.assertTrue(m.flags(m.index(1, 0)) & Qt.ItemFlag.ItemIsUserCheckable)
        self.assertFalse(m.flags(m.index(1, 0)) & Qt.ItemFlag.ItemIsDragEnabled)
        self.assertTrue(m.flags(m.index(2, 0)) & Qt.ItemFlag.ItemIsUserCheckable)
        self.assertFalse(m.flags(m.index(2, 0)) & Qt.ItemFlag.ItemIsDragEnabled)

        m.setAdditionalItems(["a"])
        self.assertFalse(m.flags(m.index(3, 0)) & Qt.ItemFlag.ItemIsUserCheckable)
        self.assertFalse(m.flags(m.index(3, 0)) & Qt.ItemFlag.ItemIsDragEnabled)

        m.setAdditionalItems([])
        m.setAllowEmptyLayer(False)
        m.setItemsCheckable(False)

        m.setItemsCanBeReordered(True)
        m.setItemsCheckable(False)
        self.assertFalse(m.flags(m.index(0, 0)) & Qt.ItemFlag.ItemIsUserCheckable)
        self.assertTrue(m.flags(m.index(0, 0)) & Qt.ItemFlag.ItemIsDragEnabled)
        self.assertFalse(m.flags(m.index(1, 0)) & Qt.ItemFlag.ItemIsUserCheckable)
        self.assertTrue(m.flags(m.index(1, 0)) & Qt.ItemFlag.ItemIsDragEnabled)
        m.setAllowEmptyLayer(True)
        self.assertFalse(m.flags(m.index(0, 0)) & Qt.ItemFlag.ItemIsUserCheckable)
        self.assertFalse(m.flags(m.index(0, 0)) & Qt.ItemFlag.ItemIsDragEnabled)
        self.assertFalse(m.flags(m.index(1, 0)) & Qt.ItemFlag.ItemIsUserCheckable)
        self.assertTrue(m.flags(m.index(1, 0)) & Qt.ItemFlag.ItemIsDragEnabled)
        self.assertFalse(m.flags(m.index(2, 0)) & Qt.ItemFlag.ItemIsUserCheckable)
        self.assertTrue(m.flags(m.index(2, 0)) & Qt.ItemFlag.ItemIsDragEnabled)

        m.setAdditionalItems(["a"])
        self.assertFalse(m.flags(m.index(3, 0)) & Qt.ItemFlag.ItemIsUserCheckable)
        self.assertFalse(m.flags(m.index(3, 0)) & Qt.ItemFlag.ItemIsDragEnabled)

        self.assertTrue(m.flags(QModelIndex()) & Qt.ItemFlag.ItemIsDropEnabled)

        QgsProject.instance().removeMapLayers([l1.id(), l2.id()])

    def testSetData(self):
        l1 = create_layer("l1")
        l2 = create_layer("l2")
        QgsProject.instance().addMapLayers([l1, l2])
        m = QgsMapLayerModel()

        # set checked
        m.setItemsCheckable(True)
        self.assertTrue(m.setData(m.index(0, 0), True, Qt.ItemDataRole.CheckStateRole))
        self.assertTrue(m.data(m.index(0, 0), Qt.ItemDataRole.CheckStateRole))
        self.assertFalse(m.data(m.index(1, 0), Qt.ItemDataRole.CheckStateRole))
        self.assertTrue(m.setData(m.index(1, 0), True, Qt.ItemDataRole.CheckStateRole))
        self.assertTrue(m.data(m.index(0, 0), Qt.ItemDataRole.CheckStateRole))
        self.assertTrue(m.data(m.index(1, 0), Qt.ItemDataRole.CheckStateRole))

        m.setAllowEmptyLayer(True)
        self.assertFalse(m.setData(m.index(0, 0), True, Qt.ItemDataRole.CheckStateRole))
        self.assertTrue(m.setData(m.index(1, 0), True, Qt.ItemDataRole.CheckStateRole))

        m.setAdditionalItems(["a"])
        self.assertFalse(m.setData(m.index(3, 0), True, Qt.ItemDataRole.CheckStateRole))

        QgsProject.instance().removeMapLayers([l1.id(), l2.id()])

    def testSetDataId(self):
        l1 = create_layer("l1")
        l2 = create_layer("l2")
        QgsProject.instance().addMapLayers([l1, l2])
        m = QgsMapLayerModel()
        self.assertEqual(
            m.data(m.index(0, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole), l1.id()
        )
        self.assertEqual(
            m.data(m.index(1, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole), l2.id()
        )

        self.assertTrue(
            m.setData(m.index(0, 0), l2.id(), QgsMapLayerModel.ItemDataRole.LayerIdRole)
        )
        self.assertEqual(
            m.data(m.index(0, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole), l2.id()
        )
        self.assertEqual(
            m.data(m.index(0, 0), QgsMapLayerModel.ItemDataRole.LayerRole), l2
        )
        self.assertEqual(
            m.data(m.index(1, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole), l2.id()
        )
        self.assertEqual(
            m.data(m.index(1, 0), QgsMapLayerModel.ItemDataRole.LayerRole), l2
        )

        self.assertTrue(
            m.setData(m.index(1, 0), l1.id(), QgsMapLayerModel.ItemDataRole.LayerIdRole)
        )
        self.assertEqual(
            m.data(m.index(0, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole), l2.id()
        )
        self.assertEqual(
            m.data(m.index(0, 0), QgsMapLayerModel.ItemDataRole.LayerRole), l2
        )
        self.assertEqual(
            m.data(m.index(1, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole), l1.id()
        )
        self.assertEqual(
            m.data(m.index(1, 0), QgsMapLayerModel.ItemDataRole.LayerRole), l1
        )

        m.setAllowEmptyLayer(True)
        self.assertFalse(
            m.data(m.index(0, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole)
        )
        self.assertFalse(m.data(m.index(0, 0), QgsMapLayerModel.ItemDataRole.LayerRole))
        self.assertEqual(
            m.data(m.index(1, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole), l2.id()
        )
        self.assertEqual(
            m.data(m.index(1, 0), QgsMapLayerModel.ItemDataRole.LayerRole), l2
        )
        self.assertEqual(
            m.data(m.index(2, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole), l1.id()
        )
        self.assertEqual(
            m.data(m.index(2, 0), QgsMapLayerModel.ItemDataRole.LayerRole), l1
        )
        self.assertTrue(
            m.setData(m.index(1, 0), l1.id(), QgsMapLayerModel.ItemDataRole.LayerIdRole)
        )
        self.assertFalse(
            m.data(m.index(0, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole)
        )
        self.assertFalse(m.data(m.index(0, 0), QgsMapLayerModel.ItemDataRole.LayerRole))
        self.assertEqual(
            m.data(m.index(1, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole), l1.id()
        )
        self.assertEqual(
            m.data(m.index(1, 0), QgsMapLayerModel.ItemDataRole.LayerRole), l1
        )
        self.assertEqual(
            m.data(m.index(2, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole), l1.id()
        )
        self.assertEqual(
            m.data(m.index(2, 0), QgsMapLayerModel.ItemDataRole.LayerRole), l1
        )
        self.assertTrue(
            m.setData(m.index(2, 0), l2.id(), QgsMapLayerModel.ItemDataRole.LayerIdRole)
        )
        self.assertFalse(
            m.data(m.index(0, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole)
        )
        self.assertFalse(m.data(m.index(0, 0), QgsMapLayerModel.ItemDataRole.LayerRole))
        self.assertEqual(
            m.data(m.index(1, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole), l1.id()
        )
        self.assertEqual(
            m.data(m.index(1, 0), QgsMapLayerModel.ItemDataRole.LayerRole), l1
        )
        self.assertEqual(
            m.data(m.index(2, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole), l2.id()
        )
        self.assertEqual(
            m.data(m.index(2, 0), QgsMapLayerModel.ItemDataRole.LayerRole), l2
        )

        m.setAdditionalItems(["a"])
        self.assertFalse(
            m.setData(m.index(3, 0), True, QgsMapLayerModel.ItemDataRole.LayerRole)
        )

        QgsProject.instance().removeMapLayers([l1.id(), l2.id()])

    def testInsertRows(self):
        l1 = create_layer("l1")
        l2 = create_layer("l2")
        QgsProject.instance().addMapLayers([l1, l2])
        m = QgsMapLayerModel()
        self.assertEqual(m.rowCount(), 2)
        self.assertEqual(
            m.data(m.index(0, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole), l1.id()
        )
        self.assertEqual(
            m.data(m.index(1, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole), l2.id()
        )

        self.assertTrue(m.insertRows(0, 2))
        self.assertEqual(m.rowCount(), 4)
        self.assertFalse(
            m.data(m.index(0, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole)
        )
        self.assertFalse(
            m.data(m.index(1, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole)
        )
        self.assertEqual(
            m.data(m.index(2, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole), l1.id()
        )
        self.assertEqual(
            m.data(m.index(3, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole), l2.id()
        )

        self.assertTrue(m.insertRows(3, 1))
        self.assertEqual(m.rowCount(), 5)
        self.assertFalse(
            m.data(m.index(0, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole)
        )
        self.assertFalse(
            m.data(m.index(1, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole)
        )
        self.assertEqual(
            m.data(m.index(2, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole), l1.id()
        )
        self.assertFalse(
            m.data(m.index(3, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole)
        )
        self.assertEqual(
            m.data(m.index(4, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole), l2.id()
        )

        self.assertTrue(m.insertRows(5, 2))
        self.assertEqual(m.rowCount(), 7)
        self.assertFalse(
            m.data(m.index(0, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole)
        )
        self.assertFalse(
            m.data(m.index(1, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole)
        )
        self.assertEqual(
            m.data(m.index(2, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole), l1.id()
        )
        self.assertFalse(
            m.data(m.index(3, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole)
        )
        self.assertEqual(
            m.data(m.index(4, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole), l2.id()
        )
        self.assertFalse(
            m.data(m.index(5, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole)
        )
        self.assertFalse(
            m.data(m.index(6, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole)
        )

        m = QgsMapLayerModel()
        m.setAllowEmptyLayer(True)
        self.assertEqual(m.rowCount(), 3)
        self.assertFalse(
            m.data(m.index(0, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole)
        )
        self.assertEqual(
            m.data(m.index(1, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole), l1.id()
        )
        self.assertEqual(
            m.data(m.index(2, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole), l2.id()
        )

        self.assertTrue(m.insertRows(2, 2))
        self.assertEqual(m.rowCount(), 5)
        self.assertFalse(
            m.data(m.index(0, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole)
        )
        self.assertEqual(
            m.data(m.index(1, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole), l1.id()
        )
        self.assertFalse(
            m.data(m.index(2, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole)
        )
        self.assertFalse(
            m.data(m.index(3, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole)
        )
        self.assertEqual(
            m.data(m.index(4, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole), l2.id()
        )

        QgsProject.instance().removeMapLayers([l1.id(), l2.id()])

    def testRemoveRows(self):
        l1 = create_layer("l1")
        l2 = create_layer("l2")
        QgsProject.instance().addMapLayers([l1, l2])
        m = QgsMapLayerModel()
        self.assertEqual(m.rowCount(), 2)
        self.assertEqual(
            m.data(m.index(0, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole), l1.id()
        )
        self.assertEqual(
            m.data(m.index(1, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole), l2.id()
        )

        self.assertTrue(m.removeRows(0, 1))
        self.assertEqual(m.rowCount(), 1)
        self.assertEqual(
            m.data(m.index(0, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole), l2.id()
        )

        self.assertTrue(m.removeRows(0, 1))
        self.assertEqual(m.rowCount(), 0)
        self.assertFalse(m.removeRows(0, 1))
        self.assertFalse(m.removeRows(-1, 1))

        m = QgsMapLayerModel()
        self.assertEqual(m.rowCount(), 2)
        self.assertTrue(m.removeRows(0, 2))
        self.assertEqual(m.rowCount(), 0)

        m = QgsMapLayerModel()
        m.setAllowEmptyLayer(True)
        self.assertEqual(m.rowCount(), 3)
        self.assertFalse(m.removeRows(0, 2))
        self.assertEqual(m.rowCount(), 3)
        self.assertTrue(m.removeRows(2, 1))
        self.assertEqual(m.rowCount(), 2)
        self.assertFalse(
            m.data(m.index(0, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole)
        )
        self.assertEqual(
            m.data(m.index(1, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole), l1.id()
        )
        self.assertFalse(m.removeRows(2, 1))
        self.assertEqual(m.rowCount(), 2)
        self.assertTrue(m.removeRows(1, 1))
        self.assertEqual(m.rowCount(), 1)
        self.assertFalse(
            m.data(m.index(0, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole)
        )
        m = QgsMapLayerModel()
        m.setAllowEmptyLayer(True)
        self.assertEqual(m.rowCount(), 3)
        self.assertFalse(m.removeRows(3, 2))
        self.assertTrue(m.removeRows(1, 2))
        self.assertEqual(m.rowCount(), 1)
        self.assertFalse(
            m.data(m.index(0, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole)
        )
        self.assertFalse(m.removeRows(1, 1))
        self.assertFalse(m.removeRows(0, 1))

        QgsProject.instance().removeMapLayers([l1.id(), l2.id()])

    def testMime(self):
        l1 = create_layer("l1")
        l2 = create_layer("l2")
        QgsProject.instance().addMapLayers([l1, l2])
        m = QgsMapLayerModel()
        self.assertEqual(m.mimeTypes(), ["application/qgis.layermodeldata"])

        data = m.mimeData([m.index(0, 0)])
        self.assertTrue(data)

        self.assertFalse(
            m.canDropMimeData(data, Qt.DropAction.MoveAction, 0, 0, QModelIndex())
        )
        m.setItemsCanBeReordered(True)
        self.assertTrue(
            m.canDropMimeData(data, Qt.DropAction.MoveAction, 0, 0, QModelIndex())
        )

        self.assertTrue(
            m.dropMimeData(data, Qt.DropAction.MoveAction, 2, 0, QModelIndex())
        )
        self.assertEqual(m.rowCount(), 3)
        self.assertEqual(
            m.data(m.index(0, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole), l1.id()
        )
        self.assertEqual(
            m.data(m.index(1, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole), l2.id()
        )
        self.assertEqual(
            m.data(m.index(2, 0), QgsMapLayerModel.ItemDataRole.LayerIdRole), l1.id()
        )

        QgsProject.instance().removeMapLayers([l1.id(), l2.id()])


if __name__ == "__main__":
    unittest.main()
