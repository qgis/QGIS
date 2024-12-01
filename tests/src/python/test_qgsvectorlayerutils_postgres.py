"""QGIS Unit tests for QgsVectorLayerUtils in a Postgres database.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Julien Cabieces"
__date__ = "22/03/2021"
__copyright__ = "Copyright 2021, The QGIS Project"

import os

from qgis.core import (
    NULL,
    QgsDefaultValue,
    QgsVectorLayer,
    QgsVectorLayerUtils,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsVectorLayerUtilsPostgres(QgisTestCase):

    def testCreateFeature(self):
        """test creating a feature respecting unique values of postgres provider"""
        layer = QgsVectorLayer(
            "Point?field=fldtxt:string&field=fldint:integer&field=flddbl:double",
            "addfeat",
            "memory",
        )

        # init connection string
        dbconn = "service=qgis_test"
        if "QGIS_PGTEST_DB" in os.environ:
            dbconn = os.environ["QGIS_PGTEST_DB"]

        # create a vector layer
        pg_layer = QgsVectorLayer(
            f'{dbconn} table="qgis_test"."authors" sql=', "authors", "postgres"
        )
        self.assertTrue(pg_layer.isValid())
        # check the default clause
        default_clause = "nextval('qgis_test.authors_pk_seq'::regclass)"
        self.assertEqual(pg_layer.dataProvider().defaultValueClause(0), default_clause)

        # though default_clause is after the first create not unique (until save), it should fill up all the features with it
        pg_layer.startEditing()
        f = QgsVectorLayerUtils.createFeature(pg_layer)
        self.assertEqual(f.attributes(), [default_clause, NULL])
        self.assertTrue(pg_layer.addFeatures([f]))
        self.assertTrue(QgsVectorLayerUtils.valueExists(pg_layer, 0, default_clause))
        f = QgsVectorLayerUtils.createFeature(pg_layer)
        self.assertEqual(f.attributes(), [default_clause, NULL])
        self.assertTrue(pg_layer.addFeatures([f]))
        f = QgsVectorLayerUtils.createFeature(pg_layer)
        self.assertEqual(f.attributes(), [default_clause, NULL])
        self.assertTrue(pg_layer.addFeatures([f]))
        # if a unique value is passed, use it
        f = QgsVectorLayerUtils.createFeature(pg_layer, attributes={0: 40, 1: NULL})
        self.assertEqual(f.attributes(), [40, NULL])
        # and if a default value is configured use it as well
        pg_layer.setDefaultValueDefinition(0, QgsDefaultValue("11*4"))
        f = QgsVectorLayerUtils.createFeature(pg_layer)
        self.assertEqual(f.attributes(), [44, NULL])
        pg_layer.rollBack()


if __name__ == "__main__":
    unittest.main()
