# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsVectorLayerSelectedFeatureSource

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '2018-07-05'
__copyright__ = 'Copyright 2018, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'


from qgis.core import (
    QgsVectorLayer,
    QgsFeatureRequest,
    QgsFeature,
    QgsGeometry,
    NULL,
    QgsVectorLayerSelectedFeatureSource
)

from qgis.testing import (
    start_app,
    unittest
)

from utilities import (
    unitTestDataPath
)

from featuresourcetestbase import FeatureSourceTestCase

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestPyQgsVectorLayerSelectedFeatureSource(unittest.TestCase, FeatureSourceTestCase):

    @classmethod
    def createLayer(cls):
        vl = QgsVectorLayer(
            'Point?crs=epsg:4326&field=pk:integer&field=cnt:integer&field=name:string(0)&field=name2:string(0)&field=num_char:string&key=pk',
            'test', 'memory')
        assert (vl.isValid())

        f1 = QgsFeature()
        f1.setAttributes([5, -200, NULL, 'NuLl', '5'])
        f1.setGeometry(QgsGeometry.fromWkt('Point (-71.123 78.23)'))

        f2 = QgsFeature()
        f2.setAttributes([3, 300, 'Pear', 'PEaR', '3'])

        f3 = QgsFeature()
        f3.setAttributes([1, 100, 'Orange', 'oranGe', '1'])
        f3.setGeometry(QgsGeometry.fromWkt('Point (-70.332 66.33)'))

        f4 = QgsFeature()
        f4.setAttributes([2, 200, 'Apple', 'Apple', '2'])
        f4.setGeometry(QgsGeometry.fromWkt('Point (-68.2 70.8)'))

        f5 = QgsFeature()
        f5.setAttributes([4, 400, 'Honey', 'Honey', '4'])
        f5.setGeometry(QgsGeometry.fromWkt('Point (-65.32 78.3)'))

        f6 = QgsFeature()
        f6.setAttributes([6, -200, NULL, 'NuLl', '5'])
        f6.setGeometry(QgsGeometry.fromWkt('Point (-71.123 78.23)'))

        f7 = QgsFeature()
        f7.setAttributes([7, 300, 'Pear', 'PEaR', '3'])

        f8 = QgsFeature()
        f8.setAttributes([8, 100, 'Orange', 'oranGe', '1'])
        f8.setGeometry(QgsGeometry.fromWkt('Point (-70.332 66.33)'))

        f9 = QgsFeature()
        f9.setAttributes([9, 200, 'Apple', 'Apple', '2'])
        f9.setGeometry(QgsGeometry.fromWkt('Point (-68.2 70.8)'))

        f10 = QgsFeature()
        f10.setAttributes([10, 400, 'Honey', 'Honey', '4'])
        f10.setGeometry(QgsGeometry.fromWkt('Point (-65.32 78.3)'))

        vl.dataProvider().addFeatures([f1, f2, f3, f4, f5, f6, f7, f8, f9, f10])
        return vl

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        # Create test layer
        cls.vl = cls.createLayer()
        assert (cls.vl.isValid())

        ids = [f.id() for f in cls.vl.getFeatures(QgsFeatureRequest().setFilterExpression('pk in (1,2,3,4,5)'))]
        assert len(ids) == 5

        cls.vl.selectByIds(ids)
        cls.source = QgsVectorLayerSelectedFeatureSource(cls.vl)

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""

    def testGetFeaturesSubsetAttributes2(self):
        """ Override and skip this test for memory provider, as it's actually more efficient for the memory provider to return
        its features as direct copies (due to implicit sharing of QgsFeature)
        """
        pass

    def testGetFeaturesNoGeometry(self):
        """ Override and skip this test for memory provider, as it's actually more efficient for the memory provider to return
        its features as direct copies (due to implicit sharing of QgsFeature)
        """
        pass


if __name__ == '__main__':
    unittest.main()
