# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsComposition.

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

import qgis
from qgis.core import (QgsRectangle,
                       QgsCoordinateReferenceSystem,
                       QgsCoordinateTransform,
                       QGis)
from utilities import (unitTestDataPath,
                       getQgisTestApp,
                       TestCase,
                       unittest
                       #expectedFailure
                       )
# Convenience instances in case you may need them
# not used in this test
QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()

class TestQgsCoordinateTransform(TestCase):

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
        myExpectedValues = [150.1509239873580270,-35.7176936443908772,
                            150.1964384662953194,-35.6971885216629090]
        myMessage = ('Expected:\n%s\nGot:\n%s\n' %
                      ( myExpectedExtent,
                        myProjectedExtent.toString()))

        self.assertAlmostEqual(myExpectedValues[0], myProjectedExtent.xMinimum(), msg=myMessage)
        self.assertAlmostEqual(myExpectedValues[1], myProjectedExtent.yMinimum(), msg=myMessage)
        self.assertAlmostEqual(myExpectedValues[2], myProjectedExtent.xMaximum(), msg=myMessage)
        self.assertAlmostEqual(myExpectedValues[3], myProjectedExtent.yMaximum(), msg=myMessage)

if __name__ == '__main__':
    unittest.main()

