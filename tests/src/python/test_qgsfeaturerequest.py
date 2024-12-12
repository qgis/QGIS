"""QGIS Unit tests for QgsFeatureRequest.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "12/06/2020"
__copyright__ = "Copyright 2020, The QGIS Project"

from qgis.PyQt.QtCore import QVariant
from qgis.core import (
    Qgis,
    QgsCoordinateReferenceSystem,
    QgsCoordinateTransformContext,
    QgsExpressionContext,
    QgsExpressionContextScope,
    QgsFeatureRequest,
    QgsField,
    QgsFields,
    QgsGeometry,
    QgsRectangle,
    QgsSimplifyMethod,
    QgsCoordinateTransform,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsFeatureRequest(QgisTestCase):

    def __init__(self, methodName):
        """Run once on class initialization."""
        QgisTestCase.__init__(self, methodName)

    def testConstructors(self):
        req = QgsFeatureRequest()
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterType.FilterNone)
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.NoFilter)
        self.assertEqual(req.filterFid(), -1)
        self.assertFalse(req.filterFids())
        self.assertTrue(req.filterRect().isNull())
        self.assertTrue(req.referenceGeometry().isNull())

        req = QgsFeatureRequest(55)
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterType.FilterFid)
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.NoFilter)
        self.assertEqual(req.filterFid(), 55)
        self.assertFalse(req.filterFids())
        self.assertTrue(req.filterRect().isNull())
        self.assertTrue(req.referenceGeometry().isNull())

        req = QgsFeatureRequest([55, 56])
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterType.FilterFids)
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.NoFilter)
        self.assertEqual(req.filterFid(), -1)
        self.assertCountEqual(req.filterFids(), [55, 56])
        self.assertTrue(req.filterRect().isNull())
        self.assertTrue(req.referenceGeometry().isNull())

        req = QgsFeatureRequest(QgsRectangle(55, 56, 57, 58))
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterType.FilterNone)
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.BoundingBox)
        self.assertEqual(req.filterFid(), -1)
        self.assertCountEqual(req.filterFids(), [])
        self.assertEqual(req.filterRect(), QgsRectangle(55, 56, 57, 58))
        self.assertTrue(req.referenceGeometry().isNull())

    def testFilterRect(self):
        req = QgsFeatureRequest().setFilterRect(QgsRectangle(1, 2, 3, 4))
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterType.FilterNone)
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.BoundingBox)
        self.assertEqual(req.filterFid(), -1)
        self.assertFalse(req.filterFids())
        self.assertEqual(req.filterRect(), QgsRectangle(1, 2, 3, 4))
        self.assertTrue(req.referenceGeometry().isNull())

        # setting filter rect should not change attribute filter
        req = (
            QgsFeatureRequest().setFilterFid(5).setFilterRect(QgsRectangle(1, 2, 3, 4))
        )
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterType.FilterFid)
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.BoundingBox)
        self.assertEqual(req.filterFid(), 5)
        self.assertFalse(req.filterFids())
        self.assertEqual(req.filterRect(), QgsRectangle(1, 2, 3, 4))

        # setting attribute filter should not change filter rect
        req = (
            QgsFeatureRequest().setFilterRect(QgsRectangle(1, 2, 3, 4)).setFilterFid(5)
        )
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterType.FilterFid)
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.BoundingBox)
        self.assertEqual(req.filterFid(), 5)
        self.assertFalse(req.filterFids())
        self.assertEqual(req.filterRect(), QgsRectangle(1, 2, 3, 4))

        # setting null rectangle should clear spatial filter
        req = (
            QgsFeatureRequest().setFilterFid(5).setFilterRect(QgsRectangle(1, 2, 3, 4))
        )
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterType.FilterFid)
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.BoundingBox)
        self.assertEqual(req.filterFid(), 5)
        self.assertFalse(req.filterFids())
        self.assertEqual(req.filterRect(), QgsRectangle(1, 2, 3, 4))
        req.setFilterRect(QgsRectangle())
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.NoFilter)
        self.assertTrue(req.filterRect().isNull())

        # setting distance within should override filter rect
        req = QgsFeatureRequest().setFilterRect(QgsRectangle(1, 2, 3, 4))
        req.setDistanceWithin(QgsGeometry.fromWkt("LineString(0 0, 10 0, 11 2)"), 1.2)
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterType.FilterNone)
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.DistanceWithin)
        self.assertEqual(req.filterFid(), -1)
        self.assertFalse(req.filterFids())
        self.assertEqual(
            req.referenceGeometry().asWkt(), "LineString (0 0, 10 0, 11 2)"
        )
        self.assertEqual(req.distanceWithin(), 1.2)

    def testDistanceWithin(self):
        req = QgsFeatureRequest().setDistanceWithin(
            QgsGeometry.fromWkt("LineString(0 0, 10 0, 11 2)"), 1.2
        )
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterType.FilterNone)
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.DistanceWithin)
        self.assertEqual(req.filterFid(), -1)
        self.assertFalse(req.filterFids())
        self.assertEqual(
            req.referenceGeometry().asWkt(), "LineString (0 0, 10 0, 11 2)"
        )
        self.assertEqual(req.distanceWithin(), 1.2)
        # filter rect should reflect bounding box of linestring + 1.2
        self.assertEqual(req.filterRect(), QgsRectangle(-1.2, -1.2, 12.2, 3.2))

        # setting distance within should not change attribute filter
        req = (
            QgsFeatureRequest()
            .setFilterFid(5)
            .setDistanceWithin(QgsGeometry.fromWkt("LineString(0 0, 10 0, 11 2)"), 1.2)
        )
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterType.FilterFid)
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.DistanceWithin)
        self.assertEqual(req.filterFid(), 5)
        self.assertFalse(req.filterFids())
        self.assertEqual(
            req.referenceGeometry().asWkt(), "LineString (0 0, 10 0, 11 2)"
        )
        self.assertEqual(req.distanceWithin(), 1.2)
        # filter rect should reflect bounding box of linestring + 1.2
        self.assertEqual(req.filterRect(), QgsRectangle(-1.2, -1.2, 12.2, 3.2))

        # setting attribute filter should not change distance within
        req = (
            QgsFeatureRequest()
            .setDistanceWithin(QgsGeometry.fromWkt("LineString(0 0, 10 0, 11 2)"), 1.2)
            .setFilterFid(5)
        )
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterType.FilterFid)
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.DistanceWithin)
        self.assertEqual(req.filterFid(), 5)
        self.assertFalse(req.filterFids())
        self.assertEqual(
            req.referenceGeometry().asWkt(), "LineString (0 0, 10 0, 11 2)"
        )
        self.assertEqual(req.distanceWithin(), 1.2)
        # filter rect should reflect bounding box of linestring + 1.2
        self.assertEqual(req.filterRect(), QgsRectangle(-1.2, -1.2, 12.2, 3.2))

        # setting filter rect should override distance within
        req = QgsFeatureRequest().setDistanceWithin(
            QgsGeometry.fromWkt("LineString(0 0, 10 0, 11 2)"), 1.2
        )
        req.setFilterRect(QgsRectangle(1, 2, 3, 4))
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterType.FilterNone)
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.BoundingBox)
        self.assertEqual(req.filterFid(), -1)
        self.assertFalse(req.filterFids())
        self.assertEqual(req.filterRect(), QgsRectangle(1, 2, 3, 4))
        self.assertTrue(req.referenceGeometry().isNull())
        self.assertEqual(req.distanceWithin(), 0)

        req = QgsFeatureRequest().setDistanceWithin(
            QgsGeometry.fromWkt("LineString(0 0, 10 0, 11 2)"), 1.2
        )
        req.setFilterRect(QgsRectangle())
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterType.FilterNone)
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.NoFilter)
        self.assertEqual(req.filterFid(), -1)
        self.assertFalse(req.filterFids())
        self.assertTrue(req.filterRect().isNull())
        self.assertTrue(req.referenceGeometry().isNull())
        self.assertEqual(req.distanceWithin(), 0)

    def testFilterFid(self):
        req = QgsFeatureRequest().setFilterFid(5)
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterType.FilterFid)
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.NoFilter)
        self.assertEqual(req.filterFid(), 5)
        self.assertFalse(req.filterFids())
        self.assertTrue(req.filterRect().isNull())
        self.assertTrue(req.referenceGeometry().isNull())

        # filter rect doesn't affect fid filter
        req.setFilterRect(QgsRectangle(1, 2, 3, 4))
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterType.FilterFid)
        self.assertEqual(req.filterFid(), 5)
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.BoundingBox)
        self.assertEqual(req.filterRect(), QgsRectangle(1, 2, 3, 4))
        self.assertTrue(req.referenceGeometry().isNull())
        req.setFilterFid(6)
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterType.FilterFid)
        self.assertEqual(req.filterFid(), 6)
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.BoundingBox)
        self.assertEqual(req.filterRect(), QgsRectangle(1, 2, 3, 4))

    def testFilterFids(self):
        req = QgsFeatureRequest().setFilterFids([5, 6])
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterType.FilterFids)
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.NoFilter)
        self.assertEqual(req.filterFid(), -1)
        self.assertCountEqual(req.filterFids(), [5, 6])
        self.assertTrue(req.filterRect().isNull())
        self.assertTrue(req.referenceGeometry().isNull())

        # filter rect doesn't affect fids filter
        req.setFilterRect(QgsRectangle(1, 2, 3, 4))
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterType.FilterFids)
        self.assertEqual(req.filterFid(), -1)
        self.assertCountEqual(req.filterFids(), [5, 6])
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.BoundingBox)
        self.assertEqual(req.filterRect(), QgsRectangle(1, 2, 3, 4))
        self.assertTrue(req.referenceGeometry().isNull())
        req.setFilterFids([8, 9])
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterType.FilterFids)
        self.assertEqual(req.filterFid(), -1)
        self.assertCountEqual(req.filterFids(), [8, 9])
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.BoundingBox)
        self.assertEqual(req.filterRect(), QgsRectangle(1, 2, 3, 4))
        self.assertTrue(req.referenceGeometry().isNull())

    def testInvalidGeomCheck(self):
        req = (
            QgsFeatureRequest()
            .setFilterFids([5, 6])
            .setInvalidGeometryCheck(
                QgsFeatureRequest.InvalidGeometryCheck.GeometrySkipInvalid
            )
        )
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterType.FilterFids)
        self.assertEqual(req.filterFid(), -1)
        self.assertCountEqual(req.filterFids(), [5, 6])
        self.assertEqual(
            req.invalidGeometryCheck(),
            QgsFeatureRequest.InvalidGeometryCheck.GeometrySkipInvalid,
        )
        req.setInvalidGeometryCheck(
            QgsFeatureRequest.InvalidGeometryCheck.GeometryNoCheck
        )
        self.assertEqual(
            req.invalidGeometryCheck(),
            QgsFeatureRequest.InvalidGeometryCheck.GeometryNoCheck,
        )

    def testFilterExpression(self):
        req = QgsFeatureRequest().setFilterExpression("a=5")
        self.assertEqual(
            req.filterType(), QgsFeatureRequest.FilterType.FilterExpression
        )
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.NoFilter)
        self.assertEqual(req.filterFid(), -1)
        self.assertFalse(req.filterFids())
        self.assertEqual(req.filterExpression().expression(), "a=5")
        self.assertTrue(req.filterRect().isNull())
        self.assertTrue(req.referenceGeometry().isNull())

        # filter rect doesn't affect fids filter
        req.setFilterRect(QgsRectangle(1, 2, 3, 4))
        self.assertEqual(req.filterFid(), -1)
        self.assertFalse(req.filterFids())
        self.assertEqual(req.filterExpression().expression(), "a=5")
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.BoundingBox)
        self.assertEqual(req.filterRect(), QgsRectangle(1, 2, 3, 4))
        self.assertTrue(req.referenceGeometry().isNull())

        req.setFilterExpression("a=8")
        self.assertEqual(req.filterFid(), -1)
        self.assertFalse(req.filterFids())
        self.assertEqual(req.filterExpression().expression(), "a=8")
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.BoundingBox)
        self.assertEqual(req.filterRect(), QgsRectangle(1, 2, 3, 4))
        self.assertTrue(req.referenceGeometry().isNull())

    def testCombineFilter(self):
        req = QgsFeatureRequest()
        req.combineFilterExpression("b=9")
        self.assertEqual(
            req.filterType(), QgsFeatureRequest.FilterType.FilterExpression
        )
        self.assertEqual(req.filterFid(), -1)
        self.assertFalse(req.filterFids())
        self.assertEqual(req.filterExpression().expression(), "b=9")
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.NoFilter)
        self.assertTrue(req.filterRect().isNull())
        self.assertTrue(req.referenceGeometry().isNull())

        req.combineFilterExpression("a=11")
        self.assertEqual(
            req.filterType(), QgsFeatureRequest.FilterType.FilterExpression
        )
        self.assertEqual(req.filterExpression().expression(), "(b=9) AND (a=11)")
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.NoFilter)
        self.assertTrue(req.filterRect().isNull())
        self.assertTrue(req.referenceGeometry().isNull())

    def testExpressionContext(self):
        req = QgsFeatureRequest()
        self.assertEqual(req.expressionContext().scopeCount(), 0)

        context = QgsExpressionContext()
        scope = QgsExpressionContextScope()
        scope.setVariable("a", 6)
        context.appendScope(scope)
        req.setExpressionContext(context)

        self.assertEqual(req.expressionContext().scopeCount(), 1)
        self.assertEqual(req.expressionContext().variable("a"), 6)

    def testDisableFilter(self):
        req = QgsFeatureRequest().setFilterFid(5).disableFilter()
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterType.FilterNone)
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.NoFilter)

        req = QgsFeatureRequest().setFilterFids([5, 6]).disableFilter()
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterType.FilterNone)
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.NoFilter)

        req = QgsFeatureRequest().setFilterExpression("a=5").disableFilter()
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterType.FilterNone)
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.NoFilter)
        self.assertFalse(req.filterExpression())

        # disable filter does not disable spatial filter
        req = (
            QgsFeatureRequest()
            .setFilterExpression("a=5")
            .setFilterRect(QgsRectangle(1, 2, 3, 4))
        )
        req.disableFilter()
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterType.FilterNone)
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.BoundingBox)
        self.assertEqual(req.filterRect(), QgsRectangle(1, 2, 3, 4))
        self.assertFalse(req.filterExpression())

    def testLimit(self):
        req = QgsFeatureRequest()
        self.assertEqual(req.limit(), -1)
        req.setLimit(6)
        self.assertEqual(req.limit(), 6)

    def testFlags(self):
        req = QgsFeatureRequest()
        self.assertFalse(req.flags())
        req.setFlags(QgsFeatureRequest.Flag.ExactIntersect)
        self.assertEqual(req.flags(), QgsFeatureRequest.Flag.ExactIntersect)

    def testSubsetAttributes(self):
        req = QgsFeatureRequest()
        self.assertFalse(req.subsetOfAttributes())
        self.assertFalse(req.flags() & QgsFeatureRequest.Flag.SubsetOfAttributes)

        req.setSubsetOfAttributes([1, 4])
        self.assertEqual(req.subsetOfAttributes(), [1, 4])
        self.assertTrue(req.flags() & QgsFeatureRequest.Flag.SubsetOfAttributes)

        req.setNoAttributes()
        self.assertEqual(req.subsetOfAttributes(), [])
        self.assertTrue(req.flags() & QgsFeatureRequest.Flag.SubsetOfAttributes)

        req.setSubsetOfAttributes([])
        self.assertFalse(req.subsetOfAttributes())
        self.assertTrue(req.flags() & QgsFeatureRequest.Flag.SubsetOfAttributes)

        req.setFlags(QgsFeatureRequest.Flags())
        f = QgsFields()
        f.append(QgsField("a", QVariant.String))
        f.append(QgsField("b", QVariant.String))
        f.append(QgsField("c", QVariant.String))
        req.setSubsetOfAttributes(["a", "c"], f)
        self.assertEqual(req.subsetOfAttributes(), [0, 2])
        self.assertTrue(req.flags() & QgsFeatureRequest.Flag.SubsetOfAttributes)

    def testSimplifyMethod(self):
        req = QgsFeatureRequest()
        self.assertEqual(
            req.simplifyMethod().methodType(),
            QgsSimplifyMethod.MethodType.NoSimplification,
        )
        method = QgsSimplifyMethod()
        method.setMethodType(QgsSimplifyMethod.MethodType.PreserveTopology)
        req.setSimplifyMethod(method)
        self.assertEqual(
            req.simplifyMethod().methodType(),
            QgsSimplifyMethod.MethodType.PreserveTopology,
        )

    def testDestinationCrs(self):
        req = QgsFeatureRequest()
        self.assertFalse(req.destinationCrs().isValid())
        context = QgsCoordinateTransformContext()
        req.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:3857"), context)
        self.assertTrue(req.destinationCrs().isValid())
        self.assertEqual(req.destinationCrs().authid(), "EPSG:3857")

    def testTimeout(self):
        req = QgsFeatureRequest()
        self.assertEqual(req.timeout(), -1)
        req.setTimeout(6)
        self.assertEqual(req.timeout(), 6)

    def testNested(self):
        req = QgsFeatureRequest()
        self.assertFalse(req.requestMayBeNested())
        req.setRequestMayBeNested(True)
        self.assertTrue(req.requestMayBeNested())

    def testCoordinateTransform(self):
        req = QgsFeatureRequest()
        self.assertFalse(req.coordinateTransform().isValid())
        req.setCoordinateTransform(
            QgsCoordinateTransform(
                QgsCoordinateReferenceSystem("EPSG:3111"),
                QgsCoordinateReferenceSystem("EPSG:3857"),
                QgsCoordinateTransformContext(),
            )
        )
        self.assertTrue(req.coordinateTransform().isValid())
        self.assertEqual(req.coordinateTransform().sourceCrs().authid(), "EPSG:3111")
        self.assertEqual(
            req.coordinateTransform().destinationCrs().authid(), "EPSG:3857"
        )

    def testAssignment(self):
        req = (
            QgsFeatureRequest()
            .setFilterFids([8, 9])
            .setFilterRect(QgsRectangle(1, 2, 3, 4))
            .setInvalidGeometryCheck(
                QgsFeatureRequest.InvalidGeometryCheck.GeometrySkipInvalid
            )
            .setLimit(6)
            .setFlags(QgsFeatureRequest.Flag.ExactIntersect)
            .setSubsetOfAttributes([1, 4])
            .setTimeout(6)
            .setRequestMayBeNested(True)
        )

        context = QgsExpressionContext()
        scope = QgsExpressionContextScope()
        scope.setVariable("a", 6)
        context.appendScope(scope)
        req.setExpressionContext(context)
        method = QgsSimplifyMethod()
        method.setMethodType(QgsSimplifyMethod.MethodType.PreserveTopology)
        req.setSimplifyMethod(method)
        context = QgsCoordinateTransformContext()
        req.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:3857"), context)
        req.setCoordinateTransform(
            QgsCoordinateTransform(
                QgsCoordinateReferenceSystem("EPSG:3111"),
                QgsCoordinateReferenceSystem("EPSG:3857"),
                QgsCoordinateTransformContext(),
            )
        )

        req2 = QgsFeatureRequest(req)
        self.assertEqual(req2.limit(), 6)
        self.assertCountEqual(req2.filterFids(), [8, 9])
        self.assertEqual(req2.filterRect(), QgsRectangle(1, 2, 3, 4))
        self.assertEqual(req2.spatialFilterType(), Qgis.SpatialFilterType.BoundingBox)
        self.assertEqual(
            req2.invalidGeometryCheck(),
            QgsFeatureRequest.InvalidGeometryCheck.GeometrySkipInvalid,
        )
        self.assertEqual(req2.expressionContext().scopeCount(), 1)
        self.assertEqual(req2.expressionContext().variable("a"), 6)
        self.assertEqual(
            req2.flags(),
            QgsFeatureRequest.Flag.ExactIntersect
            | QgsFeatureRequest.Flag.SubsetOfAttributes,
        )
        self.assertEqual(req2.subsetOfAttributes(), [1, 4])
        self.assertEqual(
            req2.simplifyMethod().methodType(),
            QgsSimplifyMethod.MethodType.PreserveTopology,
        )
        self.assertEqual(req2.destinationCrs().authid(), "EPSG:3857")
        self.assertEqual(req2.timeout(), 6)
        self.assertTrue(req2.requestMayBeNested())
        self.assertEqual(req2.coordinateTransform().sourceCrs().authid(), "EPSG:3111")
        self.assertEqual(
            req2.coordinateTransform().destinationCrs().authid(), "EPSG:3857"
        )

        # copy distance within request
        req = QgsFeatureRequest().setDistanceWithin(
            QgsGeometry.fromWkt("LineString( 0 0, 10 0, 11 2)"), 1.2
        )
        req2 = QgsFeatureRequest(req)
        self.assertEqual(
            req2.spatialFilterType(), Qgis.SpatialFilterType.DistanceWithin
        )
        self.assertEqual(
            req2.referenceGeometry().asWkt(), "LineString (0 0, 10 0, 11 2)"
        )
        self.assertEqual(req2.distanceWithin(), 1.2)
        self.assertEqual(req2.filterRect(), QgsRectangle(-1.2, -1.2, 12.2, 3.2))

    def test_compare(self):
        req1 = (
            QgsFeatureRequest()
            .setFilterFids([8, 9])
            .setFilterRect(QgsRectangle(1, 2, 3, 4))
            .setInvalidGeometryCheck(
                QgsFeatureRequest.InvalidGeometryCheck.GeometrySkipInvalid
            )
            .setLimit(6)
            .setFlags(QgsFeatureRequest.Flag.ExactIntersect)
            .setSubsetOfAttributes([1, 4])
            .setTimeout(6)
            .setRequestMayBeNested(True)
        )
        req2 = QgsFeatureRequest(req1)
        self.assertTrue(req1.compare(req1))
        self.assertTrue(req1.compare(req2))

        req3 = QgsFeatureRequest(req2)
        self.assertTrue(req3.compare(req2))
        self.assertTrue(req3.compare(req1))
        req3.setFilterFids([8, 9, 10])
        self.assertFalse(req3.compare(req1))

        req3 = QgsFeatureRequest(req2)
        req3.setFilterRect(QgsRectangle(1, 2, 3, 5))
        self.assertFalse(req3.compare(req1))

        req3 = QgsFeatureRequest(req2)
        req3.setInvalidGeometryCheck(
            QgsFeatureRequest.InvalidGeometryCheck.GeometryNoCheck
        )
        self.assertFalse(req3.compare(req1))

        req3 = QgsFeatureRequest(req2)
        req3.setLimit(7)
        self.assertFalse(req3.compare(req1))

        req3 = QgsFeatureRequest(req2)
        req3.setFlags(QgsFeatureRequest.Flag.NoGeometry)
        self.assertFalse(req3.compare(req1))

        req3 = QgsFeatureRequest(req2)
        req3.setSubsetOfAttributes([1, 4, 5])
        self.assertFalse(req3.compare(req1))

        req3 = QgsFeatureRequest(req2)
        req3.setTimeout(7)
        self.assertFalse(req3.compare(req1))

        req3 = QgsFeatureRequest(req2)
        req3.setRequestMayBeNested(False)
        self.assertFalse(req3.compare(req1))

        req3 = QgsFeatureRequest(req2)
        orderClause = QgsFeatureRequest.OrderByClause("a", False)
        order = QgsFeatureRequest.OrderBy([orderClause])
        req3.setOrderBy(order)
        self.assertFalse(req3.compare(req1))
        req4 = QgsFeatureRequest(req2)
        orderClause = QgsFeatureRequest.OrderByClause("a", False)
        order2 = QgsFeatureRequest.OrderBy([orderClause])
        req4.setOrderBy(order2)
        self.assertTrue(req4.compare(req3))
        self.assertTrue(order == order2)

        # Expression Context is not checked
        req3 = QgsFeatureRequest(req2)
        context = QgsExpressionContext()
        scope = QgsExpressionContextScope()
        scope.setVariable("a", 6)
        context.appendScope(scope)
        req3.setExpressionContext(context)
        self.assertTrue(req3.compare(req1))

        # coordinate transform
        req3 = QgsFeatureRequest(req2)
        req2.setCoordinateTransform(
            QgsCoordinateTransform(
                QgsCoordinateReferenceSystem("EPSG:3111"),
                QgsCoordinateReferenceSystem("EPSG:3857"),
                QgsCoordinateTransformContext(),
            )
        )
        self.assertFalse(req3.compare(req2))
        req3.setCoordinateTransform(req2.coordinateTransform())
        self.assertTrue(req3.compare(req2))

    def test_order_by_equality(self):

        orderClause1 = QgsFeatureRequest.OrderByClause("a", False)
        orderClause2 = QgsFeatureRequest.OrderByClause("a", False)
        self.assertTrue(orderClause1 == orderClause2)
        orderClause2 = QgsFeatureRequest.OrderByClause("b", False)
        self.assertFalse(orderClause1 == orderClause2)
        orderClause2 = QgsFeatureRequest.OrderByClause("a", True)
        self.assertFalse(orderClause1 == orderClause2)

        order1 = QgsFeatureRequest.OrderBy([orderClause1])
        order2 = QgsFeatureRequest.OrderBy([orderClause1])
        self.assertTrue(order1 == order2)
        order2 = QgsFeatureRequest.OrderBy([orderClause2])
        self.assertFalse(order1 == order2)
        order2 = QgsFeatureRequest.OrderBy([orderClause1, orderClause2])
        self.assertFalse(order1 == order2)

    def test_calculate_transform(self):
        """
        Test transform calculation
        """
        req = QgsFeatureRequest()
        # no transformation
        transform = req.calculateTransform(QgsCoordinateReferenceSystem("EPSG:4326"))
        self.assertFalse(transform.isValid())

        # transform using destination crs
        req.setDestinationCrs(
            QgsCoordinateReferenceSystem("EPSG:3857"), QgsCoordinateTransformContext()
        )
        transform = req.calculateTransform(QgsCoordinateReferenceSystem("EPSG:4326"))
        self.assertTrue(transform.isValid())
        self.assertEqual(transform.sourceCrs().authid(), "EPSG:4326")
        self.assertEqual(transform.destinationCrs().authid(), "EPSG:3857")

        # transform using a specific coordinate transform, must take precedence
        req.setCoordinateTransform(
            QgsCoordinateTransform(
                QgsCoordinateReferenceSystem("EPSG:3111"),
                QgsCoordinateReferenceSystem("EPSG:3857"),
                QgsCoordinateTransformContext(),
            )
        )
        # source crs is ignored
        transform = req.calculateTransform(QgsCoordinateReferenceSystem("EPSG:4326"))
        self.assertTrue(transform.isValid())
        self.assertEqual(transform.sourceCrs().authid(), "EPSG:3111")
        self.assertEqual(transform.destinationCrs().authid(), "EPSG:3857")


if __name__ == "__main__":
    unittest.main()
