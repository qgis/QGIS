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

import qgis  # NOQA

from qgis.core import QgsPointXY

from qgis.testing import start_app, unittest

start_app()


class TestQgsPointXY(unittest.TestCase):

    def __init__(self, methodName):
        """Run once on class initialization."""
        unittest.TestCase.__init__(self, methodName)

    def setUp(self):
        self.mPoint = QgsPointXY(10.0, 10.0)

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

    def test_hash(self):
        a = QgsPointXY(2.0, 1.0)
        b = QgsPointXY(2.0, 2.0)
        c = QgsPointXY(1.0, 2.0)
        d = QgsPointXY(1.0, 1.0)
        e = QgsPointXY(2.0, 1.0)
        assert a.__hash__() != b.__hash__()
        assert e.__hash__() == a.__hash__()

        mySet = set([a, b, c, d, e])
        assert len(mySet) == 4


if __name__ == '__main__':
    unittest.main()
