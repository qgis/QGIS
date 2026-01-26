"""QGIS Unit test utils for provider tests.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Denis Rouzaud"
__date__ = "2016-11-07"
__copyright__ = "Copyright 2015, The QGIS Project"

import os

from qgis.core import (
    QgsFeature,
    QgsFeatureRequest,
    QgsPoint,
    QgsProject,
    QgsRelation,
    QgsRelationManager,
    QgsVectorLayer,
    QgsVectorLayerTools,
    Qgis,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class SubQgsVectorLayerTools(QgsVectorLayerTools):

    def __init__(self):
        super().__init__()

    def addFeature(self, layer):
        pass

    def startEditing(self, layer):
        pass

    def stopEditing(self, layer):
        pass

    def saveEdits(self, layer):
        pass


class TestQgsVectorLayerTools(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        """
        Setup the involved layers and relations for a n:m relation
        :return:
        """
        super().setUpClass()
        cls.dbconn = "service='qgis_test'"
        if "QGIS_PGTEST_DB" in os.environ:
            cls.dbconn = os.environ["QGIS_PGTEST_DB"]

        # Create test layers
        cls.vl = QgsVectorLayer(
            cls.dbconn
            + ' sslmode=disable key=\'pk\' table="qgis_test"."someData" (geom) sql=',
            "layer",
            "postgres",
        )

        cls.vl2 = QgsVectorLayer(
            "Point?crs=EPSG:4326&field=id:integer(10,0)", "points", "memory"
        )
        f = QgsFeature(cls.vl2.fields())
        f.setGeometry(QgsPoint(1, 1))
        f.setAttributes([1])
        cls.vl2.startEditing()
        cls.vl2.addFeature(f)
        cls.vl2.commitChanges()

        cls.vl3 = QgsVectorLayer(
            "NoGeometry?crs=EPSG:4326&field=point_id:integer(10,0)", "details", "memory"
        )
        f = QgsFeature(cls.vl3.fields())
        f.setAttributes([1])
        cls.vl3.startEditing()
        cls.vl3.addFeature(f)
        cls.vl3.addFeature(f)
        cls.vl3.commitChanges()

        QgsProject.instance().addMapLayers([cls.vl, cls.vl2, cls.vl3])

        relation = QgsRelation()
        relation.setName("test")
        relation.setReferencedLayer(cls.vl2.id())
        relation.setReferencingLayer(cls.vl3.id())
        relation.setStrength(Qgis.RelationshipStrength.Composition)
        relation.addFieldPair("point_id", "id")
        QgsProject.instance().relationManager().addRelation(relation)

        cls.vltools = SubQgsVectorLayerTools()
        cls.vltools.setProject(QgsProject.instance())

    def testCopyMoveFeature(self):
        """Test copy and move features"""
        rqst = QgsFeatureRequest()
        rqst.setFilterFid(4)
        features_count = self.vl.featureCount()
        self.vl.startEditing()
        (ok, rqst, msg) = self.vltools.copyMoveFeatures(self.vl, rqst, -0.1, 0.2)
        self.assertTrue(ok)
        self.assertEqual(self.vl.featureCount(), features_count + 1)
        for f in self.vl.getFeatures(rqst):
            geom = f.geometry()
            self.assertAlmostEqual(geom.asPoint().x(), -65.42)
            self.assertAlmostEqual(geom.asPoint().y(), 78.5)

    def testCopyMoveFeatureRelationship(self):
        """Test copy and move features"""
        rqst = QgsFeatureRequest()
        rqst.setFilterFid(1)
        self.vl2.startEditing()
        (ok, rqst, msg) = self.vltools.copyMoveFeatures(self.vl2, rqst, -0.1, 0.2)
        self.assertTrue(ok)
        self.assertEqual(self.vl3.featureCount(), 4)


if __name__ == "__main__":
    unittest.main()
