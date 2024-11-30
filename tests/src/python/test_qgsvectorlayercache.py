"""QGIS Unit tests for QgsVectorLayerCache.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "08/06/2017"
__copyright__ = "Copyright 2017, The QGIS Project"

from qgis.PyQt.QtCore import QDate, QDateTime, QTime
from qgis.core import (
    NULL,
    QgsFeature,
    QgsGeometry,
    QgsVectorLayer,
    QgsVectorLayerCache,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from featuresourcetestbase import FeatureSourceTestCase

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
            "Point?crs=epsg:4326&field=pk:integer&field=cnt:integer&field=name:string(0)&field=name2:string(0)&field=num_char:string&field=dt:datetime&field=date:date&field=time:time&key=pk",
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


if __name__ == "__main__":
    unittest.main()
