"""QGIS Unit tests for QgsMapThemeCollection.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "8/03/2017"
__copyright__ = "Copyright 2017, The QGIS Project"

from qgis.PyQt.QtTest import QSignalSpy
from qgis.core import QgsMapThemeCollection, QgsProject, QgsVectorLayer
from qgis.gui import QgsLayerTreeMapCanvasBridge, QgsMapCanvas
import unittest
from qgis.testing import start_app, QgisTestCase

app = start_app()


class TestQgsMapThemeCollection(QgisTestCase):

    def setUp(self):
        pass

    def testThemeChanged(self):
        """
        Test that the mapTheme(s)Changed signals are correctly emitted in all relevant situations
        """
        project = QgsProject()
        collection = QgsMapThemeCollection(project)

        record = QgsMapThemeCollection.MapThemeRecord()

        theme_changed_spy = QSignalSpy(collection.mapThemeChanged)
        themes_changed_spy = QSignalSpy(collection.mapThemesChanged)

        collection.insert("theme1", record)
        self.assertEqual(len(theme_changed_spy), 1)
        self.assertEqual(theme_changed_spy[-1][0], "theme1")
        self.assertEqual(len(themes_changed_spy), 1)

        # reinsert
        collection.insert("theme1", record)
        self.assertEqual(len(theme_changed_spy), 2)
        self.assertEqual(theme_changed_spy[-1][0], "theme1")
        self.assertEqual(len(themes_changed_spy), 2)

        # update
        collection.update("theme1", record)
        self.assertEqual(len(theme_changed_spy), 3)
        self.assertEqual(theme_changed_spy[-1][0], "theme1")
        self.assertEqual(len(themes_changed_spy), 3)

        # remove invalid
        collection.removeMapTheme(
            "i wish i was a slave to an age old trade... like riding around on rail cars and working long days"
        )
        self.assertEqual(len(theme_changed_spy), 3)
        self.assertEqual(len(themes_changed_spy), 3)
        # remove valid
        collection.removeMapTheme("theme1")
        self.assertEqual(len(theme_changed_spy), 3)  # not changed - removed!
        self.assertEqual(len(themes_changed_spy), 4)

        # reinsert
        collection.insert("theme1", record)
        self.assertEqual(len(theme_changed_spy), 4)
        self.assertEqual(len(themes_changed_spy), 5)

        # clear
        collection.clear()
        self.assertEqual(len(theme_changed_spy), 4)  # not changed - removed!
        self.assertEqual(len(themes_changed_spy), 6)

        # check that mapThemeChanged is emitted if layer is removed
        layer = QgsVectorLayer("Point?field=fldtxt:string", "layer1", "memory")
        layer2 = QgsVectorLayer("Point?field=fldtxt:string", "layer2", "memory")
        project.addMapLayers([layer, layer2])

        # record for layer1
        record.addLayerRecord(QgsMapThemeCollection.MapThemeLayerRecord(layer))
        collection.insert("theme1", record)
        self.assertEqual(len(theme_changed_spy), 5)
        self.assertEqual(len(themes_changed_spy), 7)

        # now kill layer 2
        project.removeMapLayer(layer2)
        self.assertEqual(
            len(theme_changed_spy), 5
        )  # signal should not be emitted - layer is not in record
        # now kill layer 1
        project.removeMapLayer(layer)
        app.processEvents()
        self.assertEqual(
            len(theme_changed_spy), 6
        )  # signal should be emitted - layer is in record

    def testMasterLayerOrder(self):
        """test master layer order"""
        prj = QgsProject.instance()
        prj.clear()
        layer = QgsVectorLayer("Point?field=fldtxt:string", "layer1", "memory")
        layer2 = QgsVectorLayer("Point?field=fldtxt:string", "layer2", "memory")
        layer3 = QgsVectorLayer("Point?field=fldtxt:string", "layer3", "memory")
        prj.addMapLayers([layer, layer2, layer3])

        prj.layerTreeRoot().setHasCustomLayerOrder(True)
        prj.layerTreeRoot().setCustomLayerOrder([layer2, layer])
        self.assertEqual(prj.mapThemeCollection().masterLayerOrder(), [layer2, layer])

        prj.layerTreeRoot().setCustomLayerOrder([layer, layer2, layer3])
        # make some themes...
        theme1 = QgsMapThemeCollection.MapThemeRecord()
        theme1.setLayerRecords(
            [
                QgsMapThemeCollection.MapThemeLayerRecord(layer3),
                QgsMapThemeCollection.MapThemeLayerRecord(layer),
            ]
        )

        theme2 = QgsMapThemeCollection.MapThemeRecord()
        theme2.setLayerRecords(
            [
                QgsMapThemeCollection.MapThemeLayerRecord(layer3),
                QgsMapThemeCollection.MapThemeLayerRecord(layer2),
                QgsMapThemeCollection.MapThemeLayerRecord(layer),
            ]
        )

        theme3 = QgsMapThemeCollection.MapThemeRecord()
        theme3.setLayerRecords(
            [
                QgsMapThemeCollection.MapThemeLayerRecord(layer2),
                QgsMapThemeCollection.MapThemeLayerRecord(layer),
            ]
        )

        prj.mapThemeCollection().insert("theme1", theme1)
        prj.mapThemeCollection().insert("theme2", theme2)
        prj.mapThemeCollection().insert("theme3", theme3)

        # order of layers in theme should respect master order
        self.assertEqual(
            prj.mapThemeCollection().mapThemeVisibleLayers("theme1"), [layer, layer3]
        )
        self.assertEqual(
            prj.mapThemeCollection().mapThemeVisibleLayers("theme2"),
            [layer, layer2, layer3],
        )
        self.assertEqual(
            prj.mapThemeCollection().mapThemeVisibleLayers("theme3"), [layer, layer2]
        )

        # also check ids!
        self.assertEqual(
            prj.mapThemeCollection().mapThemeVisibleLayerIds("theme1"),
            [layer.id(), layer3.id()],
        )
        self.assertEqual(
            prj.mapThemeCollection().mapThemeVisibleLayerIds("theme2"),
            [layer.id(), layer2.id(), layer3.id()],
        )
        self.assertEqual(
            prj.mapThemeCollection().mapThemeVisibleLayerIds("theme3"),
            [layer.id(), layer2.id()],
        )

        # reset master order
        prj.layerTreeRoot().setCustomLayerOrder([layer2, layer3, layer])
        self.assertEqual(
            prj.mapThemeCollection().mapThemeVisibleLayers("theme1"), [layer3, layer]
        )
        self.assertEqual(
            prj.mapThemeCollection().mapThemeVisibleLayers("theme2"),
            [layer2, layer3, layer],
        )
        self.assertEqual(
            prj.mapThemeCollection().mapThemeVisibleLayers("theme3"), [layer2, layer]
        )
        self.assertEqual(
            prj.mapThemeCollection().mapThemeVisibleLayerIds("theme1"),
            [layer3.id(), layer.id()],
        )
        self.assertEqual(
            prj.mapThemeCollection().mapThemeVisibleLayerIds("theme2"),
            [layer2.id(), layer3.id(), layer.id()],
        )
        self.assertEqual(
            prj.mapThemeCollection().mapThemeVisibleLayerIds("theme3"),
            [layer2.id(), layer.id()],
        )

        # check that layers include those hidden in the layer tree
        canvas = QgsMapCanvas()
        bridge = QgsLayerTreeMapCanvasBridge(prj.layerTreeRoot(), canvas)
        root = prj.layerTreeRoot()
        layer_node = root.findLayer(layer2.id())
        layer_node.setItemVisibilityChecked(False)
        app.processEvents()
        prj.layerTreeRoot().setHasCustomLayerOrder(False)
        self.assertEqual(
            prj.mapThemeCollection().masterLayerOrder(), [layer, layer2, layer3]
        )

        self.assertEqual(
            prj.mapThemeCollection().mapThemeVisibleLayers("theme1"), [layer, layer3]
        )
        self.assertEqual(
            prj.mapThemeCollection().mapThemeVisibleLayers("theme2"),
            [layer, layer2, layer3],
        )
        self.assertEqual(
            prj.mapThemeCollection().mapThemeVisibleLayers("theme3"), [layer, layer2]
        )
        self.assertEqual(
            prj.mapThemeCollection().mapThemeVisibleLayerIds("theme1"),
            [layer.id(), layer3.id()],
        )
        self.assertEqual(
            prj.mapThemeCollection().mapThemeVisibleLayerIds("theme2"),
            [layer.id(), layer2.id(), layer3.id()],
        )
        self.assertEqual(
            prj.mapThemeCollection().mapThemeVisibleLayerIds("theme3"),
            [layer.id(), layer2.id()],
        )

    def testMasterVisibleLayers(self):
        """test master visible layers"""
        prj = QgsProject.instance()
        prj.clear()
        layer = QgsVectorLayer("Point?field=fldtxt:string", "layer1", "memory")
        layer2 = QgsVectorLayer("Point?field=fldtxt:string", "layer2", "memory")
        layer3 = QgsVectorLayer("Point?field=fldtxt:string", "layer3", "memory")
        prj.addMapLayers([layer, layer2, layer3])

        # general setup...
        prj.layerTreeRoot().setHasCustomLayerOrder(True)
        prj.layerTreeRoot().setCustomLayerOrder([layer2, layer])
        self.assertEqual(
            prj.mapThemeCollection().masterVisibleLayers(), [layer2, layer]
        )
        prj.layerTreeRoot().setCustomLayerOrder([layer3, layer, layer2])
        self.assertEqual(
            prj.mapThemeCollection().masterVisibleLayers(), [layer3, layer, layer2]
        )

        # hide some layers
        root = prj.layerTreeRoot()
        layer_node = root.findLayer(layer2)
        layer_node.setItemVisibilityChecked(False)
        self.assertEqual(
            prj.mapThemeCollection().masterVisibleLayers(), [layer3, layer]
        )
        layer_node.setItemVisibilityChecked(True)
        self.assertEqual(
            prj.mapThemeCollection().masterVisibleLayers(), [layer3, layer, layer2]
        )
        layer_node.setItemVisibilityChecked(False)
        prj.layerTreeRoot().setCustomLayerOrder([layer, layer2, layer3])
        self.assertEqual(
            prj.mapThemeCollection().masterVisibleLayers(), [layer, layer3]
        )

        # test with no project layer order set, should respect tree order
        prj.layerTreeRoot().setCustomLayerOrder([])
        self.assertEqual(
            prj.mapThemeCollection().masterVisibleLayers(), [layer, layer3]
        )
        layer_node.setItemVisibilityChecked(True)
        self.assertEqual(
            prj.mapThemeCollection().masterVisibleLayers(), [layer, layer2, layer3]
        )


if __name__ == "__main__":
    unittest.main()
