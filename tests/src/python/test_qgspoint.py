# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsPoint.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Tim Sutton'
__date__ = '20/08/2012'
__copyright__ = 'Copyright 2012, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os

from qgis.core import QgsPoint

from utilities import (unitTestDataPath,
                       getQgisTestApp,
                       TestCase,
                       unittest,
                       expectedFailure
                       )

QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()

class TestQgsPoint(TestCase):

    def __init__(self, methodName):
        """Run once on class initialisation."""
        unittest.TestCase.__init__(self, methodName)


    def setUp(self):
        self.mPoint = QgsPoint(10.0, 10.0)

    def test_Point(self):
        myExpectedValue = 10.0
        myActualValue = self.mPoint.x()
        myMessage = 'Expected: %s Got: %s' % (myExpectedValue, myActualValue)
        assert myExpectedValue == myActualValue, myMessage


    def test_pointToString(self):
        myExpectedValue = '10, 10'
        myActualValue = self.mPoint.toString()
        myMessage = 'Expected: %s Got: %s' % (myExpectedValue, myActualValue)
        assert myExpectedValue == myActualValue, myMessage


if __name__ == '__main__':
    unittest.main()
