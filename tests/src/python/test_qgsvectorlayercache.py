"""QGIS Unit tests for QgsVectorLayerCache.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "08/06/2017"
__copyright__ = "Copyright 2017, The QGIS Project"

import unittest

from featuresourcetestbase import FeatureSourceTestCase
from qgis.core import (
    NULL,
    QgsFeature,
    QgsFeatureRequest,
    QgsField,
    QgsGeometry,
    QgsPoint,
    QgsRectangle,
    QgsVectorLayer,
    QgsVectorLayerCache,
)
from qgis.PyQt.QtCore import QDate, QDateTime, QTime, QVariant
from qgis.PyQt.QtTest import QSignalSpy
from qgis.testing import QgisTestCase, start_app

start_app()


class TestQgsVectorLayerCache(QgisTestCase, FeatureSourceTestCase):
    @classmethod
    def getSource(cls):
        cache = QgsVectorLayerCache(cls.vl, 100)
        return cache

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()
        # Create test layer for FeatureSourceTestCase
        cls.vl = QgsVectorLayer(
            "Point?crs=epsg:4326&field=pk:integer&field=cnt:integer&field=name:string(0)&field=name2:string(0)&field=num_char:string&field=dt:datetime&field=date:date&field=time:time",
            "test",
            "memory",
        )
        assert cls.vl.isValid()

        f1 = QgsFeature(5)
        f1.setAttributes(
            [
                5,
                -200,
                NULL,
                "NuLl",
                "5",
                QDateTime(QDate(2020, 5, 4), QTime(12, 13, 14)),
                QDate(2020, 5, 2),
                QTime(12, 13, 1),
            ]
        )
        f1.setGeometry(QgsGeometry.fromWkt("Point (-71.123 78.23)"))

        f2 = QgsFeature(3)
        f2.setAttributes([3, 300, "Pear", "PEaR", "3", NULL, NULL, NULL])

        f3 = QgsFeature(1)
        f3.setAttributes(
            [
                1,
                100,
                "Orange",
                "oranGe",
                "1",
                QDateTime(QDate(2020, 5, 3), QTime(12, 13, 14)),
                QDate(2020, 5, 3),
                QTime(12, 13, 14),
            ]
        )
        f3.setGeometry(QgsGeometry.fromWkt("Point (-70.332 66.33)"))

        f4 = QgsFeature(2)
        f4.setAttributes(
            [
                2,
                200,
                "Apple",
                "Apple",
                "2",
                QDateTime(QDate(2020, 5, 4), QTime(12, 14, 14)),
                QDate(2020, 5, 4),
                QTime(12, 14, 14),
            ]
        )
        f4.setGeometry(QgsGeometry.fromWkt("Point (-68.2 70.8)"))

        f5 = QgsFeature(4)
        f5.setAttributes(
            [
                4,
                400,
                "Honey",
                "Honey",
                "4",
                QDateTime(QDate(2021, 5, 4), QTime(13, 13, 14)),
                QDate(2021, 5, 4),
                QTime(13, 13, 14),
            ]
        )
        f5.setGeometry(QgsGeometry.fromWkt("Point (-65.32 78.3)"))

        assert cls.vl.dataProvider().addFeatures([f1, f2, f3, f4, f5])
        cls.source = QgsVectorLayerCache(cls.vl, 100)

    def testGetFeaturesSubsetAttributes2(self):
        """Override and skip this QgsFeatureSource test. We are using a memory provider, and it's actually more efficient for the memory provider to return
        its features as direct copies (due to implicit sharing of QgsFeature)
        """
        pass

    def testGetFeaturesNoGeometry(self):
        """Override and skip this QgsFeatureSource test. We are using a memory provider, and it's actually more efficient for the memory provider to return
        its features as direct copies (due to implicit sharing of QgsFeature)
        """
        pass

    def testUniqueValues(self):
        """Skip unique values test - not implemented by the cache (yet)"""
        pass

    def testMinimumValue(self):
        """Skip min values test - not implemented by the cache (yet)"""
        pass

    def testMaximumValue(self):
        """Skip max values test - not implemented by the cache (yet)"""
        pass

    def testAllFeatureIds(self):
        """Skip allFeatureIds test - not implemented by the cache (yet)"""
        pass

    def testOpenIteratorAfterSourceRemoval(self):
        """
        Skip this test -- the iterators from the cache CANNOT be used after the cache is deleted
        """
        pass

    def _create_test_data(self, num_features=5, cache_size=10):
        layer = QgsVectorLayer(
            "Point?crs=EPSG:4326&field=name:string&field=val:int",
            "test_vector_layer",
            "memory",
        )
        provider = layer.dataProvider()

        features = []
        for i in range(1, num_features + 1):
            f = QgsFeature(layer.fields())
            f.setGeometry(QgsGeometry(QgsPoint(i * 1.0, i * 1.0)))
            f.setAttributes([f"item_{i}", i * 10])
            features.append(f)

        provider.addFeatures(features)
        layer.updateExtents()

        fids = [f.id() for f in layer.getFeatures()]
        cache = QgsVectorLayerCache(layer, cache_size)

        return layer, cache, fids

    def test_basic_cache_properties(self):
        layer, cache, _ = self._create_test_data(num_features=5)

        self.assertEqual(cache.layer(), layer)
        self.assertEqual(cache.sourceCrs().authid(), layer.crs().authid())
        self.assertEqual(cache.wkbType(), layer.wkbType())
        self.assertEqual(cache.fields().count(), len(layer.fields()))
        self.assertEqual(cache.featureCount(), 5)

    def test_python_methods(self):
        layer, cache, _ = self._create_test_data(num_features=5)
        self.assertEqual(len(cache), 5)
        self.assertTrue(bool(cache))

        # empty cache should still return True for bool conversion
        layer, cache, _ = self._create_test_data(num_features=0)
        self.assertTrue(bool(cache))

    def test_cache_size(self):
        layer, cache, _ = self._create_test_data(cache_size=10)
        self.assertEqual(cache.cacheSize(), 10)

        cache.setCacheSize(3)
        self.assertEqual(cache.cacheSize(), 3)

    def test_cache_geometry_toggle(self):
        layer, cache, _ = self._create_test_data()

        self.assertTrue(cache.cacheGeometry())

        cache.setCacheGeometry(False)
        self.assertFalse(cache.cacheGeometry())

        cache.setCacheGeometry(True)
        self.assertTrue(cache.cacheGeometry())

    def test_attribute_subset_configuration(self):
        layer, cache, _ = self._create_test_data()
        cache.setCacheAddedAttributes(False)
        cache.setCacheSubsetOfAttributes([0])
        self.assertEqual(cache.cacheSubsetOfAttributes(), [0])

        # add some new attributes, should not be in cache by default
        layer.startEditing()
        layer.addAttribute(QgsField("name2", QVariant.String))
        layer.addAttribute(QgsField("val2", QVariant.Int))
        self.assertEqual(cache.cacheSubsetOfAttributes(), [0])

        # retry with setCacheAddedAttributes
        layer, cache, _ = self._create_test_data()
        cache.setCacheAddedAttributes(True)
        cache.setCacheSubsetOfAttributes([0])
        self.assertEqual(cache.cacheSubsetOfAttributes(), [0])

        # add some new attributes, should be automatically added to cache
        layer.startEditing()
        layer.addAttribute(QgsField("name2", QVariant.String))
        layer.addAttribute(QgsField("val2", QVariant.Int))
        self.assertEqual(cache.cacheSubsetOfAttributes(), [0, 2, 3])

    def test_full_cache(self):
        layer, cache, fids = self._create_test_data(num_features=5)
        self.assertFalse(cache.hasFullCache())
        cached_ids = cache.cachedFeatureIds()
        self.assertFalse(cached_ids)
        for fid in fids:
            self.assertFalse(cache.isFidCached(fid))

        cache.setFullCache(True)
        self.assertTrue(cache.hasFullCache())

        cached_ids = cache.cachedFeatureIds()
        self.assertEqual(set(cached_ids), set(fids))
        for fid in fids:
            self.assertTrue(cache.isFidCached(fid))

    def test_get_feature_by_id(self):
        layer, cache, fids = self._create_test_data()

        target_fid = fids[0]
        feature = cache.getFeature(target_fid)

        self.assertTrue(feature.isValid())
        self.assertEqual(feature.id(), target_fid)
        self.assertEqual(feature.attribute("name"), "item_1")
        self.assertEqual(feature.attribute("val"), 10)

        target_fid = fids[1]
        feature = cache.getFeature(target_fid)

        self.assertTrue(feature.isValid())
        self.assertEqual(feature.id(), target_fid)
        self.assertEqual(feature.attribute("name"), "item_2")
        self.assertEqual(feature.attribute("val"), 20)

    def test_get_invalid_feature(self):
        layer, cache, fids = self._create_test_data()
        self.assertNotIn(99999, fids)
        feature = cache.getFeature(99999)
        self.assertFalse(feature.isValid())

    def test_feature_at_id(self):
        layer, cache, fids = self._create_test_data()
        target_fid = fids[1]

        ok, feat1 = cache.featureAtId(target_fid)
        self.assertTrue(ok)
        self.assertTrue(feat1.isValid())
        self.assertEqual(feat1.attribute("name"), "item_2")
        self.assertEqual(feat1.attribute("val"), 20)
        self.assertFalse(feat1.geometry().isEmpty())

        ok, feat2 = cache.featureAtIdWithAllAttributes(target_fid)
        self.assertTrue(ok)
        self.assertTrue(feat2.isValid())
        self.assertEqual(feat2.attribute("name"), "item_2")
        self.assertEqual(feat2.attribute("val"), 20)
        self.assertFalse(feat2.geometry().isEmpty())

        ok, feat3 = cache.completeFeatureAtId(target_fid)
        self.assertTrue(ok)
        self.assertTrue(feat3.isValid())
        self.assertEqual(feat3.attribute("name"), "item_2")
        self.assertEqual(feat3.attribute("val"), 20)
        self.assertFalse(feat3.geometry().isEmpty())

    def test_get_features_iterate_all(self):
        layer, cache, fids = self._create_test_data(num_features=5)

        self.assertCountEqual([f.id() for f in cache.getFeatures()], fids)

    def test_get_features_by_fids(self):
        layer, cache, fids = self._create_test_data(num_features=5)

        requested_ids = fids[:2]
        iterator = cache.getFeatures(requested_ids)
        retrieved_ids = [f.id() for f in iterator]

        self.assertCountEqual(retrieved_ids, requested_ids)

    def test_get_features_by_rectangle(self):
        layer, cache, _ = self._create_test_data(num_features=5)

        iterator = cache.getFeatures(QgsRectangle(0.5, 0.5, 2.5, 2.5))

        self.assertEqual(len(list(iterator)), 2)

    def test_get_features_by_expression(self):
        layer, cache, _ = self._create_test_data(num_features=5)
        iterator = cache.getFeatures('"val" >= 30')
        self.assertEqual(len(list(iterator)), 3)

    def test_remove_cached_feature(self):
        """Test removing a cached feature."""
        layer, cache, fids = self._create_test_data()
        cache.setFullCache(True)

        target_fid = fids[0]
        self.assertTrue(cache.isFidCached(target_fid))

        self.assertTrue(cache.removeCachedFeature(target_fid))
        self.assertFalse(cache.isFidCached(target_fid))

        # already removed
        self.assertFalse(cache.removeCachedFeature(target_fid))

    def test_layer_feature_added_no_full_cache(self):
        layer, cache, _ = self._create_test_data()

        about_to_be_added_spy = QSignalSpy(cache.featureAboutToBeAdded)
        added_spy = QSignalSpy(cache.featureAdded)

        new_features = []
        on_about_to_be_added_called = []
        on_added_called = []

        def _on_about_to_be_added_called(fid):
            nonlocal new_features
            nonlocal on_about_to_be_added_called
            self.assertEqual(fid, new_features[0].id())
            on_about_to_be_added_called.append(True)

            # at this point the feature must NOT be present in the cache
            self.assertFalse(cache.isFidCached(fid))

        cache.featureAboutToBeAdded.connect(_on_about_to_be_added_called)

        def _on_added_called(fid):
            nonlocal new_features
            nonlocal on_added_called
            self.assertEqual(fid, new_features[0].id())
            on_added_called.append(True)

            # still not present in the cache, we aren't doing a full cache
            self.assertFalse(cache.isFidCached(fid))

        cache.featureAdded.connect(_on_added_called)

        layer.startEditing()
        new_feat = QgsFeature(layer.fields())
        new_feat.setGeometry(QgsGeometry(QgsPoint(10.0, 10.0)))
        new_feat.setAttributes(["item_new", 99])
        new_features.append(new_feat)
        layer.addFeature(new_feat)

        self.assertEqual(len(about_to_be_added_spy), 1)
        self.assertEqual(about_to_be_added_spy[-1][0], new_feat.id())

        self.assertEqual(on_added_called, [True])

        self.assertEqual(len(added_spy), 1)
        self.assertEqual(added_spy[-1][0], new_feat.id())

        # by default newly added features AREN'T added to the cache!
        self.assertFalse(cache.isFidCached(new_feat.id()))

    def test_layer_feature_add_with_full_cache(self):
        layer, cache, _ = self._create_test_data()
        cache.setFullCache(True)

        about_to_be_added_spy = QSignalSpy(cache.featureAboutToBeAdded)
        added_spy = QSignalSpy(cache.featureAdded)

        new_features = []
        on_about_to_be_added_called = []
        on_added_called = []

        def _on_about_to_be_added_called(fid):
            nonlocal new_features
            nonlocal on_about_to_be_added_called
            self.assertEqual(fid, new_features[0].id())
            on_about_to_be_added_called.append(True)

            # at this point the feature must NOT be present in the cache
            self.assertFalse(cache.isFidCached(fid))

        cache.featureAboutToBeAdded.connect(_on_about_to_be_added_called)

        def _on_added_called(fid):
            nonlocal new_features
            nonlocal on_added_called
            self.assertEqual(fid, new_features[0].id())
            on_added_called.append(True)

            # with full cache newly added features ARE added to the cache!
            self.assertTrue(cache.isFidCached(fid))

        cache.featureAdded.connect(_on_added_called)

        layer.startEditing()
        new_feat = QgsFeature(layer.fields())
        new_feat.setGeometry(QgsGeometry(QgsPoint(10.0, 10.0)))
        new_feat.setAttributes(["item_new", 99])
        new_features.append(new_feat)
        layer.addFeature(new_feat)

        self.assertEqual(len(about_to_be_added_spy), 1)
        self.assertEqual(about_to_be_added_spy[-1][0], new_feat.id())

        self.assertEqual(on_added_called, [True])

        self.assertEqual(len(added_spy), 1)
        self.assertEqual(added_spy[-1][0], new_feat.id())

        # with full cache newly added features ARE added to the cache!
        self.assertTrue(cache.isFidCached(new_feat.id()))

    def test_layer_feature_add_and_commit(self):
        layer, cache, _ = self._create_test_data()
        cache.setFullCache(True)

        on_deleted_called_fids = []
        on_about_to_be_deleted_fids = []
        on_added_fids = []
        on_about_to_be_added_fids = []

        original_fids = set(f.id() for f in layer.getFeatures())

        def _on_about_to_be_deleted_called(fid):
            nonlocal on_about_to_be_deleted_fids
            on_about_to_be_deleted_fids.append(fid)

        cache.featureAboutToBeDeleted.connect(_on_about_to_be_deleted_called)

        def _on_deleted_called(fid):
            nonlocal on_deleted_called_fids
            on_deleted_called_fids.append(fid)

        cache.featureDeleted.connect(_on_deleted_called)

        def _on_about_to_be_added_called(fid):
            nonlocal on_about_to_be_added_fids
            on_about_to_be_added_fids.append(fid)

            # at this point the feature must NOT be present in the cache
            self.assertFalse(cache.isFidCached(fid))

        cache.featureAboutToBeAdded.connect(_on_about_to_be_added_called)

        def _on_added_called(fid):
            nonlocal on_added_fids
            on_added_fids.append(fid)

            # with full cache newly added features ARE added to the cache!
            self.assertTrue(cache.isFidCached(fid))

        cache.featureAdded.connect(_on_added_called)

        layer.startEditing()
        new_feat = QgsFeature(layer.fields())
        new_feat.setGeometry(QgsGeometry(QgsPoint(10.0, 10.0)))
        new_feat.setAttributes(["item_new", 99])

        layer.addFeature(new_feat)
        uncommitted_feature_id = new_feat.id()

        self.assertEqual(on_about_to_be_added_fids, [uncommitted_feature_id])
        self.assertEqual(on_added_fids, [uncommitted_feature_id])

        self.assertEqual(on_about_to_be_deleted_fids, [])
        self.assertEqual(on_deleted_called_fids, [])

        self.assertTrue(cache.isFidCached(uncommitted_feature_id))

        # committing changes
        layer.commitChanges()

        # commitChanges should result in the following:
        # - featureAboutToBeDeleted, with the uncommitted feature fid
        # - featureDeleted, with the uncommitted feature fid
        # - featureAboutToBeAdded, with the newly committed (provider level) feature fid
        # - featureAdded, with the newly committed (provider level) feature fid

        new_fids = set(f.id() for f in layer.getFeatures())
        new_feature_provider_fid = list(new_fids.difference(original_fids))[0]

        self.assertEqual(on_about_to_be_deleted_fids, [uncommitted_feature_id])
        self.assertEqual(on_deleted_called_fids, [uncommitted_feature_id])
        self.assertEqual(
            on_about_to_be_added_fids,
            [uncommitted_feature_id, new_feature_provider_fid],
        )
        self.assertEqual(
            on_added_fids, [uncommitted_feature_id, new_feature_provider_fid]
        )

        # uncommitted feature id should be evicted from cache
        self.assertFalse(cache.isFidCached(uncommitted_feature_id))
        # but newly committed provider feature id should be present
        self.assertTrue(cache.isFidCached(new_feature_provider_fid))

    def test_layer_attribute_changed_signal(self):
        layer, cache, fids = self._create_test_data()

        spy = QSignalSpy(cache.attributeValueChanged)

        target_fid = fids[0]
        layer.startEditing()
        layer.changeAttributeValue(target_fid, 0, "item_updated")
        layer.commitChanges()

        self.assertEqual(len(spy), 1)
        self.assertEqual(spy[-1][0], target_fid)
        self.assertEqual(spy[-1][1], 0)
        self.assertEqual(spy[-1][2], "item_updated")

    def test_cache_invalidation_on_attribute_addition(self):
        layer, cache, _ = self._create_test_data()

        spy = QSignalSpy(cache.invalidated)

        cache.setFullCache(True)
        self.assertTrue(cache.hasFullCache())

        layer.startEditing()
        layer.addAttribute(QgsField("new_col", QVariant.Double))
        layer.commitChanges()

        self.assertEqual(len(spy), 1)
        self.assertFalse(cache.hasFullCache())

    def test_has_full_cache(self):
        layer, cache, fids = self._create_test_data(num_features=5, cache_size=10)

        # empty cache, with capacity for 10 features
        self.assertEqual(len(cache.cachedFeatureIds()), 0)
        self.assertEqual(cache.cacheSize(), 10)
        self.assertFalse(cache.hasFullCache())

        # request all 5 features -- should fit in cache, as 5 < cache capacity of 10
        all_features = [f for f in cache.getFeatures()]
        self.assertEqual(len(all_features), 5)

        # all features fit in cache!
        self.assertTrue(cache.hasFullCache())

    def test_has_full_cache_too_small(self):
        layer, cache, fids = self._create_test_data(num_features=10, cache_size=5)

        # empty cache, with capacity for 5 features
        self.assertEqual(len(cache.cachedFeatureIds()), 0)
        self.assertEqual(cache.cacheSize(), 5)
        self.assertFalse(cache.hasFullCache())

        # request all 10 features -- can't fit in cache!
        all_features = [f for f in cache.getFeatures()]
        self.assertEqual(len(all_features), 10)

        self.assertFalse(cache.hasFullCache())

    def test_attribute_deleted_invalidates_cache(self):
        layer, cache, fids = self._create_test_data()

        cache.setFullCache(True)
        target_fid = fids[0]
        self.assertTrue(cache.hasFullCache())
        self.assertTrue(cache.isFidCached(target_fid))

        invalidated_spy = QSignalSpy(cache.invalidated)

        layer.startEditing()
        layer.deleteAttribute(1)

        self.assertEqual(len(invalidated_spy), 1)
        self.assertFalse(cache.hasFullCache())
        self.assertFalse(cache.isFidCached(target_fid))


if __name__ == "__main__":
    unittest.main()
