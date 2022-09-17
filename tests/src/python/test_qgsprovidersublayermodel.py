# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsProviderSublayerModel.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '05/07/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import qgis  # NOQA

from qgis.core import (
    Qgis,
    QgsMapLayerType,
    QgsWkbTypes,
    QgsProviderSublayerDetails,
    QgsProviderSublayerProxyModel,
    QgsProviderSublayerModel
)
from qgis.PyQt.QtCore import (
    Qt,
    QModelIndex
)
from qgis.testing import start_app, unittest

# Convenience instances in case you may need them
# to find the srs.db
start_app()


class TestQgsProviderSublayerModel(unittest.TestCase):

    def test_model(self):
        model = QgsProviderSublayerModel()
        self.assertEqual(model.rowCount(QModelIndex()), 0)
        self.assertEqual(model.columnCount(QModelIndex()), 2)
        self.assertEqual(model.headerData(0, Qt.Horizontal, Qt.DisplayRole), 'Item')
        self.assertEqual(model.headerData(0, Qt.Horizontal, Qt.ToolTipRole), 'Item')
        self.assertEqual(model.headerData(1, Qt.Horizontal, Qt.DisplayRole), 'Description')
        self.assertEqual(model.headerData(1, Qt.Horizontal, Qt.ToolTipRole), 'Description')

        # no crash, should return invalid results
        self.assertFalse(model.indexToSublayer(model.index(0, 0, QModelIndex())).name())
        self.assertFalse(model.indexToNonLayerItem(model.index(0, 0, QModelIndex())).name())

        layer1 = QgsProviderSublayerDetails()
        layer1.setType(QgsMapLayerType.RasterLayer)
        layer1.setName('layer 1')
        layer1.setDescription('description 1')
        layer1.setProviderKey('gdal')
        layer1.setUri('uri 1')

        model.setSublayerDetails([layer1])
        self.assertEqual(model.rowCount(QModelIndex()), 1)
        self.assertEqual(model.data(model.index(0, 0), Qt.DisplayRole), 'layer 1')
        self.assertEqual(model.data(model.index(0, 1), Qt.DisplayRole), 'description 1')
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.ProviderKey), 'gdal')
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.LayerType), QgsMapLayerType.RasterLayer)
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.Uri), 'uri 1')
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.Name), 'layer 1')
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.Description), 'description 1')
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.Flags), 0)

        self.assertEqual(model.indexToSublayer(model.index(0, 0, QModelIndex())), layer1)
        self.assertFalse(model.indexToSublayer(model.index(1, 0, QModelIndex())).name())
        self.assertFalse(model.indexToNonLayerItem(model.index(0, 0, QModelIndex())).name())

        layer2 = QgsProviderSublayerDetails()
        layer2.setType(QgsMapLayerType.VectorLayer)
        layer2.setName('layer 2')
        layer2.setDescription('description 2')
        layer2.setProviderKey('ogr')
        layer2.setUri('uri 2')
        layer2.setFeatureCount(-1)
        layer2.setWkbType(QgsWkbTypes.LineString)
        layer2.setFlags(Qgis.SublayerFlags(Qgis.SublayerFlag.SystemTable))

        model.setSublayerDetails([layer1, layer2])
        self.assertEqual(model.rowCount(QModelIndex()), 2)
        self.assertEqual(model.data(model.index(0, 0), Qt.DisplayRole), 'layer 1')
        self.assertEqual(model.data(model.index(0, 1), Qt.DisplayRole), 'description 1')
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.ProviderKey), 'gdal')
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.LayerType), QgsMapLayerType.RasterLayer)
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.Uri), 'uri 1')
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.Name), 'layer 1')
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.Description), 'description 1')
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.Flags), 0)

        self.assertEqual(model.data(model.index(1, 0), Qt.DisplayRole), 'layer 2')
        self.assertEqual(model.data(model.index(1, 1), Qt.DisplayRole), 'description 2 - LineString (Uncounted)')
        self.assertEqual(model.data(model.index(1, 0), QgsProviderSublayerModel.Role.ProviderKey), 'ogr')
        self.assertEqual(model.data(model.index(1, 0), QgsProviderSublayerModel.Role.LayerType), QgsMapLayerType.VectorLayer)
        self.assertEqual(model.data(model.index(1, 0), QgsProviderSublayerModel.Role.Uri), 'uri 2')
        self.assertEqual(model.data(model.index(1, 0), QgsProviderSublayerModel.Role.Name), 'layer 2')
        self.assertEqual(model.data(model.index(1, 0), QgsProviderSublayerModel.Role.Description), 'description 2')
        self.assertEqual(model.data(model.index(1, 0), QgsProviderSublayerModel.Role.Flags), 1)

        self.assertEqual(model.indexToSublayer(model.index(0, 0, QModelIndex())), layer1)
        self.assertEqual(model.indexToSublayer(model.index(1, 0, QModelIndex())), layer2)
        self.assertFalse(model.indexToSublayer(model.index(2, 0, QModelIndex())).name())
        self.assertFalse(model.indexToNonLayerItem(model.index(0, 0, QModelIndex())).name())

        layer3 = QgsProviderSublayerDetails()
        layer3.setType(QgsMapLayerType.VectorLayer)
        layer3.setName('layer 3')
        layer3.setProviderKey('ogr')
        layer3.setUri('uri 3')
        layer3.setFeatureCount(1001)
        layer3.setWkbType(QgsWkbTypes.Polygon)

        model.setSublayerDetails([layer1, layer2, layer3])
        self.assertEqual(model.rowCount(QModelIndex()), 3)
        self.assertEqual(model.data(model.index(0, 0), Qt.DisplayRole), 'layer 1')
        self.assertEqual(model.data(model.index(0, 1), Qt.DisplayRole), 'description 1')
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.ProviderKey), 'gdal')
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.LayerType), QgsMapLayerType.RasterLayer)
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.Uri), 'uri 1')
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.Name), 'layer 1')
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.Description), 'description 1')

        self.assertEqual(model.data(model.index(1, 0), Qt.DisplayRole), 'layer 2')
        self.assertEqual(model.data(model.index(1, 1), Qt.DisplayRole), 'description 2 - LineString (Uncounted)')
        self.assertEqual(model.data(model.index(1, 0), QgsProviderSublayerModel.Role.ProviderKey), 'ogr')
        self.assertEqual(model.data(model.index(1, 0), QgsProviderSublayerModel.Role.LayerType), QgsMapLayerType.VectorLayer)
        self.assertEqual(model.data(model.index(1, 0), QgsProviderSublayerModel.Role.Uri), 'uri 2')
        self.assertEqual(model.data(model.index(1, 0), QgsProviderSublayerModel.Role.Name), 'layer 2')
        self.assertEqual(model.data(model.index(1, 0), QgsProviderSublayerModel.Role.Description), 'description 2')

        self.assertEqual(model.data(model.index(2, 0), Qt.DisplayRole), 'layer 3')
        self.assertEqual(model.data(model.index(2, 1), Qt.DisplayRole), 'Polygon (1,001)')
        self.assertEqual(model.data(model.index(2, 0), QgsProviderSublayerModel.Role.ProviderKey), 'ogr')
        self.assertEqual(model.data(model.index(2, 0), QgsProviderSublayerModel.Role.LayerType), QgsMapLayerType.VectorLayer)
        self.assertEqual(model.data(model.index(2, 0), QgsProviderSublayerModel.Role.Uri), 'uri 3')
        self.assertEqual(model.data(model.index(2, 0), QgsProviderSublayerModel.Role.Name), 'layer 3')
        self.assertEqual(model.data(model.index(2, 0), QgsProviderSublayerModel.Role.Description), None)

        self.assertEqual(model.indexToSublayer(model.index(0, 0, QModelIndex())), layer1)
        self.assertEqual(model.indexToSublayer(model.index(1, 0, QModelIndex())), layer2)
        self.assertEqual(model.indexToSublayer(model.index(2, 0, QModelIndex())), layer3)
        self.assertFalse(model.indexToSublayer(model.index(3, 0, QModelIndex())).name())
        self.assertFalse(model.indexToNonLayerItem(model.index(0, 0, QModelIndex())).name())

        # remove a layer
        model.setSublayerDetails([layer3, layer1])
        self.assertEqual(model.rowCount(QModelIndex()), 2)
        self.assertEqual(model.data(model.index(0, 0), Qt.DisplayRole), 'layer 1')
        self.assertEqual(model.data(model.index(0, 1), Qt.DisplayRole), 'description 1')
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.ProviderKey), 'gdal')
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.LayerType), QgsMapLayerType.RasterLayer)
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.Uri), 'uri 1')
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.Name), 'layer 1')
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.Description), 'description 1')

        self.assertEqual(model.data(model.index(1, 0), Qt.DisplayRole), 'layer 3')
        self.assertEqual(model.data(model.index(1, 1), Qt.DisplayRole), 'Polygon (1,001)')
        self.assertEqual(model.data(model.index(1, 0), QgsProviderSublayerModel.Role.ProviderKey), 'ogr')
        self.assertEqual(model.data(model.index(1, 0), QgsProviderSublayerModel.Role.LayerType), QgsMapLayerType.VectorLayer)
        self.assertEqual(model.data(model.index(1, 0), QgsProviderSublayerModel.Role.Uri), 'uri 3')
        self.assertEqual(model.data(model.index(1, 0), QgsProviderSublayerModel.Role.Name), 'layer 3')
        self.assertEqual(model.data(model.index(1, 0), QgsProviderSublayerModel.Role.Description), None)

        # remove another layer
        model.setSublayerDetails([layer3])
        self.assertEqual(model.rowCount(QModelIndex()), 1)
        self.assertEqual(model.data(model.index(0, 0), Qt.DisplayRole), 'layer 3')
        self.assertEqual(model.data(model.index(0, 1), Qt.DisplayRole), 'Polygon (1,001)')
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.ProviderKey), 'ogr')
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.LayerType), QgsMapLayerType.VectorLayer)
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.Uri), 'uri 3')
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.Name), 'layer 3')
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.Description), None)

    def test_non_layer_item(self):
        item1 = QgsProviderSublayerModel.NonLayerItem()
        item1.setUri('item uri 1')
        item1.setName('item name 1')
        item1.setType('item type 1')
        item1.setDescription('item desc 1')

        item2 = QgsProviderSublayerModel.NonLayerItem(item1)
        self.assertEqual(item1, item2)
        self.assertFalse(item1 != item2)

        item2.setUri('uu')
        self.assertNotEqual(item1, item2)
        self.assertTrue(item1 != item2)

        item2 = QgsProviderSublayerModel.NonLayerItem(item1)
        item2.setName('item name 2')
        self.assertNotEqual(item1, item2)
        self.assertTrue(item1 != item2)

        item2 = QgsProviderSublayerModel.NonLayerItem(item1)
        item2.setType('item type 2')
        self.assertNotEqual(item1, item2)
        self.assertTrue(item1 != item2)

        item2 = QgsProviderSublayerModel.NonLayerItem(item1)
        item2.setDescription('item description 2')
        self.assertNotEqual(item1, item2)
        self.assertTrue(item1 != item2)

    def test_model_with_non_layer_items(self):
        model = QgsProviderSublayerModel()
        self.assertEqual(model.rowCount(QModelIndex()), 0)
        self.assertEqual(model.columnCount(QModelIndex()), 2)
        self.assertEqual(model.headerData(0, Qt.Horizontal, Qt.DisplayRole), 'Item')
        self.assertEqual(model.headerData(0, Qt.Horizontal, Qt.ToolTipRole), 'Item')
        self.assertEqual(model.headerData(1, Qt.Horizontal, Qt.DisplayRole), 'Description')
        self.assertEqual(model.headerData(1, Qt.Horizontal, Qt.ToolTipRole), 'Description')

        layer1 = QgsProviderSublayerDetails()
        layer1.setType(QgsMapLayerType.RasterLayer)
        layer1.setName('layer 1')
        layer1.setDescription('description 1')
        layer1.setProviderKey('gdal')
        layer1.setUri('uri 1')

        model.setSublayerDetails([layer1])
        self.assertEqual(model.rowCount(QModelIndex()), 1)
        self.assertEqual(model.data(model.index(0, 0), Qt.DisplayRole), 'layer 1')
        self.assertEqual(model.data(model.index(0, 1), Qt.DisplayRole), 'description 1')
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.ProviderKey), 'gdal')
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.LayerType), QgsMapLayerType.RasterLayer)
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.Uri), 'uri 1')
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.Name), 'layer 1')
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.Description), 'description 1')
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.IsNonLayerItem), False)
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.Flags), 0)

        self.assertEqual(model.indexToSublayer(model.index(0, 0, QModelIndex())), layer1)
        self.assertFalse(model.indexToSublayer(model.index(1, 0, QModelIndex())).name())
        self.assertFalse(model.indexToNonLayerItem(model.index(0, 0, QModelIndex())).name())

        item1 = QgsProviderSublayerModel.NonLayerItem()
        item1.setUri('item uri 1')
        item1.setName('item name 1')
        item1.setType('item type 1')
        item1.setDescription('item desc 1')

        model.addNonLayerItem(item1)
        self.assertEqual(model.rowCount(QModelIndex()), 2)
        self.assertEqual(model.data(model.index(0, 0), Qt.DisplayRole), 'layer 1')
        self.assertEqual(model.data(model.index(0, 1), Qt.DisplayRole), 'description 1')
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.ProviderKey), 'gdal')
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.LayerType), QgsMapLayerType.RasterLayer)
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.Uri), 'uri 1')
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.Name), 'layer 1')
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.Description), 'description 1')
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.IsNonLayerItem), False)

        self.assertEqual(model.data(model.index(1, 0), Qt.DisplayRole), 'item name 1')
        self.assertEqual(model.data(model.index(1, 1), Qt.DisplayRole), 'item desc 1')
        self.assertEqual(model.data(model.index(1, 0), QgsProviderSublayerModel.Role.Uri), 'item uri 1')
        self.assertEqual(model.data(model.index(1, 0), QgsProviderSublayerModel.Role.Name), 'item name 1')
        self.assertEqual(model.data(model.index(1, 0), QgsProviderSublayerModel.Role.Description), 'item desc 1')
        self.assertEqual(model.data(model.index(1, 0), QgsProviderSublayerModel.Role.IsNonLayerItem), True)
        self.assertEqual(model.data(model.index(1, 0), QgsProviderSublayerModel.Role.NonLayerItemType), 'item type 1')
        self.assertEqual(model.data(model.index(1, 0), QgsProviderSublayerModel.Role.Flags), None)

        self.assertEqual(model.indexToSublayer(model.index(0, 0, QModelIndex())), layer1)
        self.assertFalse(model.indexToSublayer(model.index(1, 0, QModelIndex())).name())
        self.assertFalse(model.indexToNonLayerItem(model.index(0, 0, QModelIndex())).name())
        self.assertEqual(model.indexToNonLayerItem(model.index(1, 0, QModelIndex())), item1)
        self.assertFalse(model.indexToNonLayerItem(model.index(2, 0, QModelIndex())).name())

        item2 = QgsProviderSublayerModel.NonLayerItem()
        item2.setUri('item uri 2')
        item2.setName('item name 2')
        item2.setType('item type 2')
        item2.setDescription('item desc 2')

        model.addNonLayerItem(item2)
        self.assertEqual(model.rowCount(QModelIndex()), 3)
        self.assertEqual(model.data(model.index(0, 0), Qt.DisplayRole), 'layer 1')
        self.assertEqual(model.data(model.index(0, 1), Qt.DisplayRole), 'description 1')
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.ProviderKey), 'gdal')
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.LayerType), QgsMapLayerType.RasterLayer)
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.Uri), 'uri 1')
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.Name), 'layer 1')
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.Description), 'description 1')
        self.assertEqual(model.data(model.index(0, 0), QgsProviderSublayerModel.Role.IsNonLayerItem), False)

        self.assertEqual(model.data(model.index(1, 0), Qt.DisplayRole), 'item name 1')
        self.assertEqual(model.data(model.index(1, 1), Qt.DisplayRole), 'item desc 1')
        self.assertEqual(model.data(model.index(1, 0), QgsProviderSublayerModel.Role.Uri), 'item uri 1')
        self.assertEqual(model.data(model.index(1, 0), QgsProviderSublayerModel.Role.Name), 'item name 1')
        self.assertEqual(model.data(model.index(1, 0), QgsProviderSublayerModel.Role.Description), 'item desc 1')
        self.assertEqual(model.data(model.index(1, 0), QgsProviderSublayerModel.Role.IsNonLayerItem), True)
        self.assertEqual(model.data(model.index(1, 0), QgsProviderSublayerModel.Role.NonLayerItemType), 'item type 1')

        self.assertEqual(model.data(model.index(2, 0), Qt.DisplayRole), 'item name 2')
        self.assertEqual(model.data(model.index(2, 1), Qt.DisplayRole), 'item desc 2')
        self.assertEqual(model.data(model.index(2, 0), QgsProviderSublayerModel.Role.Uri), 'item uri 2')
        self.assertEqual(model.data(model.index(2, 0), QgsProviderSublayerModel.Role.Name), 'item name 2')
        self.assertEqual(model.data(model.index(2, 0), QgsProviderSublayerModel.Role.Description), 'item desc 2')
        self.assertEqual(model.data(model.index(2, 0), QgsProviderSublayerModel.Role.IsNonLayerItem), True)
        self.assertEqual(model.data(model.index(2, 0), QgsProviderSublayerModel.Role.NonLayerItemType), 'item type 2')

        self.assertEqual(model.indexToSublayer(model.index(0, 0, QModelIndex())), layer1)
        self.assertFalse(model.indexToSublayer(model.index(1, 0, QModelIndex())).name())
        self.assertFalse(model.indexToNonLayerItem(model.index(0, 0, QModelIndex())).name())
        self.assertEqual(model.indexToNonLayerItem(model.index(1, 0, QModelIndex())), item1)
        self.assertEqual(model.indexToNonLayerItem(model.index(2, 0, QModelIndex())), item2)
        self.assertFalse(model.indexToNonLayerItem(model.index(3, 0, QModelIndex())).name())

    def test_model_with_paths(self):
        model = QgsProviderSublayerModel()

        layer1 = QgsProviderSublayerDetails()
        layer1.setType(QgsMapLayerType.RasterLayer)
        layer1.setName('layer 1')
        layer1.setDescription('description 1')
        layer1.setProviderKey('gdal')
        layer1.setUri('uri 1')
        layer1.setPath(['my', 'path'])

        model.setSublayerDetails([layer1])
        self.assertEqual(model.rowCount(QModelIndex()), 1)
        my_group_index = model.index(0, 0)
        self.assertEqual(model.data(my_group_index, Qt.DisplayRole), 'my')
        self.assertEqual(model.rowCount(my_group_index), 1)
        path_group_index = model.index(0, 0, my_group_index)
        self.assertEqual(model.data(path_group_index, Qt.DisplayRole), 'path')
        self.assertEqual(model.rowCount(path_group_index), 1)
        self.assertEqual(model.data(model.index(0, 0, path_group_index), Qt.DisplayRole), 'layer 1')
        self.assertEqual(model.data(model.index(0, 1, path_group_index), Qt.DisplayRole), 'description 1')
        self.assertEqual(model.data(model.index(0, 0, path_group_index), QgsProviderSublayerModel.Role.ProviderKey), 'gdal')
        self.assertEqual(model.data(model.index(0, 0, path_group_index), QgsProviderSublayerModel.Role.LayerType), QgsMapLayerType.RasterLayer)
        self.assertEqual(model.data(model.index(0, 0, path_group_index), QgsProviderSublayerModel.Role.Uri), 'uri 1')
        self.assertEqual(model.data(model.index(0, 0, path_group_index), QgsProviderSublayerModel.Role.Name), 'layer 1')
        self.assertEqual(model.data(model.index(0, 0, path_group_index), QgsProviderSublayerModel.Role.Description), 'description 1')
        self.assertEqual(model.data(model.index(0, 0, path_group_index), QgsProviderSublayerModel.Role.Flags), 0)

        # different path
        layer2 = QgsProviderSublayerDetails()
        layer2.setType(QgsMapLayerType.VectorLayer)
        layer2.setName('layer 2')
        layer2.setDescription('description 2')
        layer2.setProviderKey('ogr')
        layer2.setUri('uri 2')
        layer2.setFeatureCount(-1)
        layer2.setWkbType(QgsWkbTypes.LineString)
        layer2.setFlags(Qgis.SublayerFlags(Qgis.SublayerFlag.SystemTable))
        layer2.setPath(['my'])

        model.setSublayerDetails([layer1, layer2])

        self.assertEqual(model.rowCount(QModelIndex()), 1)
        my_group_index = model.index(0, 0)
        self.assertEqual(model.data(my_group_index, Qt.DisplayRole), 'my')
        self.assertEqual(model.rowCount(my_group_index), 2)
        path_group_index = model.index(0, 0, my_group_index)
        self.assertEqual(model.data(path_group_index, Qt.DisplayRole), 'path')
        self.assertEqual(model.rowCount(path_group_index), 1)
        self.assertEqual(model.data(model.index(0, 0, path_group_index), Qt.DisplayRole), 'layer 1')
        self.assertEqual(model.data(model.index(0, 1, path_group_index), Qt.DisplayRole), 'description 1')
        self.assertEqual(model.data(model.index(0, 0, path_group_index), QgsProviderSublayerModel.Role.ProviderKey), 'gdal')
        self.assertEqual(model.data(model.index(0, 0, path_group_index), QgsProviderSublayerModel.Role.LayerType), QgsMapLayerType.RasterLayer)
        self.assertEqual(model.data(model.index(0, 0, path_group_index), QgsProviderSublayerModel.Role.Uri), 'uri 1')
        self.assertEqual(model.data(model.index(0, 0, path_group_index), QgsProviderSublayerModel.Role.Name), 'layer 1')
        self.assertEqual(model.data(model.index(0, 0, path_group_index), QgsProviderSublayerModel.Role.Description), 'description 1')
        self.assertEqual(model.data(model.index(0, 0, path_group_index), QgsProviderSublayerModel.Role.Flags), 0)
        layer2_index = model.index(1, 0, my_group_index)
        self.assertEqual(model.data(layer2_index, Qt.DisplayRole), 'layer 2')
        self.assertEqual(model.data(model.index(1, 1, my_group_index), Qt.DisplayRole), 'description 2 - LineString (Uncounted)')
        self.assertEqual(model.data(layer2_index, QgsProviderSublayerModel.Role.ProviderKey), 'ogr')
        self.assertEqual(model.data(layer2_index, QgsProviderSublayerModel.Role.LayerType), QgsMapLayerType.VectorLayer)
        self.assertEqual(model.data(layer2_index, QgsProviderSublayerModel.Role.Uri), 'uri 2')
        self.assertEqual(model.data(layer2_index, QgsProviderSublayerModel.Role.Name), 'layer 2')
        self.assertEqual(model.data(layer2_index, QgsProviderSublayerModel.Role.Description), 'description 2')
        self.assertEqual(model.data(layer2_index, QgsProviderSublayerModel.Role.Flags), 1)

        # no path
        layer3 = QgsProviderSublayerDetails()
        layer3.setType(QgsMapLayerType.VectorLayer)
        layer3.setName('layer 3')
        layer3.setProviderKey('ogr')
        layer3.setUri('uri 3')
        layer3.setFeatureCount(1001)
        layer3.setWkbType(QgsWkbTypes.Polygon)

        model.setSublayerDetails([layer1, layer2, layer3])

        self.assertEqual(model.rowCount(QModelIndex()), 2)
        my_group_index = model.index(0, 0)
        self.assertEqual(model.data(my_group_index, Qt.DisplayRole), 'my')
        self.assertEqual(model.rowCount(my_group_index), 2)
        path_group_index = model.index(0, 0, my_group_index)
        self.assertEqual(model.data(path_group_index, Qt.DisplayRole), 'path')
        self.assertEqual(model.rowCount(path_group_index), 1)
        layer1_index = model.index(0, 0, path_group_index)
        self.assertEqual(model.data(layer1_index, Qt.DisplayRole), 'layer 1')
        self.assertEqual(model.data(model.index(0, 1, path_group_index), Qt.DisplayRole), 'description 1')
        self.assertEqual(model.data(layer1_index, QgsProviderSublayerModel.Role.ProviderKey), 'gdal')
        self.assertEqual(model.data(layer1_index, QgsProviderSublayerModel.Role.LayerType), QgsMapLayerType.RasterLayer)
        self.assertEqual(model.data(layer1_index, QgsProviderSublayerModel.Role.Uri), 'uri 1')
        self.assertEqual(model.data(layer1_index, QgsProviderSublayerModel.Role.Name), 'layer 1')
        self.assertEqual(model.data(layer1_index, QgsProviderSublayerModel.Role.Description), 'description 1')
        self.assertEqual(model.data(layer1_index, QgsProviderSublayerModel.Role.Flags), 0)
        layer2_index = model.index(1, 0, my_group_index)
        self.assertEqual(model.data(layer2_index, Qt.DisplayRole), 'layer 2')
        self.assertEqual(model.data(model.index(1, 1, my_group_index), Qt.DisplayRole), 'description 2 - LineString (Uncounted)')
        self.assertEqual(model.data(layer2_index, QgsProviderSublayerModel.Role.ProviderKey), 'ogr')
        self.assertEqual(model.data(layer2_index, QgsProviderSublayerModel.Role.LayerType), QgsMapLayerType.VectorLayer)
        self.assertEqual(model.data(layer2_index, QgsProviderSublayerModel.Role.Uri), 'uri 2')
        self.assertEqual(model.data(layer2_index, QgsProviderSublayerModel.Role.Name), 'layer 2')
        self.assertEqual(model.data(layer2_index, QgsProviderSublayerModel.Role.Description), 'description 2')
        self.assertEqual(model.data(layer2_index, QgsProviderSublayerModel.Role.Flags), 1)
        layer3_index = model.index(1, 0)
        self.assertEqual(model.data(layer3_index, Qt.DisplayRole), 'layer 3')
        self.assertEqual(model.data(model.index(1, 1), Qt.DisplayRole), 'Polygon (1,001)')
        self.assertEqual(model.data(layer3_index, QgsProviderSublayerModel.Role.ProviderKey), 'ogr')
        self.assertEqual(model.data(layer3_index, QgsProviderSublayerModel.Role.LayerType), QgsMapLayerType.VectorLayer)
        self.assertEqual(model.data(layer3_index, QgsProviderSublayerModel.Role.Uri), 'uri 3')
        self.assertEqual(model.data(layer3_index, QgsProviderSublayerModel.Role.Name), 'layer 3')
        self.assertEqual(model.data(layer3_index, QgsProviderSublayerModel.Role.Description), None)

        self.assertEqual(model.indexToSublayer(layer1_index), layer1)
        self.assertEqual(model.indexToSublayer(layer2_index), layer2)
        self.assertEqual(model.indexToSublayer(layer3_index), layer3)
        self.assertFalse(model.indexToSublayer(model.index(3, 0, QModelIndex())).name())
        self.assertFalse(model.indexToNonLayerItem(model.index(0, 0, QModelIndex())).name())

        # remove a layer
        model.setSublayerDetails([layer3, layer1])

        self.assertEqual(model.rowCount(QModelIndex()), 2)
        my_group_index = model.index(0, 0)
        self.assertEqual(model.data(my_group_index, Qt.DisplayRole), 'my')
        self.assertEqual(model.rowCount(my_group_index), 1)
        path_group_index = model.index(0, 0, my_group_index)
        self.assertEqual(model.data(path_group_index, Qt.DisplayRole), 'path')
        self.assertEqual(model.rowCount(path_group_index), 1)
        layer1_index = model.index(0, 0, path_group_index)
        self.assertEqual(model.data(layer1_index, Qt.DisplayRole), 'layer 1')
        self.assertEqual(model.data(model.index(0, 1, path_group_index), Qt.DisplayRole), 'description 1')
        self.assertEqual(model.data(layer1_index, QgsProviderSublayerModel.Role.ProviderKey), 'gdal')
        self.assertEqual(model.data(layer1_index, QgsProviderSublayerModel.Role.LayerType), QgsMapLayerType.RasterLayer)
        self.assertEqual(model.data(layer1_index, QgsProviderSublayerModel.Role.Uri), 'uri 1')
        self.assertEqual(model.data(layer1_index, QgsProviderSublayerModel.Role.Name), 'layer 1')
        self.assertEqual(model.data(layer1_index, QgsProviderSublayerModel.Role.Description), 'description 1')
        self.assertEqual(model.data(layer1_index, QgsProviderSublayerModel.Role.Flags), 0)
        layer3_index = model.index(1, 0)
        self.assertEqual(model.data(layer3_index, Qt.DisplayRole), 'layer 3')
        self.assertEqual(model.data(model.index(1, 1), Qt.DisplayRole), 'Polygon (1,001)')
        self.assertEqual(model.data(layer3_index, QgsProviderSublayerModel.Role.ProviderKey), 'ogr')
        self.assertEqual(model.data(layer3_index, QgsProviderSublayerModel.Role.LayerType), QgsMapLayerType.VectorLayer)
        self.assertEqual(model.data(layer3_index, QgsProviderSublayerModel.Role.Uri), 'uri 3')
        self.assertEqual(model.data(layer3_index, QgsProviderSublayerModel.Role.Name), 'layer 3')
        self.assertEqual(model.data(layer3_index, QgsProviderSublayerModel.Role.Description), None)

        # remove another layer
        model.setSublayerDetails([layer3])

        self.assertEqual(model.rowCount(QModelIndex()), 2)
        my_group_index = model.index(0, 0)
        self.assertEqual(model.data(my_group_index, Qt.DisplayRole), 'my')
        self.assertEqual(model.rowCount(my_group_index), 1)
        path_group_index = model.index(0, 0, my_group_index)
        self.assertEqual(model.data(path_group_index, Qt.DisplayRole), 'path')
        self.assertEqual(model.rowCount(path_group_index), 0)
        layer3_index = model.index(1, 0)
        self.assertEqual(model.data(layer3_index, Qt.DisplayRole), 'layer 3')
        self.assertEqual(model.data(model.index(1, 1), Qt.DisplayRole), 'Polygon (1,001)')
        self.assertEqual(model.data(layer3_index, QgsProviderSublayerModel.Role.ProviderKey), 'ogr')
        self.assertEqual(model.data(layer3_index, QgsProviderSublayerModel.Role.LayerType), QgsMapLayerType.VectorLayer)
        self.assertEqual(model.data(layer3_index, QgsProviderSublayerModel.Role.Uri), 'uri 3')
        self.assertEqual(model.data(layer3_index, QgsProviderSublayerModel.Role.Name), 'layer 3')
        self.assertEqual(model.data(layer3_index, QgsProviderSublayerModel.Role.Description), None)

    def test_proxy(self):
        """
        Test QgsProviderSublayerProxyModel
        """
        model = QgsProviderSublayerModel()
        proxy = QgsProviderSublayerProxyModel()
        proxy.setSourceModel(model)
        self.assertEqual(model.rowCount(QModelIndex()), 0)
        self.assertEqual(proxy.columnCount(QModelIndex()), 2)
        self.assertEqual(proxy.headerData(0, Qt.Horizontal, Qt.DisplayRole), 'Item')
        self.assertEqual(proxy.headerData(0, Qt.Horizontal, Qt.ToolTipRole), 'Item')
        self.assertEqual(proxy.headerData(1, Qt.Horizontal, Qt.DisplayRole), 'Description')
        self.assertEqual(proxy.headerData(1, Qt.Horizontal, Qt.ToolTipRole), 'Description')

        layer1 = QgsProviderSublayerDetails()
        layer1.setType(QgsMapLayerType.RasterLayer)
        layer1.setName('layer 1')
        layer1.setDescription('description 1')
        layer1.setProviderKey('gdal')
        layer1.setUri('uri 1')

        layer2 = QgsProviderSublayerDetails()
        layer2.setType(QgsMapLayerType.VectorLayer)
        layer2.setName('another layer 2')
        layer2.setDescription('description 2')
        layer2.setProviderKey('ogr')
        layer2.setUri('uri 2')
        layer2.setFeatureCount(-1)
        layer2.setWkbType(QgsWkbTypes.LineString)

        layer3 = QgsProviderSublayerDetails()
        layer3.setType(QgsMapLayerType.VectorLayer)
        layer3.setName('yet another layer 3')
        layer3.setDescription('an empty layer')
        layer3.setProviderKey('ogr')
        layer3.setUri('uri 3')
        layer3.setFeatureCount(0)
        layer3.setWkbType(QgsWkbTypes.Polygon)

        model.setSublayerDetails([layer1, layer2, layer3])

        item1 = QgsProviderSublayerModel.NonLayerItem()
        item1.setUri('item uri 1')
        item1.setName('item name 1')
        item1.setType('item type 1')
        item1.setDescription('item desc 1')

        model.addNonLayerItem(item1)

        self.assertEqual(proxy.rowCount(QModelIndex()), 4)

        self.assertEqual(proxy.data(proxy.index(0, 0), Qt.DisplayRole), 'item name 1')
        self.assertEqual(proxy.data(proxy.index(0, 1), Qt.DisplayRole), 'item desc 1')
        self.assertEqual(proxy.data(proxy.index(0, 0), QgsProviderSublayerModel.Role.Uri), 'item uri 1')
        self.assertEqual(proxy.data(proxy.index(0, 0), QgsProviderSublayerModel.Role.Name), 'item name 1')
        self.assertEqual(proxy.data(proxy.index(0, 0), QgsProviderSublayerModel.Role.Description), 'item desc 1')
        self.assertEqual(proxy.data(proxy.index(0, 0), QgsProviderSublayerModel.Role.IsNonLayerItem), True)

        self.assertEqual(proxy.data(proxy.index(1, 0), Qt.DisplayRole), 'another layer 2')
        self.assertEqual(proxy.data(proxy.index(1, 1), Qt.DisplayRole), 'description 2 - LineString (Uncounted)')
        self.assertEqual(proxy.data(proxy.index(1, 0), QgsProviderSublayerModel.Role.ProviderKey), 'ogr')
        self.assertEqual(proxy.data(proxy.index(1, 0), QgsProviderSublayerModel.Role.LayerType), QgsMapLayerType.VectorLayer)
        self.assertEqual(proxy.data(proxy.index(1, 0), QgsProviderSublayerModel.Role.Uri), 'uri 2')
        self.assertEqual(proxy.data(proxy.index(1, 0), QgsProviderSublayerModel.Role.Name), 'another layer 2')
        self.assertEqual(proxy.data(proxy.index(1, 0), QgsProviderSublayerModel.Role.Description), 'description 2')
        self.assertEqual(proxy.data(proxy.index(1, 0), QgsProviderSublayerModel.Role.IsNonLayerItem), False)

        self.assertEqual(proxy.data(proxy.index(2, 0), Qt.DisplayRole), 'layer 1')
        self.assertEqual(proxy.data(proxy.index(2, 1), Qt.DisplayRole), 'description 1')
        self.assertEqual(proxy.data(proxy.index(2, 0), QgsProviderSublayerModel.Role.ProviderKey), 'gdal')
        self.assertEqual(proxy.data(proxy.index(2, 0), QgsProviderSublayerModel.Role.LayerType), QgsMapLayerType.RasterLayer)
        self.assertEqual(proxy.data(proxy.index(2, 0), QgsProviderSublayerModel.Role.Uri), 'uri 1')
        self.assertEqual(proxy.data(proxy.index(2, 0), QgsProviderSublayerModel.Role.Name), 'layer 1')
        self.assertEqual(proxy.data(proxy.index(2, 0), QgsProviderSublayerModel.Role.Description), 'description 1')
        self.assertEqual(proxy.data(proxy.index(2, 0), QgsProviderSublayerModel.Role.IsNonLayerItem), False)

        self.assertEqual(proxy.data(proxy.index(3, 0), Qt.DisplayRole), 'yet another layer 3')
        self.assertEqual(proxy.data(proxy.index(3, 1), Qt.DisplayRole), 'an empty layer - Polygon (0)')
        self.assertEqual(proxy.data(proxy.index(3, 0), QgsProviderSublayerModel.Role.ProviderKey), 'ogr')
        self.assertEqual(proxy.data(proxy.index(3, 0), QgsProviderSublayerModel.Role.LayerType), QgsMapLayerType.VectorLayer)
        self.assertEqual(proxy.data(proxy.index(3, 0), QgsProviderSublayerModel.Role.Uri), 'uri 3')
        self.assertEqual(proxy.data(proxy.index(3, 0), QgsProviderSublayerModel.Role.Name), 'yet another layer 3')
        self.assertEqual(proxy.data(proxy.index(3, 0), QgsProviderSublayerModel.Role.Description), 'an empty layer')
        self.assertEqual(proxy.data(proxy.index(3, 0), QgsProviderSublayerModel.Role.IsNonLayerItem), False)

        proxy.setFilterString(' 1')
        self.assertEqual(proxy.rowCount(QModelIndex()), 2)
        self.assertEqual(proxy.data(proxy.index(0, 0), Qt.DisplayRole), 'item name 1')
        self.assertEqual(proxy.data(proxy.index(1, 0), Qt.DisplayRole), 'layer 1')

        proxy.setFilterString(' 2')
        self.assertEqual(proxy.rowCount(QModelIndex()), 1)
        self.assertEqual(proxy.data(proxy.index(0, 0), Qt.DisplayRole), 'another layer 2')

        proxy.setFilterString('ITEM')
        self.assertEqual(proxy.rowCount(QModelIndex()), 1)
        self.assertEqual(proxy.data(proxy.index(0, 0), Qt.DisplayRole), 'item name 1')

        # should also allow filtering by vector layer wkb type strings
        proxy.setFilterString('LineSTRING')
        self.assertEqual(proxy.rowCount(QModelIndex()), 1)
        self.assertEqual(proxy.data(proxy.index(0, 0), Qt.DisplayRole), 'another layer 2')

        # should also allow filtering by the feature count as it appears in the description too but it's not in the description role
        proxy.setFilterString('0')
        self.assertEqual(proxy.rowCount(QModelIndex()), 1)
        self.assertEqual(proxy.data(proxy.index(0, 0), Qt.DisplayRole), 'yet another layer 3')

        proxy.setFilterString('')
        self.assertEqual(proxy.rowCount(QModelIndex()), 4)
        self.assertEqual(proxy.data(proxy.index(0, 0), Qt.DisplayRole), 'item name 1')
        self.assertEqual(proxy.data(proxy.index(1, 0), Qt.DisplayRole), 'another layer 2')
        self.assertEqual(proxy.data(proxy.index(2, 0), Qt.DisplayRole), 'layer 1')
        self.assertEqual(proxy.data(proxy.index(3, 0), Qt.DisplayRole), 'yet another layer 3')

        # add a system table
        layer_system = QgsProviderSublayerDetails()
        layer_system.setType(QgsMapLayerType.VectorLayer)
        layer_system.setName('system table')
        layer_system.setFlags(Qgis.SublayerFlags(Qgis.SublayerFlag.SystemTable))

        model.setSublayerDetails([layer1, layer2, layer3, layer_system])
        # system tables should be hidden by default
        self.assertEqual(proxy.rowCount(QModelIndex()), 4)
        self.assertEqual(proxy.data(proxy.index(0, 0), Qt.DisplayRole), 'item name 1')
        self.assertEqual(proxy.data(proxy.index(1, 0), Qt.DisplayRole), 'another layer 2')
        self.assertEqual(proxy.data(proxy.index(2, 0), Qt.DisplayRole), 'layer 1')
        self.assertEqual(proxy.data(proxy.index(3, 0), Qt.DisplayRole), 'yet another layer 3')

        proxy.setIncludeSystemTables(True)
        self.assertEqual(proxy.rowCount(QModelIndex()), 5)
        self.assertEqual(proxy.data(proxy.index(0, 0), Qt.DisplayRole), 'item name 1')
        self.assertEqual(proxy.data(proxy.index(1, 0), Qt.DisplayRole), 'another layer 2')
        self.assertEqual(proxy.data(proxy.index(2, 0), Qt.DisplayRole), 'layer 1')
        self.assertEqual(proxy.data(proxy.index(3, 0), Qt.DisplayRole), 'system table')
        self.assertEqual(proxy.data(proxy.index(4, 0), Qt.DisplayRole), 'yet another layer 3')

        # hide empty vector layers
        proxy.setIncludeEmptyLayers(False)
        self.assertEqual(proxy.rowCount(QModelIndex()), 4)
        self.assertEqual(proxy.data(proxy.index(0, 0), Qt.DisplayRole), 'item name 1')
        self.assertEqual(proxy.data(proxy.index(1, 0), Qt.DisplayRole), 'another layer 2')
        self.assertEqual(proxy.data(proxy.index(2, 0), Qt.DisplayRole), 'layer 1')
        self.assertEqual(proxy.data(proxy.index(3, 0), Qt.DisplayRole), 'system table')


if __name__ == '__main__':
    unittest.main()
