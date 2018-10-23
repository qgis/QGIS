# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsMapLayerStore.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '2017-05'
__copyright__ = 'Copyright 2017, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from qgis.core import QgsMapLayerStore, QgsVectorLayer, QgsMapLayer
from qgis.testing import start_app, unittest
from qgis.PyQt.QtCore import QT_VERSION_STR
from qgis.PyQt import sip
from qgis.PyQt.QtTest import QSignalSpy
from time import sleep

start_app()


def createLayer(name):
    return QgsVectorLayer("Point?field=x:string", name, "memory")


class TestQgsMapLayerStore(unittest.TestCase):

    def setUp(self):
        pass

    def test_addMapLayer(self):
        """ test adding individual map layers to store"""
        store = QgsMapLayerStore()

        l1 = createLayer('test')
        self.assertEqual(store.addMapLayer(l1), l1)
        self.assertEqual(len(store.mapLayersByName('test')), 1)
        self.assertEqual(store.count(), 1)
        self.assertEqual(len(store), 1)

        # adding a second layer should leave existing layers intact
        l2 = createLayer('test2')
        self.assertEqual(store.addMapLayer(l2), l2)
        self.assertEqual(len(store.mapLayersByName('test')), 1)
        self.assertEqual(len(store.mapLayersByName('test2')), 1)
        self.assertEqual(store.count(), 2)
        self.assertEqual(len(store), 2)

    def test_addMapLayerAlreadyAdded(self):
        """ test that already added layers can't be readded to store """
        store = QgsMapLayerStore()

        l1 = createLayer('test')
        store.addMapLayer(l1)
        self.assertEqual(len(store.mapLayersByName('test')), 1)
        self.assertEqual(store.count(), 1)
        self.assertEqual(store.addMapLayer(l1), None)
        self.assertEqual(len(store.mapLayersByName('test')), 1)
        self.assertEqual(store.count(), 1)
        self.assertEqual(len(store), 1)

    def test_addMapLayerInvalid(self):
        """ test that invalid map layers can't be added to store """
        store = QgsMapLayerStore()

        self.assertEqual(store.addMapLayer(QgsVectorLayer("Point?field=x:string", 'test', "xxx")), None)
        self.assertEqual(len(store.mapLayersByName('test')), 0)
        self.assertEqual(store.count(), 0)

    def test_addMapLayerSignals(self):
        """ test that signals are correctly emitted when adding map layer"""

        store = QgsMapLayerStore()

        layer_was_added_spy = QSignalSpy(store.layerWasAdded)
        layers_added_spy = QSignalSpy(store.layersAdded)

        l1 = createLayer('test')
        store.addMapLayer(l1)

        # can't seem to actually test the data which was emitted, so best we can do is test
        # the signal count
        self.assertEqual(len(layer_was_added_spy), 1)
        self.assertEqual(len(layers_added_spy), 1)

        store.addMapLayer(createLayer('test2'))
        self.assertEqual(len(layer_was_added_spy), 2)
        self.assertEqual(len(layers_added_spy), 2)

        # try readding a layer already in the store
        store.addMapLayer(l1)
        # should be no extra signals emitted
        self.assertEqual(len(layer_was_added_spy), 2)
        self.assertEqual(len(layers_added_spy), 2)

    def test_addMapLayers(self):
        """ test adding multiple map layers to store """
        store = QgsMapLayerStore()

        l1 = createLayer('test')
        l2 = createLayer('test2')
        self.assertEqual(set(store.addMapLayers([l1, l2])), {l1, l2})
        self.assertEqual(len(store.mapLayersByName('test')), 1)
        self.assertEqual(len(store.mapLayersByName('test2')), 1)
        self.assertEqual(store.count(), 2)

        # adding more layers should leave existing layers intact
        l3 = createLayer('test3')
        l4 = createLayer('test4')
        self.assertEqual(set(store.addMapLayers([l3, l4])), {l3, l4})
        self.assertEqual(len(store.mapLayersByName('test')), 1)
        self.assertEqual(len(store.mapLayersByName('test2')), 1)
        self.assertEqual(len(store.mapLayersByName('test3')), 1)
        self.assertEqual(len(store.mapLayersByName('test4')), 1)
        self.assertEqual(store.count(), 4)

        store.removeAllMapLayers()

    def test_addMapLayersInvalid(self):
        """ test that invalid map layersd can't be added to store """
        store = QgsMapLayerStore()

        self.assertEqual(store.addMapLayers([QgsVectorLayer("Point?field=x:string", 'test', "xxx")]), [])
        self.assertEqual(len(store.mapLayersByName('test')), 0)
        self.assertEqual(store.count(), 0)

    def test_addMapLayersAlreadyAdded(self):
        """ test that already added layers can't be readded to store """
        store = QgsMapLayerStore()

        l1 = createLayer('test')
        self.assertEqual(store.addMapLayers([l1]), [l1])
        self.assertEqual(len(store.mapLayersByName('test')), 1)
        self.assertEqual(store.count(), 1)
        self.assertEqual(store.addMapLayers([l1]), [])
        self.assertEqual(len(store.mapLayersByName('test')), 1)
        self.assertEqual(store.count(), 1)

    def test_addMapLayersSignals(self):
        """ test that signals are correctly emitted when adding map layers"""
        store = QgsMapLayerStore()

        layer_was_added_spy = QSignalSpy(store.layerWasAdded)
        layers_added_spy = QSignalSpy(store.layersAdded)

        l1 = createLayer('test')
        l2 = createLayer('test2')
        store.addMapLayers([l1, l2])

        # can't seem to actually test the data which was emitted, so best we can do is test
        # the signal count
        self.assertEqual(len(layer_was_added_spy), 2)
        self.assertEqual(len(layers_added_spy), 1)

        store.addMapLayers([createLayer('test3'), createLayer('test4')])
        self.assertEqual(len(layer_was_added_spy), 4)
        self.assertEqual(len(layers_added_spy), 2)

        # try readding a layer already in the store
        store.addMapLayers([l1, l2])
        # should be no extra signals emitted
        self.assertEqual(len(layer_was_added_spy), 4)
        self.assertEqual(len(layers_added_spy), 2)

    def test_mapLayerById(self):
        """ test retrieving map layer by ID """
        store = QgsMapLayerStore()

        # test no crash with empty store
        self.assertEqual(store.mapLayer('bad'), None)
        self.assertEqual(store.mapLayer(None), None)

        l1 = createLayer('test')
        l2 = createLayer('test2')

        store.addMapLayers([l1, l2])

        self.assertEqual(store.mapLayer('bad'), None)
        self.assertEqual(store.mapLayer(None), None)
        self.assertEqual(store.mapLayer(l1.id()), l1)
        self.assertEqual(store.mapLayer(l2.id()), l2)

    def test_mapLayersByName(self):
        """ test retrieving map layer by name """
        store = QgsMapLayerStore()

        # test no crash with empty store
        self.assertEqual(store.mapLayersByName('bad'), [])
        self.assertEqual(store.mapLayersByName(None), [])

        l1 = createLayer('test')
        l2 = createLayer('test2')

        store.addMapLayers([l1, l2])

        self.assertEqual(store.mapLayersByName('bad'), [])
        self.assertEqual(store.mapLayersByName(None), [])
        self.assertEqual(store.mapLayersByName('test'), [l1])
        self.assertEqual(store.mapLayersByName('test2'), [l2])

        #duplicate name

        # little bit of a hack - we don't want a duplicate ID and since IDs are currently based on time we wait a bit here
        sleep(0.1)
        l3 = createLayer('test')

        store.addMapLayer(l3)
        self.assertEqual(set(store.mapLayersByName('test')), {l1, l3})

    def test_mapLayers(self):
        """ test retrieving map layers list """
        store = QgsMapLayerStore()

        # test no crash with empty store
        self.assertEqual(store.mapLayers(), {})

        l1 = createLayer('test')
        l2 = createLayer('test2')

        store.addMapLayers([l1, l2])

        self.assertEqual(store.mapLayers(), {l1.id(): l1, l2.id(): l2})

    def test_removeMapLayersById(self):
        """ test removing map layers by ID """
        store = QgsMapLayerStore()

        # test no crash with empty store
        store.removeMapLayersById(['bad'])
        store.removeMapLayersById([None])

        l1 = createLayer('test')
        l2 = createLayer('test2')
        l3 = createLayer('test3')

        store.addMapLayers([l1, l2, l3])
        self.assertEqual(store.count(), 3)

        #remove bad layers
        store.removeMapLayersById(['bad'])
        self.assertEqual(store.count(), 3)
        store.removeMapLayersById([None])
        self.assertEqual(store.count(), 3)

        # remove valid layers
        l1_id = l1.id()
        store.removeMapLayersById([l1_id])
        self.assertEqual(store.count(), 2)
        # double remove
        store.removeMapLayersById([l1_id])
        self.assertEqual(store.count(), 2)

        # test that layer has been deleted
        self.assertTrue(sip.isdeleted(l1))

        # remove multiple
        store.removeMapLayersById([l2.id(), l3.id()])
        self.assertEqual(store.count(), 0)
        self.assertTrue(sip.isdeleted(l2))

        # try removing a layer not in the store
        l4 = createLayer('test4')
        store.removeMapLayersById([l4.id()])
        self.assertFalse(sip.isdeleted(l4))

    def test_removeMapLayersByLayer(self):
        """ test removing map layers by layer"""
        store = QgsMapLayerStore()

        # test no crash with empty store
        store.removeMapLayers([None])

        l1 = createLayer('test')
        l2 = createLayer('test2')
        l3 = createLayer('test3')

        store.addMapLayers([l1, l2, l3])
        self.assertEqual(store.count(), 3)

        #remove bad layers
        store.removeMapLayers([None])
        self.assertEqual(store.count(), 3)

        # remove valid layers
        store.removeMapLayers([l1])
        self.assertEqual(store.count(), 2)

        # test that layer has been deleted
        self.assertTrue(sip.isdeleted(l1))

        # remove multiple
        store.removeMapLayers([l2, l3])
        self.assertEqual(store.count(), 0)
        self.assertTrue(sip.isdeleted(l2))
        self.assertTrue(sip.isdeleted(l3))

    def test_removeMapLayerById(self):
        """ test removing a map layer by ID """
        store = QgsMapLayerStore()

        # test no crash with empty store
        store.removeMapLayer('bad')
        store.removeMapLayer(None)

        l1 = createLayer('test')
        l2 = createLayer('test2')

        store.addMapLayers([l1, l2])
        self.assertEqual(store.count(), 2)

        #remove bad layers
        store.removeMapLayer('bad')
        self.assertEqual(store.count(), 2)
        store.removeMapLayer(None)
        self.assertEqual(store.count(), 2)

        # remove valid layers
        l1_id = l1.id()
        store.removeMapLayer(l1_id)
        self.assertEqual(store.count(), 1)
        # double remove
        store.removeMapLayer(l1_id)
        self.assertEqual(store.count(), 1)

        # test that layer has been deleted
        self.assertTrue(sip.isdeleted(l1))

        # remove second layer
        store.removeMapLayer(l2.id())
        self.assertEqual(store.count(), 0)
        self.assertTrue(sip.isdeleted(l2))

        # try removing a layer not in the store
        l3 = createLayer('test3')
        store.removeMapLayer(l3.id())
        self.assertFalse(sip.isdeleted(l3))

    def test_removeMapLayerByLayer(self):
        """ test removing a map layer by layer """
        store = QgsMapLayerStore()

        # test no crash with empty store
        store.removeMapLayer('bad')
        store.removeMapLayer(None)

        l1 = createLayer('test')
        l2 = createLayer('test2')

        store.addMapLayers([l1, l2])
        self.assertEqual(store.count(), 2)

        #remove bad layers
        store.removeMapLayer(None)
        self.assertEqual(store.count(), 2)
        l3 = createLayer('test3')
        store.removeMapLayer(l3)
        self.assertEqual(store.count(), 2)

        # remove valid layers
        store.removeMapLayer(l1)
        self.assertEqual(store.count(), 1)

        # test that layer has been deleted
        self.assertTrue(sip.isdeleted(l1))

        # remove second layer
        store.removeMapLayer(l2)
        self.assertEqual(store.count(), 0)
        self.assertTrue(sip.isdeleted(l2))

        # try removing a layer not in the store
        l3 = createLayer('test3')
        store.removeMapLayer(l3)
        self.assertFalse(sip.isdeleted(l3))

    def test_removeAllMapLayers(self):
        """ test removing all map layers from store """
        store = QgsMapLayerStore()
        l1 = createLayer('test')
        l2 = createLayer('test2')

        store.addMapLayers([l1, l2])
        self.assertEqual(store.count(), 2)
        store.removeAllMapLayers()
        self.assertEqual(store.count(), 0)
        self.assertEqual(store.mapLayersByName('test'), [])
        self.assertEqual(store.mapLayersByName('test2'), [])

    def test_addRemoveLayersSignals(self):
        """ test that signals are correctly emitted when removing map layers"""
        store = QgsMapLayerStore()

        layers_will_be_removed_spy = QSignalSpy(store.layersWillBeRemoved)
        layer_will_be_removed_spy_str = QSignalSpy(store.layerWillBeRemoved[str])
        layer_will_be_removed_spy_layer = QSignalSpy(store.layerWillBeRemoved[QgsMapLayer])
        layers_removed_spy = QSignalSpy(store.layersRemoved)
        layer_removed_spy = QSignalSpy(store.layerRemoved)
        remove_all_spy = QSignalSpy(store.allLayersRemoved)

        l1 = createLayer('l1')
        l2 = createLayer('l2')
        l3 = createLayer('l3')
        l4 = createLayer('l4')
        store.addMapLayers([l1, l2, l3, l4])

        # remove 1 layer
        store.removeMapLayer(l1)
        # can't seem to actually test the data which was emitted, so best we can do is test
        # the signal count
        self.assertEqual(len(layers_will_be_removed_spy), 1)
        self.assertEqual(len(layer_will_be_removed_spy_str), 1)
        self.assertEqual(len(layer_will_be_removed_spy_layer), 1)
        self.assertEqual(len(layers_removed_spy), 1)
        self.assertEqual(len(layer_removed_spy), 1)
        self.assertEqual(len(remove_all_spy), 0)
        self.assertEqual(store.count(), 3)

        # remove 2 layers at once
        store.removeMapLayersById([l2.id(), l3.id()])
        self.assertEqual(len(layers_will_be_removed_spy), 2)
        self.assertEqual(len(layer_will_be_removed_spy_str), 3)
        self.assertEqual(len(layer_will_be_removed_spy_layer), 3)
        self.assertEqual(len(layers_removed_spy), 2)
        self.assertEqual(len(layer_removed_spy), 3)
        self.assertEqual(len(remove_all_spy), 0)
        self.assertEqual(store.count(), 1)

        # remove all
        store.removeAllMapLayers()
        self.assertEqual(len(layers_will_be_removed_spy), 3)
        self.assertEqual(len(layer_will_be_removed_spy_str), 4)
        self.assertEqual(len(layer_will_be_removed_spy_layer), 4)
        self.assertEqual(len(layers_removed_spy), 3)
        self.assertEqual(len(layer_removed_spy), 4)
        self.assertEqual(len(remove_all_spy), 1)

        #remove some layers which aren't in the store
        store.removeMapLayersById(['asdasd'])
        self.assertEqual(len(layers_will_be_removed_spy), 3)
        self.assertEqual(len(layer_will_be_removed_spy_str), 4)
        self.assertEqual(len(layer_will_be_removed_spy_layer), 4)
        self.assertEqual(len(layers_removed_spy), 3)
        self.assertEqual(len(layer_removed_spy), 4)
        self.assertEqual(len(remove_all_spy), 1)

        l5 = createLayer('test5')
        store.removeMapLayer(l5)
        self.assertEqual(len(layers_will_be_removed_spy), 3)
        self.assertEqual(len(layer_will_be_removed_spy_str), 4)
        self.assertEqual(len(layer_will_be_removed_spy_layer), 4)
        self.assertEqual(len(layers_removed_spy), 3)
        self.assertEqual(len(layer_removed_spy), 4)
        self.assertEqual(len(remove_all_spy), 1)

    def test_RemoveLayerShouldNotSegFault(self):
        store = QgsMapLayerStore()

        # Should not segfault
        store.removeMapLayersById(['not_exists'])
        store.removeMapLayer('not_exists2')

        # check also that the removal of an unexistent layer does not insert a null layer
        for k, layer in list(store.mapLayers().items()):
            assert(layer is not None)

    def testTakeLayer(self):
        # test taking ownership of a layer from the store
        l1 = createLayer('l1')
        l2 = createLayer('l2')
        store = QgsMapLayerStore()

        # add one layer to store
        store.addMapLayer(l1)
        self.assertEqual(store.mapLayers(), {l1.id(): l1})
        self.assertEqual(l1.parent(), store)

        # try taking some layers which don't exist in store
        self.assertFalse(store.takeMapLayer(None))
        self.assertFalse(store.takeMapLayer(l2))
        # but l2 should still exist..
        self.assertTrue(l2.isValid())

        # take layer from store
        self.assertEqual(store.takeMapLayer(l1), l1)
        self.assertFalse(store.mapLayers()) # no layers left
        # but l1 should still exist
        self.assertTrue(l1.isValid())
        # layer should have no parent now
        self.assertFalse(l1.parent())

        # destroy store
        store = None
        self.assertTrue(l1.isValid())

    def testTransferLayers(self):
        # test transferring all layers from another store
        store1 = QgsMapLayerStore()
        store2 = QgsMapLayerStore()

        # empty stores
        store1.transferLayersFromStore(store2)

        # silly behavior checks
        store1.transferLayersFromStore(None)
        store1.transferLayersFromStore(store1)

        l1 = createLayer('l1')
        l2 = createLayer('l2')
        store1.addMapLayer(l1)
        store1.addMapLayer(l2)

        l3 = createLayer('l3')
        store2.addMapLayer(l3)

        store2.transferLayersFromStore(store1)
        self.assertFalse(store1.mapLayers()) # no layers left
        self.assertEqual(len(store2.mapLayers()), 3)
        self.assertEqual(store2.mapLayers(), {l1.id(): l1, l2.id(): l2, l3.id(): l3})

        store1.transferLayersFromStore(store2)
        self.assertFalse(store2.mapLayers()) # no layers left
        self.assertEqual(len(store1.mapLayers()), 3)
        self.assertEqual(store1.mapLayers(), {l1.id(): l1, l2.id(): l2, l3.id(): l3})


if __name__ == '__main__':
    unittest.main()
