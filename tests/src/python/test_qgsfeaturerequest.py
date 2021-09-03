# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsFeatureRequest.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '12/06/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import qgis  # NOQA

from qgis.core import (QgsFeatureRequest,
                       QgsRectangle,
                       QgsExpressionContext,
                       QgsExpressionContextScope,
                       QgsFields,
                       QgsField,
                       QgsSimplifyMethod,
                       QgsCoordinateReferenceSystem,
                       QgsCoordinateTransformContext,
                       QgsGeometry,
                       Qgis)
from qgis.PyQt.QtCore import QVariant
from qgis.testing import start_app, unittest


from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsFeatureRequest(unittest.TestCase):

    def __init__(self, methodName):
        """Run once on class initialization."""
        unittest.TestCase.__init__(self, methodName)

    def testConstructors(self):
        req = QgsFeatureRequest()
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterNone)
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.NoFilter)
        self.assertEqual(req.filterFid(), -1)
        self.assertFalse(req.filterFids())
        self.assertTrue(req.filterRect().isNull())
        self.assertTrue(req.referenceGeometry().isNull())

        req = QgsFeatureRequest(55)
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterFid)
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.NoFilter)
        self.assertEqual(req.filterFid(), 55)
        self.assertFalse(req.filterFids())
        self.assertTrue(req.filterRect().isNull())
        self.assertTrue(req.referenceGeometry().isNull())

        req = QgsFeatureRequest([55, 56])
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterFids)
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.NoFilter)
        self.assertEqual(req.filterFid(), -1)
        self.assertCountEqual(req.filterFids(), [55, 56])
        self.assertTrue(req.filterRect().isNull())
        self.assertTrue(req.referenceGeometry().isNull())

        req = QgsFeatureRequest(QgsRectangle(55, 56, 57, 58))
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterNone)
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.BoundingBox)
        self.assertEqual(req.filterFid(), -1)
        self.assertCountEqual(req.filterFids(), [])
        self.assertEqual(req.filterRect(), QgsRectangle(55, 56, 57, 58))
        self.assertTrue(req.referenceGeometry().isNull())

    def testFilterRect(self):
        req = QgsFeatureRequest().setFilterRect(QgsRectangle(1, 2, 3, 4))
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterNone)
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.BoundingBox)
        self.assertEqual(req.filterFid(), -1)
        self.assertFalse(req.filterFids())
        self.assertEqual(req.filterRect(), QgsRectangle(1, 2, 3, 4))
        self.assertTrue(req.referenceGeometry().isNull())

        # setting filter rect should not change attribute filter
        req = QgsFeatureRequest().setFilterFid(5).setFilterRect(QgsRectangle(1, 2, 3, 4))
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterFid)
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.BoundingBox)
        self.assertEqual(req.filterFid(), 5)
        self.assertFalse(req.filterFids())
        self.assertEqual(req.filterRect(), QgsRectangle(1, 2, 3, 4))

        # setting attribute filter should not change filter rect
        req = QgsFeatureRequest().setFilterRect(QgsRectangle(1, 2, 3, 4)).setFilterFid(5)
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterFid)
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.BoundingBox)
        self.assertEqual(req.filterFid(), 5)
        self.assertFalse(req.filterFids())
        self.assertEqual(req.filterRect(), QgsRectangle(1, 2, 3, 4))

        # setting null rectangle should clear spatial filter
        req = QgsFeatureRequest().setFilterFid(5).setFilterRect(QgsRectangle(1, 2, 3, 4))
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterFid)
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.BoundingBox)
        self.assertEqual(req.filterFid(), 5)
        self.assertFalse(req.filterFids())
        self.assertEqual(req.filterRect(), QgsRectangle(1, 2, 3, 4))
        req.setFilterRect(QgsRectangle())
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.NoFilter)
        self.assertTrue(req.filterRect().isNull())

        # setting distance within should override filter rect
        req = QgsFeatureRequest().setFilterRect(QgsRectangle(1, 2, 3, 4))
        req.setDistanceWithin(QgsGeometry.fromWkt('LineString(0 0, 10 0, 11 2)'), 1.2)
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterNone)
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.DistanceWithin)
        self.assertEqual(req.filterFid(), -1)
        self.assertFalse(req.filterFids())
        self.assertEqual(req.referenceGeometry().asWkt(), 'LineString (0 0, 10 0, 11 2)')
        self.assertEqual(req.distanceWithin(), 1.2)

    def testDistanceWithin(self):
        req = QgsFeatureRequest().setDistanceWithin(QgsGeometry.fromWkt('LineString(0 0, 10 0, 11 2)'), 1.2)
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterNone)
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.DistanceWithin)
        self.assertEqual(req.filterFid(), -1)
        self.assertFalse(req.filterFids())
        self.assertEqual(req.referenceGeometry().asWkt(), 'LineString (0 0, 10 0, 11 2)')
        self.assertEqual(req.distanceWithin(), 1.2)
        # filter rect should reflect bounding box of linestring + 1.2
        self.assertEqual(req.filterRect(), QgsRectangle(-1.2, -1.2, 12.2, 3.2))

        # setting distance within should not change attribute filter
        req = QgsFeatureRequest().setFilterFid(5).setDistanceWithin(QgsGeometry.fromWkt('LineString(0 0, 10 0, 11 2)'), 1.2)
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterFid)
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.DistanceWithin)
        self.assertEqual(req.filterFid(), 5)
        self.assertFalse(req.filterFids())
        self.assertEqual(req.referenceGeometry().asWkt(), 'LineString (0 0, 10 0, 11 2)')
        self.assertEqual(req.distanceWithin(), 1.2)
        # filter rect should reflect bounding box of linestring + 1.2
        self.assertEqual(req.filterRect(), QgsRectangle(-1.2, -1.2, 12.2, 3.2))

        # setting attribute filter should not change distance within
        req = QgsFeatureRequest().setDistanceWithin(QgsGeometry.fromWkt('LineString(0 0, 10 0, 11 2)'), 1.2).setFilterFid(5)
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterFid)
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.DistanceWithin)
        self.assertEqual(req.filterFid(), 5)
        self.assertFalse(req.filterFids())
        self.assertEqual(req.referenceGeometry().asWkt(), 'LineString (0 0, 10 0, 11 2)')
        self.assertEqual(req.distanceWithin(), 1.2)
        # filter rect should reflect bounding box of linestring + 1.2
        self.assertEqual(req.filterRect(), QgsRectangle(-1.2, -1.2, 12.2, 3.2))

        # setting filter rect should override distance within
        req = QgsFeatureRequest().setDistanceWithin(QgsGeometry.fromWkt('LineString(0 0, 10 0, 11 2)'), 1.2)
        req.setFilterRect(QgsRectangle(1, 2, 3, 4))
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterNone)
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.BoundingBox)
        self.assertEqual(req.filterFid(), -1)
        self.assertFalse(req.filterFids())
        self.assertEqual(req.filterRect(), QgsRectangle(1, 2, 3, 4))
        self.assertTrue(req.referenceGeometry().isNull())
        self.assertEqual(req.distanceWithin(), 0)

        req = QgsFeatureRequest().setDistanceWithin(QgsGeometry.fromWkt('LineString(0 0, 10 0, 11 2)'), 1.2)
        req.setFilterRect(QgsRectangle())
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterNone)
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.NoFilter)
        self.assertEqual(req.filterFid(), -1)
        self.assertFalse(req.filterFids())
        self.assertTrue(req.filterRect().isNull())
        self.assertTrue(req.referenceGeometry().isNull())
        self.assertEqual(req.distanceWithin(), 0)

    def testFilterFid(self):
        req = QgsFeatureRequest().setFilterFid(5)
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterFid)
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.NoFilter)
        self.assertEqual(req.filterFid(), 5)
        self.assertFalse(req.filterFids())
        self.assertTrue(req.filterRect().isNull())
        self.assertTrue(req.referenceGeometry().isNull())

        # filter rect doesn't affect fid filter
        req.setFilterRect(QgsRectangle(1, 2, 3, 4))
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterFid)
        self.assertEqual(req.filterFid(), 5)
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.BoundingBox)
        self.assertEqual(req.filterRect(), QgsRectangle(1, 2, 3, 4))
        self.assertTrue(req.referenceGeometry().isNull())
        req.setFilterFid(6)
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterFid)
        self.assertEqual(req.filterFid(), 6)
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.BoundingBox)
        self.assertEqual(req.filterRect(), QgsRectangle(1, 2, 3, 4))

    def testFilterFids(self):
        req = QgsFeatureRequest().setFilterFids([5, 6])
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterFids)
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.NoFilter)
        self.assertEqual(req.filterFid(), -1)
        self.assertCountEqual(req.filterFids(), [5, 6])
        self.assertTrue(req.filterRect().isNull())
        self.assertTrue(req.referenceGeometry().isNull())

        # filter rect doesn't affect fids filter
        req.setFilterRect(QgsRectangle(1, 2, 3, 4))
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterFids)
        self.assertEqual(req.filterFid(), -1)
        self.assertCountEqual(req.filterFids(), [5, 6])
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.BoundingBox)
        self.assertEqual(req.filterRect(), QgsRectangle(1, 2, 3, 4))
        self.assertTrue(req.referenceGeometry().isNull())
        req.setFilterFids([8, 9])
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterFids)
        self.assertEqual(req.filterFid(), -1)
        self.assertCountEqual(req.filterFids(), [8, 9])
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.BoundingBox)
        self.assertEqual(req.filterRect(), QgsRectangle(1, 2, 3, 4))
        self.assertTrue(req.referenceGeometry().isNull())

    def testInvalidGeomCheck(self):
        req = QgsFeatureRequest().setFilterFids([5, 6]).setInvalidGeometryCheck(QgsFeatureRequest.GeometrySkipInvalid)
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterFids)
        self.assertEqual(req.filterFid(), -1)
        self.assertCountEqual(req.filterFids(), [5, 6])
        self.assertEqual(req.invalidGeometryCheck(), QgsFeatureRequest.GeometrySkipInvalid)
        req.setInvalidGeometryCheck(QgsFeatureRequest.GeometryNoCheck)
        self.assertEqual(req.invalidGeometryCheck(), QgsFeatureRequest.GeometryNoCheck)

    def testFilterExpression(self):
        req = QgsFeatureRequest().setFilterExpression('a=5')
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterExpression)
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.NoFilter)
        self.assertEqual(req.filterFid(), -1)
        self.assertFalse(req.filterFids())
        self.assertEqual(req.filterExpression().expression(), 'a=5')
        self.assertTrue(req.filterRect().isNull())
        self.assertTrue(req.referenceGeometry().isNull())

        # filter rect doesn't affect fids filter
        req.setFilterRect(QgsRectangle(1, 2, 3, 4))
        self.assertEqual(req.filterFid(), -1)
        self.assertFalse(req.filterFids())
        self.assertEqual(req.filterExpression().expression(), 'a=5')
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.BoundingBox)
        self.assertEqual(req.filterRect(), QgsRectangle(1, 2, 3, 4))
        self.assertTrue(req.referenceGeometry().isNull())

        req.setFilterExpression('a=8')
        self.assertEqual(req.filterFid(), -1)
        self.assertFalse(req.filterFids())
        self.assertEqual(req.filterExpression().expression(), 'a=8')
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.BoundingBox)
        self.assertEqual(req.filterRect(), QgsRectangle(1, 2, 3, 4))
        self.assertTrue(req.referenceGeometry().isNull())

    def testCombineFilter(self):
        req = QgsFeatureRequest()
        req.combineFilterExpression('b=9')
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterExpression)
        self.assertEqual(req.filterFid(), -1)
        self.assertFalse(req.filterFids())
        self.assertEqual(req.filterExpression().expression(), 'b=9')
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.NoFilter)
        self.assertTrue(req.filterRect().isNull())
        self.assertTrue(req.referenceGeometry().isNull())

        req.combineFilterExpression('a=11')
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterExpression)
        self.assertEqual(req.filterExpression().expression(), '(b=9) AND (a=11)')
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.NoFilter)
        self.assertTrue(req.filterRect().isNull())
        self.assertTrue(req.referenceGeometry().isNull())

    def testExpressionContext(self):
        req = QgsFeatureRequest()
        self.assertEqual(req.expressionContext().scopeCount(), 0)

        context = QgsExpressionContext()
        scope = QgsExpressionContextScope()
        scope.setVariable('a', 6)
        context.appendScope(scope)
        req.setExpressionContext(context)

        self.assertEqual(req.expressionContext().scopeCount(), 1)
        self.assertEqual(req.expressionContext().variable('a'), 6)

    def testDisableFilter(self):
        req = QgsFeatureRequest().setFilterFid(5).disableFilter()
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterNone)
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.NoFilter)

        req = QgsFeatureRequest().setFilterFids([5, 6]).disableFilter()
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterNone)
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.NoFilter)

        req = QgsFeatureRequest().setFilterExpression('a=5').disableFilter()
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterNone)
        self.assertEqual(req.spatialFilterType(), Qgis.SpatialFilterType.NoFilter)
        self.assertFalse(req.filterExpression())

        # disable filter does not disable spatial filter
        req = QgsFeatureRequest().setFilterExpression('a=5').setFilterRect(QgsRectangle(1, 2, 3, 4))
        req.disableFilter()
        self.assertEqual(req.filterType(), QgsFeatureRequest.FilterNone)
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
        req.setFlags(QgsFeatureRequest.ExactIntersect)
        self.assertEqual(req.flags(), QgsFeatureRequest.ExactIntersect)

    def testSubsetAttributes(self):
        req = QgsFeatureRequest()
        self.assertFalse(req.subsetOfAttributes())
        self.assertFalse(req.flags() & QgsFeatureRequest.SubsetOfAttributes)

        req.setSubsetOfAttributes([1, 4])
        self.assertEqual(req.subsetOfAttributes(), [1, 4])
        self.assertTrue(req.flags() & QgsFeatureRequest.SubsetOfAttributes)

        req.setNoAttributes()
        self.assertEqual(req.subsetOfAttributes(), [])
        self.assertTrue(req.flags() & QgsFeatureRequest.SubsetOfAttributes)

        req.setSubsetOfAttributes([])
        self.assertFalse(req.subsetOfAttributes())
        self.assertTrue(req.flags() & QgsFeatureRequest.SubsetOfAttributes)

        req.setFlags(QgsFeatureRequest.Flags())
        f = QgsFields()
        f.append(QgsField('a', QVariant.String))
        f.append(QgsField('b', QVariant.String))
        f.append(QgsField('c', QVariant.String))
        req.setSubsetOfAttributes(['a', 'c'], f)
        self.assertEqual(req.subsetOfAttributes(), [0, 2])
        self.assertTrue(req.flags() & QgsFeatureRequest.SubsetOfAttributes)

    def testSimplifyMethod(self):
        req = QgsFeatureRequest()
        self.assertEqual(req.simplifyMethod().methodType(), QgsSimplifyMethod.NoSimplification)
        method = QgsSimplifyMethod()
        method.setMethodType(QgsSimplifyMethod.PreserveTopology)
        req.setSimplifyMethod(method)
        self.assertEqual(req.simplifyMethod().methodType(), QgsSimplifyMethod.PreserveTopology)

    def testDestinationCrs(self):
        req = QgsFeatureRequest()
        self.assertFalse(req.destinationCrs().isValid())
        context = QgsCoordinateTransformContext()
        req.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:3857'), context)
        self.assertTrue(req.destinationCrs().isValid())
        self.assertEqual(req.destinationCrs().authid(), 'EPSG:3857')

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

    def testAssignment(self):
        req = QgsFeatureRequest().setFilterFids([8, 9]).setFilterRect(QgsRectangle(1, 2, 3, 4)).setInvalidGeometryCheck(QgsFeatureRequest.GeometrySkipInvalid).setLimit(6).setFlags(QgsFeatureRequest.ExactIntersect).setSubsetOfAttributes([1, 4]).setTimeout(6).setRequestMayBeNested(True)

        context = QgsExpressionContext()
        scope = QgsExpressionContextScope()
        scope.setVariable('a', 6)
        context.appendScope(scope)
        req.setExpressionContext(context)
        method = QgsSimplifyMethod()
        method.setMethodType(QgsSimplifyMethod.PreserveTopology)
        req.setSimplifyMethod(method)
        context = QgsCoordinateTransformContext()
        req.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:3857'), context)

        req2 = QgsFeatureRequest(req)
        self.assertEqual(req2.limit(), 6)
        self.assertCountEqual(req2.filterFids(), [8, 9])
        self.assertEqual(req2.filterRect(), QgsRectangle(1, 2, 3, 4))
        self.assertEqual(req2.spatialFilterType(), Qgis.SpatialFilterType.BoundingBox)
        self.assertEqual(req2.invalidGeometryCheck(), QgsFeatureRequest.GeometrySkipInvalid)
        self.assertEqual(req2.expressionContext().scopeCount(), 1)
        self.assertEqual(req2.expressionContext().variable('a'), 6)
        self.assertEqual(req2.flags(), QgsFeatureRequest.ExactIntersect | QgsFeatureRequest.SubsetOfAttributes)
        self.assertEqual(req2.subsetOfAttributes(), [1, 4])
        self.assertEqual(req2.simplifyMethod().methodType(), QgsSimplifyMethod.PreserveTopology)
        self.assertEqual(req2.destinationCrs().authid(), 'EPSG:3857')
        self.assertEqual(req2.timeout(), 6)
        self.assertTrue(req2.requestMayBeNested())

        # copy distance within request
        req = QgsFeatureRequest().setDistanceWithin(QgsGeometry.fromWkt('LineString( 0 0, 10 0, 11 2)'), 1.2)
        req2 = QgsFeatureRequest(req)
        self.assertEqual(req2.spatialFilterType(), Qgis.SpatialFilterType.DistanceWithin)
        self.assertEqual(req2.referenceGeometry().asWkt(), 'LineString (0 0, 10 0, 11 2)')
        self.assertEqual(req2.distanceWithin(), 1.2)
        self.assertEqual(req2.filterRect(), QgsRectangle(-1.2, -1.2, 12.2, 3.2))


if __name__ == '__main__':
    unittest.main()
