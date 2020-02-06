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

import qgis  # NOQA

from qgis.core import QgsPointXY, QgsPoint, QgsWkbTypes
from qgis.PyQt.QtCore import QPointF

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

    def test_issue_32443(self):
        p = QgsPoint()
        assert p.wkbType() == QgsWkbTypes.Point and p.x() != p.x() and p.y() != p.y()

        # ctor from QgsPointXY should be available
        p = QgsPoint(QgsPointXY(1, 2))
        assert p.wkbType() == QgsWkbTypes.Point and p.x() == 1 and p.y() == 2

        # ctor from QPointF should be available
        p = QgsPoint(QPointF(1, 2))
        assert p.wkbType() == QgsWkbTypes.Point and p.x() == 1 and p.y() == 2

        p = QgsPoint(1, 2)
        assert p.wkbType() == QgsWkbTypes.Point and p.x() == 1 and p.y() == 2

        p = QgsPoint(1, 2, 3)
        assert p.wkbType() == QgsWkbTypes.PointZ and p.x() == 1 and p.y() == 2 and p.z() == 3

        p = QgsPoint(1, 2, z=3)
        assert p.wkbType() == QgsWkbTypes.PointZ and p.x() == 1 and p.y() == 2 and p.z() == 3

        p = QgsPoint(1, 2, m=3)
        assert p.wkbType() == QgsWkbTypes.PointM and p.x() == 1 and p.y() == 2 and p.m() == 3

        p = QgsPoint(1, 2, wkbType=QgsWkbTypes.PointM)
        assert p.wkbType() == QgsWkbTypes.PointM and p.x() == 1 and p.y() == 2 and p.m() != p.m()

        p = QgsPoint(1, 2, 3, 4)
        assert p.wkbType() == QgsWkbTypes.PointZM and p.x() == 1 and p.y() == 2 and p.z() == 3 and p.m() == 4

        p = QgsPoint(1, 2, m=4, z=3)
        assert p.wkbType() == QgsWkbTypes.PointZM and p.x() == 1 and p.y() == 2 and p.z() == 3 and p.m() == 4


if __name__ == '__main__':
    unittest.main()
