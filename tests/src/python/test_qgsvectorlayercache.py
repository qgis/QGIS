# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsVectorLayerCache.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '08/06/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

import os

from qgis.PyQt.QtCore import QVariant, Qt
from qgis.PyQt.QtGui import QPainter
from qgis.PyQt.QtXml import QDomDocument

from qgis.core import (QgsWkbTypes,
                       QgsVectorLayer,
                       QgsVectorLayerCache,
                       QgsRectangle,
                       QgsFeature,
                       QgsFeatureRequest,
                       QgsGeometry,
                       QgsPointXY,
                       QgsField,
                       QgsFields,
                       QgsCoordinateReferenceSystem,
                       QgsProject,
                       QgsPoint,
                       NULL)
from qgis.testing import start_app, unittest
from featuresourcetestbase import FeatureSourceTestCase
from utilities import unitTestDataPath

start_app()


class TestQgsVectorLayerCache(unittest.TestCase, FeatureSourceTestCase):

    @classmethod
    def getSource(cls):
        cache = QgsVectorLayerCache(cls.vl, 100)
        return cache

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        # Create test layer for FeatureSourceTestCase
        cls.vl = QgsVectorLayer(
            'Point?crs=epsg:4326&field=pk:integer&field=cnt:integer&field=name:string(0)&field=name2:string(0)&field=num_char:string&key=pk',
            'test', 'memory')
        assert (cls.vl.isValid())

        f1 = QgsFeature(5)
        f1.setAttributes([5, -200, NULL, 'NuLl', '5'])
        f1.setGeometry(QgsGeometry.fromWkt('Point (-71.123 78.23)'))

        f2 = QgsFeature(3)
        f2.setAttributes([3, 300, 'Pear', 'PEaR', '3'])

        f3 = QgsFeature(1)
        f3.setAttributes([1, 100, 'Orange', 'oranGe', '1'])
        f3.setGeometry(QgsGeometry.fromWkt('Point (-70.332 66.33)'))

        f4 = QgsFeature(2)
        f4.setAttributes([2, 200, 'Apple', 'Apple', '2'])
        f4.setGeometry(QgsGeometry.fromWkt('Point (-68.2 70.8)'))

        f5 = QgsFeature(4)
        f5.setAttributes([4, 400, 'Honey', 'Honey', '4'])
        f5.setGeometry(QgsGeometry.fromWkt('Point (-65.32 78.3)'))

        assert cls.vl.dataProvider().addFeatures([f1, f2, f3, f4, f5])
        cls.source = QgsVectorLayerCache(cls.vl, 100)

    def testGetFeaturesSubsetAttributes2(self):
        """ Override and skip this QgsFeatureSource test. We are using a memory provider, and it's actually more efficient for the memory provider to return
        its features as direct copies (due to implicit sharing of QgsFeature)
        """
        pass

    def testGetFeaturesNoGeometry(self):
        """ Override and skip this QgsFeatureSource test. We are using a memory provider, and it's actually more efficient for the memory provider to return
        its features as direct copies (due to implicit sharing of QgsFeature)
        """
        pass

    def testUniqueValues(self):
        """ Skip unique values test - not implemented by the cache (yet)
        """
        pass

    def testMinimumValue(self):
        """ Skip min values test - not implemented by the cache (yet)
        """
        pass

    def testMaximumValue(self):
        """ Skip max values test - not implemented by the cache (yet)
        """
        pass

    def testAllFeatureIds(self):
        """ Skip allFeatureIds test - not implemented by the cache (yet)
        """
        pass


if __name__ == '__main__':
    unittest.main()
