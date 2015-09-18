# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsFeatureIterator.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Matthias Kuhn'
__date__ = '18/09/2013'
__copyright__ = 'Copyright 2013, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis
import os

from qgis.core import QgsVectorLayer, QgsFeatureRequest, QgsFeature
from utilities import (unitTestDataPath,
                       getQgisTestApp,
                       TestCase,
                       unittest
                       )
QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsFeatureIterator(TestCase):

    def __init__(self, methodName):
        """Run once on class initialisation."""
        unittest.TestCase.__init__(self, methodName)

    def test_FilterExpression(self):
        # create point layer
        myShpFile = os.path.join(TEST_DATA_DIR, 'points.shp')
        pointLayer = QgsVectorLayer(myShpFile, 'Points', 'ogr')

        ids = [feat.id() for feat in pointLayer.getFeatures(QgsFeatureRequest().setFilterExpression('Staff > 3'))]
        expectedIds = [1, 5, 6, 7, 8]
        myMessage = '\nExpected: {0} features\nGot: {1} features'.format(repr(expectedIds), repr(ids))
        assert ids == expectedIds, myMessage

        pointLayer.startEditing()
        self.addFeatures(pointLayer)

        ids = [feat.id() for feat in pointLayer.getFeatures(QgsFeatureRequest().setFilterExpression('Staff > 3'))]
        expectedIds = [-2, 1, 5, 6, 7, 8]
        myMessage = '\nExpected: {0} features\nGot: {1} features'.format(repr(expectedIds), repr(ids))
        assert ids == expectedIds, myMessage

        pointLayer.rollBack()

        ids = [feat.id() for feat in pointLayer.getFeatures(QgsFeatureRequest().setFilterExpression('Staff > 3'))]
        expectedIds = [1, 5, 6, 7, 8]
        myMessage = '\nExpected: {0} features\nGot: {1} features'.format(repr(expectedIds), repr(ids))
        assert ids == expectedIds, myMessage

    def test_FilterFids(self):
        # create point layer
        myShpFile = os.path.join(TEST_DATA_DIR, 'points.shp')
        pointLayer = QgsVectorLayer(myShpFile, 'Points', 'ogr')

        ids = [feat.id() for feat in pointLayer.getFeatures(QgsFeatureRequest().setFilterFids([7, 8, 12, 30]))]
        expectedIds = [7, 8, 12]
        myMessage = '\nExpected: {0} features\nGot: {1} features'.format(repr(expectedIds), repr(ids))
        assert ids == expectedIds, myMessage

        pointLayer.startEditing()
        self.addFeatures(pointLayer)

        ids = [feat.id() for feat in pointLayer.getFeatures(QgsFeatureRequest().setFilterFids([-4, 7, 8, 12, 30]))]
        expectedIds = [-4, 7, 8, 12]
        myMessage = '\nExpected: {0} features\nGot: {1} features'.format(repr(expectedIds), repr(ids))
        assert ids == expectedIds, myMessage

        pointLayer.rollBack()

        ids = [feat.id() for feat in pointLayer.getFeatures(QgsFeatureRequest().setFilterFids([-2, 7, 8, 12, 30]))]
        expectedIds = [7, 8, 12]
        myMessage = '\nExpected: {0} features\nGot: {1} features'.format(repr(expectedIds), repr(ids))
        assert ids == expectedIds, myMessage

    def addFeatures(self, vl):
        feat = QgsFeature()
        fields = vl.pendingFields()
        feat.setFields(fields)
        feat['Staff'] = 4
        vl.addFeature(feat)

        feat = QgsFeature()
        fields = vl.pendingFields()
        feat.setFields(fields)
        feat['Staff'] = 2
        vl.addFeature(feat)

if __name__ == '__main__':
    unittest.main()
