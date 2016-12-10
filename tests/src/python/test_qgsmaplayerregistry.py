# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsProject.

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

from qgis.core import QgsProject, QgsVectorLayer, QgsMapLayer
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


class TestQgsProjectMapLayers(unittest.TestCase):

    def setUp(self):
        pass

    def testInstance(self):
        """ test retrieving global instance """
        self.assertTrue(QgsProject.instance())

        # register a layer to the singleton
        QgsProject.instance().addMapLayer(createLayer('test'))

        # check that the same instance is returned
        self.assertEqual(len(QgsProject.instance().mapLayersByName('test')), 1)

        QgsProject.instance().removeAllMapLayers()

    def test_addMapLayer(self):
        """ test adding individual map layers to registry """
        QgsProject.instance().removeAllMapLayers()

        l1 = createLayer('test')
        self.assertEqual(QgsProject.instance().addMapLayer(l1), l1)
        self.assertEqual(len(QgsProject.instance().mapLayersByName('test')), 1)
        self.assertEqual(QgsProject.instance().count(), 1)

        # adding a second layer should leave existing layers intact
        l2 = createLayer('test2')
        self.assertEqual(QgsProject.instance().addMapLayer(l2), l2)
        self.assertEqual(len(QgsProject.instance().mapLayersByName('test')), 1)
        self.assertEqual(len(QgsProject.instance().mapLayersByName('test2')), 1)
        self.assertEqual(QgsProject.instance().count(), 2)

        QgsProject.instance().removeAllMapLayers()

    def test_addMapLayerAlreadyAdded(self):
        """ test that already added layers can't be readded to registry """
        QgsProject.instance().removeAllMapLayers()

        l1 = createLayer('test')
        QgsProject.instance().addMapLayer(l1)
        self.assertEqual(len(QgsProject.instance().mapLayersByName('test')), 1)
        self.assertEqual(QgsProject.instance().count(), 1)
        self.assertEqual(QgsProject.instance().addMapLayer(l1), None)
        self.assertEqual(len(QgsProject.instance().mapLayersByName('test')), 1)
        self.assertEqual(QgsProject.instance().count(), 1)

        QgsProject.instance().removeAllMapLayers()

    def test_addMapLayerInvalid(self):
        """ test that invalid map layersd can't be added to registry """
        QgsProject.instance().removeAllMapLayers()

        self.assertEqual(QgsProject.instance().addMapLayer(QgsVectorLayer("Point?field=x:string", 'test', "xxx")), None)
        self.assertEqual(len(QgsProject.instance().mapLayersByName('test')), 0)
        self.assertEqual(QgsProject.instance().count(), 0)

        QgsProject.instance().removeAllMapLayers()

    @unittest.skipIf(not use_signal_spy, "No QSignalSpy available")
    def test_addMapLayerSignals(self):
        """ test that signals are correctly emitted when adding map layer"""

        QgsProject.instance().removeAllMapLayers()

        layer_was_added_spy = QSignalSpy(QgsProject.instance().layerWasAdded)
        layers_added_spy = QSignalSpy(QgsProject.instance().layersAdded)
        legend_layers_added_spy = QSignalSpy(QgsProject.instance().legendLayersAdded)

        l1 = createLayer('test')
        QgsProject.instance().addMapLayer(l1)

        # can't seem to actually test the data which was emitted, so best we can do is test
        # the signal count
        self.assertEqual(len(layer_was_added_spy), 1)
        self.assertEqual(len(layers_added_spy), 1)
        self.assertEqual(len(legend_layers_added_spy), 1)

        # layer not added to legend
        QgsProject.instance().addMapLayer(createLayer('test2'), False)
        self.assertEqual(len(layer_was_added_spy), 2)
        self.assertEqual(len(layers_added_spy), 2)
        self.assertEqual(len(legend_layers_added_spy), 1)

        # try readding a layer already in the registry
        QgsProject.instance().addMapLayer(l1)
        # should be no extra signals emitted
        self.assertEqual(len(layer_was_added_spy), 2)
        self.assertEqual(len(layers_added_spy), 2)
        self.assertEqual(len(legend_layers_added_spy), 1)

    def test_addMapLayers(self):
        """ test adding multiple map layers to registry """
        QgsProject.instance().removeAllMapLayers()

        l1 = createLayer('test')
        l2 = createLayer('test2')
        self.assertEqual(set(QgsProject.instance().addMapLayers([l1, l2])), set([l1, l2]))
        self.assertEqual(len(QgsProject.instance().mapLayersByName('test')), 1)
        self.assertEqual(len(QgsProject.instance().mapLayersByName('test2')), 1)
        self.assertEqual(QgsProject.instance().count(), 2)

        # adding more layers should leave existing layers intact
        l3 = createLayer('test3')
        l4 = createLayer('test4')
        self.assertEqual(set(QgsProject.instance().addMapLayers([l3, l4])), set([l3, l4]))
        self.assertEqual(len(QgsProject.instance().mapLayersByName('test')), 1)
        self.assertEqual(len(QgsProject.instance().mapLayersByName('test2')), 1)
        self.assertEqual(len(QgsProject.instance().mapLayersByName('test3')), 1)
        self.assertEqual(len(QgsProject.instance().mapLayersByName('test4')), 1)
        self.assertEqual(QgsProject.instance().count(), 4)

        QgsProject.instance().removeAllMapLayers()

    def test_addMapLayersInvalid(self):
        """ test that invalid map layersd can't be added to registry """
        QgsProject.instance().removeAllMapLayers()

        self.assertEqual(QgsProject.instance().addMapLayers([QgsVectorLayer("Point?field=x:string", 'test', "xxx")]), [])
        self.assertEqual(len(QgsProject.instance().mapLayersByName('test')), 0)
        self.assertEqual(QgsProject.instance().count(), 0)

        QgsProject.instance().removeAllMapLayers()

    def test_addMapLayersAlreadyAdded(self):
        """ test that already added layers can't be readded to registry """
        QgsProject.instance().removeAllMapLayers()

        l1 = createLayer('test')
        self.assertEqual(QgsProject.instance().addMapLayers([l1]), [l1])
        self.assertEqual(len(QgsProject.instance().mapLayersByName('test')), 1)
        self.assertEqual(QgsProject.instance().count(), 1)
        self.assertEqual(QgsProject.instance().addMapLayers([l1]), [])
        self.assertEqual(len(QgsProject.instance().mapLayersByName('test')), 1)
        self.assertEqual(QgsProject.instance().count(), 1)

        QgsProject.instance().removeAllMapLayers()

    @unittest.skipIf(not use_signal_spy, "No QSignalSpy available")
    def test_addMapLayersSignals(self):
        """ test that signals are correctly emitted when adding map layers"""
        QgsProject.instance().removeAllMapLayers()

        layer_was_added_spy = QSignalSpy(QgsProject.instance().layerWasAdded)
        layers_added_spy = QSignalSpy(QgsProject.instance().layersAdded)
        legend_layers_added_spy = QSignalSpy(QgsProject.instance().legendLayersAdded)

        l1 = createLayer('test')
        l2 = createLayer('test2')
        QgsProject.instance().addMapLayers([l1, l2])

        # can't seem to actually test the data which was emitted, so best we can do is test
        # the signal count
        self.assertEqual(len(layer_was_added_spy), 2)
        self.assertEqual(len(layers_added_spy), 1)
        self.assertEqual(len(legend_layers_added_spy), 1)

        # layer not added to legend
        QgsProject.instance().addMapLayers([createLayer('test3'), createLayer('test4')], False)
        self.assertEqual(len(layer_was_added_spy), 4)
        self.assertEqual(len(layers_added_spy), 2)
        self.assertEqual(len(legend_layers_added_spy), 1)

        # try readding a layer already in the registry
        QgsProject.instance().addMapLayers([l1, l2])
        # should be no extra signals emitted
        self.assertEqual(len(layer_was_added_spy), 4)
        self.assertEqual(len(layers_added_spy), 2)
        self.assertEqual(len(legend_layers_added_spy), 1)

    def test_mapLayerById(self):
        """ test retrieving map layer by ID """
        QgsProject.instance().removeAllMapLayers()

        # test no crash with empty registry
        self.assertEqual(QgsProject.instance().mapLayer('bad'), None)
        self.assertEqual(QgsProject.instance().mapLayer(None), None)

        l1 = createLayer('test')
        l2 = createLayer('test2')

        QgsProject.instance().addMapLayers([l1, l2])

        self.assertEqual(QgsProject.instance().mapLayer('bad'), None)
        self.assertEqual(QgsProject.instance().mapLayer(None), None)
        self.assertEqual(QgsProject.instance().mapLayer(l1.id()), l1)
        self.assertEqual(QgsProject.instance().mapLayer(l2.id()), l2)

    def test_mapLayersByName(self):
        """ test retrieving map layer by name """
        QgsProject.instance().removeAllMapLayers()

        # test no crash with empty registry
        self.assertEqual(QgsProject.instance().mapLayersByName('bad'), [])
        self.assertEqual(QgsProject.instance().mapLayersByName(None), [])

        l1 = createLayer('test')
        l2 = createLayer('test2')

        QgsProject.instance().addMapLayers([l1, l2])

        self.assertEqual(QgsProject.instance().mapLayersByName('bad'), [])
        self.assertEqual(QgsProject.instance().mapLayersByName(None), [])
        self.assertEqual(QgsProject.instance().mapLayersByName('test'), [l1])
        self.assertEqual(QgsProject.instance().mapLayersByName('test2'), [l2])

        #duplicate name
        l3 = createLayer('test')
        QgsProject.instance().addMapLayer(l3)
        self.assertEqual(set(QgsProject.instance().mapLayersByName('test')), set([l1, l3]))

    def test_mapLayers(self):
        """ test retrieving map layers list """
        QgsProject.instance().removeAllMapLayers()

        # test no crash with empty registry
        self.assertEqual(QgsProject.instance().mapLayers(), {})

        l1 = createLayer('test')
        l2 = createLayer('test2')

        QgsProject.instance().addMapLayers([l1, l2])

        self.assertEqual(QgsProject.instance().mapLayers(), {l1.id(): l1, l2.id(): l2})

    def test_removeMapLayersById(self):
        """ test removing map layers by ID """
        QgsProject.instance().removeAllMapLayers()

        # test no crash with empty registry
        QgsProject.instance().removeMapLayers(['bad'])
        QgsProject.instance().removeMapLayers([None])

        l1 = createLayer('test')
        l2 = createLayer('test2')
        l3 = createLayer('test3')

        QgsProject.instance().addMapLayers([l1, l2, l3])
        self.assertEqual(QgsProject.instance().count(), 3)

        #remove bad layers
        QgsProject.instance().removeMapLayers(['bad'])
        self.assertEqual(QgsProject.instance().count(), 3)
        QgsProject.instance().removeMapLayers([None])
        self.assertEqual(QgsProject.instance().count(), 3)

        # remove valid layers
        l1_id = l1.id()
        QgsProject.instance().removeMapLayers([l1_id])
        self.assertEqual(QgsProject.instance().count(), 2)
        # double remove
        QgsProject.instance().removeMapLayers([l1_id])
        self.assertEqual(QgsProject.instance().count(), 2)

        # test that layer has been deleted
        self.assertTrue(sip.isdeleted(l1))

        # remove multiple
        QgsProject.instance().removeMapLayers([l2.id(), l3.id()])
        self.assertEqual(QgsProject.instance().count(), 0)
        self.assertTrue(sip.isdeleted(l2))

        # try removing a layer not in the registry
        l4 = createLayer('test4')
        QgsProject.instance().removeMapLayers([l4.id()])
        self.assertFalse(sip.isdeleted(l4))

    # fails on qt5 due to removeMapLayers list type conversion - needs a PyName alias
    # added to removeMapLayers for QGIS 3.0
    @unittest.expectedFailure(QT_VERSION_STR[0] == '5')
    def test_removeMapLayersByLayer(self):
        """ test removing map layers by layer"""
        QgsProject.instance().removeAllMapLayers()

        # test no crash with empty registry
        QgsProject.instance().removeMapLayers([None])

        l1 = createLayer('test')
        l2 = createLayer('test2')
        l3 = createLayer('test3')

        QgsProject.instance().addMapLayers([l1, l2, l3])
        self.assertEqual(QgsProject.instance().count(), 3)

        #remove bad layers
        QgsProject.instance().removeMapLayers([None])
        self.assertEqual(QgsProject.instance().count(), 3)

        # remove valid layers
        QgsProject.instance().removeMapLayers([l1])
        self.assertEqual(QgsProject.instance().count(), 2)

        # test that layer has been deleted
        self.assertTrue(sip.isdeleted(l1))

        # remove multiple
        QgsProject.instance().removeMapLayers([l2, l3])
        self.assertEqual(QgsProject.instance().count(), 0)
        self.assertTrue(sip.isdeleted(l2))
        self.assertTrue(sip.isdeleted(l3))

    def test_removeMapLayerById(self):
        """ test removing a map layer by ID """
        QgsProject.instance().removeAllMapLayers()

        # test no crash with empty registry
        QgsProject.instance().removeMapLayer('bad')
        QgsProject.instance().removeMapLayer(None)

        l1 = createLayer('test')
        l2 = createLayer('test2')

        QgsProject.instance().addMapLayers([l1, l2])
        self.assertEqual(QgsProject.instance().count(), 2)

        #remove bad layers
        QgsProject.instance().removeMapLayer('bad')
        self.assertEqual(QgsProject.instance().count(), 2)
        QgsProject.instance().removeMapLayer(None)
        self.assertEqual(QgsProject.instance().count(), 2)

        # remove valid layers
        l1_id = l1.id()
        QgsProject.instance().removeMapLayer(l1_id)
        self.assertEqual(QgsProject.instance().count(), 1)
        # double remove
        QgsProject.instance().removeMapLayer(l1_id)
        self.assertEqual(QgsProject.instance().count(), 1)

        # test that layer has been deleted
        self.assertTrue(sip.isdeleted(l1))

        # remove second layer
        QgsProject.instance().removeMapLayer(l2.id())
        self.assertEqual(QgsProject.instance().count(), 0)
        self.assertTrue(sip.isdeleted(l2))

        # try removing a layer not in the registry
        l3 = createLayer('test3')
        QgsProject.instance().removeMapLayer(l3.id())
        self.assertFalse(sip.isdeleted(l3))

    def test_removeMapLayerByLayer(self):
        """ test removing a map layer by layer """
        QgsProject.instance().removeAllMapLayers()

        # test no crash with empty registry
        QgsProject.instance().removeMapLayer('bad')
        QgsProject.instance().removeMapLayer(None)

        l1 = createLayer('test')
        l2 = createLayer('test2')

        QgsProject.instance().addMapLayers([l1, l2])
        self.assertEqual(QgsProject.instance().count(), 2)

        #remove bad layers
        QgsProject.instance().removeMapLayer(None)
        self.assertEqual(QgsProject.instance().count(), 2)
        l3 = createLayer('test3')
        QgsProject.instance().removeMapLayer(l3)
        self.assertEqual(QgsProject.instance().count(), 2)

        # remove valid layers
        QgsProject.instance().removeMapLayer(l1)
        self.assertEqual(QgsProject.instance().count(), 1)

        # test that layer has been deleted
        self.assertTrue(sip.isdeleted(l1))

        # remove second layer
        QgsProject.instance().removeMapLayer(l2)
        self.assertEqual(QgsProject.instance().count(), 0)
        self.assertTrue(sip.isdeleted(l2))

        # try removing a layer not in the registry
        l3 = createLayer('test3')
        QgsProject.instance().removeMapLayer(l3)
        self.assertFalse(sip.isdeleted(l3))

    def test_removeAllMapLayers(self):
        """ test removing all map layers from registry """
        QgsProject.instance().removeAllMapLayers()
        l1 = createLayer('test')
        l2 = createLayer('test2')

        QgsProject.instance().addMapLayers([l1, l2])
        self.assertEqual(QgsProject.instance().count(), 2)
        QgsProject.instance().removeAllMapLayers()
        self.assertEqual(QgsProject.instance().count(), 0)
        self.assertEqual(QgsProject.instance().mapLayersByName('test'), [])
        self.assertEqual(QgsProject.instance().mapLayersByName('test2'), [])

    @unittest.skipIf(not use_signal_spy, "No QSignalSpy available")
    def test_addRemoveLayersSignals(self):
        """ test that signals are correctly emitted when removing map layers"""
        QgsProject.instance().removeAllMapLayers()

        layers_will_be_removed_spy = QSignalSpy(QgsProject.instance().layersWillBeRemoved)
        layer_will_be_removed_spy_str = QSignalSpy(QgsProject.instance().layerWillBeRemoved[str])
        layer_will_be_removed_spy_layer = QSignalSpy(QgsProject.instance().layerWillBeRemoved[QgsMapLayer])
        layers_removed_spy = QSignalSpy(QgsProject.instance().layersRemoved)
        layer_removed_spy = QSignalSpy(QgsProject.instance().layerRemoved)
        remove_all_spy = QSignalSpy(QgsProject.instance().removeAll)

        l1 = createLayer('l1')
        l2 = createLayer('l2')
        l3 = createLayer('l3')
        l4 = createLayer('l4')
        QgsProject.instance().addMapLayers([l1, l2, l3, l4])

        # remove 1 layer
        QgsProject.instance().removeMapLayer(l1)
        # can't seem to actually test the data which was emitted, so best we can do is test
        # the signal count
        self.assertEqual(len(layers_will_be_removed_spy), 1)
        self.assertEqual(len(layer_will_be_removed_spy_str), 1)
        self.assertEqual(len(layer_will_be_removed_spy_layer), 1)
        self.assertEqual(len(layers_removed_spy), 1)
        self.assertEqual(len(layer_removed_spy), 1)
        self.assertEqual(len(remove_all_spy), 0)
        self.assertEqual(QgsProject.instance().count(), 3)

        # remove 2 layers at once
        QgsProject.instance().removeMapLayers([l2.id(), l3.id()])
        self.assertEqual(len(layers_will_be_removed_spy), 2)
        self.assertEqual(len(layer_will_be_removed_spy_str), 3)
        self.assertEqual(len(layer_will_be_removed_spy_layer), 3)
        self.assertEqual(len(layers_removed_spy), 2)
        self.assertEqual(len(layer_removed_spy), 3)
        self.assertEqual(len(remove_all_spy), 0)
        self.assertEqual(QgsProject.instance().count(), 1)

        # remove all
        QgsProject.instance().removeAllMapLayers()
        self.assertEqual(len(layers_will_be_removed_spy), 3)
        self.assertEqual(len(layer_will_be_removed_spy_str), 4)
        self.assertEqual(len(layer_will_be_removed_spy_layer), 4)
        self.assertEqual(len(layers_removed_spy), 3)
        self.assertEqual(len(layer_removed_spy), 4)
        self.assertEqual(len(remove_all_spy), 1)

        #remove some layers which aren't in the registry
        QgsProject.instance().removeMapLayers(['asdasd'])
        self.assertEqual(len(layers_will_be_removed_spy), 3)
        self.assertEqual(len(layer_will_be_removed_spy_str), 4)
        self.assertEqual(len(layer_will_be_removed_spy_layer), 4)
        self.assertEqual(len(layers_removed_spy), 3)
        self.assertEqual(len(layer_removed_spy), 4)
        self.assertEqual(len(remove_all_spy), 1)

        l5 = createLayer('test5')
        QgsProject.instance().removeMapLayer(l5)
        self.assertEqual(len(layers_will_be_removed_spy), 3)
        self.assertEqual(len(layer_will_be_removed_spy_str), 4)
        self.assertEqual(len(layer_will_be_removed_spy_layer), 4)
        self.assertEqual(len(layers_removed_spy), 3)
        self.assertEqual(len(layer_removed_spy), 4)
        self.assertEqual(len(remove_all_spy), 1)

    def test_RemoveLayerShouldNotSegFault(self):
        QgsProject.instance().removeAllMapLayers()

        reg = QgsProject.instance()
        # Should not segfault
        reg.removeMapLayers(['not_exists'])
        reg.removeMapLayer('not_exists2')

        # check also that the removal of an unexistant layer does not insert a null layer
        for k, layer in list(reg.mapLayers().items()):
            assert(layer is not None)


if __name__ == '__main__':
    unittest.main()
