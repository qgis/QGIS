# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsRelationManager.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '17/05/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.core import (QgsVectorLayer,
                       QgsRelation,
                       QgsRelationManager,
                       QgsMapLayerRegistry
                       )
from qgis.testing import start_app, unittest

start_app()


def createReferencingLayer():
    layer = QgsVectorLayer("Point?field=fldtxt:string&field=foreignkey:integer",
                           "referencinglayer", "memory")
    return layer


def createReferencedLayer():
    layer = QgsVectorLayer(
        "Point?field=x:string&field=y:integer&field=z:integer",
        "referencedlayer", "memory")
    return layer


class TestQgsRelationManager(unittest.TestCase):

    def setUp(self):
        self.referencedLayer = createReferencedLayer()
        self.referencingLayer = createReferencingLayer()
        QgsMapLayerRegistry.instance().addMapLayers([self.referencedLayer, self.referencingLayer])

    def tearDown(self):
        QgsMapLayerRegistry.instance().removeAllMapLayers()

    def createRelation(self):
        rel = QgsRelation()
        rel.setReferencingLayer(self.referencingLayer.id())
        rel.setReferencedLayer(self.referencedLayer.id())
        rel.addFieldPair('foreignkey', 'y')
        return rel

    def test_addRelation(self):
        """ test adding relations to a manager """
        manager = QgsRelationManager()

        relations = manager.relations()
        self.assertEqual(len(relations), 0)

        rel = self.createRelation()
        rel.setRelationId('rel1')
        rel.setRelationName('Relation Number One')
        assert rel.isValid()

        manager.addRelation(rel)

        relations = manager.relations()
        self.assertEqual(len(relations), 1)
        self.assertEqual(relations['rel1'].id(), 'rel1')

        rel = self.createRelation()
        rel.setRelationId('rel2')
        rel.setRelationName('Relation Number Two')
        assert rel.isValid()

        manager.addRelation(rel)

        relations = manager.relations()
        self.assertEqual(len(relations), 2)
        ids = [r.id() for r in relations.values()]
        self.assertEqual(set(ids), set(['rel1', 'rel2']))

    def test_relationById(self):
        """ test retrieving relation by id"""
        manager = QgsRelationManager()

        rel = manager.relation('does not exist')
        self.assertFalse(rel.isValid())

        # add two relations
        rel = self.createRelation()
        rel.setRelationId('rel1')
        rel.setRelationName('Relation Number One')
        assert rel.isValid()
        manager.addRelation(rel)
        rel = self.createRelation()
        rel.setRelationId('rel2')
        rel.setRelationName('Relation Number Two')
        assert rel.isValid()
        manager.addRelation(rel)

        rel = manager.relation('does not exist')
        self.assertFalse(rel.isValid())

        rel = manager.relation('rel1')
        self.assertEqual(rel.id(), 'rel1')

        rel = manager.relation('rel2')
        self.assertEqual(rel.id(), 'rel2')

    def test_relationByName(self):
        """ test retrieving relations by name"""
        manager = QgsRelationManager()

        rels = manager.relationsByName('does not exist')
        self.assertEqual(rels, [])

        # add some relations
        rel = self.createRelation()
        rel.setRelationId('rel1')
        rel.setRelationName('my relation')
        assert rel.isValid()
        manager.addRelation(rel)
        rel = self.createRelation()
        rel.setRelationId('rel2')
        rel.setRelationName('dupe name')
        assert rel.isValid()
        manager.addRelation(rel)
        rel = self.createRelation()
        rel.setRelationId('rel3')
        rel.setRelationName('dupe name')
        assert rel.isValid()
        manager.addRelation(rel)

        rels = manager.relationsByName('does not exist')
        self.assertEqual(rels, [])

        rels = manager.relationsByName('my relation')
        ids = [r.id() for r in rels]
        self.assertEqual(set(ids), set(['rel1']))

        # case insensitive
        rels = manager.relationsByName('My RelAtion')
        ids = [r.id() for r in rels]
        self.assertEqual(set(ids), set(['rel1']))

        # multiple results
        rels = manager.relationsByName('dupe name')
        ids = [r.id() for r in rels]
        self.assertEqual(set(ids), set(['rel2', 'rel3']))


if __name__ == '__main__':
    unittest.main()
