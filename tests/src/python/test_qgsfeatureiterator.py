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

from qgis.core import QgsVectorLayer, QgsFeatureRequest, QgsFeature, QgsField, NULL
from qgis.testing import (start_app,
                          unittest
                          )
from PyQt4.QtCore import QVariant
from utilities import (unitTestDataPath,
                       compareWkt
                       )
start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsFeatureIterator(unittest.TestCase):

    def __init__(self, methodName):
        """Run once on class initialization."""
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

    def test_FilterExpressionWithAccents(self):
        myShpFile = os.path.join(TEST_DATA_DIR, 'france_parts.shp')
        layer = QgsVectorLayer(myShpFile, 'poly', 'ogr')

        layer.setProviderEncoding("ISO-8859-1")
        ids = [feat.id() for feat in layer.getFeatures(QgsFeatureRequest().setFilterExpression(u"TYPE_1 = 'Région'"))]
        expectedIds = [0, 1, 2, 3]
        myMessage = '\nExpected: {0} features\nGot: {1} features'.format(repr(expectedIds), repr(ids))
        assert ids == expectedIds, myMessage

        layer.setProviderEncoding("UTF-8")
        ids = [feat.id() for feat in layer.getFeatures(QgsFeatureRequest().setFilterExpression(u"TYPE_1 = 'Région'"))]
        expectedIds = []
        myMessage = '\nExpected: {0} features\nGot: {1} features'.format(repr(expectedIds), repr(ids))
        assert ids == expectedIds, myMessage

    def test_FilterFids(self):
        # create point layer
        myShpFile = os.path.join(TEST_DATA_DIR, 'points.shp')
        pointLayer = QgsVectorLayer(myShpFile, 'Points', 'ogr')

        ids = [feat.id() for feat in pointLayer.getFeatures(QgsFeatureRequest().setFilterFids([7, 8, 12, 30]))]
        expectedIds = [7, 8, 12]
        self.assertEquals(set(ids), set(expectedIds))

        pointLayer.startEditing()
        self.addFeatures(pointLayer)

        ids = [feat.id() for feat in pointLayer.getFeatures(QgsFeatureRequest().setFilterFids([-4, 7, 8, 12, 30]))]
        expectedIds = [-4, 7, 8, 12]
        self.assertEquals(set(ids), set(expectedIds))

        pointLayer.rollBack()

        ids = [feat.id() for feat in pointLayer.getFeatures(QgsFeatureRequest().setFilterFids([-2, 7, 8, 12, 30]))]
        expectedIds = [7, 8, 12]
        self.assertEquals(set(ids), set(expectedIds))

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

    def test_ExpressionFieldNested(self):
        myShpFile = os.path.join(TEST_DATA_DIR, 'points.shp')
        layer = QgsVectorLayer(myShpFile, 'Points', 'ogr')
        self.assertTrue(layer.isValid())

        cnt = layer.pendingFields().count()
        idx = layer.addExpressionField('"Staff"*2', QgsField('exp1', QVariant.LongLong))
        idx = layer.addExpressionField('"exp1"-1', QgsField('exp2', QVariant.LongLong))

        fet = next(layer.getFeatures(QgsFeatureRequest().setSubsetOfAttributes(['exp2'], layer.fields())))
        self.assertEqual(fet['Class'], NULL)
        # nested virtual fields should make all these attributes be fetched
        self.assertEqual(fet['Staff'], 2)
        self.assertEqual(fet['exp2'], 3)
        self.assertEqual(fet['exp1'], 4)

    def test_ExpressionFieldNestedGeometry(self):
        myShpFile = os.path.join(TEST_DATA_DIR, 'points.shp')
        layer = QgsVectorLayer(myShpFile, 'Points', 'ogr')
        self.assertTrue(layer.isValid())

        cnt = layer.pendingFields().count()
        idx = layer.addExpressionField('$x*2', QgsField('exp1', QVariant.LongLong))
        idx = layer.addExpressionField('"exp1"/1.5', QgsField('exp2', QVariant.LongLong))

        fet = next(layer.getFeatures(QgsFeatureRequest().setFlags(QgsFeatureRequest.NoGeometry).setSubsetOfAttributes(['exp2'], layer.fields())))
        # nested virtual fields should have made geometry be fetched
        self.assertEqual(fet['exp2'], -156)
        self.assertEqual(fet['exp1'], -234)

    def test_ExpressionFieldNestedCircular(self):
        """ test circular virtual field definitions """

        myShpFile = os.path.join(TEST_DATA_DIR, 'points.shp')
        layer = QgsVectorLayer(myShpFile, 'Points', 'ogr')
        self.assertTrue(layer.isValid())

        cnt = layer.pendingFields().count()
        idx = layer.addExpressionField('"exp3"*2', QgsField('exp1', QVariant.LongLong))
        idx = layer.addExpressionField('"exp1"-1', QgsField('exp2', QVariant.LongLong))
        idx = layer.addExpressionField('"exp2"*3', QgsField('exp3', QVariant.LongLong))

        # really just testing that this doesn't hang/crash... there's no good result here!
        fet = next(layer.getFeatures(QgsFeatureRequest().setSubsetOfAttributes(['exp2'], layer.fields())))
        self.assertEqual(fet['Class'], NULL)


if __name__ == '__main__':
    unittest.main()
