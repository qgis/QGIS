# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsVectorLayerFeatureCounter.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Mathieu Pellerin'
__date__ = '08/02/2021'
__copyright__ = 'Copyright 2021, The QGIS Project'

import qgis  # NOQA

import os

from qgis.PyQt.QtCore import QVariant, Qt, QDateTime, QDate, QTime
from qgis.PyQt.QtGui import QPainter
from qgis.PyQt.QtXml import QDomDocument
from qgis.PyQt.QtTest import QSignalSpy

from qgis.core import (QgsWkbTypes,
                       QgsVectorLayer,
                       QgsFeature,
                       QgsGeometry,
                       QgsField,
                       QgsFields,
                       NULL)
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

start_app()


class TestQgsVectorLayerFeatureCounter(unittest.TestCase):

    def setUp(self):

        self.vl = QgsVectorLayer(
            'Point?crs=epsg:4326&field=pk:integer&field=cnt:integer&field=name:string(0)&field=name2:string(0)&field=num_char:string&field=dt:datetime&field=date:date&field=time:time&key=pk',
            'test', 'memory')
        assert (self.vl.isValid())

        f1 = QgsFeature(5)
        f1.setAttributes([5, -200, NULL, 'NuLl', '5', QDateTime(QDate(2020, 5, 4), QTime(12, 13, 14)), QDate(2020, 5, 2), QTime(12, 13, 1)])
        f1.setGeometry(QgsGeometry.fromWkt('Point (-71.123 78.23)'))

        f2 = QgsFeature(3)
        f2.setAttributes([3, 300, 'Pear', 'PEaR', '3', NULL, NULL, NULL])

        f3 = QgsFeature(1)
        f3.setAttributes([1, 100, 'Orange', 'oranGe', '1', QDateTime(QDate(2020, 5, 3), QTime(12, 13, 14)), QDate(2020, 5, 3), QTime(12, 13, 14)])
        f3.setGeometry(QgsGeometry.fromWkt('Point (-70.332 66.33)'))

        f4 = QgsFeature(2)
        f4.setAttributes([2, 200, 'Apple', 'Apple', '2', QDateTime(QDate(2020, 5, 4), QTime(12, 14, 14)), QDate(2020, 5, 4), QTime(12, 14, 14)])
        f4.setGeometry(QgsGeometry.fromWkt('Point (-68.2 70.8)'))

        f5 = QgsFeature(4)
        f5.setAttributes([4, 400, 'Honey', 'Honey', '4', QDateTime(QDate(2021, 5, 4), QTime(13, 13, 14)), QDate(2021, 5, 4), QTime(13, 13, 14)])
        f5.setGeometry(QgsGeometry.fromWkt('Point (-65.32 78.3)'))

        assert self.vl.dataProvider().addFeatures([f1, f2, f3, f4, f5])

        self.vl2 = QgsVectorLayer(
            'Point?crs=epsg:4326&field=pk:integer&field=cnt:integer&field=name:string(0)&field=name2:string(0)&field=num_char:string&field=dt:datetime&field=date:date&field=time:time&key=pk',
            'test', 'memory')
        assert (self.vl2.isValid())

    def tearDown(self):
        del self.vl
        del self.vl2

    def testFeaturesCount(self):

        self.assertTrue(self.vl.renderer().legendSymbolItems())

        signal_spy = QSignalSpy(self.vl.symbolFeatureCountMapChanged)
        self.vl.countSymbolFeatures()
        signal_spy.wait()

        self.assertEqual(len(signal_spy), 1)
        self.assertEqual(self.vl.featureCount(self.vl.renderer().legendSymbolItems()[0].ruleKey()), 5)

    def testFeaturesCountOnEmptyLayer(self):

        self.assertTrue(self.vl2.renderer().legendSymbolItems())

        signal_spy = QSignalSpy(self.vl2.symbolFeatureCountMapChanged)
        self.vl2.countSymbolFeatures()
        signal_spy.wait()

        self.assertEqual(len(signal_spy), 1)
        self.assertEqual(self.vl2.featureCount(self.vl2.renderer().legendSymbolItems()[0].ruleKey()), 0)


if __name__ == '__main__':
    unittest.main()
