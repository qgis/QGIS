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

import tempfile
import os

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
                       QgsField,
                       QgsFields,
                       QgsGeometry,
                       QgsLineString,
                       QgsPoint,
                       QgsPointXY,
                       QgsCoordinateReferenceSystem,
                       QgsCoordinateTransformContext,
                       QgsVectorLayer,
                       QgsVectorLayerEditUtils,
                       QgsVectorFileWriter,
                       QgsWkbTypes)


from qgis.testing import start_app, unittest

start_app()


def createEmptyLayer(geomType):
    layer = QgsVectorLayer(geomType,
                           geomType.lower(), "memory")
    assert layer.isValid()
    return layer


def createEmptyPolygonLayer():
    return createEmptyLayer("Polygon")


def createEmptyMultiPolygonLayer():
    return createEmptyLayer("MultiPolygon")


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

    def testSplitParts(self):
        layer = createEmptyMultiPolygonLayer()
        self.assertTrue(layer.startEditing())

        pr = layer.dataProvider()

        # Add three MultiPolygon features
        # Each feature is composed of two squares side by side
        # Each feature is on a separate row to form a 3*2 grid
        f = QgsFeature(layer.fields(), 1)
        f.setGeometry(QgsGeometry.fromWkt('MULTIPOLYGON(((0 0, 4 0, 4 4, 0 4, 0 0)), ((6 0, 10 0, 10 4, 6 4, 6 0)))'))
        assert pr.addFeatures([f])

        f = QgsFeature(layer.fields(), 2)
        f.setGeometry(QgsGeometry.fromWkt('MULTIPOLYGON(((0 6, 4 6, 4 10, 0 10, 0 6)), ((6 6, 10 6, 10 10, 6 10, 6 6)))'))
        assert pr.addFeatures([f])

        f = QgsFeature(layer.fields(), 3)
        f.setGeometry(QgsGeometry.fromWkt('MULTIPOLYGON(((0 12, 4 12, 4 16, 0 16, 0 12)), ((6 12, 10 12, 10 16, 6 16, 6 12)))'))
        assert pr.addFeatures([f])

        self.assertEqual(layer.featureCount(), 3)

        vle = QgsVectorLayerEditUtils(layer)

        # Split the first feature with a horizontal line that crosses both its parts
        # After this operation, the first feature has 4 parts, the other two are unchanged
        result = vle.splitParts([QgsPointXY(0, 2), QgsPointXY(10, 2)], False)
        self.assertEqual(result, Qgis.GeometryOperationResult.Success)

        # Split all three features with a vertical Line
        # After this operation, the first feature has 6 parts, the other two have 6 parts
        result = vle.splitParts([QgsPointXY(2, 0), QgsPointXY(2, 16)], False)
        self.assertEqual(result, Qgis.GeometryOperationResult.Success)

        layer.commitChanges()

        self.assertEqual(
            layer.getFeature(1).geometry().asWkt(),
            'MultiPolygon (((2 0, 2 2, 4 2, 4 0, 2 0)),((2 2, 2 0, 0 0, 0 2, 2 2)),((2 2, 2 4, 4 4, 4 2, 2 2)),((2 4, 2 2, 0 2, 0 4, 2 4)),((6 2, 10 2, 10 0, 6 0, 6 2)),((10 2, 6 2, 6 4, 10 4, 10 2)))'
        )
        self.assertEqual(
            layer.getFeature(2).geometry().asWkt(),
            'MultiPolygon (((2 6, 2 10, 4 10, 4 6, 2 6)),((2 10, 2 6, 0 6, 0 10, 2 10)),((6 6, 10 6, 10 10, 6 10, 6 6)))'
        )

        self.assertEqual(
            layer.getFeature(3).geometry().asWkt(),
            'MultiPolygon (((2 12, 2 16, 4 16, 4 12, 2 12)),((2 16, 2 12, 0 12, 0 16, 2 16)),((6 12, 10 12, 10 16, 6 16, 6 12)))'
        )

    def testMergeFeatures(self):
        layer = createEmptyPolygonLayer()
        self.assertTrue(layer.startEditing())
        layer.addAttribute(QgsField('name', QVariant.String))
        pr = layer.dataProvider()
        f1 = QgsFeature(layer.fields(), 1)
        f1.setGeometry(QgsGeometry.fromWkt('POLYGON((0 0, 5 0, 5 5, 0 5, 0 0))'))
        f1.setAttribute('name', 'uno')
        assert pr.addFeatures([f1])
        self.assertEqual(layer.featureCount(), 1)

        pr = layer.dataProvider()
        f2 = QgsFeature(layer.fields(), 2)
        f2.setGeometry(QgsGeometry.fromWkt('POLYGON((3 3, 8 3, 8 8, 3 8, 3 3))'))
        f2.setAttribute('name', 'due')
        assert pr.addFeatures([f2])
        self.assertEqual(layer.featureCount(), 2)

        featureIds = [f1.id(), f2.id()]

        unionGeom = f1.geometry()
        unionGeom = unionGeom.combine(f2.geometry())
        self.assertFalse(unionGeom.isNull())

        vle = QgsVectorLayerEditUtils(layer)
        success, errorMessage = vle.mergeFeatures(featureIds, ['tre'], unionGeom)
        self.assertFalse(errorMessage)
        self.assertTrue(success)

        layer.commitChanges()

        self.assertEqual(layer.featureCount(), 1)
        mergedFeature = next(layer.getFeatures())
        self.assertEqual(
            mergedFeature.geometry().asWkt(),
            "Polygon ((5 0, 0 0, 0 5, 3 5, 3 8, 8 8, 8 3, 5 3, 5 0))"
        )
        self.assertEqual(mergedFeature.attribute('name'), 'tre')

    def testMergeFeaturesIntoExisting(self):
        tempgpkg = os.path.join(tempfile.mkdtemp(), 'testMergeFeaturesIntoExisting.gpkg')
        fields = QgsFields()
        fields.append(QgsField('name', QVariant.String))

        crs = QgsCoordinateReferenceSystem('epsg:4326')
        options = QgsVectorFileWriter.SaveVectorOptions()
        options.driverName = "GPKG"
        QgsVectorFileWriter.create(
            fileName=tempgpkg,
            fields=fields,
            geometryType=QgsWkbTypes.Polygon,
            srs=crs,
            transformContext=QgsCoordinateTransformContext(),
            options=options)

        layer = QgsVectorLayer(tempgpkg, 'my_layer', 'ogr')

        self.assertTrue(layer.startEditing())
        pr = layer.dataProvider()
        f1 = QgsFeature(layer.fields(), 1)
        f1.setGeometry(QgsGeometry.fromWkt('POLYGON((0 0, 5 0, 5 5, 0 5, 0 0))'))
        f1.setAttribute('name', 'uno')
        assert pr.addFeatures([f1])
        self.assertEqual(layer.featureCount(), 1)

        pr = layer.dataProvider()
        f2 = QgsFeature(layer.fields(), 2)
        f2.setGeometry(QgsGeometry.fromWkt('POLYGON((3 3, 8 3, 8 8, 3 8, 3 3))'))
        f2.setAttribute('name', 'due')
        assert pr.addFeatures([f2])
        self.assertEqual(layer.featureCount(), 2)

        featureIds = [f1.id(), f2.id()]

        unionGeom = f1.geometry()
        unionGeom = unionGeom.combine(f2.geometry())
        self.assertFalse(unionGeom.isNull())

        vle = QgsVectorLayerEditUtils(layer)
        success, errorMessage = vle.mergeFeatures(featureIds, [2, 'tre'], unionGeom)
        self.assertFalse(errorMessage)
        self.assertTrue(success)

        layer.commitChanges()

        self.assertEqual(layer.featureCount(), 1)
        mergedFeature = layer.getFeature(2)
        self.assertEqual(
            mergedFeature.geometry().asWkt(),
            "Polygon ((5 0, 0 0, 0 5, 3 5, 3 8, 8 8, 8 3, 5 3, 5 0))"
        )
        self.assertEqual(mergedFeature.attribute('name'), 'tre')


if __name__ == '__main__':
    unittest.main()
