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
                       QgsPoint,
                       QgsPointXY,
                       QgsVectorLayer,
                       QgsVectorLayerEditUtils)


from qgis.testing import start_app, unittest

start_app()


def createEmptyPolygonLayer():
    layer = QgsVectorLayer("Polygon",
                           "polygon", "memory")
    assert layer.isValid()
    return layer


class TestQgsVectorLayerEditUtils(unittest.TestCase):

    def testAddRing(self):
        # test adding ring to a vector layer
        layer = createEmptyPolygonLayer()
        self.assertTrue(layer.startEditing())

        pr = layer.dataProvider()
        f = QgsFeature(layer.fields(), 1)
        f.setGeometry(QgsGeometry.fromWkt('POLYGON((0 0, 5 0, 5 5, 0 5, 0 0))'))
        assert pr.addFeatures([f])
        self.assertEqual(layer.featureCount(), 1)

        vle = QgsVectorLayerEditUtils(layer)
        result = vle.addRing([QgsPointXY(1, 1), QgsPointXY(1, 2), QgsPointXY(2, 2), QgsPointXY(2, 1), QgsPointXY(1, 1)])
        self.assertEqual(Qgis.GeometryOperationResult.Success, result[0])
        layer.commitChanges()

        self.assertEqual(
            layer.getFeature(1).geometry().asWkt(),
            "Polygon ((0 0, 5 0, 5 5, 0 5, 0 0),(1 1, 1 2, 2 2, 2 1, 1 1))"
        )

    def testAddRingOutside(self):
        # test trying to add ring outside the feature's extent
        layer = createEmptyPolygonLayer()
        self.assertTrue(layer.startEditing())

        pr = layer.dataProvider()
        f = QgsFeature(layer.fields(), 1)
        f.setGeometry(QgsGeometry.fromWkt('POLYGON((0 0, 5 0, 5 5, 0 5, 0 0))'))
        assert pr.addFeatures([f])
        self.assertEqual(layer.featureCount(), 1)

        vle = QgsVectorLayerEditUtils(layer)
        result = vle.addRing([QgsPointXY(-1, -1), QgsPointXY(-1, -2), QgsPointXY(-2, -2), QgsPointXY(-2, -1), QgsPointXY(-1, -1)])
        self.assertEqual(Qgis.GeometryOperationResult.AddRingNotInExistingFeature, result[0])
        layer.commitChanges()

        self.assertEqual(
            layer.getFeature(1).geometry().asWkt(),
            "Polygon ((0 0, 5 0, 5 5, 0 5, 0 0))"
        )

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
        self.assertEqual(layer.featureCount(), 2)

        vle = QgsVectorLayerEditUtils(layer)
        result = vle.addRing([QgsPointXY(3, 3), QgsPointXY(3, 4), QgsPointXY(4, 4), QgsPointXY(4, 3), QgsPointXY(3, 3)])
        self.assertEqual(Qgis.GeometryOperationResult.Success,
                         result[0])
        layer.commitChanges()

        self.assertEqual(
            layer.getFeature(1).geometry().asWkt(),
            "Polygon ((0 0, 5 0, 5 5, 0 5, 0 0),(3 3, 3 4, 4 4, 4 3, 3 3))"
        )
        self.assertEqual(
            layer.getFeature(2).geometry().asWkt(),
            "Polygon ((2 2, 6 2, 6 6, 2 6, 2 2))"
        )

    def testAddRingV2NotEditable(self):
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
        self.assertEqual(layer.featureCount(), 2)
        layer.commitChanges()

        vle = QgsVectorLayerEditUtils(layer)
        result = vle.addRingV2(QgsLineString([QgsPoint(3, 3), QgsPoint(3, 4), QgsPoint(4, 4), QgsPoint(4, 3), QgsPoint(3, 3)]))
        self.assertEqual(Qgis.GeometryOperationResult.LayerNotEditable, result[0])
        self.assertEqual([], result[1])
        layer.commitChanges()

        self.assertEqual(
            layer.getFeature(1).geometry().asWkt(),
            "Polygon ((0 0, 5 0, 5 5, 0 5, 0 0))"
        )
        self.assertEqual(
            layer.getFeature(2).geometry().asWkt(),
            "Polygon ((2 2, 6 2, 6 6, 2 6, 2 2))"
        )

    def testAddRingV2NotClosedRing(self):
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
        self.assertEqual(layer.featureCount(), 2)

        vle = QgsVectorLayerEditUtils(layer)
        result = vle.addRingV2(QgsLineString([QgsPoint(3, 3), QgsPoint(3, 4), QgsPoint(4, 4), QgsPoint(4, 3)]))
        self.assertEqual(Qgis.GeometryOperationResult.AddRingNotClosed, result[0])
        self.assertEqual([], result[1])
        layer.commitChanges()

        self.assertEqual(
            layer.getFeature(1).geometry().asWkt(),
            "Polygon ((0 0, 5 0, 5 5, 0 5, 0 0))"
        )
        self.assertEqual(
            layer.getFeature(2).geometry().asWkt(),
            "Polygon ((2 2, 6 2, 6 6, 2 6, 2 2))"
        )

    def testAddRingV2Outside(self):
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
        self.assertEqual(layer.featureCount(), 2)

        vle = QgsVectorLayerEditUtils(layer)
        result = vle.addRingV2(QgsLineString([QgsPoint(8, 8), QgsPoint(8, 9), QgsPoint(9, 9), QgsPoint(9, 8), QgsPoint(8, 8)]))
        self.assertEqual(
            Qgis.GeometryOperationResult.AddRingNotInExistingFeature,
            result[0]
        )
        self.assertEqual([], result[1])
        layer.commitChanges()

        self.assertEqual(
            layer.getFeature(1).geometry().asWkt(),
            "Polygon ((0 0, 5 0, 5 5, 0 5, 0 0))"
        )
        self.assertEqual(
            layer.getFeature(2).geometry().asWkt(),
            "Polygon ((2 2, 6 2, 6 6, 2 6, 2 2))"
        )

    def testAddRingV2(self):
        # test adding ring on multi features
        layer = createEmptyPolygonLayer()
        self.assertTrue(layer.startEditing())

        pr = layer.dataProvider()
        f1 = QgsFeature(layer.fields(), 1)
        f1.setGeometry(QgsGeometry.fromWkt('POLYGON((0 0, 5 0, 5 5, 0 5, 0 0))'))
        f2 = QgsFeature(layer.fields(), 1)
        f2.setGeometry(QgsGeometry.fromWkt('POLYGON((2 2, 6 2, 6 6, 2 6, 2 2))'))
        assert pr.addFeatures([f1, f2])
        self.assertEqual(layer.featureCount(), 2)

        vle = QgsVectorLayerEditUtils(layer)
        result = vle.addRingV2(QgsLineString([QgsPoint(3, 3), QgsPoint(3, 4), QgsPoint(4, 4), QgsPoint(4, 3), QgsPoint(3, 3)]))
        self.assertEqual(Qgis.GeometryOperationResult.Success, result[0])
        self.assertEqual({2, 1}, set(result[1]))
        layer.commitChanges()

        self.assertEqual(
            layer.getFeature(1).geometry().asWkt(),
            "Polygon ((0 0, 5 0, 5 5, 0 5, 0 0),(3 3, 3 4, 4 4, 4 3, 3 3))"
        )
        self.assertEqual(
            layer.getFeature(2).geometry().asWkt(),
            "Polygon ((2 2, 6 2, 6 6, 2 6, 2 2),(3 3, 3 4, 4 4, 4 3, 3 3))"
        )

    def testAddRingV2SelectedFeatures(self):
        # test adding ring on multi features
        # test selected features
        layer = createEmptyPolygonLayer()
        self.assertTrue(layer.startEditing())

        pr = layer.dataProvider()
        f1 = QgsFeature(layer.fields(), 1)
        f1.setGeometry(QgsGeometry.fromWkt('POLYGON((0 0, 5 0, 5 5, 0 5, 0 0))'))
        f2 = QgsFeature(layer.fields(), 1)
        f2.setGeometry(QgsGeometry.fromWkt('POLYGON((2 2, 6 2, 6 6, 2 6, 2 2))'))
        assert pr.addFeatures([f1, f2])
        self.assertEqual(layer.featureCount(), 2)

        vle = QgsVectorLayerEditUtils(layer)
        result = vle.addRingV2(QgsLineString([QgsPoint(3, 3), QgsPoint(3, 4), QgsPoint(4, 4), QgsPoint(4, 3), QgsPoint(3, 3)]), [1])
        self.assertEqual(Qgis.GeometryOperationResult.Success, result[0])
        self.assertEqual([1], result[1])
        layer.commitChanges()

        self.assertEqual(
            layer.getFeature(1).geometry().asWkt(),
            "Polygon ((0 0, 5 0, 5 5, 0 5, 0 0),(3 3, 3 4, 4 4, 4 3, 3 3))"
        )
        self.assertEqual(
            layer.getFeature(2).geometry().asWkt(),
            "Polygon ((2 2, 6 2, 6 6, 2 6, 2 2))"
        )

    def testMoveVertex(self):
        layer = QgsVectorLayer("Point", "point", "memory")
        self.assertTrue(layer.startEditing())
        self.assertTrue(layer.isValid())

        pr = layer.dataProvider()
        f1 = QgsFeature(layer.fields(), 1)
        f1.setGeometry(QgsGeometry.fromWkt('Point(0 0)'))
        self.assertTrue(pr.addFeatures([f1]))
        self.assertEqual(layer.featureCount(), 1)

        vle = QgsVectorLayerEditUtils(layer)

        self.assertTrue(vle.moveVertex(1, 2, f1.id(), 0))
        f1 = layer.getFeature(1)

        self.assertEqual(f1.geometry().constGet().x(), 1)
        self.assertEqual(f1.geometry().constGet().y(), 2)

        self.assertTrue(vle.moveVertexV2(QgsPoint(3, 4, 5, 6), f1.id(), 0))
        f1 = layer.getFeature(1)

        self.assertEqual(f1.geometry().constGet().x(), 3)
        self.assertEqual(f1.geometry().constGet().y(), 4)
        self.assertFalse(f1.geometry().constGet().is3D())
        self.assertFalse(f1.geometry().constGet().isMeasure())

    def testMoveVertexPointZ(self):
        layer = QgsVectorLayer("PointZ", "pointZ", "memory")
        self.assertTrue(layer.startEditing())
        self.assertTrue(layer.isValid())

        pr = layer.dataProvider()
        f1 = QgsFeature(layer.fields(), 1)
        f1.setGeometry(QgsGeometry.fromWkt('PointZ(0 0 0)'))
        self.assertTrue(pr.addFeatures([f1]))
        self.assertEqual(layer.featureCount(), 1)

        vle = QgsVectorLayerEditUtils(layer)

        self.assertTrue(vle.moveVertex(1, 2, f1.id(), 0))
        f1 = layer.getFeature(1)

        self.assertEqual(f1.geometry().constGet().x(), 1)
        self.assertEqual(f1.geometry().constGet().y(), 2)
        self.assertEqual(f1.geometry().constGet().z(), 0)

        self.assertTrue(vle.moveVertexV2(QgsPoint(3, 4, 5, 6), f1.id(), 0))
        f1 = layer.getFeature(1)

        self.assertEqual(f1.geometry().constGet().x(), 3)
        self.assertEqual(f1.geometry().constGet().y(), 4)
        self.assertEqual(f1.geometry().constGet().z(), 5)
        self.assertTrue(f1.geometry().constGet().is3D())
        self.assertFalse(f1.geometry().constGet().isMeasure())

        # Add a non-Z point and check that Z get added on move
        f2 = QgsFeature(layer.fields(), 2)
        f2.setGeometry(QgsGeometry.fromWkt('Point(0 0)'))
        self.assertTrue(pr.addFeatures([f2]))
        self.assertEqual(layer.featureCount(), 2)

        self.assertFalse(f2.geometry().constGet().is3D())
        self.assertTrue(vle.moveVertexV2(QgsPoint(3, 4, 5, 6), f2.id(), 0))
        f2 = layer.getFeature(2)
        self.assertEqual(f2.geometry().constGet().x(), 3)
        self.assertEqual(f2.geometry().constGet().y(), 4)
        self.assertEqual(f2.geometry().constGet().z(), 5)
        self.assertTrue(f2.geometry().constGet().is3D())
        self.assertFalse(f2.geometry().constGet().isMeasure())

    def testMoveVertexPointM(self):
        layer = QgsVectorLayer("PointM", "pointM", "memory")
        self.assertTrue(layer.startEditing())
        self.assertTrue(layer.isValid())

        pr = layer.dataProvider()
        f1 = QgsFeature(layer.fields(), 1)
        f1.setGeometry(QgsGeometry.fromWkt('PointM(0 0 0)'))
        self.assertTrue(pr.addFeatures([f1]))
        self.assertEqual(layer.featureCount(), 1)

        vle = QgsVectorLayerEditUtils(layer)

        self.assertTrue(vle.moveVertexV2(QgsPoint(3, 4, 5, 6), f1.id(), 0))
        f1 = layer.getFeature(1)

        self.assertEqual(f1.geometry().constGet().x(), 3)
        self.assertEqual(f1.geometry().constGet().y(), 4)
        self.assertEqual(f1.geometry().constGet().m(), 6)
        self.assertFalse(f1.geometry().constGet().is3D())
        self.assertTrue(f1.geometry().constGet().isMeasure())

        # Add a non-M point and check that M get added on move
        f2 = QgsFeature(layer.fields(), 2)
        f2.setGeometry(QgsGeometry.fromWkt('Point(0 0)'))
        self.assertTrue(pr.addFeatures([f2]))
        self.assertEqual(layer.featureCount(), 2)

        self.assertFalse(f2.geometry().constGet().isMeasure())

        self.assertTrue(vle.moveVertexV2(QgsPoint(3, 4, 5, 6), f2.id(), 0))
        f2 = layer.getFeature(2)

        self.assertTrue(f2.geometry().constGet().isMeasure())
        self.assertFalse(f2.geometry().constGet().is3D())
        self.assertEqual(f2.geometry().constGet().x(), 3)
        self.assertEqual(f2.geometry().constGet().y(), 4)
        self.assertEqual(f2.geometry().constGet().m(), 6)

    def testMoveVertexPointZM(self):
        layer = QgsVectorLayer("PointZM", "pointZM", "memory")
        self.assertTrue(layer.startEditing())
        self.assertTrue(layer.isValid())

        pr = layer.dataProvider()
        f1 = QgsFeature(layer.fields(), 1)
        f1.setGeometry(QgsGeometry.fromWkt('PointZM(0 0 0 0)'))
        self.assertTrue(pr.addFeatures([f1]))
        self.assertEqual(layer.featureCount(), 1)

        vle = QgsVectorLayerEditUtils(layer)

        self.assertTrue(vle.moveVertexV2(QgsPoint(3, 4, 5, 6), f1.id(), 0))
        f1 = layer.getFeature(1)

        self.assertEqual(f1.geometry().constGet().x(), 3)
        self.assertEqual(f1.geometry().constGet().y(), 4)
        self.assertEqual(f1.geometry().constGet().z(), 5)
        self.assertEqual(f1.geometry().constGet().m(), 6)
        self.assertTrue(f1.geometry().constGet().is3D())
        self.assertTrue(f1.geometry().constGet().isMeasure())

        # Add a non-ZM point and check that Z and M get added on move
        f2 = QgsFeature(layer.fields(), 2)
        f2.setGeometry(QgsGeometry.fromWkt('Point(0 0)'))
        self.assertTrue(pr.addFeatures([f2]))
        self.assertEqual(layer.featureCount(), 2)

        self.assertFalse(f2.geometry().constGet().isMeasure())
        self.assertFalse(f2.geometry().constGet().is3D())

        self.assertTrue(vle.moveVertexV2(QgsPoint(3, 4, 5, 6), f2.id(), 0))
        f2 = layer.getFeature(2)
        self.assertTrue(f2.geometry().constGet().isMeasure())
        self.assertTrue(f2.geometry().constGet().is3D())

        self.assertEqual(f2.geometry().constGet().x(), 3)
        self.assertEqual(f2.geometry().constGet().y(), 4)
        self.assertEqual(f2.geometry().constGet().z(), 5)
        self.assertEqual(f2.geometry().constGet().m(), 6)


if __name__ == '__main__':
    unittest.main()
