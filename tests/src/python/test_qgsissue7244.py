"""QGIS Unit tests for splitting features in vector layers

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Vincent Mora"
__date__ = "09/07/2013"
__copyright__ = "Copyright 2013, The QGIS Project"

import unittest

from qgis.core import Qgis, QgsFeature, QgsGeometry, QgsPointXY, QgsVectorLayer
from qgis.testing import QgisTestCase, start_app

start_app()


class TestQgsVectorLayerSplitFeatures(QgisTestCase):
    def test_SplitMultipolygon(self):
        """
        Split multipolygon with 4 parts into 7
        """
        layer = QgsVectorLayer("MultiPolygon?crs=EPSG:4326", "test layer", "memory")
        self.assertTrue(layer.isValid())
        self.assertTrue(layer.isSpatial())
        f = QgsFeature(layer.fields())
        f.setGeometry(
            QgsGeometry.fromWkt(
                "MULTIPOLYGON(((0 0,1 0,1 1,0 1,0 0)),((0 2,1 2,1 3,0 3,0 2)),((2 0,3 0,3 1,2 1,2 0)),((2 2,3 2,3 3,2 3,2 2)))"
            )
        )
        self.assertTrue(layer.dataProvider().addFeature(f))
        self.assertEqual(layer.featureCount(), 1)

        # cut through three of the polygons, should result in 7 features
        layer.startEditing()
        self.assertEqual(
            layer.splitFeatures([QgsPointXY(0.5, -0.5), QgsPointXY(0.5, 1.5)], 0),
            Qgis.GeometryOperationResult.Success,
        )

        self.assertEqual(
            layer.splitFeatures([QgsPointXY(2.5, -0.5), QgsPointXY(2.5, 4)], 0),
            Qgis.GeometryOperationResult.Success,
        )

        self.assertTrue(layer.commitChanges())

        res = [f.geometry().asWkt(3) for f in layer.getFeatures()]
        self.assertCountEqual(
            res,
            [
                "MultiPolygon (((0 2, 0 3, 1 3, 1 2, 0 2)))",
                "MultiPolygon (((0.5 1, 0.5 0, 0 0, 0 1, 0.5 1)))",
                "MultiPolygon (((0.5 0, 0.5 1, 1 1, 1 0, 0.5 0)))",
                "MultiPolygon (((2.5 1, 2.5 0, 2 0, 2 1, 2.5 1)))",
                "MultiPolygon (((2.5 2, 2.5 3, 3 3, 3 2, 2.5 2)))",
                "MultiPolygon (((2.5 3, 2.5 2, 2 2, 2 3, 2.5 3)))",
                "MultiPolygon (((2.5 0, 2.5 1, 3 1, 3 0, 2.5 0)))",
            ],
        )

    def test_SplitTruToCreateCutEdge(self):
        """
        Donut shaped polygon with interior ring, try to cut through donut.

        The polygon should not be modified
        """
        layer = QgsVectorLayer("Polygon?crs=EPSG:4326", "test layer", "memory")
        self.assertTrue(layer.isValid())
        self.assertTrue(layer.isSpatial())
        f = QgsFeature(layer.fields())
        f.setGeometry(
            QgsGeometry.fromWkt("POLYGON((0 0,3 0,3 3,0 3,0 0),(1 1,1 2,2 2,2 1,1 1))")
        )
        self.assertTrue(layer.dataProvider().addFeature(f))
        self.assertEqual(layer.featureCount(), 1)

        layer.startEditing()
        self.assertEqual(
            layer.splitFeatures([QgsPointXY(1.5, -0.5), QgsPointXY(1.5, 1.5)], 0),
            Qgis.GeometryOperationResult.NothingHappened,
        )

        self.assertTrue(layer.commitChanges())

        res = [f.geometry().asWkt(3) for f in layer.getFeatures()]
        self.assertCountEqual(
            res, ["Polygon ((0 0, 3 0, 3 3, 0 3, 0 0),(1 1, 1 2, 2 2, 2 1, 1 1))"]
        )


if __name__ == "__main__":
    unittest.main()
