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

import qgis  # NOQA

import os

from qgis.core import QgsVectorLayer, QgsFeatureRequest, QgsFeature, QgsField, NULL, QgsMapLayerRegistry, QgsVectorJoinInfo
from qgis.testing import start_app, unittest
from qgis.PyQt.QtCore import QVariant

from utilities import unitTestDataPath
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

    def test_JoinUsingExpression(self):
        """ test joining a layer using a virtual field """
        joinLayer = QgsVectorLayer(
            "Point?field=x:string&field=y:integer&field=z:integer",
            "joinlayer", "memory")
        pr = joinLayer.dataProvider()
        f1 = QgsFeature()
        f1.setAttributes(["foo", 246, 321])
        f2 = QgsFeature()
        f2.setAttributes(["bar", 456, 654])
        self.assertTrue(pr.addFeatures([f1, f2]))

        layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer",
                               "addfeat", "memory")
        pr = layer.dataProvider()
        f = QgsFeature()
        f.setAttributes(["test", 123])
        self.assertTrue(pr.addFeatures([f]))
        layer.addExpressionField('"fldint"*2', QgsField('exp1', QVariant.LongLong))

        QgsMapLayerRegistry.instance().addMapLayers([layer, joinLayer])

        join = QgsVectorJoinInfo()
        join.targetFieldName = "exp1"
        join.joinLayerId = joinLayer.id()
        join.joinFieldName = "y"
        join.memoryCache = True
        layer.addJoin(join)

        f = QgsFeature()
        fi = layer.getFeatures()
        self.assertTrue(fi.nextFeature(f))
        attrs = f.attributes()
        self.assertEqual(attrs[0], "test")
        self.assertEqual(attrs[1], 123)
        self.assertEqual(attrs[2], "foo")
        self.assertEqual(attrs[3], 321)
        self.assertEqual(attrs[4], 246)
        self.assertFalse(fi.nextFeature(f))

        QgsMapLayerRegistry.instance().removeMapLayers([layer.id(), joinLayer.id()])

    def test_JoinUsingExpression2(self):
        """ test joining a layer using a virtual field (the other way!) """
        joinLayer = QgsVectorLayer(
            "Point?field=x:string&field=y:integer&field=z:integer",
            "joinlayer", "memory")
        pr = joinLayer.dataProvider()
        f1 = QgsFeature()
        f1.setAttributes(["foo", 246, 321])
        f2 = QgsFeature()
        f2.setAttributes(["bar", 456, 654])
        self.assertTrue(pr.addFeatures([f1, f2]))
        joinLayer.addExpressionField('"y"/2', QgsField('exp1', QVariant.LongLong))

        layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer",
                               "addfeat", "memory")
        pr = layer.dataProvider()
        f = QgsFeature()
        f.setAttributes(["test", 123])
        self.assertTrue(pr.addFeatures([f]))

        QgsMapLayerRegistry.instance().addMapLayers([layer, joinLayer])

        join = QgsVectorJoinInfo()
        join.targetFieldName = "fldint"
        join.joinLayerId = joinLayer.id()
        join.joinFieldName = "exp1"
        join.memoryCache = True
        layer.addJoin(join)

        f = QgsFeature()
        fi = layer.getFeatures()
        self.assertTrue(fi.nextFeature(f))
        attrs = f.attributes()
        self.assertEqual(attrs[0], "test")
        self.assertEqual(attrs[1], 123)
        self.assertEqual(attrs[2], "foo")
        self.assertEqual(attrs[3], 246)
        self.assertEqual(attrs[4], 321)
        self.assertFalse(fi.nextFeature(f))

        QgsMapLayerRegistry.instance().removeMapLayers([layer.id(), joinLayer.id()])

    def test_JoinUsingFeatureRequestExpression(self):
        """ test requesting features using a filter expression which requires joined columns """
        joinLayer = QgsVectorLayer(
            "Point?field=x:string&field=y:integer&field=z:integer",
            "joinlayer", "memory")
        pr = joinLayer.dataProvider()
        f1 = QgsFeature()
        f1.setAttributes(["foo", 123, 321])
        f2 = QgsFeature()
        f2.setAttributes(["bar", 124, 654])
        self.assertTrue(pr.addFeatures([f1, f2]))

        layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer",
                               "addfeat", "memory")
        pr = layer.dataProvider()
        f1 = QgsFeature()
        f1.setAttributes(["test", 123])
        f2 = QgsFeature()
        f2.setAttributes(["test", 124])
        self.assertTrue(pr.addFeatures([f1, f2]))

        QgsMapLayerRegistry.instance().addMapLayers([layer, joinLayer])

        join = QgsVectorJoinInfo()
        join.targetFieldName = "fldint"
        join.joinLayerId = joinLayer.id()
        join.joinFieldName = "y"
        join.memoryCache = True
        layer.addJoin(join)

        f = QgsFeature()
        fi = layer.getFeatures(QgsFeatureRequest().setFlags(QgsFeatureRequest.SubsetOfAttributes).setFilterExpression('joinlayer_z=654'))
        self.assertTrue(fi.nextFeature(f))
        self.assertEqual(f['fldint'], 124)
        self.assertEqual(f['joinlayer_z'], 654)

        QgsMapLayerRegistry.instance().removeMapLayers([layer.id(), joinLayer.id()])


if __name__ == '__main__':
    unittest.main()
