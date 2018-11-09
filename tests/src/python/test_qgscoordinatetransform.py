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
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.core import (QgsRectangle,
                       QgsCoordinateReferenceSystem,
                       QgsCoordinateTransform,
                       QgsCoordinateTransformContext,
                       QgsDatumTransform,
                       QgsProject)
from qgis.testing import start_app, unittest

start_app()


class TestQgsCoordinateTransform(unittest.TestCase):

    def testTransformBoundingBox(self):
        """Test that we can transform a rectangular bbox from utm56s to LonLat"""
        myExtent = QgsRectangle(242270, 6043737, 246330, 6045897)
        myGeoCrs = QgsCoordinateReferenceSystem()
        myGeoCrs.createFromId(4326, QgsCoordinateReferenceSystem.EpsgCrsId)
        myUtmCrs = QgsCoordinateReferenceSystem()
        myUtmCrs.createFromId(32756, QgsCoordinateReferenceSystem.EpsgCrsId)
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

    def testTransformQgsRectangle_Regression17600(self):
        """Test that rectangle transform is in the bindings"""
        myExtent = QgsRectangle(-1797107, 4392148, 6025926, 6616304)
        myGeoCrs = QgsCoordinateReferenceSystem()
        myGeoCrs.createFromId(4326, QgsCoordinateReferenceSystem.EpsgCrsId)
        myUtmCrs = QgsCoordinateReferenceSystem()
        myUtmCrs.createFromId(3857, QgsCoordinateReferenceSystem.EpsgCrsId)
        myXForm = QgsCoordinateTransform(myUtmCrs, myGeoCrs, QgsProject.instance())
        myTransformedExtent = myXForm.transform(myExtent)
        myTransformedExtentForward = myXForm.transform(myExtent, QgsCoordinateTransform.ForwardTransform)
        self.assertAlmostEquals(myTransformedExtentForward.xMaximum(), myTransformedExtent.xMaximum())
        self.assertAlmostEquals(myTransformedExtentForward.xMinimum(), myTransformedExtent.xMinimum())
        self.assertAlmostEquals(myTransformedExtentForward.yMaximum(), myTransformedExtent.yMaximum())
        self.assertAlmostEquals(myTransformedExtentForward.yMinimum(), myTransformedExtent.yMinimum())
        self.assertAlmostEquals(myTransformedExtentForward.xMaximum(), 54.13181426773211)
        self.assertAlmostEquals(myTransformedExtentForward.xMinimum(), -16.14368685298181)
        self.assertAlmostEquals(myTransformedExtentForward.yMaximum(), 50.971783118386895)
        self.assertAlmostEquals(myTransformedExtentForward.yMinimum(), 36.66235970825241)
        myTransformedExtentReverse = myXForm.transform(myTransformedExtent, QgsCoordinateTransform.ReverseTransform)
        self.assertAlmostEquals(myTransformedExtentReverse.xMaximum(), myExtent.xMaximum())
        self.assertAlmostEquals(myTransformedExtentReverse.xMinimum(), myExtent.xMinimum())
        self.assertAlmostEquals(myTransformedExtentReverse.yMaximum(), myExtent.yMaximum())
        self.assertAlmostEquals(myTransformedExtentReverse.yMinimum(), myExtent.yMinimum())

    @unittest.skip('ifdefed out in c++ until required')
    def testContextSingle(self):
        """
        Various tests to ensure that datum transforms are correctly set respecting context
        """
        context = QgsCoordinateTransformContext()
        context.addSourceDatumTransform(QgsCoordinateReferenceSystem('EPSG:28356'), 1)
        context.addDestinationDatumTransform(QgsCoordinateReferenceSystem('EPSG:4283'), 2)
        context.addSourceDestinationDatumTransform(QgsCoordinateReferenceSystem('EPSG:28356'),
                                                   QgsCoordinateReferenceSystem('EPSG:4283'),
                                                   3, 4)

        transform = QgsCoordinateTransform(QgsCoordinateReferenceSystem('EPSG:28354'), QgsCoordinateReferenceSystem('EPSG:28353'), context)
        # should be no datum transforms
        self.assertEqual(transform.sourceDatumTransformId(), -1)
        self.assertEqual(transform.destinationDatumTransformId(), -1)
        # matching source
        transform = QgsCoordinateTransform(QgsCoordinateReferenceSystem('EPSG:28356'), QgsCoordinateReferenceSystem('EPSG:28353'), context)
        self.assertEqual(transform.sourceDatumTransformId(), 1)
        self.assertEqual(transform.destinationDatumTransformId(), -1)
        # matching dest
        transform = QgsCoordinateTransform(QgsCoordinateReferenceSystem('EPSG:28354'),
                                           QgsCoordinateReferenceSystem('EPSG:4283'), context)
        self.assertEqual(transform.sourceDatumTransformId(), -1)
        self.assertEqual(transform.destinationDatumTransformId(), 2)
        # matching src/dest pair
        transform = QgsCoordinateTransform(QgsCoordinateReferenceSystem('EPSG:28356'),
                                           QgsCoordinateReferenceSystem('EPSG:4283'), context)
        self.assertEqual(transform.sourceDatumTransformId(), 3)
        self.assertEqual(transform.destinationDatumTransformId(), 4)

        # test manual overwriting
        transform.setSourceDatumTransform(11)
        transform.setDestinationDatumTransform(13)
        self.assertEqual(transform.sourceDatumTransformId(), 11)
        self.assertEqual(transform.destinationDatumTransformId(), 13)

        # test that auto datum setting occurs when updating src/dest crs
        transform.setSourceCrs(QgsCoordinateReferenceSystem('EPSG:28356'))
        self.assertEqual(transform.sourceDatumTransformId(), 3)
        self.assertEqual(transform.destinationDatumTransformId(), 4)
        transform.setSourceDatumTransform(11)
        transform.setDestinationDatumTransform(13)

        transform.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:4283'))
        self.assertEqual(transform.sourceDatumTransformId(), 3)
        self.assertEqual(transform.destinationDatumTransformId(), 4)
        transform.setSourceDatumTransform(11)
        transform.setDestinationDatumTransform(13)

        # delayed context set
        transform = QgsCoordinateTransform()
        self.assertEqual(transform.sourceDatumTransformId(), -1)
        self.assertEqual(transform.destinationDatumTransformId(), -1)
        transform.setSourceCrs(QgsCoordinateReferenceSystem('EPSG:28356'))
        transform.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:4283'))
        self.assertEqual(transform.sourceDatumTransformId(), -1)
        self.assertEqual(transform.destinationDatumTransformId(), -1)
        transform.setContext(context)
        self.assertEqual(transform.sourceDatumTransformId(), 3)
        self.assertEqual(transform.destinationDatumTransformId(), 4)

    def testContext(self):
        """
        Various tests to ensure that datum transforms are correctly set respecting context
        """
        context = QgsCoordinateTransformContext()
        context.addSourceDestinationDatumTransform(QgsCoordinateReferenceSystem('EPSG:28356'),
                                                   QgsCoordinateReferenceSystem('EPSG:4283'),
                                                   3, 4)

        transform = QgsCoordinateTransform(QgsCoordinateReferenceSystem('EPSG:28354'), QgsCoordinateReferenceSystem('EPSG:28353'), context)
        self.assertEqual(list(transform.context().sourceDestinationDatumTransforms().keys()), [('EPSG:28356', 'EPSG:4283')])
        # should be no datum transforms
        self.assertEqual(transform.sourceDatumTransformId(), -1)
        self.assertEqual(transform.destinationDatumTransformId(), -1)
        # matching source
        transform = QgsCoordinateTransform(QgsCoordinateReferenceSystem('EPSG:28356'), QgsCoordinateReferenceSystem('EPSG:28353'), context)
        self.assertEqual(transform.sourceDatumTransformId(), -1)
        self.assertEqual(transform.destinationDatumTransformId(), -1)
        # matching dest
        transform = QgsCoordinateTransform(QgsCoordinateReferenceSystem('EPSG:28354'),
                                           QgsCoordinateReferenceSystem('EPSG:4283'), context)
        self.assertEqual(transform.sourceDatumTransformId(), -1)
        self.assertEqual(transform.destinationDatumTransformId(), -1)
        # matching src/dest pair
        transform = QgsCoordinateTransform(QgsCoordinateReferenceSystem('EPSG:28356'),
                                           QgsCoordinateReferenceSystem('EPSG:4283'), context)
        self.assertEqual(transform.sourceDatumTransformId(), 3)
        self.assertEqual(transform.destinationDatumTransformId(), 4)

        # test manual overwriting
        transform.setSourceDatumTransformId(11)
        transform.setDestinationDatumTransformId(13)
        self.assertEqual(transform.sourceDatumTransformId(), 11)
        self.assertEqual(transform.destinationDatumTransformId(), 13)

        # test that auto datum setting occurs when updating src/dest crs
        transform.setSourceCrs(QgsCoordinateReferenceSystem('EPSG:28356'))
        self.assertEqual(transform.sourceDatumTransformId(), 3)
        self.assertEqual(transform.destinationDatumTransformId(), 4)
        transform.setSourceDatumTransformId(11)
        transform.setDestinationDatumTransformId(13)

        transform.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:4283'))
        self.assertEqual(transform.sourceDatumTransformId(), 3)
        self.assertEqual(transform.destinationDatumTransformId(), 4)
        transform.setSourceDatumTransformId(11)
        transform.setDestinationDatumTransformId(13)

        # delayed context set
        transform = QgsCoordinateTransform()
        self.assertEqual(transform.sourceDatumTransformId(), -1)
        self.assertEqual(transform.destinationDatumTransformId(), -1)
        transform.setSourceCrs(QgsCoordinateReferenceSystem('EPSG:28356'))
        transform.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:4283'))
        self.assertEqual(transform.sourceDatumTransformId(), -1)
        self.assertEqual(transform.destinationDatumTransformId(), -1)
        transform.setContext(context)
        self.assertEqual(transform.sourceDatumTransformId(), 3)
        self.assertEqual(transform.destinationDatumTransformId(), 4)
        self.assertEqual(list(transform.context().sourceDestinationDatumTransforms().keys()), [('EPSG:28356', 'EPSG:4283')])

    def testProjectContext(self):
        """
        Test creating transform using convenience constructor which takes project reference
        """
        p = QgsProject()
        context = p.transformContext()
        context.addSourceDestinationDatumTransform(QgsCoordinateReferenceSystem('EPSG:28356'),
                                                   QgsCoordinateReferenceSystem('EPSG:3111'), 1, 2)
        p.setTransformContext(context)

        transform = QgsCoordinateTransform(QgsCoordinateReferenceSystem('EPSG:28356'), QgsCoordinateReferenceSystem('EPSG:3111'), p)
        self.assertEqual(transform.sourceDatumTransformId(), 1)
        self.assertEqual(transform.destinationDatumTransformId(), 2)

    def testTransformInfo(self):
        # hopefully this transform is available on all platforms!
        transforms = QgsDatumTransform.datumTransformations(QgsCoordinateReferenceSystem(4613), QgsCoordinateReferenceSystem(4326))
        self.assertTrue(len(transforms) > 0)
        self.assertIn('+towgs84=-403,684,41', [QgsDatumTransform.datumTransformToProj(t.sourceTransformId) for t in transforms])
        self.assertEqual([''] * len(transforms), [QgsDatumTransform.datumTransformToProj(t.destinationTransformId) for t in transforms])
        self.assertIn('EPSG:4613', [QgsDatumTransform.datumTransformInfo(t.sourceTransformId).sourceCrsAuthId for t in
                                    transforms])
        self.assertEqual([''] * len(transforms), [QgsDatumTransform.datumTransformInfo(t.destinationTransformId).destinationCrsAuthId for t in
                                                  transforms])

        # and the reverse
        transforms = QgsDatumTransform.datumTransformations(QgsCoordinateReferenceSystem(4326), QgsCoordinateReferenceSystem(4613))
        self.assertTrue(len(transforms) > 0)
        self.assertEqual([''] * len(transforms), [QgsDatumTransform.datumTransformToProj(t.sourceTransformId) for t in transforms])
        self.assertIn('+towgs84=-403,684,41',
                      [QgsDatumTransform.datumTransformToProj(t.destinationTransformId) for t in transforms])
        self.assertEqual([''] * len(transforms), [QgsDatumTransform.datumTransformInfo(t.sourceTransformId).destinationCrsAuthId for t in
                                                  transforms])
        self.assertIn('EPSG:4613', [QgsDatumTransform.datumTransformInfo(t.destinationTransformId).sourceCrsAuthId for t in
                                    transforms])

    def testStringToTransformId(self):
        """
        Test converting proj strings to corresponding datum IDs
        """
        self.assertEqual(QgsDatumTransform.projStringToDatumTransformId(''), -1)
        self.assertEqual(QgsDatumTransform.projStringToDatumTransformId('not'), -1)
        test_string = '+towgs84=-403,684,41'
        id = QgsDatumTransform.projStringToDatumTransformId(test_string)
        self.assertNotEqual(id, -1)
        string = QgsDatumTransform.datumTransformToProj(id)
        self.assertEqual(string, test_string)
        self.assertEqual(QgsDatumTransform.projStringToDatumTransformId(test_string.upper()), id)


if __name__ == '__main__':
    unittest.main()
