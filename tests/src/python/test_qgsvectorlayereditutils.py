"""QGIS Unit tests for QgsVectorLayerEditUtils.

From build dir, run:
ctest -R PyQgsVectorLayerEditUtils -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Loïc Bartoletti"
__date__ = "18/10/2022"
__copyright__ = "Copyright 2022, The QGIS Project"

import os
import tempfile
import unittest

from qgis.core import (
    Qgis,
    QgsCompoundCurve,
    QgsCoordinateReferenceSystem,
    QgsCoordinateTransformContext,
    QgsDefaultValue,
    QgsFeature,
    QgsField,
    QgsFields,
    QgsGeometry,
    QgsLineString,
    QgsPoint,
    QgsPointXY,
    QgsUnsetAttributeValue,
    QgsVectorFileWriter,
    QgsVectorLayer,
    QgsVectorLayerEditUtils,
    QgsVectorLayerUtils,
    QgsWkbTypes,
)
from qgis.PyQt.QtCore import (
    QVariant,
)
from qgis.testing import QgisTestCase, start_app

start_app()


def createEmptyLayer(geomType):
    layer = QgsVectorLayer(geomType, geomType.lower(), "memory")
    assert layer.isValid()
    return layer


def createEmptyPolygonLayer():
    return createEmptyLayer("Polygon")


def createEmptyMultiPolygonLayer():
    return createEmptyLayer("MultiPolygon")


def createEmptyCurvePolygonLayer():
    return createEmptyLayer("CurvePolygon")


class TestQgsVectorLayerEditUtils(QgisTestCase):
    def testAddRing(self):
        # test adding ring to a vector layer
        layer = createEmptyPolygonLayer()
        self.assertTrue(layer.startEditing())

        pr = layer.dataProvider()
        f = QgsFeature(layer.fields(), 1)
        f.setGeometry(QgsGeometry.fromWkt("POLYGON((0 0, 5 0, 5 5, 0 5, 0 0))"))
        assert pr.addFeatures([f])
        self.assertEqual(layer.featureCount(), 1)

        vle = QgsVectorLayerEditUtils(layer)
        result = vle.addRing(
            [
                QgsPointXY(1, 1),
                QgsPointXY(1, 2),
                QgsPointXY(2, 2),
                QgsPointXY(2, 1),
                QgsPointXY(1, 1),
            ]
        )
        self.assertEqual(Qgis.GeometryOperationResult.Success, result[0])
        layer.commitChanges()

        self.assertEqual(
            layer.getFeature(1).geometry().asWkt(),
            "Polygon ((0 0, 5 0, 5 5, 0 5, 0 0),(1 1, 1 2, 2 2, 2 1, 1 1))",
        )

    def testAddRingOutside(self):
        # test trying to add ring outside the feature's extent
        layer = createEmptyPolygonLayer()
        self.assertTrue(layer.startEditing())

        pr = layer.dataProvider()
        f = QgsFeature(layer.fields(), 1)
        f.setGeometry(QgsGeometry.fromWkt("POLYGON((0 0, 5 0, 5 5, 0 5, 0 0))"))
        assert pr.addFeatures([f])
        self.assertEqual(layer.featureCount(), 1)

        vle = QgsVectorLayerEditUtils(layer)
        result = vle.addRing(
            [
                QgsPointXY(-1, -1),
                QgsPointXY(-1, -2),
                QgsPointXY(-2, -2),
                QgsPointXY(-2, -1),
                QgsPointXY(-1, -1),
            ]
        )
        self.assertEqual(
            Qgis.GeometryOperationResult.AddRingNotInExistingFeature, result[0]
        )
        layer.commitChanges()

        self.assertEqual(
            layer.getFeature(1).geometry().asWkt(),
            "Polygon ((0 0, 5 0, 5 5, 0 5, 0 0))",
        )

    def testAddRingOverlappedFeatures(self):
        # test adding ring on multi features
        # the ring will be added only to the first one
        layer = createEmptyPolygonLayer()
        self.assertTrue(layer.startEditing())

        pr = layer.dataProvider()
        f1 = QgsFeature(layer.fields(), 1)
        f1.setGeometry(QgsGeometry.fromWkt("POLYGON((0 0, 5 0, 5 5, 0 5, 0 0))"))
        f2 = QgsFeature(layer.fields(), 1)
        f2.setGeometry(QgsGeometry.fromWkt("POLYGON((2 2, 6 2, 6 6, 2 6, 2 2))"))
        assert pr.addFeatures([f1, f2])
        self.assertEqual(layer.featureCount(), 2)

        vle = QgsVectorLayerEditUtils(layer)
        result = vle.addRing(
            [
                QgsPointXY(3, 3),
                QgsPointXY(3, 4),
                QgsPointXY(4, 4),
                QgsPointXY(4, 3),
                QgsPointXY(3, 3),
            ]
        )
        self.assertEqual(Qgis.GeometryOperationResult.Success, result[0])
        layer.commitChanges()

        self.assertEqual(
            layer.getFeature(1).geometry().asWkt(),
            "Polygon ((0 0, 5 0, 5 5, 0 5, 0 0),(3 3, 3 4, 4 4, 4 3, 3 3))",
        )
        self.assertEqual(
            layer.getFeature(2).geometry().asWkt(),
            "Polygon ((2 2, 6 2, 6 6, 2 6, 2 2))",
        )

    def testAddRingV2NotEditable(self):
        # test adding ring on multi features
        # layer not editable
        layer = createEmptyPolygonLayer()
        self.assertTrue(layer.startEditing())

        pr = layer.dataProvider()
        f1 = QgsFeature(layer.fields(), 1)
        f1.setGeometry(QgsGeometry.fromWkt("POLYGON((0 0, 5 0, 5 5, 0 5, 0 0))"))
        f2 = QgsFeature(layer.fields(), 1)
        f2.setGeometry(QgsGeometry.fromWkt("POLYGON((2 2, 6 2, 6 6, 2 6, 2 2))"))
        assert pr.addFeatures([f1, f2])
        self.assertEqual(layer.featureCount(), 2)
        layer.commitChanges()

        vle = QgsVectorLayerEditUtils(layer)
        result = vle.addRingV2(
            QgsLineString(
                [
                    QgsPoint(3, 3),
                    QgsPoint(3, 4),
                    QgsPoint(4, 4),
                    QgsPoint(4, 3),
                    QgsPoint(3, 3),
                ]
            )
        )
        self.assertEqual(Qgis.GeometryOperationResult.LayerNotEditable, result[0])
        self.assertEqual([], result[1])
        layer.commitChanges()

        self.assertEqual(
            layer.getFeature(1).geometry().asWkt(),
            "Polygon ((0 0, 5 0, 5 5, 0 5, 0 0))",
        )
        self.assertEqual(
            layer.getFeature(2).geometry().asWkt(),
            "Polygon ((2 2, 6 2, 6 6, 2 6, 2 2))",
        )

    def testAddRingV2NotClosedRing(self):
        # test adding ring on multi features
        # Not closed ring
        layer = createEmptyPolygonLayer()
        self.assertTrue(layer.startEditing())

        pr = layer.dataProvider()
        f1 = QgsFeature(layer.fields(), 1)
        f1.setGeometry(QgsGeometry.fromWkt("POLYGON((0 0, 5 0, 5 5, 0 5, 0 0))"))
        f2 = QgsFeature(layer.fields(), 1)
        f2.setGeometry(QgsGeometry.fromWkt("POLYGON((2 2, 6 2, 6 6, 2 6, 2 2))"))
        assert pr.addFeatures([f1, f2])
        self.assertEqual(layer.featureCount(), 2)

        vle = QgsVectorLayerEditUtils(layer)
        result = vle.addRingV2(
            QgsLineString(
                [QgsPoint(3, 3), QgsPoint(3, 4), QgsPoint(4, 4), QgsPoint(4, 3)]
            )
        )
        self.assertEqual(Qgis.GeometryOperationResult.AddRingNotClosed, result[0])
        self.assertEqual([], result[1])
        layer.commitChanges()

        self.assertEqual(
            layer.getFeature(1).geometry().asWkt(),
            "Polygon ((0 0, 5 0, 5 5, 0 5, 0 0))",
        )
        self.assertEqual(
            layer.getFeature(2).geometry().asWkt(),
            "Polygon ((2 2, 6 2, 6 6, 2 6, 2 2))",
        )

    def testAddRingV2Outside(self):
        # test adding ring on multi features
        # Not In Existing Feature
        layer = createEmptyPolygonLayer()
        self.assertTrue(layer.startEditing())

        pr = layer.dataProvider()
        f1 = QgsFeature(layer.fields(), 1)
        f1.setGeometry(QgsGeometry.fromWkt("POLYGON((0 0, 5 0, 5 5, 0 5, 0 0))"))
        f2 = QgsFeature(layer.fields(), 1)
        f2.setGeometry(QgsGeometry.fromWkt("POLYGON((2 2, 6 2, 6 6, 2 6, 2 2))"))
        assert pr.addFeatures([f1, f2])
        self.assertEqual(layer.featureCount(), 2)

        vle = QgsVectorLayerEditUtils(layer)
        result = vle.addRingV2(
            QgsLineString(
                [
                    QgsPoint(8, 8),
                    QgsPoint(8, 9),
                    QgsPoint(9, 9),
                    QgsPoint(9, 8),
                    QgsPoint(8, 8),
                ]
            )
        )
        self.assertEqual(
            Qgis.GeometryOperationResult.AddRingNotInExistingFeature, result[0]
        )
        self.assertEqual([], result[1])
        layer.commitChanges()

        self.assertEqual(
            layer.getFeature(1).geometry().asWkt(),
            "Polygon ((0 0, 5 0, 5 5, 0 5, 0 0))",
        )
        self.assertEqual(
            layer.getFeature(2).geometry().asWkt(),
            "Polygon ((2 2, 6 2, 6 6, 2 6, 2 2))",
        )

    def testAddRingV2(self):
        # test adding ring on multi features
        layer = createEmptyPolygonLayer()
        self.assertTrue(layer.startEditing())

        pr = layer.dataProvider()
        f1 = QgsFeature(layer.fields(), 1)
        f1.setGeometry(QgsGeometry.fromWkt("POLYGON((0 0, 5 0, 5 5, 0 5, 0 0))"))
        f2 = QgsFeature(layer.fields(), 1)
        f2.setGeometry(QgsGeometry.fromWkt("POLYGON((2 2, 6 2, 6 6, 2 6, 2 2))"))
        assert pr.addFeatures([f1, f2])
        self.assertEqual(layer.featureCount(), 2)

        vle = QgsVectorLayerEditUtils(layer)
        result = vle.addRingV2(
            QgsLineString(
                [
                    QgsPoint(3, 3),
                    QgsPoint(3, 4),
                    QgsPoint(4, 4),
                    QgsPoint(4, 3),
                    QgsPoint(3, 3),
                ]
            )
        )
        self.assertEqual(Qgis.GeometryOperationResult.Success, result[0])
        self.assertEqual({2, 1}, set(result[1]))
        layer.commitChanges()

        self.assertEqual(
            layer.getFeature(1).geometry().asWkt(),
            "Polygon ((0 0, 5 0, 5 5, 0 5, 0 0),(3 3, 3 4, 4 4, 4 3, 3 3))",
        )
        self.assertEqual(
            layer.getFeature(2).geometry().asWkt(),
            "Polygon ((2 2, 6 2, 6 6, 2 6, 2 2),(3 3, 3 4, 4 4, 4 3, 3 3))",
        )

    def testAddRingV2SelectedFeatures(self):
        # test adding ring on multi features
        # test selected features
        layer = createEmptyPolygonLayer()
        self.assertTrue(layer.startEditing())

        pr = layer.dataProvider()
        f1 = QgsFeature(layer.fields(), 1)
        f1.setGeometry(QgsGeometry.fromWkt("POLYGON((0 0, 5 0, 5 5, 0 5, 0 0))"))
        f2 = QgsFeature(layer.fields(), 1)
        f2.setGeometry(QgsGeometry.fromWkt("POLYGON((2 2, 6 2, 6 6, 2 6, 2 2))"))
        assert pr.addFeatures([f1, f2])
        self.assertEqual(layer.featureCount(), 2)

        vle = QgsVectorLayerEditUtils(layer)
        result = vle.addRingV2(
            QgsLineString(
                [
                    QgsPoint(3, 3),
                    QgsPoint(3, 4),
                    QgsPoint(4, 4),
                    QgsPoint(4, 3),
                    QgsPoint(3, 3),
                ]
            ),
            [1],
        )
        self.assertEqual(Qgis.GeometryOperationResult.Success, result[0])
        self.assertEqual([1], result[1])
        layer.commitChanges()

        self.assertEqual(
            layer.getFeature(1).geometry().asWkt(),
            "Polygon ((0 0, 5 0, 5 5, 0 5, 0 0),(3 3, 3 4, 4 4, 4 3, 3 3))",
        )
        self.assertEqual(
            layer.getFeature(2).geometry().asWkt(),
            "Polygon ((2 2, 6 2, 6 6, 2 6, 2 2))",
        )

    def testAddRingV2AtLeastOne(self):
        # test adding ring on multi features
        # Succeed if the ring can be added at least to 1 feature
        layer = createEmptyPolygonLayer()
        self.assertTrue(layer.startEditing())

        pr = layer.dataProvider()
        f1 = QgsFeature(layer.fields(), 1)
        f1.setGeometry(QgsGeometry.fromWkt("POLYGON((0 0, 5 0, 5 5, 0 5, 0 0))"))
        f2 = QgsFeature(layer.fields(), 1)
        f2.setGeometry(QgsGeometry.fromWkt("POLYGON((2 2, 6 2, 6 6, 2 6, 2 2))"))
        assert pr.addFeatures([f1, f2])
        self.assertEqual(layer.featureCount(), 2)

        vle = QgsVectorLayerEditUtils(layer)
        result = vle.addRingV2(
            QgsLineString(
                [
                    QgsPoint(0.5, 3),
                    QgsPoint(1.5, 3),
                    QgsPoint(1.5, 1.5),
                    QgsPoint(3, 1.5),
                    QgsPoint(3, 0.5),
                    QgsPoint(0.5, 0.5),
                    QgsPoint(0.5, 3),
                ]
            )
        )
        self.assertEqual(Qgis.GeometryOperationResult.Success, result[0])
        self.assertEqual({1}, set(result[1]))
        layer.commitChanges()

        self.assertEqual(
            layer.getFeature(1).geometry().asWkt(),
            "Polygon ((0 0, 5 0, 5 5, 0 5, 0 0),(0.5 3, 1.5 3, 1.5 1.5, 3 1.5, 3 0.5, 0.5 0.5, 0.5 3))",
        )
        self.assertEqual(
            layer.getFeature(2).geometry().asWkt(),
            "Polygon ((2 2, 6 2, 6 6, 2 6, 2 2))",
        )

    def testMoveVertex(self):
        layer = QgsVectorLayer("Point", "point", "memory")
        self.assertTrue(layer.startEditing())
        self.assertTrue(layer.isValid())

        pr = layer.dataProvider()
        f1 = QgsFeature(layer.fields(), 1)
        f1.setGeometry(QgsGeometry.fromWkt("Point(0 0)"))
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
        f1.setGeometry(QgsGeometry.fromWkt("PointZ(0 0 0)"))
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
        f2.setGeometry(QgsGeometry.fromWkt("Point(0 0)"))
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
        f1.setGeometry(QgsGeometry.fromWkt("PointM(0 0 0)"))
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
        f2.setGeometry(QgsGeometry.fromWkt("Point(0 0)"))
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
        f1.setGeometry(QgsGeometry.fromWkt("PointZM(0 0 0 0)"))
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
        f2.setGeometry(QgsGeometry.fromWkt("Point(0 0)"))
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

    def testSplitPolygonFeaturesWithSegment(self):
        layer = createEmptyMultiPolygonLayer()
        self.assertTrue(layer.startEditing())

        # Add one Polygon feature
        f = QgsFeature(layer.fields(), 1)
        f.setGeometry(
            QgsGeometry.fromWkt(
                "Polygon ((-0.76603773584905643 -0.10943396226415092, -0.32327044025157226 -0.12452830188679243, -0.32098304780368553 -0.12340567889760526, -0.31871547626528141 -0.1222435403926411, -0.3164684082166121 -0.12104223619671484, -0.3142425200660065 -0.11980212792422629, -0.31203848184625943 -0.11852358887030773, -0.30985695701293925 -0.11720700389845523, -0.3076986022446756 -0.11585276932467767, -0.30556406724548768 -0.11446129279819854, -0.3034539945492109 -0.11303299317874607, -0.30136901932608307 -0.11156830041046883, -0.29930976919154623 -0.1100676553925147, -0.29727686401732312 -0.10853150984631227, -0.29527091574482456 -0.10696032617959442, -0.29329252820094409 -0.10535457734720524, -0.29134229691629476 -0.103714746708732, -0.2894208089459438 -0.10204132788300511, -0.2875286426926984 -0.10033482459950989, -0.28566636773299586 -0.09859575054675489, -0.28383454464545088 -0.09682462921764222, -0.28203372484211098 -0.09502199375188673, -0.28026445040247161 -0.09318838677553128, -0.27852725391030031 -0.09132436023760636, -0.27682265829331931 -0.0894304752439835, -0.27515117666579492 -0.0875073018884721, -0.27351331217408048 -0.08555541908121089, -0.27190955784516063 -0.08357541437440552, -0.2703403964382407 -0.08156788378546451, -0.26880630029942798 -0.07953343161758727, -0.26730773121954632 -0.07747267027785786, -0.26584514029512929 -0.0753862200928993, -0.26441896779263152 -0.07327470912214402, -0.26302964301590076 -0.07113877296877645, -0.26167758417694942 -0.068979054588405, -0.26036319827006527 -0.06679620409552076, -0.25908688094929866 -0.06459087856780114, -0.25784901640936359 -0.06236374184831757, -0.25664997726998806 -0.06011546434570669, -0.25549012446374864 -0.05784672283236515, -0.25436980712742352 -0.0555582002407288, -0.25328936249689549 -0.05325058545769767, -0.25224911580563836 -0.0509245731172684, -0.25124938018681536 -0.04858086339143686, -0.25029045657902066 -0.04622016177943363, -0.24937263363569112 -0.0438431788953559, -0.24849618763821651 -0.04145063025425966, -0.24766138241277383 -0.03904323605677678, -0.24686846925091069 -0.03662172097232141, -0.24611768683390209 -0.0341868139209514, -0.24540926116090303 -0.03173924785395005, -0.24474340548091855 -0.02927975953319446, -0.24412032022861202 -0.02680908930937689, -0.24354019296397061 -0.02432798089914562, -0.2430031983158463 -0.02183718116123281, -0.24250949792938944 -0.01933743987163641, -0.24205924041739074 -0.01682950949792397, -0.24165256131554583 -0.01431414497272622, -0.2412895830416569 -0.01179210346648861, -0.24097041485878246 -0.00926414415954925, -0.24069515284234724 -0.00673102801361186, -0.24046387985122181 -0.00419351754268241, -0.24027666550278054 -0.00165237658353858, -0.24013356615194539 0.00089162993419899, -0.24003462487422217 0.00343773621832445, -0.239979871452734 0.0059851758445641, -0.23996932236925603 0.00853318198728403, -0.24000298079925414 0.01108098765031854, -0.24008083661092908 0.01362782589785, -0.24020286636826627 0.01617293008527041, -0.24036903333809059 0.01871553408995539, -0.24057928750112359 0.02125487254188101, -0.24083356556704025 0.02379018105401407, -0.24113179099352056 0.02632069645240653, -0.24147387400929013 0.02884565700592468, -0.24185971164114295 0.03136430265554411, -0.24228918774493821 0.03387587524314128, -0.24276217304056183 0.03637961873971286, -0.24327852515084203 0.03887477947295429, -0.24383808864440762 0.04136060635412871, -0.24444069508247546 0.04383635110415846, -0.24508616306955372 0.04630126847887061, -0.24577429830804523 0.04875461649332899, -0.24650489365673456 0.05119565664518521, -0.2472777291931412 0.05362365413698121, -0.2480925722797204 0.05603787809733674, -0.24894917763389121 0.05843760180095484, -0.24984728740187095 0.06082210288737943, -0.25078663123629391 0.06319066357843892, -0.2517669263775909 0.06554257089431047, -0.25278787773910505 0.06787711686813991, -0.25384917799591789 0.07019359875915257, -0.25495050767736033 0.07249131926419111, -0.25609153526317863 0.07476958672761641, -0.25727191728332832 0.07702771534950846, -0.25849129842136465 0.07926502539210464, -0.2597493116213998 0.08148084338441314, -0.26104557819859275 0.08367450232493993, -0.26237970795314075 0.0858453418824684, -0.26375129928773633 0.08799270859483101, -0.26515993932845544 0.0901159560656133, -0.2666052040490402 0.09221444515873095, -0.26808665839853818 0.09428754419082136, -0.26960385643226081 0.09633462912139186, -0.27115634144602097 0.09835508374066716, -0.27274364611360885 0.10034829985507981, -0.27436529262746623 0.10231367747034741, -0.27602079284251468 0.10425062497208183, -0.27770964842309637 0.10615855930387597, -0.27943135099298216 0.10803690614281419, -0.28118538228840206 0.10988510007235412, -0.28297121431405203 0.11170258475252715, -0.28478830950203016 0.11348881308740695, -0.28663612087365431 0.11524324738979537, -0.28851409220411284 0.11696535954307592, -0.29042165818989818 0.11865463116018653, -0.29235824461897353 0.1203105537396635, -0.29432326854362134 0.12193262881870963, -0.29631613845592064 0.12352036812324067, -0.29833625446580231 0.12507329371486459, -0.30038300848162636 0.12659093813474986, -0.30245578439322884 0.12807284454433884, -0.30455395825738235 0.12951856686286473, -0.30667689848561386 0.13092766990162988, -0.3088239660343246 0.1322997294950054, -0.31099451459715394 0.13363433262811292, -0.31318789079952891 0.13493107756114942, -0.31540343439534224 0.13618957395031836, -0.31764047846569832 0.13740944296533009, -0.31989834961966768 0.13859031740343672, -0.3221763681969903 0.13973184179996695, -0.32447384847266547 0.14083367253532716, -0.32679009886336735 0.14189547793843735, -0.32912442213562448 0.14291693838657007, -0.33147611561569945 0.14389774640156272, -0.33384447140110646 0.144837606742374, -0.33622877657370304 0.14573623649395692, -0.33862831341429117 0.14659336515242125, -0.34104235961866403 0.14740873470646024, -0.34347018851503242 0.14818209971501664, -0.34591106928276633 0.14891322738116491, -0.34836426717238528 0.1496018976221874, -0.35082904372673113 0.15024790313582315, -0.35330465700325719 0.15085104946266975, -0.35579036179736651 0.15141115504471897, -0.3582854098667318 0.15192805128000925, -0.36078905015653012 0.15240158257337785, -0.36330052902552401 0.15283160638329776, -0.3658190904729211 0.15321799326478538, -0.36834397636594429 0.15356062690836578, -0.37087442666804338 0.15385940417508398, -0.37340967966767974 0.15411423512755165, -0.37594897220761564 0.15432504305701986, -0.37849153991463819 0.15449176450646984, -0.38103661742964978 0.15461434928971471, -0.38358343863805489 0.15469276050650643, -0.3861312369003746 0.1547269745536434, -0.38867924528301878 0.15471698113207555, -0.76603773584905643 -0.10943396226415092))"
            )
        )
        assert layer.addFeatures([f])
        layer.commitChanges(stopEditing=False)

        self.assertEqual(layer.featureCount(), 1)

        vle = QgsVectorLayerEditUtils(layer)

        split_curve = QgsLineString()
        split_curve.fromWkt(
            "LineString (-0.53502791870773025 0.11478501924662199, -0.46789915300518725 -0.1872944264148213)"
        )

        result, _ = vle.splitFeatures(
            split_curve, preserveCircular=True, topologicalEditing=False
        )
        self.assertEqual(result, Qgis.GeometryOperationResult.Success)

        self.assertEqual(layer.featureCount(), 2)

        # Check that no curves are present in the split geometries
        for feature in layer.getFeatures():
            self.assertFalse(feature.geometry().constGet().hasCurvedSegments())

        layer.rollBack()

    def testSplitPolygonFeaturesWithCurve(self):
        layer = createEmptyMultiPolygonLayer()
        self.assertTrue(layer.startEditing())

        # Add one Polygon feature (same polygon used in the split with segment test)
        f = QgsFeature(layer.fields(), 1)
        f.setGeometry(
            QgsGeometry.fromWkt(
                "Polygon ((-0.76603773584905643 -0.10943396226415092, -0.32327044025157226 -0.12452830188679243, -0.32098304780368553 -0.12340567889760526, -0.31871547626528141 -0.1222435403926411, -0.3164684082166121 -0.12104223619671484, -0.3142425200660065 -0.11980212792422629, -0.31203848184625943 -0.11852358887030773, -0.30985695701293925 -0.11720700389845523, -0.3076986022446756 -0.11585276932467767, -0.30556406724548768 -0.11446129279819854, -0.3034539945492109 -0.11303299317874607, -0.30136901932608307 -0.11156830041046883, -0.29930976919154623 -0.1100676553925147, -0.29727686401732312 -0.10853150984631227, -0.29527091574482456 -0.10696032617959442, -0.29329252820094409 -0.10535457734720524, -0.29134229691629476 -0.103714746708732, -0.2894208089459438 -0.10204132788300511, -0.2875286426926984 -0.10033482459950989, -0.28566636773299586 -0.09859575054675489, -0.28383454464545088 -0.09682462921764222, -0.28203372484211098 -0.09502199375188673, -0.28026445040247161 -0.09318838677553128, -0.27852725391030031 -0.09132436023760636, -0.27682265829331931 -0.0894304752439835, -0.27515117666579492 -0.0875073018884721, -0.27351331217408048 -0.08555541908121089, -0.27190955784516063 -0.08357541437440552, -0.2703403964382407 -0.08156788378546451, -0.26880630029942798 -0.07953343161758727, -0.26730773121954632 -0.07747267027785786, -0.26584514029512929 -0.0753862200928993, -0.26441896779263152 -0.07327470912214402, -0.26302964301590076 -0.07113877296877645, -0.26167758417694942 -0.068979054588405, -0.26036319827006527 -0.06679620409552076, -0.25908688094929866 -0.06459087856780114, -0.25784901640936359 -0.06236374184831757, -0.25664997726998806 -0.06011546434570669, -0.25549012446374864 -0.05784672283236515, -0.25436980712742352 -0.0555582002407288, -0.25328936249689549 -0.05325058545769767, -0.25224911580563836 -0.0509245731172684, -0.25124938018681536 -0.04858086339143686, -0.25029045657902066 -0.04622016177943363, -0.24937263363569112 -0.0438431788953559, -0.24849618763821651 -0.04145063025425966, -0.24766138241277383 -0.03904323605677678, -0.24686846925091069 -0.03662172097232141, -0.24611768683390209 -0.0341868139209514, -0.24540926116090303 -0.03173924785395005, -0.24474340548091855 -0.02927975953319446, -0.24412032022861202 -0.02680908930937689, -0.24354019296397061 -0.02432798089914562, -0.2430031983158463 -0.02183718116123281, -0.24250949792938944 -0.01933743987163641, -0.24205924041739074 -0.01682950949792397, -0.24165256131554583 -0.01431414497272622, -0.2412895830416569 -0.01179210346648861, -0.24097041485878246 -0.00926414415954925, -0.24069515284234724 -0.00673102801361186, -0.24046387985122181 -0.00419351754268241, -0.24027666550278054 -0.00165237658353858, -0.24013356615194539 0.00089162993419899, -0.24003462487422217 0.00343773621832445, -0.239979871452734 0.0059851758445641, -0.23996932236925603 0.00853318198728403, -0.24000298079925414 0.01108098765031854, -0.24008083661092908 0.01362782589785, -0.24020286636826627 0.01617293008527041, -0.24036903333809059 0.01871553408995539, -0.24057928750112359 0.02125487254188101, -0.24083356556704025 0.02379018105401407, -0.24113179099352056 0.02632069645240653, -0.24147387400929013 0.02884565700592468, -0.24185971164114295 0.03136430265554411, -0.24228918774493821 0.03387587524314128, -0.24276217304056183 0.03637961873971286, -0.24327852515084203 0.03887477947295429, -0.24383808864440762 0.04136060635412871, -0.24444069508247546 0.04383635110415846, -0.24508616306955372 0.04630126847887061, -0.24577429830804523 0.04875461649332899, -0.24650489365673456 0.05119565664518521, -0.2472777291931412 0.05362365413698121, -0.2480925722797204 0.05603787809733674, -0.24894917763389121 0.05843760180095484, -0.24984728740187095 0.06082210288737943, -0.25078663123629391 0.06319066357843892, -0.2517669263775909 0.06554257089431047, -0.25278787773910505 0.06787711686813991, -0.25384917799591789 0.07019359875915257, -0.25495050767736033 0.07249131926419111, -0.25609153526317863 0.07476958672761641, -0.25727191728332832 0.07702771534950846, -0.25849129842136465 0.07926502539210464, -0.2597493116213998 0.08148084338441314, -0.26104557819859275 0.08367450232493993, -0.26237970795314075 0.0858453418824684, -0.26375129928773633 0.08799270859483101, -0.26515993932845544 0.0901159560656133, -0.2666052040490402 0.09221444515873095, -0.26808665839853818 0.09428754419082136, -0.26960385643226081 0.09633462912139186, -0.27115634144602097 0.09835508374066716, -0.27274364611360885 0.10034829985507981, -0.27436529262746623 0.10231367747034741, -0.27602079284251468 0.10425062497208183, -0.27770964842309637 0.10615855930387597, -0.27943135099298216 0.10803690614281419, -0.28118538228840206 0.10988510007235412, -0.28297121431405203 0.11170258475252715, -0.28478830950203016 0.11348881308740695, -0.28663612087365431 0.11524324738979537, -0.28851409220411284 0.11696535954307592, -0.29042165818989818 0.11865463116018653, -0.29235824461897353 0.1203105537396635, -0.29432326854362134 0.12193262881870963, -0.29631613845592064 0.12352036812324067, -0.29833625446580231 0.12507329371486459, -0.30038300848162636 0.12659093813474986, -0.30245578439322884 0.12807284454433884, -0.30455395825738235 0.12951856686286473, -0.30667689848561386 0.13092766990162988, -0.3088239660343246 0.1322997294950054, -0.31099451459715394 0.13363433262811292, -0.31318789079952891 0.13493107756114942, -0.31540343439534224 0.13618957395031836, -0.31764047846569832 0.13740944296533009, -0.31989834961966768 0.13859031740343672, -0.3221763681969903 0.13973184179996695, -0.32447384847266547 0.14083367253532716, -0.32679009886336735 0.14189547793843735, -0.32912442213562448 0.14291693838657007, -0.33147611561569945 0.14389774640156272, -0.33384447140110646 0.144837606742374, -0.33622877657370304 0.14573623649395692, -0.33862831341429117 0.14659336515242125, -0.34104235961866403 0.14740873470646024, -0.34347018851503242 0.14818209971501664, -0.34591106928276633 0.14891322738116491, -0.34836426717238528 0.1496018976221874, -0.35082904372673113 0.15024790313582315, -0.35330465700325719 0.15085104946266975, -0.35579036179736651 0.15141115504471897, -0.3582854098667318 0.15192805128000925, -0.36078905015653012 0.15240158257337785, -0.36330052902552401 0.15283160638329776, -0.3658190904729211 0.15321799326478538, -0.36834397636594429 0.15356062690836578, -0.37087442666804338 0.15385940417508398, -0.37340967966767974 0.15411423512755165, -0.37594897220761564 0.15432504305701986, -0.37849153991463819 0.15449176450646984, -0.38103661742964978 0.15461434928971471, -0.38358343863805489 0.15469276050650643, -0.3861312369003746 0.1547269745536434, -0.38867924528301878 0.15471698113207555, -0.76603773584905643 -0.10943396226415092))"
            )
        )
        assert layer.addFeatures([f])
        layer.commitChanges(stopEditing=False)

        self.assertEqual(layer.featureCount(), 1)

        vle = QgsVectorLayerEditUtils(layer)

        split_curve = QgsCompoundCurve()
        split_curve.fromWkt(
            "CompoundCurve (CircularString (-0.61893887583590912 0.07450775982509616, -0.5467754527056754 -0.05303689500973552, -0.56187942498874766 -0.16883401584662217))"
        )

        result, _ = vle.splitFeatures(
            split_curve, preserveCircular=True, topologicalEditing=False
        )
        self.assertEqual(result, Qgis.GeometryOperationResult.Success)

        self.assertEqual(layer.featureCount(), 2)

        # Check that curves ARE present in the split geometries
        any_curve = False
        for feature in layer.getFeatures():
            if feature.geometry().constGet().hasCurvedSegments():
                any_curve = True
                break

        self.assertTrue(any_curve)
        layer.rollBack()
        # NOTE: we test the result of the split operation, not the final geometry
        # stored in the layer when committing changes. This because GeoPackage and
        # memory layers behave differently when saving: GeoPackage converts to non-curve,
        # whereas memory layers CAN store compound curves in a QgsPolygon!!!

    def testSplitCurvePolygonFeaturesWithSegment(self):
        layer = createEmptyCurvePolygonLayer()
        self.assertTrue(layer.startEditing())

        # Add one CurvePolygon feature
        f = QgsFeature(layer.fields(), 1)
        f.setGeometry(
            QgsGeometry.fromWkt(
                "CurvePolygon (CompoundCurve ((0.48176100628930829 -0.0691823899371069, 0.95974842767295598 -0.07169811320754715),CircularString (0.95974842767295598 -0.07169811320754715, 1.01509433962264151 0.07672955974842777, 0.92452830188679247 0.23773584905660383),(0.92452830188679247 0.23773584905660383, 0.48176100628930829 -0.0691823899371069)))"
            )
        )
        assert layer.addFeatures([f])
        layer.commitChanges(stopEditing=False)

        self.assertEqual(layer.featureCount(), 1)

        vle = QgsVectorLayerEditUtils(layer)

        split_curve = QgsLineString()
        split_curve.fromWkt(
            "LineString (0.61079395915591483 0.40485712994934753, 0.69909271296411513 -1.02999761943391199)"
        )

        result, _ = vle.splitFeatures(
            split_curve, preserveCircular=True, topologicalEditing=False
        )
        self.assertEqual(result, Qgis.GeometryOperationResult.Success)

        self.assertEqual(layer.featureCount(), 2)

        # Check that curves ARE present in the split geometries
        any_curve = False
        for feature in layer.getFeatures():
            if feature.geometry().constGet().hasCurvedSegments():
                any_curve = True
                break

        self.assertTrue(any_curve)
        layer.rollBack()

    def testSplitCurvePolygonFeaturesWithCurve(self):
        layer = createEmptyCurvePolygonLayer()
        self.assertTrue(layer.startEditing())

        # Add one CurvePolygon feature (same geom used in the split with segment case)
        f = QgsFeature(layer.fields(), 1)
        f.setGeometry(
            QgsGeometry.fromWkt(
                "CurvePolygon (CompoundCurve ((0.48176100628930829 -0.0691823899371069, 0.95974842767295598 -0.07169811320754715),CircularString (0.95974842767295598 -0.07169811320754715, 1.01509433962264151 0.07672955974842777, 0.92452830188679247 0.23773584905660383),(0.92452830188679247 0.23773584905660383, 0.48176100628930829 -0.0691823899371069)))"
            )
        )
        assert layer.addFeatures([f])
        layer.commitChanges(stopEditing=False)

        self.assertEqual(layer.featureCount(), 1)

        vle = QgsVectorLayerEditUtils(layer)

        split_curve = QgsCompoundCurve()
        split_curve.fromWkt(
            "CompoundCurve (CircularString (0.58871927070386443 0.34967040881922218, 0.78739146677231586 -0.46157439179362081, 0.66598068028604018 -1.16244575014621354))"
        )

        result, _ = vle.splitFeatures(
            split_curve, preserveCircular=True, topologicalEditing=False
        )
        self.assertEqual(result, Qgis.GeometryOperationResult.Success)

        self.assertEqual(layer.featureCount(), 2)

        # Check that curves ARE present in the split geometries
        any_curve = False
        for feature in layer.getFeatures():
            if feature.geometry().constGet().hasCurvedSegments():
                any_curve = True
                break

        self.assertTrue(any_curve)
        layer.rollBack()

    def testSplitParts(self):
        layer = createEmptyMultiPolygonLayer()
        self.assertTrue(layer.startEditing())

        pr = layer.dataProvider()

        # Add three MultiPolygon features
        # Each feature is composed of two squares side by side
        # Each feature is on a separate row to form a 3*2 grid
        f = QgsFeature(layer.fields(), 1)
        f.setGeometry(
            QgsGeometry.fromWkt(
                "MULTIPOLYGON(((0 0, 4 0, 4 4, 0 4, 0 0)), ((6 0, 10 0, 10 4, 6 4, 6 0)))"
            )
        )
        assert pr.addFeatures([f])

        f = QgsFeature(layer.fields(), 2)
        f.setGeometry(
            QgsGeometry.fromWkt(
                "MULTIPOLYGON(((0 6, 4 6, 4 10, 0 10, 0 6)), ((6 6, 10 6, 10 10, 6 10, 6 6)))"
            )
        )
        assert pr.addFeatures([f])

        f = QgsFeature(layer.fields(), 3)
        f.setGeometry(
            QgsGeometry.fromWkt(
                "MULTIPOLYGON(((0 12, 4 12, 4 16, 0 16, 0 12)), ((6 12, 10 12, 10 16, 6 16, 6 12)))"
            )
        )
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
            "MultiPolygon (((2 0, 2 2, 4 2, 4 0, 2 0)),((2 2, 2 0, 0 0, 0 2, 2 2)),((2 2, 2 4, 4 4, 4 2, 2 2)),((2 4, 2 2, 0 2, 0 4, 2 4)),((6 2, 10 2, 10 0, 6 0, 6 2)),((10 2, 6 2, 6 4, 10 4, 10 2)))",
        )
        self.assertEqual(
            layer.getFeature(2).geometry().asWkt(),
            "MultiPolygon (((2 6, 2 10, 4 10, 4 6, 2 6)),((2 10, 2 6, 0 6, 0 10, 2 10)),((6 6, 10 6, 10 10, 6 10, 6 6)))",
        )

        self.assertEqual(
            layer.getFeature(3).geometry().asWkt(),
            "MultiPolygon (((2 12, 2 16, 4 16, 4 12, 2 12)),((2 16, 2 12, 0 12, 0 16, 2 16)),((6 12, 10 12, 10 16, 6 16, 6 12)))",
        )

    def testMergeFeatures(self):
        layer = createEmptyPolygonLayer()
        self.assertTrue(layer.startEditing())
        layer.addAttribute(QgsField("name", QVariant.String))
        pr = layer.dataProvider()

        f1 = QgsFeature(layer.fields(), 1)
        f1.setGeometry(QgsGeometry.fromWkt("POLYGON((0 0, 5 0, 5 5, 0 5, 0 0))"))
        f1.setAttribute("name", "uno")
        assert pr.addFeatures([f1])
        self.assertEqual(layer.featureCount(), 1)

        f2 = QgsFeature(layer.fields(), 2)
        f2.setGeometry(QgsGeometry.fromWkt("POLYGON((3 3, 8 3, 8 8, 3 8, 3 3))"))
        f2.setAttribute("name", "due")
        assert pr.addFeatures([f2])
        self.assertEqual(layer.featureCount(), 2)

        featureIds = [f1.id(), f2.id()]

        unionGeom = f1.geometry()
        unionGeom = unionGeom.combine(f2.geometry())
        self.assertFalse(unionGeom.isNull())

        vle = QgsVectorLayerEditUtils(layer)
        success, errorMessage = vle.mergeFeatures(
            f1.id(), featureIds, ["tre"], unionGeom
        )
        self.assertFalse(errorMessage)
        self.assertTrue(success)

        layer.commitChanges()

        self.assertEqual(layer.featureCount(), 1)
        mergedFeature = next(layer.getFeatures())
        self.assertEqual(
            mergedFeature.geometry().asWkt(),
            "Polygon ((5 0, 0 0, 0 5, 3 5, 3 8, 8 8, 8 3, 5 3, 5 0))",
        )
        self.assertEqual(mergedFeature.attribute("name"), "tre")

    def testMergeFeaturesIntoExisting(self):
        tempgpkg = os.path.join(
            tempfile.mkdtemp(), "testMergeFeaturesIntoExisting.gpkg"
        )
        fields = QgsFields()
        fields.append(QgsField("name", QVariant.String))

        crs = QgsCoordinateReferenceSystem("epsg:4326")
        options = QgsVectorFileWriter.SaveVectorOptions()
        options.driverName = "GPKG"
        QgsVectorFileWriter.create(
            fileName=tempgpkg,
            fields=fields,
            geometryType=QgsWkbTypes.Type.Polygon,
            srs=crs,
            transformContext=QgsCoordinateTransformContext(),
            options=options,
        )

        layer = QgsVectorLayer(tempgpkg, "my_layer", "ogr")
        self.assertTrue(layer.startEditing())
        pr = layer.dataProvider()

        f1 = QgsFeature(layer.fields(), 1)
        f1.setGeometry(QgsGeometry.fromWkt("POLYGON((0 0, 5 0, 5 5, 0 5, 0 0))"))
        f1.setAttribute("name", "uno")
        assert pr.addFeatures([f1])
        self.assertEqual(layer.featureCount(), 1)

        f2 = QgsFeature(layer.fields(), 2)
        f2.setGeometry(QgsGeometry.fromWkt("POLYGON((3 3, 8 3, 8 8, 3 8, 3 3))"))
        f2.setAttribute("name", "due")
        assert pr.addFeatures([f2])
        self.assertEqual(layer.featureCount(), 2)

        featureIds = [f1.id(), f2.id()]

        unionGeom = f1.geometry()
        unionGeom = unionGeom.combine(f2.geometry())
        self.assertFalse(unionGeom.isNull())

        vle = QgsVectorLayerEditUtils(layer)
        success, errorMessage = vle.mergeFeatures(
            f2.id(), featureIds, [2, "tre"], unionGeom
        )
        self.assertFalse(errorMessage)
        self.assertTrue(success)

        layer.commitChanges()

        self.assertEqual(layer.featureCount(), 1)
        mergedFeature = layer.getFeature(2)
        self.assertEqual(
            mergedFeature.geometry().asWkt(),
            "Polygon ((5 0, 0 0, 0 5, 3 5, 3 8, 8 8, 8 3, 5 3, 5 0))",
        )
        self.assertEqual(mergedFeature.attribute("name"), "tre")

    def test_split_policy_lines(self):
        temp_layer = QgsVectorLayer(
            "LineString?crs=epsg:3111&field=pk:int&field=field_default:real&field=field_dupe:real&field=field_unset:real&field=field_ratio:real&field=apply_default:real",
            "vl",
            "memory",
        )
        self.assertTrue(temp_layer.isValid())

        temp_layer.setDefaultValueDefinition(1, QgsDefaultValue("301"))
        temp_layer.setDefaultValueDefinition(2, QgsDefaultValue("302"))
        temp_layer.setDefaultValueDefinition(3, QgsDefaultValue("303"))
        temp_layer.setDefaultValueDefinition(4, QgsDefaultValue("304"))
        temp_layer.setDefaultValueDefinition(5, QgsDefaultValue("305", True))

        temp_layer.setFieldSplitPolicy(1, Qgis.FieldDomainSplitPolicy.DefaultValue)
        temp_layer.setFieldSplitPolicy(2, Qgis.FieldDomainSplitPolicy.Duplicate)
        temp_layer.setFieldSplitPolicy(3, Qgis.FieldDomainSplitPolicy.UnsetField)
        temp_layer.setFieldSplitPolicy(4, Qgis.FieldDomainSplitPolicy.GeometryRatio)
        # this will be ignored -- the apply on update default will override it
        temp_layer.setFieldSplitPolicy(5, Qgis.FieldDomainSplitPolicy.GeometryRatio)

        temp_layer.startEditing()

        feature = QgsVectorLayerUtils.createFeature(
            temp_layer, QgsGeometry.fromWkt("LineString( 0 0, 10 0)")
        )
        feature[1] = 3301
        feature[5] = 3305
        self.assertTrue(temp_layer.addFeature(feature))

        temp_layer.commitChanges()

        original_feature = next(temp_layer.getFeatures())
        self.assertEqual(
            original_feature.attributes(), [None, 3301.0, 302.0, 303.0, 304.0, 3305.0]
        )

        temp_layer.startEditing()

        split_curve = QgsGeometry.fromWkt(
            "LineString (0.3 0.2, 0.27 -0.2, 0.86 -0.24, 0.88 0.22)"
        )
        temp_layer.splitFeatures(split_curve.constGet(), preserveCircular=False)

        features = list(temp_layer.getFeatures())
        attributes = [f.attributes() for f in features]
        self.assertCountEqual(
            attributes,
            [
                [None, 3301.0, 302.0, 303.0, 277.53878260869567, 305.0],
                [None, 301, 302.0, QgsUnsetAttributeValue(), 8.664000000000001, 305.0],
                [None, 301, 302.0, QgsUnsetAttributeValue(), 17.797217391304347, 305.0],
            ],
        )

        temp_layer.rollBack()

    def test_split_policy_polygon(self):
        temp_layer = QgsVectorLayer(
            "Polygon?crs=epsg:3111&field=pk:int&field=field_default:real&field=field_dupe:real&field=field_unset:real&field=field_ratio:real",
            "vl",
            "memory",
        )
        self.assertTrue(temp_layer.isValid())

        temp_layer.setDefaultValueDefinition(1, QgsDefaultValue("301"))
        temp_layer.setDefaultValueDefinition(2, QgsDefaultValue("302"))
        temp_layer.setDefaultValueDefinition(3, QgsDefaultValue("303"))
        temp_layer.setDefaultValueDefinition(4, QgsDefaultValue("304"))

        temp_layer.setFieldSplitPolicy(1, Qgis.FieldDomainSplitPolicy.DefaultValue)
        temp_layer.setFieldSplitPolicy(2, Qgis.FieldDomainSplitPolicy.Duplicate)
        temp_layer.setFieldSplitPolicy(3, Qgis.FieldDomainSplitPolicy.UnsetField)
        temp_layer.setFieldSplitPolicy(4, Qgis.FieldDomainSplitPolicy.GeometryRatio)

        temp_layer.startEditing()

        feature = QgsVectorLayerUtils.createFeature(
            temp_layer, QgsGeometry.fromWkt("Polygon(( 0 0, 10 0, 10 10, 0 10, 0 0))")
        )
        feature[1] = 3301
        self.assertTrue(temp_layer.addFeature(feature))

        temp_layer.commitChanges()

        original_feature = next(temp_layer.getFeatures())
        self.assertEqual(
            original_feature.attributes(), [None, 3301.0, 302.0, 303.0, 304.0]
        )

        temp_layer.startEditing()

        split_curve = QgsGeometry.fromWkt("LineString (-2.7 4.3, 5.5 11.8, 5.28 -5.57)")
        temp_layer.splitFeatures(split_curve.constGet(), preserveCircular=False)

        def round_attributes(_attributes, places):
            res = []
            for a in _attributes:
                if isinstance(a, float):
                    res.append(round(a, places))
                else:
                    res.append(a)

            return res

        features = list(temp_layer.getFeatures())
        attributes = [round_attributes(f.attributes(), 4) for f in features]
        self.assertCountEqual(
            attributes,
            [
                [None, 3301.0, 302.0, 303.0, 147.2385],
                [None, 301, 302.0, QgsUnsetAttributeValue(), 17.3433],
                [None, 301, 302.0, QgsUnsetAttributeValue(), 139.4182],
            ],
        )

        temp_layer.rollBack()


if __name__ == "__main__":
    unittest.main()
