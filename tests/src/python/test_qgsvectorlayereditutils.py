# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsVectorLayerEditUtils.

From build dir, run:
ctest -R PyQgsVectorLayerEditUtils -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Loïc Bartoletti'
__date__ = '18/10/2022'
__copyright__ = 'Copyright 2022, The QGIS Project'

import qgis  # NOQA

from qgis.PyQt.QtCore import (
    QDate,
    QDateTime,
    QVariant,
    Qt,
    QDateTime,
    QDate,
    QTime,
    QTimer,
    QTemporaryDir,
)

from qgis.core import (Qgis,
                       QgsFeature,
                       QgsGeometry,
                       QgsLineString,
                       QgsPolygon,
                       QgsPoint,
                       QgsPointXY,
                       QgsVectorLayer,
                       QgsVectorLayerTools,
                       QgsVectorLayerEditUtils)


from qgis.testing import start_app, unittest

start_app()


def createEmptyPolygonLayer():
    layer = QgsVectorLayer("Polygon",
                           "polygon", "memory")
    assert layer.isValid()
    return layer


class TestQgsVectorLayerEditBuffer(unittest.TestCase):

    def testAddRing(self):
        # test adding ring to a vector layer
        layer = createEmptyPolygonLayer()
        self.assertTrue(layer.startEditing())

        pr = layer.dataProvider()
        f = QgsFeature(layer.fields(), 1)
        f.setGeometry(QgsGeometry.fromWkt('POLYGON((0 0, 5 0, 5 5, 0 5, 0 0))'))
        assert pr.addFeatures([f])
        assert layer.featureCount() == 1

        vle = QgsVectorLayerEditUtils(layer)
        result = vle.addRing([QgsPointXY(1, 1), QgsPointXY(1, 2), QgsPointXY(2, 2), QgsPointXY(2, 1), QgsPointXY(1, 1)])
        assert Qgis.GeometryOperationResult.Success == result[0]
        layer.commitChanges()

        assert layer.getFeature(1).geometry().asWkt() == "Polygon ((0 0, 5 0, 5 5, 0 5, 0 0),(1 1, 1 2, 2 2, 2 1, 1 1))"

    def testAddRingOutside(self):
        # test trying to add ring outside the feature's extent
        layer = createEmptyPolygonLayer()
        self.assertTrue(layer.startEditing())

        pr = layer.dataProvider()
        f = QgsFeature(layer.fields(), 1)
        f.setGeometry(QgsGeometry.fromWkt('POLYGON((0 0, 5 0, 5 5, 0 5, 0 0))'))
        assert pr.addFeatures([f])
        assert layer.featureCount() == 1

        vle = QgsVectorLayerEditUtils(layer)
        result = vle.addRing([QgsPointXY(-1, -1), QgsPointXY(-1, -2), QgsPointXY(-2, -2), QgsPointXY(-2, -1), QgsPointXY(-1, -1)])
        assert Qgis.GeometryOperationResult.AddRingNotInExistingFeature == result[0]
        layer.commitChanges()

        assert layer.getFeature(1).geometry().asWkt() == "Polygon ((0 0, 5 0, 5 5, 0 5, 0 0))"

    def testAddRingOverlappedFeatures(self):
        # test adding ring on multi features
        # the ring will be added only to the first one
        layer = createEmptyPolygonLayer()
        self.assertTrue(layer.startEditing())

        pr = layer.dataProvider()
        f1 = QgsFeature(layer.fields(), 1)
        f1.setGeometry(QgsGeometry.fromWkt('POLYGON((0 0, 5 0, 5 5, 0 5, 0 0))'))
        f2 = QgsFeature(layer.fields(), 1)
        f2.setGeometry(QgsGeometry.fromWkt('POLYGON((2 2, 6 2, 6 6, 2 6, 2 2))'))
        assert pr.addFeatures([f1, f2])
        assert layer.featureCount() == 2

        vle = QgsVectorLayerEditUtils(layer)
        result = vle.addRing([QgsPointXY(3, 3), QgsPointXY(3, 4), QgsPointXY(4, 4), QgsPointXY(4, 3), QgsPointXY(3, 3)])
        assert Qgis.GeometryOperationResult.Success == result[0]
        layer.commitChanges()

        assert layer.getFeature(1).geometry().asWkt() == "Polygon ((0 0, 5 0, 5 5, 0 5, 0 0),(3 3, 3 4, 4 4, 4 3, 3 3))"
        assert layer.getFeature(2).geometry().asWkt() == "Polygon ((2 2, 6 2, 6 6, 2 6, 2 2))"

    def testAddMultiRingNotEditable(self):
        # test adding ring on multi features
        # layer not editable
        layer = createEmptyPolygonLayer()
        self.assertTrue(layer.startEditing())

        pr = layer.dataProvider()
        f1 = QgsFeature(layer.fields(), 1)
        f1.setGeometry(QgsGeometry.fromWkt('POLYGON((0 0, 5 0, 5 5, 0 5, 0 0))'))
        f2 = QgsFeature(layer.fields(), 1)
        f2.setGeometry(QgsGeometry.fromWkt('POLYGON((2 2, 6 2, 6 6, 2 6, 2 2))'))
        assert pr.addFeatures([f1, f2])
        assert layer.featureCount() == 2
        layer.commitChanges()

        vle = QgsVectorLayerEditUtils(layer)
        assert Qgis.GeometryOperationResult.LayerNotEditable == vle.addRingV2(QgsLineString([QgsPoint(3, 3), QgsPoint(3, 4), QgsPoint(4, 4), QgsPoint(4, 3), QgsPoint(3, 3)]))
        layer.commitChanges()

        assert layer.getFeature(1).geometry().asWkt() == "Polygon ((0 0, 5 0, 5 5, 0 5, 0 0))"
        assert layer.getFeature(2).geometry().asWkt() == "Polygon ((2 2, 6 2, 6 6, 2 6, 2 2))"

    def testAddMultiRingNotClosedRing(self):
        # test adding ring on multi features
        # Not closed ring
        layer = createEmptyPolygonLayer()
        self.assertTrue(layer.startEditing())

        pr = layer.dataProvider()
        f1 = QgsFeature(layer.fields(), 1)
        f1.setGeometry(QgsGeometry.fromWkt('POLYGON((0 0, 5 0, 5 5, 0 5, 0 0))'))
        f2 = QgsFeature(layer.fields(), 1)
        f2.setGeometry(QgsGeometry.fromWkt('POLYGON((2 2, 6 2, 6 6, 2 6, 2 2))'))
        assert pr.addFeatures([f1, f2])
        assert layer.featureCount() == 2

        vle = QgsVectorLayerEditUtils(layer)
        assert Qgis.GeometryOperationResult.AddRingNotClosed == vle.addRingV2(QgsLineString([QgsPoint(3, 3), QgsPoint(3, 4), QgsPoint(4, 4), QgsPoint(4, 3)]))
        layer.commitChanges()

        assert layer.getFeature(1).geometry().asWkt() == "Polygon ((0 0, 5 0, 5 5, 0 5, 0 0))"
        assert layer.getFeature(2).geometry().asWkt() == "Polygon ((2 2, 6 2, 6 6, 2 6, 2 2))"

    def testAddMultiRingOutside(self):
        # test adding ring on multi features
        # Not In Existing Feature
        layer = createEmptyPolygonLayer()
        self.assertTrue(layer.startEditing())

        pr = layer.dataProvider()
        f1 = QgsFeature(layer.fields(), 1)
        f1.setGeometry(QgsGeometry.fromWkt('POLYGON((0 0, 5 0, 5 5, 0 5, 0 0))'))
        f2 = QgsFeature(layer.fields(), 1)
        f2.setGeometry(QgsGeometry.fromWkt('POLYGON((2 2, 6 2, 6 6, 2 6, 2 2))'))
        assert pr.addFeatures([f1, f2])
        assert layer.featureCount() == 2

        vle = QgsVectorLayerEditUtils(layer)
        assert Qgis.GeometryOperationResult.AddRingNotInExistingFeature == vle.addRingV2(QgsLineString([QgsPoint(8, 8), QgsPoint(8, 9), QgsPoint(9, 9), QgsPoint(9, 8), QgsPoint(8, 8)]))
        layer.commitChanges()

        assert layer.getFeature(1).geometry().asWkt() == "Polygon ((0 0, 5 0, 5 5, 0 5, 0 0))"
        assert layer.getFeature(2).geometry().asWkt() == "Polygon ((2 2, 6 2, 6 6, 2 6, 2 2))"

    def testAddMultiRing(self):
        # test adding ring on multi features
        layer = createEmptyPolygonLayer()
        self.assertTrue(layer.startEditing())

        pr = layer.dataProvider()
        f1 = QgsFeature(layer.fields(), 1)
        f1.setGeometry(QgsGeometry.fromWkt('POLYGON((0 0, 5 0, 5 5, 0 5, 0 0))'))
        f2 = QgsFeature(layer.fields(), 1)
        f2.setGeometry(QgsGeometry.fromWkt('POLYGON((2 2, 6 2, 6 6, 2 6, 2 2))'))
        assert pr.addFeatures([f1, f2])
        assert layer.featureCount() == 2

        vle = QgsVectorLayerEditUtils(layer)
        assert Qgis.GeometryOperationResult.Success == vle.addRingV2(QgsLineString([QgsPoint(3, 3), QgsPoint(3, 4), QgsPoint(4, 4), QgsPoint(4, 3), QgsPoint(3, 3)]))
        layer.commitChanges()

        assert layer.getFeature(1).geometry().asWkt() == "Polygon ((0 0, 5 0, 5 5, 0 5, 0 0),(3 3, 3 4, 4 4, 4 3, 3 3))"
        assert layer.getFeature(2).geometry().asWkt() == "Polygon ((2 2, 6 2, 6 6, 2 6, 2 2),(3 3, 3 4, 4 4, 4 3, 3 3))"


if __name__ == '__main__':
    unittest.main()
