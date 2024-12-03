"""QGIS Unit tests for QgsRelationManager.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "17/05/2016"
__copyright__ = "Copyright 2016, The QGIS Project"


from qgis.core import (
    QgsPolymorphicRelation,
    QgsProject,
    QgsRelation,
    QgsRelationManager,
    QgsVectorLayer,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


def createReferencingLayer():
    layer = QgsVectorLayer(
        "Point?field=fldtxt:string&field=foreignkey:integer&field=referenced_layer:string&field=referenced_fid:string",
        "referencinglayer",
        "memory",
    )
    return layer


def createReferencedLayer():
    layer = QgsVectorLayer(
        "Point?field=x:string&field=y:integer&field=z:integer",
        "referencedlayer",
        "memory",
    )
    return layer


class TestQgsRelationManager(QgisTestCase):

    def setUp(self):
        self.referencedLayer = createReferencedLayer()
        self.referencingLayer = createReferencingLayer()
        QgsProject.instance().addMapLayers(
            [self.referencedLayer, self.referencingLayer]
        )

    def tearDown(self):
        QgsProject.instance().removeAllMapLayers()

    def createRelation(self):
        rel = QgsRelation()
        rel.setReferencingLayer(self.referencingLayer.id())
        rel.setReferencedLayer(self.referencedLayer.id())
        rel.addFieldPair("foreignkey", "y")
        return rel

    def createPolymorphicRelation(self):
        polyRel = QgsPolymorphicRelation()
        polyRel.setReferencingLayer(self.referencingLayer.id())
        polyRel.setReferencedLayerIds([self.referencedLayer.id()])
        polyRel.setReferencedLayerField("referenced_layer")
        polyRel.setReferencedLayerExpression("@layer_name")
        polyRel.addFieldPair("referenced_fid", "x")
        return polyRel

    def test_addRelation(self):
        """test adding relations to a manager"""
        manager = QgsRelationManager()

        relations = manager.relations()
        self.assertEqual(len(relations), 0)

        rel = self.createRelation()
        rel.setId("rel1")
        rel.setName("Relation Number One")
        assert rel.isValid()

        manager.addRelation(rel)

        relations = manager.relations()
        self.assertEqual(len(relations), 1)
        self.assertEqual(relations["rel1"].id(), "rel1")

        rel = self.createRelation()
        rel.setId("rel2")
        rel.setName("Relation Number Two")
        assert rel.isValid()

        manager.addRelation(rel)

        relations = manager.relations()
        self.assertEqual(len(relations), 2)
        ids = [r.id() for r in list(relations.values())]
        self.assertEqual(set(ids), {"rel1", "rel2"})

    def test_relationById(self):
        """test retrieving relation by id"""
        manager = QgsRelationManager()

        rel = manager.relation("does not exist")
        self.assertFalse(rel.isValid())

        # add two relations
        rel = self.createRelation()
        rel.setId("rel1")
        rel.setName("Relation Number One")
        assert rel.isValid()
        manager.addRelation(rel)
        rel = self.createRelation()
        rel.setId("rel2")
        rel.setName("Relation Number Two")
        assert rel.isValid()
        manager.addRelation(rel)

        rel = manager.relation("does not exist")
        self.assertFalse(rel.isValid())

        rel = manager.relation("rel1")
        self.assertEqual(rel.id(), "rel1")

        rel = manager.relation("rel2")
        self.assertEqual(rel.id(), "rel2")

    def test_relationByName(self):
        """test retrieving relations by name"""
        manager = QgsRelationManager()

        rels = manager.relationsByName("does not exist")
        self.assertEqual(rels, [])

        # add some relations
        rel = self.createRelation()
        rel.setId("rel1")
        rel.setName("my relation")
        assert rel.isValid()
        manager.addRelation(rel)
        rel = self.createRelation()
        rel.setId("rel2")
        rel.setName("dupe name")
        assert rel.isValid()
        manager.addRelation(rel)
        rel = self.createRelation()
        rel.setId("rel3")
        rel.setName("dupe name")
        assert rel.isValid()
        manager.addRelation(rel)

        rels = manager.relationsByName("does not exist")
        self.assertEqual(rels, [])

        rels = manager.relationsByName("my relation")
        ids = [r.id() for r in rels]
        self.assertEqual(set(ids), {"rel1"})

        # case insensitive
        rels = manager.relationsByName("My RelAtion")
        ids = [r.id() for r in rels]
        self.assertEqual(set(ids), {"rel1"})

        # multiple results
        rels = manager.relationsByName("dupe name")
        ids = [r.id() for r in rels]
        self.assertEqual(set(ids), {"rel2", "rel3"})

    def test_setPolymorphicRelations(self):
        # tests polymorphicRelations/setPolymorphicRelations

        manager = QgsRelationManager()

        # initially the map should be empty
        self.assertListEqual(list(manager.polymorphicRelations()), [])

        poly_rel1 = self.createPolymorphicRelation()
        poly_rel1.setId("poly_rel1")
        poly_rel1.setName("Poly Rel 1")
        poly_rel2 = self.createPolymorphicRelation()
        poly_rel2.setId("poly_rel2")
        poly_rel2.setName("Poly Rel 2")
        poly_rel3 = self.createPolymorphicRelation()
        poly_rel3.setId("poly_rel3")
        poly_rel3.setName("Poly Rel 3")

        # the relation should be valid now
        self.assertTrue(poly_rel1.isValid())
        self.assertTrue(poly_rel2.isValid())
        self.assertTrue(poly_rel3.isValid())

        manager.setPolymorphicRelations([poly_rel1, poly_rel2, poly_rel3])

        # the relations should match
        self.assertListEqual(
            list(manager.polymorphicRelations()),
            ["poly_rel1", "poly_rel2", "poly_rel3"],
        )
        self.assertTrue(manager.polymorphicRelations()["poly_rel1"])
        self.assertTrue(manager.polymorphicRelations()["poly_rel2"])
        self.assertTrue(manager.polymorphicRelations()["poly_rel3"])
        self.assertTrue(
            manager.polymorphicRelations()["poly_rel1"].hasEqualDefinition(poly_rel1)
        )
        self.assertTrue(
            manager.polymorphicRelations()["poly_rel2"].hasEqualDefinition(poly_rel2)
        )
        self.assertTrue(
            manager.polymorphicRelations()["poly_rel3"].hasEqualDefinition(poly_rel3)
        )

        manager.setPolymorphicRelations([poly_rel1])
        self.assertListEqual(list(manager.polymorphicRelations()), ["poly_rel1"])
        manager.setPolymorphicRelations([])
        self.assertListEqual(list(manager.polymorphicRelations()), [])

    def test_polymorphicRelation(self):
        # tests addPolymorphicRelation/polymorphicRelation

        manager = QgsRelationManager()

        # initially the map should be empty
        self.assertFalse(manager.polymorphicRelations())

        poly_rel1 = self.createPolymorphicRelation()
        poly_rel1.setId("poly_rel1")
        poly_rel1.setName("Poly Rel 1")
        poly_rel2 = self.createPolymorphicRelation()
        poly_rel2.setId("poly_rel2")
        poly_rel2.setName("Poly Rel 2")

        # the relation should be valid now
        self.assertTrue(poly_rel1.isValid())
        self.assertTrue(poly_rel2.isValid())

        manager.addPolymorphicRelation(poly_rel1)
        manager.addPolymorphicRelation(poly_rel2)

        self.assertFalse(manager.polymorphicRelation("poly_invalid_id").isValid())
        self.assertTrue(manager.polymorphicRelation("poly_rel1").isValid())
        self.assertTrue(manager.polymorphicRelation("poly_rel2").isValid())
        self.assertTrue(
            manager.polymorphicRelation("poly_rel1").hasEqualDefinition(poly_rel1)
        )
        self.assertTrue(
            manager.polymorphicRelation("poly_rel2").hasEqualDefinition(poly_rel2)
        )

    def test_removePolymorphicRelation(self):
        # tests addPolymorphicRelation/removePolymorphicRelation

        manager = QgsRelationManager()

        # initially the map should be empty
        self.assertFalse(manager.polymorphicRelations())
        self.assertFalse(manager.relations())

        poly_rel1 = self.createPolymorphicRelation()
        poly_rel1.setId("poly_rel1")
        poly_rel1.setName("Poly Rel 1")
        poly_rel2 = self.createPolymorphicRelation()
        poly_rel2.setId("poly_rel2")
        poly_rel2.setName("Poly Rel 2")

        # the relation should be valid now
        self.assertTrue(poly_rel1.isValid())
        self.assertTrue(poly_rel2.isValid())
        self.assertFalse(manager.polymorphicRelation("poly_invalid_id").isValid())
        self.assertListEqual(
            [r.polymorphicRelationId() for r in manager.relations().values()], []
        )

        manager.addPolymorphicRelation(poly_rel1)
        self.assertTrue(manager.polymorphicRelation("poly_rel1").isValid())
        self.assertTrue(
            manager.polymorphicRelation("poly_rel1").hasEqualDefinition(poly_rel1)
        )
        self.assertListEqual(
            [r.polymorphicRelationId() for r in manager.relations().values()],
            [poly_rel1.id()],
        )

        manager.addPolymorphicRelation(poly_rel2)
        self.assertTrue(manager.polymorphicRelation("poly_rel2").isValid())
        self.assertTrue(
            manager.polymorphicRelation("poly_rel2").hasEqualDefinition(poly_rel2)
        )
        self.assertListEqual(
            [r.polymorphicRelationId() for r in manager.relations().values()],
            [poly_rel1.id(), poly_rel2.id()],
        )

        manager.removePolymorphicRelation(poly_rel1.id())
        self.assertFalse(manager.polymorphicRelation("poly_rel1").isValid())
        self.assertTrue(manager.polymorphicRelation("poly_rel2").isValid())
        self.assertListEqual(
            [r.polymorphicRelationId() for r in manager.relations().values()],
            [poly_rel2.id()],
        )
        manager.removePolymorphicRelation(poly_rel2.id())
        self.assertFalse(manager.polymorphicRelation("poly_rel1").isValid())
        self.assertFalse(manager.polymorphicRelation("poly_rel2").isValid())
        self.assertListEqual(
            [r.polymorphicRelationId() for r in manager.relations().values()], []
        )


if __name__ == "__main__":
    unittest.main()
