"""QGIS Unit tests for QgsPolymorphicRelation.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Ivan Ivanov"
__date__ = "11/1/2021"
__copyright__ = "Copyright 2021, QGIS Project"


from qgis.core import (
    QgsFeature,
    QgsGeometry,
    QgsPointXY,
    QgsPolymorphicRelation,
    QgsProject,
    QgsVectorLayer,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


def createReferencingLayer():
    layer = QgsVectorLayer(
        "NoGeometry?field=fid:integer&field=referenced_layer:string&field=referenced_fid:string&field=url:string",
        "referencinglayer",
        "memory",
    )
    assert layer.isValid()
    f1 = QgsFeature()
    f1.setFields(layer.fields())
    f1.setAttributes([1, "referencedlayer1", "foo", "./file1.jpg"])
    f2 = QgsFeature()
    f2.setFields(layer.fields())
    f2.setAttributes([2, "referencedlayer1", "foo", "./file2.jpg"])
    f3 = QgsFeature()
    f3.setFields(layer.fields())
    f3.setAttributes([3, "referencedlayer1", "bar", "./file3.jpg"])
    f4 = QgsFeature()
    f4.setFields(layer.fields())
    f4.setAttributes([4, "referencedlayer2", "foobar'bar", "./file4.jpg"])
    assert layer.dataProvider().addFeatures([f1, f2, f3, f4])
    return layer


def createReferencedLayer(layer_name):
    layer = QgsVectorLayer(
        "Point?field=fid:string&field=value:integer", layer_name, "memory"
    )
    assert layer.isValid()
    f1 = QgsFeature()
    f1.setFields(layer.fields())
    f1.setAttributes(["foo", 123])
    f1.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(1, 1)))
    f2 = QgsFeature()
    f2.setFields(layer.fields())
    f2.setAttributes(["bar", 456])
    f2.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(2, 2)))
    f3 = QgsFeature()
    f3.setFields(layer.fields())
    f3.setAttributes(["foobar'bar", 789])
    f3.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(2, 3)))
    assert layer.dataProvider().addFeatures([f1, f2, f3])
    return layer


def formatAttributes(attrs):
    return repr([str(a) for a in attrs])


class TestQgsRelation(QgisTestCase):

    def setUp(self):
        self.referencedLayer1 = createReferencedLayer("referencedlayer1")
        self.referencedLayer2 = createReferencedLayer("referencedlayer2")
        self.referencingLayer = createReferencingLayer()
        QgsProject.instance().addMapLayers(
            [self.referencedLayer1, self.referencedLayer2, self.referencingLayer]
        )

    def tearDown(self):
        QgsProject.instance().removeAllMapLayers()

    def test_isValid(self):
        poly_rel = QgsPolymorphicRelation()
        self.assertFalse(poly_rel.isValid())

        poly_rel.setId("poly_rel1")
        self.assertFalse(poly_rel.isValid())

        poly_rel.setName("Polymorphic Relation Number One")
        self.assertFalse(poly_rel.isValid())

        poly_rel.setReferencingLayer(self.referencingLayer.id())
        self.assertFalse(poly_rel.isValid())

        poly_rel.setReferencedLayerIds(
            [self.referencedLayer1.id(), self.referencedLayer2.id()]
        )
        self.assertFalse(poly_rel.isValid())

        poly_rel.setReferencedLayerField("referenced_layer")
        self.assertFalse(poly_rel.isValid())

        poly_rel.setReferencedLayerExpression("@layer_name")
        self.assertFalse(poly_rel.isValid())

        poly_rel.addFieldPair("referenced_fid", "fid")
        self.assertTrue(poly_rel.isValid())

    def test_setId(self):
        poly_rel = QgsPolymorphicRelation()
        self.assertEqual(poly_rel.id(), "")
        poly_rel.setId("poly_rel_1")
        self.assertEqual(poly_rel.id(), "poly_rel_1")

    def test_setName(self):
        poly_rel = QgsPolymorphicRelation()
        self.assertEqual(poly_rel.name(), 'Polymorphic relations for "<NO LAYER>"')
        poly_rel.setReferencingLayer(self.referencingLayer.id())
        self.assertEqual(
            poly_rel.name(), 'Polymorphic relations for "referencinglayer"'
        )
        poly_rel.setName("Polymorphic Relation 1")
        self.assertEqual(poly_rel.name(), "Polymorphic Relation 1")

    def test_setReferencingLayer(self):
        poly_rel = QgsPolymorphicRelation()
        self.assertEqual(poly_rel.referencingLayerId(), "")
        poly_rel.setReferencingLayer(self.referencingLayer.id())
        self.assertEqual(poly_rel.referencingLayerId(), self.referencingLayer.id())

    def test_setReferencedLayerIds(self):
        poly_rel = QgsPolymorphicRelation()
        self.assertListEqual(poly_rel.referencedLayerIds(), [])
        poly_rel.setReferencedLayerIds(
            [self.referencedLayer1.id(), self.referencedLayer2.id()]
        )
        self.assertListEqual(
            poly_rel.referencedLayerIds(),
            [self.referencedLayer1.id(), self.referencedLayer2.id()],
        )

    def test_setReferencedLayerField(self):
        poly_rel = QgsPolymorphicRelation()
        self.assertEqual(poly_rel.referencedLayerField(), "")
        poly_rel.setReferencedLayerField(self.referencingLayer.id())
        self.assertEqual(poly_rel.referencedLayerField(), self.referencingLayer.id())

    def test_setReferencedLayerExpression(self):
        poly_rel = QgsPolymorphicRelation()
        self.assertEqual(poly_rel.referencedLayerExpression(), "")
        poly_rel.setReferencedLayerExpression("@layer_name")
        self.assertEqual(poly_rel.referencedLayerExpression(), "@layer_name")

    def test_addFieldPair(self):
        poly_rel = QgsPolymorphicRelation()
        self.assertEqual(poly_rel.fieldPairs(), {})
        poly_rel.addFieldPair("referenced_fid", "fid")
        self.assertEqual(poly_rel.fieldPairs(), {"referenced_fid": "fid"})

    def test_layerRepresentation(self):
        poly_rel = QgsPolymorphicRelation()
        poly_rel.setId("poly_rel1")
        poly_rel.setName("Polymorphic Relation Number One")
        poly_rel.setReferencingLayer(self.referencingLayer.id())
        poly_rel.setReferencedLayerIds(
            [self.referencedLayer1.id(), self.referencedLayer2.id()]
        )
        poly_rel.setReferencedLayerField("referenced_layer")
        poly_rel.setReferencedLayerExpression("@layer_name")
        poly_rel.addFieldPair("referenced_fid", "fid")

        self.assertEqual(
            poly_rel.layerRepresentation(self.referencedLayer1), "referencedlayer1"
        )

    def test_generateRelations(self):
        poly_rel = QgsPolymorphicRelation()
        poly_rel.setId("poly_rel1")
        poly_rel.setName("Polymorphic Relation Number One")
        poly_rel.setReferencingLayer(self.referencingLayer.id())
        poly_rel.setReferencedLayerIds(
            [self.referencedLayer1.id(), self.referencedLayer2.id()]
        )
        poly_rel.setReferencedLayerField("referenced_layer")
        poly_rel.setReferencedLayerExpression("@layer_name")
        poly_rel.addFieldPair("referenced_fid", "fid")

        QgsProject.instance().relationManager().addPolymorphicRelation(poly_rel)

        self.assertTrue(poly_rel.isValid())

        rels = poly_rel.generateRelations()

        self.assertEqual(len(rels), 2)

        rel1, rel2 = rels

        self.assertTrue(rel1.isValid())
        self.assertEqual(rel1.polymorphicRelationId(), poly_rel.id())
        self.assertEqual(rel1.referencingLayer(), poly_rel.referencingLayer())
        self.assertEqual(rel1.referencedLayer(), self.referencedLayer1)
        self.assertEqual(rel1.fieldPairs(), {"referenced_fid": "fid"})

        features = list(self.referencedLayer1.getFeatures())
        self.assertEqual(len(features), 3)
        self.assertEqual(
            rel1.getRelatedFeaturesFilter(features[0]),
            "\"referenced_layer\" = 'referencedlayer1' AND \"referenced_fid\" = 'foo'",
        )
        it = rel1.getRelatedFeatures(features[0])
        self.assertListEqual(
            [f.attributes() for f in it],
            [
                [1, "referencedlayer1", "foo", "./file1.jpg"],
                [2, "referencedlayer1", "foo", "./file2.jpg"],
            ],
        )

        self.assertEqual(
            rel1.getRelatedFeaturesFilter(features[1]),
            "\"referenced_layer\" = 'referencedlayer1' AND \"referenced_fid\" = 'bar'",
        )
        it = rel1.getRelatedFeatures(features[1])
        self.assertListEqual(
            [f.attributes() for f in it],
            [
                [3, "referencedlayer1", "bar", "./file3.jpg"],
            ],
        )

        self.assertEqual(
            rel1.getRelatedFeaturesFilter(features[2]),
            "\"referenced_layer\" = 'referencedlayer1' AND \"referenced_fid\" = 'foobar''bar'",
        )
        it = rel1.getRelatedFeatures(features[2])
        self.assertListEqual([f.attributes() for f in it], [])

        self.assertTrue(rel2.isValid())
        self.assertEqual(rel2.polymorphicRelationId(), poly_rel.id())
        self.assertEqual(rel2.polymorphicRelationId(), poly_rel.id())
        self.assertEqual(rel2.referencingLayer(), poly_rel.referencingLayer())
        self.assertEqual(rel2.referencedLayer(), self.referencedLayer2)
        self.assertEqual(rel2.fieldPairs(), {"referenced_fid": "fid"})

        features = list(self.referencedLayer2.getFeatures())
        self.assertEqual(len(features), 3)
        self.assertEqual(
            rel2.getRelatedFeaturesFilter(features[0]),
            "\"referenced_layer\" = 'referencedlayer2' AND \"referenced_fid\" = 'foo'",
        )
        it = rel2.getRelatedFeatures(features[0])
        self.assertListEqual([f.attributes() for f in it], [])

        self.assertEqual(
            rel2.getRelatedFeaturesFilter(features[1]),
            "\"referenced_layer\" = 'referencedlayer2' AND \"referenced_fid\" = 'bar'",
        )
        it = rel2.getRelatedFeatures(features[1])
        self.assertListEqual([f.attributes() for f in it], [])

        self.assertEqual(
            rel2.getRelatedFeaturesFilter(features[2]),
            "\"referenced_layer\" = 'referencedlayer2' AND \"referenced_fid\" = 'foobar''bar'",
        )
        it = rel2.getRelatedFeatures(features[2])
        self.assertListEqual(
            [f.attributes() for f in it],
            [[4, "referencedlayer2", "foobar'bar", "./file4.jpg"]],
        )


if __name__ == "__main__":
    unittest.main()
