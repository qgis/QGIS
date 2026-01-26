"""QGIS Unit tests for QgsFeatureIterator.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Matthias Kuhn"
__date__ = "18/09/2013"
__copyright__ = "Copyright 2013, The QGIS Project"

import os

from qgis.PyQt.QtCore import QVariant
from qgis.core import (
    NULL,
    QgsAuxiliaryStorage,
    QgsFeature,
    QgsFeatureRequest,
    QgsField,
    QgsGeometry,
    QgsProject,
    QgsPropertyDefinition,
    QgsVectorLayer,
    QgsVectorLayerJoinInfo,
    QgsPoint,
    QgsCoordinateReferenceSystem,
    QgsCoordinateTransform,
    QgsCoordinateTransformContext,
    QgsDatumTransform,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsFeatureIterator(QgisTestCase):

    def __init__(self, methodName):
        """Run once on class initialization."""
        QgisTestCase.__init__(self, methodName)

    def test_FilterExpression(self):
        # create point layer
        myShpFile = os.path.join(TEST_DATA_DIR, "points.shp")
        pointLayer = QgsVectorLayer(myShpFile, "Points", "ogr")

        ids = [
            feat.id()
            for feat in pointLayer.getFeatures(
                QgsFeatureRequest().setFilterExpression("Staff > 3")
            )
        ]
        expectedIds = [1, 5, 6, 7, 8]
        myMessage = (
            f"\nExpected: {repr(expectedIds)} features\nGot: {repr(ids)} features"
        )
        assert ids == expectedIds, myMessage

        pointLayer.startEditing()
        self.addFeatures(pointLayer)

        ids = [
            feat.id()
            for feat in pointLayer.getFeatures(
                QgsFeatureRequest().setFilterExpression("Staff > 3")
            )
        ]
        expectedIds = [-2, 1, 5, 6, 7, 8]
        myMessage = (
            f"\nExpected: {repr(expectedIds)} features\nGot: {repr(ids)} features"
        )
        assert ids == expectedIds, myMessage

        pointLayer.rollBack()

        ids = [
            feat.id()
            for feat in pointLayer.getFeatures(
                QgsFeatureRequest().setFilterExpression("Staff > 3")
            )
        ]
        expectedIds = [1, 5, 6, 7, 8]
        myMessage = (
            f"\nExpected: {repr(expectedIds)} features\nGot: {repr(ids)} features"
        )
        assert ids == expectedIds, myMessage

    def test_FilterExpressionWithAccents(self):
        myShpFile = os.path.join(TEST_DATA_DIR, "france_parts.shp")
        layer = QgsVectorLayer(myShpFile, "poly", "ogr")

        layer.setProviderEncoding("ISO-8859-1")
        ids = [
            feat.id()
            for feat in layer.getFeatures(
                QgsFeatureRequest().setFilterExpression("TYPE_1 = 'Région'")
            )
        ]
        expectedIds = [0, 1, 2, 3]
        myMessage = (
            f"\nExpected: {repr(expectedIds)} features\nGot: {repr(ids)} features"
        )
        assert ids == expectedIds, myMessage

        layer.setProviderEncoding("UTF-8")
        ids = [
            feat.id()
            for feat in layer.getFeatures(
                QgsFeatureRequest().setFilterExpression("TYPE_1 = 'Région'")
            )
        ]
        expectedIds = []
        myMessage = (
            f"\nExpected: {repr(expectedIds)} features\nGot: {repr(ids)} features"
        )
        assert ids == expectedIds, myMessage

    def test_FilterFids(self):
        # create point layer
        myShpFile = os.path.join(TEST_DATA_DIR, "points.shp")
        pointLayer = QgsVectorLayer(myShpFile, "Points", "ogr")

        ids = [
            feat.id()
            for feat in pointLayer.getFeatures(
                QgsFeatureRequest().setFilterFids([7, 8, 12, 30])
            )
        ]
        expectedIds = [7, 8, 12]
        self.assertEqual(set(ids), set(expectedIds))

        pointLayer.startEditing()
        self.addFeatures(pointLayer)

        ids = [
            feat.id()
            for feat in pointLayer.getFeatures(
                QgsFeatureRequest().setFilterFids([-4, 7, 8, 12, 30])
            )
        ]
        expectedIds = [-4, 7, 8, 12]
        self.assertEqual(set(ids), set(expectedIds))

        pointLayer.rollBack()

        ids = [
            feat.id()
            for feat in pointLayer.getFeatures(
                QgsFeatureRequest().setFilterFids([-2, 7, 8, 12, 30])
            )
        ]
        expectedIds = [7, 8, 12]
        self.assertEqual(set(ids), set(expectedIds))

    def addFeatures(self, vl):
        feat = QgsFeature()
        fields = vl.fields()
        feat.setFields(fields)
        feat["Staff"] = 4
        vl.addFeature(feat)

        feat = QgsFeature()
        fields = vl.fields()
        feat.setFields(fields)
        feat["Staff"] = 2
        vl.addFeature(feat)

    def test_VectorLayerEditing(self):
        ogr_layer = QgsVectorLayer(
            os.path.join(TEST_DATA_DIR, "points.shp"), "Points", "ogr"
        )
        self.assertTrue(ogr_layer.isValid())

        request = QgsFeatureRequest()
        iterator = ogr_layer.getFeatures(request)
        self.assertTrue(iterator.isValid())

        self.assertTrue(ogr_layer.startEditing())
        iterator = ogr_layer.getFeatures(request)
        self.assertTrue(iterator.isValid())

        memory_layer = QgsVectorLayer(
            "Point?field=x:string&field=y:integer&field=z:integer", "layer", "memory"
        )
        self.assertTrue(memory_layer.isValid())

        request = QgsFeatureRequest()
        iterator = memory_layer.getFeatures(request)
        self.assertTrue(iterator.isValid())

        self.assertTrue(memory_layer.startEditing())
        iterator = memory_layer.getFeatures(request)
        self.assertTrue(iterator.isValid())

    def test_ExpressionFieldNested(self):
        myShpFile = os.path.join(TEST_DATA_DIR, "points.shp")
        layer = QgsVectorLayer(myShpFile, "Points", "ogr")
        self.assertTrue(layer.isValid())

        idx = layer.addExpressionField(
            '"Staff"*2', QgsField("exp1", QVariant.LongLong)
        )  # NOQA
        idx = layer.addExpressionField(
            '"exp1"-1', QgsField("exp2", QVariant.LongLong)
        )  # NOQA

        fet = next(
            layer.getFeatures(
                QgsFeatureRequest().setSubsetOfAttributes(["exp2"], layer.fields())
            )
        )
        self.assertEqual(fet["Class"], NULL)
        # nested virtual fields should make all these attributes be fetched
        self.assertEqual(fet["Staff"], 2)
        self.assertEqual(fet["exp2"], 3)
        self.assertEqual(fet["exp1"], 4)

    def test_ExpressionFieldNestedGeometry(self):
        myShpFile = os.path.join(TEST_DATA_DIR, "points.shp")
        layer = QgsVectorLayer(myShpFile, "Points", "ogr")
        self.assertTrue(layer.isValid())

        idx = layer.addExpressionField(
            "$x*2", QgsField("exp1", QVariant.LongLong)
        )  # NOQA
        idx = layer.addExpressionField(
            '"exp1"/1.5', QgsField("exp2", QVariant.LongLong)
        )  # NOQA

        fet = next(
            layer.getFeatures(
                QgsFeatureRequest()
                .setFlags(QgsFeatureRequest.Flag.NoGeometry)
                .setSubsetOfAttributes(["exp2"], layer.fields())
            )
        )
        # nested virtual fields should have made geometry be fetched
        self.assertEqual(fet["exp2"], -156)
        self.assertEqual(fet["exp1"], -234)

    def test_ExpressionFieldDependingOnOtherFields(self):
        myShpFile = os.path.join(TEST_DATA_DIR, "points.shp")
        layer = QgsVectorLayer(myShpFile, "Points", "ogr")
        self.assertTrue(layer.isValid())

        idx = layer.addExpressionField(
            "eval('Class')", QgsField("exp1", QVariant.String)
        )  # NOQA

        fet = next(
            layer.getFeatures(
                QgsFeatureRequest()
                .setFlags(QgsFeatureRequest.Flag.NoGeometry)
                .setSubsetOfAttributes(["exp1"], layer.fields())
            )
        )

        self.assertEqual(fet["exp1"], "Jet")

    def test_ExpressionFieldNestedCircular(self):
        """test circular virtual field definitions"""

        myShpFile = os.path.join(TEST_DATA_DIR, "points.shp")
        layer = QgsVectorLayer(myShpFile, "Points", "ogr")
        self.assertTrue(layer.isValid())

        cnt = layer.fields().count()  # NOQA
        idx = layer.addExpressionField(
            '"exp3"*2', QgsField("exp1", QVariant.LongLong)
        )  # NOQA
        idx = layer.addExpressionField(
            '"exp1"-1', QgsField("exp2", QVariant.LongLong)
        )  # NOQA
        idx = layer.addExpressionField(
            '"exp2"*3', QgsField("exp3", QVariant.LongLong)
        )  # NOQA

        # really just testing that this doesn't hang/crash... there's no good result here!
        fet = next(
            layer.getFeatures(
                QgsFeatureRequest().setSubsetOfAttributes(["exp2"], layer.fields())
            )
        )
        self.assertEqual(fet["Class"], NULL)

    def test_JoinUsingExpression(self):
        """test joining a layer using a virtual field"""
        joinLayer = QgsVectorLayer(
            "Point?field=x:string&field=y:integer&field=z:integer",
            "joinlayer",
            "memory",
        )
        pr = joinLayer.dataProvider()
        f1 = QgsFeature()
        f1.setAttributes(["foo", 246, 321])
        f2 = QgsFeature()
        f2.setAttributes(["bar", 456, 654])
        self.assertTrue(pr.addFeatures([f1, f2]))

        layer = QgsVectorLayer(
            "Point?field=fldtxt:string&field=fldint:integer", "addfeat", "memory"
        )
        pr = layer.dataProvider()
        f = QgsFeature()
        f.setAttributes(["test", 123])
        self.assertTrue(pr.addFeatures([f]))
        layer.addExpressionField('"fldint"*2', QgsField("exp1", QVariant.LongLong))

        QgsProject.instance().addMapLayers([layer, joinLayer])

        join = QgsVectorLayerJoinInfo()
        join.setTargetFieldName("exp1")
        join.setJoinLayer(joinLayer)
        join.setJoinFieldName("y")
        join.setUsingMemoryCache(True)
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

        QgsProject.instance().removeMapLayers([layer.id(), joinLayer.id()])

    def test_JoinUsingExpression2(self):
        """test joining a layer using a virtual field (the other way!)"""
        joinLayer = QgsVectorLayer(
            "Point?field=x:string&field=y:integer&field=z:integer",
            "joinlayer",
            "memory",
        )
        pr = joinLayer.dataProvider()
        f1 = QgsFeature()
        f1.setAttributes(["foo", 246, 321])
        f2 = QgsFeature()
        f2.setAttributes(["bar", 456, 654])
        self.assertTrue(pr.addFeatures([f1, f2]))
        joinLayer.addExpressionField('"y"/2', QgsField("exp1", QVariant.LongLong))

        layer = QgsVectorLayer(
            "Point?field=fldtxt:string&field=fldint:integer", "addfeat", "memory"
        )
        pr = layer.dataProvider()
        f = QgsFeature()
        f.setAttributes(["test", 123])
        self.assertTrue(pr.addFeatures([f]))

        QgsProject.instance().addMapLayers([layer, joinLayer])

        join = QgsVectorLayerJoinInfo()
        join.setTargetFieldName("fldint")
        join.setJoinLayer(joinLayer)
        join.setJoinFieldName("exp1")
        join.setUsingMemoryCache(True)
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

        QgsProject.instance().removeMapLayers([layer.id(), joinLayer.id()])

    def test_JoinUsingFeatureRequestExpression(self):
        """test requesting features using a filter expression which requires joined columns"""
        joinLayer = QgsVectorLayer(
            "Point?field=x:string&field=y:integer&field=z:integer",
            "joinlayer",
            "memory",
        )
        pr = joinLayer.dataProvider()
        f1 = QgsFeature()
        f1.setAttributes(["foo", 123, 321])
        f2 = QgsFeature()
        f2.setAttributes(["bar", 124, 654])
        self.assertTrue(pr.addFeatures([f1, f2]))

        layer = QgsVectorLayer(
            "Point?field=fldtxt:string&field=fldint:integer", "addfeat", "memory"
        )
        pr = layer.dataProvider()
        f1 = QgsFeature()
        f1.setAttributes(["test", 123])
        f2 = QgsFeature()
        f2.setAttributes(["test", 124])
        self.assertTrue(pr.addFeatures([f1, f2]))

        QgsProject.instance().addMapLayers([layer, joinLayer])

        join = QgsVectorLayerJoinInfo()
        join.setTargetFieldName("fldint")
        join.setJoinLayer(joinLayer)
        join.setJoinFieldName("y")
        join.setUsingMemoryCache(True)
        layer.addJoin(join)

        f = QgsFeature()
        fi = layer.getFeatures(
            QgsFeatureRequest()
            .setFlags(QgsFeatureRequest.Flag.SubsetOfAttributes)
            .setFilterExpression("joinlayer_z=654")
        )
        self.assertTrue(fi.nextFeature(f))
        self.assertEqual(f["fldint"], 124)
        self.assertEqual(f["joinlayer_z"], 654)

        QgsProject.instance().removeMapLayers([layer.id(), joinLayer.id()])

    def test_FeatureRequestSortByVirtualField(self):
        layer = QgsVectorLayer(
            "Point?field=fldtxt:string&field=fldint:integer", "addfeat", "memory"
        )
        pr = layer.dataProvider()
        f1 = QgsFeature()
        f1.setAttributes(["test", 123])
        f2 = QgsFeature()
        f2.setAttributes(["test", 124])
        self.assertTrue(pr.addFeatures([f1, f2]))

        idx = layer.addExpressionField(
            'if("fldint"=123,3,2)', QgsField("exp1", QVariant.LongLong)
        )  # NOQA

        QgsProject.instance().addMapLayers([layer])

        request = QgsFeatureRequest()
        request.setOrderBy(
            QgsFeatureRequest.OrderBy([QgsFeatureRequest.OrderByClause("exp1", True)])
        )
        ids = []
        for feat in layer.getFeatures(request):
            ids.append(feat.id())
        self.assertEqual(ids, [2, 1])

        request.setOrderBy(
            QgsFeatureRequest.OrderBy([QgsFeatureRequest.OrderByClause("exp1", False)])
        )
        ids = []
        for feat in layer.getFeatures(request):
            ids.append(feat.id())
        self.assertEqual(ids, [1, 2])

        QgsProject.instance().removeMapLayers([layer.id()])

    def test_FeatureRequestSortByJoinField(self):
        """test sorting requested features using a joined columns"""
        joinLayer = QgsVectorLayer(
            "Point?field=x:string&field=y:integer&field=z:integer",
            "joinlayer",
            "memory",
        )
        pr = joinLayer.dataProvider()
        f1 = QgsFeature()
        f1.setAttributes(["foo", 123, 321])
        f2 = QgsFeature()
        f2.setAttributes(["bar", 124, 654])
        self.assertTrue(pr.addFeatures([f1, f2]))

        layer = QgsVectorLayer(
            "Point?field=fldtxt:string&field=fldint:integer", "addfeat", "memory"
        )
        pr = layer.dataProvider()
        f1 = QgsFeature()
        f1.setAttributes(["test", 123])
        f2 = QgsFeature()
        f2.setAttributes(["test", 124])
        self.assertTrue(pr.addFeatures([f1, f2]))

        QgsProject.instance().addMapLayers([layer, joinLayer])

        join = QgsVectorLayerJoinInfo()
        join.setTargetFieldName("fldint")
        join.setJoinLayer(joinLayer)
        join.setJoinFieldName("y")
        join.setUsingMemoryCache(True)
        layer.addJoin(join)

        request = QgsFeatureRequest()
        request.setOrderBy(
            QgsFeatureRequest.OrderBy(
                [QgsFeatureRequest.OrderByClause("joinlayer_z", True)]
            )
        )
        ids = []
        for feat in layer.getFeatures(request):
            ids.append(feat.id())
        self.assertEqual(ids, [1, 2])

        request.setOrderBy(
            QgsFeatureRequest.OrderBy(
                [QgsFeatureRequest.OrderByClause("joinlayer_z", False)]
            )
        )
        ids = []
        for feat in layer.getFeatures(request):
            ids.append(feat.id())
        self.assertEqual(ids, [2, 1])

        QgsProject.instance().removeMapLayers([layer.id(), joinLayer.id()])

    def test_ZFeatureRequestSortByAuxiliaryField(self):
        s = QgsAuxiliaryStorage()
        self.assertTrue(s.isValid())

        layer = QgsVectorLayer(
            "Point?field=fldtxt:string&field=fldint:integer", "addfeat", "memory"
        )
        pr = layer.dataProvider()
        f1 = QgsFeature()
        f1.setAttributes(["test", 123])
        f2 = QgsFeature()
        f2.setAttributes(["test", 124])
        self.assertTrue(pr.addFeatures([f1, f2]))

        # Create a new auxiliary layer with 'pk' as key
        pkf = layer.fields().field(layer.fields().indexOf("fldint"))
        al = s.createAuxiliaryLayer(pkf, layer)
        self.assertTrue(al.isValid())
        layer.setAuxiliaryLayer(al)

        prop = QgsPropertyDefinition()
        prop.setComment("test_field")
        prop.setDataType(QgsPropertyDefinition.DataType.DataTypeNumeric)
        prop.setOrigin("user")
        prop.setName("custom")
        self.assertTrue(al.addAuxiliaryField(prop))

        layer.startEditing()
        i = 2
        for feat in layer.getFeatures():
            feat.setAttribute(2, i)
            layer.updateFeature(feat)
            i -= 1
        layer.commitChanges()

        request = QgsFeatureRequest()
        request.setOrderBy(
            QgsFeatureRequest.OrderBy(
                [QgsFeatureRequest.OrderByClause(layer.fields()[2].name(), True)]
            )
        )
        ids = []
        for feat in layer.getFeatures(request):
            ids.append(feat.id())
        self.assertEqual(ids, [2, 1])

        request.setOrderBy(
            QgsFeatureRequest.OrderBy(
                [QgsFeatureRequest.OrderByClause(layer.fields()[2].name(), False)]
            )
        )
        ids = []
        for feat in layer.getFeatures(request):
            ids.append(feat.id())
        self.assertEqual(ids, [1, 2])

        QgsProject.instance().removeMapLayers([layer.id()])

    def test_invalidGeometryFilter(self):
        layer = QgsVectorLayer("Polygon?field=x:string", "joinlayer", "memory")

        # add some features, one has invalid geometry
        pr = layer.dataProvider()
        f1 = QgsFeature(1)
        f1.setAttributes(["a"])
        f1.setGeometry(
            QgsGeometry.fromWkt("Polygon((0 0, 1 0, 1 1, 0 1, 0 0))")
        )  # valid
        f2 = QgsFeature(2)
        f2.setAttributes(["b"])
        f2.setGeometry(
            QgsGeometry.fromWkt("Polygon((0 0, 1 0, 0 1, 1 1, 0 0))")
        )  # invalid
        f3 = QgsFeature(3)
        f3.setAttributes(["c"])
        f3.setGeometry(
            QgsGeometry.fromWkt("Polygon((0 0, 1 0, 1 1, 0 1, 0 0))")
        )  # valid
        self.assertTrue(pr.addFeatures([f1, f2, f3]))

        res = [
            f["x"]
            for f in layer.getFeatures(
                QgsFeatureRequest().setInvalidGeometryCheck(
                    QgsFeatureRequest.InvalidGeometryCheck.GeometryNoCheck
                )
            )
        ]
        self.assertEqual(res, ["a", "b", "c"])
        res = [
            f["x"]
            for f in layer.getFeatures(
                QgsFeatureRequest().setInvalidGeometryCheck(
                    QgsFeatureRequest.InvalidGeometryCheck.GeometrySkipInvalid
                )
            )
        ]
        self.assertEqual(res, ["a", "c"])
        res = [
            f["x"]
            for f in layer.getFeatures(
                QgsFeatureRequest().setInvalidGeometryCheck(
                    QgsFeatureRequest.InvalidGeometryCheck.GeometryAbortOnInvalid
                )
            )
        ]
        self.assertEqual(res, ["a"])

        # with callback
        self.callback_feature_val = None

        def callback(feature):
            self.callback_feature_val = feature["x"]

        res = [
            f["x"]
            for f in layer.getFeatures(
                QgsFeatureRequest()
                .setInvalidGeometryCheck(
                    QgsFeatureRequest.InvalidGeometryCheck.GeometryAbortOnInvalid
                )
                .setInvalidGeometryCallback(callback)
            )
        ]
        self.assertEqual(res, ["a"])
        self.assertEqual(self.callback_feature_val, "b")
        # clear callback
        res = [
            f["x"]
            for f in layer.getFeatures(
                QgsFeatureRequest()
                .setInvalidGeometryCheck(
                    QgsFeatureRequest.InvalidGeometryCheck.GeometryAbortOnInvalid
                )
                .setInvalidGeometryCallback(None)
            )
        ]
        self.assertEqual(res, ["a"])

        # check with filter fids
        res = [
            f["x"]
            for f in layer.getFeatures(
                QgsFeatureRequest()
                .setFilterFid(f2.id())
                .setInvalidGeometryCheck(
                    QgsFeatureRequest.InvalidGeometryCheck.GeometryNoCheck
                )
            )
        ]
        self.assertEqual(res, ["b"])
        res = [
            f["x"]
            for f in layer.getFeatures(
                QgsFeatureRequest()
                .setFilterFid(f2.id())
                .setInvalidGeometryCheck(
                    QgsFeatureRequest.InvalidGeometryCheck.GeometrySkipInvalid
                )
            )
        ]
        self.assertEqual(res, [])
        res = [
            f["x"]
            for f in layer.getFeatures(
                QgsFeatureRequest()
                .setFilterFid(f2.id())
                .setInvalidGeometryCheck(
                    QgsFeatureRequest.InvalidGeometryCheck.GeometryAbortOnInvalid
                )
            )
        ]
        self.assertEqual(res, [])

        f4 = QgsFeature(4)
        f4.setAttributes(["d"])
        f4.setGeometry(
            QgsGeometry.fromWkt("Polygon((0 0, 1 0, 0 1, 1 1, 0 0))")
        )  # invalid

        # check with added features
        layer.startEditing()
        self.assertTrue(layer.addFeatures([f4]))
        res = [
            f["x"]
            for f in layer.getFeatures(
                QgsFeatureRequest().setInvalidGeometryCheck(
                    QgsFeatureRequest.InvalidGeometryCheck.GeometryNoCheck
                )
            )
        ]
        self.assertEqual(set(res), {"a", "b", "c", "d"})
        res = [
            f["x"]
            for f in layer.getFeatures(
                QgsFeatureRequest().setInvalidGeometryCheck(
                    QgsFeatureRequest.InvalidGeometryCheck.GeometrySkipInvalid
                )
            )
        ]
        self.assertEqual(set(res), {"a", "c"})
        res = [
            f["x"]
            for f in layer.getFeatures(
                QgsFeatureRequest().setInvalidGeometryCheck(
                    QgsFeatureRequest.InvalidGeometryCheck.GeometryAbortOnInvalid
                )
            )
        ]
        self.assertEqual(res, ["a"])

        # check with features with changed geometry
        layer.rollBack()
        layer.startEditing()
        layer.changeGeometry(
            2, QgsGeometry.fromWkt("Polygon((0 0, 1 0, 1 1, 0 1, 0 0))")
        )  # valid
        layer.changeGeometry(
            3, QgsGeometry.fromWkt("Polygon((0 0, 1 0, 0 1, 1 1, 0 0))")
        )  # invalid
        res = [
            f["x"]
            for f in layer.getFeatures(
                QgsFeatureRequest().setInvalidGeometryCheck(
                    QgsFeatureRequest.InvalidGeometryCheck.GeometryNoCheck
                )
            )
        ]
        self.assertEqual(set(res), {"a", "b", "c"})
        res = [
            f["x"]
            for f in layer.getFeatures(
                QgsFeatureRequest().setInvalidGeometryCheck(
                    QgsFeatureRequest.InvalidGeometryCheck.GeometrySkipInvalid
                )
            )
        ]
        self.assertEqual(set(res), {"a", "b"})
        res = [
            f["x"]
            for f in layer.getFeatures(
                QgsFeatureRequest().setInvalidGeometryCheck(
                    QgsFeatureRequest.InvalidGeometryCheck.GeometryAbortOnInvalid
                )
            )
        ]
        self.assertEqual(res, ["a", "b"])
        layer.rollBack()

    def test_vertical_transformation_4978_to_4979(self):
        """
        Test vertical transformations are correctly handled during iteration

        EPSG:4978 to EPSG:4979
        """

        vl = QgsVectorLayer("PointZ?crs=EPSG:4978", "gda2020points", "memory")
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.crs().authid(), "EPSG:4978")

        self.assertEqual(vl.crs3D().horizontalCrs().authid(), "EPSG:4978")

        f = QgsFeature()
        f.setGeometry(QgsPoint(134.445567853, -23.445567853, 5543.325))
        self.assertTrue(vl.dataProvider().addFeature(f))

        dest_crs = QgsCoordinateReferenceSystem("EPSG:4979")
        self.assertTrue(dest_crs.isValid())
        self.assertEqual(dest_crs.horizontalCrs().authid(), "EPSG:4979")

        transform = QgsCoordinateTransform(
            vl.crs3D(), dest_crs, QgsCoordinateTransformContext()
        )

        request = QgsFeatureRequest().setCoordinateTransform(transform)

        transformed_features = list(vl.getFeatures(request))
        self.assertEqual(len(transformed_features), 1)
        geom = transformed_features[0].geometry()
        self.assertAlmostEqual(geom.constGet().x(), -9.8921668708, 4)
        self.assertAlmostEqual(geom.constGet().y(), 89.839008, 4)
        self.assertAlmostEqual(geom.constGet().z(), -6351023.00373, 3)

    def test_vertical_transformation_gda2020_to_AVWS(self):
        """
        Test vertical transformations are correctly handled during iteration

        GDA2020 to AVWS
        """

        # GDA2020 vertical CRS
        vl = QgsVectorLayer("PointZ?crs=EPSG:7843", "gda2020points", "memory")
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.crs().authid(), "EPSG:7843")

        self.assertEqual(vl.crs3D().horizontalCrs().authid(), "EPSG:7843")

        f = QgsFeature()
        f.setGeometry(QgsPoint(134.445567853, -23.445567853, 5543.325))
        self.assertTrue(vl.dataProvider().addFeature(f))

        # AVWS
        dest_crs, msg = QgsCoordinateReferenceSystem.createCompoundCrs(
            QgsCoordinateReferenceSystem("EPSG:7844"),
            QgsCoordinateReferenceSystem("EPSG:9458"),
        )
        self.assertFalse(msg)
        self.assertTrue(dest_crs.isValid())
        self.assertEqual(dest_crs.horizontalCrs().authid(), "EPSG:7844")
        self.assertEqual(dest_crs.verticalCrs().authid(), "EPSG:9458")

        available_operations = QgsDatumTransform.operations(vl.crs3D(), dest_crs)
        self.assertEqual(len(available_operations[0].grids), 1)
        self.assertEqual(
            available_operations[0].grids[0].shortName, "au_ga_AGQG_20201120.tif"
        )
        if not available_operations[0].isAvailable:
            self.skipTest(
                f"Required grid {available_operations[0].grids[0].shortName} not available on system"
            )

        transform = QgsCoordinateTransform(
            vl.crs3D(), dest_crs, QgsCoordinateTransformContext()
        )
        # for debugging
        # self.assertEqual(transform.instantiatedCoordinateOperationDetails().proj, '+proj=pipeline +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +inv +proj=vgridshift +grids=au_ga_AGQG_20201120.tif +multiplier=1 +step +proj=unitconvert +xy_in=rad +xy_out=deg')

        request = QgsFeatureRequest().setCoordinateTransform(transform)

        transformed_features = list(vl.getFeatures(request))
        self.assertEqual(len(transformed_features), 1)
        geom = transformed_features[0].geometry()
        self.assertAlmostEqual(geom.constGet().x(), 134.445567853, 6)
        self.assertAlmostEqual(geom.constGet().y(), -23.445567853, 6)
        # comparing against results from https://geodesyapps.ga.gov.au/avws
        self.assertAlmostEqual(geom.constGet().z(), 5524.13969, 3)

    def test_vertical_transformation_AVWS_to_gda2020(self):
        """
        Test vertical transformations are correctly handled during iteration

        AVWS to GDA2020
        """

        # GDA2020 vertical CRS
        vl = QgsVectorLayer("PointZ?crs=EPSG:7844", "gda2020points", "memory")
        self.assertTrue(vl.isValid())

        f = QgsFeature()
        f.setGeometry(QgsPoint(134.445567853, -23.445567853, 5524.13969))
        self.assertTrue(vl.dataProvider().addFeature(f))

        # AVWS
        source_crs, msg = QgsCoordinateReferenceSystem.createCompoundCrs(
            QgsCoordinateReferenceSystem("EPSG:7844"),
            QgsCoordinateReferenceSystem("EPSG:9458"),
        )
        self.assertFalse(msg)
        self.assertTrue(source_crs.isValid())
        self.assertEqual(source_crs.horizontalCrs().authid(), "EPSG:7844")
        self.assertEqual(source_crs.verticalCrs().authid(), "EPSG:9458")

        available_operations = QgsDatumTransform.operations(
            source_crs, QgsCoordinateReferenceSystem("EPSG:7843")
        )
        self.assertEqual(len(available_operations[0].grids), 1)
        self.assertEqual(
            available_operations[0].grids[0].shortName, "au_ga_AGQG_20201120.tif"
        )
        if not available_operations[0].isAvailable:
            self.skipTest(
                f"Required grid {available_operations[0].grids[0].shortName} not available on system"
            )

        # dest CRS is GDA2020

        transform = QgsCoordinateTransform(
            source_crs,
            QgsCoordinateReferenceSystem("EPSG:7843"),
            QgsCoordinateTransformContext(),
        )
        # for debugging
        # self.assertEqual(transform.instantiatedCoordinateOperationDetails().proj, '+proj=pipeline +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +inv +proj=vgridshift +grids=au_ga_AGQG_20201120.tif +multiplier=1 +step +proj=unitconvert +xy_in=rad +xy_out=deg')

        request = QgsFeatureRequest().setCoordinateTransform(transform)

        transformed_features = list(vl.getFeatures(request))
        self.assertEqual(len(transformed_features), 1)
        geom = transformed_features[0].geometry()
        self.assertAlmostEqual(geom.constGet().x(), 134.445567853, 6)
        self.assertAlmostEqual(geom.constGet().y(), -23.445567853, 6)
        # comparing against results from https://geodesyapps.ga.gov.au/avws
        self.assertAlmostEqual(geom.constGet().z(), 5543.325, 3)

    def test_vertical_transformation_gda2020_to_AHD(self):
        """
        Test vertical transformations are correctly handled during iteration

        GDA2020 to AHD
        """

        # GDA2020 vertical CRS
        vl = QgsVectorLayer("PointZ?crs=EPSG:7843", "gda2020points", "memory")
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.crs().authid(), "EPSG:7843")

        self.assertEqual(vl.crs3D().horizontalCrs().authid(), "EPSG:7843")

        f = QgsFeature()
        f.setGeometry(QgsPoint(134.445567853, -23.445567853, 5543.325))
        self.assertTrue(vl.dataProvider().addFeature(f))

        # AHD
        dest_crs, msg = QgsCoordinateReferenceSystem.createCompoundCrs(
            QgsCoordinateReferenceSystem("EPSG:7844"),
            QgsCoordinateReferenceSystem("EPSG:5711"),
        )
        self.assertFalse(msg)
        self.assertTrue(dest_crs.isValid())
        self.assertEqual(dest_crs.horizontalCrs().authid(), "EPSG:7844")
        self.assertEqual(dest_crs.verticalCrs().authid(), "EPSG:5711")

        available_operations = QgsDatumTransform.operations(vl.crs3D(), dest_crs)
        self.assertEqual(len(available_operations[0].grids), 1)
        self.assertEqual(
            available_operations[0].grids[0].shortName,
            "au_ga_AUSGeoid2020_20180201.tif",
        )
        if not available_operations[0].isAvailable:
            self.skipTest(
                f"Required grid {available_operations[0].grids[0].shortName} not available on system"
            )

        transform = QgsCoordinateTransform(
            vl.crs3D(), dest_crs, QgsCoordinateTransformContext()
        )
        # for debugging
        # self.assertEqual(transform.instantiatedCoordinateOperationDetails().proj, '+proj=pipeline +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +inv +proj=vgridshift +grids=au_ga_AGQG_20201120.tif +multiplier=1 +step +proj=unitconvert +xy_in=rad +xy_out=deg')

        request = QgsFeatureRequest().setCoordinateTransform(transform)

        transformed_features = list(vl.getFeatures(request))
        self.assertEqual(len(transformed_features), 1)
        geom = transformed_features[0].geometry()
        self.assertAlmostEqual(geom.constGet().x(), 134.445567853, 6)
        self.assertAlmostEqual(geom.constGet().y(), -23.445567853, 6)
        # comparing against results from https://geodesyapps.ga.gov.au/ausgeoid2020
        self.assertAlmostEqual(geom.constGet().z(), 5523.598, 3)

    def test_vertical_transformation_AHD_to_gda2020(self):
        """
        Test vertical transformations are correctly handled during iteration

        AHD to GDA2020
        """

        # GDA2020 vertical CRS
        vl = QgsVectorLayer("PointZ?crs=EPSG:7844", "gda2020points", "memory")
        self.assertTrue(vl.isValid())

        f = QgsFeature()
        f.setGeometry(QgsPoint(134.445567853, -23.445567853, 5523.598))
        self.assertTrue(vl.dataProvider().addFeature(f))

        # AHD
        source_crs, msg = QgsCoordinateReferenceSystem.createCompoundCrs(
            QgsCoordinateReferenceSystem("EPSG:7844"),
            QgsCoordinateReferenceSystem("EPSG:5711"),
        )
        self.assertFalse(msg)
        self.assertTrue(source_crs.isValid())
        self.assertEqual(source_crs.horizontalCrs().authid(), "EPSG:7844")
        self.assertEqual(source_crs.verticalCrs().authid(), "EPSG:5711")

        # dest CRS is GDA2020

        available_operations = QgsDatumTransform.operations(
            source_crs, QgsCoordinateReferenceSystem("EPSG:7843")
        )
        self.assertEqual(len(available_operations[0].grids), 1)
        self.assertEqual(
            available_operations[0].grids[0].shortName,
            "au_ga_AUSGeoid2020_20180201.tif",
        )
        if not available_operations[0].isAvailable:
            self.skipTest(
                f"Required grid {available_operations[0].grids[0].shortName} not available on system"
            )

        transform = QgsCoordinateTransform(
            source_crs,
            QgsCoordinateReferenceSystem("EPSG:7843"),
            QgsCoordinateTransformContext(),
        )
        # for debugging
        # self.assertEqual(transform.instantiatedCoordinateOperationDetails().proj, '+proj=pipeline +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +inv +proj=vgridshift +grids=au_ga_AGQG_20201120.tif +multiplier=1 +step +proj=unitconvert +xy_in=rad +xy_out=deg')

        request = QgsFeatureRequest().setCoordinateTransform(transform)

        transformed_features = list(vl.getFeatures(request))
        self.assertEqual(len(transformed_features), 1)
        geom = transformed_features[0].geometry()
        self.assertAlmostEqual(geom.constGet().x(), 134.445567853, 6)
        self.assertAlmostEqual(geom.constGet().y(), -23.445567853, 6)
        # comparing against results from https://geodesyapps.ga.gov.au/avws
        self.assertAlmostEqual(geom.constGet().z(), 5543.325, 3)


if __name__ == "__main__":
    unittest.main()
