# -*- coding: utf-8 -*-
"""QGIS Unit test utils for offline editing tests.

There are three layers referenced through the code:

- the "online_layer" is the layer being edited online (WFS or PostGIS) layer inside
  QGIS client
- the "offline_layer" (SQLite)
- the "layer", is the shapefile layer that is served by QGIS Server WFS, in case of
  PostGIS, this will be the same layer referenced by online_layer

Each test simulates one working session.

When testing on PostGIS, the first two layers will be exactly the same object.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

from builtins import object

__author__ = 'Alessandro Pasotti'
__date__ = '2016-06-30'
__copyright__ = 'Copyright 2016, The QGIS Project'

from time import sleep

from qgis.core import (
    QgsFeature,
    QgsGeometry,
    QgsPointXY,
    QgsFeatureRequest,
    QgsExpression,
    QgsProject,
    QgsOfflineEditing,
)


# Tet features, fields: [id, name, geometry]
# "id" is used as a pk to retrieve features by attribute
TEST_FEATURES = [
    (1, 'name 1', QgsPointXY(9, 45)),
    (2, 'name 2', QgsPointXY(9.5, 45.5)),
    (3, 'name 3', QgsPointXY(9.5, 46)),
    (4, 'name 4', QgsPointXY(10, 46.5)),
]

# Additional features for insert test
TEST_FEATURES_INSERT = [
    (5, 'name 5', QgsPointXY(9.7, 45.7)),
    (6, 'name 6', QgsPointXY(10.6, 46.6)),
]


class OfflineTestBase(object):

    """Generic test methods for all online providers"""

    def _setUp(self):
        """Called by setUp: run before each test."""
        # Setup: create some features for the test layer
        features = []
        layer = self._getLayer('test_point')
        assert layer.startEditing()
        for id, name, geom in TEST_FEATURES:
            f = QgsFeature(layer.fields())
            f['id'] = id
            f['name'] = name
            f.setGeometry(QgsGeometry.fromPointXY(geom))
            features.append(f)
        layer.addFeatures(features)
        assert layer.commitChanges()
        # Add the online layer
        self.registry = QgsProject.instance()
        self.registry.removeAllMapLayers()
        assert self.registry.addMapLayer(self._getOnlineLayer('test_point')) is not None

    def _tearDown(self):
        """Called by tearDown: run after each test."""
        # Delete the sqlite db
        #os.unlink(os.path.join(self.temp_path, 'offlineDbFile.sqlite'))
        pass

    @classmethod
    def _compareFeature(cls, layer, attributes):
        """Compare id, name and geometry"""
        f = cls._getFeatureByAttribute(layer, 'id', attributes[0])
        return f['name'] == attributes[1] and f.geometry().asPoint().toString() == attributes[2].toString()

    @classmethod
    def _clearLayer(cls, layer):
        """
        Delete all features from the given layer
        """
        layer.startEditing()
        layer.deleteFeatures([f.id() for f in layer.getFeatures()])
        layer.commitChanges()
        assert layer.featureCount() == 0

    @classmethod
    def _getLayer(cls, layer_name):
        """
        Layer factory (return the backend layer), provider specific
        """
        raise NotImplementedError

    @classmethod
    def _getOnlineLayer(cls, type_name, layer_name=None):
        """
        Layer factory (return the online layer), provider specific
        """
        raise NotImplementedError

    @classmethod
    def _getFeatureByAttribute(cls, layer, attr_name, attr_value):
        """
        Find the feature and return it, raise exception if not found
        """
        request = QgsFeatureRequest(QgsExpression("%s=%s" % (attr_name,
                                                             attr_value)))
        try:
            return next(layer.dataProvider().getFeatures(request))
        except StopIteration:
            raise Exception("Wrong attributes in WFS layer %s" %
                            layer.name())

    def _testInit(self):
        """
        Preliminary checks for each test
        """
        # goes offline
        ol = QgsOfflineEditing()
        online_layer = list(self.registry.mapLayers().values())[0]
        self.assertTrue(online_layer.isSpatial())
        # Check we have features
        self.assertEqual(len([f for f in online_layer.getFeatures()]), len(TEST_FEATURES))
        self.assertTrue(ol.convertToOfflineProject(self.temp_path, 'offlineDbFile.sqlite', [online_layer.id()]))
        offline_layer = list(self.registry.mapLayers().values())[0]
        self.assertTrue(offline_layer.isSpatial())
        self.assertTrue(offline_layer.isValid())
        self.assertTrue(offline_layer.name().find('(offline)') > -1)
        self.assertEqual(len([f for f in offline_layer.getFeatures()]), len(TEST_FEATURES))
        return ol, offline_layer

    def test_updateFeatures(self):
        ol, offline_layer = self._testInit()
        # Edit feature 2
        feat2 = self._getFeatureByAttribute(offline_layer, 'name', "'name 2'")
        self.assertTrue(offline_layer.startEditing())
        self.assertTrue(offline_layer.changeAttributeValue(feat2.id(), offline_layer.fields().lookupField('name'), 'name 2 edited'))
        self.assertTrue(offline_layer.changeGeometry(feat2.id(), QgsGeometry.fromPointXY(QgsPointXY(33.0, 60.0))))
        self.assertTrue(offline_layer.commitChanges())
        feat2 = self._getFeatureByAttribute(offline_layer, 'name', "'name 2 edited'")
        self.assertTrue(ol.isOfflineProject())
        # Sync
        ol.synchronize()
        sleep(2)
        # Does anybody know why the sleep is needed? Is that a threaded WFS consequence?
        online_layer = list(self.registry.mapLayers().values())[0]
        self.assertTrue(online_layer.isValid())
        self.assertFalse(online_layer.name().find('(offline)') > -1)
        self.assertEqual(len([f for f in online_layer.getFeatures()]), len(TEST_FEATURES))
        # Check that data have changed in the backend (raise exception if not found)
        feat2 = self._getFeatureByAttribute(self._getLayer('test_point'), 'name', "'name 2 edited'")
        feat2 = self._getFeatureByAttribute(online_layer, 'name', "'name 2 edited'")
        self.assertEqual(feat2.geometry().asPoint().toString(), QgsPointXY(33.0, 60.0).toString())
        # Check that all other features have not changed
        layer = self._getLayer('test_point')
        self.assertTrue(self._compareFeature(layer, TEST_FEATURES[1 - 1]))
        self.assertTrue(self._compareFeature(layer, TEST_FEATURES[3 - 1]))
        self.assertTrue(self._compareFeature(layer, TEST_FEATURES[4 - 1]))

        # Test for regression on double sync (it was a SEGFAULT)
        # goes offline
        ol = QgsOfflineEditing()
        offline_layer = list(self.registry.mapLayers().values())[0]
        # Edit feature 2
        feat2 = self._getFeatureByAttribute(offline_layer, 'name', "'name 2 edited'")
        self.assertTrue(offline_layer.startEditing())
        self.assertTrue(offline_layer.changeAttributeValue(feat2.id(), offline_layer.fields().lookupField('name'), 'name 2'))
        self.assertTrue(offline_layer.changeGeometry(feat2.id(), QgsGeometry.fromPointXY(TEST_FEATURES[1][2])))
        # Edit feat 4
        feat4 = self._getFeatureByAttribute(offline_layer, 'name', "'name 4'")
        self.assertTrue(offline_layer.changeAttributeValue(feat4.id(), offline_layer.fields().lookupField('name'), 'name 4 edited'))
        self.assertTrue(offline_layer.commitChanges())
        # Sync
        ol.synchronize()
        # Does anybody knows why the sleep is needed? Is that a threaded WFS consequence?
        sleep(1)
        online_layer = list(self.registry.mapLayers().values())[0]
        layer = self._getLayer('test_point')
        # Check that data have changed in the backend (raise exception if not found)
        feat4 = self._getFeatureByAttribute(layer, 'name', "'name 4 edited'")
        feat4 = self._getFeatureByAttribute(online_layer, 'name', "'name 4 edited'")
        feat2 = self._getFeatureByAttribute(layer, 'name', "'name 2'")
        feat2 = self._getFeatureByAttribute(online_layer, 'name', "'name 2'")
        # Check that all other features have not changed
        layer = self._getLayer('test_point')
        self.assertTrue(self._compareFeature(layer, TEST_FEATURES[1 - 1]))
        self.assertTrue(self._compareFeature(layer, TEST_FEATURES[2 - 1]))
        self.assertTrue(self._compareFeature(layer, TEST_FEATURES[3 - 1]))

    def test_deleteOneFeature(self):
        """
        Delete a single feature
        """
        ol, offline_layer = self._testInit()
        # Delete feature 3
        feat3 = self._getFeatureByAttribute(offline_layer, 'name', "'name 3'")
        self.assertTrue(offline_layer.startEditing())
        self.assertTrue(offline_layer.deleteFeatures([feat3.id()]))
        self.assertTrue(offline_layer.commitChanges())
        self.assertRaises(Exception, lambda: self._getFeatureByAttribute(offline_layer, 'name', "'name 3'"))
        self.assertTrue(ol.isOfflineProject())
        # Sync
        ol.synchronize()
        # Does anybody know why the sleep is needed? Is that a threaded WFS consequence?
        sleep(1)
        online_layer = list(self.registry.mapLayers().values())[0]
        self.assertTrue(online_layer.isValid())
        self.assertFalse(online_layer.name().find('(offline)') > -1)
        self.assertEqual(len([f for f in online_layer.getFeatures()]), len(TEST_FEATURES) - 1)
        # Check that data have changed in the backend (raise exception if not found)
        self.assertRaises(Exception, lambda: self._getFeatureByAttribute(online_layer, 'name', "'name 3'"))
        # Check that all other features have not changed
        layer = self._getLayer('test_point')
        self.assertTrue(self._compareFeature(layer, TEST_FEATURES[1 - 1]))
        self.assertTrue(self._compareFeature(layer, TEST_FEATURES[2 - 1]))
        self.assertTrue(self._compareFeature(layer, TEST_FEATURES[4 - 1]))

    def test_deleteMultipleFeatures(self):
        """
        Delete a multiple features
        """
        ol, offline_layer = self._testInit()
        # Delete feature 1 and 3
        feat1 = self._getFeatureByAttribute(offline_layer, 'name', "'name 1'")
        feat3 = self._getFeatureByAttribute(offline_layer, 'name', "'name 3'")
        self.assertTrue(offline_layer.startEditing())
        self.assertTrue(offline_layer.deleteFeatures([feat3.id(), feat1.id()]))
        self.assertTrue(offline_layer.commitChanges())
        self.assertRaises(Exception, lambda: self._getFeatureByAttribute(offline_layer, 'name', "'name 3'"))
        self.assertRaises(Exception, lambda: self._getFeatureByAttribute(offline_layer, 'name', "'name 1'"))
        self.assertTrue(ol.isOfflineProject())
        # Sync
        ol.synchronize()
        # Does anybody know why the sleep is needed? Is that a threaded WFS consequence?
        sleep(1)
        online_layer = list(self.registry.mapLayers().values())[0]
        self.assertTrue(online_layer.isValid())
        self.assertFalse(online_layer.name().find('(offline)') > -1)
        self.assertEqual(len([f for f in online_layer.getFeatures()]), len(TEST_FEATURES) - 2)
        # Check that data have changed in the backend (raise exception if not found)
        self.assertRaises(Exception, lambda: self._getFeatureByAttribute(online_layer, 'name', "'name 3'"))
        self.assertRaises(Exception, lambda: self._getFeatureByAttribute(online_layer, 'name', "'name 1'"))
        # Check that all other features have not changed
        layer = self._getLayer('test_point')
        self.assertTrue(self._compareFeature(layer, TEST_FEATURES[2 - 1]))
        self.assertTrue(self._compareFeature(layer, TEST_FEATURES[4 - 1]))

    def test_InsertFeatures(self):
        """
        Insert multiple features
        """
        ol, offline_layer = self._testInit()
        # Insert feature 5 and 6
        self.assertTrue(offline_layer.startEditing())
        features = []
        for id, name, geom in TEST_FEATURES_INSERT:
            f = QgsFeature(offline_layer.fields())
            f['id'] = id
            f['name'] = name
            f.setGeometry(QgsGeometry.fromPointXY(geom))
            features.append(f)
        offline_layer.addFeatures(features)
        self.assertTrue(offline_layer.commitChanges())
        self._getFeatureByAttribute(offline_layer, 'name', "'name 5'")
        self._getFeatureByAttribute(offline_layer, 'name', "'name 6'")
        self.assertTrue(ol.isOfflineProject())
        # Sync
        ol.synchronize()
        # Does anybody know why the sleep is needed? Is that a threaded WFS consequence?
        sleep(1)
        online_layer = list(self.registry.mapLayers().values())[0]
        self.assertTrue(online_layer.isValid())
        self.assertFalse(online_layer.name().find('(offline)') > -1)
        self.assertEqual(len([f for f in online_layer.getFeatures()]), len(TEST_FEATURES) + 2)
        # Check that data have changed in the backend (raise exception if not found)
        self._getFeatureByAttribute(online_layer, 'name', "'name 5'")
        self._getFeatureByAttribute(online_layer, 'name', "'name 6'")
        # Check that all other features have not changed
        layer = self._getLayer('test_point')
        self.assertTrue(self._compareFeature(layer, TEST_FEATURES[1 - 1]))
        self.assertTrue(self._compareFeature(layer, TEST_FEATURES[2 - 1]))
        self.assertTrue(self._compareFeature(layer, TEST_FEATURES[3 - 1]))
        self.assertTrue(self._compareFeature(layer, TEST_FEATURES[4 - 1]))
