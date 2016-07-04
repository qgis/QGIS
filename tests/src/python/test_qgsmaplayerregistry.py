# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsMapLayerRegistry.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Alessandro Pasotti'
__date__ = '04/12/2015'
__copyright__ = 'Copyright 2015, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from qgis.core import QgsMapLayerRegistry, QgsVectorLayer, QgsMapLayer
from qgis.testing import start_app, unittest
from qgis.PyQt.QtCore import QT_VERSION_STR
import sip

try:
    from qgis.PyQt.QtTest import QSignalSpy
    use_signal_spy = True
except:
    use_signal_spy = False

start_app()


def createLayer(name):
    return QgsVectorLayer("Point?field=x:string", name, "memory")


class TestQgsMapLayerRegistry(unittest.TestCase):

    def setUp(self):
        pass

    def testInstance(self):
        """ test retrieving global instance """
        self.assertTrue(QgsMapLayerRegistry.instance())

        # register a layer to the singleton
        QgsMapLayerRegistry.instance().addMapLayer(createLayer('test'))

        # check that the same instance is returned
        self.assertEqual(len(QgsMapLayerRegistry.instance().mapLayersByName('test')), 1)

        QgsMapLayerRegistry.instance().removeAllMapLayers()

    def test_addMapLayer(self):
        """ test adding individual map layers to registry """
        QgsMapLayerRegistry.instance().removeAllMapLayers()

        l1 = createLayer('test')
        self.assertEqual(QgsMapLayerRegistry.instance().addMapLayer(l1), l1)
        self.assertEqual(len(QgsMapLayerRegistry.instance().mapLayersByName('test')), 1)
        self.assertEqual(QgsMapLayerRegistry.instance().count(), 1)

        # adding a second layer should leave existing layers intact
        l2 = createLayer('test2')
        self.assertEqual(QgsMapLayerRegistry.instance().addMapLayer(l2), l2)
        self.assertEqual(len(QgsMapLayerRegistry.instance().mapLayersByName('test')), 1)
        self.assertEqual(len(QgsMapLayerRegistry.instance().mapLayersByName('test2')), 1)
        self.assertEqual(QgsMapLayerRegistry.instance().count(), 2)

        QgsMapLayerRegistry.instance().removeAllMapLayers()

    def test_addMapLayerAlreadyAdded(self):
        """ test that already added layers can't be readded to registry """
        QgsMapLayerRegistry.instance().removeAllMapLayers()

        l1 = createLayer('test')
        QgsMapLayerRegistry.instance().addMapLayer(l1)
        self.assertEqual(len(QgsMapLayerRegistry.instance().mapLayersByName('test')), 1)
        self.assertEqual(QgsMapLayerRegistry.instance().count(), 1)
        self.assertEqual(QgsMapLayerRegistry.instance().addMapLayer(l1), None)
        self.assertEqual(len(QgsMapLayerRegistry.instance().mapLayersByName('test')), 1)
        self.assertEqual(QgsMapLayerRegistry.instance().count(), 1)

        QgsMapLayerRegistry.instance().removeAllMapLayers()

    def test_addMapLayerInvalid(self):
        """ test that invalid map layersd can't be added to registry """
        QgsMapLayerRegistry.instance().removeAllMapLayers()

        self.assertEqual(QgsMapLayerRegistry.instance().addMapLayer(QgsVectorLayer("Point?field=x:string", 'test', "xxx")), None)
        self.assertEqual(len(QgsMapLayerRegistry.instance().mapLayersByName('test')), 0)
        self.assertEqual(QgsMapLayerRegistry.instance().count(), 0)

        QgsMapLayerRegistry.instance().removeAllMapLayers()

    @unittest.skipIf(not use_signal_spy, "No QSignalSpy available")
    def test_addMapLayerSignals(self):
        """ test that signals are correctly emitted when adding map layer"""

        QgsMapLayerRegistry.instance().removeAllMapLayers()

        layer_was_added_spy = QSignalSpy(QgsMapLayerRegistry.instance().layerWasAdded)
        layers_added_spy = QSignalSpy(QgsMapLayerRegistry.instance().layersAdded)
        legend_layers_added_spy = QSignalSpy(QgsMapLayerRegistry.instance().legendLayersAdded)

        l1 = createLayer('test')
        QgsMapLayerRegistry.instance().addMapLayer(l1)

        # can't seem to actually test the data which was emitted, so best we can do is test
        # the signal count
        self.assertEqual(len(layer_was_added_spy), 1)
        self.assertEqual(len(layers_added_spy), 1)
        self.assertEqual(len(legend_layers_added_spy), 1)

        # layer not added to legend
        QgsMapLayerRegistry.instance().addMapLayer(createLayer('test2'), False)
        self.assertEqual(len(layer_was_added_spy), 2)
        self.assertEqual(len(layers_added_spy), 2)
        self.assertEqual(len(legend_layers_added_spy), 1)

        # try readding a layer already in the registry
        QgsMapLayerRegistry.instance().addMapLayer(l1)
        # should be no extra signals emitted
        self.assertEqual(len(layer_was_added_spy), 2)
        self.assertEqual(len(layers_added_spy), 2)
        self.assertEqual(len(legend_layers_added_spy), 1)

    def test_addMapLayers(self):
        """ test adding multiple map layers to registry """
        QgsMapLayerRegistry.instance().removeAllMapLayers()

        l1 = createLayer('test')
        l2 = createLayer('test2')
        self.assertEqual(set(QgsMapLayerRegistry.instance().addMapLayers([l1, l2])), set([l1, l2]))
        self.assertEqual(len(QgsMapLayerRegistry.instance().mapLayersByName('test')), 1)
        self.assertEqual(len(QgsMapLayerRegistry.instance().mapLayersByName('test2')), 1)
        self.assertEqual(QgsMapLayerRegistry.instance().count(), 2)

        # adding more layers should leave existing layers intact
        l3 = createLayer('test3')
        l4 = createLayer('test4')
        self.assertEqual(set(QgsMapLayerRegistry.instance().addMapLayers([l3, l4])), set([l3, l4]))
        self.assertEqual(len(QgsMapLayerRegistry.instance().mapLayersByName('test')), 1)
        self.assertEqual(len(QgsMapLayerRegistry.instance().mapLayersByName('test2')), 1)
        self.assertEqual(len(QgsMapLayerRegistry.instance().mapLayersByName('test3')), 1)
        self.assertEqual(len(QgsMapLayerRegistry.instance().mapLayersByName('test4')), 1)
        self.assertEqual(QgsMapLayerRegistry.instance().count(), 4)

        QgsMapLayerRegistry.instance().removeAllMapLayers()

    def test_addMapLayersInvalid(self):
        """ test that invalid map layersd can't be added to registry """
        QgsMapLayerRegistry.instance().removeAllMapLayers()

        self.assertEqual(QgsMapLayerRegistry.instance().addMapLayers([QgsVectorLayer("Point?field=x:string", 'test', "xxx")]), [])
        self.assertEqual(len(QgsMapLayerRegistry.instance().mapLayersByName('test')), 0)
        self.assertEqual(QgsMapLayerRegistry.instance().count(), 0)

        QgsMapLayerRegistry.instance().removeAllMapLayers()

    def test_addMapLayersAlreadyAdded(self):
        """ test that already added layers can't be readded to registry """
        QgsMapLayerRegistry.instance().removeAllMapLayers()

        l1 = createLayer('test')
        self.assertEqual(QgsMapLayerRegistry.instance().addMapLayers([l1]), [l1])
        self.assertEqual(len(QgsMapLayerRegistry.instance().mapLayersByName('test')), 1)
        self.assertEqual(QgsMapLayerRegistry.instance().count(), 1)
        self.assertEqual(QgsMapLayerRegistry.instance().addMapLayers([l1]), [])
        self.assertEqual(len(QgsMapLayerRegistry.instance().mapLayersByName('test')), 1)
        self.assertEqual(QgsMapLayerRegistry.instance().count(), 1)

        QgsMapLayerRegistry.instance().removeAllMapLayers()

    @unittest.skipIf(not use_signal_spy, "No QSignalSpy available")
    def test_addMapLayersSignals(self):
        """ test that signals are correctly emitted when adding map layers"""
        QgsMapLayerRegistry.instance().removeAllMapLayers()

        layer_was_added_spy = QSignalSpy(QgsMapLayerRegistry.instance().layerWasAdded)
        layers_added_spy = QSignalSpy(QgsMapLayerRegistry.instance().layersAdded)
        legend_layers_added_spy = QSignalSpy(QgsMapLayerRegistry.instance().legendLayersAdded)

        l1 = createLayer('test')
        l2 = createLayer('test2')
        QgsMapLayerRegistry.instance().addMapLayers([l1, l2])

        # can't seem to actually test the data which was emitted, so best we can do is test
        # the signal count
        self.assertEqual(len(layer_was_added_spy), 2)
        self.assertEqual(len(layers_added_spy), 1)
        self.assertEqual(len(legend_layers_added_spy), 1)

        # layer not added to legend
        QgsMapLayerRegistry.instance().addMapLayers([createLayer('test3'), createLayer('test4')], False)
        self.assertEqual(len(layer_was_added_spy), 4)
        self.assertEqual(len(layers_added_spy), 2)
        self.assertEqual(len(legend_layers_added_spy), 1)

        # try readding a layer already in the registry
        QgsMapLayerRegistry.instance().addMapLayers([l1, l2])
        # should be no extra signals emitted
        self.assertEqual(len(layer_was_added_spy), 4)
        self.assertEqual(len(layers_added_spy), 2)
        self.assertEqual(len(legend_layers_added_spy), 1)

    def test_mapLayerById(self):
        """ test retrieving map layer by ID """
        QgsMapLayerRegistry.instance().removeAllMapLayers()

        # test no crash with empty registry
        self.assertEqual(QgsMapLayerRegistry.instance().mapLayer('bad'), None)
        self.assertEqual(QgsMapLayerRegistry.instance().mapLayer(None), None)

        l1 = createLayer('test')
        l2 = createLayer('test2')

        QgsMapLayerRegistry.instance().addMapLayers([l1, l2])

        self.assertEqual(QgsMapLayerRegistry.instance().mapLayer('bad'), None)
        self.assertEqual(QgsMapLayerRegistry.instance().mapLayer(None), None)
        self.assertEqual(QgsMapLayerRegistry.instance().mapLayer(l1.id()), l1)
        self.assertEqual(QgsMapLayerRegistry.instance().mapLayer(l2.id()), l2)

    def test_mapLayersByName(self):
        """ test retrieving map layer by name """
        QgsMapLayerRegistry.instance().removeAllMapLayers()

        # test no crash with empty registry
        self.assertEqual(QgsMapLayerRegistry.instance().mapLayersByName('bad'), [])
        self.assertEqual(QgsMapLayerRegistry.instance().mapLayersByName(None), [])

        l1 = createLayer('test')
        l2 = createLayer('test2')

        QgsMapLayerRegistry.instance().addMapLayers([l1, l2])

        self.assertEqual(QgsMapLayerRegistry.instance().mapLayersByName('bad'), [])
        self.assertEqual(QgsMapLayerRegistry.instance().mapLayersByName(None), [])
        self.assertEqual(QgsMapLayerRegistry.instance().mapLayersByName('test'), [l1])
        self.assertEqual(QgsMapLayerRegistry.instance().mapLayersByName('test2'), [l2])

        #duplicate name
        l3 = createLayer('test')
        QgsMapLayerRegistry.instance().addMapLayer(l3)
        self.assertEqual(set(QgsMapLayerRegistry.instance().mapLayersByName('test')), set([l1, l3]))

    def test_mapLayers(self):
        """ test retrieving map layers list """
        QgsMapLayerRegistry.instance().removeAllMapLayers()

        # test no crash with empty registry
        self.assertEqual(QgsMapLayerRegistry.instance().mapLayers(), {})

        l1 = createLayer('test')
        l2 = createLayer('test2')

        QgsMapLayerRegistry.instance().addMapLayers([l1, l2])

        self.assertEqual(QgsMapLayerRegistry.instance().mapLayers(), {l1.id(): l1, l2.id(): l2})

    def test_removeMapLayersById(self):
        """ test removing map layers by ID """
        QgsMapLayerRegistry.instance().removeAllMapLayers()

        # test no crash with empty registry
        QgsMapLayerRegistry.instance().removeMapLayers(['bad'])
        QgsMapLayerRegistry.instance().removeMapLayers([None])

        l1 = createLayer('test')
        l2 = createLayer('test2')
        l3 = createLayer('test3')

        QgsMapLayerRegistry.instance().addMapLayers([l1, l2, l3])
        self.assertEqual(QgsMapLayerRegistry.instance().count(), 3)

        #remove bad layers
        QgsMapLayerRegistry.instance().removeMapLayers(['bad'])
        self.assertEqual(QgsMapLayerRegistry.instance().count(), 3)
        QgsMapLayerRegistry.instance().removeMapLayers([None])
        self.assertEqual(QgsMapLayerRegistry.instance().count(), 3)

        # remove valid layers
        l1_id = l1.id()
        QgsMapLayerRegistry.instance().removeMapLayers([l1_id])
        self.assertEqual(QgsMapLayerRegistry.instance().count(), 2)
        # double remove
        QgsMapLayerRegistry.instance().removeMapLayers([l1_id])
        self.assertEqual(QgsMapLayerRegistry.instance().count(), 2)

        # test that layer has been deleted
        self.assertTrue(sip.isdeleted(l1))

        # remove multiple
        QgsMapLayerRegistry.instance().removeMapLayers([l2.id(), l3.id()])
        self.assertEqual(QgsMapLayerRegistry.instance().count(), 0)
        self.assertTrue(sip.isdeleted(l2))

        # try removing a layer not in the registry
        l4 = createLayer('test4')
        QgsMapLayerRegistry.instance().removeMapLayers([l4.id()])
        self.assertFalse(sip.isdeleted(l4))

    # fails on qt5 due to removeMapLayers list type conversion - needs a PyName alias
    # added to removeMapLayers for QGIS 3.0
    @unittest.expectedFailure(QT_VERSION_STR[0] == '5')
    def test_removeMapLayersByLayer(self):
        """ test removing map layers by layer"""
        QgsMapLayerRegistry.instance().removeAllMapLayers()

        # test no crash with empty registry
        QgsMapLayerRegistry.instance().removeMapLayers([None])

        l1 = createLayer('test')
        l2 = createLayer('test2')
        l3 = createLayer('test3')

        QgsMapLayerRegistry.instance().addMapLayers([l1, l2, l3])
        self.assertEqual(QgsMapLayerRegistry.instance().count(), 3)

        #remove bad layers
        QgsMapLayerRegistry.instance().removeMapLayers([None])
        self.assertEqual(QgsMapLayerRegistry.instance().count(), 3)

        # remove valid layers
        QgsMapLayerRegistry.instance().removeMapLayers([l1])
        self.assertEqual(QgsMapLayerRegistry.instance().count(), 2)

        # test that layer has been deleted
        self.assertTrue(sip.isdeleted(l1))

        # remove multiple
        QgsMapLayerRegistry.instance().removeMapLayers([l2, l3])
        self.assertEqual(QgsMapLayerRegistry.instance().count(), 0)
        self.assertTrue(sip.isdeleted(l2))
        self.assertTrue(sip.isdeleted(l3))

    def test_removeMapLayerById(self):
        """ test removing a map layer by ID """
        QgsMapLayerRegistry.instance().removeAllMapLayers()

        # test no crash with empty registry
        QgsMapLayerRegistry.instance().removeMapLayer('bad')
        QgsMapLayerRegistry.instance().removeMapLayer(None)

        l1 = createLayer('test')
        l2 = createLayer('test2')

        QgsMapLayerRegistry.instance().addMapLayers([l1, l2])
        self.assertEqual(QgsMapLayerRegistry.instance().count(), 2)

        #remove bad layers
        QgsMapLayerRegistry.instance().removeMapLayer('bad')
        self.assertEqual(QgsMapLayerRegistry.instance().count(), 2)
        QgsMapLayerRegistry.instance().removeMapLayer(None)
        self.assertEqual(QgsMapLayerRegistry.instance().count(), 2)

        # remove valid layers
        l1_id = l1.id()
        QgsMapLayerRegistry.instance().removeMapLayer(l1_id)
        self.assertEqual(QgsMapLayerRegistry.instance().count(), 1)
        # double remove
        QgsMapLayerRegistry.instance().removeMapLayer(l1_id)
        self.assertEqual(QgsMapLayerRegistry.instance().count(), 1)

        # test that layer has been deleted
        self.assertTrue(sip.isdeleted(l1))

        # remove second layer
        QgsMapLayerRegistry.instance().removeMapLayer(l2.id())
        self.assertEqual(QgsMapLayerRegistry.instance().count(), 0)
        self.assertTrue(sip.isdeleted(l2))

        # try removing a layer not in the registry
        l3 = createLayer('test3')
        QgsMapLayerRegistry.instance().removeMapLayer(l3.id())
        self.assertFalse(sip.isdeleted(l3))

    def test_removeMapLayerByLayer(self):
        """ test removing a map layer by layer """
        QgsMapLayerRegistry.instance().removeAllMapLayers()

        # test no crash with empty registry
        QgsMapLayerRegistry.instance().removeMapLayer('bad')
        QgsMapLayerRegistry.instance().removeMapLayer(None)

        l1 = createLayer('test')
        l2 = createLayer('test2')

        QgsMapLayerRegistry.instance().addMapLayers([l1, l2])
        self.assertEqual(QgsMapLayerRegistry.instance().count(), 2)

        #remove bad layers
        QgsMapLayerRegistry.instance().removeMapLayer(None)
        self.assertEqual(QgsMapLayerRegistry.instance().count(), 2)
        l3 = createLayer('test3')
        QgsMapLayerRegistry.instance().removeMapLayer(l3)
        self.assertEqual(QgsMapLayerRegistry.instance().count(), 2)

        # remove valid layers
        QgsMapLayerRegistry.instance().removeMapLayer(l1)
        self.assertEqual(QgsMapLayerRegistry.instance().count(), 1)

        # test that layer has been deleted
        self.assertTrue(sip.isdeleted(l1))

        # remove second layer
        QgsMapLayerRegistry.instance().removeMapLayer(l2)
        self.assertEqual(QgsMapLayerRegistry.instance().count(), 0)
        self.assertTrue(sip.isdeleted(l2))

        # try removing a layer not in the registry
        l3 = createLayer('test3')
        QgsMapLayerRegistry.instance().removeMapLayer(l3)
        self.assertFalse(sip.isdeleted(l3))

    def test_removeAllMapLayers(self):
        """ test removing all map layers from registry """
        QgsMapLayerRegistry.instance().removeAllMapLayers()
        l1 = createLayer('test')
        l2 = createLayer('test2')

        QgsMapLayerRegistry.instance().addMapLayers([l1, l2])
        self.assertEqual(QgsMapLayerRegistry.instance().count(), 2)
        QgsMapLayerRegistry.instance().removeAllMapLayers()
        self.assertEqual(QgsMapLayerRegistry.instance().count(), 0)
        self.assertEqual(QgsMapLayerRegistry.instance().mapLayersByName('test'), [])
        self.assertEqual(QgsMapLayerRegistry.instance().mapLayersByName('test2'), [])

    @unittest.skipIf(not use_signal_spy, "No QSignalSpy available")
    def test_addRemoveLayersSignals(self):
        """ test that signals are correctly emitted when removing map layers"""
        QgsMapLayerRegistry.instance().removeAllMapLayers()

        layers_will_be_removed_spy = QSignalSpy(QgsMapLayerRegistry.instance().layersWillBeRemoved)
        layer_will_be_removed_spy_str = QSignalSpy(QgsMapLayerRegistry.instance().layerWillBeRemoved[str])
        layer_will_be_removed_spy_layer = QSignalSpy(QgsMapLayerRegistry.instance().layerWillBeRemoved[QgsMapLayer])
        layers_removed_spy = QSignalSpy(QgsMapLayerRegistry.instance().layersRemoved)
        layer_removed_spy = QSignalSpy(QgsMapLayerRegistry.instance().layerRemoved)
        remove_all_spy = QSignalSpy(QgsMapLayerRegistry.instance().removeAll)

        l1 = createLayer('l1')
        l2 = createLayer('l2')
        l3 = createLayer('l3')
        l4 = createLayer('l4')
        QgsMapLayerRegistry.instance().addMapLayers([l1, l2, l3, l4])

        # remove 1 layer
        QgsMapLayerRegistry.instance().removeMapLayer(l1)
        # can't seem to actually test the data which was emitted, so best we can do is test
        # the signal count
        self.assertEqual(len(layers_will_be_removed_spy), 1)
        self.assertEqual(len(layer_will_be_removed_spy_str), 1)
        self.assertEqual(len(layer_will_be_removed_spy_layer), 1)
        self.assertEqual(len(layers_removed_spy), 1)
        self.assertEqual(len(layer_removed_spy), 1)
        self.assertEqual(len(remove_all_spy), 0)
        self.assertEqual(QgsMapLayerRegistry.instance().count(), 3)

        # remove 2 layers at once
        QgsMapLayerRegistry.instance().removeMapLayers([l2.id(), l3.id()])
        self.assertEqual(len(layers_will_be_removed_spy), 2)
        self.assertEqual(len(layer_will_be_removed_spy_str), 3)
        self.assertEqual(len(layer_will_be_removed_spy_layer), 3)
        self.assertEqual(len(layers_removed_spy), 2)
        self.assertEqual(len(layer_removed_spy), 3)
        self.assertEqual(len(remove_all_spy), 0)
        self.assertEqual(QgsMapLayerRegistry.instance().count(), 1)

        # remove all
        QgsMapLayerRegistry.instance().removeAllMapLayers()
        self.assertEqual(len(layers_will_be_removed_spy), 3)
        self.assertEqual(len(layer_will_be_removed_spy_str), 4)
        self.assertEqual(len(layer_will_be_removed_spy_layer), 4)
        self.assertEqual(len(layers_removed_spy), 3)
        self.assertEqual(len(layer_removed_spy), 4)
        self.assertEqual(len(remove_all_spy), 1)

        #remove some layers which aren't in the registry
        QgsMapLayerRegistry.instance().removeMapLayers(['asdasd'])
        self.assertEqual(len(layers_will_be_removed_spy), 3)
        self.assertEqual(len(layer_will_be_removed_spy_str), 4)
        self.assertEqual(len(layer_will_be_removed_spy_layer), 4)
        self.assertEqual(len(layers_removed_spy), 3)
        self.assertEqual(len(layer_removed_spy), 4)
        self.assertEqual(len(remove_all_spy), 1)

        l5 = createLayer('test5')
        QgsMapLayerRegistry.instance().removeMapLayer(l5)
        self.assertEqual(len(layers_will_be_removed_spy), 3)
        self.assertEqual(len(layer_will_be_removed_spy_str), 4)
        self.assertEqual(len(layer_will_be_removed_spy_layer), 4)
        self.assertEqual(len(layers_removed_spy), 3)
        self.assertEqual(len(layer_removed_spy), 4)
        self.assertEqual(len(remove_all_spy), 1)

    def test_RemoveLayerShouldNotSegFault(self):
        QgsMapLayerRegistry.instance().removeAllMapLayers()

        reg = QgsMapLayerRegistry.instance()
        # Should not segfault
        reg.removeMapLayers(['not_exists'])
        reg.removeMapLayer('not_exists2')

        # check also that the removal of an unexistant layer does not insert a null layer
        for k, layer in reg.mapLayers().items():
            assert(layer is not None)


if __name__ == '__main__':
    unittest.main()
