"""QGIS Unit tests for QgsLayerTreeFilterProxyModel.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import unittest

from qgis.PyQt.QtCore import Qt, QStringListModel, QItemSelectionModel
from qgis.PyQt.QtTest import QAbstractItemModelTester, QSignalSpy
from qgis.core import (
    QgsLayerTree,
    QgsLayerTreeModel,
    QgsProject,
    QgsVectorLayer,
    QgsCategorizedSymbolRenderer,
    QgsRendererCategory,
    QgsMarkerSymbol,
    QgsMapLayerLegend,
    QgsLayerTreeFilterProxyModel,
)
from qgis.testing import start_app, QgisTestCase
from utilities import unitTestDataPath

app = start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsLayerTreeFilterProxyModel(QgisTestCase):

    def __init__(self, methodName):
        """Run once on class initialization."""

        QgisTestCase.__init__(self, methodName)

        # setup a dummy project
        self.project = QgsProject()
        self.layer = QgsVectorLayer("Point?field=fldtxt:string", "layer1", "memory")
        self.layer2 = QgsVectorLayer("Point?field=fldtxt:string", "layer2", "memory")
        self.layer3 = QgsVectorLayer("Point?field=fldtxt:string", "layer3", "memory")
        self.layer4 = QgsVectorLayer("Point?field=fldtxt:string", "layer4", "memory")
        self.layer5 = QgsVectorLayer("Point?field=fldtxt:string", "layer5", "memory")
        self.project.addMapLayers([self.layer, self.layer2, self.layer3])
        self.model = QgsLayerTreeModel(self.project.layerTreeRoot())

    def test_filter(self):
        """Test proxy model filtering and private layers"""

        proxy_model = QgsLayerTreeFilterProxyModel()
        proxy_model.setLayerTreeModel(self.model)

        self.assertEqual(self.model.rowCount(), 3)
        self.assertEqual(proxy_model.rowCount(), 3)

        items = []
        for r in range(self.model.rowCount()):
            items.append(self.model.data(self.model.index(r, 0)))

        self.assertEqual(items, ["layer1", "layer2", "layer3"])

        proxy_items = []
        for r in range(proxy_model.rowCount()):
            proxy_items.append(
                proxy_model.data(proxy_model.index(r, 0), Qt.ItemDataRole.DisplayRole)
            )

        self.assertEqual(proxy_items, ["layer1", "layer2", "layer3"])

        self.layer3.setFlags(self.layer.Private)

        self.assertEqual(self.model.rowCount(), 3)
        self.assertEqual(proxy_model.rowCount(), 3)
        proxy_model.setShowPrivateLayers(False)
        self.assertEqual(proxy_model.rowCount(), 2)

        proxy_items = []
        for r in range(proxy_model.rowCount()):
            proxy_items.append(
                proxy_model.data(proxy_model.index(r, 0), Qt.ItemDataRole.DisplayRole)
            )

        self.assertEqual(proxy_items, ["layer1", "layer2"])

        proxy_model.setShowPrivateLayers(True)

        self.assertEqual(proxy_model.rowCount(), 3)


if __name__ == "__main__":
    unittest.main()
