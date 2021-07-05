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
import os

from qgis.core import (
    QgsProviderRegistry,
    QgsMapLayerType,
    QgsWkbTypes,
    QgsProviderSublayerDetails,
    Qgis,
    QgsCoordinateTransformContext,
    QgsVectorLayer,
    QgsProviderSublayerModel
)
from qgis.PyQt.QtCore import (
    Qt,
    QModelIndex
)
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

# Convenience instances in case you may need them
# to find the srs.db
start_app()


class TestQgsProviderSublayerModel(unittest.TestCase):

    def test_model(self):
        model = QgsProviderSublayerModel()
        self.assertEqual(model.rowCount(QModelIndex()), 0)
        self.assertEqual(model.columnCount(QModelIndex()), 2)
        self.assertEqual(model.headerData(0, Qt.Horizontal, Qt.DisplayRole), 'Layer')
        self.assertEqual(model.headerData(0, Qt.Horizontal, Qt.ToolTipRole), 'Layer')
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

        layer2 = QgsProviderSublayerDetails()
        layer2.setType(QgsMapLayerType.VectorLayer)
        layer2.setName('layer 2')
        layer2.setDescription('description 2')
        layer2.setProviderKey('ogr')
        layer2.setUri('uri 2')
        layer2.setFeatureCount(-1)
        layer2.setWkbType(QgsWkbTypes.LineString)

        model.setSublayerDetails([layer1, layer2])
        self.assertEqual(model.rowCount(QModelIndex()), 2)
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

    def test_model_with_non_layer_items(self):
        model = QgsProviderSublayerModel()
        self.assertEqual(model.rowCount(QModelIndex()), 0)
        self.assertEqual(model.columnCount(QModelIndex()), 2)
        self.assertEqual(model.headerData(0, Qt.Horizontal, Qt.DisplayRole), 'Layer')
        self.assertEqual(model.headerData(0, Qt.Horizontal, Qt.ToolTipRole), 'Layer')
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


if __name__ == '__main__':
    unittest.main()
