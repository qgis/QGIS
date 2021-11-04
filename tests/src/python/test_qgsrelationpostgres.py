# -*- coding: utf-8 -*-
"""QGIS Unit tests for relations with postgresql provider.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Loïc Bartoletti'
__date__ = '27/07/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import qgis  # NOQA

import os

from qgis.core import (
    QgsVectorLayer,
    QgsProject,
    QgsRelation,
    QgsRelationManager
)
from qgis.gui import (
    QgsMapCanvas
)
from qgis.testing import start_app, unittest

start_app()


class TestQgsRelationPostgresql(unittest.TestCase):

    @classmethod
    def setUpClass(cls):

        cls.dbconn = 'service=\'qgis_test\''
        if 'QGIS_PGTEST_DB' in os.environ:
            cls.dbconn = os.environ['QGIS_PGTEST_DB']

        cls.relMgr = QgsProject.instance().relationManager()

    def setUp(self):
        """
        Setup the involved layers and relations for a n:m relation
        :return:
        """
        QgsProject.instance().clear()

    def tearDown(self):
        QgsProject.instance().removeAllMapLayers()

    def test_discover_relations(self):
        """
        Test the automatic discovery of relations
        """

        # Create test layer
        tables = ['c_amgmt_amgmt_lot', 'c_batiment_bat_lot', 'c_ens_immo_amgmt', 'c_ens_immo_bat', 'c_terrain_ens_immo', 't_actes', 't_adresse', 't_amgmt', 't_amgmt_lot', 't_bat', 't_bat_lot', 't_ens_immo', 't_terrain']  # spellok
        vl_tables = ['vl_c_amgmt_amgmt_lot', 'vl_c_batiment_bat_lot', 'vl_c_ens_immo_amgmt', 'vl_c_ens_immo_bat', 'vl_c_terrain_ens_immo', 'vl_t_actes', 'vl_t_adresse', 'vl_t_amgmt', 'vl_t_amgmt_lot', 'vl_t_bat', 'vl_t_bat_lot', 'vl_t_ens_immo', 'vl_t_terrain']  # spellok

        for i in range(len(tables)):
            vl_tables[i] = QgsVectorLayer(self.dbconn + ' sslmode=disable key=\'pk\' table="relations"."{}" sql='.format(tables[i]), tables[i], 'postgres')
            assert(vl_tables[i].isValid())
            QgsProject.instance().addMapLayer(vl_tables[i])

        relations = self.relMgr.discoverRelations([], vl_tables)
        for r in relations:
            self.assertTrue(r.isValid())
        self.assertEqual(len(relations), 18)
        self.assertEqual(sum([len(r.referencingFields()) for r in relations]), 18)
        self.assertEqual(sum([len(r.referencedFields()) for r in relations]), 18)

    def test_discover_relations_spaced(self):
        """Test regression https://github.com/qgis/QGIS/issues/39025 and https://github.com/qgis/QGIS/issues/39036"""

        vl_parent = QgsVectorLayer(self.dbconn + ' sslmode=disable key=\'pk\' table="spaced schema"."spaced parent" sql=', 'parent', 'postgres')
        self.assertTrue(vl_parent.isValid())
        vl_child = QgsVectorLayer(self.dbconn + ' sslmode=disable key=\'pk\' table="spaced schema"."spaced child" sql=', 'child', 'postgres')
        self.assertTrue(vl_child.isValid())

        QgsProject.instance().clear()
        QgsProject.instance().addMapLayers([vl_child, vl_parent])

        relations = self.relMgr.discoverRelations([], [vl_child, vl_parent])
        self.assertEqual(len(relations), 1)

    def test_relation_with_mismatching_field_pairs_type(self):
        """
        mix field types (fkey is text, attribute is integer)
        """
        vl_product = QgsVectorLayer(self.dbconn + ' sslmode=disable key=\'id\' table="qgis_test"."product" sql=', 'product', 'postgres')
        vl_owner = QgsVectorLayer(self.dbconn + ' sslmode=disable key=\'id\' table="qgis_test"."owner" sql=', 'owner', 'postgres')

        QgsProject.instance().addMapLayers([vl_product, vl_owner])
        layers = QgsProject.instance().mapLayers()

        self.assertTrue(vl_product.isValid())
        self.assertTrue(vl_owner.isValid())

        # fkey is int, id is text
        rel = QgsRelation()
        rel.setId('rel1')
        rel.setName('Super Relation')
        rel.setReferencingLayer(vl_product.id())
        rel.setReferencedLayer(vl_owner.id())
        rel.addFieldPair('fk_owner_int', 'id_text')
        # referenced
        referencing_feature = next(vl_product.getFeatures("name = 'Flying suit'"))
        self.assertTrue(referencing_feature.isValid())
        referenced_feature = rel.getReferencedFeature(referencing_feature)
        self.assertTrue(referenced_feature.isValid())
        self.assertEqual(referenced_feature[2], 'Superman')
        # referencing
        referencing_feature = next(rel.getRelatedFeatures(referenced_feature))
        self.assertTrue(referencing_feature.isValid())
        self.assertEqual(referencing_feature[1], 'Flying suit')

        # fkey is text, id is int
        rel2 = QgsRelation()
        rel2.setId('rel2')
        rel2.setName('Super Relation 2')
        rel2.setReferencingLayer(vl_product.id())
        rel2.setReferencedLayer(vl_owner.id())
        rel2.addFieldPair('fk_owner_text', 'id')
        referencing_feature = next(vl_product.getFeatures("name = 'Flying suit'"))
        self.assertTrue(referencing_feature.isValid())
        referenced_feature = rel.getReferencedFeature(referencing_feature)
        self.assertTrue(referenced_feature.isValid())
        self.assertEqual(referenced_feature[2], 'Superman')
        # referencing
        referencing_feature = next(rel.getRelatedFeatures(referenced_feature))
        self.assertTrue(referencing_feature.isValid())
        self.assertEqual(referencing_feature[1], 'Flying suit')


if __name__ == '__main__':
    unittest.main()
