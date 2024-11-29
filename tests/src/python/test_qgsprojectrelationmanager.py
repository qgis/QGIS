"""QGIS Unit tests for QgsRelationManager

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "David Marteau"
__date__ = "19/12/2019"
__copyright__ = "Copyright 2019, The QGIS Project"

import os

from qgis.core import (
    QgsProject,
    QgsRelation,
    QgsVectorLayer,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()


#
# Check consistency of relations when getting manager from project
#
# We want to make sure that updated relation from project which is not the global project
# instance are valid
#


def createReferencingLayer():
    layer = QgsVectorLayer(
        "Point?field=fldtxt:string&field=foreignkey:integer",
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


class TestQgsProjectRelationManager(QgisTestCase):

    def setUp(self):
        self.referencedLayer = createReferencedLayer()
        self.referencingLayer = createReferencingLayer()

        self.project = QgsProject()
        self.project.addMapLayers([self.referencedLayer, self.referencingLayer])

    def test_addRelation(self):
        """test adding relations to a manager"""
        manager = self.project.relationManager()
        relations = manager.relations()
        self.assertEqual(len(relations), 0)

        rel = QgsRelation(manager.context())
        rel.setReferencingLayer(self.referencingLayer.id())
        rel.setReferencedLayer(self.referencedLayer.id())
        rel.addFieldPair("foreignkey", "y")

        rel.setId("rel1")
        rel.setName("Relation Number One")
        assert rel.isValid()

        manager.addRelation(rel)

        relations = manager.relations()
        self.assertEqual(len(relations), 1)
        self.assertEqual(relations["rel1"].id(), "rel1")

    def test_loadRelation(self):
        """Test loading relation with project"""
        project = QgsProject()
        project.read(
            os.path.join(
                unitTestDataPath(), "projects", "test-project-with-relations.qgs"
            )
        )

        manager = project.relationManager()
        relations = manager.relations()
        assert len(relations) > 0


if __name__ == "__main__":
    unittest.main()
