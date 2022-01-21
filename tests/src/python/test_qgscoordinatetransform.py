# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsCoordinateTransform.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2012 by Tim Sutton'
__date__ = '20/08/2012'
__copyright__ = 'Copyright 2012, The QGIS Project'

import qgis  # NOQA

from qgis.core import (QgsRectangle,
                       QgsCoordinateReferenceSystem,
                       QgsCoordinateTransform,
                       QgsCoordinateTransformContext,
                       QgsDatumTransform,
                       QgsProject,
                       QgsProjUtils)
from qgis.testing import start_app, unittest

start_app()


class TestQgsCoordinateTransform(unittest.TestCase):

    def testTransformBoundingBox(self):
        """Test that we can transform a rectangular bbox from utm56s to LonLat"""
        myExtent = QgsRectangle(242270, 6043737, 246330, 6045897)
        myGeoCrs = QgsCoordinateReferenceSystem('EPSG:4326')
        myUtmCrs = QgsCoordinateReferenceSystem('EPSG:32756')
        myXForm = QgsCoordinateTransform(myUtmCrs, myGeoCrs, QgsProject.instance())
        myProjectedExtent = myXForm.transformBoundingBox(myExtent)
        myExpectedExtent = ('150.1509239873580270,-35.7176936443908772 : '
                            '150.1964384662953194,-35.6971885216629090')
        myExpectedValues = [150.1509239873580270, -35.7176936443908772,
                            150.1964384662953194, -35.6971885216629090]
        myMessage = ('Expected:\n%s\nGot:\n%s\n' %
                     (myExpectedExtent,
                      myProjectedExtent.toString()))

        self.assertAlmostEqual(myExpectedValues[0], myProjectedExtent.xMinimum(), msg=myMessage)
        self.assertAlmostEqual(myExpectedValues[1], myProjectedExtent.yMinimum(), msg=myMessage)
        self.assertAlmostEqual(myExpectedValues[2], myProjectedExtent.xMaximum(), msg=myMessage)
        self.assertAlmostEqual(myExpectedValues[3], myProjectedExtent.yMaximum(), msg=myMessage)

    def testTransformBoundingBoxSizeOverflowProtection(self):
        """Test transform bounding box size overflow protection (github issue #32302)"""
        extent = QgsRectangle(-176.0454709164556562, 89.9999999999998153, 180.0000000000000000, 90.0000000000000000)
        transform = d = QgsCoordinateTransform(QgsCoordinateReferenceSystem('EPSG:4236'), QgsCoordinateReferenceSystem('EPSG:3031'), QgsProject.instance())
        # this test checks that the line below doesn't assert and crash
        transformedExtent = transform.transformBoundingBox(extent)

    def testTransformQgsRectangle_Regression17600(self):
        """Test that rectangle transform is in the bindings"""
        myExtent = QgsRectangle(-1797107, 4392148, 6025926, 6616304)
        myGeoCrs = QgsCoordinateReferenceSystem('EPSG:4326')
        myUtmCrs = QgsCoordinateReferenceSystem('EPSG:3857')
        myXForm = QgsCoordinateTransform(myUtmCrs, myGeoCrs, QgsProject.instance())
        myTransformedExtent = myXForm.transform(myExtent)
        myTransformedExtentForward = myXForm.transform(myExtent, QgsCoordinateTransform.ForwardTransform)
        self.assertAlmostEqual(myTransformedExtentForward.xMaximum(), myTransformedExtent.xMaximum())
        self.assertAlmostEqual(myTransformedExtentForward.xMinimum(), myTransformedExtent.xMinimum())
        self.assertAlmostEqual(myTransformedExtentForward.yMaximum(), myTransformedExtent.yMaximum())
        self.assertAlmostEqual(myTransformedExtentForward.yMinimum(), myTransformedExtent.yMinimum())
        self.assertAlmostEqual(myTransformedExtentForward.xMaximum(), 54.13181426773211)
        self.assertAlmostEqual(myTransformedExtentForward.xMinimum(), -16.14368685298181)
        self.assertAlmostEqual(myTransformedExtentForward.yMaximum(), 50.971783118386895)
        self.assertAlmostEqual(myTransformedExtentForward.yMinimum(), 36.66235970825241)
        myTransformedExtentReverse = myXForm.transform(myTransformedExtent, QgsCoordinateTransform.ReverseTransform)
        self.assertAlmostEqual(myTransformedExtentReverse.xMaximum(), myExtent.xMaximum())
        self.assertAlmostEqual(myTransformedExtentReverse.xMinimum(), myExtent.xMinimum())
        self.assertAlmostEqual(myTransformedExtentReverse.yMaximum(), myExtent.yMaximum())
        self.assertAlmostEqual(myTransformedExtentReverse.yMinimum(), myExtent.yMinimum())

    def testContextProj6(self):
        """
        Various tests to ensure that datum transforms are correctly set respecting context
        """
        context = QgsCoordinateTransformContext()
        context.addCoordinateOperation(QgsCoordinateReferenceSystem('EPSG:28356'),
                                       QgsCoordinateReferenceSystem('EPSG:4283'),
                                       'proj')

        transform = QgsCoordinateTransform(QgsCoordinateReferenceSystem('EPSG:28354'), QgsCoordinateReferenceSystem('EPSG:28353'), context)
        self.assertEqual(list(transform.context().coordinateOperations().keys()), [('EPSG:28356', 'EPSG:4283')])
        # should be no coordinate operation
        self.assertEqual(transform.coordinateOperation(), '')
        # should default to allowing fallback transforms
        self.assertTrue(transform.allowFallbackTransforms())

        transform = QgsCoordinateTransform(QgsCoordinateReferenceSystem('EPSG:28356'),
                                           QgsCoordinateReferenceSystem('EPSG:4283'), context)
        self.assertTrue(transform.allowFallbackTransforms())
        context.addCoordinateOperation(QgsCoordinateReferenceSystem('EPSG:28356'),
                                       QgsCoordinateReferenceSystem('EPSG:4283'),
                                       'proj', False)
        transform = QgsCoordinateTransform(QgsCoordinateReferenceSystem('EPSG:28356'),
                                           QgsCoordinateReferenceSystem('EPSG:4283'), context)
        self.assertFalse(transform.allowFallbackTransforms())

        # matching source
        transform = QgsCoordinateTransform(QgsCoordinateReferenceSystem('EPSG:28356'), QgsCoordinateReferenceSystem('EPSG:28353'), context)
        self.assertEqual(transform.coordinateOperation(), '')
        # matching dest
        transform = QgsCoordinateTransform(QgsCoordinateReferenceSystem('EPSG:28354'),
                                           QgsCoordinateReferenceSystem('EPSG:4283'), context)
        self.assertEqual(transform.coordinateOperation(), '')
        # matching src/dest pair
        transform = QgsCoordinateTransform(QgsCoordinateReferenceSystem('EPSG:28356'),
                                           QgsCoordinateReferenceSystem('EPSG:4283'), context)
        self.assertEqual(transform.coordinateOperation(), 'proj')

        # test manual overwriting
        transform.setCoordinateOperation('proj2')
        self.assertEqual(transform.coordinateOperation(), 'proj2')
        transform.setAllowFallbackTransforms(False)
        self.assertFalse(transform.allowFallbackTransforms())
        transform.setAllowFallbackTransforms(True)
        self.assertTrue(transform.allowFallbackTransforms())

        # test that auto operation setting occurs when updating src/dest crs
        transform.setSourceCrs(QgsCoordinateReferenceSystem('EPSG:28356'))
        self.assertEqual(transform.coordinateOperation(), 'proj')
        transform.setCoordinateOperation('proj2')

        transform.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:4283'))
        self.assertEqual(transform.coordinateOperation(), 'proj')
        transform.setCoordinateOperation('proj2')

        # delayed context set
        transform = QgsCoordinateTransform()
        self.assertEqual(transform.coordinateOperation(), '')
        transform.setSourceCrs(QgsCoordinateReferenceSystem('EPSG:28356'))
        transform.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:4283'))
        self.assertEqual(transform.coordinateOperation(), '')
        transform.setContext(context)
        self.assertEqual(transform.coordinateOperation(), 'proj')
        self.assertEqual(list(transform.context().coordinateOperations().keys()), [('EPSG:28356', 'EPSG:4283')])

    def testProjectContextProj6(self):
        """
        Test creating transform using convenience constructor which takes project reference
        """
        p = QgsProject()
        context = p.transformContext()
        context.addCoordinateOperation(QgsCoordinateReferenceSystem('EPSG:28356'),
                                       QgsCoordinateReferenceSystem('EPSG:3111'), 'proj')
        p.setTransformContext(context)

        transform = QgsCoordinateTransform(QgsCoordinateReferenceSystem('EPSG:28356'), QgsCoordinateReferenceSystem('EPSG:3111'), p)
        self.assertEqual(transform.coordinateOperation(), 'proj')

    def testTransformBoundingBoxFullWorldToWebMercator(self):
        extent = QgsRectangle(-180, -90, 180, 90)
        transform = QgsCoordinateTransform(QgsCoordinateReferenceSystem('EPSG:4326'), QgsCoordinateReferenceSystem('EPSG:3857'), QgsProject.instance())
        transformedExtent = transform.transformBoundingBox(extent)
        self.assertAlmostEqual(transformedExtent.xMinimum(), -20037508.343, delta=1e-3)
        self.assertAlmostEqual(transformedExtent.yMinimum(), -44927335.427, delta=1e-3)
        self.assertAlmostEqual(transformedExtent.xMaximum(), 20037508.343, delta=1e-3)
        self.assertAlmostEqual(transformedExtent.yMaximum(), 44927335.427, delta=1e-3)


if __name__ == '__main__':
    unittest.main()
