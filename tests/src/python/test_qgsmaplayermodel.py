# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsMapLayerModel

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '16/11/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.core import QgsVectorLayer, QgsProject, QgsMapLayerModel
from qgis.PyQt.QtCore import Qt, QModelIndex

from qgis.testing import start_app, unittest

start_app()


def create_layer(name):
    layer = QgsVectorLayer("Point?crs=EPSG:3111&field=fldtxt:string&field=fldint:integer",
                           name, "memory")
    return layer


class TestQgsMapLayerModel(unittest.TestCase):

    def testGettersSetters(self):
        """ test model getters/setters """
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

        m.setAdditionalItems(['a', 'b'])
        self.assertEqual(m.additionalItems(), ['a', 'b'])
        m.setAdditionalItems([])
        self.assertFalse(m.additionalItems())

    def testAddingRemovingLayers(self):
        # test model handles layer addition and removal
        m = QgsMapLayerModel()

        self.assertEqual(m.rowCount(QModelIndex()), 0)

        l1 = create_layer('l1')
        QgsProject.instance().addMapLayer(l1)
        self.assertEqual(m.rowCount(QModelIndex()), 1)
        self.assertEqual(m.layerFromIndex(m.index(0, 0)), l1)
        l2 = create_layer('l2')
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
        l1 = create_layer('l1')
        l2 = create_layer('l2')
        QgsProject.instance().addMapLayers([l1, l2])
        m = QgsMapLayerModel()
        self.assertEqual(m.rowCount(QModelIndex()), 2)
        QgsProject.instance().removeMapLayers([l1.id(), l2.id()])
        self.assertEqual(m.rowCount(QModelIndex()), 0)

    def testCheckAll(self):
        l1 = create_layer('l1')
        l2 = create_layer('l2')
        QgsProject.instance().addMapLayers([l1, l2])
        m = QgsMapLayerModel()
        m.setItemsCheckable(True)
        self.assertFalse(m.layersChecked())
        self.assertEqual(set(m.layersChecked(Qt.Unchecked)), set([l1, l2]))

        m.checkAll(Qt.Checked)
        self.assertEqual(set(m.layersChecked()), set([l1, l2]))
        self.assertFalse(set(m.layersChecked(Qt.Unchecked)))

        m.checkAll(Qt.Unchecked)
        self.assertFalse(m.layersChecked())
        self.assertEqual(set(m.layersChecked(Qt.Unchecked)), set([l1, l2]))

        QgsProject.instance().removeMapLayers([l1.id(), l2.id()])

    def testAllowEmpty(self):
        l1 = create_layer('l1')
        l2 = create_layer('l2')
        QgsProject.instance().addMapLayers([l1, l2])
        m = QgsMapLayerModel()
        self.assertEqual(m.rowCount(QModelIndex()), 2)

        m.setAllowEmptyLayer(True)
        self.assertEqual(m.rowCount(QModelIndex()), 3)
        self.assertFalse(m.data(m.index(0, 0), Qt.DisplayRole))
        m.setAllowEmptyLayer(False)
        self.assertEqual(m.rowCount(QModelIndex()), 2)
        self.assertTrue(m.data(m.index(0, 0), Qt.DisplayRole))

        # add layers after allow empty is true
        m.setAllowEmptyLayer(True)
        l3 = create_layer('l3')
        QgsProject.instance().addMapLayers([l3])
        self.assertEqual(m.rowCount(QModelIndex()), 4)
        self.assertFalse(m.data(m.index(0, 0), Qt.DisplayRole))
        self.assertEqual(m.data(m.index(1, 0), Qt.DisplayRole), 'l1')
        self.assertEqual(m.data(m.index(2, 0), Qt.DisplayRole), 'l2')
        self.assertEqual(m.data(m.index(3, 0), Qt.DisplayRole), 'l3')

        QgsProject.instance().removeMapLayers([l1.id(), l2.id(), l3.id()])

    def testAdditionalItems(self):
        l1 = create_layer('l1')
        l2 = create_layer('l2')
        QgsProject.instance().addMapLayers([l1, l2])
        m = QgsMapLayerModel()
        self.assertEqual(m.rowCount(QModelIndex()), 2)

        m.setAdditionalItems(['a', 'b'])
        self.assertEqual(m.rowCount(QModelIndex()), 4)
        self.assertEqual(m.data(m.index(0, 0), Qt.DisplayRole), 'l1')
        self.assertEqual(m.data(m.index(1, 0), Qt.DisplayRole), 'l2')
        self.assertEqual(m.data(m.index(2, 0), Qt.DisplayRole), 'a')
        self.assertEqual(m.data(m.index(3, 0), Qt.DisplayRole), 'b')

        m.setAllowEmptyLayer(True)
        self.assertEqual(m.rowCount(QModelIndex()), 5)
        self.assertFalse(m.data(m.index(0, 0), Qt.DisplayRole))
        self.assertEqual(m.data(m.index(1, 0), Qt.DisplayRole), 'l1')
        self.assertEqual(m.data(m.index(2, 0), Qt.DisplayRole), 'l2')
        self.assertEqual(m.data(m.index(3, 0), Qt.DisplayRole), 'a')
        self.assertEqual(m.data(m.index(4, 0), Qt.DisplayRole), 'b')

        QgsProject.instance().removeMapLayers([l1.id(), l2.id()])

        self.assertEqual(m.rowCount(QModelIndex()), 3)
        self.assertFalse(m.data(m.index(0, 0), Qt.DisplayRole))
        self.assertEqual(m.data(m.index(1, 0), Qt.DisplayRole), 'a')
        self.assertEqual(m.data(m.index(2, 0), Qt.DisplayRole), 'b')

    def testIndexFromLayer(self):
        l1 = create_layer('l1')
        l2 = create_layer('l2')
        QgsProject.instance().addMapLayers([l1, l2])
        m = QgsMapLayerModel()
        l3 = create_layer('l3')  # not in registry

        self.assertEqual(m.indexFromLayer(l1).row(), 0)
        self.assertEqual(m.layerFromIndex(m.indexFromLayer(l1)), l1)
        self.assertEqual(m.indexFromLayer(l2).row(), 1)
        self.assertEqual(m.layerFromIndex(m.indexFromLayer(l2)), l2)
        self.assertFalse(m.indexFromLayer(l3).isValid())

        m.setAllowEmptyLayer(True)
        self.assertEqual(m.indexFromLayer(l1).row(), 1)
        self.assertEqual(m.indexFromLayer(l2).row(), 2)

        QgsProject.instance().removeMapLayers([l1.id(), l2.id()])

    def testDisplayRole(self):
        l1 = create_layer('l1')
        l2 = create_layer('l2')
        QgsProject.instance().addMapLayers([l1, l2])
        m = QgsMapLayerModel()
        self.assertEqual(m.data(m.index(0, 0), Qt.DisplayRole), 'l1')
        self.assertEqual(m.data(m.index(1, 0), Qt.DisplayRole), 'l2')
        m.setAllowEmptyLayer(True)
        self.assertFalse(m.data(m.index(0, 0), Qt.DisplayRole))
        self.assertEqual(m.data(m.index(1, 0), Qt.DisplayRole), 'l1')
        self.assertEqual(m.data(m.index(2, 0), Qt.DisplayRole), 'l2')

        QgsProject.instance().removeMapLayers([l1.id(), l2.id()])

    def testDisplayRoleShowCrs(self):
        l1 = create_layer('l1')
        l2 = create_layer('l2')
        QgsProject.instance().addMapLayers([l1, l2])
        m = QgsMapLayerModel()
        m.setShowCrs(True)
        self.assertEqual(m.data(m.index(0, 0), Qt.DisplayRole), 'l1 [EPSG:3111]')
        self.assertEqual(m.data(m.index(1, 0), Qt.DisplayRole), 'l2 [EPSG:3111]')
        m.setAllowEmptyLayer(True)
        self.assertFalse(m.data(m.index(0, 0), Qt.DisplayRole))
        self.assertEqual(m.data(m.index(1, 0), Qt.DisplayRole), 'l1 [EPSG:3111]')
        self.assertEqual(m.data(m.index(2, 0), Qt.DisplayRole), 'l2 [EPSG:3111]')

        m.setAdditionalItems(['a'])
        self.assertEqual(m.data(m.index(3, 0), Qt.DisplayRole), 'a')

        QgsProject.instance().removeMapLayers([l1.id(), l2.id()])

    def testLayerIdRole(self):
        l1 = create_layer('l1')
        l2 = create_layer('l2')
        QgsProject.instance().addMapLayers([l1, l2])
        m = QgsMapLayerModel()
        self.assertEqual(m.data(m.index(0, 0), QgsMapLayerModel.LayerIdRole), l1.id())
        self.assertEqual(m.data(m.index(1, 0), QgsMapLayerModel.LayerIdRole), l2.id())
        m.setAllowEmptyLayer(True)
        self.assertFalse(m.data(m.index(0, 0), QgsMapLayerModel.LayerIdRole))
        self.assertEqual(m.data(m.index(1, 0), QgsMapLayerModel.LayerIdRole), l1.id())
        self.assertEqual(m.data(m.index(2, 0), QgsMapLayerModel.LayerIdRole), l2.id())

        m.setAdditionalItems(['a'])
        self.assertFalse(m.data(m.index(3, 0), QgsMapLayerModel.LayerIdRole))

        QgsProject.instance().removeMapLayers([l1.id(), l2.id()])

    def testLayerRole(self):
        l1 = create_layer('l1')
        l2 = create_layer('l2')
        QgsProject.instance().addMapLayers([l1, l2])
        m = QgsMapLayerModel()
        self.assertEqual(m.data(m.index(0, 0), QgsMapLayerModel.LayerRole), l1)
        self.assertEqual(m.data(m.index(1, 0), QgsMapLayerModel.LayerRole), l2)
        m.setAllowEmptyLayer(True)
        self.assertFalse(m.data(m.index(0, 0), QgsMapLayerModel.LayerRole))
        self.assertEqual(m.data(m.index(1, 0), QgsMapLayerModel.LayerRole), l1)
        self.assertEqual(m.data(m.index(2, 0), QgsMapLayerModel.LayerRole), l2)

        m.setAdditionalItems(['a'])
        self.assertFalse(m.data(m.index(3, 0), QgsMapLayerModel.LayerRole))

        QgsProject.instance().removeMapLayers([l1.id(), l2.id()])

    def testIsEmptyRole(self):
        l1 = create_layer('l1')
        l2 = create_layer('l2')
        QgsProject.instance().addMapLayers([l1, l2])
        m = QgsMapLayerModel()
        self.assertFalse(m.data(m.index(0, 0), QgsMapLayerModel.EmptyRole))
        self.assertFalse(m.data(m.index(1, 0), QgsMapLayerModel.EmptyRole))
        m.setAllowEmptyLayer(True)
        self.assertTrue(m.data(m.index(0, 0), QgsMapLayerModel.EmptyRole))
        self.assertFalse(m.data(m.index(1, 0), QgsMapLayerModel.EmptyRole))
        self.assertFalse(m.data(m.index(2, 0), QgsMapLayerModel.EmptyRole))

        m.setAdditionalItems(['a'])
        self.assertFalse(m.data(m.index(3, 0), QgsMapLayerModel.EmptyRole))

        QgsProject.instance().removeMapLayers([l1.id(), l2.id()])

    def testIsAdditionalRole(self):
        l1 = create_layer('l1')
        l2 = create_layer('l2')
        QgsProject.instance().addMapLayers([l1, l2])
        m = QgsMapLayerModel()
        self.assertFalse(m.data(m.index(0, 0), QgsMapLayerModel.AdditionalRole))
        self.assertFalse(m.data(m.index(1, 0), QgsMapLayerModel.AdditionalRole))
        m.setAllowEmptyLayer(True)
        self.assertFalse(m.data(m.index(0, 0), QgsMapLayerModel.AdditionalRole))
        self.assertFalse(m.data(m.index(1, 0), QgsMapLayerModel.AdditionalRole))
        self.assertFalse(m.data(m.index(2, 0), QgsMapLayerModel.AdditionalRole))

        m.setAdditionalItems(['a'])
        self.assertFalse(m.data(m.index(0, 0), QgsMapLayerModel.AdditionalRole))
        self.assertFalse(m.data(m.index(1, 0), QgsMapLayerModel.AdditionalRole))
        self.assertFalse(m.data(m.index(2, 0), QgsMapLayerModel.AdditionalRole))
        self.assertTrue(m.data(m.index(3, 0), QgsMapLayerModel.AdditionalRole))

        QgsProject.instance().removeMapLayers([l1.id(), l2.id()])

    def testCheckStateRole(self):
        l1 = create_layer('l1')
        l2 = create_layer('l2')
        QgsProject.instance().addMapLayers([l1, l2])
        m = QgsMapLayerModel()

        # not checkable
        self.assertFalse(m.data(m.index(0, 0), Qt.CheckStateRole))
        self.assertFalse(m.data(m.index(1, 0), Qt.CheckStateRole))
        m.setAllowEmptyLayer(True)
        self.assertFalse(m.data(m.index(0, 0), Qt.CheckStateRole))
        self.assertFalse(m.data(m.index(1, 0), Qt.CheckStateRole))
        self.assertFalse(m.data(m.index(2, 0), Qt.CheckStateRole))
        m.setAllowEmptyLayer(False)

        # checkable
        m.setItemsCheckable(True)
        m.checkAll(Qt.Checked)
        self.assertTrue(m.data(m.index(0, 0), Qt.CheckStateRole))
        self.assertTrue(m.data(m.index(1, 0), Qt.CheckStateRole))
        m.setAllowEmptyLayer(True)
        self.assertFalse(m.data(m.index(0, 0), Qt.CheckStateRole))
        self.assertTrue(m.data(m.index(1, 0), Qt.CheckStateRole))
        self.assertTrue(m.data(m.index(2, 0), Qt.CheckStateRole))

        m.setAdditionalItems(['a'])
        self.assertFalse(m.data(m.index(3, 0), Qt.CheckStateRole))

        QgsProject.instance().removeMapLayers([l1.id(), l2.id()])

    def testFlag(self):
        l1 = create_layer('l1')
        l2 = create_layer('l2')
        QgsProject.instance().addMapLayers([l1, l2])
        m = QgsMapLayerModel()

        # not checkable
        self.assertFalse(m.flags(m.index(0, 0)) & Qt.ItemIsUserCheckable)
        self.assertFalse(m.flags(m.index(1, 0)) & Qt.ItemIsUserCheckable)
        m.setAllowEmptyLayer(True)
        self.assertFalse(m.flags(m.index(0, 0)) & Qt.ItemIsUserCheckable)
        self.assertFalse(m.flags(m.index(1, 0)) & Qt.ItemIsUserCheckable)
        self.assertFalse(m.flags(m.index(2, 0)) & Qt.ItemIsUserCheckable)
        m.setAllowEmptyLayer(False)

        # checkable
        m.setItemsCheckable(True)
        self.assertTrue(m.flags(m.index(0, 0)) & Qt.ItemIsUserCheckable)
        self.assertTrue(m.flags(m.index(1, 0)) & Qt.ItemIsUserCheckable)
        m.setAllowEmptyLayer(True)
        self.assertFalse(m.flags(m.index(0, 0)) & Qt.ItemIsUserCheckable)
        self.assertTrue(m.flags(m.index(1, 0)) & Qt.ItemIsUserCheckable)
        self.assertTrue(m.flags(m.index(2, 0)) & Qt.ItemIsUserCheckable)

        m.setAdditionalItems(['a'])
        self.assertFalse(m.flags(m.index(3, 0)) & Qt.ItemIsUserCheckable)

        QgsProject.instance().removeMapLayers([l1.id(), l2.id()])

    def testSetData(self):
        l1 = create_layer('l1')
        l2 = create_layer('l2')
        QgsProject.instance().addMapLayers([l1, l2])
        m = QgsMapLayerModel()

        # set checked
        m.setItemsCheckable(True)
        self.assertTrue(m.setData(m.index(0, 0), True, Qt.CheckStateRole))
        self.assertTrue(m.data(m.index(0, 0), Qt.CheckStateRole))
        self.assertFalse(m.data(m.index(1, 0), Qt.CheckStateRole))
        self.assertTrue(m.setData(m.index(1, 0), True, Qt.CheckStateRole))
        self.assertTrue(m.data(m.index(0, 0), Qt.CheckStateRole))
        self.assertTrue(m.data(m.index(1, 0), Qt.CheckStateRole))

        m.setAllowEmptyLayer(True)
        self.assertFalse(m.setData(m.index(0, 0), True, Qt.CheckStateRole))
        self.assertTrue(m.setData(m.index(1, 0), True, Qt.CheckStateRole))

        m.setAdditionalItems(['a'])
        self.assertFalse(m.setData(m.index(3, 0), True, Qt.CheckStateRole))

        QgsProject.instance().removeMapLayers([l1.id(), l2.id()])


if __name__ == '__main__':
    unittest.main()
