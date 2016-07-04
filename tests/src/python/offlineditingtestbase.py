# -*- coding: utf-8 -*-
"""QGIS Unit test utils for offline editing tests.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
from __future__ import print_function
from builtins import str
from builtins import object
__author__ = 'Alessandro Pasotti'
__date__ = '2016-06-30'
__copyright__ = 'Copyright 2016, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from time import sleep

from qgis.core import (
    QgsFeature,
    QgsGeometry,
    QgsPoint,
    QgsFeatureRequest,
    QgsExpression,
    QgsMapLayerRegistry,
    QgsOfflineEditing,
)


# Tet features, fields: [id, name, geometry]
# "id" is used as a pk to retriev features by attribute
TEST_FEATURES = [
    (1, 'name 1', QgsPoint(9, 45)),
    (2, 'name 2', QgsPoint(9.5, 45.5)),
    (3, 'name 3', QgsPoint(9.5, 46)),
    (4, 'name 4', QgsPoint(10, 46.5)),
]


class OfflineTestBase(object):

    """Generic test methods for all online providers"""

    def _setUp(self):
        """Called by setUp: run before each test."""
        # Setup: create some features for the test layer
        features = []
        layer = self._getLayer('test_point')
        for id, name, geom in TEST_FEATURES:
            f = QgsFeature(layer.pendingFields())
            f['id'] = id
            f['name'] = name
            f.setGeometry(QgsGeometry.fromPoint(geom))
            features.append(f)
        layer.dataProvider().addFeatures(features)
        # Add the remote layer
        self.registry = QgsMapLayerRegistry.instance()
        self.registry.removeAllMapLayers()
        assert self.registry.addMapLayer(self._getOnlineLayer('test_point')) is not None

    def _tearDown(self):
        """Called by tearDown: run after each test."""
        # Clear test layers
        self._clearLayer('test_point')

    @classmethod
    def _compareFeature(cls, layer, attributes):
        """Compare id, name and geometry"""
        f = cls._getFeatureByAttribute(layer, 'id', attributes[0])
        return f['name'] == attributes[1] and f.geometry().asPoint().toString() == attributes[2].toString()

    @classmethod
    def _clearLayer(cls, layer_name):
        """
        Delete all features from the backend layer
        """
        layer = cls._getLayer(layer_name)
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

    def test_offlineConversion(self):
        # goes offline
        ol = QgsOfflineEditing()
        online_layer = list(self.registry.mapLayers().values())[0]
        self.assertTrue(online_layer.hasGeometryType())
        # Check we have 3 features
        self.assertEqual(len([f for f in online_layer.getFeatures()]), len(TEST_FEATURES))
        self.assertTrue(ol.convertToOfflineProject(self.temp_path, 'offlineDbFile.sqlite', [online_layer.id()]))
        offline_layer = list(self.registry.mapLayers().values())[0]
        self.assertTrue(offline_layer.hasGeometryType())
        self.assertTrue(offline_layer.isValid())
        self.assertTrue(offline_layer.name().find('(offline)') > -1)
        self.assertEqual(len([f for f in offline_layer.getFeatures()]), len(TEST_FEATURES))
        # Edit feature 2
        feat2 = self._getFeatureByAttribute(offline_layer, 'name', "'name 2'")
        self.assertTrue(offline_layer.startEditing())
        self.assertTrue(offline_layer.changeAttributeValue(feat2.id(), offline_layer.fieldNameIndex('name'), 'name 2 edited'))
        self.assertTrue(offline_layer.changeGeometry(feat2.id(), QgsGeometry.fromPoint(QgsPoint(33.0, 60.0))))
        self.assertTrue(offline_layer.commitChanges())
        feat2 = self._getFeatureByAttribute(offline_layer, 'name', "'name 2 edited'")
        self.assertTrue(ol.isOfflineProject())
        # Sync
        ol.synchronize()
        # Does anybody know why the sleep is needed? Is that a threaded WFS consequence?
        sleep(1)
        online_layer = list(self.registry.mapLayers().values())[0]
        self.assertTrue(online_layer.isValid())
        self.assertFalse(online_layer.name().find('(offline)') > -1)
        self.assertEqual(len([f for f in online_layer.getFeatures()]), len(TEST_FEATURES))
        # Check that data have changed in the backend (raise exception if not found)
        feat2 = self._getFeatureByAttribute(self._getLayer('test_point'), 'name', "'name 2 edited'")
        feat2 = self._getFeatureByAttribute(online_layer, 'name', "'name 2 edited'")
        self.assertEqual(feat2.geometry().asPoint().toString(), QgsPoint(33.0, 60.0).toString())
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
        self.assertTrue(offline_layer.changeAttributeValue(feat2.id(), offline_layer.fieldNameIndex('name'), 'name 2'))
        self.assertTrue(offline_layer.changeGeometry(feat2.id(), QgsGeometry.fromPoint(TEST_FEATURES[1][2])))
        # Edit feat 4
        feat4 = self._getFeatureByAttribute(offline_layer, 'name', "'name 4'")
        self.assertTrue(offline_layer.changeAttributeValue(feat4.id(), offline_layer.fieldNameIndex('name'), 'name 4 edited'))
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
