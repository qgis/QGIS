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
                       QgsCoordinateTransform
                       )
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
        myXForm = QgsCoordinateTransform(myUtmCrs, myGeoCrs)
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
        myXForm = QgsCoordinateTransform(myUtmCrs, myGeoCrs)
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


if __name__ == '__main__':
    unittest.main()
